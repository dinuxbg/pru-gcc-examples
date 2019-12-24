struct shiftctx;
struct shiftctx *shift_init(const char *requester,
			    const char *ds_chip, int ds_line,
			    const char *shcp_chip, int shcp_line,
			    const char *stcp_chip, int stcp_line);
void shift_vis(struct shiftctx *ctx, unsigned long val);
void shift_close(struct shiftctx *ctx);
