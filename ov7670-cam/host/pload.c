#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>

#include <libelf.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"

#include "image.h"
#include "ov7670-i2c.h"

#define AM33XX_PRUSS_IRAM_SIZE               8192
#define AM33XX_PRUSS_DRAM_SIZE               8192

/*
 * PRU elf executable file for a core.
 *
 * NOTE: We rely on LD to put all Program Memory into the PF_X ELF segment,
 * and all data, bss and ro.data memory variables into the PF_R|PF_W ELF
 * segment.
 */
struct pruelf {
	int fd;
	Elf *e;
	Elf32_Phdr *imem_phdr;
	Elf_Data *imem_data;
	Elf32_Phdr *dmem_phdr;
	Elf_Data *dmem_data;
};

struct prufw {
	int coreid;
	struct pruelf elf;
	void *dmem;
};

static int pru_open_elf(struct pruelf *elf, const char *filename)
{
	size_t phdrnum;
	Elf32_Phdr *phdr;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initialization failed: %s",
				elf_errmsg(-1));

	if ((elf->fd = open(filename, O_RDONLY, 0)) < 0)
		err(EXIT_FAILURE, "open \%s\" failed", filename);

	if ((elf->e = elf_begin(elf->fd, ELF_C_READ, NULL)) == NULL)
		errx(EXIT_FAILURE, "elf_begin() failed: %s.", elf_errmsg(-1));

	if (elf_kind(elf->e) != ELF_K_ELF)
		errx(EXIT_FAILURE, "%s is not an ELF object.", filename);

	if (elf_getphdrnum(elf->e, &phdrnum))
		errx(EXIT_FAILURE, "%s: elf_getphdrnum() failed: %s",
				filename, elf_errmsg(-1));

	if ((phdr = elf32_getphdr(elf->e)) == 0)
		errx(EXIT_FAILURE, "%s: elf_getphdr() failed: %s",
				filename, elf_errmsg(-1));

	while (phdrnum-- > 0) {
		if (phdr->p_flags & PF_X) {
			elf->imem_phdr = phdr;
			elf->imem_data = elf_getdata_rawchunk(elf->e,
					phdr->p_offset, phdr->p_filesz,
					SHF_ALLOC | SHF_EXECINSTR);
		} else if ((phdr->p_flags & (PF_W | PF_R)) == (PF_W | PF_R)) {
			elf->dmem_phdr = phdr;
			elf->dmem_data = elf_getdata_rawchunk(elf->e,
					phdr->p_offset, phdr->p_filesz,
					SHF_ALLOC | SHF_WRITE);
		}

		phdr++;
	}

	if (!elf->dmem_phdr || !elf->dmem_data)
		warnx("%s: could not find a .data segment\n", filename);
	if (!elf->imem_phdr || !elf->imem_data)
		warnx("%s: could not find a .text segment\n", filename);

	return elf->dmem_data && elf->imem_data ? 0 : -1;
}

static int pru_load_elf(int coreid, struct prufw *fw, const char *filename)
{
	int ret;
	unsigned int dram_id;
	unsigned int iram_id;

	memset(fw, 0, sizeof(*fw));

	fw->coreid = coreid;

	if (coreid == 0) {
		dram_id = PRUSS0_PRU0_DATARAM;
		iram_id = PRUSS0_PRU0_IRAM;
	} else if (coreid == 1) {
		dram_id = PRUSS0_PRU1_DATARAM;
		iram_id = PRUSS0_PRU1_IRAM;
	} else {
		return -1;
	}

	ret = pru_open_elf(&fw->elf, filename);
	if (ret)
		return ret;

	if (fw->elf.imem_phdr->p_memsz > AM33XX_PRUSS_IRAM_SIZE)
		errx(EXIT_FAILURE, "TEXT file section cannot fit in IRAM.\n");
	if (fw->elf.dmem_phdr->p_memsz > AM33XX_PRUSS_DRAM_SIZE)
		errx(EXIT_FAILURE, "DATA file section cannot fit in DRAM.\n");

	prussdrv_pru_disable(coreid);
	prussdrv_map_prumem (dram_id, &fw->dmem);

	/* TODO: take care of non-zero DATA or TEXT load address. */
	if (fw->elf.imem_phdr->p_memsz > fw->elf.imem_phdr->p_filesz) {
		/* must zero-fill the portion of memory not present in file
		 * (this is usually BSS segment)
		 */
		void *p = calloc(1, fw->elf.imem_phdr->p_memsz);
		prussdrv_pru_write_memory(iram_id, 0, p,
				fw->elf.imem_phdr->p_memsz);
		free(p);
	}
	prussdrv_pru_write_memory(iram_id, 0, fw->elf.imem_data->d_buf,
				fw->elf.imem_data->d_size);

	if (fw->elf.dmem_phdr->p_memsz > fw->elf.dmem_phdr->p_filesz) {
		/* must zero-fill the portion of memory not present in file
		 * (this is usually BSS segment)
		 */
		void *p = calloc(1, fw->elf.dmem_phdr->p_memsz);
		prussdrv_pru_write_memory(dram_id, 0, p,
				fw->elf.dmem_phdr->p_memsz);
		free(p);
	}
	prussdrv_pru_write_memory(dram_id, 0, fw->elf.dmem_data->d_buf,
				fw->elf.dmem_data->d_size);

	return ret;
}

static int pru_close_elf(struct prufw *fw)
{
	elf_end(fw->elf.e);
	close(fw->elf.fd);

	memset(fw, 0, sizeof(*fw));
	
	return 0;
}

static int pru_find_symbol_addr(struct prufw *fw, const char *symstr,
				uint32_t *addr)
{
	int symbol_count, i;
	Elf *elf;
	Elf_Scn *scn = NULL;
	Elf32_Shdr *shdr;
	Elf_Data *edata = NULL;

	elf = elf_begin(fw->elf.fd, ELF_C_READ, NULL);

	while((scn = elf_nextscn(elf, scn)) != NULL) {
		shdr = elf32_getshdr(scn);

		if(shdr->sh_type != SHT_SYMTAB)
			continue;

		edata = elf_getdata(scn, edata);
		symbol_count = shdr->sh_size / shdr->sh_entsize;

		for(i = 0; i < symbol_count; i++) {
			Elf32_Sym *sym;
			const char *s;

			sym = &((Elf32_Sym *)edata->d_buf)[i];
			s = elf_strptr(elf, shdr->sh_link, sym->st_name);

			if ((ELF32_ST_BIND(sym->st_info) == STB_GLOBAL)
				&& ELF32_ST_TYPE(sym->st_info) == STT_OBJECT
				&& !strcmp(symstr, s)) {

				*addr = sym->st_value;
				if (0)
					printf("%s: %08x\n", symstr,
							sym->st_value);
				return 0;

			}
		}
	}

	return -1;
}

static int pru_write_videomem_addr(struct prufw *fw, const uint32_t addr)
{
	uint32_t pru_vmem_ptr_offset;

	if (pru_find_symbol_addr(fw, "_video_memory", &pru_vmem_ptr_offset)) {
		warnx("no VMEM pointer found in PRU%d firmware\n", fw->coreid);
		return -EIO;
	}

	/* little endian PRU assumed */
	((uint8_t *)fw->dmem)[pru_vmem_ptr_offset + 0] = (addr >> 0) & 0xff;
	((uint8_t *)fw->dmem)[pru_vmem_ptr_offset + 1] = (addr >> 8) & 0xff;
	((uint8_t *)fw->dmem)[pru_vmem_ptr_offset + 2] = (addr >> 16) & 0xff;
	((uint8_t *)fw->dmem)[pru_vmem_ptr_offset + 3] = (addr >> 24) & 0xff;

	printf("Wrote VMEM address %08x to PRU%d pointer @ DMEM=0x%x\n",
			addr, fw->coreid, pru_vmem_ptr_offset);

	return 0;
}

static int pru_read_var(struct prufw *fw, const char *var, uint32_t *val)
{
	uint32_t pru_var_offset;
	uint8_t *p;

	if (pru_find_symbol_addr(fw, var, &pru_var_offset)) {
		warnx("no \"%s\" found in PRU%d firmware\n", var, fw->coreid);
		return -EIO;
	}

	/* little endian PRU assumed */
	p = ((uint8_t *)fw->dmem) + pru_var_offset;
	*val = (uint32_t)(p[0]) << 0;
	*val |= (uint32_t)(p[1]) << 8;
	*val |= (uint32_t)(p[2]) << 16;
	*val |= (uint32_t)(p[3]) << 24;

	return 0;
}

/* Print statistics gathered during PRU firmware execution */
static void dump_stats(struct prufw *fw)
{
	uint32_t val;
	if (!pru_read_var(fw, "_num_frames", &val))
		printf("   num_frames=%u\n", val);
}

int main (int argc, char *argv[])
{
	void *vmem;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	struct prufw fw[2];
	int ret;

	if (argc != 4)
		errx(EXIT_FAILURE, "Usage: %s <PRU0.elf> <PRU1.elf> <image>\n",
				argv[0]);

	printf("Initializing the PRUs...\n");
	prussdrv_init();

	/* Open PRU Interrupt */
	ret = prussdrv_open(PRU_EVTOUT_0);
	if (ret)
		errx(EXIT_FAILURE, "prussdrv_open open failed\n");

	/* Get the interrupt initialized */
	prussdrv_pruintc_init(&pruss_intc_initdata);

	ret = pru_load_elf(0, &fw[0], argv[1]);
	if (ret)
		errx(EXIT_FAILURE, "could not load \"%s\".\n", argv[1]);
	ret = pru_load_elf(1, &fw[1], argv[2]);
	if (ret)
		errx(EXIT_FAILURE, "could not load \"%s\".\n", argv[2]);

	/* Provide video memory address to PRU core */
	prussdrv_map_extmem(&vmem);
	ret = pru_write_videomem_addr(&fw[1], prussdrv_get_phys_addr(vmem));
	if (ret)
		errx(EXIT_FAILURE, "could not init VMEM.\n");

	printf("Starting ...\n");
	prussdrv_pru_enable(0);
	prussdrv_pru_enable(1);

	/* let XCLK to stabilize - this is needed for I2C to function */
	usleep(100 * 1000);
	if (ov7670_i2c_setup(1, 0x21)) {
		printf("ERROR: Could not initialize OV7670 via I2C\n");
	}

	/* let PRU run for a while */
	usleep(1 * 1000 * 1000);

	/* disable PRU and close memory mapping */
	printf("Stopping PRU... \n");
	fflush(stdout);
	prussdrv_pru_disable(0);
	prussdrv_pru_disable(1);

	dump_stats(&fw[1]);

	/* save image from the shared video memory into a PPM file */
	if (1)
		ret = save_image_rgb(vmem, 640, 480, 640 * 2, argv[3]);
	else
		ret = save_image_yuv(vmem, 640, 480, 640 * 2, argv[3]);
	if (ret)
		errx(EXIT_FAILURE, "could not save \"%s\".\n", argv[3]);

	pru_close_elf(&fw[0]);
	pru_close_elf(&fw[1]);
	prussdrv_exit();

	printf("Done.\n");

	return EXIT_SUCCESS;
}
