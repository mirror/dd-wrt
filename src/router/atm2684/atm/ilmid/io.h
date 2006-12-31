/*
 * io.h - Ilmi input/output routines
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
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

#ifndef IO_H
#define IO_H

#include <sys/time.h>
#include <atm.h>
#include <linux/atmdev.h>
#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"

#define MAX_ILMI_MSG 484

AsnOid *get_esi(int fd, int itf);
void update_nsap(int itf, AsnOid *netprefix, AsnOid *esi);
int wait_for_message(int fd, struct timeval *timeout);
int read_message(int fd, Message *message);
int send_message(int fd, Message *message);
int open_ilmi(int itf,const char *qos_spec);
int get_ci_range(struct atm_cirange *ci);

#endif
