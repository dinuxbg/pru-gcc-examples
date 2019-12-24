/*
 * sudo apt install libgpiod-dev
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>


#include <shift-stripe.h>

int main(int argc, char *argv[])
{
	struct shiftctx *ctx;
	int i;

	/* Warning: show-pins.pl reports gpiochip number with one
	 * higher than what libgpiod expects!. */
	ctx = shift_init(argv[0],
			"gpiochip4", 12,  /* DS:   P9.30 */
			"gpiochip3", 17,  /* SHCP: P9.28 */
			"gpiochip6", 11); /* STCP: P9.23 */


	for (i = 0; i < 16; i++) {
		shift_vis(ctx, 1 << i);
		usleep(300 * 1000);
	}
	for (i = 0; i < 160; i++) {
		shift_vis(ctx, random() * random());
		usleep(30 * 1000);
	}

	shift_close(ctx);
	return EXIT_SUCCESS;
}
