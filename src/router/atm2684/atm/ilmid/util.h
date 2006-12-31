/*
 * util.h - Various utility procedures
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

#ifndef UTIL_H
#define UTIL_H

#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "atmd.h"

#define AsnOidCopy(o1, o2) ({memcpy((o1)->octs, (o2)->octs, (o1)->octetLen = (o2)->octetLen);})

#define AsnOidAppend(o1, o2) ({memcpy(&(o1)->octs[(o1)->octetLen], (o2)->octs, (o2)->octetLen); \
                              (o1)->octetLen += (o2)->octetLen;})

typedef enum _AsnOidResult {
  AsnOidLess,
  AsnOidRoot,
  AsnOidEqual,
  AsnOidGreater
} AsnOidResult;

AsnOidResult AsnOidCompare(AsnOid *o1, AsnOid *o2);

int AsnOidSize(AsnOid *oid);

VarBind *AppendVarBind(VarBindList *list);

void AppendListNode(VarBindList *list, AsnListNode *node);

#endif
