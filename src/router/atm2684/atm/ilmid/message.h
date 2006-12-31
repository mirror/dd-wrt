/*
 * message.h - commonly used ilmi messages
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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"

#define ADDRESS_LEN 13
#define ADDRESS_OID "\53\06\01\04\01\202\141\02\06\01\01\03\0"

extern int no_var_bindings;

AsnOid atmAddressStatus;

Message *create_poll_message(void);
Message *create_set_message(void);
Message *create_coldstart_message(void);

#endif
