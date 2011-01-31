/*
 * mib.c - MIB Primitives
 *
 * Written by Scott W. Shumate
 * 
 * Copyright (c) 1995-97 All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * I ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIM ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "mib.h"
#include "sysgroup.h"
#include "atmf_uni.h"
#include "util.h"
#include "atmd.h"
#include "io.h"

#include <unistd.h>		/* gethostname()	*/
#include <netdb.h>		/* gethostbyname()	*/

#define COMPONENT "MIB"

static Variable variables[] =
/* keep this sorted by the value of entry.name */
{
  { &sysDescr, getString, NULL, NULL, &sysDescrValue },
  { &sysObjectID, getOid, NULL, NULL, &sysObjectIDValue },
  { &sysUpTime, getUpTime, NULL, NULL, NULL },
  { &sysContact, getString, NULL, NULL, &sysContactValue },
  { &sysName, getString, NULL, NULL, &sysNameValue },
  { &sysLocation, getString, NULL, NULL, &sysLocationValue },
  { &sysServices, getInteger, NULL, NULL, &sysServicesValue },
  { &foreQ2931AdminConfigType, getInteger, NULL, NULL,
    &foreQ2931AdminConfigTypeValue },
  { &foreQ2931NNIProto, getInteger, NULL, NULL, &foreQ2931NNIProtoValue },
  { &atmfPortIndex, getInteger, NULL, NULL, &atmfPortIndexValue },
  { &atmfPortMyIdentifier, getInteger, NULL, NULL, &atmfPortIndexValue },
  { &atmfMyIpNmAddress, getIpAddr, NULL, NULL, &atmfMyIpNmAddressValue },
  { &atmfMySystemIdentifier, getString, NULL, NULL, &atmfMySystemIdentifierValue },
  { &atmfAtmLayerMaxVpiBits, getVpiRange, NULL, NULL, &atmfAtmLayerMaxVpiBitsValue },
  { &atmfAtmLayerMaxVciBits, getVciRange, NULL, NULL, &atmfAtmLayerMaxVciBitsValue },
  { &atmfAtmLayerUniType, getInteger, NULL, NULL, &atmfAtmLayerUniTypeValue},
  { &atmfAtmLayerUniVersion, getInteger, NULL, NULL,
     &atmfAtmLayerUniVersionValue },
  { &atmfAtmLayerDeviceType, getInteger, NULL, NULL,
    &atmfAtmLayerDeviceTypeValue},
  { &atmfAtmLayerIlmiVersion, getInteger, NULL, NULL,
    &atmfAtmLayerIlmiVersionValue},
  { &atmfAtmLayerNniSigVersion, getInteger, NULL, NULL,
    &atmfAtmLayerNniSigVersionValue},
  { &atmfNetPrefixStatus, getNetPrefix, getnextNetPrefix, setNetPrefix, NULL },
  { NULL }
};



void MIBget(VarBindList *list, PDUInt *status, AsnInt *offset)
{
  VarBind *varbind;
  Variable *var;
  AsnOidResult result;
  
  *offset = 1;
  FOR_EACH_LIST_ELMT(varbind, list)
  {
    /* Find the first MIB object not lexigraphically less than the *
     * requested OID                                               */
    var = variables;
    while(var->name != NULL)
    {
      result = AsnOidCompare(var->name, &varbind->name);
      if(result != AsnOidLess)
	break;
      var++;
    }

    /* Call get if the requested OID is equal to a simple MIB object */    
    /* OR if the requested OID is a leaf of a complex MIB object */
    if((result == AsnOidEqual && var->getnext == NULL) ||
       (result == AsnOidRoot && var->getnext != NULL))
      *status = var->get(varbind, var);
    /* else the object was not found */
    else
      *status = NOSUCHNAME;
    
    /* Check if the get failed */
    if(*status != NOERROR)
      return;
    
    (*offset)++;
  }
  *offset = 0;
  return;
}
  

void MIBgetnext(VarBindList *list, PDUInt *status, AsnInt *offset)
{
  VarBind *varbind;
  Variable *var;
  AsnOidResult result;

  *offset = 1;
  FOR_EACH_LIST_ELMT(varbind, list)
  {
    /* Find the first complex MIB object not lexigraphically less than *
     * or simple MIB object greater than the requested OID            */
    var = variables;
    while(var->name != NULL)
    {
      result = AsnOidCompare(var->name, &varbind->name);
      if(var->getnext == NULL)
      {
	if(result == AsnOidGreater)
	  break;
      } else if(result != AsnOidLess)
	break;
      var++;
    }

    /* Find the next valid MIB object */
    for(*status = NOSUCHNAME;
	var->name != NULL && *status == NOSUCHNAME;
	var++)
      if(var->getnext == NULL)
      {
	varbind->name.octs = Asn1Alloc(var->name->octetLen);
	AsnOidCopy(&varbind->name, var->name);
	*status = var->get(varbind, var);
      }
      else
	*status = var->getnext(varbind, var);

    /* Check if no valid MIB object found */
    if(*status != NOERROR)
      return;
      
    (*offset)++;
  }
  *offset = 0;
  return;
}

void MIBset(VarBindList *list, PDUInt *status, AsnInt *offset)
{
  VarBind *varbind;
  Variable *var;
  AsnOidResult result;

  *offset = 1;
  FOR_EACH_LIST_ELMT(varbind, list)
  {
    /* Find the first MIB object not lexigraphically less than the *
     * requested OID                                               */
    var = variables;
    while(var->name != NULL)
    {
      result = AsnOidCompare(var->name, &varbind->name);
      if(result != AsnOidLess)
	break;
      var++;
    }

    /* Call set if the requested variable is equal to a simple MIB object */    
    if((result == AsnOidEqual && var->getnext == NULL) ||
    /* OR if the request variable is a leaf of a complex MIB object */
      (result == AsnOidRoot && var->getnext != NULL))
      /* Return read only if no set function exists */
      if(var->set == NULL)
	*status = READONLY;
      else
	*status = var->set(varbind, var);
    else
    /* else the MIB object was not found */
      *status = NOSUCHNAME;

    /* Check if the set failed */
    if(*status != NOERROR)
      return;
      
    (*offset)++;
  }
  *offset = 0;
  return;
}

void *MIBdelete(AsnOid *oid)
{
  Variable *var;
  void *value;
  AsnOidResult result;

  /* Find the first MIB object not lexigraphically less than the *
   * requested variable                                          */
    var = variables;
    while(var->name != NULL)
    {
      result = AsnOidCompare(var->name, oid);
      if(result != AsnOidLess)
	break;
      var++;
    }

  /* Return NULL if the MIB object is not found */
  if(result != AsnOidEqual)
    return NULL;

  value = var->value;
  var->value = NULL;

  return value;
}

AsnInt getString(VarBind *varbind, Variable *var)
{
  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_STRING;
  varbind->value->a.simple->a.string = (AsnOcts*) var->value;
  return NOERROR;
}

AsnInt getOid(VarBind *varbind, Variable *var)
{
  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_OBJECT;
  varbind->value->a.simple->a.object = (AsnOid*) var->value;
  return NOERROR;
}

AsnInt getInteger(VarBind *varbind, Variable *var)
{
  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  varbind->value->a.simple->a.number = *((AsnInt *) var->value);
  return NOERROR;
}

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>


#define MAX_ITFS 500


static const char *itf_types[] = {
    "",					/* fallback */
    "slip*","ppp*","eth*","lec*",	/* candidates */
    "atm*",
    "!lo",				/* blacklist */
    NULL				/* the end */
};


static uint32_t get_local_ip(void)
{
    struct ifconf ifc;
    struct ifreq ifr[MAX_ITFS];
    const char **best;
    uint32_t best_ip;
    int s,i;

    s = socket(PF_INET,SOCK_DGRAM,0);
    if (s < 0) {
	perror("socket");
	return 0;
    }
    ifc.ifc_len = MAX_ITFS*sizeof(struct ifreq);
    ifc.ifc_req = ifr;
    if (ioctl(s,SIOCGIFCONF,&ifc) < 0) {
	perror("SIOCGIFCONF");
	return 0;
    }
    best = itf_types;
    best_ip = 0;
    for (i = 0; i < ifc.ifc_len/sizeof(struct ifreq); i++) {
	struct sockaddr_in *addr;
	const char **walk;
	uint32_t ip;

	addr = (struct sockaddr_in *) &ifr[i].ifr_addr;
	if (addr->sin_family != AF_INET) continue;
	ip = addr->sin_addr.s_addr;
	if (!ip) continue;
	if (ioctl(s,SIOCGIFFLAGS,&ifr[i]) < 0) {
	    perror("SIOCGIFFLAGS");
	    continue;
	}
	if (ifr[i].ifr_flags & IFF_LOOPBACK) continue;
	if (!(ifr[i].ifr_flags & IFF_UP)) continue;
	for (walk = best+1; *walk; walk++) {
	    const char *pos,*wc,*end;
	    int not;

	    if ((not = *(pos = *walk) == '!')) pos++;
	    if (*pos) {
		int len;

		wc = strchr(pos,'*');
		end = wc ? wc : strchr(pos,0);
		len = end-pos;
		if (strncmp(ifr[i].ifr_name,pos,end-pos)) continue;
		if (len < IFNAMSIZ && ifr[i].ifr_name[len])
		    if (!wc || !isdigit(ifr[i].ifr_name[len])) continue;
	    }
	    if (not) break;
	    best = walk;
	    best_ip = ip;
	}
    }
    return best_ip;
}


static uint32_t local_ip = 0;


void set_local_ip(uint32_t ip)
{
    local_ip = ip;
}


AsnInt getIpAddr(VarBind *varbind, Variable *var)
{
  if (!local_ip) {
      local_ip = get_local_ip();
      if (!local_ip) {
	struct hostent* h;
	char hostname[128];

	if (!gethostname(hostname, sizeof(hostname)-1)) {
	    h = (struct hostent*) gethostbyname(hostname);
	    if (!h) local_ip = 0; /* give up :-( */
	    else memcpy(&local_ip,h->h_addr_list[0],4);
	}
      }
  }
  if (!((IpAddress*) var->value)->octs) {
      unsigned char *p;

      ((IpAddress *) var->value)->octs = p = alloc(4);
      memcpy(((IpAddress *) var->value)->octs,&local_ip,4);
      diag(COMPONENT,DIAG_DEBUG,"Local IP address is %d.%d.%d.%d",p[0],p[1],
	p[2],p[3]);
  }
  varbind->value = Asn1Alloc(sizeof(ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_APPLICATION_WIDE;
  varbind->value->a.application_wide = Asn1Alloc(sizeof(ApplicationSyntax));
  varbind->value->a.application_wide->choiceId = APPLICATIONSYNTAX_ADDRESS;
  varbind->value->a.application_wide->a.address = Asn1Alloc(sizeof(NetworkAddress));
  varbind->value->a.application_wide->a.address->choiceId = NETWORKADDRESS_INTERNET;
  varbind->value->a.application_wide->a.address->a.internet = (IpAddress*) var->value;
  return NOERROR;
}


#include <atm.h>
#include <linux/atmdev.h>


static AsnInt put_ci(VarBind *varbind,Variable *var,
  struct atm_cirange *ci,char *bits)
{
  if (get_ci_range(ci) < 0) return GENERR;
  diag(COMPONENT,DIAG_DEBUG,"VPI: %d bits, VCI: %d bits",ci->vpi_bits,
    ci->vci_bits);
  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  varbind->value->a.simple->a.number = *((AsnInt *) var->value) = *bits;
  return NOERROR;
}


AsnInt getVpiRange(VarBind *varbind, Variable *var)
{
  struct atm_cirange ci;

  return put_ci(varbind,var,&ci,&ci.vpi_bits);
}


AsnInt getVciRange(VarBind *varbind, Variable *var)
{
  struct atm_cirange ci;

  return put_ci(varbind,var,&ci,&ci.vci_bits);
}
