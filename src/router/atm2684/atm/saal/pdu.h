/* pdu.h - SSCOP (Q.2110) PDU reader */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */


#ifndef PDU_H
#define PDU_H

#include <stdint.h>
#include <atmd.h>


/* SSCOP PDU types, Q.2110 section 7.1 */

#define SSCOP_BGN	 1 /* Request Initialization */
#define SSCOP_BGAK	 2 /* Request Acknowledgement */
#define SSCOP_BGREJ	 7 /* Connection Reject */
#define SSCOP_END	 3 /* Disconnect Command */
#define SSCOP_ENDAK	 4 /* Disconnect Acknowledgement */
#define SSCOP_RS	 5 /* Resynchronization Command */
#define SSCOP_RSAK	 6 /* Resynchronization Acknowledgement */
#define SSCOP_ER	 9 /* Recovery Command */
#define SSCOP_ERAK	15 /* Recovery Acknowledgement */
#define SSCOP_SD	 8 /* Sequence Connection-mode Data */
#define SSCOP_POLL	10 /* Transmitter State Information with request ... */
#define SSCOP_STAT	11 /* Solicited Receiver State Information */
#define SSCOP_USTAT	12 /* Unsolicited Receiver State Information */
#define SSCOP_UD	13 /* Unnumbered User Data */
#define SSCOP_MD	14 /* Unnumbered Management Data */


/* Trailer format macros */

#define SSCOP_TRAIL(type,pad,n) (htonl((n) | ((type) << 24) | ((pad) << 30)))
#define SSCOP_S_BIT 0x10000000
#define SSCOP_TYPE(last) ((read_netl(last) >> 24) & 15)
#define SSCOP_PAD(last) (read_netl(last) >> 30)
#define SSCOP_N(last) (read_netl(last) & 0xffffff)
#define SSCOP_S(last) (read_netl(last) & SSCOP_S_BIT)
#define SSCOP_SQ(last) (read_netl((char *) last-4) & 0xff)


extern const char *pdu_name[];


/* Helper macros for PDU construction and decomposition */

#define PDU_VARS \
  unsigned char type; \
  int length; \
  int s,ps,r,mr,sq
#define DECOMPOSE_PDU(maa_arg,msg,size) decompose_pdu(maa_arg,msg,size,&type, \
  &length,&s,&ps,&r,&mr,&sq)
#define PRINT_PDU(label,data) print_pdu(label,type,data,&length,&s,&ps,&r, \
  &mr,&sq)


/*
 * Severity codes for pdu_diag. Surprisingly, they happen to have the same
 * numerical values as their corresponding diag counterparts.
 */

#define SP_DEBUG	3
#define SP_WARN		1
#define SP_ERROR	0


extern void (*pdu_maa)(void *arg,char code,int count);
extern void (*pdu_diag)(int severity,const char *fmt,...);


void print_pdu(const char *label,unsigned char type,void *data,
  const int *length,const int *s,const int *ps,const int *r,const int *mr,
  const int *sq);
int decompose_pdu(void *maa_arg,void *msg,int size,unsigned char *type,
  int *length,int *s,int *ps,int *r,int *mr,int *sq);

#endif
