/* 
 * Public Domain poly1305 from Andrew Moon
 * poly1305-donna-unrolled.c from https://github.com/floodyberry/poly1305-donna
 */

#ifndef POLY1305_H
#define POLY1305_H

#include "mod_sftp.h"

#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
#include <sys/types.h>

#define POLY1305_KEYLEN		32
#define POLY1305_TAGLEN		16

void poly1305_auth(u_char out[POLY1305_TAGLEN], const u_char *m, size_t inlen,
    const u_char key[POLY1305_KEYLEN])
    __attribute__((__bounded__(__minbytes__, 1, POLY1305_TAGLEN)))
    __attribute__((__bounded__(__buffer__, 2, 3)))
    __attribute__((__bounded__(__minbytes__, 4, POLY1305_KEYLEN)));

#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
#endif /* POLY1305_H */
