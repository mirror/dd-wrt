/*
 * Misc useful os-independent macros and functions.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifndef	_bcmutils_h_
#define	_bcmutils_h_

/*** driver-only section ***/
#ifdef BCMDRIVER
#include <osl.h>

#define _BCM_U	0x01	/* upper */
#define _BCM_L	0x02	/* lower */
#define _BCM_D	0x04	/* digit */
#define _BCM_C	0x08	/* cntrl */
#define _BCM_P	0x10	/* punct */
#define _BCM_S	0x20	/* white space (space/lf/tab) */
#define _BCM_X	0x40	/* hex digit */
#define _BCM_SP	0x80	/* hard space (0x20) */

#define GPIO_PIN_NOTDEFINED 	0x20 

extern unsigned char bcm_ctype[];
#define bcm_ismask(x) (bcm_ctype[(int)(unsigned char)(x)])

#define bcm_isalnum(c)	((bcm_ismask(c)&(_BCM_U|_BCM_L|_BCM_D)) != 0)
#define bcm_isalpha(c)	((bcm_ismask(c)&(_BCM_U|_BCM_L)) != 0)
#define bcm_iscntrl(c)	((bcm_ismask(c)&(_BCM_C)) != 0)
#define bcm_isdigit(c)	((bcm_ismask(c)&(_BCM_D)) != 0)
#define bcm_isgraph(c)	((bcm_ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D)) != 0)
#define bcm_islower(c)	((bcm_ismask(c)&(_BCM_L)) != 0)
#define bcm_isprint(c)	((bcm_ismask(c)&(_BCM_P|_BCM_U|_BCM_L|_BCM_D|_BCM_SP)) != 0)
#define bcm_ispunct(c)	((bcm_ismask(c)&(_BCM_P)) != 0)
#define bcm_isspace(c)	((bcm_ismask(c)&(_BCM_S)) != 0)
#define bcm_isupper(c)	((bcm_ismask(c)&(_BCM_U)) != 0)
#define bcm_isxdigit(c)	((bcm_ismask(c)&(_BCM_D|_BCM_X)) != 0)

/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINWAIT(exp, us) { \
	uint countdown = (us) + 9; \
	while ((exp) && (countdown >= 10)) {\
		OSL_DELAY(10); \
		countdown -= 10; \
	} \
}

/* generic osl packet queue */
struct pktq {
	void *head;	/* first packet to dequeue */
	void *tail;	/* last packet to dequeue */
	uint len;	/* number of queued packets */
	uint maxlen;	/* maximum number of queued packets */
	bool priority;	/* enqueue by packet priority */
	uint8 prio_map[MAXPRIO+1]; /* user priority to packet enqueue policy map */
};
#define DEFAULT_QLEN	128

#define	pktq_len(q)	((q)->len)
#define	pktq_avail(q)	((q)->maxlen - (q)->len)
#define	pktq_head(q)	((q)->head)
#define	pktq_full(q)	((q)->len >= (q)->maxlen)
#define	_pktq_pri(q, pri)	((q)->prio_map[pri])
#define	pktq_tailpri(q)	((q)->tail ? _pktq_pri(q, PKTPRIO((q)->tail)) : _pktq_pri(q, 0))

/* externs */
/* packet */
extern uint pktcopy(osl_t *osh, void *p, uint offset, int len, uchar *buf);
extern uint pkttotlen(osl_t *osh, void *);
extern void pktq_init(struct pktq *q, uint maxlen, const uint8 prio_map[]);
extern void pktenq(struct pktq *q, void *p, bool lifo);
extern void *pktdeq(struct pktq *q);
extern void *pktdeqtail(struct pktq *q);
/* string */
extern uint bcm_atoi(char *s);
extern uchar bcm_toupper(uchar c);
extern ulong bcm_strtoul(char *cp, char **endp, uint base);
extern char *bcmstrstr(char *haystack, char *needle);
extern char *bcmstrcat(char *dest, const char *src);
extern ulong wchar2ascii(char *abuf, ushort *wbuf, ushort wbuflen, ulong abuflen);
/* ethernet address */
extern char *bcm_ether_ntoa(char *ea, char *buf);
extern int bcm_ether_atoe(char *p, char *ea);
/* delay */
extern void bcm_mdelay(uint ms);
/* variable access */
extern char *getvar(char *vars, char *name);
extern int getintvar(char *vars, char *name);
extern uint getgpiopin(char *vars, char *pin_name, uint def_pin);
#define	bcmlog(fmt, a1, a2)
#define	bcmdumplog(buf, size)	*buf = '\0'
#define	bcmdumplogent(buf, idx)	-1

#endif	/* #ifdef BCMDRIVER */

/*** driver/apps-shared section ***/

#define BCME_STRLEN 		64
#define VALID_BCMERROR(e)  ((e <= 0) && (e >= BCME_LAST))


/* 
 * error codes could be added but the defined ones shouldn't be changed/deleted 
 * these error codes are exposed to the user code 
 * when ever a new error code is added to this list 
 * please update errorstring table with the related error string and 
 * update osl files with os specific errorcode map   
*/

#define BCME_ERROR			-1	/* Error generic */
#define BCME_BADARG			-2	/* Bad Argument */
#define BCME_BADOPTION			-3	/* Bad option */
#define BCME_NOTUP			-4	/* Not up */
#define BCME_NOTDOWN			-5	/* Not down */
#define BCME_NOTAP			-6	/* Not AP */
#define BCME_NOTSTA			-7	/* Not STA  */
#define BCME_BADKEYIDX			-8	/* BAD Key Index */
#define BCME_RADIOOFF 			-9	/* Radio Off */
#define BCME_NOTBANDLOCKED		-10	/* Not  bandlocked */
#define BCME_NOCLK			-11	/* No Clock*/
#define BCME_BADRATESET			-12	/* BAD RateSet*/
#define BCME_BADBAND			-13	/* BAD Band */
#define BCME_BUFTOOSHORT		-14	/* Buffer too short */	
#define BCME_BUFTOOLONG			-15	/* Buffer too Long */	
#define BCME_BUSY			-16	/* Busy*/	
#define BCME_NOTASSOCIATED		-17	/* Not associated*/
#define BCME_BADSSIDLEN			-18	/* BAD SSID Len */
#define BCME_OUTOFRANGECHAN		-19	/* Out of Range Channel*/
#define BCME_BADCHAN			-20	/* BAD Channel */
#define BCME_BADADDR			-21	/* BAD Address*/
#define BCME_NORESOURCE			-22	/* No resources*/
#define BCME_UNSUPPORTED		-23	/* Unsupported*/
#define BCME_BADLEN			-24	/* Bad Length*/
#define BCME_NOTREADY			-25	/* Not ready Yet*/
#define BCME_EPERM			-26	/* Not Permitted */
#define BCME_NOMEM			-27	/* No Memory */
#define BCME_ASSOCIATED			-28	/* Associated */
#define BCME_RANGE			-29	/* Range Error*/
#define BCME_NOTFOUND			-30	/* Not found */
#define BCME_LAST			BCME_NOTFOUND	

#ifndef ABS
#define	ABS(a)			(((a)<0)?-(a):(a))
#endif

#ifndef MIN
#define	MIN(a, b)		(((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a, b)		(((a)>(b))?(a):(b))
#endif

#define CEIL(x, y)		(((x) + ((y)-1)) / (y))
#define	ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#define	ISALIGNED(a, x)		(((a) & ((x)-1)) == 0)
#define	ISPOWEROF2(x)		((((x)-1)&(x))==0)
#define VALID_MASK(mask)	!((mask) & ((mask) + 1))
#define	OFFSETOF(type, member)	((uint)(uintptr)&((type *)0)->member)
#define ARRAYSIZE(a)		(sizeof(a)/sizeof(a[0]))

/* bit map related macros */
#ifndef setbit
#define	NBBY	8	/* 8 bits per byte */
#define	setbit(a,i)	(((uint8 *)a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	(((uint8 *)a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	(((uint8 *)a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	((((uint8 *)a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif

#define	NBITS(type)	(sizeof(type) * 8)
#define NBITVAL(bits)	(1 << (bits))
#define MAXBITVAL(bits)	((1 << (bits)) - 1)

/* crc defines */
#define CRC8_INIT_VALUE  0xff		/* Initial CRC8 checksum value */
#define CRC8_GOOD_VALUE  0x9f		/* Good final CRC8 checksum value */
#define CRC16_INIT_VALUE 0xffff		/* Initial CRC16 checksum value */
#define CRC16_GOOD_VALUE 0xf0b8		/* Good final CRC16 checksum value */
#define CRC32_INIT_VALUE 0xffffffff	/* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3	/* Good final CRC32 checksum value */

/* bcm_format_flags() bit description structure */
typedef struct bcm_bit_desc {
	uint32	bit;
	char*	name;
} bcm_bit_desc_t;

/* tag_ID/length/value_buffer tuple */
typedef struct bcm_tlv {
	uint8	id;
	uint8	len;
	uint8	data[1];
} bcm_tlv_t;

/* Check that bcm_tlv_t fits into the given buflen */
#define bcm_valid_tlv(elt, buflen) ((buflen) >= 2 && (int)(buflen) >= (int)(2 + (elt)->len))

/* buffer length for ethernet address from bcm_ether_ntoa() */
#define ETHER_ADDR_STR_LEN	18

/* unaligned load and store macros */
#ifdef IL_BIGENDIAN
static INLINE uint32
load32_ua(uint8 *a)
{
	return ((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]);
}

static INLINE void
store32_ua(uint8 *a, uint32 v)
{
	a[0] = (v >> 24) & 0xff;
	a[1] = (v >> 16) & 0xff;
	a[2] = (v >> 8) & 0xff;
	a[3] = v & 0xff;
}

static INLINE uint16
load16_ua(uint8 *a)
{
	return ((a[0] << 8) | a[1]);
}

static INLINE void
store16_ua(uint8 *a, uint16 v)
{
	a[0] = (v >> 8) & 0xff;
	a[1] = v & 0xff;
}

#else

static INLINE uint32
load32_ua(uint8 *a)
{
	return ((a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0]);
}

static INLINE void
store32_ua(uint8 *a, uint32 v)
{
	a[3] = (v >> 24) & 0xff;
	a[2] = (v >> 16) & 0xff;
	a[1] = (v >> 8) & 0xff;
	a[0] = v & 0xff;
}

static INLINE uint16
load16_ua(uint8 *a)
{
	return ((a[1] << 8) | a[0]);
}

static INLINE void
store16_ua(uint8 *a, uint16 v)
{
	a[1] = (v >> 8) & 0xff;
	a[0] = v & 0xff;
}

#endif

/* externs */
/* crc */
extern uint8 hndcrc8(uint8 *p, uint nbytes, uint8 crc);
extern uint16 hndcrc16(uint8 *p, uint nbytes, uint16 crc);
extern uint32 hndcrc32(uint8 *p, uint nbytes, uint32 crc);
/* format/print */
/* IE parsing */
extern bcm_tlv_t *bcm_next_tlv(bcm_tlv_t *elt, int *buflen);
extern bcm_tlv_t *bcm_parse_tlvs(void *buf, int buflen, uint key);
extern bcm_tlv_t *bcm_parse_ordered_tlvs(void *buf, int buflen, uint key);

/* bcmerror*/
extern const char *bcmerrorstr(int bcmerror);

/* multi-bool data type: set of bools, mbool is true if any is set */
typedef uint32 mbool;
#define mboolset(mb, bit)		(mb |= bit)		/* set one bool */
#define mboolclr(mb, bit)		(mb &= ~bit)		/* clear one bool */
#define mboolisset(mb, bit)		((mb & bit) != 0)	/* TRUE if one bool is set */
#define	mboolmaskset(mb, mask, val)	((mb) = (((mb) & ~(mask)) | (val)))

/* power conversion */
extern uint16 bcm_qdbm_to_mw(uint8 qdbm);
extern uint8 bcm_mw_to_qdbm(uint16 mw);

/* generic datastruct to help dump routines */
struct fielddesc {
	char 	*nameandfmt;
	uint32 	offset;
	uint32 	len;
};

typedef  uint32 (*readreg_rtn)(void *arg0, void *arg1, uint32 offset);
extern uint bcmdumpfields(readreg_rtn func_ptr, void *arg0, void *arg1, struct fielddesc *str, char *buf, uint32 bufsize);

extern uint bcm_mkiovar(char *name, char *data, uint datalen, char *buf, uint len);

#endif	/* _bcmutils_h_ */
