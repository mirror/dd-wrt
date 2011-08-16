/* $Id: daq_nfq.c,v 1.19 2010/12/08 15:53:58 rcombs Exp $ */
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
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "daq_api.h"
#include "sfbpf.h"

#define DAQ_MOD_VERSION  4

#define DAQ_NAME "nfq"
#define DAQ_TYPE (DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_INLINE_CAPABLE | \
                  DAQ_TYPE_MULTI_INSTANCE | DAQ_TYPE_NO_UNPRIV)

// FIXTHIS meta data is 80 bytes on my test platform but who knows?
// documentation is poor; need correct way to size meta data
// erring on the high side here to avoid truncation errors
#define META_DATA_SIZE 512

#define MSG_BUF_SIZE META_DATA_SIZE + IP_MAXPACKET

typedef struct
{
    int protos, sock, qid;
    int qlen;

    struct nfq_handle* nf_handle;
    struct nfq_q_handle* nf_queue;

    const char* device;
    char* filter;
    struct sfbpf_program fcode;

    ip_t* net;
    eth_t* link;

    uint8_t* buf;
    void* user_data;
    DAQ_Analysis_Func_t user_func;

    volatile int count;
    int passive;
    uint32_t snaplen;
    unsigned timeout;

    char error[DAQ_ERRBUF_SIZE];
    DAQ_State state;
    DAQ_Stats_t stats;
} NfqImpl;

static void nfq_daq_shutdown(void* handle);
static int daq_nfq_callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg,
    struct nfq_data* nfad, void* data);

//-------------------------------------------------------------------------
// utilities

#define DEFAULT_Q 0

#define IP4(i) (i->protos & 0x1)
#define IP6(i) (i->protos & 0x2)

// FIXTHIS ip4 is always enabled??

static int nfq_daq_get_protos (const char* s)
{
    if ( !s || !strncasecmp(s, "ip4", 3) )
        return 0x1;

    if ( !strncasecmp(s, "ip6", 3) )
        return 0x2;
#if 0
    // doesn't look like both can be handled simultaneously
    if ( !strncasecmp(s, "ip*", 3) )
        return 0x3;
#endif
    return 0;
}

static int nfq_daq_get_setup (
    NfqImpl* impl, const DAQ_Config_t* cfg, char* errBuf, size_t errMax)
{
    DAQ_Dict* entry;

    impl->protos = 0x1;
    impl->qid = DEFAULT_Q;
    impl->qlen = 0;

    for ( entry = cfg->values; entry; entry = entry->next)
    {
        if ( !entry->value || !*entry->value )
        {
            snprintf(errBuf, errMax,
                "%s: variable needs value (%s)\n", __FUNCTION__, entry->key);
                return DAQ_ERROR;
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
        else if ( !strcmp(entry->key, "proto") )
        {
            impl->protos = nfq_daq_get_protos(entry->value);
            if ( !impl->protos )
            {
                snprintf(errBuf, errMax, "%s: bad proto (%s)\n",
                    __FUNCTION__, entry->value);
                return DAQ_ERROR;
            }
        }
        else if ( !strcmp(entry->key, "queue") )
        {
            char* end = entry->value;

            impl->qid = (int)strtol(entry->value, &end, 0);

            if ( *end || impl->qid < 0 || impl->qid > 65535 )
            {
                snprintf(errBuf, errMax, "%s: bad queue (%s)\n",
                    __FUNCTION__, entry->value);
                return DAQ_ERROR;
            }
        }
        else if ( !strcmp(entry->key, "queue_len") )
        {
            char* end = entry->value;
            impl->qlen = (int)strtol(entry->value, &end, 0);

            if ( *end || impl->qlen < 0 || impl->qlen > 65535 )
            {
                snprintf(errBuf, errMax, "%s: bad queue length (%s)\n",
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

    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int nfq_daq_initialize (
    const DAQ_Config_t* cfg, void** handle, char* errBuf, size_t errMax)
{
    if(cfg->name && *(cfg->name))
    {
        snprintf(errBuf, errMax, "The nfq DAQ module does not support interface or readback mode!");
        return DAQ_ERROR_INVAL;
    }
    // setup internal stuff
    NfqImpl *impl = calloc(1, sizeof(*impl));

    if ( !impl )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate nfq context\n",
            __FUNCTION__);
        return DAQ_ERROR_NOMEM;
    }

    if ( nfq_daq_get_setup(impl, cfg, errBuf, errMax) != DAQ_SUCCESS )
    {
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    if ( (impl->buf = malloc(MSG_BUF_SIZE)) == NULL )
    {
        snprintf(errBuf, errMax, "%s: failed to allocate nfq buffer\n",
            __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR_NOMEM;
    }

    // setup input stuff
    // 1. get a new q handle
    if ( !(impl->nf_handle = nfq_open()) )
    {
        snprintf(errBuf, errMax, "%s: failed to get handle for nfq\n",
            __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // 2. now use the new q handle to rip the rug out from other
    //    nfq users / handles?  actually that doesn't seem to
    //    happen which is good, but then why is this *supposed*
    //    to be necessary?  especially since we haven't bound to
    //    a qid yet, and that is exclusive anyway.
    if (
        (IP4(impl) && nfq_unbind_pf(impl->nf_handle, PF_INET) < 0) ||
        (IP6(impl) && nfq_unbind_pf(impl->nf_handle, PF_INET6) < 0) )
    {
        snprintf(errBuf, errMax, "%s: failed to unbind protocols for nfq\n",
            __FUNCTION__);
        //nfq_daq_shutdown(impl);
        //return DAQ_ERROR;
    }

    // 3. select protocols for the q handle
    //    this is necessary but insufficient because we still
    //    must configure iptables externally, eg:
    //
    //    iptables -A OUTPUT -p icmp -j NFQUEUE [--queue-num <#>]
    //    (# defaults to 0).
    //
    // :( iptables rules should be managed automatically to avoid
    //    queueing packets to nowhere or waiting for packets that
    //    will never come.  (ie this bind should take the -p, -s,
    //    etc args you can pass to iptables and create the dang
    //    rule!)
    if (
        (IP4(impl) && nfq_bind_pf(impl->nf_handle, PF_INET) < 0) ||
        (IP6(impl) && nfq_bind_pf(impl->nf_handle, PF_INET6) < 0) )
    {
        snprintf(errBuf, errMax, "%s: failed to bind protocols for nfq\n",
            __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // 4. bind to/allocate the specified nfqueue instance
    //    (this is the puppy specified via iptables as in
    //    above example.)
    //
    // ** there can be at most 1 nf_queue per qid
    if ( !(impl->nf_queue = nfq_create_queue(
        impl->nf_handle, impl->qid, daq_nfq_callback, impl)) )
    {
        snprintf(errBuf, errMax, "%s: nf queue creation failed\n",
            __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // 5. configure copying for maximum overhead
    if ( nfq_set_mode(impl->nf_queue, NFQNL_COPY_PACKET, IP_MAXPACKET) < 0 )
    {
        snprintf(errBuf, errMax, "%s: unable to set packet copy mode\n",
            __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // 6. set queue length (optional)
    if ( impl->qlen > 0 &&
            nfq_set_queue_maxlen(impl->nf_queue, impl->qlen))
    {
        snprintf(errBuf, errMax, "%s: unable to set queue length\n",
                __FUNCTION__);
        nfq_daq_shutdown(impl);
        return DAQ_ERROR;
    }

    // 7. get the q socket descriptor
    //    (after getting not 1 but 2 handles!)
    impl->sock = nfq_fd(impl->nf_handle);

    // setup output stuff
    // we've got 2 handles and a socket descriptor but, incredibly,
    // no way to inject?
    if ( impl->device && strcasecmp(impl->device, "ip") )
    {
        impl->link = eth_open(impl->device);

        if ( !impl->link )
        {
            snprintf(errBuf, errMax, "%s: can't open %s!\n",
                __FUNCTION__, impl->device);
            nfq_daq_shutdown(impl);
            return DAQ_ERROR;
        }
    }
    else
    {
        impl->net = ip_open();

        if ( !impl->net )
        {
            snprintf(errBuf, errMax, "%s: can't open ip!\n", __FUNCTION__);
            nfq_daq_shutdown(impl);
            return DAQ_ERROR;
        }
    }

    impl->state = DAQ_STATE_INITIALIZED;

    *handle = impl;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static void nfq_daq_shutdown (void* handle)
{
    NfqImpl *impl = (NfqImpl*)handle;
    impl->state = DAQ_STATE_UNINITIALIZED;

    if (impl->nf_queue)
        nfq_destroy_queue(impl->nf_queue);

    // note that we don't unbind here because
    // we will unbind other programs too

    if(impl->nf_handle)
        nfq_close(impl->nf_handle);

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

static inline int SetPktHdr (
    NfqImpl* impl,
    struct nfq_data* nfad,
    DAQ_PktHdr_t* hdr,
    uint8_t** pkt
) {
    int len = nfq_get_payload(nfad, (char**)pkt);

    if ( len <= 0 )
        return -1;

    hdr->caplen = ((uint32_t)len <= impl->snaplen) ? (uint32_t)len : impl->snaplen;
    hdr->pktlen = len;
    hdr->flags = 0;

    nfq_get_timestamp(nfad, &hdr->ts);
    hdr->device_index = nfq_get_physindev(nfad);

    return 0;
}

//-------------------------------------------------------------------------

// forward all but blocks and blacklists:
static const int s_fwd[MAX_DAQ_VERDICT] = { 1, 0, 1, 1, 0, 1 };

static int daq_nfq_callback(
    struct nfq_q_handle* qh,
    struct nfgenmsg* nfmsg,
    struct nfq_data* nfad,
    void* data)
{
    NfqImpl *impl = (NfqImpl*)data;
    struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfad);

    DAQ_Verdict verdict;
    DAQ_PktHdr_t hdr;
    uint8_t* pkt;
    int nf_verdict;
    uint32_t data_len;

    if ( impl->state != DAQ_STATE_STARTED )
        return -1;

    if ( !ph || SetPktHdr(impl, nfad, &hdr, &pkt) )
    {
        DPE(impl->error, "%s: can't setup packet header",
            __FUNCTION__);
        return -1;
    }

    if (
        impl->fcode.bf_insns &&
        sfbpf_filter(impl->fcode.bf_insns, pkt, hdr.caplen, hdr.caplen) == 0
    ) {
        verdict = DAQ_VERDICT_PASS;
        impl->stats.packets_filtered++;
    }
    else
    {
        verdict = impl->user_func(impl->user_data, &hdr, pkt);
        impl->stats.verdicts[verdict]++;
        impl->stats.packets_received++;
    }
    nf_verdict = ( impl->passive || s_fwd[verdict] ) ? NF_ACCEPT : NF_DROP;
    data_len = ( verdict == DAQ_VERDICT_REPLACE ) ? hdr.caplen : 0;

    nfq_set_verdict(
        impl->nf_queue, ntohl(ph->packet_id),
        nf_verdict, data_len, pkt);

    return 0;
}

//-------------------------------------------------------------------------
// 0. we open the queue and supply our internal callback,
//    daq_nfq_callback.
// 1. the daq client calls in here to get packets.
//    we save off the user's data and callback.
// 2. then we call nfq_handle_packet for each packet received
// 3. nfq_handle_packet casts a magic spell and then calls our
//    internal callback.
// 4. that in turn applies an optional bpf and passes traffic
//    that is filtered out.
// 5. traffic that is not filtered out is passed to the previously
//    saved user callback along with the user data.
// 6. the verdict returned from the callback is used to issue
//    a pass / drop verdict to the nfq.
// 7. this unwinds and we repeat back at step 2.

static int nfq_daq_acquire (
    void* handle, int c, DAQ_Analysis_Func_t callback, void* user)
{
    NfqImpl *impl = (NfqImpl*)handle;

    int n = 0;
    fd_set fdset;
    struct timeval tv;
    tv.tv_usec = 0;

    // If c is <= 0, don't limit the packets acquired.  However,
    // impl->count = 0 has a special meaning, so interpret accordingly.
    impl->count = (c == 0) ? -1 : c;
    impl->user_data = user;
    impl->user_func = callback;

    while ( (impl->count < 0) || (n < impl->count) )
    {
        FD_ZERO(&fdset);
        FD_SET(impl->sock, &fdset);

        // set this per call
        tv.tv_sec = impl->timeout;
        tv.tv_usec = 0;

        // at least ipq had a timeout!
        if ( select(impl->sock+1, &fdset, NULL, NULL, &tv) < 0 )
        {
            if ( errno == EINTR )
                break;
            DPE(impl->error, "%s: select = %s",
                __FUNCTION__, strerror(errno));
            return DAQ_ERROR;
        }

        if (FD_ISSET(impl->sock, &fdset))
        {
            int len = recv(impl->sock, impl->buf, MSG_BUF_SIZE, 0);

            if ( len > 0)
            {
                int stat = nfq_handle_packet(
                    impl->nf_handle, (char*)impl->buf, len);

                impl->stats.hw_packets_received++;

                if ( stat < 0 )
                {
                    DPE(impl->error, "%s: nfq_handle_packet = %s",
                        __FUNCTION__, strerror(errno));
                    return DAQ_ERROR;
                }
                n++;
            }
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

static int nfq_daq_inject (
    void* handle, const DAQ_PktHdr_t* hdr, const uint8_t* buf, uint32_t len,
    int reverse)
{
    NfqImpl* impl = (NfqImpl*)handle;
    ssize_t sent = 0;

    if ( impl->link )
        sent = eth_send(impl->link, buf, len);

    else if ( impl->net )
        sent = ip_send(impl->net, buf, len);

    if ( sent != len )
    {
        DPE(impl->error, "%s: failed to send",
            __FUNCTION__);
        return DAQ_ERROR;
    }
    impl->stats.packets_injected++;
    return DAQ_SUCCESS;
}

//-------------------------------------------------------------------------

static int nfq_daq_set_filter (void* handle, const char* filter)
{
    NfqImpl* impl = (NfqImpl*)handle;
    struct sfbpf_program fcode;
    int dlt = IP4(impl) ? DLT_IPV4 : DLT_IPV6;

    if (sfbpf_compile(impl->snaplen, dlt, &fcode, filter, 1, 0) < 0)
    {
        DPE(impl->error, "%s: failed to compile bpf '%s'",
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

static int nfq_daq_start (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    impl->state = DAQ_STATE_STARTED;
    return DAQ_SUCCESS;
}

static int nfq_daq_breakloop (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    impl->count = 0;
    return DAQ_SUCCESS;
}

static int nfq_daq_stop (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    impl->state = DAQ_STATE_STOPPED;
    return DAQ_SUCCESS;
}

static DAQ_State nfq_daq_check_status (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    return impl->state;
}

static int nfq_daq_get_stats (void* handle, DAQ_Stats_t* stats)
{
    NfqImpl* impl = (NfqImpl*)handle;
    *stats = impl->stats;
    return DAQ_SUCCESS;
}

static void nfq_daq_reset_stats (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    memset(&impl->stats, 0, sizeof(impl->stats));
}

static int nfq_daq_get_snaplen (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    return impl->snaplen;
}

static uint32_t nfq_daq_get_capabilities (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    uint32_t caps = DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT
        | DAQ_CAPA_BREAKLOOP | DAQ_CAPA_UNPRIV_START | DAQ_CAPA_BPF;
    if ( impl->net ) caps |= DAQ_CAPA_INJECT_RAW;
    return caps;
}

static int nfq_daq_get_datalink_type(void *handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    int dlt = IP4(impl) ? DLT_IPV4 : DLT_IPV6;
    return dlt;
}

static const char* nfq_daq_get_errbuf (void* handle)
{
    NfqImpl* impl = (NfqImpl*)handle;
    return (char*)impl->error;
}

static void nfq_daq_set_errbuf (void* handle, const char* s)
{
    NfqImpl* impl = (NfqImpl*)handle;
    DPE(impl->error, "%s", s ? s : "");
}

static int nfq_daq_get_device_index(void* handle, const char* device)
{
    return DAQ_ERROR_NOTSUP;
}

//-------------------------------------------------------------------------

#ifdef BUILDING_SO
SO_PUBLIC DAQ_Module_t DAQ_MODULE_DATA =
#else
DAQ_Module_t nfq_daq_module_data =
#endif
{
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_MOD_VERSION,
    .name = DAQ_NAME,
    .type = DAQ_TYPE,
    .initialize = nfq_daq_initialize,
    .set_filter = nfq_daq_set_filter,
    .start = nfq_daq_start,
    .acquire = nfq_daq_acquire,
    .inject = nfq_daq_inject,
    .breakloop = nfq_daq_breakloop,
    .stop = nfq_daq_stop,
    .shutdown = nfq_daq_shutdown,
    .check_status = nfq_daq_check_status,
    .get_stats = nfq_daq_get_stats,
    .reset_stats = nfq_daq_reset_stats,
    .get_snaplen = nfq_daq_get_snaplen,
    .get_capabilities = nfq_daq_get_capabilities,
    .get_datalink_type = nfq_daq_get_datalink_type,
    .get_errbuf = nfq_daq_get_errbuf,
    .set_errbuf = nfq_daq_set_errbuf,
    .get_device_index = nfq_daq_get_device_index
};

