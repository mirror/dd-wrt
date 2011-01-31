/* sapequal.c - Compares SAP specifications for compatibility */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>
#include <string.h>

#include "atm.h"
#include "atmsap.h"


static int bhli_compat(const struct atm_bhli a,const struct atm_bhli b,
  int flags)
{
    int length;

    if (!a.hl_type && (flags & SXE_COMPATIBLE)) return 1;
    if (a.hl_type != b.hl_type) return 0;
    switch (a.hl_type) {
	case ATM_HL_ISO:
	case ATM_HL_USER:
	    length = a.hl_length;
	    if (length != b.hl_length) return 0;
	    break;
	case ATM_HL_HLP:
	    length = 4;
	    break;
	case ATM_HL_VENDOR:
	    length = 7;
	    break;
	default:
	    length = 0;
    }
    return !length || !memcmp(a.hl_info,b.hl_info,length);
}


#define CHECK(FIELD,CONSTRAINT) \
  if (res && !res->FIELD) res->FIELD = a.FIELD; \
  if (a.FIELD && b.FIELD && a.FIELD != b.FIELD) { \
    if (!(flags & SXE_NEGOTIATION)) return 0; \
    if (!CONSTRAINT) return 0; \
    if (res) res->FIELD = a.FIELD < b.FIELD ? a.FIELD : b.FIELD; \
  }


static int match_blli(const struct atm_blli a,const struct atm_blli b,
  int flags,struct atm_blli *res)
{
    if (res) *res = b;
    if (a.l2_proto != b.l2_proto || a.l3_proto != b.l3_proto) return 0;
    switch (a.l2_proto) {
	case ATM_L2_X25_LL:
	case ATM_L2_X25_ML:
	case ATM_L2_HDLC_ARM:
	case ATM_L2_HDLC_NRM:
	case ATM_L2_HDLC_ABM:
	case ATM_L2_Q922:
	case ATM_L2_ISO7776:
	    CHECK(l2.itu.mode,1);
	    CHECK(l2.itu.window,a.l2.itu.window > b.l2.itu.window);
	    break;
    }
    switch (a.l3_proto) {
	case ATM_L3_X25:
	case ATM_L3_ISO8208:
	case ATM_L3_X223:
	    CHECK(l3.itu.mode,1);
	    CHECK(l3.itu.def_size,a.l3.itu.def_size > b.l3.itu.def_size);
	    CHECK(l3.itu.window,a.l3.itu.window > b.l3.itu.window);
	    break;
	case ATM_L3_TR9577:
	    if (a.l3.tr9577.ipi != b.l3.tr9577.ipi) return 0;
	    if (a.l3.tr9577.ipi == NLPID_IEEE802_1_SNAP)
		if (memcmp(a.l3.tr9577.snap,b.l3.tr9577.snap,5)) return 0;
	    break;
	case ATM_L3_USER:
	    if (a.l3.user != b.l3.user) return 0;
	    break;
    }
    return 1;
}


#undef CHECK


static int blli_compat(const struct atm_blli *a,const struct atm_blli *b,
   int flags,struct atm_blli *res)
{
    int i,j;

    if (!(flags & SXE_COMPATIBLE)) {
	for (i = 0; i < ATM_MAX_BLLI; i++)
	    if (blli_in_use(a[i]))
		if (!blli_in_use(b[i])) return 0;
		else {
		    if (!match_blli(a[i],b[i],0,NULL)) return 0;
		}
	    else if (blli_in_use(b[i])) return 0;
		else break;
	if (res) *res = *a;
	return 1;
    }
    if (!blli_in_use(*a)) {
	if (res) *res = *b;
	return 1;
    }
    for (i = 0; i < ATM_MAX_BLLI && blli_in_use(a[i]); i++)
	for (j = 0; j < ATM_MAX_BLLI && blli_in_use(b[j]); j++)
	    if (match_blli(a[i],b[j],flags,res)) return 1;
    return 0;
}


int sap_equal(const struct atm_sap *a,const struct atm_sap *b,int flags,...)
{
    va_list ap;
    struct atm_sap *res;

    va_start(ap,flags);
    res = flags & SXE_RESULT ? va_arg(ap,struct atm_sap *) : NULL;
    va_end(ap);
    if (!bhli_compat(a->bhli,b->bhli,flags)) return 0;
    if (!blli_compat(a->blli,b->blli,flags,res ? res->blli : NULL)) return 0;
    if (res) {
	res->bhli = b->bhli;
	memset(res->blli+1,0,sizeof(struct atm_blli)*(ATM_MAX_BLLI-1));
    }
    return 1;
}
