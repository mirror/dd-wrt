/*
 * mib.h - MIB Primitives
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

#ifndef MIB_H
#define MIB_H

#include "atmd.h"

#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"

typedef struct Variable
{
  AsnOid *name;
  AsnInt (*get)(VarBind *varbind, struct Variable *var);
  AsnInt (*getnext)(VarBind *varbind, struct Variable *var);
  AsnInt (*set)(VarBind *varbind, struct Variable *var);
  void *value;
} Variable;

void MIBget(VarBindList *list, PDUInt *status, AsnInt *index);
void MIBgetnext(VarBindList *list, PDUInt *status, AsnInt *index);
void MIBset(VarBindList *list, PDUInt *status, AsnInt *index);
void *MIBdelete(AsnOid *oid);

AsnInt getString(VarBind *varbind, Variable *var);
AsnInt getOid(VarBind *varbind, Variable *var);
AsnInt getInteger(VarBind *varbind, Variable *var);
AsnInt getIpAddr(VarBind *varbind, Variable *var);
AsnInt getVpiRange(VarBind *varbind, Variable *var);
AsnInt getVciRange(VarBind *varbind, Variable *var);

void set_local_ip(uint32_t ip);

#endif
