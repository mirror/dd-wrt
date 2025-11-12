/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NFTABLES_NFT_H
#define NFTABLES_NFT_H

#include <config.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Just free(), but casts to a (void*). This is for places where
 * we have a const pointer that we know we want to free. We could just
 * do the (void*) cast, but free_const() makes it clear that this is
 * something we frequently need to do and it's intentional. */
#define free_const(ptr) free((void *)(ptr))

#endif /* NFTABLES_NFT_H */
