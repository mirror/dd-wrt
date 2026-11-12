/* minimal stand-in for Mbed TLS, needed by p256-m.c */
#include <stddef.h>
static inline void mbedtls_platform_zeroize(void *p, size_t n)
{
	volatile unsigned char *v = p;
	while (n--) *v++ = 0;
}
