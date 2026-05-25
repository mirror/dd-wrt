/* ISC license. */

#include "sha256-internal.h"

#define F1(x, y, z) ((x & y) | ((~x) & z))
#define F2(x, y, z) ((x & y) | (x & z) | (y & z))

#define ROTR(x,n) (((x)>>(n)) | ((x)<<(32-(n))))
#define CAPITALSIGMA0(x) (ROTR(x,2)^ROTR(x,13)^ROTR(x,22))
#define CAPITALSIGMA1(x) (ROTR(x,6)^ROTR(x,11)^ROTR(x,25))
#define SMALLSIGMA0(x) (ROTR(x,7)^ROTR(x,18)^((x)>>3))
#define SMALLSIGMA1(x) (ROTR(x,17)^ROTR(x,19)^((x)>>10))

static uint32_t const sha256_constants[64] =
{
  0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
  0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
  0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
  0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
  0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
  0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
  0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
  0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
  0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
  0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
  0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
  0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
  0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
  0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
  0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
  0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
} ;

void sha256_transform (uint32_t *buf, uint32_t const *in)
{
  uint32_t w[64] ;
  unsigned int i = 0 ;
  uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3], e = buf[4], f = buf[5], g = buf[6], h = buf[7] ;

  for (; i < 16 ; i++) w[i] = in[i] ;
  for (; i < 64 ; i++)
    w[i] = SMALLSIGMA1(w[i-2]) + w[i-7] + SMALLSIGMA0(w[i-15]) + w[i-16] ;
  for (i = 0 ; i < 64 ; i++)
  {
    uint32_t temp1 = h + CAPITALSIGMA1(e) + F1(e, f, g) + sha256_constants[i] + w[i] ;
    uint32_t temp2 = CAPITALSIGMA0(a) + F2(a, b, c) ;
    h = g ; g = f ; f = e ; e = d + temp1 ;
    d = c ; c = b ; b = a ; a = temp1 + temp2 ;
  }
  buf[0] += a ; buf[1] += b ; buf[2] += c ; buf[3] += d ;
  buf[4] += e ; buf[5] += f ; buf[6] += g ; buf[7] += h ;
}
