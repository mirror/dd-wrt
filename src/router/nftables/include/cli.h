#ifndef _NFT_CLI_H_
#define _NFT_CLI_H_

#include <nftables/libnftables.h>

#if defined(HAVE_LIBREADLINE) || defined(HAVE_LIBEDIT) || defined(HAVE_LIBLINENOISE)
extern int cli_init(struct nft_ctx *nft);
#else
static inline int cli_init(struct nft_ctx *nft)
{
        return -1;
}
#endif
extern void cli_exit(int rc);

#endif
