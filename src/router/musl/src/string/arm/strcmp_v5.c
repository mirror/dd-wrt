#if __ARM_ARCH < 6 || (__ARM_ARCH == 6 && __ARM_ARCH_6M__ < 1)

#include <string.h>

int strcmp(const char *l, const char *r)
{
	for (; *l==*r && *l; l++, r++);
	return *(unsigned char *)l - *(unsigned char *)r;
}
#endif