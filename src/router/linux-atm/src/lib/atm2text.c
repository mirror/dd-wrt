/* atm2text.c - Converts binary encoding of ATM address to textual
		representation */

/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "atm.h"
#include "atmres.h"


static int put_item(char **buffer,int *length,int value)
{
    char *walk,*scan;
    char tmp;

    if (!*length) return FATAL;
    if (value <= 0) {
	if (!value) *(*buffer)++ = '0';
	else if (value == ATM_VCI_ANY) *(*buffer)++ = '*';
	    else if (value == ATM_VCI_UNSPEC) *(*buffer)++ = '?';
		else return FATAL;
			/* ATM_*_ANY and ATM_*_UNSPEC have all the same value,
			   respectively */
	(*length)--;
	return 0;
    }
    for (walk = *buffer; value; value /= 10) {
	if (!(*length)--) return FATAL;
	*walk++ = '0'+(value % 10);
    }
    for (scan = walk-1; scan > *buffer; (*buffer)++) {
	tmp = *scan;
	*scan-- = **buffer;
	**buffer = tmp;
    }
    *buffer = walk;
    return 0;
}


static int do_pvc(char *buffer,int length,const struct sockaddr_atmpvc *addr,
  int flags)
{
    int orig_len;

    orig_len = length;
    if (put_item(&buffer,&length,addr->sap_addr.itf)) return FATAL;
    if (!length--) return FATAL;
    *buffer++ = '.';
    if (put_item(&buffer,&length,addr->sap_addr.vpi)) return FATAL;
    if (!length--) return FATAL;
    *buffer++ = '.';
    if (put_item(&buffer,&length,addr->sap_addr.vci)) return FATAL;
    if (!length) return FATAL;
    *buffer = 0;
    return orig_len-length;
}


static int do_svc(char *buffer,int length,const struct sockaddr_atmsvc *addr,
  int flags)
{
    static int pure[] = { 20 };
    static int bin[] = { 1,2,10,6,1 };
    static int local[] = { 1,12,6,1 };
    static int e164[] = { 4,6,1 };

    int orig_len,len,i,left,value;
    int *fmt;

    orig_len = length;
    if (!*addr->sas_addr.pub && !*addr->sas_addr.prv) return FATAL;
    if (*addr->sas_addr.pub) {
	len = strlen(addr->sas_addr.pub);
	if (!*addr->sas_addr.prv && length >= len+2) {
	    *buffer++ = '+';
	    length--;
	}
	if (length < len+1) return FATAL;
	strcpy(buffer,addr->sas_addr.pub);
	buffer += len;
	length -= len;
	if (*addr->sas_addr.prv) {
	    if (!length--) return FATAL;
	    *buffer++ = '+';
	}
    }
    if (*addr->sas_addr.prv) {
	fmt = pure;
	i = 0;
	if (flags & A2T_PRETTY)
	    switch (*addr->sas_addr.prv) {
		case ATM_AFI_DCC:
		case ATM_AFI_ICD:
		case ATM_AFI_DCC_GROUP:
		case ATM_AFI_ICD_GROUP:
		    fmt = bin;
		    break;
		case ATM_AFI_LOCAL:
		case ATM_AFI_LOCAL_GROUP:
		    fmt = local;
		    break;
		case ATM_AFI_E164:
		case ATM_AFI_E164_GROUP:
		    for (i = 2; i < 17; i++)
			if (addr->sas_addr.prv[i >> 1] & (0xf0 >> 4*(i & 1)))
			    break;
		    while (i < 17) {
			value = (addr->sas_addr.prv[i >> 1] >> 4*(1-(i & 1))) &
			  0xf;
			if (value > 9) return FATAL;
			if (!length--) return FATAL;
			*buffer++ = '0'+value;
			i++;
		    }
		    if (!length--) return FATAL;
		    *buffer++ = ':';
		    i = 9;
		    fmt = e164;
		    break;
		default:
		    break;
	    }
	for (left = *fmt++; i < ATM_ESA_LEN; i++) {
	    if (!left--) {
		if (!length--) return FATAL;
		*buffer++ = '.';
		left = *fmt++-1;
	    }
	    if (length < 2) return FATAL;
	    sprintf(buffer,"%02X",addr->sas_addr.prv[i]);
	    length -= 2;
	    buffer += 2;
	}
    }
    if (!length) return FATAL;
    *buffer = 0;
    return orig_len-length;
}


static int search(FILE *file,char *buffer,int length,
  const struct sockaddr *addr,int flags)
{
    struct sockaddr_atmsvc temp;
    char line[MAX_ATM_NAME_LEN+1];
    const char *here;
 
    while (fgets(line,MAX_ATM_NAME_LEN,file)) {
        if (!(here = strtok(line,"\t\n "))) continue;
	if (text2atm(here,(struct sockaddr *) &temp,sizeof(temp),flags) < 0)
	    continue;
	if (temp.sas_family != addr->sa_family) continue;
	if (temp.sas_family == AF_ATMPVC) {
	    if (((const struct sockaddr_atmpvc *) addr)->sap_addr.itf !=
	      ((struct sockaddr_atmpvc *) &temp)->sap_addr.itf ||
	      ((const struct sockaddr_atmpvc *) addr)->sap_addr.vpi !=
	      ((struct sockaddr_atmpvc *) &temp)->sap_addr.vpi ||
	      ((const struct sockaddr_atmpvc *) addr)->sap_addr.vci !=
	      ((struct sockaddr_atmpvc *) &temp)->sap_addr.vci) continue;
	}
	else if (!atm_equal(addr,(struct sockaddr *) &temp,0,0)) continue;
        while ((here = strtok(NULL,"\t\n ")))
	    if (strlen(here) < length) {
		strcpy(buffer,here);
		return 0;
	    }
	return FATAL;
    }
    return TRY_OTHER;
}
 
 
static int try_name(char *buffer,int length,const struct sockaddr *addr)
{
    FILE *file;
    int result;
 
    if (!(file = fopen(HOSTS_ATM,"r"))) return TRY_OTHER;
    result = search(file,buffer,length,addr,addr->sa_family == AF_ATMPVC ?
      T2A_PVC : T2A_SVC);
    (void) fclose(file);
    return result;
}


int atm2text(char *buffer,int length,const struct sockaddr *addr,int flags)
{
    int result;

    if (addr->sa_family != AF_ATMPVC && addr->sa_family != AF_ATMSVC)
	return -1;
    if (!length) return -1;
    if (flags & A2T_NAME) {
	result = try_name(buffer,length,addr);
	if (result == TRY_OTHER && !(flags & A2T_LOCAL))
	    result = ans_byaddr(buffer,length,
	      (const struct sockaddr_atmsvc *) addr,flags);
	if (result == FATAL) return FATAL;
	if (result != TRY_OTHER) return strlen(buffer);
    }
    if (addr->sa_family == AF_ATMPVC)
	return do_pvc(buffer,length,(const struct sockaddr_atmpvc *) addr,
	  flags);
    else return do_svc(buffer,length,(const struct sockaddr_atmsvc *) addr,
	  flags);
    return -1;
}
