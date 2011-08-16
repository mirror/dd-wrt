/*
** Copyright (C) 2010 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef WIN32
#include <sys/types.h>
#include <netinet/in.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pcap.h>
#ifndef PCAP_OLDSTYLE
# ifdef HAVE_LINUX_IF_PACKET_H
#include <linux/if_packet.h>
# endif /* HAVE_LINUX_IF_PACKET_H */
#include <unistd.h>
#endif /* PCAP_OLDSTYLE */

#include "daq_api.h"

#define DAQ_PCAP_VERSION 3

typedef struct _pcap_context
{
    char *device;
    char *file;
    char *filter_string;
    int snaplen;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    int promisc_flag;
    int timeout;
    int buffer_size;
    int packets;
    int delayed_open;
    DAQ_Analysis_Func_t analysis_func;
    u_char *user_data;
    uint32_t netmask;
    DAQ_Stats_t stats;
    uint32_t base_recv;
    uint32_t base_drop;
    uint64_t rollover_recv;
    uint64_t rollover_drop;
    uint32_t wrap_recv;
    uint32_t wrap_drop;
    DAQ_State state;
} Pcap_Context_t;

static void pcap_daq_reset_stats(void *handle);

#ifndef PCAP_OLDSTYLE
/* Attempt to convert from the PCAP_FRAMES environment variable used by Phil Wood's PCAP-Ring
    to a buffer size I can pass to PCAP 1.0.0's pcap_set_buffer_size(). */
static int translate_PCAP_FRAMES(int snaplen)
{
# ifdef HAVE_LINUX_IF_PACKET_H
    char *frames_str = getenv("PCAP_FRAMES");
    int frame_size, block_size, frames_per_block;
    int frames;

    if (!frames_str)
        return 0;

    /* Look, I didn't make these numbers and calculations up, I'm just using them. */
    frame_size = TPACKET_ALIGN(snaplen + TPACKET_ALIGN(TPACKET_HDRLEN) + sizeof(struct sockaddr_ll));
    block_size = getpagesize();
    while (block_size < frame_size)
        block_size <<= 1;
    frames_per_block = block_size / frame_size;

    if (strncmp(frames_str, "max", 3) && strncmp(frames_str, "MAX", 3))
        frames = strtol(frames_str, NULL, 10);
    else
        frames = 0x8000; /* Default maximum of 32k frames. */

    printf("PCAP_FRAMES -> %d * %d / %d = %d (%d)\n", frames, block_size, frames_per_block, frames * block_size / frames_per_block, frame_size);
    return frames * block_size / frames_per_block;
# else
    return 0;
# endif
}
#endif /* PCAP_OLDSTYLE */

static int pcap_daq_open(Pcap_Context_t *context)
{
    uint32_t localnet, netmask;
    uint32_t defaultnet = 0xFFFFFF00;
#ifndef PCAP_OLDSTYLE
    int status;
#endif /* PCAP_OLDSTYLE */

    if (context->handle)
        return DAQ_SUCCESS;

    if (context->device)
    {
#ifndef PCAP_OLDSTYLE
        context->handle = pcap_create(context->device, context->errbuf);
        if (!context->handle)
            return DAQ_ERROR;
        if ((status = pcap_set_snaplen(context->handle, context->snaplen)) < 0)
            goto fail;
        if ((status = pcap_set_promisc(context->handle, context->promisc_flag ? 1 : 0)) < 0)
            goto fail;
        if ((status = pcap_set_timeout(context->handle, context->timeout)) < 0)
            goto fail;
        if ((status = pcap_set_buffer_size(context->handle, context->buffer_size)) < 0)
            goto fail;
        if ((status = pcap_activate(context->handle)) < 0)
            goto fail;
#else
        context->handle = pcap_open_live(context->device, context->snaplen,
                                         context->promisc_flag ? 1 : 0, context->timeout, context->errbuf);
        if (!context->handle)
            return DAQ_ERROR;
#endif /* PCAP_OLDSTYLE */
        if (pcap_lookupnet(context->device, &localnet, &netmask, context->errbuf) < 0)
            netmask = htonl(defaultnet);
    }
    else
    {
        context->handle = pcap_open_offline(context->file, context->errbuf);
        if (!context->handle)
            return DAQ_ERROR;

        netmask = htonl(defaultnet);
    }
    context->netmask = htonl(defaultnet);

    return DAQ_SUCCESS;

#ifndef PCAP_OLDSTYLE
fail:
    if (status == PCAP_ERROR || status == PCAP_ERROR_NO_SUCH_DEVICE || status == PCAP_ERROR_PERM_DENIED)
        DPE(context->errbuf, "%s", pcap_geterr(context->handle));
    else
        DPE(context->errbuf, "%s: %s", context->device, pcap_statustostr(status));
    pcap_close(context->handle);
    context->handle = NULL;
    return DAQ_ERROR;
#endif /* PCAP_OLDSTYLE */
}

static int update_hw_stats(Pcap_Context_t *context)
{
    struct pcap_stat ps;

    if (context->handle && context->device)
    {
        memset(&ps, 0, sizeof(struct pcap_stat));
        if (pcap_stats(context->handle, &ps) == -1)
        {
            DPE(context->errbuf, "%s", pcap_geterr(context->handle));
            return DAQ_ERROR;
        }

        /* PCAP receive counter wrapped */
        if (ps.ps_recv < context->wrap_recv)
            context->rollover_recv += UINT32_MAX;

        /* PCAP drop counter wrapped */
        if (ps.ps_drop < context->wrap_drop)
            context->rollover_drop += UINT32_MAX;

        context->wrap_recv = ps.ps_recv;
        context->wrap_drop = ps.ps_drop;

        context->stats.hw_packets_received = context->rollover_recv + context->wrap_recv - context->base_recv;
        context->stats.hw_packets_dropped = context->rollover_drop + context->wrap_drop - context->base_drop;
    }

    return DAQ_SUCCESS;
}

static int pcap_daq_initialize(const DAQ_Config_t *config, void **ctxt_ptr, char *errbuf, size_t len)
{
    Pcap_Context_t *context;
#ifndef PCAP_OLDSTYLE
    DAQ_Dict *entry;
#endif

    context = calloc(1, sizeof(Pcap_Context_t));
    if (!context)
    {
        snprintf(errbuf, len, "%s: Couldn't allocate memory for the new PCAP context!", __FUNCTION__);
        return DAQ_ERROR_NOMEM;
    }

    context->snaplen = config->snaplen;
    context->promisc_flag = (config->flags & DAQ_CFG_PROMISC);
    context->timeout = config->timeout;

#ifndef PCAP_OLDSTYLE
    /* Retrieve the requested buffer size (default = 0) */
    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, "buffer_size"))
            context->buffer_size = strtol(entry->key, NULL, 10);
    }
    /* Try to account for legacy PCAP_FRAMES environment variable if we weren't passed a buffer size. */
    if (context->buffer_size == 0)
        context->buffer_size = translate_PCAP_FRAMES(context->snaplen);
#endif

    if (config->mode == DAQ_MODE_READ_FILE)
    {
        context->file = strdup(config->name);
        if (!context->file)
        {
            snprintf(errbuf, len, "%s: Couldn't allocate memory for the filename string!", __FUNCTION__);
            free(context);
            return DAQ_ERROR_NOMEM;
        }
        context->delayed_open = 0;
    }
    else
    {
        context->device = strdup(config->name);
        if (!context->device)
        {
            snprintf(errbuf, len, "%s: Couldn't allocate memory for the device string!", __FUNCTION__);
            free(context);
            return DAQ_ERROR_NOMEM;
        }
        context->delayed_open = 1;
    }

    if (!context->delayed_open)
    {
        if (pcap_daq_open(context) != DAQ_SUCCESS)
        {
            snprintf(errbuf, len, "%s", context->errbuf);
            free(context);
            return DAQ_ERROR;
        }
    }

    context->state = DAQ_STATE_INITIALIZED;

    *ctxt_ptr = context;
    return DAQ_SUCCESS;
}

static int pcap_daq_set_filter(void *handle, const char *filter)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;
    struct bpf_program fcode;
    pcap_t *dead_handle;

    if (context->handle)
    {
        if (pcap_compile(context->handle, &fcode, (char *)filter, 1, context->netmask) < 0)
        {
            DPE(context->errbuf, "%s: pcap_compile: %s", __FUNCTION__, pcap_geterr(context->handle));
            return DAQ_ERROR;
        }

        if (pcap_setfilter(context->handle, &fcode) < 0)
        {
            pcap_freecode(&fcode);
            DPE(context->errbuf, "%s: pcap_setfilter: %s", __FUNCTION__, pcap_geterr(context->handle));
            return DAQ_ERROR;
        }

        pcap_freecode(&fcode);
    }
    else
    {
        /* Try to validate the BPF with a dead PCAP handle. */
        dead_handle = pcap_open_dead(DLT_EN10MB, context->snaplen);
        if (!dead_handle)
        {
            DPE(context->errbuf, "%s: Could not allocate a dead PCAP handle!", __FUNCTION__);
            return DAQ_ERROR_NOMEM;
        }
        if (pcap_compile(dead_handle, &fcode, (char *)filter, 1, context->netmask) < 0)
        {
            DPE(context->errbuf, "%s: pcap_compile: %s", __FUNCTION__, pcap_geterr(dead_handle));
            return DAQ_ERROR;
        }
        pcap_freecode(&fcode);
        pcap_close(dead_handle);

        /* Store the BPF string for later. */
        if (context->filter_string)
            free(context->filter_string);
        context->filter_string = strdup(filter);
        if (!context->filter_string)
        {
            DPE(context->errbuf, "%s: Could not allocate space to store a copy of the filter string!", __FUNCTION__);
            return DAQ_ERROR_NOMEM;
        }
    }

    return DAQ_SUCCESS;
}

static int pcap_daq_start(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (pcap_daq_open(context) != DAQ_SUCCESS)
        return DAQ_ERROR;

    pcap_daq_reset_stats(handle);

    if (context->filter_string)
    {
        if (pcap_daq_set_filter(handle, context->filter_string))
            return DAQ_ERROR;
        free(context->filter_string);
        context->filter_string = NULL;
    }

    context->state = DAQ_STATE_STARTED;

    return DAQ_SUCCESS;
}

static void pcap_process_loop(u_char *user, const struct pcap_pkthdr *pkth, const u_char *data)
{
    Pcap_Context_t *context = (Pcap_Context_t *) user;
    DAQ_PktHdr_t hdr;
    DAQ_Verdict verdict;

    hdr.caplen = pkth->caplen;
    hdr.pktlen = pkth->len;
    hdr.ts = pkth->ts;
    hdr.device_index = -1;
    hdr.flags = 0;

    /* Increment the current acquire loop's packet counter. */
    context->packets++;
    /* ...and then the module instance's packet counter. */
    context->stats.packets_received++;
    verdict = context->analysis_func(context->user_data, &hdr, data);
    if (verdict >= MAX_DAQ_VERDICT)
        verdict = DAQ_VERDICT_PASS;
    context->stats.verdicts[verdict]++;
}

static int pcap_daq_acquire(
    void *handle, int cnt, DAQ_Analysis_Func_t callback, void *user)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;
    int ret;

    context->analysis_func = callback;
    context->user_data = user;

    context->packets = 0;
    while (context->packets < cnt || cnt <= 0)
    {
        ret = pcap_dispatch(
            context->handle, cnt-context->packets, pcap_process_loop, (void *) context);
        if (ret == -1)
        {
            DPE(context->errbuf, "%s", pcap_geterr(context->handle));
            return ret;
        }
        /* In read-file mode, PCAP returns 0 when it hits the end of the file. */
        else if (context->file && ret == 0)
            return DAQ_READFILE_EOF;
        /* If we hit a breakloop call or timed out without reading any packets, break out. */
        else if (ret == -2 || ret == 0)
            break;
    }

    return 0;
}

static int pcap_daq_inject(void *handle, const DAQ_PktHdr_t *hdr, const uint8_t *packet_data, uint32_t len, int reverse)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (pcap_inject(context->handle, packet_data, len) < 0)
    {
        DPE(context->errbuf, "%s", pcap_geterr(context->handle));
        return DAQ_ERROR;
    }

    context->stats.packets_injected++;
    return DAQ_SUCCESS;
}

static int pcap_daq_breakloop(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (!context->handle)
        return DAQ_ERROR;

    pcap_breakloop(context->handle);

    return DAQ_SUCCESS;
}

static int pcap_daq_stop(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (context->handle)
    {
        /* Store the hardware stats for post-stop stat calls. */
        update_hw_stats(context);
        pcap_close(context->handle);
        context->handle = NULL;
    }

    context->state = DAQ_STATE_STOPPED;

    return DAQ_SUCCESS;
}

static void pcap_daq_shutdown(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (context->handle)
        pcap_close(context->handle);
    if (context->device)
        free(context->device);
    if (context->file)
        free(context->file);
    if (context->filter_string)
        free(context->filter_string);
    free(context);
}

static DAQ_State pcap_daq_check_status(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    return context->state;
}

static int pcap_daq_get_stats(void *handle, DAQ_Stats_t *stats)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (update_hw_stats(context) != DAQ_SUCCESS)
        return DAQ_ERROR;

    memcpy(stats, &context->stats, sizeof(DAQ_Stats_t));

    return DAQ_SUCCESS;
}

static void pcap_daq_reset_stats(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;
    struct pcap_stat ps;

    memset(&context->stats, 0, sizeof(DAQ_Stats_t));

    if (!context->handle)
        return;

    memset(&ps, 0, sizeof(struct pcap_stat));
    if (context->handle && context->device && pcap_stats(context->handle, &ps) == 0)
    {
        context->base_recv = context->wrap_recv = ps.ps_recv;
        context->base_drop = context->wrap_drop = ps.ps_drop;
    }
}

static int pcap_daq_get_snaplen(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (context->handle)
        return pcap_snapshot(context->handle);

    return context->snaplen;
}

static uint32_t pcap_daq_get_capabilities(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;
    uint32_t capabilities = DAQ_CAPA_BPF;

    if (context->device)
        capabilities |= DAQ_CAPA_INJECT;

    capabilities |= DAQ_CAPA_BREAKLOOP;

    if (!context->delayed_open)
        capabilities |= DAQ_CAPA_UNPRIV_START;

    return capabilities;
}

static int pcap_daq_get_datalink_type(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (context->handle)
        return pcap_datalink(context->handle);

    return DLT_NULL;
}

static const char *pcap_daq_get_errbuf(void *handle)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    return context->errbuf;
}

static void pcap_daq_set_errbuf(void *handle, const char *string)
{
    Pcap_Context_t *context = (Pcap_Context_t *) handle;

    if (!string)
        return;

    DPE(context->errbuf, "%s", string);
}

static int pcap_daq_get_device_index(void *handle, const char *device)
{
    return DAQ_ERROR_NOTSUP;
}

#ifdef BUILDING_SO
SO_PUBLIC const DAQ_Module_t DAQ_MODULE_DATA =
#else
const DAQ_Module_t pcap_daq_module_data =
#endif
{
#ifndef WIN32
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_PCAP_VERSION,
    .name = "pcap",
    .type = DAQ_TYPE_FILE_CAPABLE | DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_MULTI_INSTANCE,
    .initialize = pcap_daq_initialize,
    .set_filter = pcap_daq_set_filter,
    .start = pcap_daq_start,
    .acquire = pcap_daq_acquire,
    .inject = pcap_daq_inject,
    .breakloop = pcap_daq_breakloop,
    .stop = pcap_daq_stop,
    .shutdown = pcap_daq_shutdown,
    .check_status = pcap_daq_check_status,
    .get_stats = pcap_daq_get_stats,
    .reset_stats = pcap_daq_reset_stats,
    .get_snaplen = pcap_daq_get_snaplen,
    .get_capabilities = pcap_daq_get_capabilities,
    .get_datalink_type = pcap_daq_get_datalink_type,
    .get_errbuf = pcap_daq_get_errbuf,
    .set_errbuf = pcap_daq_set_errbuf,
    .get_device_index = pcap_daq_get_device_index
#else
    DAQ_API_VERSION,
    DAQ_PCAP_VERSION,
    "pcap",
    DAQ_TYPE_FILE_CAPABLE | DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_MULTI_INSTANCE,
    pcap_daq_initialize,
    pcap_daq_set_filter,
    pcap_daq_start,
    pcap_daq_acquire,
    pcap_daq_inject,
    pcap_daq_breakloop,
    pcap_daq_stop,
    pcap_daq_shutdown,
    pcap_daq_check_status,
    pcap_daq_get_stats,
    pcap_daq_reset_stats,
    pcap_daq_get_snaplen,
    pcap_daq_get_capabilities,
    pcap_daq_get_datalink_type,
    pcap_daq_get_errbuf,
    pcap_daq_set_errbuf,
    pcap_daq_get_device_index
#endif
};
