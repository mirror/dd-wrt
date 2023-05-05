/****************************************************************************
 *
 * Copyright (C) 2007-2013 Sourcefire, Inc.
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/socket.h>
#endif

#include "daq.h"
#include "daq_api.h"

#define DAQ_MOD_VERSION 4

#define DAQ_NAME "dump"
#define DAQ_TYPE (DAQ_TYPE_FILE_CAPABLE | DAQ_TYPE_INTF_CAPABLE | \
                  DAQ_TYPE_INLINE_CAPABLE | DAQ_TYPE_MULTI_INSTANCE)

#define DAQ_DUMP_PCAP_FILE "inline-out.pcap"
#define DAQ_DUMP_TEXT_FILE "inline-out.txt"

typedef enum {
    DUMP_OUTPUT_NONE = 0x0,
    DUMP_OUTPUT_PCAP = 0x1,
    DUMP_OUTPUT_TEXT = 0x2,
    DUMP_OUTPUT_BOTH = 0x3
} DumpOutputType;

typedef struct {
    // delegate most stuff to daq_pcap
    DAQ_Module_t* module;
    void* handle;

    // but write all output packets here
    pcap_dumper_t* dump;
    char* pcap_filename;

    // and write other textual output here
    FILE *text_out;
    char* text_filename;

    DumpOutputType output_type;

    // by linking in with these
    DAQ_Analysis_Func_t callback;
    void* user;

    DAQ_Stats_t stats;
} DumpImpl;

static int dump_daq_stop(void*);

static int daq_dump_get_vars (
    DumpImpl* impl, DAQ_Config_t* cfg, char* errBuf, size_t errMax
) {
    const char* s = NULL;
    DAQ_Dict* entry;

    for ( entry = cfg->values; entry; entry = entry->next)
    {
        if ( !strcmp(entry->key, "load-mode") )
        {
            s = entry->value;
        }
        else if ( !strcmp(entry->key, "file") )
        {
            impl->pcap_filename = strdup(entry->value);
        }
        else if ( !strcmp(entry->key, "text-file") )
        {
            impl->text_filename = strdup(entry->value);
        }
        else if ( !strcmp(entry->key, "output") )
        {
            if ( !strcmp(entry->value, "none") )
                impl->output_type = DUMP_OUTPUT_NONE;
            else if ( !strcmp(entry->value, "pcap") )
                impl->output_type = DUMP_OUTPUT_PCAP;
            else if ( !strcmp(entry->value, "text") )
                impl->output_type = DUMP_OUTPUT_TEXT;
            else if ( !strcmp(entry->value, "both") )
                impl->output_type = DUMP_OUTPUT_BOTH;
            else
            {
                snprintf(errBuf, errMax, "invalid output type (%s)", entry->value);
            }
        }
    }
    if ( !s )
        return 1;

    if ( !strcasecmp(s, "read-file") )
    {
        cfg->mode = DAQ_MODE_READ_FILE;
        return 1;
    }
    else if ( !strcasecmp(s, "passive") )
    {
        cfg->mode = DAQ_MODE_PASSIVE;
        return 1;
    }
    else if ( !strcasecmp(s, "inline") )
    {
        cfg->mode = DAQ_MODE_INLINE;
        return 1;
    }
    snprintf(errBuf, errMax, "invalid load-mode (%s)", s);
    return 0;
}

static void hexdump(FILE *fp, const uint8_t *data, unsigned int len, const char *prefix)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {
        if (i % 16 == 0)
            fprintf(fp, "\n%s", prefix ? prefix : "");
        else if (i % 2 == 0)
            fprintf(fp, " ");
        fprintf(fp, "%02x", data[i]);
    }
    fprintf(fp, "\n");
}

//-------------------------------------------------------------------------
// constructor / destructor

static int dump_daq_initialize (
    const DAQ_Config_t* cfg, void** handle, char* errBuf, size_t errMax)
{
    DumpImpl* impl;
    impl = calloc(1, sizeof(*impl));
    DAQ_Module_t* mod = (DAQ_Module_t*)cfg->extra;
    DAQ_Config_t sub_cfg = *cfg;
    int err;

    if ( !impl )
    {
        snprintf(errBuf, errMax,
            "%s: Couldn't allocate memory for the DAQ context",
            __func__);
        return DAQ_ERROR_NOMEM;
    }
    if ( !mod || !(mod->type & DAQ_TYPE_FILE_CAPABLE) )
    {
        snprintf(errBuf, errMax, "%s: no file capable daq provided", __func__);
        free(impl);
        return DAQ_ERROR;
    }

    impl->output_type = DUMP_OUTPUT_PCAP;
    if ( !daq_dump_get_vars(impl, &sub_cfg, errBuf, errMax) )
    {
        free(impl);
        return DAQ_ERROR;
    }
    err = mod->initialize(&sub_cfg, &impl->handle, errBuf, errMax);

    if ( err )
    {
        free(impl);
        return err;
    }
    impl->module = mod;
    *handle = impl;

    return DAQ_SUCCESS;
}

static void dump_daq_shutdown (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    impl->module->shutdown(impl->handle);
    if ( impl->pcap_filename )
        free(impl->pcap_filename);
    if ( impl->text_filename )
        free(impl->text_filename);
    free(impl);
}

//-------------------------------------------------------------------------
// packet processing functions:
// forward all but blocks, retries and blacklists:
static const int s_fwd[MAX_DAQ_VERDICT] = { 1, 0, 1, 1, 0, 1, 0 };
// We don't have access to daq_verdict_string() because we're not linking
// against LibDAQ, so pack our own copy.
static const char *daq_verdict_strings[MAX_DAQ_VERDICT] = {
    "Pass",         // DAQ_VERDICT_PASS
    "Block",        // DAQ_VERDICT_BLOCK
    "Replace",      // DAQ_VERDICT_REPLACE
    "Whitelist",    // DAQ_VERDICT_WHITELIST
    "Blacklist",    // DAQ_VERDICT_BLACKLIST
    "Ignore",       // DAQ_VERDICT_IGNORE
    "Retry"         // DAQ_VERDICT_RETRY
};

static DAQ_Verdict daq_dump_capture (
    void* user, const DAQ_PktHdr_t* hdr, const uint8_t* pkt)
{
    DumpImpl* impl = (DumpImpl*)user;
    DAQ_Verdict verdict = impl->callback(impl->user, hdr, pkt);

    if ( verdict >= MAX_DAQ_VERDICT )
        verdict = DAQ_VERDICT_BLOCK;

    impl->stats.verdicts[verdict]++;

    if ( impl->dump && s_fwd[verdict] )
        pcap_dump((u_char*)impl->dump, (struct pcap_pkthdr*)hdr, pkt);

    if (impl->text_out)
    {
        fprintf(impl->text_out, "PV: %lu.%lu(%u): %s\n", (unsigned long) hdr->ts.tv_sec,
                (unsigned long) hdr->ts.tv_usec, hdr->caplen, daq_verdict_strings[verdict]);
        if (verdict == DAQ_VERDICT_REPLACE)
            hexdump(impl->text_out, pkt, hdr->caplen, "    ");
    }

    return verdict;
}

static int dump_daq_acquire (
    void* handle, int cnt, DAQ_Analysis_Func_t callback, DAQ_Meta_Func_t metaback, void* user)
{
    DumpImpl* impl = (DumpImpl*)handle;
    impl->callback = callback;
    impl->user = user;
    return impl->module->acquire(impl->handle, cnt, daq_dump_capture, metaback, impl);
}

static int dump_daq_inject (
    void* handle, const DAQ_PktHdr_t* hdr, const uint8_t* data, uint32_t len,
    int reverse)
{
    DumpImpl* impl = (DumpImpl*)handle;

    if (impl->text_out)
    {
        fprintf(impl->text_out, "%cI: %lu.%lu(%u): %u\n", reverse ? 'R' : 'F',
                (unsigned long) hdr->ts.tv_sec, (unsigned long) hdr->ts.tv_usec, hdr->caplen, len);
        hexdump(impl->text_out, data, len, "    ");
        fprintf(impl->text_out, "\n");
    }

    if (impl->dump)
    {
        // copy the original header to get the same
        // timestamps but overwrite the lengths
        DAQ_PktHdr_t h = *hdr;
        h.pktlen = h.caplen = len;
        pcap_dump((u_char*)impl->dump, (struct pcap_pkthdr*)&h, data);
        if ( ferror(pcap_dump_file(impl->dump)) )
        {
            impl->module->set_errbuf(impl->handle, "inject can't write to dump file");
            return DAQ_ERROR;
        }
    }

    impl->stats.packets_injected++;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int dump_daq_start (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    int dlt;
    int snap;

    int ret = impl->module->start(impl->handle);

    if ( ret )
        return ret;

    dlt = impl->module->get_datalink_type(impl->handle);
    snap = impl->module->get_snaplen(impl->handle);

    if ( impl->output_type & DUMP_OUTPUT_PCAP )
    {
        const char* pcap_filename = impl->pcap_filename ? impl->pcap_filename : DAQ_DUMP_PCAP_FILE;
        pcap_t* pcap;

        pcap = pcap_open_dead(dlt, snap);
        impl->dump = pcap ? pcap_dump_open(pcap, pcap_filename) : NULL;
        if ( !impl->dump )
        {
            impl->module->stop(impl->handle);
            impl->module->set_errbuf(impl->handle, "can't open dump file");
            return DAQ_ERROR;
        }
        pcap_close(pcap);
    }

    if ( impl->output_type & DUMP_OUTPUT_TEXT )
    {
        const char* text_filename = impl->text_filename ? impl->text_filename : DAQ_DUMP_TEXT_FILE;

        impl->text_out = fopen(text_filename, "w");
        if ( !impl->text_out )
        {
            impl->module->stop(impl->handle);
            impl->module->set_errbuf(impl->handle, "can't open text output file");
            return DAQ_ERROR;
        }
    }

    return DAQ_SUCCESS;
}

static int dump_daq_stop (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    int err = impl->module->stop(impl->handle);

    if ( err )
        return err;

    if ( impl->dump )
    {
        pcap_dump_close(impl->dump);
        impl->dump = NULL;
    }

    if ( impl->text_out )
    {
        fclose(impl->text_out);
        impl->text_out = NULL;
    }

    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------
// these methods are delegated to the pcap daq

static int dump_daq_set_filter (void* handle, const char* filter)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->set_filter(impl->handle, filter);
}

static int dump_daq_breakloop (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->breakloop(impl->handle);
}

static DAQ_State dump_daq_check_status (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->check_status(impl->handle);
}

static int dump_daq_get_stats (void* handle, DAQ_Stats_t* stats)
{
    DumpImpl* impl = (DumpImpl*)handle;
    int ret = impl->module->get_stats(impl->handle, stats);
    int i;

    for ( i = 0; i < MAX_DAQ_VERDICT; i++ )
        stats->verdicts[i] = impl->stats.verdicts[i];

    stats->packets_injected = impl->stats.packets_injected;
    return ret;
}

static void dump_daq_reset_stats (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    impl->module->reset_stats(impl->handle);
    memset(&impl->stats, 0, sizeof(impl->stats));
}

static int dump_daq_get_snaplen (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->get_snaplen(impl->handle);
}

static uint32_t dump_daq_get_capabilities (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    uint32_t caps = impl->module->get_capabilities(impl->handle);
    caps |= DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT;
    return caps;
}

static int dump_daq_get_datalink_type (void *handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->get_datalink_type(impl->handle);
}

static const char* dump_daq_get_errbuf (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->get_errbuf(impl->handle);
}

static void dump_daq_set_errbuf (void* handle, const char* s)
{
    DumpImpl* impl = (DumpImpl*)handle;
    impl->module->set_errbuf(impl->handle, s ? s : "");
}

static int dump_daq_get_device_index(void* handle, const char* device)
{
    DumpImpl* impl = (DumpImpl*)handle;
    return impl->module->get_device_index(impl->handle, device);
}

static int dump_daq_modify_flow(void *handle, const DAQ_PktHdr_t *hdr, const DAQ_ModFlow_t *modify)
{
    DumpImpl* impl = (DumpImpl*)handle;

    if (impl->text_out)
    {
        fprintf(impl->text_out, "MF: %lu.%lu(%u): %d %u \n", (unsigned long) hdr->ts.tv_sec,
                (unsigned long) hdr->ts.tv_usec, hdr->caplen, modify->type, modify->length);
        hexdump(impl->text_out, modify->value, modify->length, "    ");
    }
    return DAQ_SUCCESS;
}

static int dump_daq_dp_add_dc(void *handle, const DAQ_PktHdr_t *hdr, DAQ_DP_key_t *dp_key,
                                const uint8_t *packet_data, DAQ_Data_Channel_Params_t *params)
{
    DumpImpl* impl = (DumpImpl*)handle;

    if (impl->text_out)
    {
        char src_addr_str[INET6_ADDRSTRLEN], dst_addr_str[INET6_ADDRSTRLEN];

        fprintf(impl->text_out, "DP: %lu.%lu(%u):\n", (unsigned long) hdr->ts.tv_sec,
                (unsigned long) hdr->ts.tv_usec, hdr->caplen);
        if (dp_key->src_af == AF_INET)
            inet_ntop(AF_INET, &dp_key->sa.src_ip4, src_addr_str, sizeof(src_addr_str));
        else
            inet_ntop(AF_INET6, &dp_key->sa.src_ip6, src_addr_str, sizeof(src_addr_str));
        if (dp_key->dst_af == AF_INET)
            inet_ntop(AF_INET, &dp_key->da.dst_ip4, dst_addr_str, sizeof(dst_addr_str));
        else
            inet_ntop(AF_INET6, &dp_key->da.dst_ip6, dst_addr_str, sizeof(dst_addr_str));
        fprintf(impl->text_out, "    %s:%hu -> %s:%hu (%hhu)\n", src_addr_str, dp_key->src_port,
                dst_addr_str, dp_key->dst_port, dp_key->protocol);
        fprintf(impl->text_out, "    %hu %hu %hu %hu 0x%X %u\n", dp_key->address_space_id, dp_key->tunnel_type,
                dp_key->vlan_id, dp_key->vlan_cnots, params ? params->flags : 0, params ? params->timeout_ms : 0);
    }
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

#ifdef BUILDING_SO
DAQ_SO_PUBLIC DAQ_Module_t DAQ_MODULE_DATA =
#else
DAQ_Module_t dump_daq_module_data =
#endif
{
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_MOD_VERSION,
    .name = DAQ_NAME,
    .type = DAQ_TYPE,
    .initialize = dump_daq_initialize,
    .set_filter = dump_daq_set_filter,
    .start = dump_daq_start,
    .acquire = dump_daq_acquire,
    .inject = dump_daq_inject,
    .breakloop = dump_daq_breakloop,
    .stop = dump_daq_stop,
    .shutdown = dump_daq_shutdown,
    .check_status = dump_daq_check_status,
    .get_stats = dump_daq_get_stats,
    .reset_stats = dump_daq_reset_stats,
    .get_snaplen = dump_daq_get_snaplen,
    .get_capabilities = dump_daq_get_capabilities,
    .get_datalink_type = dump_daq_get_datalink_type,
    .get_errbuf = dump_daq_get_errbuf,
    .set_errbuf = dump_daq_set_errbuf,
    .get_device_index = dump_daq_get_device_index,
    .modify_flow = dump_daq_modify_flow,
    .hup_prep = NULL,
    .hup_apply = NULL,
    .hup_post = NULL,
    .dp_add_dc = dump_daq_dp_add_dc,
    .query_flow = NULL,
};

