/*
 * atmf_uni.c - ATM Forum UNI MIB
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

#include "atmf_uni.h"
#include "atmd.h"
#include "util.h"

#define COMPONENT "NETPREFIX"
#define NETPREFIX_LEN 12
#define NETPREFIX_STRINGLEN '\15'
#define INDEX_LEN 15
#define LOCALUNI '\0'
#define VALID 1
#define INVALID 2

AsnOid atmfPortIndex =          {13, "\53\06\01\04\01\202\141\02\01\01\01\01\00"};
AsnOid atmfPortMyIdentifier =   {13, "\53\06\01\04\01\202\141\02\01\01\01\10\00"};
AsnOid atmfMyIpNmAddress =      {11, "\53\06\01\04\01\202\141\02\01\02\00"};
AsnOid atmfMySystemIdentifier = {11, "\53\06\01\04\01\202\141\02\01\04\00"};
AsnOid atmfAtmLayerMaxVpiBits = {13, "\53\06\01\04\01\202\141\02\02\01\01\6\00"};
AsnOid atmfAtmLayerMaxVciBits = {13, "\53\06\01\04\01\202\141\02\02\01\01\7\00"};
AsnOid atmfAtmLayerUniVersion = {13, "\53\06\01\04\01\202\141\02\02\01\01\11\00"};
AsnOid atmfNetPrefixStatus = {NETPREFIX_LEN, "\53\06\01\04\01\202\141\02\07\01\01\03"};

AsnInt atmfPortIndexValue = 0;
AsnOcts atmfMySystemIdentifierValue = {6 , NULL};
IpAddress atmfMyIpNmAddressValue = {4 , NULL};

/* The following two values depend on the capabilities of both
   the switch AND the adapter - DO THEY ?? */
AsnInt atmfAtmLayerMaxVpiBitsValue;
AsnInt atmfAtmLayerMaxVciBitsValue;


#if defined(UNI30) || defined(DYNAMIC_UNI)
AsnInt atmfAtmLayerUniVersionValue = 2;		// version3point0(2)
#else
#ifdef UNI31
AsnInt atmfAtmLayerUniVersionValue = 3;		// version3point1(3)
#else
AsnInt atmfAtmLayerUniVersionValue = 4;		// version4point0(4)
#endif
#endif

static AsnOid atmNetPrefix = {0, NULL};

typedef struct NetPrefixNode
{
  AsnOid *name;
  struct NetPrefixNode *prev;
  struct NetPrefixNode *next;
} NetPrefixNode;

AsnOid *accessNetPrefix(void)
{
  if(atmNetPrefix.octs == NULL)
    return NULL;

  return(&atmNetPrefix);
}

void deleteNetPrefix(void)
{
  NetPrefixNode *prefix, *nextPrefix;

  for(prefix = (NetPrefixNode *) MIBdelete(&atmfNetPrefixStatus);
      prefix != NULL;
      prefix = nextPrefix)
    {
      nextPrefix = prefix->next;
      free(prefix->name->octs);
      free(prefix->name);
      free(prefix);
    }

  if(atmNetPrefix.octs != NULL)
    {
      free(atmNetPrefix.octs);
      atmNetPrefix.octs = NULL;
    }
}

AsnInt getNetPrefix(VarBind *varbind, Variable *var)
{
  int cmp;
  NetPrefixNode *prefix;
  AsnOid *varBindName;

  varBindName = &varbind->name;
  if(AsnOidSize(varBindName) != NETPREFIX_LEN + INDEX_LEN ||
     varbind->name.octs[NETPREFIX_LEN] != LOCALUNI ||
     varbind->name.octs[NETPREFIX_LEN + 1] != NETPREFIX_STRINGLEN)
    return NOSUCHNAME;

  for(prefix = (NetPrefixNode *) var->value, cmp = AsnOidLess;
      prefix != NULL && (cmp = AsnOidCompare(varBindName, prefix->name)) <
	AsnOidEqual;
      prefix = prefix->next);

  if(cmp != AsnOidEqual)
    return NOSUCHNAME;

  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  varbind->value->a.simple->a.number = VALID;
  return NOERROR;
}


AsnInt getnextNetPrefix(VarBind *varbind, Variable *var)
{
  NetPrefixNode *prefix;
  AsnOid *varBindName;

  varBindName = &varbind->name;
  for(prefix = (NetPrefixNode *) var->value;
      prefix != NULL && AsnOidCompare(prefix->name, varBindName) != AsnOidGreater;
      prefix = prefix->next);

  if(prefix == NULL)
    return NOSUCHNAME;

  varbind->name.octs = Asn1Alloc(prefix->name->octetLen);
  AsnOidCopy(varBindName, prefix->name);

  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  varbind->value->a.simple->a.number = VALID;

  return NOERROR;
}

AsnInt setNetPrefix(VarBind *varbind, Variable *var)
{
  int cmp;
  NetPrefixNode *prefix, *newPrefix;
  AsnOid *varBindName;

  varBindName = &varbind->name;
  if(varbind->value->choiceId != OBJECTSYNTAX_SIMPLE ||
     varbind->value->a.simple->choiceId != SIMPLESYNTAX_NUMBER ||
     (varbind->value->a.simple->a.number != VALID && 
      varbind->value->a.simple->a.number != INVALID))
    return BADVALUE;
      
  if(AsnOidSize(varBindName) != NETPREFIX_LEN + INDEX_LEN ||
     varBindName->octs[NETPREFIX_LEN] != LOCALUNI || 
     varBindName->octs[NETPREFIX_LEN + 1] != NETPREFIX_STRINGLEN)
    return NOSUCHNAME;

  for(prefix = (NetPrefixNode *) var->value, cmp = AsnOidLess;
      prefix != NULL && (cmp = AsnOidCompare(varBindName, prefix->name)) <
	AsnOidEqual;
      prefix = prefix->next);

  if(varbind->value->a.simple->a.number == VALID && cmp != AsnOidEqual)
    {
      newPrefix = alloc_t(NetPrefixNode);
      newPrefix->name = alloc_t(AsnOid);
      newPrefix->name->octs = alloc(varBindName->octetLen);
      AsnOidCopy(newPrefix->name, varBindName);
      Q_INSERT_BEFORE((NetPrefixNode *) var->value, newPrefix, prefix);
      if(atmNetPrefix.octs == NULL)
	{
	  atmNetPrefix.octetLen = varBindName->octetLen - NETPREFIX_LEN - 2;
	  atmNetPrefix.octs = alloc(atmNetPrefix.octetLen);
	  memcpy(atmNetPrefix.octs, &varBindName->octs[NETPREFIX_LEN + 2], atmNetPrefix.octetLen);
	}
    }
  else if (varbind->value->a.simple->a.number == INVALID && cmp == AsnOidEqual)
    {
      Q_REMOVE((NetPrefixNode *) var->value, prefix);
    }

  return NOERROR;
}
