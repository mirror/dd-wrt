#include <stdio.h>

static double power(unsigned int base, unsigned int expo);

static double power(unsigned int base, unsigned int expo)
{
	return (expo == 0) ? 1 : base * power(base, expo - 1);
}

/* idea of this function is copied from top size scaling */
const char *scale_size(unsigned long size, unsigned int exponent, int si, int humanreadable)
{
	static char up[] = { 'B', 'K', 'M', 'G', 'T', 'P', 0 };
	static char buf[BUFSIZ];
	int i;
	float base;
	long long bytes;

	base = si ? 1000.0 : 1024.0;
	bytes = size * 1024LL;

	if (!humanreadable) {
		switch (exponent) {
		case 0:
			/* default output */
			snprintf(buf, sizeof(buf), "%ld", (long int)(bytes / (long long int)base));
			return buf;
		case 1:
			/* in bytes, which can not be in SI */
			snprintf(buf, sizeof(buf), "%lld", bytes);
			return buf;
		default:
			/* In desired scale. */
			snprintf(buf, sizeof(buf), "%ld",
			        (long)(bytes / power(base, exponent-1)));
			return buf;
		}
	}

	/* human readable output */
	if (4 >= snprintf(buf, sizeof(buf), "%lld%c", bytes, up[0]))
		return buf;

	for (i = 1; up[i] != 0; i++) {
		if (si) {
			if (4 >= snprintf(buf, sizeof(buf), "%.1f%c",
			                  (float)(bytes / power(base, i)), up[i]))
				return buf;
			if (4 >= snprintf(buf, sizeof(buf), "%ld%c",
			                  (long)(bytes / power(base, i)), up[i]))
				return buf;
		} else {
			if (5 >= snprintf(buf, sizeof(buf), "%.1f%ci",
			                  (float)(bytes / power(base, i)), up[i]))
				return buf;
			if (5 >= snprintf(buf, sizeof(buf), "%ld%ci",
			                  (long)(bytes / power(base, i)), up[i]))
				return buf;
		}
	}
	/*
	 * On system where there is more than exbibyte of memory or swap the
	 * output does not fit to column. For incoming few years this should
	 * not be a big problem (wrote at Apr, 2015).
	 */
	return buf;
}
