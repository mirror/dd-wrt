#ifndef ECC_TEST_CODE
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/nand_ecc_mlc.h>
#include <asm/byteorder.h>
#endif

/*
 * (C) 2009 MikroTik
 * This is an implementation of a BCH(4174,4096,13) code.
 * 
 * Bits in the following coder are reversed, that is,
 *   bit 0x01 of byte 0 is the most significant data bit,
 *   bit 0x80 of byte 517 is the least significant data bit.
 * prime is the primitive polynomial with bits reversed (and without the
 *   highest order term, i.e. without x^13):
 *   bit 0x1 is the 13th bit (most significant, x^12 term)
 *   bit 0x1000 is the 1st bit (least significant, x^0 term)
 * (gen3 << 64) | (gen2 << 32) | gen1 is generating polynomial,
 *   with bits reversed, and multiplied by 2 (that is, the
 *   original polynomial is divided by 2, then reversed), so that
 *   it can be xor'ed before shifting the intermediate reminder sum, not after.
 * inside encode() function (sum3 << 64) | (sum2 << 32) | sum1 is the computed
 *   reminder (also with bits reversed) of dividing reversed data by the
 *   reversed g. first (most significant) two bits are always 0, so only
 *   518 full bytes are divided.
 * encode(), syndrome() and chien() are optimized for speed on a MIPS system.
 */

#define PRIME 0x1B00
#define gen1 0x86499E7E
#define gen2 0xCE9D8793
#define gen3 0x00005F46
#define POLY_SIZE 13
/* this is the length of error location polynomial and root vector.
 * it is longer than necessary, but since it does not make coding use
 * noticably more time or memory, it's better to be safe here.
 */
#define LPOLY_SIZE POLY_SIZE + 2
#define CODE_BITS (522 * 8 - 2)
#define CODE_BYTES 522
#define DATA_BITS (512 * 8)
#define DATA_BYTES 512
#define CHECK_BYTES 10
#define UNIT_SHIFT 13
#define MODULO ((1 << UNIT_SHIFT) - 1)
#define bch_assert(x) do {;} while (0)

/* Type tpnum_t is used for tabular values from the "mod PRIME(x)" group. */
typedef short tpnum_t;
/* Type pnum_t is used for temporary values from the "mod PRIME(x)" group.
 * smallest number that can hold 13 bit unsigned values is short.
 * On some architectures, int is faster than short.
 */
typedef int pnum_t;

/* pow[x] == a**x; pow[MODULO] = 0 (exponentiation table) */
static tpnum_t pow[MODULO + 1];
/* log[x] == i, where a**i == x; log[0] == MODULO (logarithm table) */
static tpnum_t log[MODULO + 1];

/* Compute reminder from dividing data bytes by generator polynomial and
 * place it into dst. result is 10 bytes, 78 significant bits.
 * unrolled for best perfomance on MIPS
 * assumes d is aligned to two bytes
 */
static void encode(const unsigned char *d, unsigned char *dst) {
    const unsigned *i;
    unsigned sum1 = 0, sum2 = 0, sum3 = 0;

    for (i = (void *) d; i != (void *) (d + DATA_BYTES); i += 1) {
	sum1 ^= le32_to_cpu(*i);
#define SHR(h, l, b) (b) ? (((h) << b) | ((l) >> (32 - b))) : (h)
#define STEP(b) \
	if (sum1 & (1 << (b))) { \
	    sum1 ^= SHR(gen1, 0, b); \
	    sum2 ^= SHR(gen2, gen1, b); \
	    sum3 ^= SHR(gen3, gen2, b); \
	}
#define SHIFT(N) \
	sum1 >>= N; \
	sum1 |= sum2 << (32 - N); \
	sum2 >>= N; \
	sum2 |= sum3 << (32 - N); \
	sum3 >>= N
#define CHUNK \
	STEP(0); STEP(1); STEP(2); STEP(3); \
	STEP(4); STEP(5); STEP(6); STEP(7); \
	STEP(8); STEP(9); STEP(10); STEP(11); \
	STEP(12); STEP(13); STEP(14); STEP(15); \
	SHIFT(16)
	CHUNK; CHUNK;
#undef STEP
#undef SHR
    }
#define SAVE(n, i) *(dst++) = (sum##n >> i * 8) & 0xff
    SAVE(1, 0); SAVE(1, 1); SAVE(1, 2); SAVE(1, 3);
    SAVE(2, 0); SAVE(2, 1); SAVE(2, 2); SAVE(2, 3);
    SAVE(3, 0); SAVE(3, 1);
#undef SAVE
    bch_assert(!(sum3 & 0xC000));
}

static void precalc(void) {
    int i;
    pnum_t a = 1 << (UNIT_SHIFT - 1);

    memset(pow, 0, sizeof(pow));
    memset(log, 0, sizeof(log));
    for (i = 0; i != MODULO; ++i) {
	bch_assert(!log[a]);
	bch_assert(a <= MODULO);
        pow[i] = a;
        log[a] = i;
        if (a & 1) a ^= PRIME << 1;
        a >>= 1;
    }
    bch_assert(a == (1 << (UNIT_SHIFT - 1)));
    log[0] = MODULO;
}

static void syndrome(unsigned char *d1, unsigned char *d2, tpnum_t *s) {
    int i;

    s[0] = MODULO;
    for (i = 1; i != POLY_SIZE; i += 2) {
        pnum_t syn0 = 0;
        pnum_t syn1 = 0;
        pnum_t si0 = (i * CODE_BITS) % MODULO;
        pnum_t si1 = ((i + 1) * CODE_BITS) % MODULO;
        unsigned char *i2;

        for (i2 = d1; i2 != d2 + CHECK_BYTES - 1; ++i2) {
	    unsigned char b;

	    if (i2 == d1 + DATA_BYTES) i2 = d2;
            b = *i2;
#define STEP(n) \
            if (si0 < i) si0 += MODULO; \
            si0 -= i; \
            if (si1 <= i) si1 += MODULO; \
            si1 -= i + 1; \
            if (b & (1 << (n))) { \
                syn0 ^= pow[si0]; \
                syn1 ^= pow[si1]; \
            }

            STEP(0); STEP(1); STEP(2); STEP(3);
            STEP(4); STEP(5); STEP(6); STEP(7);
        }
        { /* last byte only has 6 significant bits */
            unsigned char b = *i2;

            STEP(0); STEP(1); STEP(2); STEP(3);
            STEP(4); STEP(5);
        }
#undef STEP
	bch_assert(!si0);
	bch_assert(!si1);
        s[i] = log[syn0];
        s[i + 1] = log[syn1];
    }
}

/* if S_j / S_j+1 == a^-k for all 0<=j<2t-1, then
 * there is single error at the positon k.
 * Clark & Cain book, pg 209..210
 */
static int fix_single(unsigned char *d, tpnum_t *s) {
    pnum_t dif = s[1];
    int i;

    if (dif == MODULO) return 0;
    /* do not need to check for division by zero inside loop because
     * MODULE 2^13-1 is prime (i.e. has no factor <= POLY_SIZE)
     */
    for (i = 2; i != POLY_SIZE; ++i) {
        pnum_t t = s[i] - s[i - 1];

        if (t < 0) t += MODULO;
        if (t != dif) return 0;
    }
    if (dif >= CODE_BITS) return 0;
    dif = CODE_BITS - 1 - dif;
    if (dif < DATA_BITS) d[dif / 8] ^= 1 << (dif % 8);
    return 1;
}

static inline pnum_t logmul(pnum_t x1, pnum_t x2) {
    bch_assert(x1 <= MODULO);
    bch_assert(x2 <= MODULO);
    if (x1 == MODULO || x2 == MODULO) return MODULO; /* multiplication by 0 */
    x1 += x2;
    if (x1 >= MODULO) x1 -= MODULO;
    bch_assert(x1 < MODULO);
    return x1;
}

/* Berlekamp Massey
 * This Implementation follows algorithm as presented in
 * "Error-correction Coding for Digital Communications"
 * by George Cyril Clark, J. Bibb Cain, 1981 Plenum Press, New York
 * page 206.
 * 
 * NOTE: this function takes so little time compared to chien() or
 * syndrome(), that it makes no sense to optimize it for speed.
 */
static int bm(tpnum_t *s, tpnum_t *l) {
    int n, i;
    int k = -1;
    int len = 0;
    pnum_t d;
    tpnum_t dif[LPOLY_SIZE];

    for (i = 0; i != LPOLY_SIZE; ++i) l[i] = dif[i] = MODULO;
    l[0] = dif[1] = 0;

    for (n = 0; n < POLY_SIZE - 1; n += 1) {
	d = 0;
	for (i = 0; i <= len; ++i) d ^= pow[logmul(s[n - i + 1], l[i])];
	if (n & 1) bch_assert(!d);
	if (!d) {
	    for (i = LPOLY_SIZE - 1; i != 0; --i) dif[i] = dif[i - 1];
	    dif[0] = MODULO;
	    continue;
	}
	d = log[d];
	if (len < n - k) {
	    int len1 = n - k;
	    k = n - len;
	    len = len1;

	    for (i = LPOLY_SIZE - 1; i >= 0; --i) {
		pnum_t t = (i > 0) ? t = l[i - 1] : MODULO;
		if (d) t = logmul(t, MODULO - d);
		l[i] = log[pow[l[i]] ^ pow[logmul(d, dif[i])]];
		dif[i] = t;
	    }
	}
	else {
	    for (i = LPOLY_SIZE - 1; i >= 0; --i) {
		l[i] = log[pow[l[i]] ^ pow[logmul(d, dif[i])]];
		dif[i] = (i > 0) ? dif[i - 1] : MODULO;
	    }
	}
    }
    return len;
}

static int chien(tpnum_t *l, tpnum_t *roots) {
    int i, ret, max, num;
    tpnum_t poly[LPOLY_SIZE];
    tpnum_t step[LPOLY_SIZE];
    pnum_t i2 = 0;

    ret = 0;
    max = 0;
    num = 0;
    for (i = 0; i != LPOLY_SIZE; ++i) {
        poly[i] = MODULO;
        step[i] = MODULO;
        if (l[i] != MODULO) {
            if (i) {
                poly[num] = l[i];
                step[num] = i;
                ++num;
            }
            max = i + 1;
        }
    }
    if (max < 1) return 0;
    for (i2 = 0; i2 != MODULO; ++i2) {
        pnum_t val = pow[l[0]];
        pnum_t x;

#define STEP(n) \
        x = poly[n]; \
        val ^= pow[x]; \
        x += step[n]; \
        if (x >= MODULO) x -= MODULO; \
        poly[n] = x; \

        STEP(0); STEP(1); STEP(2);

        for (i = 3; i < num; ++i) {
            x = poly[i];
            val ^= pow[x];
            x += step[i];
            if (x >= MODULO) x -= MODULO;
            poly[i] = x;
        }
        if (!val) {
            roots[ret++] = i2;
            /* break out when all roots are found */
            if (ret == max - 1) break;
        }
    }
    return ret;
}


int nand_calculate_ecc_mlc(const unsigned char *dat, unsigned char *ecc_code)
{
        encode(dat, ecc_code);
	return 0;
}

static int ecc_fix_mult(unsigned char *dat, tpnum_t s[POLY_SIZE]) {
    tpnum_t roots[POLY_SIZE];
    int rnum;
    int i;
    {
        tpnum_t l[LPOLY_SIZE];
        int len = bm(s, l);

        rnum = chien(l, roots);
        if (rnum != len) {
		printk("MLC failed to fix %d vs %d\n", rnum, len);
		return -1;
	}
    }
    for (i = 0; i != rnum; ++i) {
        int pos = roots[i] - (MODULO - CODE_BITS) - 1;

        if (pos < 0) pos += MODULO;
	bch_assert(pos < CODE_BITS);
        if (pos >= CODE_BITS) {
		printk("MLC failed to fix %d/%d pos %d\n", i, rnum, pos);
		return -1;
	}
	if (pos < DATA_BITS) dat[pos / 8] ^= 1 << (pos % 8);
    }
//    printk("MLC fixed %d\n", rnum);
    return rnum;
}

static int ecc_fix(unsigned char *dat, unsigned char *read_ecc) {
    tpnum_t s[POLY_SIZE];
    syndrome(dat, read_ecc, s);
    if (fix_single(dat, s)) {
//	    printk("MLC single fix\n");
	    return 1;
    }
    return ecc_fix_mult(dat, s);
}

int nand_correct_data_mlc(unsigned char *dat, unsigned char *read_ecc)
{
    unsigned char calc_ecc[CHECK_BYTES];
    encode(dat, calc_ecc);
    read_ecc[9] &= 0x3F;
    if (!memcmp(read_ecc, calc_ecc, CHECK_BYTES)) return 0;
    if (!*pow) precalc();
    return ecc_fix(dat, read_ecc);
}
