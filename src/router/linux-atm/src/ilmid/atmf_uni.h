/*
 * atmf_uni.h - ATM Forum UNI MIB
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

#ifndef ATMF_UNI_H
#define ATMF_UNI_H

#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "mib.h"

extern AsnOid foreQ2931NNIProto;
extern AsnOid foreQ2931AdminConfigType;
extern AsnOid atmfPortIndex;
extern AsnOid atmfPortMyIfName;
extern AsnOid atmfPortMyIdentifier;
extern AsnOid atmfMyIpNmAddress;
extern AsnOid atmfMySystemIdentifier;
extern AsnOid atmfNetPrefixStatus;
extern AsnOid atmfAtmLayerMaxVpiBits;
extern AsnOid atmfAtmLayerMaxVciBits;
extern AsnOid atmfAtmLayerUniType;
extern AsnOid atmfAtmLayerUniVersion;
extern AsnOid atmfAtmLayerDeviceType;
extern AsnOid atmfAtmLayerIlmiVersion;
extern AsnOid atmfAtmLayerNniSigVersion;
extern AsnOid atmfSrvcRegATMAddress;
extern AsnOid atmfSrvcRegTable;
extern AsnOid atmfAddressTable;
extern AsnOid atmfAddressStatus;
extern AsnOid atmfAddressRegistrationAdminStatus;

extern AsnInt foreQ2931NNIProtoValue;
extern AsnInt foreQ2931AdminConfigTypeValue;
extern AsnInt atmfPortIndexValue;
extern IpAddress atmfMyIpNmAddressValue;
extern AsnOcts atmfMySystemIdentifierValue;
extern AsnInt atmfAtmLayerMaxVpiBitsValue;
extern AsnInt atmfAtmLayerMaxVciBitsValue;
extern AsnInt atmfAtmLayerUniTypeValue;
extern AsnInt atmfAtmLayerUniVersionValue;
extern AsnInt atmfAtmLayerDeviceTypeValue;
extern AsnInt atmfAtmLayerIlmiVersionValue;
extern AsnInt atmfAtmLayerNniSigVersionValue;

AsnOid *accessNetPrefix(void);
void deleteNetPrefix(void);
AsnInt getNetPrefix(VarBind *varbind, Variable *var);
AsnInt getnextNetPrefix(VarBind *varbind, Variable *var);
AsnInt setNetPrefix(VarBind *varbind, Variable *var);
AsnInt getMyIp(VarBind *varbind, Variable *var);

AsnOid *accessLecsAddress( void );
AsnInt setLecsAddress(VarBind *varbind, Variable *var);
AsnInt getLecsAddress(VarBind *varbind, Variable *var);

#endif
