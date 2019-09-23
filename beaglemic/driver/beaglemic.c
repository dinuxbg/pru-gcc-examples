/*
 * Remote processor messaging -PRU audio card
 *
 * Based on rpmsg_client_sample.c
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 * Copyright (C) 2019 Dimitar Dimitrov <dimitar@dinux.eu>
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/rpmsg.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#include "beaglemic-rpc.h"

static int aindex = SNDRV_DEFAULT_IDX1;  /* Index 0-MAX */
static char *aid = SNDRV_DEFAULT_STR1;   /* ID for this card */

module_param(aindex, int, 0444);
MODULE_PARM_DESC(aindex, "Index value for SGI HAL2 soundcard.");
module_param(aid, charp, 0444);
MODULE_PARM_DESC(aid, "ID string for SGI HAL2 soundcard.");

struct instance_data {
	struct rpmsg_device *rpdev;
	struct snd_card *card;
	struct snd_pcm_substream *substream;
	int rx_count;
	unsigned int hwptr;
	bool capture_started;

	struct work_struct trigger_work;
};

static int rpmsg_beaglemic_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	struct instance_data *idata = dev_get_drvdata(&rpdev->dev);
	struct beaglemic_pru_status *pru_status = data;

	dev_dbg(&rpdev->dev, "incoming msg %d (src: 0x%x)\n",
		 ++idata->rx_count, src);

	if (len != sizeof(*pru_status)) {
		dev_err(&rpdev->dev, "unexpected receive length: %d\n", len);
		return -EIO;
	}
#if 0
	/* send a new message now */
	ret = rpmsg_send(rpdev->ept, MSG, strlen(MSG));
	if (ret)
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
#endif

	WRITE_ONCE(idata->hwptr, pru_status->hwptr);
	dev_dbg(&rpdev->dev, "PRU reported total frame count: %u\n",
			pru_status->frame_counter);
	if (pru_status->err)
		dev_err(&rpdev->dev, "PRU reported error: %u\n",
			pru_status->err);

	snd_pcm_period_elapsed(idata->substream);

	return 0;
}


static const struct snd_pcm_hardware beaglemic_pcm_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
		 SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER |
		 SNDRV_PCM_INFO_SYNC_APPLPTR),
	.formats =          SNDRV_PCM_FMTBIT_S16_LE,
	.rates =            SNDRV_PCM_RATE_32000,
	.rate_min =         BEAGLEMIC_PCM_SAMPLE_RATE,
	.rate_max =         BEAGLEMIC_PCM_SAMPLE_RATE,
	.channels_min =     BEAGLEMIC_PCM_NCHANNELS,
	.channels_max =     BEAGLEMIC_PCM_NCHANNELS,
	.buffer_bytes_max = 1024 * 1024,
	.period_bytes_min = 1024,
	.period_bytes_max = 512 * 1024,
	.periods_min =      2,
	.periods_max =      1024,
};

static int beaglemic_pcm_hw_params(struct snd_pcm_substream *substream,
			      struct snd_pcm_hw_params *hwparams)
{
	int err;

	if (params_rate(hwparams) != BEAGLEMIC_PCM_SAMPLE_RATE) {
		dev_err(substream->pcm->card->dev,
			"Invalid sampling rate:%d\n",
			params_rate(hwparams));
		return -EINVAL;
	}

	err = snd_pcm_lib_malloc_pages(substream,
				       params_buffer_bytes(hwparams));
	if (err < 0)
		return err;

	return 0;
}

static int beaglemic_pcm_hw_free(struct snd_pcm_substream *substream)
{
	return snd_pcm_lib_free_pages(substream);
}

static int beaglemic_capture_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct instance_data *idata = snd_pcm_substream_chip(substream);

	/* Will ALSA allow simultaneous openings of a single PCM? */
	BUG_ON (idata->substream);
	idata->substream = substream;

	runtime->hw = beaglemic_pcm_hw;

	return 0;
}

static int beaglemic_capture_close(struct snd_pcm_substream *substream)
{
	struct instance_data *idata = snd_pcm_substream_chip(substream);

	idata->substream = NULL;
	return 0;
}

static int beaglemic_capture_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct instance_data *idata = snd_pcm_substream_chip(substream);
	struct beaglemic_pru_prepare_rec pru_prepare;
	int ret;

	dev_dbg(substream->pcm->card->dev, "Capture prepare: dma_addr=%pad\n",
		runtime->dma_addr);
	dev_dbg(substream->pcm->card->dev, "Capture prepare: dma_bytes=%zu\n",
		runtime->dma_bytes);
	dev_dbg(substream->pcm->card->dev, "Capture prepare: period_size=%zu\n",
		(size_t)runtime->period_size);

	idata->hwptr = 0;

	flush_work(&idata->trigger_work);

	pru_prepare.cmd = BEAGLEMIC_PRUCMD_PREPARE;
	pru_prepare.channels = BEAGLEMIC_PCM_NCHANNELS;
	pru_prepare.buffer_addr = runtime->dma_addr;
	pru_prepare.buffer_nbytes = runtime->dma_bytes;
	pru_prepare.period_size = frames_to_bytes(runtime, runtime->period_size);

	ret = rpmsg_send(idata->rpdev->ept, &pru_prepare, sizeof(pru_prepare));
	if (ret) {
		dev_err(&idata->rpdev->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}

	return 0;
}


static void pru_trigger_startstop(struct work_struct *work)
{
	struct instance_data *idata = container_of(work,
						   struct instance_data,
						   trigger_work);
	struct beaglemic_pru_simple_command cmd;
	int ret;

	if (idata->capture_started)
		cmd.cmd = BEAGLEMIC_PRUCMD_START;
	else
		cmd.cmd = BEAGLEMIC_PRUCMD_STOP;

	ret = rpmsg_send(idata->rpdev->ept, &cmd, sizeof(cmd));
	if (ret) {
		dev_err(&idata->rpdev->dev, "rpmsg_send failed: %d\n", ret);
	}
}


static int beaglemic_capture_trigger(struct snd_pcm_substream *substream, int cmd)
{
	/* Core will copy pcm->private_data to substream->private_data. */
	struct instance_data *idata = snd_pcm_substream_chip(substream);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		idata->capture_started = true;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		idata->capture_started = false;
		break;
	default:
		return -EINVAL;
	}
	smp_wmb();

	/* Use work for PCM trigger, since we can't use the sleepable rpmsg
	 * send function in the supposedly atomic PCM trigger. */
	schedule_work(&idata->trigger_work);

	return 0;
}

static snd_pcm_uframes_t
beaglemic_capture_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct instance_data *idata = snd_pcm_substream_chip(substream);

	/* Return the last position reported from PRU. */
	return bytes_to_frames(runtime, READ_ONCE(idata->hwptr));
}

static const struct snd_pcm_ops beaglemic_capture_ops = {
	.open =        beaglemic_capture_open,
	.close =       beaglemic_capture_close,
	.ioctl =       snd_pcm_lib_ioctl,
	.hw_params =   beaglemic_pcm_hw_params,
	.hw_free =     beaglemic_pcm_hw_free,
	.prepare =     beaglemic_capture_prepare,
	.trigger =     beaglemic_capture_trigger,
	.pointer =     beaglemic_capture_pointer,
};

static int beaglemic_pcm_create(struct instance_data *idata)
{
	struct snd_pcm *pcm;
	int err;

	/* create first pcm device with one input */
	err = snd_pcm_new(idata->card, "beaglemic-pcm", 0, 0, 1, &pcm);
	if (err < 0)
		return err;

	pcm->private_data = idata;
	strcpy(pcm->name, "Beaglemic PCM");

	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&beaglemic_capture_ops);
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
					   NULL,
					   256*1024, 1024 * 1024);

	return 0;
}

static struct snd_device_ops beaglemic_ops = {
	/* Empty for now. */
};

static int rpmsg_beaglemic_probe(struct rpmsg_device *rpdev)
{
	int ret;
	struct instance_data *idata;

	dev_dbg(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
					rpdev->src, rpdev->dst);

	idata = devm_kzalloc(&rpdev->dev, sizeof(*idata), GFP_KERNEL);
	if (!idata)
		return -ENOMEM;

	INIT_WORK(&idata->trigger_work, pru_trigger_startstop);

	idata->rpdev = rpdev;
	dev_set_drvdata(&rpdev->dev, idata);

#if 0
	/* TODO - get supported audio modes from PRU */
	ret = rpmsg_send(rpdev->ept, MSG, strlen(MSG));
	if (ret) {
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}
#endif

	ret = snd_card_new(&rpdev->dev, aindex, aid, THIS_MODULE, 0,
			   &idata->card);
	if (ret < 0)
		return ret;


	ret = snd_device_new(idata->card, SNDRV_DEV_LOWLEVEL, idata,
			     &beaglemic_ops);
	if (ret < 0) {
		snd_card_free(idata->card);
		return ret;
	}

	ret = beaglemic_pcm_create(idata);
	if (ret < 0) {
		snd_card_free(idata->card);
		return ret;
	}

	/* TODO - add controls via snd_ctl_add */

	strcpy(idata->card->driver, "Beaglemic Audio Driver");
	strcpy(idata->card->shortname, "Beaglemic Audio Card");
	sprintf(idata->card->longname, "%s 0x%x/0x%x",
		idata->card->shortname,
		rpdev->src, rpdev->dst);

	ret = snd_card_register(idata->card);
	if (ret < 0) {
		snd_card_free(idata->card);
		return ret;
	}

	return 0;
}

static void rpmsg_beaglemic_remove(struct rpmsg_device *rpdev)
{
	struct instance_data *idata = dev_get_drvdata(&rpdev->dev);

	snd_card_free(idata->card);

	dev_info(&rpdev->dev, "rpmsg beaglemic driver is removed\n");
}

static struct rpmsg_device_id rpmsg_driver_id_table[] = {
	{ .name	= RPMSG_BEAGLEMIC_CHAN_NAME },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_id_table);

static struct rpmsg_driver rpmsg_beaglemic_client = {
	.drv.name	= KBUILD_MODNAME,
	.id_table	= rpmsg_driver_id_table,
	.probe		= rpmsg_beaglemic_probe,
	.callback	= rpmsg_beaglemic_cb,
	.remove		= rpmsg_beaglemic_remove,
};
module_rpmsg_driver(rpmsg_beaglemic_client);

MODULE_DESCRIPTION("Beaglemic audio card utilizing PRU");
MODULE_AUTHOR("Dimitar Dimitrov <dimitar@dinux.eu>");
MODULE_LICENSE("GPL v2");
