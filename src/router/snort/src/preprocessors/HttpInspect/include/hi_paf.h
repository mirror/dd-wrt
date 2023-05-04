/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

//--------------------------------------------------------------------
// hi stuff
//
// @file    hi_paf.h
// @author  Russ Combs <rcombs@sourcefire.com>
//--------------------------------------------------------------------

#ifndef __HI_PAF_H__
#define __HI_PAF_H__

#include "sfPolicy.h"
#include "sf_types.h"

bool hi_paf_init(uint32_t cap);
void hi_paf_term(void);

int hi_paf_register_port(struct _SnortConfig *sc, uint16_t port, bool client, bool server, tSfPolicyId pid, bool auto_on);
int hi_paf_register_service(struct _SnortConfig *, uint16_t service, bool client, bool server, tSfPolicyId pid, bool auto_on);

bool hi_paf_simple_request(void* ssn);
bool hi_paf_resp_eoh(void* ssn);
uint32_t hi_paf_resp_bytes_processed(void* ssn);
bool hi_paf_disable_te(void *ssn, bool to_server);
bool hi_paf_valid_http(void* ssn);
uint32_t hi_paf_get_size();

#endif

