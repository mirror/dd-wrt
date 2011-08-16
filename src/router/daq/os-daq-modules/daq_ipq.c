/* $Id: daq_ipq.c,v 1.20 2010/12/08 15:53:58 rcombs Exp $ */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/unistd.h>

#include <netinet/ip.h>

#include <dnet.h>
#include <linux/netfilter.h>
#include <libipq.h>

#include "daq_api.h"
#include "sfbpf.h"

#define DAQ_MOD_VERSION  4

#define DAQ_TYPE (DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_INLINE_CAPABLE | \
                  DAQ_TYPE_MULTI_INSTANCE | DAQ_TYPE_NO_UNPRIV)

#define MSG_BUF_SIZE (sizeof(ipq_packet_msg_t) + IP_MAXPACKET)

typedef struct {
    int proto;
    char* device;
    struct ipq_handle* ipqh;

    ip_t* net;
    eth_t* link;

    char* filter;
    struct sfbpf_program fcode;

    uint8_t* buf;
    char error[DAQ_ERRBUF_SIZE];

    volatile int count;
    int passive;
    uint32_t snaplen;
    unsigned timeout;

    DAQ_State state;
    DAQ_Stats_t stats;
} IpqImpl;

static void ipq_daq_shutdown(void* handle);

//-------------------------------------------------------------------------
// FIXTHIS ip6 yields "ipq_set_mode error Failed to send netlink message"

static int ipq_daq_get_proto (const char* s)
{
    if ( s && !strcasecmp(s, "ip4") )
        return PF_INET;

    if ( s && !strcasecmp(s, "ip6") )
        return PF_INET6;

    return 0;
}

static int ipq_daq_get_setup (
    IpqImpl* impl, const DAQ_Config_t* cfg, char* errBuf, size_t errMax)
{
    DAQ_Dict* entry;

    impl->proto = PF_INET;

    for ( entry = cfg->values; entry; entry = entry->next)
    {
        if ( !entry->value || !*entry->value )
        {
            snprintf(errBuf, errMax,
                "%s: variable needs value (%s)\n", __FUNCTION__, entry->key);
                return DAQ_ERROR;
        }
        else if ( !strcmp(entry->key, "proto") )
        {
            impl->proto = ipq_daq_get_proto(entry->value);
            if ( !impl->proto )
            {
                snprintf(errBuf, errMax, "%s: bad proto (%s)\n",
                    __FUNCTION__, entry->value);
                return DAQ_ERROR;
            }
        }
        else if ( !strcmp(entry->key, "device") )
        {
            impl->device = strdup(entry->value);
            if ( !impl->device )
            {
                snprintf(errBuf, errMax,
                    "%s: can't allocate memory for device (%s)\n",
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
    impl->timeout = cfg->timeout * 1000;  // cfg is ms, ipq is us
    impl->passive = ( cfg->mode == DAQ_MODE_PASSIVE );

    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------
// initialization and clean up

static int ipq_daq_initialize (
    const DAQ_Config_t* cfg, void** handle, char* errBuf, size_t errMax)
{
    int status;

    if(cfg->name && *(cfg->name))
    {
        snprintf(errBuf, errMax, "The ipq DAQ module does not support interface or readback mode!");
        return DAQ_ERROR_INVAL;
    }

    IpqImpl* impl = calloc(1, sizeof(*impl));

    if ( !impl )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate the ipq context!",
            __FUNCTION__);
        return DAQ_ERROR_NOMEM;
    }

    if ( ipq_daq_get_setup(impl, cfg, errBuf, errMax) != DAQ_SUCCESS )
    {
        ipq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    impl->buf = malloc(MSG_BUF_SIZE);

    if ( !impl->buf )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate the ipq buffer!",
            __FUNCTION__);
        ipq_daq_shutdown(impl);
        return DAQ_ERROR_NOMEM;
    }

    // remember to also link in, eg:
    //    iptables -A OUTPUT -p icmp -j QUEUE
    impl->ipqh = ipq_create_handle(0, impl->proto);

    if ( !impl->ipqh )
    {
        snprintf(errBuf, errMax, "%s: ipq_create_handle error %s\n",
            __FUNCTION__, ipq_errstr());
        ipq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // copy both packet metadata and packet payload
    // paket payload is limited to IP_MAXPACKET
    status = ipq_set_mode(impl->ipqh, IPQ_COPY_PACKET, IP_MAXPACKET);

    if ( status < 0 )
    {
        snprintf(errBuf, errMax, "%s: ipq_set_mode error %s\n",
            __FUNCTION__, ipq_errstr());
        ipq_daq_shutdown(impl);
        return DAQ_ERROR;
    }
    if ( impl->device && strcasecmp(impl->device, "ip") )
    {
        impl->link = eth_open(impl->device);

        if ( !impl->link )
        {
            snprintf(errBuf, errMax, "%s: can't open %s!\n",
                __FUNCTION__, impl->device);
            ipq_daq_shutdown(impl);
            return DAQ_ERROR;
        }
    }
    else
    {
        impl->net = ip_open();

        if ( !impl->net )
        {
            snprintf(errBuf, errMax, "%s: can't open ip!\n", __FUNCTION__);
            ipq_daq_shutdown(impl);
            return DAQ_ERROR;
        }
    }
    impl->state = DAQ_STATE_INITIALIZED;

    *handle = impl;
    return DAQ_SUCCESS;
}

static void ipq_daq_shutdown (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;

    if ( impl->ipqh )
        ipq_destroy_handle(impl->ipqh);

    if ( impl->device )
        free(impl->device);

    if ( impl->link )
        eth_close(impl->link);

    if ( impl->net )
        ip_close(impl->net);

    if ( impl->filter )
        free(impl->filter);

    if ( impl->buf )
        free(impl->buf);

    free(impl);
}

//-------------------------------------------------------------------------

static void SetPktHdr(
    IpqImpl* impl, ipq_packet_msg_t* m, DAQ_PktHdr_t* phdr)
{
    if ( !m->timestamp_sec )
    {
        struct timeval t;
        gettimeofday(&t, NULL);

        phdr->ts.tv_sec = t.tv_sec;
        phdr->ts.tv_usec = t.tv_usec;
    }
    else
    {
        phdr->ts.tv_sec = m->timestamp_sec;
        phdr->ts.tv_usec = m->timestamp_usec;
    }
    phdr->caplen = (m->data_len <= impl->snaplen) ? m->data_len : impl->snaplen;
    phdr->pktlen = m->data_len;
}

//-------------------------------------------------------------------------
// public packet processing

static int ipq_daq_acquire (
    void* handle, int cnt, DAQ_Analysis_Func_t callback, void* user)
{
    IpqImpl* impl = (IpqImpl*)handle;

    int n = 0;
    DAQ_PktHdr_t hdr;

    // If cnt is <= 0, don't limit the packets acquired.  However,
    // impl->count = 0 has a special meaning, so interpret accordingly.
    impl->count = (cnt == 0) ? -1 : cnt;
    hdr.device_index = 0;
    hdr.flags = 0;

    while ( impl->count < 0 || n < impl->count )
    {
        int ipqt, status = ipq_read(
            impl->ipqh, impl->buf, MSG_BUF_SIZE, impl->timeout);

        if ( status <= 0 )
        {
            if ( status < 0 )
            {
                DPE(impl->error, "%s: ipq_read=%d error %s",
                    __FUNCTION__, status, ipq_errstr());
                return DAQ_ERROR;
            }
            return 0;
        }

        ipqt = ipq_message_type(impl->buf);

        if ( ipqt == IPQM_PACKET )
        {
            DAQ_Verdict verdict;
            ipq_packet_msg_t* ipqm = ipq_get_packet(impl->buf);
            SetPktHdr(impl, ipqm, &hdr);
            impl->stats.hw_packets_received++;

            if (
                impl->fcode.bf_insns &&
                sfbpf_filter(impl->fcode.bf_insns, ipqm->payload,
                    hdr.caplen, hdr.caplen) == 0
            ) {
                verdict = DAQ_VERDICT_PASS;
                impl->stats.packets_filtered++;
            }
            else
            {
                verdict = callback(user, &hdr, (uint8_t*)ipqm->payload);
                impl->stats.verdicts[verdict]++;
                impl->stats.packets_received++;
            }
            if ( impl->passive ) verdict = DAQ_VERDICT_PASS;

            switch ( verdict ) {
            case DAQ_VERDICT_BLOCK:
            case DAQ_VERDICT_BLACKLIST:
                status = ipq_set_verdict(
                    impl->ipqh, ipqm->packet_id, NF_DROP, 0, NULL);
                break;

            case DAQ_VERDICT_REPLACE:
                status = ipq_set_verdict(
                    impl->ipqh, ipqm->packet_id, NF_ACCEPT,
                    hdr.pktlen, ipqm->payload);
                break;

            case DAQ_VERDICT_PASS:
            case DAQ_VERDICT_WHITELIST:
            case DAQ_VERDICT_IGNORE:
            default:
                status = ipq_set_verdict(
                    impl->ipqh, ipqm->packet_id, NF_ACCEPT, 0, NULL);
                break;
            }
            if ( status < 0 )
            {
                DPE(impl->error, "%s: ipq_set_verdict=%d error %s",
                    __FUNCTION__, status, ipq_errstr());
                return DAQ_ERROR;
            }
            n++;
        }
        else
        {
            // NLMSG_ERROR is supposed to be the only other valid type
            status = ipq_get_msgerr(impl->buf);
            DPE(impl->error, "%s: ipq_message_type=%d error=%d %s",
                __FUNCTION__, ipqt, status, ipq_errstr());
            // ipq_message_type=2 error=1 Timeout
            // keep looping upon timeout or other errors
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

static int ipq_daq_inject (
    void* handle, const DAQ_PktHdr_t* hdr, const uint8_t* buf, uint32_t len,
    int reverse)
{
    IpqImpl* impl = (IpqImpl*)handle;
    ssize_t sent = 0;

    if ( impl->link )
        sent = eth_send(impl->link, buf, len);

    else if ( impl->net )
        sent = ip_send(impl->net, buf, len);

    if ( (uint32_t)sent != len )
    {
        DPE(impl->error, "%s: failed to send",
            __FUNCTION__);
        return DAQ_ERROR;
    }
    impl->stats.packets_injected++;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int ipq_daq_set_filter (void* handle, const char* filter)
{
    IpqImpl* impl = (IpqImpl*)handle;
    struct sfbpf_program fcode;
    int dlt = (impl->proto == PF_INET) ? DLT_IPV4 : DLT_IPV6;

    if (sfbpf_compile(impl->snaplen, dlt, &fcode, filter, 1, 0) < 0)
    {
        DPE(impl->error, "%s: failed to compile '%s'",
            __FUNCTION__, filter);
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

static int ipq_daq_start (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    impl->state = DAQ_STATE_STARTED;
    return DAQ_SUCCESS;
}

static int ipq_daq_breakloop (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    impl->count = 0;
    return DAQ_SUCCESS;
}

static int ipq_daq_stop (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    impl->state = DAQ_STATE_STOPPED;
    return DAQ_SUCCESS;
}

static DAQ_State ipq_daq_check_status (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    return impl->state;
}

static int ipq_daq_get_stats (void* handle, DAQ_Stats_t* stats)
{
    IpqImpl* impl = (IpqImpl*)handle;
    *stats = impl->stats;
    return DAQ_SUCCESS;
}

static void ipq_daq_reset_stats (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    memset(&impl->stats, 0, sizeof(impl->stats));
}

static int ipq_daq_get_snaplen (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    return impl->snaplen;
}

static uint32_t ipq_daq_get_capabilities (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    uint32_t caps = DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT
        | DAQ_CAPA_BREAKLOOP | DAQ_CAPA_UNPRIV_START | DAQ_CAPA_BPF;
    if ( impl->net ) caps |= DAQ_CAPA_INJECT_RAW;
    return caps;
}

static int ipq_daq_get_datalink_type(void *handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    int dlt = (impl->proto == PF_INET) ? DLT_IPV4 : DLT_IPV6;
    return dlt;
}

static const char* ipq_daq_get_errbuf (void* handle)
{
    IpqImpl* impl = (IpqImpl*)handle;
    return impl->error;
}

static void ipq_daq_set_errbuf (void* handle, const char* s)
{
    IpqImpl* impl = (IpqImpl*)handle;
    DPE(impl->error, "%s", s ? s : "");
}

static int ipq_daq_get_device_index(void* handle, const char* device)
{
    return DAQ_ERROR_NOTSUP;
}

//-------------------------------------------------------------------------

#ifdef BUILDING_SO
SO_PUBLIC DAQ_Module_t DAQ_MODULE_DATA =
#else
DAQ_Module_t ipq_daq_module_data =
#endif
{
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_MOD_VERSION,
    .name = "ipq",
    .type = DAQ_TYPE,
    .initialize = ipq_daq_initialize,
    .set_filter = ipq_daq_set_filter,
    .start = ipq_daq_start,
    .acquire = ipq_daq_acquire,
    .inject = ipq_daq_inject,
    .breakloop = ipq_daq_breakloop,
    .stop = ipq_daq_stop,
    .shutdown = ipq_daq_shutdown,
    .check_status = ipq_daq_check_status,
    .get_stats = ipq_daq_get_stats,
    .reset_stats = ipq_daq_reset_stats,
    .get_snaplen = ipq_daq_get_snaplen,
    .get_capabilities = ipq_daq_get_capabilities,
    .get_datalink_type = ipq_daq_get_datalink_type,
    .get_errbuf = ipq_daq_get_errbuf,
    .set_errbuf = ipq_daq_set_errbuf,
    .get_device_index = ipq_daq_get_device_index
};

