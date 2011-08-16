/* $Id: daq_ipfw.c,v 1.13 2010/12/08 16:03:09 rcombs Exp $ */
/*
 ** Portions Copyright (C) 1998-2010 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include "daq_api.h"
#include "sfbpf.h"

#ifndef IPPROTO_DIVERT
#define IPPROTO_DIVERT 254
#endif

#define DAQ_MOD_VERSION 2

#define DAQ_NAME "ipfw"
#define DAQ_TYPE (DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_INLINE_CAPABLE | \
                  DAQ_TYPE_MULTI_INSTANCE)

typedef struct {
    int sock;
    int port;
    int proto;
    int count;
    int passive;

    unsigned timeout;
    unsigned snaplen;

    uint8_t* buf;
    char* filter;
    char error[DAQ_ERRBUF_SIZE];

    struct sfbpf_program fcode;
    struct sockaddr_in sin;

    DAQ_State state;
    DAQ_Stats_t stats;
} IpfwImpl;

static int ipfw_daq_stop(void*);
static void ipfw_daq_shutdown(void*);

//-------------------------------------------------------------------------
// utilities

#define DEFAULT_PORT 8000

static int ipfw_daq_get_setup (
    IpfwImpl* impl, const DAQ_Config_t* cfg, char* errBuf, size_t errMax)
{
    DAQ_Dict* entry;

    impl->proto = PF_INET;  // TBD add ip6 support when ipfw does
    impl->port = DEFAULT_PORT;

    for ( entry = cfg->values; entry; entry = entry->next)
    {
        if ( !entry->value || !*entry->value )
        {
            snprintf(errBuf, errMax,
                "%s: variable needs value (%s)\n", __FUNCTION__, entry->key);
                return DAQ_ERROR;
        }
        else if ( !strcmp(entry->key, "port") )
        {
            char* end = entry->value;
            impl->port = (int)strtol(entry->value, &end, 0);

            if ( *end || impl->port <= 0 || impl->port > 65535 )
            {
                snprintf(errBuf, errMax, "%s: bad port (%s)\n",
                    __FUNCTION__, entry->value);
                return DAQ_ERROR;
            }
        }
        else
        {
            snprintf(errBuf, errMax,
                "%s: unsupported variable (%s=%s)\n",
                    __FUNCTION__, entry->key, entry->value);
                return DAQ_ERROR;
        }
    }

    impl->snaplen = cfg->snaplen ? cfg->snaplen : IP_MAXPACKET;
    impl->timeout = cfg->timeout;
    impl->passive = ( cfg->mode == DAQ_MODE_PASSIVE );

    impl->sin.sin_family = impl->proto;
    impl->sin.sin_addr.s_addr = INADDR_ANY;
    impl->sin.sin_port = htons(impl->port);

    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------
// initialization and clean up

static int ipfw_daq_initialize (
    const DAQ_Config_t* cfg, void** handle, char* errBuf, size_t errMax)
{
    IpfwImpl* impl = calloc(1, sizeof(*impl));

    if ( !impl )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate the ipfw context!",
            __FUNCTION__);
        return DAQ_ERROR_NOMEM;
    }

    if ( ipfw_daq_get_setup(impl, cfg, errBuf, errMax) != DAQ_SUCCESS )
    {
        ipfw_daq_shutdown(impl);
        return DAQ_ERROR;
    }
    impl->buf = malloc(impl->snaplen);

    if ( !impl->buf )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate the ipfw buffer!",
            __FUNCTION__);
        ipfw_daq_shutdown(impl);
        return DAQ_ERROR_NOMEM;
    }

    impl->sock = -1;
    impl->state = DAQ_STATE_INITIALIZED;

    *handle = impl;
    return DAQ_SUCCESS;
}

static void ipfw_daq_shutdown (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;

    if ( impl->filter )
        free(impl->filter);

    if ( impl->buf )
        free(impl->buf);

    free(impl);
}

//-------------------------------------------------------------------------

static int ipfw_daq_set_filter (void* handle, const char* filter)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    struct sfbpf_program fcode;

    if ( sfbpf_compile(impl->snaplen, DLT_IPV4, &fcode, filter, 1, 0) < 0 )
    {
        // FIXTHIS is errno set?
        // XICHE: No, which is why relying on strerror for everything is bad.
        return DAQ_ERROR;
    }

    if ( impl->filter )
        free((void *)impl->filter);

    if ( impl->fcode.bf_insns )
        free(impl->fcode.bf_insns);

    impl->filter = strdup(filter);
    impl->fcode = fcode;

    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int ipfw_daq_start (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;

    if ( (impl->sock = socket(impl->proto, SOCK_RAW, IPPROTO_DIVERT)) == -1 )
    {
        DPE(impl->error, "%s: can't create divert socket (%s)\n",
            __FUNCTION__, strerror(errno));
        return DAQ_ERROR;
    }

    if ( bind(impl->sock, (struct sockaddr *)&impl->sin, sizeof(impl->sin)) == -1 )
    {
        DPE(impl->error, "%s: can't bind divert socket (%s)\n",
            __FUNCTION__, strerror(errno));
        return DAQ_ERROR;
    }

    impl->state = DAQ_STATE_STARTED;
    return DAQ_SUCCESS;
}

static int ipfw_daq_stop (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    close(impl->sock);
    impl->sock = -1;
    impl->state = DAQ_STATE_STOPPED;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int ipfw_daq_forward (
    IpfwImpl* impl, const DAQ_PktHdr_t* hdr, const uint8_t* buf, uint32_t len,
    int reverse)
{
    int status = sendto(
        impl->sock, buf, len, 0,
        (struct sockaddr*)&impl->sin, sizeof(impl->sin));

    if ( status == -1 )
    {
        DPE(impl->error, "%s: can't sendto divert socket (%s)\n",
            __FUNCTION__, strerror(errno));
        return DAQ_ERROR;
    }
    return DAQ_SUCCESS;
}

static int ipfw_daq_inject (
    void* handle, const DAQ_PktHdr_t* hdr, const uint8_t* buf, uint32_t len,
    int reverse)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    int status = ipfw_daq_forward(impl, hdr, impl->buf, hdr->pktlen, 0);

    if ( status == DAQ_SUCCESS )
        impl->stats.packets_injected++;

    return status;
}

static void SetPktHdr(DAQ_PktHdr_t* phdr, ssize_t len)
{
    static struct timeval t;
    memset (&t, 0, sizeof(struct timeval));
    gettimeofday(&t, NULL);
    phdr->ts.tv_sec = t.tv_sec;
    phdr->ts.tv_usec = t.tv_usec;
    phdr->caplen = len;
    phdr->pktlen = len;
    phdr->device_index = 0;
    phdr->flags = 0;
}

//-------------------------------------------------------------------------

// forward all but drops and blacklists:
static const int s_fwd[MAX_DAQ_VERDICT] = { 1, 0, 1, 1, 0, 1 };

static int ipfw_daq_acquire (
    void* handle, int cnt, DAQ_Analysis_Func_t callback, void* user)
{
    IpfwImpl* impl = (IpfwImpl*)handle;

    fd_set fdset;
    int n = 0;
    struct timeval tv;

    tv.tv_usec = 0;
    // If cnt is <= 0, don't limit the packets acquired.  However,
    // impl->count = 0 has a special meaning, so interpret accordingly.
    impl->count = (cnt == 0) ? -1 : cnt;

    while ( impl->count < 0 || n < impl->count )
    {
        FD_ZERO(&fdset);
        FD_SET(impl->sock, &fdset);

        // set this per call
        tv.tv_sec = impl->timeout;

        if ( select(impl->sock+1, &fdset, NULL, NULL, &tv) < 0 )
        {
            DPE(impl->error, "%s: can't select divert socket (%s)\n",
                __FUNCTION__, strerror(errno));
            return DAQ_ERROR;
        }

        if (FD_ISSET(impl->sock, &fdset))
        {
            socklen_t sinlen = sizeof(impl->sin);
            ssize_t pktlen;

            DAQ_PktHdr_t hdr;
            DAQ_Verdict verdict;

            if ((pktlen = recvfrom(impl->sock, impl->buf, impl->snaplen, 0,
                (struct sockaddr *)&impl->sin, &sinlen)) == -1)
            {
                if (errno != EINTR)
                {
                    DPE(impl->error, "%s: can't readfrom divert socket (%s)\n",
                        __FUNCTION__, strerror(errno));
                    return DAQ_ERROR;
                }
            }

            SetPktHdr(&hdr, pktlen);
            impl->stats.hw_packets_received++;

            if (
                impl->fcode.bf_insns &&
                sfbpf_filter(impl->fcode.bf_insns, impl->buf,
                    hdr.caplen, hdr.caplen) == 0
            ) {
                verdict = DAQ_VERDICT_PASS;
                impl->stats.packets_filtered++;
            }
            else
            {
                verdict = callback(NULL, &hdr, impl->buf);
                impl->stats.verdicts[verdict]++;
                impl->stats.packets_received++;
            }
            if ( impl->passive || s_fwd[verdict] )
                ipfw_daq_forward(impl, &hdr, impl->buf, hdr.pktlen, 0);

            n++;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

static int ipfw_daq_breakloop (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    impl->count = 0;
    return DAQ_SUCCESS;
}

static DAQ_State ipfw_daq_check_status (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    return impl->state;
}

static int ipfw_daq_get_stats (void* handle, DAQ_Stats_t* stats)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    *stats = impl->stats;
    return DAQ_SUCCESS;
}

static void ipfw_daq_reset_stats (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    memset(&impl->stats, 0, sizeof(impl->stats));
}

static int ipfw_daq_get_snaplen (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    return impl->snaplen;
}

static uint32_t ipfw_daq_get_capabilities (void* handle)
{
    return DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT | DAQ_CAPA_INJECT_RAW
        | DAQ_CAPA_BREAKLOOP | DAQ_CAPA_UNPRIV_START | DAQ_CAPA_BPF;
}

static int ipfw_daq_get_datalink_type(void *handle)
{
    return DLT_IPV4;
}

static const char* ipfw_daq_get_errbuf (void* handle)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    return impl->error;
}

static void ipfw_daq_set_errbuf (void* handle, const char* s)
{
    IpfwImpl* impl = (IpfwImpl*)handle;
    DPE(impl->error, "%s", s ? s : "");
}

static int ipfw_daq_get_device_index(void* handle, const char* device)
{
    return DAQ_ERROR_NOTSUP;
}

//-------------------------------------------------------------------------

#ifdef BUILDING_SO
SO_PUBLIC DAQ_Module_t DAQ_MODULE_DATA =
#else
DAQ_Module_t ipfw_daq_module_data =
#endif
{
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_MOD_VERSION,
    .name = DAQ_NAME,
    .type = DAQ_TYPE,
    .initialize = ipfw_daq_initialize,
    .set_filter = ipfw_daq_set_filter,
    .start = ipfw_daq_start,
    .acquire = ipfw_daq_acquire,
    .inject = ipfw_daq_inject,
    .breakloop = ipfw_daq_breakloop,
    .stop = ipfw_daq_stop,
    .shutdown = ipfw_daq_shutdown,
    .check_status = ipfw_daq_check_status,
    .get_stats = ipfw_daq_get_stats,
    .reset_stats = ipfw_daq_reset_stats,
    .get_snaplen = ipfw_daq_get_snaplen,
    .get_capabilities = ipfw_daq_get_capabilities,
    .get_datalink_type = ipfw_daq_get_datalink_type,
    .get_errbuf = ipfw_daq_get_errbuf,
    .set_errbuf = ipfw_daq_set_errbuf,
    .get_device_index = ipfw_daq_get_device_index
};

