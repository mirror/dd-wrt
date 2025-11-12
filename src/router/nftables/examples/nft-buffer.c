/* gcc nft-buffer.c -o nft-buffer -lnftables */
#include <stdlib.h>
#include <nftables/libnftables.h>

const char ruleset[] =
	"flush ruleset;"
	"add table x;"
	"add chain x y { type filter hook input priority 0; };"
	"add rule x y counter;";

int main(void)
{
	struct nft_ctx *ctx;
	int err;

	ctx = nft_ctx_new(0);
	if (!ctx) {
		perror("cannot allocate nft context");
		return EXIT_FAILURE;
	}

	/* create ruleset: all commands in the buffer are atomically applied */
	err = nft_run_cmd_from_buffer(ctx, ruleset);
	if (err < 0)
		fprintf(stderr, "failed to run nftables command\n");

	err = nft_run_cmd_from_buffer(ctx, "list ruleset");
	if (err < 0)
		fprintf(stderr, "failed to run nftables command\n");

	nft_ctx_free(ctx);

	return EXIT_SUCCESS;
}
