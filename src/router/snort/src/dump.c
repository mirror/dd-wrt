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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pcap.h>
#include <sfbpf.h>

#include "snort.h"
#include "dump.h"
#include "util.h"

uint16_t packet_dump_address_space_id;
volatile pcap_dumper_t* packet_dump_file = NULL;
pcap_t* packet_dump_pcap = NULL;
volatile int packet_dump_stop;
struct sfbpf_program packet_dump_fcode;

void PacketDumpClose(void)
{
    if (packet_dump_file)
    {
        pcap_dump_close((pcap_dumper_t*)packet_dump_file);
        packet_dump_file = NULL;
        pcap_close(packet_dump_pcap);
        packet_dump_pcap = NULL;
        sfbpf_freecode(&packet_dump_fcode);
        LogMessage("Stopped packet dump\n");
    }
    packet_dump_stop = 0;
}

static int PacketDumpStop(void)
{
    if (packet_dump_file)
    {
        int i;

        packet_dump_stop = 1;
        for (i = 5; 0 < i; i--)
        {
            if (!packet_dump_stop)
                break;
            sleep(1);
        }
        return packet_dump_stop;
    }
    return 0;
}

int PacketDumpCommand(uint16_t type, const uint8_t *data, uint32_t length, void **new_config,
                      char *statusBuf, int statusBuf_len)
{
    if (!length)
        PacketDumpStop();
    else
    {
        const char* p;
        const char* end;
        const char* fname;
        char fileName[PATH_MAX];
        char *filter;

        if (PacketDumpStop())
        {
            WarningMessage("Failed to stop the previous packet dump\n");
            return 1;
        }

        if (sizeof(packet_dump_address_space_id) > length)
        {
            WarningMessage("Invalid packet dump message length\n");
            return 1;
        }
        packet_dump_address_space_id = *(const uint16_t*)data;
        data += sizeof(packet_dump_address_space_id);
        length -= sizeof(packet_dump_address_space_id);

        fname = (const char*)data;
        end = fname + length;
        for (p = fname; p < end && *p && (isalnum(*p) || *p == '/' || *p == '.' || *p == '-' || *p == '_'); p++);
        if (p >= end || *p)
        {
            WarningMessage("Invalid packet dump file name\n");
            return 1;
        }

        p++;
        if (p >= end || *(end - 1))
        {
            WarningMessage("Invalid packet dump BPF\n");
            return 1;
        }
        filter = strdup(p);
        if (!filter)
        {
            WarningMessage("Couldn't allocate memory for the packet dump filter string\n");
            return 1;
        }
        memset(&packet_dump_fcode, 0, sizeof(packet_dump_fcode));
        if (sfbpf_compile(65535, DLT_EN10MB, &packet_dump_fcode, filter, 1, 0) < 0)
        {
            free(filter);
            WarningMessage("Packet dump BPF state machine compilation failed\n");
            return 1;
        }
        free(filter);
        if (!sfbpf_validate(packet_dump_fcode.bf_insns, packet_dump_fcode.bf_len))
        {
            WarningMessage("Packet dump BPF state machine validation failed\n");
            return 1;
        }

        if (!(packet_dump_pcap = pcap_open_dead(DLT_EN10MB, 65535)))
        {
            WarningMessage("Packet dump failed to open a pcap instance\n");
            return 1;
        }
        snprintf(fileName, sizeof(fileName), "%s.%u", fname, (unsigned)(snort_conf->event_log_id >> 16));
        if (!(packet_dump_file = pcap_dump_open(packet_dump_pcap, fileName)))
        {
            pcap_close(packet_dump_pcap);
            packet_dump_pcap = NULL;
            WarningMessage("Packet dump failed to open the pcap file\n");
            return 1;
        }

        LogMessage("Opened %s for packet dump of address space ID %u with filter (%s)\n",
                   fileName, (unsigned)packet_dump_address_space_id, p);
    }
    return 0;
}


