/*
 * sysgroup.h - MIB-II system group
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

#ifndef SYSGROUP_H
#define SYSGROUP_H

#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "mib.h"

extern AsnOid sysDescr;
extern AsnOid sysObjectID;
extern AsnOid sysUpTime;
extern AsnOid sysContact;
extern AsnOid sysName;
extern AsnOid sysLocation;
extern AsnOid sysServices;

extern AsnOcts sysDescrValue;
extern AsnOid sysObjectIDValue;
extern AsnOcts sysContactValue;
extern AsnOcts sysNameValue;
extern AsnOcts sysLocationValue;
extern AsnInt sysServicesValue;

AsnInt getString(VarBind *varbind, Variable *var);
AsnInt getOid(VarBind *varbind, Variable *var);
void resetUpTime(void);
TimeTicks accessUpTime(void);
AsnInt getUpTime(VarBind *varbind, Variable *var);
AsnInt getInteger(VarBind *varbind, Variable *var);

#endif
