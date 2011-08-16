/* $Id: daq_dump.c,v 1.7 2010/10/21 16:15:28 rcombs Exp $*/
/****************************************************************************
 *
 * Copyright (C) 2007-2010 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#include "daq.h"
#include "daq_api.h"

#define DAQ_MOD_VERSION 1

#define DAQ_NAME "dump"
#define DAQ_TYPE (DAQ_TYPE_FILE_CAPABLE | DAQ_TYPE_INTF_CAPABLE | \
                  DAQ_TYPE_INLINE_CAPABLE | DAQ_TYPE_MULTI_INSTANCE)

#define DAQ_DUMP_FILE "inline-out.pcap"

typedef struct {
    // delegate most stuff to daq_pcap
    DAQ_Module_t* module;
    void* handle;

    // but write all output packets here
    pcap_dumper_t* dump;
    char* name;

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
            impl->name = strdup(entry->value);
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
            __FUNCTION__);
        return DAQ_ERROR_NOMEM;
    }
    if ( !mod || !(mod->type & DAQ_TYPE_FILE_CAPABLE) )
    {
        snprintf(errBuf, errMax, "%s: no file capable daq provided", __FUNCTION__);
        free(impl);
        return DAQ_ERROR;
    }

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
    if ( impl->name ) free(impl->name);
    free(impl);
}

//-------------------------------------------------------------------------
// packet processing functions:
// forward all but blocks and blacklists:
static const int s_fwd[MAX_DAQ_VERDICT] = { 1, 0, 1, 1, 0, 1 };

static DAQ_Verdict daq_dump_capture (
    void* user, const DAQ_PktHdr_t* hdr, const uint8_t* pkt)
{
    DumpImpl* impl = (DumpImpl*)user;
    DAQ_Verdict verdict = impl->callback(impl->user, hdr, pkt);

    if ( verdict > MAX_DAQ_VERDICT )
        verdict = DAQ_VERDICT_BLOCK;

    impl->stats.verdicts[verdict]++;

    if ( s_fwd[verdict] )
        pcap_dump((u_char*)impl->dump, (struct pcap_pkthdr*)hdr, pkt);

    return verdict;
}

static int dump_daq_acquire (
    void* handle, int cnt, DAQ_Analysis_Func_t callback, void* user)
{
    DumpImpl* impl = (DumpImpl*)handle;
    impl->callback = callback;
    impl->user = user;
    return impl->module->acquire(impl->handle, cnt, daq_dump_capture, impl);
}

static int dump_daq_inject (
    void* handle, const DAQ_PktHdr_t* hdr, const uint8_t* data, uint32_t len,
    int reverse)
{
    DumpImpl* impl = (DumpImpl*)handle;

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
    impl->stats.packets_injected++;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int dump_daq_start (void* handle)
{
    DumpImpl* impl = (DumpImpl*)handle;
    const char* name = impl->name ? impl->name : DAQ_DUMP_FILE;
    pcap_t* pcap;
    int dlt;
    uint16_t snap;

    int ret = impl->module->start(impl->handle);

    if ( ret )
        return ret;

    dlt = impl->module->get_datalink_type(impl->handle);
    snap = impl->module->get_snaplen(impl->handle);

    pcap = pcap_open_dead(dlt, snap);

    impl->dump = pcap ? pcap_dump_open(pcap, name) : NULL;

    if ( !impl->dump )
    {
        impl->module->stop(impl->handle);
        impl->module->set_errbuf(impl->handle, "can't open dump file");
        return DAQ_ERROR;
    }
    pcap_close(pcap);
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

//-------------------------------------------------------------------------

#ifdef BUILDING_SO
SO_PUBLIC DAQ_Module_t DAQ_MODULE_DATA = 
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
    .get_device_index = dump_daq_get_device_index
};

