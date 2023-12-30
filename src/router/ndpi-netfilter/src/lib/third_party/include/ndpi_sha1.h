#ifndef NDPI_SHA1_H
#define NDPI_SHA1_H
/* ================ sha1.h ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

typedef struct {
  u_int32_t state[5];
  u_int32_t count[2];
  unsigned char buffer[64];
} SHA1_CTX;

NDPI_STATIC void SHA1Transform(u_int32_t state[5], const unsigned char buffer[64]);
NDPI_STATIC void SHA1Init(SHA1_CTX* context);
NDPI_STATIC void SHA1Update(SHA1_CTX* context, const unsigned char* data, u_int32_t len);
NDPI_STATIC void SHA1Final(unsigned char digest[20], SHA1_CTX* context);
#endif