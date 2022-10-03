#ifdef bKERNELMOD
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#define UINT64_C(x) x##ULL
#endif

#ifndef bKERNELMOD
#include <string.h>
#include <stdint.h>
#endif


int siphash128(const void *in, const size_t inlen, const void *k, uint8_t *out);
