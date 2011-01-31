/* text2sap.c - Converts textual representation of a SAP (Service Access Point;
		BHLI and BLLI) to binary encoding */

/* Written 1997 by Werner Almesberger, EPFL-ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <atmsap.h>
#include <linux/atmsap.h>

#include "atm.h"


#define fetch __atmlib_fetch


static int bytes(const char **text,unsigned char *buf,int *len,int min,int max)
{
    int num,value;

    num = 0;
    if (**text == '0' && ((*text)[1] == 'x' || (*text)[1] == 'X')) *text += 2;
    while (sscanf(*text,"%2x",&value) == 1) {
	if (++num > max) return -1;
	*buf++ = value;
	*text += 2;
    }
    if (num < min) return -1;
    if (len) *len = num;
    return 0;
}


static int text2bhli(const char **text,struct atm_bhli *bhli)
{
    int len;

    switch (fetch(text,"iso=","user=","hlp=","oui=",NULL)) {
	case 0:
	    bhli->hl_type = ATM_HL_ISO;
	    if (bytes(text,bhli->hl_info,&len,1,8) < 0) return -1;
	    bhli->hl_length = len;
	    break;
	case 1:
	    bhli->hl_type = ATM_HL_USER;
	    if (bytes(text,bhli->hl_info,&len,1,8) < 0) return -1;
	    bhli->hl_length = len;
	    break;
#if defined(UNI30) || defined(ALLOW_UNI30)
	case 2:
	    bhli->hl_type = ATM_HL_HLP;
	    if (bytes(text,bhli->hl_info,NULL,4,4) < 0) return -1;
	    bhli->hl_length = 4;
	    break;
#endif
	case 3:
	    bhli->hl_type = ATM_HL_VENDOR;
	    if (bytes(text,bhli->hl_info,NULL,3,3) < 0) return -1;
	    if (fetch(text,",id=",NULL) < 0) return -1;
	    if (bytes(text,bhli->hl_info+3,NULL,4,4) < 0) return -1;
	    bhli->hl_length = 7;
	    break;
	default:
	    return -1;
    }
    return 0;
}


static int get_int(const char **text,int *value,int min,int max)
{
    char *end;

    *value = strtoul(*text,&end,0);
    if (end == *text) return -1;
    if (*value < min || *value > max) return -1;
    *text = end;
    return 0;
}


static int text2l2_proto(const char **text,struct atm_blli *blli)
{
    static int map[] = {
      /* No parameters */
      ATM_L2_ISO1745, ATM_L2_Q291,ATM_L2_LAPB,ATM_L2_ISO8802,ATM_L2_X75,
      /* With ITU parameters */
      ATM_L2_X25_LL, ATM_L2_X25_ML, ATM_L2_HDLC_ARM, ATM_L2_HDLC_NRM,
      ATM_L2_HDLC_ABM, ATM_L2_Q922, ATM_L2_ISO7776,
      /* Other parameter sets */
      ATM_L2_USER };
    int item,value;

    item = fetch(text,"iso1745","q291","lapb","iso8802","x75","x25_ll",
      "x25_ml","hdlc_arm","hdlc_nrm","hdlc_abm","q992","iso7776","user,info=",
      NULL);
    if (item < 0) return -1;
    blli->l2_proto = map[item];
    if (blli->l2_proto < ATM_L2_X25_LL) return 0;
    if (blli->l2_proto == ATM_L2_USER) {
	if (get_int(text,&value,0,255) < 0) return -1;
	blli->l2.user = value;
	return 0;
    }
    if (!fetch(text,",mode=",NULL)) {
	switch (fetch(text,"norm","ext",NULL)) {
	    case 0:
		blli->l2.itu.mode = ATM_IMD_NORMAL;
		break;
	    case 1:
		blli->l2.itu.mode = ATM_IMD_EXTENDED;
		break;
	    default:
		return -1;
	}
    }
    if (!fetch(text,",window=",NULL)) {
	if (get_int(text,&value,1,127) < 0) return -1;
	blli->l2.itu.window = value;
    }
    return 0;
}


static int text2l3_proto(const char **text,struct atm_blli *blli)
{
    static int map[] = {
      /* No parameters */
      ATM_L3_ISO8473,ATM_L3_T70,ATM_L3_H321,
      /* With ITU parameters */
      ATM_L3_X25,ATM_L3_ISO8208,ATM_L3_X223,
      /* Other parameter sets */
      ATM_L3_TR9577,ATM_L3_USER,ATM_L3_H310 };
    int item,value;

    item = fetch(text,"iso8473","t70","h321","x25","iso8208","x223",
      "tr9577,ipi=","user,info=","h310",NULL);
    if (item < 0) return -1;
    blli->l3_proto = map[item];
    if (blli->l3_proto < ATM_L3_X25) return 0;
    if (blli->l3_proto == ATM_L3_TR9577) {
	if (!fetch(text,"snap",NULL)) value = NLPID_IEEE802_1_SNAP;
	else if (get_int(text,&value,0,255) < 0) return -1;
	blli->l3.tr9577.ipi = value;
	if (value != NLPID_IEEE802_1_SNAP) return 0;
	if (fetch(text,",oui=",NULL) < 0) return -1;
	if (bytes(text,blli->l3.tr9577.snap,NULL,3,3) < 0) return -1;
	if (fetch(text,",pid=",NULL) < 0) return -1;
	if (bytes(text,blli->l3.tr9577.snap+3,NULL,2,2) < 0) return -1;
	return 0;
    }
    if (blli->l3_proto == ATM_L3_USER) {
	if (get_int(text,&value,0,255) < 0) return -1;
	blli->l3.user = value;
	return 0;
    }
    if (blli->l3_proto == ATM_L3_H310) {
	if (fetch(text,",term=",NULL)) return 0;
	item = fetch(text,"!none","rx","tx","rxtx",NULL);
	if (item == -1) return -1;
	blli->l3.h310.term_type = item;
	if (!fetch(text,",fw_mpx=",NULL)) {
	    item = fetch(text,"!none","ts","ts_fec","ps","ps_fec","h221",NULL);
	    if (item == -1) return -1;
	    blli->l3.h310.fw_mpx_cap = item;
	}
	if (!fetch(text,",bw_mpx=",NULL)) {
	    item = fetch(text,"!none","ts","ts_fec","ps","ps_fec","h221",NULL);
	    if (item == -1) return -1;
	    blli->l3.h310.bw_mpx_cap = item;
	}
	return 0;
    }
    if (!fetch(text,",mode=",NULL)) {
	switch (fetch(text,"norm","ext",NULL)) {
	    case 0:
		blli->l3.itu.mode = ATM_IMD_NORMAL;
		break;
	    case 1:
		blli->l3.itu.mode = ATM_IMD_EXTENDED;
		break;
	    default:
		return -1;
	}
    }
    if (!fetch(text,",size=",NULL)) {
	if (get_int(text,&value,4,12) < 0) return -1;
	blli->l3.itu.def_size = value;
    }
    if (!fetch(text,",window=",NULL)) {
	if (get_int(text,&value,1,127) < 0) return -1;
	blli->l3.itu.window = value;
    }
    return 0;
}


static int text2blli(const char **text,struct atm_blli *blli)
{
    switch (fetch(text,"l2=","l3=",NULL)) {
	case 0:
	    if (text2l2_proto(text,blli) < 0) return -1;
	    break;
	case 1:
	    return text2l3_proto(text,blli);
	default:
	    return -1;
    }
    if (!**text) return 0;
    if (fetch(text,",l3=",NULL) < 0) return 0;
    return text2l3_proto(text,blli);
}


int text2sap(const char *text,struct atm_sap *sap,int flags)
{
    int bllis;

    memset(sap,0,sizeof(*sap));
    if (!*text) return 0;
    switch (fetch(&text,"bhli:","blli:",NULL)) {
	case 0:
	    if (text2bhli(&text,&sap->bhli) < 0) return -1;
	    bllis = 0;
	    break;
	case 1:
	    if (text2blli(&text,sap->blli) < 0) return -1;
	    bllis = 1;
	    break;
	default:
	    return -1;
    }
    while (1) {
	if (!*text) return 0;
	if (fetch(&text,",blli:",NULL) < 0) return -1;
	if (bllis == ATM_MAX_BLLI) return 0; /* ignore extra BLLIs */
	if (text2blli(&text,sap->blli+bllis) < 0) return -1;
	bllis++;
    }
}
