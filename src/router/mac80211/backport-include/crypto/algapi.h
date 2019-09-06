#ifndef __BACKPORT_CRYPTO_ALGAPI_H
#define __BACKPORT_CRYPTO_ALGAPI_H
#include_next <crypto/algapi.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
/* consider properly backporting this? */
static int crypto_memneq(const void *a, const void *b, size_t size)
{
	unsigned long neq = 0;

#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
	while (size >= sizeof(unsigned long)) {
		neq |= *(unsigned long *)a ^ *(unsigned long *)b;
		/* OPTIMIZER_HIDE_VAR(neq); */
		barrier();
		a += sizeof(unsigned long);
		b += sizeof(unsigned long);
		size -= sizeof(unsigned long);
	}
#endif /* CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS */
	while (size > 0) {
		neq |= *(unsigned char *)a ^ *(unsigned char *)b;
		/* OPTIMIZER_HIDE_VAR(neq); */
		barrier();
		a += 1;
		b += 1;
		size -= 1;
	}
	return neq != 0UL ? 1 : 0;
}
#endif

#endif /* __BACKPORT_CRYPTO_AEAD_H */
