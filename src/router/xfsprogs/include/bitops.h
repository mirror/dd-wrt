#ifndef __BITOPS_H__
#define __BITOPS_H__

/*
 * fls: find last bit set.
 */

#ifndef HAVE_FLS
static inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x = (x & 0xffffu) << 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x = (x & 0xffffffu) << 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x = (x & 0xfffffffu) << 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x = (x & 0x3fffffffu) << 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		r -= 1;
	}
	return r;
}
#endif /* HAVE_FLS */

static inline int fls64(__u64 x)
{
	__u32 h = x >> 32;
	if (h)
		return fls(h) + 32;
	return fls(x);
}

static inline unsigned fls_long(unsigned long l)
{
        if (sizeof(l) == 4)
                return fls(l);
        return fls64(l);
}

/*
 * ffz: find first zero bit.
 * Result is undefined if no zero bit exists.
 */
#define ffz(x)	ffs(~(x))

#endif
