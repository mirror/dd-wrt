/* atmequal.c - Compares ATM addresses for equality */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */


#include <string.h>
#include <assert.h>
#include <sys/socket.h>

#include "atm.h"


static int atm_equal_pvc(const struct sockaddr_atmpvc *a,
  const struct sockaddr_atmpvc *b,int flags)
{
    int wc = flags & AXE_WILDCARD;

#define EQ(field,any) \
  (a->sap_addr.field == b->sap_addr.field || \
    (wc && (a->sap_addr.field == any || b->sap_addr.field == any)))
    return EQ(itf,ATM_ITF_ANY) && EQ(vpi,ATM_VPI_ANY) && EQ(vci,ATM_VCI_ANY);
#undef EQ
}


static int atm_equal_svc(const struct sockaddr_atmsvc *a,
  const struct sockaddr_atmsvc *b,int len,int flags)
{
    const unsigned char *a_prv,*b_prv;
    int len_a,len_b;

    if (!(flags & AXE_WILDCARD)) len = ATM_ESA_LEN*8;
    assert(len >= 0 && len <= ATM_ESA_LEN*8);
    if (*a->sas_addr.prv && *b->sas_addr.prv) {
	a_prv = a->sas_addr.prv;
	b_prv = b->sas_addr.prv;
	if ((flags & AXE_WILDCARD) && len >= 8 && *a_prv == ATM_AFI_E164 &&
	  *b_prv == ATM_AFI_E164) {
	    if (len < 68) return 0; /* no comparison possible */
	    else {
		int a_pos,b_pos;
		unsigned char a_val,b_val;

		for (a_pos = 2; !a_prv[a_pos/2]; a_pos += 2);
		if (!(a_prv[a_pos/2] & 0xf0)) a_pos++;
		for (b_pos = 2; !b_prv[b_pos/2]; b_pos += 2);
		if (!(b_prv[b_pos/2] & 0xf0)) b_pos++;
		while (1) {
		    a_val = (a_prv[a_pos/2] >> (((~a_pos) & 1)*4)) & 0xf;
		    b_val = (b_prv[b_pos/2] >> (((~b_pos) & 1)*4)) & 0xf;
		    if (a_val == 15 || b_val == 15) break;
		    if (a_val != b_val) return 0;
		    a_pos++;
		    b_pos++;
		}
		a_prv += 9;
		b_prv += 9;
		if ((len -= 72) < 0) len = 0;
	    }
	}
	if (memcmp(a_prv,b_prv,len/8)) return 0;
	if ((len & 7) && (a_prv[len/8+1]^b_prv[len/8+1]) &
	  (0xff00 >> (len & 7))) return 0;
	return 1;
    }
    if ((*a->sas_addr.prv || *b->sas_addr.prv) && !(flags & AXE_PRVOPT))
	return 0;
    if (!*a->sas_addr.pub || !*b->sas_addr.pub) return 0;
    len_a = strlen(a->sas_addr.pub);
    len_b = strlen(b->sas_addr.pub);
    if (len_a != len_b && !(flags & AXE_WILDCARD)) return 0;
    return !strncmp(a->sas_addr.pub,b->sas_addr.pub,len_a < len_b ? len_a :
      len_b);
}


int atm_equal(const struct sockaddr *a,const struct sockaddr *b,int len,
  int flags)
{
    assert((a->sa_family == AF_ATMPVC && b->sa_family == AF_ATMPVC) ||
      (a->sa_family == AF_ATMSVC && b->sa_family == AF_ATMSVC));
    if (a->sa_family == AF_ATMPVC)
	return atm_equal_pvc((const struct sockaddr_atmpvc *) a,
	  (const struct sockaddr_atmpvc *) b,flags);
    return atm_equal_svc((const struct sockaddr_atmsvc *) a,
      (const struct sockaddr_atmsvc *) b,len,flags);
}
