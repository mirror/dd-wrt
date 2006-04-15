/*
 *  Copyright (c) 2004 Atheros Communications Inc.,  All rights reserved.
 */

/* js_prime.c - prime number retrieval */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <openssl/dh.h>

#include "common.h"
#include "js_primes_impl.h"

#define b2B(x) ((x)>>3) /* bits to bytes */
#define JS_PRIME_SZ             1024 /* bits */
#define JS_BUF_SZ               b2B(JS_PRIME_SZ)
#define JS_NPRIMES              128

/*
 * gets a random prime number from the file of stored primes
 */ 
BIGNUM *
js_prime_get(void)
{
    u8 rand;
    int     ret, index;
    u8 *buf;

    ret = RAND_bytes(&rand, 1);
    if (ret <= 0) {
        printf("Error generating random number\n");
        return NULL;
    }

    index = rand % JS_NPRIMES;
    buf   = js_primes[index];

    return (BN_bin2bn(buf, b2B(JS_PRIME_SZ), NULL));
}

/* XXX Divy. Shoukd be moved in another file */
int RAND_poll(void)
{
	u8 rkey[32];
	if (hostapd_get_rand(rkey, sizeof(rkey))) {
		printf("Failed to get random data for WPA initialization.\n");
		return -1;
	}
	
	RAND_add(rkey, sizeof(rkey), sizeof(rkey));
	return 0;
}
