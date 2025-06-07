/*
 * strutils.c - various string routines shared by commands
 * This file was originally copied from util-linux at fall 2011.
 *
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 * Copyright (C) 2010 Davidlohr Bueso <dave@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <assert.h>
#include <wchar.h>
#include "xalloc.h"
#include "c.h"
#include "xalloc.h"
#include "strutils.h"

/*
 * same as strtol(3) but exit on failure instead of returning crap
 */
long strtol_or_err(const char *str, const char *errmesg)
{
	long num;
	char *end = NULL;

	if (str != NULL && *str != '\0') {
		errno = 0;
		num = strtol(str, &end, 10);
		if (errno == 0 && str != end && end != NULL && *end == '\0')
			return num;
	}
	error(EXIT_FAILURE, errno, "%s: '%s'", errmesg, str);
	return 0;
}

/*
 * same as strtod(3) but exit on failure instead of returning crap
 */
double strtod_or_err(const char *str, const char *errmesg)
{
	double num;
	char *end = NULL;

	if (str != NULL && *str != '\0') {
		errno = 0;
		num = strtod(str, &end);
		if (errno == 0 && str != end && end != NULL && *end == '\0')
			return num;
	}
	error(EXIT_FAILURE, errno, "%s: '%s'", errmesg, str);
	return 0;
}

/*
 * Covert a string into a double in a non-locale aware way.
 * This means the decimal point can be either . or ,
 * Also means you cannot use the other for thousands separator
 *
 * Exits on failure like its other _or_err cousins
 */
double strtod_nol_or_err(const char *str, const char *errmesg)
{
	double num;
	const char *cp, *radix;
	double mult;
	int negative = 0;

	if (str != NULL && *str != '\0') {
		num = 0.0;
		cp = str;
		/* strip leading spaces */
		while (isspace(*cp))
			cp++;

		/* get sign */
		if (*cp == '-') {
			negative = 1;
			cp++;
		} else if (*cp == '+')
			cp++;

		/* find radix */
		radix = cp;
		mult=0.1;
		while(isdigit(*radix)) {
			radix++;
			mult *= 10;
		}
		while(isdigit(*cp)) {
			num += (*cp - '0') * mult;
			mult /= 10;
			cp++;
		}
		/* got the integers */
		if (*cp == '\0')
			return (negative?-num:num);
		if (*cp != '.' && *cp != ',')
			error(EXIT_FAILURE, EINVAL, "%s: '%s'", errmesg, str);

		cp++;
		mult = 0.1;
		while(isdigit(*cp)) {
			num += (*cp - '0') * mult;
			mult /= 10;
			cp++;
		}
		if (*cp == '\0')
			return (negative?-num:num);
	}
	error(EXIT_FAILURE, errno, "%s: '%s'", errmesg, str);
	return 0;
}

// column width of a multi-byte string
// s is \0-term.
// pwcs !=NULL => address of s converted to wide string is stored in *pwcs, will
// be \0-term., no additional cost in receiving it, caller free()s.
// Error => -1 and *pwcs is unchanged.
//
// When addstr()ing in ncurses, make use of pwcs and addwstr() it, cause else
// ncurses will do the whole conversion and validation of the sequences in s
// again.
int mbswidth(const char *restrict s, wchar_t *restrict *const restrict pwcs)
{
	assert(s);

	size_t wclen = mbstowcs(NULL, s, 0);  // wclen doesn't incl. \0
	if (wclen == (size_t)-1) {
		errno = EILSEQ;
		return -1;
	}
	++wclen;  // ok, it's !=SIZE_MAX
	if ((uintmax_t)wclen * sizeof(wchar_t) / sizeof(wchar_t) != wclen) {
		errno = EOVERFLOW;
		return -1;
	}

	wchar_t *wcs = xmalloc(wclen * sizeof(*wcs));
	mbstowcs(wcs, s, wclen);

	// wcswidth() is useless. POSIX 2001 doesn't say what it does when the width
	// is > INT_MAX. May be some UB.
	int ret = 0;
	int curwid;
	for (--wclen; wclen; --wclen) {
		curwid = wcwidth(wcs[wclen-1]);
		if (curwid == -1) {
			free(wcs);
			errno = EILSEQ;
			return -1;
		}
		if (INT_MAX - ret < curwid) {
			free(wcs);
			errno = EOVERFLOW;
			return -1;
		}
		ret += curwid;
	}

	if (pwcs) {
		*pwcs = wcs;
	} else free(wcs);
	return ret;
}

// Stable mergesort (the stability of qsort() is unspecified).
// In and out in R/W array "base".
void stablesort(void *const base, size_t nritems, size_t itemsize, int (*cmp)(const void *, const void *)) {
	const size_t arraysize = nritems * itemsize;
	void *base2;
	unsigned char *in, *out;
	size_t seglen, merged, lseg_done, rseg_done, rseg_len;

	if (arraysize <= 1)
		return;
	base2 = xmalloc(arraysize);
	// the top-most cycle will run at least once, no need to memcpy to base2[]

	in = base;
	out = base2;
	seglen = 1;

	while (true) {
		assert(nritems > seglen);
		// is there still more than one full segment left?
		// addition safe, there can never be more merged items than nritems
		for (merged=0; nritems-merged>seglen; merged+=seglen+rseg_len) {
			// lseg start idx = merged
			// lseg len = seglen
			// rseg start idx = merged+seglen
			// rseg len = min(seglen, nritems-merged-seglen)i
			lseg_done = rseg_done = 0;
			if (seglen <= nritems-merged-seglen)
				rseg_len = seglen;  // most of the time
			else rseg_len = nritems-merged-seglen;
			// lseg len > 0, rseg len > 0
			while (lseg_done < seglen && rseg_done < rseg_len) {
				if (cmp(in+itemsize*(merged+lseg_done), in+itemsize*(merged+seglen+rseg_done)) <= 0) {
					memcpy(out+itemsize*(merged+lseg_done+rseg_done), in+itemsize*(merged+lseg_done), itemsize);
					++lseg_done;
				}
				else {
					memcpy(out+itemsize*(merged+lseg_done+rseg_done), in+itemsize*(merged+seglen+rseg_done), itemsize);
					++rseg_done;
				}
			}
			if (lseg_done < seglen)
				memcpy(out+itemsize*(merged+lseg_done+rseg_done), in+itemsize*(merged+lseg_done), itemsize*(seglen-lseg_done));
			else memcpy(out+itemsize*(merged+lseg_done+rseg_done), in+itemsize*(merged+seglen+rseg_done), itemsize*(rseg_len-rseg_done));
		}
		if (merged < nritems)
			memcpy(out+itemsize*merged, in+itemsize*merged, itemsize*(nritems-merged));

		if (SIZE_MAX-seglen < seglen || (seglen*=2) >= nritems)
			break;

		base2=in; in=out; out=base2;
	}

	if (base == in) {
		memcpy(base, out, arraysize);
		free(out);
	}
	else free(in);
}

/*
int cmpint(const void *i1, const void *i2) {
	if (*(int *)i1 < *(int *)i2) return -1;
	if (*(int *)i1 == *(int *)i2) return 0;
	return 1;
}

int cmpintptr(const void *i1, const void *i2) {
	if (**(int **)i1 < **(int **)i2) return -1;
	if (**(int **)i1 == **(int **)i2) return 0;
	return 1;
}

int main() {
	int a[] = {-90,90,3,3,3,5,3,-1,0,0,90,91,90,5,4,40,30,40,20};
	const size_t anr = sizeof(a) / sizeof(*a);
	//int b[] = {-90,90,3,3,3,5,3,-1,0,0,90,91,90,5,4,40,30,40,20};
	int b[] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	const size_t bnr = sizeof(b) / sizeof(*b);
	int *c[bnr];
	const size_t cnr = bnr;
	unsigned i;

	for (i=0; i<anr; ++i)
		printf("%d ", a[i]);
	putchar('\n');
	stablesort(a, anr, sizeof(*a), cmpint);
	for (i=0; i<anr; ++i)
		printf("%d ", a[i]);
	putchar('\n');
	putchar('\n');

	// stability test
	for (i=0; i<bnr; ++i)
		c[i] = b+i;
	for (i=0; i<cnr; ++i)
		printf("c[%u] = b+%td = %d\n", i, c[i]-b, *c[i]);
	putchar('\n');
	stablesort(c, cnr, sizeof(*c), cmpintptr);
	for (i=0; i<cnr; ++i)
		printf("c[%u] = b+%td = %d\n", i, c[i]-b, *c[i]);

	return 0;
}
*/
