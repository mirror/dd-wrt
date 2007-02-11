/** file: test.c - test of 64-bit integer stuff
*
*
* 21-jan-1998: David Perkins <dperkins@dsperkins.com>
*
*/

#include <net-snmp/net-snmp-config.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/library/int64.h>
#include <net-snmp/library/snmp_assert.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/snmp_logging.h>

#define TRUE 1
#define FALSE 0

/** divBy10 - divide an unsigned 64-bit integer by 10
*
* call with:
*   u64 - number to be divided
*   pu64Q - location to store quotient
*   puR - location to store remainder
*
*/
void
divBy10(U64 u64, U64 * pu64Q, unsigned int *puR)
{
    unsigned long   ulT;
    unsigned long   ulQ;
    unsigned long   ulR;


    /*
     * top 16 bits 
     */
    ulT = (u64.high >> 16) & 0x0ffff;
    ulQ = ulT / 10;
    ulR = ulT % 10;
    pu64Q->high = ulQ << 16;

    /*
     * next 16 
     */
    ulT = (u64.high & 0x0ffff);
    ulT += (ulR << 16);
    ulQ = ulT / 10;
    ulR = ulT % 10;
    pu64Q->high = pu64Q->high | ulQ;

    /*
     * next 16 
     */
    ulT = ((u64.low >> 16) & 0x0ffff) + (ulR << 16);
    ulQ = ulT / 10;
    ulR = ulT % 10;
    pu64Q->low = ulQ << 16;

    /*
     * final 16 
     */
    ulT = (u64.low & 0x0ffff);
    ulT += (ulR << 16);
    ulQ = ulT / 10;
    ulR = ulT % 10;
    pu64Q->low = pu64Q->low | ulQ;

    *puR = (unsigned int) (ulR);


}                               /* divBy10 */


/** multBy10 - multiply an unsigned 64-bit integer by 10
*
* call with:
*   u64 - number to be multiplied
*   pu64P - location to store product
*
*/
void
multBy10(U64 u64, U64 * pu64P)
{
    unsigned long   ulT;
    unsigned long   ulP;
    unsigned long   ulK;


    /*
     * lower 16 bits 
     */
    ulT = u64.low & 0x0ffff;
    ulP = ulT * 10;
    ulK = ulP >> 16;
    pu64P->low = ulP & 0x0ffff;

    /*
     * next 16 
     */
    ulT = (u64.low >> 16) & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP >> 16;
    pu64P->low = (ulP & 0x0ffff) << 16 | pu64P->low;

    /*
     * next 16 bits 
     */
    ulT = u64.high & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP >> 16;
    pu64P->high = ulP & 0x0ffff;

    /*
     * final 16 
     */
    ulT = (u64.high >> 16) & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP >> 16;
    pu64P->high = (ulP & 0x0ffff) << 16 | pu64P->high;


}                               /* multBy10 */


/** incrByU16 - add an unsigned 16-bit int to an unsigned 64-bit integer
*
* call with:
*   pu64 - number to be incremented
*   u16 - amount to add
*
*/
void
incrByU16(U64 * pu64, unsigned int u16)
{
    unsigned long   ulT1;
    unsigned long   ulT2;
    unsigned long   ulR;
    unsigned long   ulK;


    /*
     * lower 16 bits 
     */
    ulT1 = pu64->low;
    ulT2 = ulT1 & 0x0ffff;
    ulR = ulT2 + u16;
    ulK = ulR >> 16;
    if (ulK == 0) {
        pu64->low = ulT1 + u16;
        return;
    }

    /*
     * next 16 bits 
     */
    ulT2 = (ulT1 >> 16) & 0x0ffff;
    ulR = ulT2 + 1;
    ulK = ulR >> 16;
    if (ulK == 0) {
        pu64->low = ulT1 + u16;
        return;
    }

    /*
     * next 32 - ignore any overflow 
     */
    pu64->low = (ulT1 + u16) & 0x0FFFFFFFFL;
    pu64->high++;
#if SIZEOF_LONG != 4
    pu64->high &= 0xffffffff;
#endif
}                               /* incrByV16 */

void
incrByU32(U64 * pu64, unsigned int u32)
{
    unsigned int    tmp;
    tmp = pu64->low;
    pu64->low += u32;
#if SIZEOF_LONG != 4
    pu64->low &= 0xffffffff;
#endif
    if (pu64->low < tmp) {
        pu64->high++;
#if SIZEOF_LONG != 4
        pu64->high &= 0xffffffff;
#endif
    }
}

/**
 * pu64out = pu64one - pu64two 
 */
void
u64Subtract(const U64 * pu64one, const U64 * pu64two, U64 * pu64out)
{
    if (pu64one->low < pu64two->low) {
        pu64out->low = 0xffffffff - pu64two->low + pu64one->low + 1;
        pu64out->high = pu64one->high - pu64two->high - 1;
    } else {
        pu64out->low = pu64one->low - pu64two->low;
        pu64out->high = pu64one->high - pu64two->high;
    }
}

/**
 * pu64out += pu64one
 */
void
u64Incr(U64 * pu64out, const U64 * pu64one)
{
    pu64out->high += pu64one->high;
#if SIZEOF_LONG != 4
    pu64out->high &= 0xffffffff;
#endif
    incrByU32(pu64out, pu64one->low);
}

/**
 * pu64out += (pu64one - pu64two)
 */
void
u64UpdateCounter(U64 * pu64out, const U64 * pu64one, const U64 * pu64two)
{
    U64 tmp;
    u64Subtract(pu64one, pu64two, &tmp);
    u64Incr(pu64out, &tmp);
}

/**
 * pu64one = pu64two 
 */
void
u64Copy(U64 * pu64one, const U64 * pu64two)
{
    pu64one->high = pu64two->high;
    pu64one->low =  pu64two->low;
}

/** zeroU64 - set an unsigned 64-bit number to zero
*
* call with:
*   pu64 - number to be zero'ed
*
*/
void
zeroU64(U64 * pu64)
{
    pu64->low = 0;
    pu64->high = 0;
}                               /* zeroU64 */


/** isZeroU64 - check if an unsigned 64-bit number is
*
* call with:
*   pu64 - number to be zero'ed
*
*/
int
isZeroU64(const U64 * pu64)
{

    if ((pu64->low == 0) && (pu64->high == 0))
        return (TRUE);
    else
        return (FALSE);

}                               /* isZeroU64 */

/**
 * check the old and new values of a counter64 for 32bit wrapping
 *
 * @param adjust : set to 1 to auto-increment new_val->high
 *                 if a 32bit wrap is detected.
 *
 * @param old_val
 * @param new_val
 *
 *@Note:
 * The old and new values must be be from within a time period
 * which would only allow the 32bit portion of the counter to
 * wrap once. i.e. if the 32bit portion of the counter could
 * wrap every 60 seconds, the old and new values should be compared
 * at least every 59 seconds (though I'd recommend at least every
 * 50 seconds to allow for timer inaccuracies).
 *
 * @retval 64 : 64bit wrap
 * @retval 32 : 32bit wrap
 * @retval  0 : did not wrap
 * @retval -1 : bad parameter
 * @retval -2 : unexpected high value (changed by more than 1)
 */
int
netsnmp_c64_check_for_32bit_wrap(struct counter64 *old_val,
                                 struct counter64 *new_val,
                                 int adjust)
{
    if( (NULL == old_val) || (NULL == new_val) )
        return -1;

    DEBUGMSGTL(("9:c64:check_wrap", "check wrap 0x%0x.0x%0x 0x%0x.0x%0x\n",
                old_val->high, old_val->low, new_val->high, new_val->low));
    
    /*
     * check for wraps
     */
    if ((new_val->low >= old_val->low) &&
        (new_val->high == old_val->high)) {
        DEBUGMSGTL(("9:c64:check_wrap", "no wrap\n"));
        return 0;
    }

    /*
     * low wrapped. did high change?
     */
    if (new_val->high == old_val->high) {
        DEBUGMSGTL(("c64:check_wrap", "32 bit wrap\n"));
        if (adjust) {
            ++new_val->high;
#if SIZEOF_LONG != 4
            new_val->high &= 0xffffffff;
#endif
        }
        return 32;
    }
    else if ((new_val->high == (old_val->high + 1)) ||
             ((0 == new_val->high) && (0xffffffff == old_val->high))) {
        DEBUGMSGTL(("c64:check_wrap", "64 bit wrap\n"));
        return 64;
    }

    return -2;
}

/**
 * update a 64 bit value with the difference between two (possibly) 32 bit vals
 *
 * @param prev_val       : the 64 bit current counter
 * @param old_prev_val   : the (possibly 32 bit) previous value
 * @param new_val        : the (possible 32bit) new value
 * @param need_wrap_check: pointer to integer indicating if wrap check is needed
 *                         flag may be cleared if 64 bit counter is detected
 *
 *@Note:
 * The old_prev_val and new_val values must be be from within a time
 * period which would only allow the 32bit portion of the counter to
 * wrap once. i.e. if the 32bit portion of the counter could
 * wrap every 60 seconds, the old and new values should be compared
 * at least every 59 seconds (though I'd recommend at least every
 * 50 seconds to allow for timer inaccuracies).
 *
 * Suggested use:
 *
 *   static needwrapcheck = 1;
 *   static counter64 current, prev_val, new_val;
 *
 *   your_functions_to_update_new_value(&new_val);
 *   if (0 == needwrapcheck)
 *      memcpy(current, new_val, sizeof(new_val));
 *   else {
 *      netsnmp_c64_check32_and_update(&current,&new,&prev,&needwrapcheck);
 *      memcpy(prev_val, new_val, sizeof(new_val));
 *   }
 *
 *
 * @retval  0 : success
 * @retval -1 : error checking for 32 bit wrap
 * @retval -2 : look like we have 64 bit values, but sums aren't consistent
 */
int
netsnmp_c64_check32_and_update(struct counter64 *prev_val, struct counter64 *new_val,
                               struct counter64 *old_prev_val, int *need_wrap_check)
{
    int rc;

    /*
     * counters are 32bit or unknown (which we'll treat as 32bit).
     * update the prev values with the difference between the
     * new stats and the prev old_stats:
     *    prev->stats += (new->stats - prev->old_stats)
     */
    if ((NULL == need_wrap_check) || (0 != *need_wrap_check)) {
        rc = netsnmp_c64_check_for_32bit_wrap(old_prev_val,new_val, 1);
        if (rc < 0) {
            snmp_log(LOG_ERR,"c64 32 bit check failed\n");
            return -1;
        }
    }
    else
        rc = 0;
    
    /*
     * update previous values
     */
    (void) u64UpdateCounter(prev_val, new_val, old_prev_val);

    /*
     * if wrap check was 32 bit, undo adjust, now that prev is updated
     */
    if (32 == rc) {
        /*
         * check wrap incremented high, so reset it. (Because having
         * high set for a 32 bit counter will confuse us in the next update).
         */
        netsnmp_assert(1 == new_val->high);
        new_val->high = 0;
    }
    else if (64 == rc) {
        /*
         * if we really have 64 bit counters, the summing we've been
         * doing for prev values should be equal to the new values.
         */
        if ((prev_val->low != new_val->low) ||
            (prev_val->high != new_val->high)) {
            snmp_log(LOG_ERR, "looks like a 64bit wrap, but prev!=new\n");
            return -2;
        }
        else if (NULL != need_wrap_check)
            *need_wrap_check = 0;
    }
    
    return 0;
}

void
printU64(char *buf,     /* char [I64CHARSZ+1]; */
                         const U64 * pu64) {
    U64             u64a;
    U64             u64b;

    char            aRes[I64CHARSZ + 1];
    unsigned int    u;
    int             j;

    u64a.high = pu64->high;
    u64a.low = pu64->low;
    aRes[I64CHARSZ] = 0;
    for (j = 0; j < I64CHARSZ; j++) {
        divBy10(u64a, &u64b, &u);
        aRes[(I64CHARSZ - 1) - j] = (char) ('0' + u);
        u64a.high = u64b.high;
        u64a.low = u64b.low;
        if (isZeroU64(&u64a))
            break;
    }
    strcpy(buf, &aRes[(I64CHARSZ - 1) - j]);
}

void
printI64(char *buf,     /* char [I64CHARSZ+1]; */
                         const U64 * pu64) {
    U64             u64a;
    U64             u64b;

    char            aRes[I64CHARSZ + 1];
    unsigned int    u;
    int             j, sign = 0;

    if (pu64->high & 0x80000000) {
        u64a.high = ~pu64->high;
        u64a.low = ~pu64->low;
        sign = 1;
        incrByU32(&u64a, 1);    /* bit invert and incr by 1 to print 2s complement */
    } else {
        u64a.high = pu64->high;
        u64a.low = pu64->low;
    }

    aRes[I64CHARSZ] = 0;
    for (j = 0; j < I64CHARSZ; j++) {
        divBy10(u64a, &u64b, &u);
        aRes[(I64CHARSZ - 1) - j] = (char) ('0' + u);
        u64a.high = u64b.high;
        u64a.low = u64b.low;
        if (isZeroU64(&u64a))
            break;
    }
    if (sign == 1) {
        aRes[(I64CHARSZ - 1) - j - 1] = '-';
        strcpy(buf, &aRes[(I64CHARSZ - 1) - j - 1]);
        return;
    }
    strcpy(buf, &aRes[(I64CHARSZ - 1) - j]);
}

int
read64(U64 * i64, const char *str)
{
    U64             i64p;
    unsigned int    u;
    int             sign = 0;
    int             ok = 0;

    zeroU64(i64);
    if (*str == '-') {
        sign = 1;
        str++;
    }

    while (*str && isdigit(*str)) {
        ok = 1;
        u = *str - '0';
        multBy10(*i64, &i64p);
        memcpy(i64, &i64p, sizeof(i64p));
        incrByU16(i64, u);
        str++;
    }
    if (sign) {
        i64->high = ~i64->high;
        i64->low = ~i64->low;
        incrByU16(i64, 1);
    }
    return ok;
}




#ifdef TESTING
void
main(int argc, char *argv[])
{
    int             i;
    int             j;
    int             l;
    unsigned int    u;
    U64             u64a;
    U64             u64b;
#define MXSZ 20
    char            aRes[MXSZ + 1];


    if (argc < 2) {
        printf("This program takes numbers from the command line\n"
               "and prints them out.\n" "Usage: test <unsignedInt>...\n");
        exit(1);
    }

    aRes[MXSZ] = 0;

    for (i = 1; i < argc; i++) {
        l = strlen(argv[i]);
        zeroU64(&u64a);
        for (j = 0; j < l; j++) {
            if (!isdigit(argv[i][j])) {
                printf("Argument is not a number \"%s\"\n", argv[i]);
                exit(1);
            }
            u = argv[i][j] - '0';
            multBy10(u64a, &u64b);
            u64a = u64b;
            incrByU16(&u64a, u);
        }

        printf("number \"%s\" in hex is '%08x%08x'h\n",
               argv[i], u64a.high, u64a.low);

        printf("number is \"%s\"\n", printU64(&u64a));
        for (j = 0; j < MXSZ; j++) {
            divBy10(u64a, &u64b, &u);
            aRes[(MXSZ - 1) - j] = (char) ('0' + u);
            u64a = u64b;
            if (isZeroU64(&u64a))
                break;
        }

        printf("number is \"%s\"\n", &aRes[(MXSZ - 1) - j]);
    }
    exit(0);
}                               /* main */
#endif                          /* TESTING */

/*
 * file: test.c 
 */
