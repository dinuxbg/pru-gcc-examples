#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <gpiod.h>


#include "shift-stripe.h"

struct shiftctx {
	struct gpiod_chip *ds_chip;
	struct gpiod_line *ds_line;
	struct gpiod_chip *shcp_chip;
	struct gpiod_line *shcp_line;
	struct gpiod_chip *stcp_chip;
	struct gpiod_line *stcp_line;
};

struct shiftctx *shift_init(const char *requester,
			    const char *ds_chip, int ds_line,
			    const char *shcp_chip, int shcp_line,
			    const char *stcp_chip, int stcp_line)
{
	struct shiftctx *ctx;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;

	ctx->ds_chip = gpiod_chip_open_lookup(ds_chip);
	if (ctx->ds_chip == NULL) {
		fprintf(stderr,"Could not find GPIO chip %s\n", ds_chip);
		exit(EXIT_FAILURE);
	}

	ctx->shcp_chip = gpiod_chip_open_lookup(shcp_chip);
	if (ctx->shcp_chip == NULL) {
		fprintf(stderr,"Could not find GPIO chip %s\n", shcp_chip);
		exit(EXIT_FAILURE);
	}

	ctx->stcp_chip = gpiod_chip_open_lookup(stcp_chip);
	if (ctx->stcp_chip == NULL) {
		fprintf(stderr,"Could not find GPIO chip %s\n", stcp_chip);
		exit(EXIT_FAILURE);
	}

	ctx->ds_line = gpiod_chip_get_line(ctx->ds_chip, ds_line);
	if (ctx->ds_line == NULL) {
		fprintf(stderr,"Could not init GPIO chip %s line %d\n", ds_chip, ds_line);
		exit(EXIT_FAILURE);
	}

	ctx->shcp_line = gpiod_chip_get_line(ctx->shcp_chip, shcp_line);
	if (ctx->shcp_line == NULL) {
		fprintf(stderr,"Could not init GPIO chip %s line %d\n", shcp_chip, shcp_line);
		exit(EXIT_FAILURE);
	}

	ctx->stcp_line = gpiod_chip_get_line(ctx->stcp_chip, stcp_line);
	if (ctx->stcp_line == NULL) {
		fprintf(stderr,"Could not init GPIO chip %s line %d\n", stcp_chip, stcp_line);
		exit(EXIT_FAILURE);
	}

	if (gpiod_line_request_output(ctx->ds_line, requester, 0) < 0) {
		fprintf(stderr,"Could not set GPIO as output\n");
		exit(EXIT_FAILURE);
	}

	if (gpiod_line_request_output(ctx->shcp_line, requester, 0) < 0) {
		fprintf(stderr,"Could not set GPIO as output\n");
		exit(EXIT_FAILURE);
	}

	if (gpiod_line_request_output(ctx->stcp_line, requester, 0) < 0) {
		fprintf(stderr,"Could not set GPIO as output\n");
		exit(EXIT_FAILURE);
	}

	return ctx;
}

void shift_vis(struct shiftctx *ctx, unsigned long val)
{
	const int nbits = 16;
	const bool lsb_first = false;
	int i;

	for (i = 0; i < nbits; i++) {
		unsigned long chkmsk = lsb_first ? 1 : (1 << (nbits - 1));

		gpiod_line_set_value(ctx->ds_line, !!(chkmsk & val));
		/* No delays, since GPIO access is supposedly slow enough
		 * to fit the 74HC595 timing requirements. */
		gpiod_line_set_value(ctx->shcp_line, 1);
		gpiod_line_set_value(ctx->shcp_line, 0);

		if (lsb_first)
			val >>= 1;
		else
			val <<= 1;
	}
	/* Strobe */
	gpiod_line_set_value(ctx->stcp_line, 1);
	gpiod_line_set_value(ctx->stcp_line, 0);
}

void shift_close(struct shiftctx *ctx)
{
	gpiod_line_release(ctx->ds_line);
	gpiod_line_release(ctx->shcp_line);
	gpiod_line_release(ctx->stcp_line);

	free(ctx);
}
