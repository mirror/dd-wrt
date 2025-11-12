/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NFTUTILS_H
#define NFTUTILS_H

#include <stddef.h>

/* The maximum buffer size for (struct protoent).p_name. It is excessively large,
 * while still reasonably fitting on the stack. Arbitrarily chosen. */
#define NFT_PROTONAME_MAXSIZE 1024

bool nft_getprotobynumber(int number, char *out_name, size_t name_len);
int nft_getprotobyname(const char *name);

/* The maximum buffer size for (struct servent).s_name. It is excessively large,
 * while still reasonably fitting on the stack. Arbitrarily chosen. */
#define NFT_SERVNAME_MAXSIZE 1024

bool nft_getservbyport(int port, const char *proto, char *out_name, size_t name_len);

#endif /* NFTUTILS_H */
