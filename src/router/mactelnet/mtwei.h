/*
    Mac-Telnet - Connect to RouterOS or mactelnetd devices via MAC address
    Copyright (C) 2022, Yandex <kmeaw@yandex-team.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/sha.h>

/* Define the state of the EC-SRP Algorithm. */
typedef struct mtwei_state_s {
    BN_CTX *ctx;		/* BN context for temporaries */
    EC_GROUP *curve25519;	/* Elliptic curve parameters */
    EC_POINT *g;		/* Curve25519 generator point */
    BIGNUM *order;		/* Curve25519 order */
    BIGNUM *w2m, *m2w;		/* Weierstrass-to-Montgomery and back conversion constants */
    BIGNUM *mod;		/* Curve25519 arithmetic modulus */
} mtwei_state_t;

/* Initialize the algorithm. */
void
mtwei_init (mtwei_state_t *state);

/* Generate a keypair. */
BIGNUM*
mtwei_keygen (mtwei_state_t *state, uint8_t *pubkey_out);

/* Use SHA256 to generate an SRP identifier. */
void
mtwei_id(const char *username, const char *password, const unsigned char *salt, uint8_t *validator_out);

/* Run EC-SRP cryptography and generate the response. */
void
mtwei_docrypto(mtwei_state_t *state, BIGNUM *privkey, const uint8_t *server_key, const uint8_t *client_key, uint8_t *validator, uint8_t *buf_out);

#define MTWEI_PUBKEY_LEN 33
#define MTWEI_VALIDATOR_LEN 32
