/* $Id: bits_test.c 3486 2006-09-21 00:58:22Z ckuethe $ */
/* test harness for bits.h */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bits.h"

/*@ -duplicatequals -formattype */
typedef unsigned long long ubig;

static unsigned char buf[80];
static union int_float i_f;
static union long_double l_d;
static char sb1,sb2;
static unsigned char ub1,ub2;
static short sw1,sw2;
static unsigned short uw1,uw2;
static int sl1,sl2;
static unsigned int ul1,ul2;
static long long sL1,sL2;
static unsigned long long uL1,uL2;
static float f1;
static double d1;

static void dumpall(void)
{
    (void)printf("getsb: %016llx %016llx %016llx %016llx\n",
		(ubig)sb1, (ubig)sb2,
		(ubig)getsb(buf, 0), (ubig)getsb(buf, 8));
    (void)printf("getub: %016llx %016llx %016llx %016llx\n",
		(ubig)ub1, (ubig)ub2,
		(ubig)getub(buf, 0), (ubig)getub(buf, 8));
    (void)printf("getsw: %016llx %016llx %016llx %016llx\n",
		(ubig)sw1, (ubig)sw2,
		(ubig)getsw(buf, 0), (ubig)getsw(buf, 8));
    (void)printf("getuw: %016llx %016llx %016llx %016llx\n",
		(ubig)uw1, (ubig)uw2,
		(ubig)getuw(buf, 0), (ubig)getuw(buf, 8));
    (void)printf("getsl: %016llx %016llx %016llx %016llx\n",
		(ubig)sl1, (ubig)sl2,
		(ubig)getsl(buf, 0), (ubig)getsl(buf, 8));
    (void)printf("getul: %016llx %016llx %016llx %016llx\n",
		(ubig)ul1, (ubig)ul2,
		(ubig)getul(buf, 0), (ubig)getul(buf, 8));
    (void)printf("getsL: %016llx %016llx %016llx %016llx\n",
		(ubig)sL1, (ubig)sL2,
		(ubig)getsL(buf, 0), (ubig)getsL(buf, 8));
    (void)printf("getuL: %016llx %016llx %016llx %016llx\n",
		(ubig)uL1, (ubig)uL2,
		(ubig)getuL(buf, 0), (ubig)getuL(buf, 8));
    (void)printf("getf: %f %f\n", f1, getf(buf, 24));
    (void)printf("getd: %.16f %.16f\n", d1, getd(buf, 16));
}

/*@ -duplicatequals +ignorequals @*/
int main(void)
{
    memcpy(buf,"\x01\x02\x03\x04\x05\x06\x07\x08",8);
    memcpy(buf+8,"\xff\xfe\xfd\xfc\xfb\xfa\xf9\xf8",8);
    memcpy(buf+16,"\x40\x09\x21\xfb\x54\x44\x2d\x18",8);
    memcpy(buf+24,"\x40\x49\x0f\xdb",4);

    /* big-endian test */
    printf("Big-endian\n");
#include "bits.h"
    sb1 = getsb(buf, 0);
    sb2 = getsb(buf, 8);
    ub1 = getub(buf, 0);
    ub2 = getub(buf, 8);
    sw1 = getsw(buf, 0);
    sw2 = getsw(buf, 8);
    uw1 = getuw(buf, 0);
    uw2 = getuw(buf, 8);
    sl1 = getsl(buf, 0);
    sl2 = getsl(buf, 8);
    ul1 = getul(buf, 0);
    ul2 = getul(buf, 8);
    sL1 = getsL(buf, 0);
    sL2 = getsL(buf, 8);
    uL1 = getuL(buf, 0);
    uL2 = getuL(buf, 8);
    f1 = getf(buf, 24);
    d1 = getd(buf, 16);

    dumpall();

#undef getub
#undef getsb
#undef getuw
#undef getsw
#undef getul
#undef getsl
#undef getuL
#undef getsL
#undef putword
#undef putlong

    /* little-endian test */
#define LITTLE_ENDIAN_PROTOCOL
    printf("Little-endian\n");
#include "bits.h"
    sb1 = getsb(buf, 0);
    sb2 = getsb(buf, 8);
    ub1 = getub(buf, 0);
    ub2 = getub(buf, 8);
    sw1 = getsw(buf, 0);
    sw2 = getsw(buf, 8);
    uw1 = getuw(buf, 0);
    uw2 = getuw(buf, 8);
    sl1 = getsl(buf, 0);
    sl2 = getsl(buf, 8);
    ul1 = getul(buf, 0);
    ul2 = getul(buf, 8);
    sL1 = getsL(buf, 0);
    sL2 = getsL(buf, 8);
    uL1 = getuL(buf, 0);
    uL2 = getuL(buf, 8);
    f1 = getf(buf, 24);
    d1 = getd(buf, 16);
    dumpall();

    exit(0);
}
