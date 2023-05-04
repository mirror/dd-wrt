/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Author(s):   Ron Dempster <rdempste@cisco.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef __DUMP_H__
#define __DUMP_H__

#include <stdint.h>
#include <pcap.h>
#include <sfbpf.h>

extern uint16_t packet_dump_address_space_id;
extern volatile pcap_dumper_t* packet_dump_file;
extern pcap_t* packet_dump_pcap;
extern volatile int packet_dump_stop;
extern struct sfbpf_program packet_dump_fcode;

void PacketDumpClose(void);
int PacketDumpCommand(uint16_t type, const uint8_t *data, uint32_t length, void **new_config,
                      char *statusBuf, int statusBuf_len);

#endif /* __DUMP_H__ */
