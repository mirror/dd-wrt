/* qos2text.c - Converts binary encoding of QOS parameters to textual
		representation */

/* Written 1996-2000 by Werner Almesberger, EPFL-LRC */


#include <stdio.h>
#include <string.h>

#include "atm.h"


#define FLD(F) \
    if (curr->F && ((ref && (ref->F != curr->F || ref->traffic_class == \
      ATM_NONE)) || (comp && comp->F == curr->F && comp->traffic_class != \
      ATM_NONE))) { \
	if (buffer != *pos && !strchr(":,",(*pos)[-1])) *(*pos)++ = ','; \
	if (curr->F != ATM_MAX_PCR) *pos += sprintf(*pos,#F "=%d",curr->F); \
	else { \
	    strcat(*pos,#F "=max"); \
	    *pos += strlen(#F)+4; \
	} \
    }


static void params(char *buffer,char **pos,const struct atm_trafprm *ref,
  const struct atm_trafprm *curr,const struct atm_trafprm *comp)
{
    FLD(max_pcr);
    FLD(pcr);
    FLD(min_pcr);
    FLD(max_sdu);
}


static void opt(const char *prefix,char *buffer,char **pos,
  const struct atm_trafprm *ref,const struct atm_trafprm *curr,
  const struct atm_trafprm *comp)
{
    char *start;

    if (curr->traffic_class == ATM_NONE) {
	if (!comp) *pos += sprintf(*pos,"%snone",prefix);
	return;
    }
    start = *pos;
    params(buffer,pos,ref,curr,comp);
    if (start == *pos) return;
    *pos = start;
    strcpy(*pos,prefix);
    *pos += strlen(prefix);
    params(buffer,pos,ref,curr,comp);
}


int qos2text(char *buffer,int length,const struct atm_qos *qos,int flags)
{
    char *pos,*start;

    if (length <= MAX_ATM_QOS_LEN) return -1;
    *(pos = buffer) = 0;
    switch (qos->txtp.traffic_class == ATM_NONE ? qos->rxtp.traffic_class :
       qos->txtp.traffic_class) {
	case ATM_UBR:
	    strcpy(buffer,"ubr");
	    pos += 3;
	    break;
	case ATM_CBR:
	    strcpy(buffer,"cbr");
	    pos += 3;
	    break;
	case ATM_ABR:
	    strcpy(buffer,"abr");
	    pos += 3;
	    break;
	default:
	    return -1;
    }
    if (qos->aal != ATM_NO_AAL) {
	strcpy(pos,",");
	pos++;
    }
    switch (qos->aal) {
	case ATM_NO_AAL:
	    break;
	case ATM_AAL0:
	    strcpy(pos,"aal0");
	    pos += 4;
	    break;
	case ATM_AAL5:
	    strcpy(pos,"aal5");
	    pos += 4;
	    break;
	default:
	    return -1;
    }
    pos++;
    start = pos;
    if (qos->txtp.traffic_class != ATM_NONE && qos->rxtp.traffic_class !=
      ATM_NONE) params(buffer,&pos,NULL,&qos->txtp,&qos->rxtp);
    opt(start == pos ? "tx:" : ",tx:",buffer,&pos,&qos->rxtp,&qos->txtp,NULL);
    opt(start == pos ? "rx:" : ",rx:",buffer,&pos,&qos->txtp,&qos->rxtp,NULL);
    if (pos != start) start[-1] = ':';
    return 0;
}
