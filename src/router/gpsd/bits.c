/* bits.c - bitfield extraction code
 *
 * This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 * Bitfield extraction functions.  In each, start is a bit index  - not
 * a byte index - and width is a bit width.  The width is bounded above by
 * 64 bits.
 *
 * The sbits() function assumes twos-complement arithmetic. ubits()
 * and sbits() assume no padding in integers.
 */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "bits.h"

uint64_t ubits(unsigned char buf[], unsigned int start, unsigned int width, bool le)
/* extract a (zero-origin) bitfield from the buffer as an unsigned big-endian uint64_t */
{
    uint64_t fld = 0;
    unsigned int i;
    unsigned end;

    /*@i1@*/ assert(width <= sizeof(uint64_t) * CHAR_BIT);
    for (i = start / CHAR_BIT;
	 i < (start + width + CHAR_BIT - 1) / CHAR_BIT; i++) {
	/*@i1@*/fld <<= CHAR_BIT;
	fld |= (unsigned char)buf[i];
    }

    end = (start + width) % CHAR_BIT;
    if (end != 0) {
	/*@i1@*/fld >>= (CHAR_BIT - end);
    }

    /*@ -shiftimplementation @*/
    fld &= ~(-1LL << width);
    /*@ +shiftimplementation @*/

    /* was extraction as a little-endian requested? */
    if (le)
    {
	uint64_t reversed = 0;

	for (i = width; i; --i)
	{
	    reversed <<= 1;
	    if (fld & 1)
		reversed |= 1;
	    fld >>= 1;
	}
	fld = reversed;
    }

    return fld;
}

int64_t sbits(signed char buf[], unsigned int start, unsigned int width, bool le)
/* extract a bitfield from the buffer as a signed big-endian long */
{
    uint64_t fld = ubits((unsigned char *)buf, start, width, le);

    /*@ +relaxtypes */
    if (fld & (1LL << (width - 1))) {
	/*@ -shiftimplementation @*/
	fld |= (-1LL << (width - 1));
	/*@ +shiftimplementation @*/
    }
    return (int64_t)fld;
    /*@ -relaxtypes */
}

union int_float {
    int32_t i;
    float f;
};

union long_double {
    int64_t l;
    double d;
};

float getlef32(const char *buf, int off)
{
    union int_float i_f;

    i_f.i = getles32(buf, off);
    return i_f.f;
}

double getled64(const char *buf, int off)
{
    union long_double l_d;

    l_d.l = getles64(buf, off);
    return l_d.d;
}

float getbef32(const char *buf, int off)
{
    union int_float i_f;

    i_f.i = getbes32(buf, off);
    return i_f.f;
}

double getbed64(const char *buf, int off)
{
    union long_double l_d;

    l_d.l = getbes64(buf, off);
    return l_d.d;
}

/*@-shiftimplementation@*/
void putbef32(char *buf, int off, float val)
{
    union int_float i_f;

    i_f.f = val;
    /* this would be a putbe32 call if not for a signedness issue */
    buf[off] = (char)(((i_f.i) >> 16) >> 8);
}
/*@+shiftimplementation@*/

#ifdef __UNUSED__
// cppcheck-suppress unusedFunction
u_int16_t swap_u16(u_int16_t i)
/* byte-swap a 16-bit unsigned int */
{
    u_int8_t c1, c2;

    c1 = i & 255;
    c2 = (i >> 8) & 255;

    return (c1 << 8) + c2;
}

// cppcheck-suppress unusedFunction
u_int32_t swap_u32(u_int32_t i)
/* byte-swap a 32-bit unsigned int */
{
    u_int8_t c1, c2, c3, c4;

    c1 = i & 255;
    c2 = (i >> 8) & 255;
    c3 = (i >> 16) & 255;
    c4 = (i >> 24) & 255;

    return ((u_int32_t)c1 << 24) + ((u_int32_t)c2 << 16) + ((u_int32_t)c3 << 8) + c4;
}

// cppcheck-suppress unusedFunction
u_int64_t swap_u64(u_int64_t i)
/* byte-swap a 64-bit unsigned int */
{
    u_int8_t c1, c2, c3, c4, c5, c6, c7, c8;

    c1 = i & 255;
    c2 = (i >> 8) & 255;
    c3 = (i >> 16) & 255;
    c4 = (i >> 24) & 255;
    c5 = (i >> 32) & 255;
    c6 = (i >> 40) & 255;
    c7 = (i >> 48) & 255;
    c8 = (i >> 56) & 255;

    return ((u_int64_t)c1 << 56) +
            ((u_int64_t)c2 << 48) +
            ((u_int64_t)c3 << 40) +
            ((u_int64_t)c4 << 32) +
            ((u_int64_t)c5 << 24) +
            ((u_int64_t)c6 << 16) +
            ((u_int64_t)c7 << 8) +
            c8;
}
#endif /* __UNUSED__ */
