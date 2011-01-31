/* pdu.c - SSCOP (Q.2110) PDU reader */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

#include "pdu.h"


#ifndef NULL
#define NULL 0
#endif


const char *pdu_name[] = { "???","BGN","BGAK","END","ENDAK","RS","RSAK",
  "BGREJ","SD","ER","POLL","STAT","USTAT","UD","MD","ERAK" };


void (*pdu_maa)(void *arg,char code,int count) = NULL;
void (*pdu_diag)(int severity,const char *fmt,...) = NULL;


void print_pdu(const char *label,unsigned char type,void *data,
  const int *length,const int *s,const int *ps,const int *r,const int *mr,
  const int *sq)
{
    int len;
    char *list;

    switch (type) {
	case SSCOP_SD:
	    pdu_diag(SP_DEBUG,"%s SD(S=%d,len=%d)",label,*s,*length);
	    break;
	case SSCOP_POLL:
	    pdu_diag(SP_DEBUG,"%s POLL(PS=%d,S=%d)",label,*ps,*s);
	    break;
	case SSCOP_STAT:
	    if (*length & 3) {
		pdu_diag(SP_WARN,"%s STAT PDU has wrong size (%d)",label,
		  *length);
		break;
	    }
	    len = *length/4;
	    pdu_diag(SP_DEBUG,"%s STAT(PS=%d,MR=%d,R=%d,items=%d:",label,*ps,
	      *mr,*r,len);
	    list = (char *) data;
	    while (len > 1) {
		pdu_diag(SP_DEBUG,"%s    %d..%d",label,read_netl(list),
		  read_netl(list+4));
		list += 8;
		len -= 2;
	    }
	    if (!len)
		pdu_diag(SP_DEBUG,"%s    <last is absent>)",label);
	    else pdu_diag(SP_DEBUG,"%s    <next is> %d)",label,read_netl(list));
	    break;
	case SSCOP_USTAT:
	    if (*length != 8) {
		pdu_diag(SP_WARN,"%s USTAT PDU has wrong size (%d)",label,
		  *length);
		break;
	    }
	    list = (char *) data;
	    pdu_diag(SP_DEBUG,"%s USTAT(MR=%d,R=%d,%d..%d)",label,*mr,*r,
	      read_netl(list),read_netl(list+4));
	    break;
	case SSCOP_UD:
	    pdu_diag(SP_DEBUG,"%s UD(len=%d)",label,*length);
	    break;
	case SSCOP_MD:
	    pdu_diag(SP_DEBUG,"%s MD(len=%d)",label,*length);
	    break;
	case SSCOP_BGN:
	    pdu_diag(SP_DEBUG,"%s BGN(SQ=%d,MR=%d,len=%d)",label,*sq,*mr,
	      *length);
	    break;
	case SSCOP_BGAK:
	    pdu_diag(SP_DEBUG,"%s BGAK(MR=%d,len=%d)",label,*mr,*length);
	    break;
	case SSCOP_BGREJ:
	    pdu_diag(SP_DEBUG,"%s BGREJ(len=%d)",label,*length);
	    break;
	case SSCOP_END:
	    pdu_diag(SP_DEBUG,"%s END(S=%d,len=%d)",label,*s,*length);
	    break;
	case SSCOP_ENDAK:
	    pdu_diag(SP_DEBUG,"%s ENDAK()",label);
	    break;
	case SSCOP_RS:
	    pdu_diag(SP_DEBUG,"%s RS(SQ=%d,MR=%d,len=%d)",label,*sq,*mr,
	      *length);
	    break;
	case SSCOP_RSAK:
	    pdu_diag(SP_DEBUG,"%s RSAK(MR=%d)",label,*mr);
	    break;
	case SSCOP_ER:
	    pdu_diag(SP_DEBUG,"%s ER(MR=%d)",label,*mr);
	    break;
	case SSCOP_ERAK:
	    pdu_diag(SP_DEBUG,"%s ERAK(MR=%d)",label,*mr);
	    break;
	default:
	    pdu_diag(SP_ERROR,"%s unknown PDU type %d\n",label,type);
    }
}


int decompose_pdu(void *maa_arg,void *msg,int size,unsigned char *type,
  int *length,int *s,int *ps,int *r,int *mr,int *sq)
{
    void *last;
    unsigned char pad;
    int n;

/*
 * *length is undefined if PDU has no variable-length data part
 */

    if (size < 4 || (size & 3)) {
	pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,"invalid message length (%d)",
	  size);
	if (pdu_maa) pdu_maa(maa_arg,'U',0);
	return -1;
    }
    last = (char *) msg+size-4;
    *type = SSCOP_TYPE(last);
    pad = SSCOP_PAD(last);
    n = SSCOP_N(last);
    *length = size-4-pad;
    switch (*type) {
	case SSCOP_SD:
	    *s = n;
	    break;
	case SSCOP_POLL:
	    if (size != 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "POLL PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *s = n;
	    *ps = SSCOP_N(msg);
	    break;
	case SSCOP_STAT:
	    if (size < 12) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "STAT PDU too short (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    if (*length & 3) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "STAT PDU has bad length (%d)",length);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *r = n;
	    *mr = SSCOP_N((char *) last-4);
	    *ps = SSCOP_N((char *) last-8);
	    *length -= 8;
	    break;
	case SSCOP_USTAT:
	    if (size != 16) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "USTAT PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *r = n;
	    *mr = SSCOP_N((char *) last-4);
	    *length -= 4;
	    break;
	case SSCOP_UD:
	case SSCOP_MD:
	    break;
	case SSCOP_BGN:
	    if (size < 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,"BGN PDU too short (%d)",
		  size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    *sq = SSCOP_SQ(last);
	    *length -= 4;
	    break;
	case SSCOP_BGAK:
	    if (size < 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "BGAK PDU too short (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    *length -= 4;
	    break;
	case SSCOP_BGREJ:
	    if (size < 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "BGREJ PDU too short (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *length -= 4;
	    break;
	case SSCOP_END:
	    if (size < 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,"END PDU too short (%d)",
		  size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *s = !!SSCOP_S(last);
	    *length -= 4;
	    break;
	case SSCOP_ENDAK:
	    if (size != 8) {
		pdu_diag(SP_DEBUG,"ENDAK PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		if (size < 4) return -1; /* make it work with Fore */
	    }
	    break;
	case SSCOP_RS:
	    if (size < 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,"RS PDU too short (%d)",
		  size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    *sq = SSCOP_SQ(last);
	    *length -= 4;
	    break;
	case SSCOP_RSAK:
	    if (size != 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "RSAK PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    break;
	case SSCOP_ER:
	    if (size != 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "ER PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    *sq = SSCOP_SQ(last);
	    break;
	case SSCOP_ERAK:
	    if (size != 8) {
		pdu_diag(pdu_maa ? SP_DEBUG : SP_WARN,
		  "ERAK PDU has bad length (%d)",size);
		if (pdu_maa) pdu_maa(maa_arg,'U',0);
		return -1;
	    }
	    *mr = n;
	    break;
	default:
	    pdu_diag(SP_ERROR,"unknown PDU type %d (0x%x)",*type,*type);
	    return -1;
    }
    return 0;
}
