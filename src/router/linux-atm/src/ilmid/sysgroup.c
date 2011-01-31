/*
 * sysgroup.c - MIB-II system group
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

#include <time.h>
#include "sysgroup.h"
#include "string.h"

AsnOid sysDescr = {8, "\53\06\01\02\01\01\01\00"};
AsnOid sysObjectID = {8, "\53\06\01\02\01\01\02\00"};
AsnOid sysUpTime = {8, "\53\06\01\02\01\01\03\00"};
AsnOid sysContact = {8, "\53\06\01\02\01\01\04\00"};
AsnOid sysName = {8, "\53\06\01\02\01\01\05\00"};
AsnOid sysLocation = {8, "\53\06\01\02\01\01\06\00"};
AsnOid sysServices = {8, "\53\06\01\02\01\01\07\00"};

AsnOcts sysDescrValue = {25, "ATM on Linux Version " VERSION};
AsnOid sysObjectIDValue = {8, "\53\06\01\04\01\03\01\01"};
AsnOcts sysContactValue = {14, "root@localhost"};
AsnOcts sysNameValue = {23, "localhost.my.domain.com"};
AsnOcts sysLocationValue = {36, "Connected to an ATM switch somewhere"};
AsnInt sysServicesValue = 4;

static time_t up_time;


void resetUpTime(void)
{
  up_time = time(0);
}

TimeTicks accessUpTime(void)
{
  /* Return time in hundreds of seconds */
  return (TimeTicks) ((time(0) - up_time) * 100);
}

AsnInt getUpTime(VarBind *varbind, Variable *var)
{
  varbind->value = Asn1Alloc(sizeof(struct ObjectSyntax));
  varbind->value->choiceId = OBJECTSYNTAX_SIMPLE;
  varbind->value->a.simple = Asn1Alloc(sizeof(struct SimpleSyntax));
  varbind->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  varbind->value->a.simple->a.number = (AsnInt) accessUpTime();
  return NOERROR;
}




