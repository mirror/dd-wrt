/* gcc nft-json-file.c -o nft-json-file -lnftables */
#include <stdlib.h>
#include <nftables/libnftables.h>

int main(void)
{
	struct nft_ctx *ctx;
	int err;

	ctx = nft_ctx_new(0);
	if (!ctx) {
		perror("cannot allocate nft context");
		return EXIT_FAILURE;
	}

	nft_ctx_output_set_flags(ctx, NFT_CTX_OUTPUT_JSON);

	/* create ruleset: all commands in the buffer are atomically applied */
	err = nft_run_cmd_from_filename(ctx, "json-ruleset.nft");
	if (err < 0)
		fprintf(stderr, "failed to run nftables command\n");

	err = nft_run_cmd_from_buffer(ctx, "list ruleset");
	if (err < 0)
		fprintf(stderr, "failed to run nftables command\n");

	nft_ctx_free(ctx);

	return EXIT_SUCCESS;
}
