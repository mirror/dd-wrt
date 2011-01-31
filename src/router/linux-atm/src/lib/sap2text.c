/* sap2text.c - Converts binary encoding of a SAP (Service Access Point; BHLI
		and BLLI) to textual representation */

/* Written 1997 by Werner Almesberger, EPFL-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <atmsap.h>
#include <linux/atmsap.h>

#include "atm.h"


/*
 * <a-b> = a-b bytes
 *
 * bhli:
 *  iso=<1-8>
 *  user=<1-8>
 *  hlp=<4>
 *  oui=<3>,id=<4>
 * blli:
 *  l2=x25_ll | x25_ml | hdlc_arm | hdlc_nrm | hdlc_abm | q922 | iso7776
 *  l2=user,info=<1>
 *  l2=iso1745 | q291 | lapb | iso8802 | x75
 *
 * etc.
 */



static int put(char **pos,char **end,int *length,const char *fmt,...)
{
    char scratch[40]; /* we don't put long strings anyway */
    va_list ap;
    int len;

    va_start(ap,fmt);
    len = vsnprintf(scratch,40,fmt,ap);
    va_end(ap);
    if (!len) return 0;
    if (!*end) return -1;
    *length -= *end-*pos;
    *pos = *end;
    if (len >= *length) return -1;
    strcpy(*pos,scratch);
    *pos += len;
    *end = *pos;
    return 0;
}


static int dump(char **pos,char **end,int *length,const unsigned char *start,
  int size)
{
    int i;

    if (!size) return -1;
    put(pos,end,length,"0x");
    for (i = 0; i < size; i++)
	if (put(pos,end,length,"%02X",start[i]) < 0) return -1;
    return 0;
}


static void maybe(char **pos,char **end,int *length,const char *str)
{
    int len;

    if (!*end) return;
    len = strlen(str)+*end-*pos;
    if (len >= *length) { /* okay, but fail on the next PUT */
	*end = NULL;
	return;
    }
    strcpy(*end,str);
    *end = *pos+len;
}


#define PUT(fmt,arg...) if (put(pos,end,length,fmt,##arg) < 0) return -1
#define DUMP(start,size) if (dump(pos,end,length,start,size) < 0) return -1
#define MAYBE(str) maybe(pos,end,length,str)


static int bhli2text(char **pos,char **end,int *length,
  const struct atm_bhli *bhli)
{
    if (!bhli->hl_type) return 0;
    PUT("bhli:");
    switch (bhli->hl_type) {
	case ATM_HL_ISO:
	    PUT("iso=");
	    DUMP(bhli->hl_info,bhli->hl_length);
	    break;
	case ATM_HL_USER:
	    PUT("user=");
	    DUMP(bhli->hl_info,bhli->hl_length);
	    break;
#if defined(UNI30) || defined(ALLOW_UNI30)
	case ATM_HL_HLP:
	    if (bhli->hl_length != 4) return -1;
	    PUT("hlp=");
	    DUMP(bhli->hl_info,bhli->hl_length);
	    break;
#endif
	case ATM_HL_VENDOR:
	    if (bhli->hl_length != 7) return -1;
	    PUT("oui=");
	    DUMP(bhli->hl_info,3);
	    PUT(",id=");
	    DUMP(bhli->hl_info+3,4);
	    break;
	default:
	    return -1;
    }
    MAYBE(",");
    return 0;
}


static int l2_proto2text(char **pos,char **end,int *length,
  const struct atm_blli *blli)
{
    if (!blli->l2_proto) return 0;
    PUT("l2=");
    switch (blli->l2_proto) {
#define X(u,l) case ATM_L2_##u: PUT(#l); MAYBE(","); return 0
	X(ISO1745,iso1745);
	X(Q291,q291);
	X(LAPB,lapb);
	X(ISO8802,iso8802);
	X(X75,x75);
#undef X
#define X(u,l) case ATM_L2_##u: PUT(#l); break
	X(X25_LL,x25_ll);
	X(X25_ML,x25_ml);
	X(HDLC_ARM,hdlc_arm);
	X(HDLC_NRM,hdlc_nrm);
	X(HDLC_ABM,hdlc_abm);
	X(Q922,q992);
	X(ISO7776,iso7776);
#undef X
	case ATM_L2_USER:
	    PUT("user,info=%d",blli->l2.user);
	    MAYBE(",");
	    return 0;
	default:
	    return -1;
    }
    MAYBE(",");
    if (blli->l2.itu.mode) {
	PUT("mode=");
	switch (blli->l2.itu.mode) {
	    case ATM_IMD_NORMAL:
		PUT("norm");
		break;
	    case ATM_IMD_EXTENDED:
		PUT("ext");
		break;
	    default:
		return -1;
	}
	MAYBE(",");
    }
    if (blli->l2.itu.window) {
	PUT("window=%d",blli->l2.itu.window);
	MAYBE(",");
    }
    return 0;
}


static int mpx_cap(char **pos,char **end,int *length,const char *label,
  int cap)
{
    if (!cap) return 0;
    PUT("%s=",label);
    switch (cap) {
	case ATM_MC_TS:
	    PUT("ts");
	    break;
	case ATM_MC_TS_FEC:
	    PUT("ts_fec");
	    break;
	case ATM_MC_PS:
	    PUT("ps");
	    break;
	case ATM_MC_PS_FEC:
	    PUT("ps_fec");
	    break;
	case ATM_MC_H221:
	    PUT("h221");
	    break;
	default:
	    return -1;
    }
    MAYBE(",");
    return 0;
}


static int l3_proto2text(char **pos,char **end,int *length,
  const struct atm_blli *blli)
{
    if (!blli->l3_proto) return 0;
    PUT("l3=");
    switch (blli->l3_proto) {
#define X(u,l) case ATM_L3_##u: PUT(#l); break
	X(X25,x25);
	X(ISO8208,iso8208);
	X(X223,x223);
#undef X
	case ATM_L3_TR9577:
	    PUT("tr9577,ipi=");
	    if (blli->l3.tr9577.ipi != NLPID_IEEE802_1_SNAP) {
		PUT("0x%x",blli->l3.tr9577.ipi);
	    }
	    else {
		PUT("snap,oui=");
		DUMP(blli->l3.tr9577.snap,3);
		PUT(",pid=");
		DUMP(blli->l3.tr9577.snap+3,2);
	    }
	    MAYBE(",");
	    return 0;
	case ATM_L3_USER:
	    PUT("user,info=%d",blli->l3.user);
	    MAYBE(",");
	    return 0;
#define X(u,l) case ATM_L3_##u: PUT(#l); MAYBE(","); return 0
	X(ISO8473,iso8473);
	X(T70,t70);
	X(H321,h321);
#undef X
	case ATM_L3_H310:
	    PUT("h310");
	    MAYBE(",");
	    switch (blli->l3.h310.term_type) {
		case ATM_TT_NONE:
		    return 0;
		case ATM_TT_RX:
		    PUT("term=rx");
		    break;
		case ATM_TT_TX:
		    PUT("term=tx");
		    break;
		case ATM_TT_RXTX:
		    PUT("term=rxtx");
		    break;
		default:
		    return -1;
	    }
	    MAYBE(",");
	    mpx_cap(pos,end,length,"fw_mpx",blli->l3.h310.fw_mpx_cap);
	    mpx_cap(pos,end,length,"bw_mpx",blli->l3.h310.bw_mpx_cap);
	    return 0;
	default:
	    return -1;
    }
    MAYBE(",");
    if (blli->l3.itu.mode) {
	PUT("mode=");
	switch (blli->l3.itu.mode) {
	    case ATM_IMD_NORMAL:
		PUT("norm");
		break;
	    case ATM_IMD_EXTENDED:
		PUT("ext");
		break;
	    default:
		return -1;
	}
	MAYBE(",");
    }
    if (blli->l3.itu.def_size) {
	PUT("size=%d",blli->l3.itu.def_size);
	MAYBE(",");
    }
    if (blli->l3.itu.window) {
	PUT("window=%d",blli->l3.itu.window);
	MAYBE(",");
    }
    return 0;
}


static int blli2text(char **pos,char **end,int *length,
  const struct atm_blli *blli)
{
    MAYBE("blli:");
    if (l2_proto2text(pos,end,length,blli)) return -1;
    return l3_proto2text(pos,end,length,blli);
}


int sap2text(char *buffer,int length,const struct atm_sap *sap,int flags)
{
    char *pos,*end;
    int i;

    pos = end = buffer;
    if (bhli2text(&pos,&end,&length,&sap->bhli) < 0) return -1;
    for (i = 0; i < ATM_MAX_BLLI; i++) {
	if (!blli_in_use(sap->blli[i])) break;
	if (blli2text(&pos,&end,&length,sap->blli+i) < 0) return -1;
    }
    *pos = 0;
    return pos-buffer;
}
