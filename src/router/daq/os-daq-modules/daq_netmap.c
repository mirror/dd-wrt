/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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

#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>

#include <daq_api.h>
#include <sfbpf.h>
#include <sfbpf_dlt.h>

#include <net/netmap.h>
#include <net/netmap_user.h>

#define DAQ_NETMAP_VERSION      3

/* Hi! I'm completely arbitrary! */
#define NETMAP_MAX_INTERFACES       32

/* FreeBSD 10.0 uses an old version of netmap, so work around it accordingly. */
#if NETMAP_API < 10
#define nm_ring_next(r, i)      NETMAP_RING_NEXT(r, i)
#define nm_ring_empty(r)        ((r)->avail == 0)
#endif

typedef struct _netmap_instance
{
    struct _netmap_instance *next;
    struct _netmap_instance *peer;
    int fd;
#define NMINST_FWD_BLOCKED     0x1
#define NMINST_TX_BLOCKED      0x2
    uint32_t flags;
    int index;
    struct netmap_if *nifp;
    /* TX ring info */
    uint16_t first_tx_ring;
    uint16_t last_tx_ring;
    uint16_t cur_tx_ring;
    /* RX ring info */
    uint16_t first_rx_ring;
    uint16_t last_rx_ring;
    uint16_t cur_rx_ring;
    /* MMAP'd memory */
    void *mem;
    uint32_t memsize;
    struct nmreq req;
    unsigned long long fwd_tx_blocked;
} NetmapInstance;

typedef struct _netmap_context
{
    char *device;
    char *filter;
    int snaplen;
    int timeout;
    int debug;
    NetmapInstance *instances;
    uint32_t intf_count;
    struct sfbpf_program fcode;
    volatile int break_loop;
    DAQ_Stats_t stats;
    DAQ_State state;
    char errbuf[256];
} Netmap_Context_t;

static inline void nminst_inc_rx_ring(NetmapInstance *instance)
{
    instance->cur_rx_ring++;
    if (instance->cur_rx_ring > instance->last_rx_ring)
        instance->cur_rx_ring = instance->first_rx_ring;
}

static inline void nminst_inc_tx_ring(NetmapInstance *instance)
{
    instance->cur_tx_ring++;
    if (instance->cur_tx_ring > instance->last_tx_ring)
        instance->cur_tx_ring = instance->first_tx_ring;
}

static void destroy_instance(NetmapInstance *instance)
{
    if (instance)
    {
        /* Unmap the packet memory region.  If we had a peer, notify them that
            the shared mapping has been freed and that we no longer exist. */
        if (instance->mem)
        {
            munmap(instance->mem, instance->memsize);
            if (instance->peer)
            {
                instance->peer->mem = MAP_FAILED;
                instance->peer->memsize = 0;
            }
        }
        if (instance->peer)
            instance->peer->peer = NULL;
        if (instance->fd != -1)
            close(instance->fd);
        free(instance);
    }
}

static int netmap_close(Netmap_Context_t *nmc)
{
    NetmapInstance *instance;

    if (!nmc)
        return -1;

    /* Free all of the device instances. */
    while ((instance = nmc->instances) != NULL)
    {
        nmc->instances = instance->next;
        if (nmc->debug)
        {
            printf("Netmap instance %s (%d) blocked %llu times on TX while forwarding.\n",
                    instance->req.nr_name, instance->index, instance->fwd_tx_blocked);
        }
        destroy_instance(instance);
    }

    sfbpf_freecode(&nmc->fcode);

    nmc->state = DAQ_STATE_STOPPED;

    return 0;
}

static NetmapInstance *create_instance(const char *device, NetmapInstance *parent, char *errbuf, size_t errlen)
{
    NetmapInstance *instance;
    struct nmreq *req;
    static int index = 0;

    instance = calloc(1, sizeof(NetmapInstance));
    if (!instance)
    {
        snprintf(errbuf, errlen, "%s: Could not allocate a new instance structure.", __FUNCTION__);
        goto err;
    }

    /* Initialize the instance, including an arbitrary and unique device index. */
    instance->mem = MAP_FAILED;
    instance->index = index;
    index++;

    /* Open /dev/netmap for communications to the driver. */
    instance->fd = open("/dev/netmap", O_RDWR);
    if (instance->fd < 0)
    {
        snprintf(errbuf, errlen, "%s: Could not open /dev/netmap: %s (%d)",
                    __FUNCTION__, strerror(errno), errno);
        goto err;
    }

    /* Initialize the netmap request object. */
    req = &instance->req;
    strncpy(req->nr_name, device, sizeof(req->nr_name));
    req->nr_version = NETMAP_API;
    req->nr_ringid = 0;
#if NETMAP_API >= 11
    req->nr_flags = NR_REG_ALL_NIC;
#endif

    return instance;

err:
    destroy_instance(instance);
    return NULL;
}

static int create_bridge(Netmap_Context_t *nmc, const char *device_name1, const char *device_name2)
{
    NetmapInstance *instance, *peer1, *peer2;

    peer1 = peer2 = NULL;
    for (instance = nmc->instances; instance; instance = instance->next)
    {
        if (!strcmp(instance->req.nr_name, device_name1))
            peer1 = instance;
        else if (!strcmp(instance->req.nr_name, device_name2))
            peer2 = instance;
    }

    if (!peer1 || !peer2)
        return DAQ_ERROR_NODEV;

    peer1->peer = peer2;
    peer2->peer = peer1;

    return DAQ_SUCCESS;
}

static int start_instance(Netmap_Context_t *nmc, NetmapInstance *instance)
{
    if (ioctl(instance->fd, NIOCREGIF, &instance->req))
    {
        DPE(nmc->errbuf, "%s: Netmap registration for %s failed: %s (%d)",
                __FUNCTION__, instance->req.nr_name, strerror(errno), errno);
        return DAQ_ERROR;
    }

    /* Only mmap the packet memory region for the first interface in a pair. */
    if (instance->peer && instance->peer->mem != MAP_FAILED)
    {
        instance->memsize = instance->peer->memsize;
        instance->mem = instance->peer->mem;
    }
    else
    {
        instance->memsize = instance->req.nr_memsize;
        instance->mem = mmap(0, instance->memsize, PROT_WRITE | PROT_READ, MAP_SHARED, instance->fd, 0);
        if (instance->mem == MAP_FAILED)
        {
            DPE(nmc->errbuf, "%s: Could not MMAP the buffer memory region for %s: %s (%d)",
                    __FUNCTION__, instance->req.nr_name, strerror(errno), errno);
            return DAQ_ERROR;
        }
    }

    instance->nifp = NETMAP_IF(instance->mem, instance->req.nr_offset);

    instance->first_tx_ring = 0;
    instance->first_rx_ring = 0;
    instance->last_tx_ring = instance->req.nr_tx_rings - 1;
    instance->last_rx_ring = instance->req.nr_rx_rings - 1;

    if (nmc->debug)
    {
        struct netmap_ring *ring;
        int i;

        printf("[%s]\n", instance->req.nr_name);
        printf("  nr_tx_slots: %u\n", instance->req.nr_tx_slots);
        printf("  nr_rx_slots: %u\n", instance->req.nr_rx_slots);
        printf("  nr_tx_rings: %hu\n", instance->req.nr_tx_rings);
        for (i = instance->first_tx_ring; i <= instance->last_tx_ring; i++)
        {
            ring = NETMAP_TXRING(instance->nifp, i);
            printf("  [TX Ring %d]\n", i);
            printf("    buf_ofs = %zu\n", ring->buf_ofs);
            printf("    num_slots = %u\n", ring->num_slots);
            printf("    nr_buf_size = %u\n", ring->nr_buf_size);
            printf("    flags = 0x%x\n", ring->flags);
        }
        printf("  nr_rx_rings: %hu\n", instance->req.nr_rx_rings);
        for (i = instance->first_rx_ring; i <= instance->last_rx_ring; i++)
        {
            ring = NETMAP_RXRING(instance->nifp, i);
            printf("  [RX Ring %d]\n", i);
            printf("    buf_ofs = %zu\n", ring->buf_ofs);
            printf("    num_slots = %u\n", ring->num_slots);
            printf("    nr_buf_size = %u\n", ring->nr_buf_size);
            printf("    flags = 0x%x\n", ring->flags);
        }
        printf("  memsize:     %u\n", instance->memsize);
        printf("  index:       %d\n", instance->index);
    }

    return DAQ_SUCCESS;
}

static int netmap_daq_initialize(const DAQ_Config_t * config, void **ctxt_ptr, char *errbuf, size_t errlen)
{
    Netmap_Context_t *nmc;
    NetmapInstance *instance;
    DAQ_Dict *entry;
    char intf[IFNAMSIZ];
    uint32_t num_intfs = 0;
    size_t len;
    char *name1, *name2, *dev;
    int rval = DAQ_ERROR;

    nmc = calloc(1, sizeof(Netmap_Context_t));
    if (!nmc)
    {
        snprintf(errbuf, errlen, "%s: Couldn't allocate memory for the new Netmap context!", __FUNCTION__);
        rval = DAQ_ERROR_NOMEM;
        goto err;
    }

    nmc->device = strdup(config->name);
    if (!nmc->device)
    {
        snprintf(errbuf, errlen, "%s: Couldn't allocate memory for the device string!", __FUNCTION__);
        rval = DAQ_ERROR_NOMEM;
        goto err;
    }

    nmc->snaplen = config->snaplen;
    nmc->timeout = (config->timeout > 0) ? (int) config->timeout : -1;

    dev = nmc->device;
    if (*dev == ':' || ((len = strlen(dev)) > 0 && *(dev + len - 1) == ':') || 
            (config->mode == DAQ_MODE_PASSIVE && strstr(dev, "::")))
    {
        snprintf(errbuf, errlen, "%s: Invalid interface specification: '%s'!", __FUNCTION__, nmc->device);
        goto err;
    }

    while (*dev != '\0')
    {
        len = strcspn(dev, ":");
        if (len >= sizeof(intf))
        {
            snprintf(errbuf, errlen, "%s: Interface name too long! (%zu)", __FUNCTION__, len);
            goto err;
        }
        if (len != 0)
        {
            nmc->intf_count++;
            if (nmc->intf_count >= NETMAP_MAX_INTERFACES)
            {
                snprintf(errbuf, errlen, "%s: Using more than %d interfaces is not supported!",
                            __FUNCTION__, NETMAP_MAX_INTERFACES);
                goto err;
            }
            snprintf(intf, len + 1, "%s", dev);
            instance = create_instance(intf, nmc->instances, errbuf, errlen);
            if (!instance)
                goto err;

            instance->next = nmc->instances;
            nmc->instances = instance;
            num_intfs++;
            if (config->mode != DAQ_MODE_PASSIVE)
            {
                if (num_intfs == 2)
                {
                    name1 = nmc->instances->next->req.nr_name;
                    name2 = nmc->instances->req.nr_name;

                    if (create_bridge(nmc, name1, name2) != DAQ_SUCCESS)
                    {
                        snprintf(errbuf, errlen, "%s: Couldn't create the bridge between %s and %s!",
                                    __FUNCTION__, name1, name2);
                        goto err;
                    }
                    num_intfs = 0;
                }
                else if (num_intfs > 2)
                    break;
            }
        }
        else
            len = 1;
        dev += len;
    }

    /* If there are any leftover unbridged interfaces and we're not in Passive mode, error out. */
    if (!nmc->instances || (config->mode != DAQ_MODE_PASSIVE && num_intfs != 0))
    {
        snprintf(errbuf, errlen, "%s: Invalid interface specification: '%s'!",
                    __FUNCTION__, nmc->device);
        goto err;
    }

    /* Initialize other default configuration values. */
    nmc->debug = 0;

    /* Import the configuration dictionary requests. */
    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, "debug"))
            nmc->debug = 1;
    }

    nmc->state = DAQ_STATE_INITIALIZED;

    *ctxt_ptr = nmc;
    return DAQ_SUCCESS;

err:
    if (nmc)
    {
        netmap_close(nmc);
        if (nmc->device)
            free(nmc->device);
        free(nmc);
    }
    return rval;
}

static int netmap_daq_set_filter(void *handle, const char *filter)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;
    struct sfbpf_program fcode;

    if (nmc->filter)
        free(nmc->filter);

    nmc->filter = strdup(filter);
    if (!nmc->filter)
    {
        DPE(nmc->errbuf, "%s: Couldn't allocate memory for the filter string!", __FUNCTION__);
        return DAQ_ERROR;
    }

    if (sfbpf_compile(nmc->snaplen, DLT_EN10MB, &fcode, nmc->filter, 1, 0) < 0)
    {
        DPE(nmc->errbuf, "%s: BPF state machine compilation failed!", __FUNCTION__);
        return DAQ_ERROR;
    }

    sfbpf_freecode(&nmc->fcode);
    nmc->fcode.bf_len = fcode.bf_len;
    nmc->fcode.bf_insns = fcode.bf_insns;

    return DAQ_SUCCESS;
}

static int netmap_daq_start(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;
    NetmapInstance *instance;

    for (instance = nmc->instances; instance; instance = instance->next)
    {
        if (start_instance(nmc, instance) != DAQ_SUCCESS)
            return DAQ_ERROR;
    }

    memset(&nmc->stats, 0, sizeof(DAQ_Stats_t));;

    nmc->state = DAQ_STATE_STARTED;

    return DAQ_SUCCESS;
}

static const DAQ_Verdict verdict_translation_table[MAX_DAQ_VERDICT] = {
    DAQ_VERDICT_PASS,       /* DAQ_VERDICT_PASS */
    DAQ_VERDICT_BLOCK,      /* DAQ_VERDICT_BLOCK */
    DAQ_VERDICT_PASS,       /* DAQ_VERDICT_REPLACE */
    DAQ_VERDICT_PASS,       /* DAQ_VERDICT_WHITELIST */
    DAQ_VERDICT_BLOCK,      /* DAQ_VERDICT_BLACKLIST */
    DAQ_VERDICT_PASS,       /* DAQ_VERDICT_IGNORE */
    DAQ_VERDICT_BLOCK       /* DAQ_VERDICT_RETRY */
};

static int netmap_daq_acquire(void *handle, int cnt, DAQ_Analysis_Func_t callback, DAQ_Meta_Func_t metaback, void *user)
{
    struct pollfd pfd[NETMAP_MAX_INTERFACES];
    struct netmap_ring *rx_ring, *tx_ring;
    struct netmap_slot *rx_slot, *tx_slot;
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;
    NetmapInstance *instance, *peer;
    DAQ_PktHdr_t daqhdr;
    DAQ_Verdict verdict;
    const uint8_t *data;
    uint32_t i, rx_cur, tx_cur;
    uint16_t len, start_rx_ring, start_tx_ring;
    int got_one, ignored_one;
    int ret, c = 0;

    while (c < cnt || cnt <= 0)
    {
        got_one = 0;
        ignored_one = 0;

        for (instance = nmc->instances; instance; instance = instance->next)
        {
            start_rx_ring = instance->cur_rx_ring;
            do
            {
                /* Has breakloop() been called? */
                if (nmc->break_loop)
                {
                    nmc->break_loop = 0;
                    return 0;
                }

                rx_ring = NETMAP_RXRING(instance->nifp, instance->cur_rx_ring);
                if (nm_ring_empty(rx_ring))
                {
                    nminst_inc_rx_ring(instance);
                    continue;
                }

                rx_cur = rx_ring->cur;
                rx_slot = &rx_ring->slot[rx_cur];
                len = rx_slot->len;

                data = (uint8_t *) NETMAP_BUF(rx_ring, rx_slot->buf_idx);

                verdict = DAQ_VERDICT_PASS;

                /* If we blocked on forwarding previously, it means we know we
                    already want to send this packet, so attempt to do so
                    immediately. */
                if (instance->flags & NMINST_FWD_BLOCKED)
                {
                    instance->flags &= ~NMINST_FWD_BLOCKED;
                    got_one = 1;
                    goto send_packet;
                }

                nmc->stats.hw_packets_received++;

                if (nmc->fcode.bf_insns && sfbpf_filter(nmc->fcode.bf_insns, data, len, len) == 0)
                {
                    ignored_one = 1;
                    nmc->stats.packets_filtered++;
                    goto send_packet;
                }
                got_one = 1;

                daqhdr.ts = rx_ring->ts;
                daqhdr.caplen = len;
                daqhdr.pktlen = len;
                daqhdr.ingress_index = instance->index;
                daqhdr.egress_index = instance->peer ? instance->peer->index : DAQ_PKTHDR_UNKNOWN;
                daqhdr.ingress_group = DAQ_PKTHDR_UNKNOWN;
                daqhdr.egress_group = DAQ_PKTHDR_UNKNOWN;
                daqhdr.flags = 0;
                daqhdr.opaque = 0;
                daqhdr.priv_ptr = NULL;
                daqhdr.address_space_id = 0;

                if (callback)
                {
                    verdict = callback(user, &daqhdr, data);
                    if (verdict >= MAX_DAQ_VERDICT)
                        verdict = DAQ_VERDICT_PASS;
                    nmc->stats.verdicts[verdict]++;
                    verdict = verdict_translation_table[verdict];
                }
                nmc->stats.packets_received++;
                c++;
send_packet:
                if (verdict == DAQ_VERDICT_PASS && instance->peer)
                {
                    uint32_t tx_buf_idx;
                    int sent = 0;

                    peer = instance->peer;
                    start_tx_ring = peer->cur_tx_ring;

                    do
                    {
                        tx_ring = NETMAP_TXRING(peer->nifp, peer->cur_tx_ring);
                        nminst_inc_tx_ring(peer);
                        if (nm_ring_empty(tx_ring))
                            continue;
                        /* Swap the RX buffer we want to forward with the next
                           unused buffer in the peer's TX ring. */
                        tx_cur = tx_ring->cur;
                        tx_slot = &tx_ring->slot[tx_cur];
                        tx_buf_idx = tx_slot->buf_idx;
                        tx_slot->len = len;
                        tx_slot->buf_idx = rx_slot->buf_idx;
                        rx_slot->buf_idx = tx_buf_idx;
                        /* Report the buffer change. */
                        tx_slot->flags |= NS_BUF_CHANGED;
                        rx_slot->flags |= NS_BUF_CHANGED;

                        tx_ring->cur = nm_ring_next(tx_ring, tx_cur);
#if NETMAP_API >= 10
                        tx_ring->head = tx_ring->cur;
#else
                        tx_ring->avail--;
#endif
                        sent = 1;
                    } while (peer->cur_tx_ring != start_tx_ring && !sent);

                    /* If we couldn't find a TX slot to use, hold on to this packet and
                        wait for TX slots to become available. */
                    if (sent == 0)
                    {
                        instance->fwd_tx_blocked++;
                        instance->flags |= NMINST_FWD_BLOCKED;
                        peer->flags |= NMINST_TX_BLOCKED;
                        goto poll;
                    }
                    else
                        peer->flags &= ~NMINST_TX_BLOCKED;
                }

                rx_ring->cur = nm_ring_next(rx_ring, rx_cur);
#if NETMAP_API >= 10
                rx_ring->head = rx_ring->cur;
#else
                rx_ring->avail--;
#endif

                /* Increment the current RX ring pointer on successfully completed processing. */
                nminst_inc_rx_ring(instance);

            } while (instance->cur_rx_ring != start_rx_ring);
        }

        if (!got_one && !ignored_one)
        {
poll:
            for (i = 0, instance = nmc->instances; instance; i++, instance = instance->next)
            {
                pfd[i].fd = instance->fd;
                pfd[i].events = 0;
                pfd[i].revents = 0;

                /* If I blocked on TX, wait for some TX to complete, otherwise,
                    if I didn't block on forwarding, wait for new packets to arrive. */
                if (instance->flags & NMINST_TX_BLOCKED)
                    pfd[i].events |= POLLOUT;
                else if (!(instance->flags & NMINST_FWD_BLOCKED))
                    pfd[i].events |= POLLIN;
            }
            ret = poll(pfd, nmc->intf_count, nmc->timeout);

            /* If we were interrupted by a signal, start the loop over.
                The user should call daq_breakloop to actually exit. */
            if (ret < 0 && errno != EINTR)
            {
                DPE(nmc->errbuf, "%s: Poll failed: %s (%d)", __FUNCTION__, strerror(errno), errno);
                return DAQ_ERROR;
            }
            /* If the poll times out, return control to the caller. */
            if (ret == 0)
                break;
            /* If some number of of sockets have events returned, check them all for badness. */
            if (ret > 0)
            {
                for (i = 0; i < nmc->intf_count; i++)
                {
                    if (pfd[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                    {
                        if (pfd[i].revents & POLLHUP)
                            DPE(nmc->errbuf, "%s: Hang-up on a packet socket", __FUNCTION__);
                        else if (pfd[i].revents & POLLERR)
                            DPE(nmc->errbuf, "%s: Encountered error condition on a packet socket", __FUNCTION__);
                        else if (pfd[i].revents & POLLNVAL)
                            DPE(nmc->errbuf, "%s: Invalid polling request on a packet socket", __FUNCTION__);
                        return DAQ_ERROR;
                    }
                }
            }
        }
    }

    return 0;
}

static int netmap_daq_inject(void *handle, const DAQ_PktHdr_t *hdr,
                             const uint8_t *packet_data, uint32_t len,
                             int reverse)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;
    NetmapInstance *instance;
    struct netmap_ring *tx_ring;
    struct netmap_slot *tx_slot;
    uint32_t tx_buf_idx, tx_cur;
    uint16_t start_tx_ring;

    /* Find the instance that the packet was received on. */
    for (instance = nmc->instances; instance; instance = instance->next)
    {
        if (instance->index == hdr->ingress_index)
            break;
    }

    if (!instance)
    {
        DPE(nmc->errbuf, "%s: Unrecognized ingress interface specified: %u",
                __FUNCTION__, hdr->ingress_index);
        return DAQ_ERROR_NODEV;
    }

    if (!reverse && !(instance = instance->peer))
    {
        DPE(nmc->errbuf, "%s: Specified ingress interface (%u) has no peer for forward injection.",
                __FUNCTION__, hdr->ingress_index);
        return DAQ_ERROR_NODEV;
    }

    /* Find a TX ring with space to send on. */
    start_tx_ring = instance->cur_tx_ring;
    do
    {
        tx_ring = NETMAP_TXRING(instance->nifp, instance->cur_tx_ring);
        nminst_inc_tx_ring(instance);
        if (nm_ring_empty(tx_ring))
            continue;

        tx_cur = tx_ring->cur;
        tx_slot = &tx_ring->slot[tx_cur];
        tx_buf_idx = tx_slot->buf_idx;
        tx_slot->len = len;
        memcpy(NETMAP_BUF(tx_ring, tx_buf_idx), packet_data, len);

        tx_ring->cur = nm_ring_next(tx_ring, tx_cur);
#if NETMAP_API >= 10
        tx_ring->head = tx_ring->cur;
#else
        tx_ring->avail--;
#endif
        nmc->stats.packets_injected++;

        return DAQ_SUCCESS;
    } while (instance->cur_tx_ring != start_tx_ring);

    /* If we got here, it means we couldn't find an available TX slot, so tell the user to try again. */
    DPE(nmc->errbuf, "%s: Could not find an available TX slot.  Try again.", __FUNCTION__);
    return DAQ_ERROR_AGAIN;
}

static int netmap_daq_breakloop(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    nmc->break_loop = 1;

    return DAQ_SUCCESS;
}

static int netmap_daq_stop(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    netmap_close(nmc);

    return DAQ_SUCCESS;
}

static void netmap_daq_shutdown(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    netmap_close(nmc);
    if (nmc->device)
        free(nmc->device);
    if (nmc->filter)
        free(nmc->filter);
    free(nmc);
}

static DAQ_State netmap_daq_check_status(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

	return nmc->state;
}

static int netmap_daq_get_stats(void *handle, DAQ_Stats_t * stats)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    memcpy(stats, &nmc->stats, sizeof(DAQ_Stats_t));

    return DAQ_SUCCESS;
}

static void netmap_daq_reset_stats(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    memset(&nmc->stats, 0, sizeof(DAQ_Stats_t));;
}

static int netmap_daq_get_snaplen(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    return nmc->snaplen;
}

static uint32_t netmap_daq_get_capabilities(void *handle)
{
    return DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT |
            DAQ_CAPA_UNPRIV_START | DAQ_CAPA_BREAKLOOP | DAQ_CAPA_BPF |
            DAQ_CAPA_DEVICE_INDEX;
}

static int netmap_daq_get_datalink_type(void *handle)
{
    return DLT_EN10MB;
}

static const char *netmap_daq_get_errbuf(void *handle)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    return nmc->errbuf;
}

static void netmap_daq_set_errbuf(void *handle, const char *string)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;

    if (!string)
        return;

    DPE(nmc->errbuf, "%s", string);
}

static int netmap_daq_get_device_index(void *handle, const char *device)
{
    Netmap_Context_t *nmc = (Netmap_Context_t *) handle;
    NetmapInstance *instance;

    for (instance = nmc->instances; instance; instance = instance->next)
    {
        if (!strcmp(device, instance->req.nr_name))
            return instance->index;
    }

    return DAQ_ERROR_NODEV;
}

#ifdef BUILDING_SO
DAQ_SO_PUBLIC const DAQ_Module_t DAQ_MODULE_DATA =
#else
const DAQ_Module_t netmap_daq_module_data =
#endif
{
    /* .api_version = */ DAQ_API_VERSION,
    /* .module_version = */ DAQ_NETMAP_VERSION,
    /* .name = */ "netmap",
    /* .type = */ DAQ_TYPE_INLINE_CAPABLE | DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_MULTI_INSTANCE,
    /* .initialize = */ netmap_daq_initialize,
    /* .set_filter = */ netmap_daq_set_filter,
    /* .start = */ netmap_daq_start,
    /* .acquire = */ netmap_daq_acquire,
    /* .inject = */ netmap_daq_inject,
    /* .breakloop = */ netmap_daq_breakloop,
    /* .stop = */ netmap_daq_stop,
    /* .shutdown = */ netmap_daq_shutdown,
    /* .check_status = */ netmap_daq_check_status,
    /* .get_stats = */ netmap_daq_get_stats,
    /* .reset_stats = */ netmap_daq_reset_stats,
    /* .get_snaplen = */ netmap_daq_get_snaplen,
    /* .get_capabilities = */ netmap_daq_get_capabilities,
    /* .get_datalink_type = */ netmap_daq_get_datalink_type,
    /* .get_errbuf = */ netmap_daq_get_errbuf,
    /* .set_errbuf = */ netmap_daq_set_errbuf,
    /* .get_device_index = */ netmap_daq_get_device_index,
    /* .modify_flow = */ NULL,
    /* .hup_prep = */ NULL,
    /* .hup_apply = */ NULL,
    /* .hup_post = */ NULL,
    /* .dp_add_dc = */ NULL
};
