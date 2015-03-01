/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2010-2013 Sourcefire, Inc.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "daq_api.h"
#include "sfbpf.h"

#define DAQ_AFPACKET_VERSION 5

#define AF_PACKET_DEFAULT_BUFFER_SIZE   128
#define AF_PACKET_MAX_INTERFACES    32

union thdr
{
    struct tpacket2_hdr *h2;
    uint8_t *raw;
};

typedef struct _af_packet_entry
{
    struct _af_packet_entry *next;
    union thdr hdr;
} AFPacketEntry;

typedef struct _af_packet_ring
{
    struct tpacket_req layout;
    unsigned int size;
    void *start;
    AFPacketEntry *entries;
    AFPacketEntry *cursor;
} AFPacketRing;

typedef struct _af_packet_instance
{
    struct _af_packet_instance *next;
    int fd;
    unsigned tp_version;
    unsigned tp_hdrlen;
    void *buffer;
    AFPacketRing rx_ring;
    AFPacketRing tx_ring;
    char *name;
    int index;
    struct _af_packet_instance *peer;
    struct sockaddr_ll sll;
} AFPacketInstance;

typedef struct _afpacket_context
{
    char *device;
    char *filter;
    int snaplen;
    int timeout;
    uint32_t size;
    int debug;
    AFPacketInstance *instances;
    uint32_t intf_count;
    struct sfbpf_program fcode;
    volatile int break_loop;
    DAQ_Stats_t stats;
    DAQ_State state;
    char errbuf[256];
} AFPacket_Context_t;

/* VLAN defintions stolen from LibPCAP's vlan.h. */
struct vlan_tag {
    u_int16_t   vlan_tpid;      /* ETH_P_8021Q */
    u_int16_t   vlan_tci;       /* VLAN TCI */
};
#define VLAN_TAG_LEN    4

static const int vlan_offset = 2 * ETH_ALEN;

static int bind_instance_interface(AFPacket_Context_t *afpc, AFPacketInstance *instance)
{
    struct sockaddr_ll sll;
    int err;
    socklen_t errlen = sizeof(err);

    /* Bind to the specified device so we only see packets from it. */
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = instance->index;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(instance->fd, (struct sockaddr *) &sll, sizeof(sll)) == -1)
    {
        DPE(afpc->errbuf, "%s: bind(%s): %s\n", __FUNCTION__, instance->name, strerror(errno));
        return DAQ_ERROR;
    }

    /* Any pending errors, e.g., network is down? */
    if (getsockopt(instance->fd, SOL_SOCKET, SO_ERROR, &err, &errlen) || err)
    {
        DPE(afpc->errbuf, "%s: getsockopt: %s", __FUNCTION__, strerror(errno));
        return DAQ_ERROR;
    }

    return DAQ_SUCCESS;
}

static int set_up_ring(AFPacket_Context_t *afpc, AFPacketInstance *instance, AFPacketRing *ring)
{
    unsigned int idx, block, block_offset, frame, frame_offset;

    /* Allocate a ring to hold packet pointers. */
    ring->entries = calloc(ring->layout.tp_frame_nr, sizeof(AFPacketEntry));
    if (!ring->entries)
    {
        DPE(afpc->errbuf, "%s: Could not allocate ring buffer entries for device %s!", __FUNCTION__, instance->name);
        return DAQ_ERROR_NOMEM;
    }

    /* Set up the buffer entry pointers in the ring. */
    idx = 0;
    for (block = 0; block < ring->layout.tp_block_nr; block++)
    {
        block_offset = block * ring->layout.tp_block_size;
        for (frame = 0; frame < (ring->layout.tp_block_size / ring->layout.tp_frame_size) && idx < ring->layout.tp_frame_nr; frame++)
        {
            frame_offset = frame * ring->layout.tp_frame_size;
            ring->entries[idx].hdr.raw = (uint8_t *) ring->start + block_offset + frame_offset;
            ring->entries[idx].next = &ring->entries[idx + 1];
            idx++;
        }
    }
    /* Make this a circular buffer ... a RING if you will! */
    ring->entries[ring->layout.tp_frame_nr - 1].next = &ring->entries[0];
    /* Initialize our entry point into the ring as the first buffer entry. */
    ring->cursor = &ring->entries[0];

    return DAQ_SUCCESS;
}

static void destroy_instance(AFPacketInstance *instance)
{
    unsigned int ringsize;
    struct tpacket_req req;

    if (instance)
    {
        if (instance->fd != -1)
        {
            /* Destroy the userspace RX ring. */
            if (instance->rx_ring.entries)
            {
                free(instance->rx_ring.entries);
                instance->rx_ring.entries = NULL;
            }
            /* Destroy the userspace TX ring. */
            if (instance->tx_ring.entries)
            {
                free(instance->tx_ring.entries);
                instance->tx_ring.entries = NULL;
            }
            /* Unmap the kernel packet ring. */
            if (instance->buffer != MAP_FAILED)
            {
                ringsize = instance->rx_ring.size + instance->tx_ring.size;
                munmap(instance->buffer, ringsize);
                instance->buffer = MAP_FAILED;
            }
            /* Tell the kernel to destroy the rings. */
            memset(&req, 0, sizeof(req));
            setsockopt(instance->fd, SOL_PACKET, PACKET_RX_RING, (void *) &req, sizeof(req));
            setsockopt(instance->fd, SOL_PACKET, PACKET_TX_RING, (void *) &req, sizeof(req));
            close(instance->fd);
        }
        if (instance->name)
        {
            free(instance->name);
            instance->name = NULL;
        }
        free(instance);
    }
}


static int iface_get_arptype(AFPacketInstance *instance)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, instance->name, sizeof(ifr.ifr_name));

    if (ioctl(instance->fd, SIOCGIFHWADDR, &ifr) == -1)
    {
        if (errno == ENODEV)
        {
            return DAQ_ERROR_NODEV;
        }
        return DAQ_ERROR;
    }

    return ifr.ifr_hwaddr.sa_family;
}

static AFPacketInstance *create_instance(const char *device, char *errbuf, size_t errlen)
{
    AFPacketInstance *instance = NULL;
    struct ifreq ifr;

    instance = calloc(1, sizeof(AFPacketInstance));
    if (!instance)
    {
        snprintf(errbuf, errlen, "%s: Could not allocate a new instance structure.", __FUNCTION__);
        goto err;
    }
    instance->buffer = MAP_FAILED;

    if ((instance->name = strdup(device)) == NULL)
    {
        snprintf(errbuf, errlen, "%s: Could not allocate a copy of the device name.", __FUNCTION__);
        goto err;;
    }

    /* Open the PF_PACKET raw socket to receive all network traffic completely unmodified. */
    instance->fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (instance->fd == -1)
    {
        snprintf(errbuf, errlen, "%s: Could not open the PF_PACKET socket: %s", __FUNCTION__, strerror(errno));
        goto err;
    }

    /* Find the device index of the specified interface. */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
    if (ioctl(instance->fd, SIOCGIFINDEX, &ifr) == -1)
    {
        snprintf(errbuf, errlen, "%s: Could not find index for device %s", __FUNCTION__, instance->name);
        goto err;
    }
    instance->index = ifr.ifr_ifindex;

    /* Initialize the sockaddr for this instance's interface for later injection/forwarding use. */
    instance->sll.sll_family = AF_PACKET;
    instance->sll.sll_ifindex = instance->index;
    instance->sll.sll_protocol = htons(ETH_P_ALL);

    return instance;

err:
    destroy_instance(instance);
    return NULL;
}

/* The function below was heavily influenced by LibPCAP's pcap-linux.c.  Thanks! */
static int determine_version(AFPacket_Context_t *afpc, AFPacketInstance *instance)
{
    socklen_t len;
    int val;

    /* Probe whether kernel supports TPACKET_V2 */
    val = TPACKET_V2;
    len = sizeof(val);
    if (getsockopt(instance->fd, SOL_PACKET, PACKET_HDRLEN, &val, &len) < 0)
    {
        DPE(afpc->errbuf, "Couldn't retrieve TPACKET_V2 header length: %s", strerror(errno));
        return -1;
    }
    instance->tp_hdrlen = val;

    /* Tell the kernel to use TPACKET_V2 */
    val = TPACKET_V2;
    if (setsockopt(instance->fd, SOL_PACKET, PACKET_VERSION, &val, sizeof(val)) < 0)
    {
        DPE(afpc->errbuf, "Couldn't activate TPACKET_V2 on packet socket: %s", strerror(errno));
        return -1;
    }
    instance->tp_version = TPACKET_V2;

    /* Reserve space for VLAN tag reconstruction */
    val = VLAN_TAG_LEN;
    if (setsockopt(instance->fd, SOL_PACKET, PACKET_RESERVE, &val, sizeof(val)) < 0)
    {
        DPE(afpc->errbuf, "Couldn't set up a %d-byte reservation packet socket: %s", val, strerror(errno));
        return -1;
    }

    if (afpc->debug)
    {
        printf("Version: %u\n", instance->tp_version);
        printf("Header Length: %u\n", instance->tp_hdrlen);
    }

    return DAQ_SUCCESS;
}

static int calculate_layout(AFPacket_Context_t *afpc, struct tpacket_req *layout, unsigned int tp_hdrlen, int order)
{
    unsigned int tp_hdrlen_sll, netoff, frames_per_block;

    /* Calculate the frame size and minimum block size required. */
    tp_hdrlen_sll = TPACKET_ALIGN(tp_hdrlen) + sizeof(struct sockaddr_ll);
    netoff = TPACKET_ALIGN(tp_hdrlen_sll + ETH_HLEN) + VLAN_TAG_LEN;
    layout->tp_frame_size = TPACKET_ALIGN(netoff - ETH_HLEN + afpc->snaplen);
    layout->tp_block_size = getpagesize() << order;
    while (layout->tp_block_size < layout->tp_frame_size)
        layout->tp_block_size <<= 1;
    frames_per_block = layout->tp_block_size / layout->tp_frame_size;
    if (frames_per_block == 0)
    {
        DPE(afpc->errbuf, "%s: Invalid frames per block (%u/%u) for %s",
                __FUNCTION__, layout->tp_block_size, layout->tp_frame_size, afpc->device);
        return DAQ_ERROR;
    }

    /* Find the total number of frames required to amount to the requested per-interface memory.
        Then find the number of blocks required to hold those packet buffer frames. */
    layout->tp_frame_nr = afpc->size / layout->tp_frame_size;
    layout->tp_block_nr = layout->tp_frame_nr / frames_per_block;
    /* afpc->layout.tp_frame_nr is requested to match frames_per_block*n_blocks */
    layout->tp_frame_nr = layout->tp_block_nr * frames_per_block;
    if (afpc->debug)
    {
        printf("AFPacket Layout:\n");
        printf("  Frame Size: %u\n", layout->tp_frame_size);
        printf("  Frames:     %u\n", layout->tp_frame_nr);
        printf("  Block Size: %u (Order %d)\n", layout->tp_block_size, order);
        printf("  Blocks:     %u\n", layout->tp_block_nr);
    }

    return DAQ_SUCCESS;
}

#define DEFAULT_ORDER 3
static int create_ring(AFPacket_Context_t *afpc, AFPacketInstance *instance, AFPacketRing *ring, int optname)
{
    int rc, order;

    /* Starting with page allocations of order 3, try to allocate an RX ring in the kernel. */
    for (order = DEFAULT_ORDER; order >= 0; order--)
    {
        if (calculate_layout(afpc, &ring->layout, instance->tp_hdrlen, order))
            return DAQ_ERROR;

        /* Ask the kernel to create the ring. */
        rc = setsockopt(instance->fd, SOL_PACKET, optname, (void*) &ring->layout, sizeof(struct tpacket_req));
        if (rc)
        {
            if (errno == ENOMEM)
            {
                if (afpc->debug)
                    printf("%s: Allocation of kernel packet ring failed with order %d, retrying...\n", instance->name, order);
                continue;
            }
            DPE(afpc->errbuf, "%s: Couldn't create kernel ring on packet socket: %s",
                    __FUNCTION__, strerror(errno));
            return DAQ_ERROR;
        }
        /* Store the total ring size for later. */
        ring->size = ring->layout.tp_block_size * ring->layout.tp_block_nr;
        if (afpc->debug)
            printf("Created a ring of type %d with total size of %u\n", optname, ring->size);
        return DAQ_SUCCESS;
    }

    /* If we got here, it means we failed allocation on order 0. */
    DPE(afpc->errbuf, "%s: Couldn't allocate enough memory for the kernel packet ring!", instance->name);
    return DAQ_ERROR;
}

static int mmap_rings(AFPacket_Context_t *afpc, AFPacketInstance *instance)
{
    unsigned int ringsize;

    /* Map the ring into userspace. */
    ringsize = instance->rx_ring.size + instance->tx_ring.size;
    instance->buffer = mmap(0, ringsize, PROT_READ | PROT_WRITE, MAP_SHARED, instance->fd, 0);
    if (instance->buffer == MAP_FAILED)
    {
        DPE(afpc->errbuf, "%s: Could not MMAP the ring: %s", __FUNCTION__, strerror(errno));
        return DAQ_ERROR;
    }
    instance->rx_ring.start = instance->buffer;
    instance->tx_ring.start = (uint8_t *) instance->buffer + instance->tx_ring.size;

    return DAQ_SUCCESS;
}

static int start_instance(AFPacket_Context_t *afpc, AFPacketInstance *instance)
{
    struct packet_mreq mr;
    int arptype;

    /* Bind the RX ring to this interface. */
    if (bind_instance_interface(afpc, instance) != 0)
        return -1;

    /* Turn on promiscuous mode for the device. */
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = instance->index;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(instance->fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
    {
        DPE(afpc->errbuf, "%s: setsockopt: %s", __FUNCTION__, strerror(errno));
        return -1;
    }

    /* Get the link-layer type. */
    arptype = iface_get_arptype(instance);
    if (arptype < 0)
    {
        DPE(afpc->errbuf, "%s: failed to get interface type for device %s: (%d) %s",
                __FUNCTION__, instance->name, errno, strerror(errno));
        return -1;
    }

    if (arptype != ARPHRD_ETHER)
    {
        DPE(afpc->errbuf, "%s: invalid interface type for device %s: %d != %d",
                __FUNCTION__, instance->name, arptype, ARPHRD_ETHER);
        return -1;
    }

    /* Determine which versions of TPACKET the socket supports. */
    if (determine_version(afpc, instance) != DAQ_SUCCESS)
        return -1;

    /* Request the kernel RX ring from af_packet... */
    if (create_ring(afpc, instance, &instance->rx_ring, PACKET_RX_RING) != DAQ_SUCCESS)
        return -1;
    /* ...request the kernel TX ring from af_packet if we're in inline mode... */
    if (instance->peer && create_ring(afpc, instance, &instance->tx_ring, PACKET_TX_RING) != DAQ_SUCCESS)
        return -1;
    /* ...map the memory for the kernel ring(s) into userspace... */
    if (mmap_rings(afpc, instance) != DAQ_SUCCESS)
        return -1;
    /* ...and, finally, set up a userspace ring buffer to represent the kernel RX ring... */
    if (set_up_ring(afpc, instance, &instance->rx_ring) != DAQ_SUCCESS)
        return -1;
    /* ...as well as one for the TX ring if we're in inline mode. */
    if (instance->peer && set_up_ring(afpc, instance, &instance->tx_ring) != DAQ_SUCCESS)
        return -1;

    return 0;
}

static void update_hw_stats(AFPacket_Context_t *afpc)
{
    AFPacketInstance *instance;
    struct tpacket_stats kstats;
    socklen_t len = sizeof (struct tpacket_stats);

    if (afpc->state != DAQ_STATE_STARTED)
        return;

    for (instance = afpc->instances; instance; instance = instance->next)
    {
        memset(&kstats, 0, len);
        if (getsockopt(instance->fd, SOL_PACKET, PACKET_STATISTICS, &kstats, &len) > -1)
        {
            /* The IOCTL adds tp_drops to tp_packets in the returned structure for some mind-boggling reason... */
            afpc->stats.hw_packets_received += kstats.tp_packets - kstats.tp_drops;
            afpc->stats.hw_packets_dropped += kstats.tp_drops;
        }
        else
            fprintf(stderr, "Failed to get stats for %s: %d %s\n", instance->name, errno, strerror(errno));
    }
}

static int af_packet_close(AFPacket_Context_t *afpc)
{
    AFPacketInstance *instance;

    if (!afpc)
        return -1;

    /* Cache the latest hardware stats before stopping. */
    update_hw_stats(afpc);

    while ((instance = afpc->instances) != NULL)
    {
        afpc->instances = instance->next;
        destroy_instance(instance);
    }

    sfbpf_freecode(&afpc->fcode);

    afpc->state = DAQ_STATE_STOPPED;

    return 0;
}

static int create_bridge(AFPacket_Context_t *afpc, const char *device_name1, const char *device_name2)
{
    AFPacketInstance *instance, *peer1, *peer2;

    peer1 = peer2 = NULL;
    for (instance = afpc->instances; instance; instance = instance->next)
    {
        if (!strcmp(instance->name, device_name1))
            peer1 = instance;
        else if (!strcmp(instance->name, device_name2))
            peer2 = instance;
    }

    if (!peer1 || !peer2)
        return DAQ_ERROR_NODEV;

    peer1->peer = peer2;
    peer2->peer = peer1;

    return DAQ_SUCCESS;
}

static void reset_stats(AFPacket_Context_t *afpc)
{
    AFPacketInstance *instance;
    struct tpacket_stats kstats;
    socklen_t len = sizeof (struct tpacket_stats);

    memset(&afpc->stats, 0, sizeof(DAQ_Stats_t));
    /* Just call PACKET_STATISTICS to clear each instance's stats. */
    for (instance = afpc->instances; instance; instance = instance->next)
        getsockopt(instance->fd, SOL_PACKET, PACKET_STATISTICS, &kstats, &len);
}

static int afpacket_daq_initialize(const DAQ_Config_t *config, void **ctxt_ptr, char *errbuf, size_t errlen)
{
    AFPacket_Context_t *afpc;
    AFPacketInstance *instance;
    const char *size_str = NULL;
    char *name1, *name2, *dev;
    char intf[IFNAMSIZ];
    uint32_t size;
    size_t len;
    int num_rings, num_intfs = 0;
    int rval = DAQ_ERROR;
    DAQ_Dict *entry;

    afpc = calloc(1, sizeof(AFPacket_Context_t));
    if (!afpc)
    {
        snprintf(errbuf, errlen, "%s: Couldn't allocate memory for the new AFPacket context!", __FUNCTION__);
        rval = DAQ_ERROR_NOMEM;
        goto err;
    }

    afpc->device = strdup(config->name);
    if (!afpc->device)
    {
        snprintf(errbuf, errlen, "%s: Couldn't allocate memory for the device string!", __FUNCTION__);
        rval = DAQ_ERROR_NOMEM;
        goto err;
    }

    afpc->snaplen = config->snaplen;
    afpc->timeout = (config->timeout > 0) ? (int) config->timeout : -1;

    dev = afpc->device;
    if (*dev == ':' || ((len = strlen(dev)) > 0 && *(dev + len - 1) == ':') || (config->mode == DAQ_MODE_PASSIVE && strstr(dev, "::")))
    {
        snprintf(errbuf, errlen, "%s: Invalid interface specification: '%s'!", __FUNCTION__, afpc->device);
        goto err;
    }

    while (*dev != '\0')
    {
        len = strcspn(dev, ":");
        if (len >= IFNAMSIZ)
        {
            snprintf(errbuf, errlen, "%s: Interface name too long! (%zu)", __FUNCTION__, len);
            goto err;
        }
        if (len != 0)
        {
            afpc->intf_count++;
            if (afpc->intf_count >= AF_PACKET_MAX_INTERFACES)
            {
                snprintf(errbuf, errlen, "%s: Using more than %d interfaces is not supported!", __FUNCTION__, AF_PACKET_MAX_INTERFACES);
                goto err;
            }
            snprintf(intf, len + 1, "%s", dev);
            instance = create_instance(intf, errbuf, errlen);
            if (!instance)
                goto err;

            instance->next = afpc->instances;
            afpc->instances = instance;
            num_intfs++;
            if (config->mode != DAQ_MODE_PASSIVE)
            {
                if (num_intfs == 2)
                {
                    name1 = afpc->instances->next->name;
                    name2 = afpc->instances->name;

                    if (create_bridge(afpc, name1, name2) != DAQ_SUCCESS)
                    {
                        snprintf(errbuf, errlen, "%s: Couldn't create the bridge between %s and %s!", __FUNCTION__, name1, name2);
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
    if (!afpc->instances || (config->mode != DAQ_MODE_PASSIVE && num_intfs != 0))
    {
        snprintf(errbuf, errlen, "%s: Invalid interface specification: '%s'!", __FUNCTION__, afpc->device);
        goto err;
    }

    /*
     * Determine the dimensions of the kernel RX ring(s) to request.
     */
    /* 1. Find the total desired packet buffer memory for all instances. */
    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, "buffer_size_mb"))
            size_str = entry->value;
        else if (!strcmp(entry->key, "debug"))
            afpc->debug = 1;
    }
    /* Fall back to the environment variable. */
    if (!size_str)
        size_str = getenv("AF_PACKET_BUFFER_SIZE");
    if (size_str && strcmp("max", size_str) != 0)
        size = strtoul(size_str, NULL, 10);
    else
        size = AF_PACKET_DEFAULT_BUFFER_SIZE;
    /* The size is specified in megabytes. */
    size = size * 1024 * 1024;

    /* 2. Divide it evenly across the number of rings.  (One per passive interface, two per inline.) */
    num_rings = 0;
    for (instance = afpc->instances; instance; instance = instance->next)
        num_rings += instance->peer ? 2 : 1;
    afpc->size = size / num_rings;

    afpc->state = DAQ_STATE_INITIALIZED;

    *ctxt_ptr = afpc;
    return DAQ_SUCCESS;

err:
    if (afpc)
    {
        af_packet_close(afpc);
        if (afpc->device)
            free(afpc->device);
        free(afpc);
    }
    return rval;
}

static int afpacket_daq_set_filter(void *handle, const char *filter)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;
    struct sfbpf_program fcode;

    if (afpc->filter)
        free(afpc->filter);

    afpc->filter = strdup(filter);
    if (!afpc->filter)
    {
        DPE(afpc->errbuf, "%s: Couldn't allocate memory for the filter string!", __FUNCTION__);
        return DAQ_ERROR;
    }

    if (sfbpf_compile(afpc->snaplen, DLT_EN10MB, &fcode, afpc->filter, 1, 0) < 0)
    {
        DPE(afpc->errbuf, "%s: BPF state machine compilation failed!", __FUNCTION__);
        return DAQ_ERROR;
    }

    sfbpf_freecode(&afpc->fcode);
    afpc->fcode.bf_len = fcode.bf_len;
    afpc->fcode.bf_insns = fcode.bf_insns;

    return DAQ_SUCCESS;
}

static int afpacket_daq_start(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;
    AFPacketInstance *instance;

    for (instance = afpc->instances; instance; instance = instance->next)
    {
        if (start_instance(afpc, instance) != 0)
            return DAQ_ERROR;
    }

    reset_stats(afpc);

    afpc->state = DAQ_STATE_STARTED;

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

static int afpacket_daq_acquire(void *handle, int cnt, DAQ_Analysis_Func_t callback, DAQ_Meta_Func_t metaback, void *user)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;
    AFPacketInstance *instance;
    DAQ_PktHdr_t daqhdr;
    DAQ_Verdict verdict;
    union thdr hdr;
    struct pollfd pfd[AF_PACKET_MAX_INTERFACES];
    const uint8_t *data;
    uint32_t i;
    int got_one, ignored_one;
    int ret, c = 0;
    unsigned int tp_len, tp_mac, tp_snaplen, tp_sec, tp_usec;

    while (c < cnt || cnt <= 0)
    {
        got_one = 0;
        ignored_one = 0;
        for (instance = afpc->instances; instance; instance = instance->next)
        {
            /* Has breakloop() been called? */
            if (afpc->break_loop)
            {
                afpc->break_loop = 0;
                return 0;
            }

            hdr = instance->rx_ring.cursor->hdr;
            if (instance->tp_version == TPACKET_V2 && (hdr.h2->tp_status & TP_STATUS_USER))
            {
                switch (instance->tp_version)
                {
                    case TPACKET_V2:
                        tp_len = hdr.h2->tp_len;
                        tp_mac = hdr.h2->tp_mac;
                        tp_snaplen = hdr.h2->tp_snaplen;
                        tp_sec = hdr.h2->tp_sec;
                        tp_usec = hdr.h2->tp_nsec / 1000;
                        break;

                    default:
                        DPE(afpc->errbuf, "%s: Unknown TPACKET version: %u!", __FUNCTION__, instance->tp_version);
                        return DAQ_ERROR;
                }
                if (tp_mac + tp_snaplen > instance->rx_ring.layout.tp_frame_size)
                {
                    DPE(afpc->errbuf, "%s: Corrupted frame on kernel ring (MAC offset %u + CapLen %u > FrameSize %d)",
                        __FUNCTION__, tp_mac, tp_snaplen, instance->rx_ring.layout.tp_frame_size);
                    return DAQ_ERROR;
                }
                data = instance->rx_ring.cursor->hdr.raw + tp_mac;

                /* Make a valiant attempt at reconstructing the VLAN tag if it has been stripped.  This really sucks. :( */
                if ((instance->tp_version == TPACKET_V2) &&
#if defined(TP_STATUS_VLAN_VALID)
                    (hdr.h2->tp_vlan_tci || (hdr.h2->tp_status & TP_STATUS_VLAN_VALID)) &&
#else
                    hdr.h2->tp_vlan_tci &&
#endif
                    tp_snaplen >= (unsigned int) vlan_offset)
                {
                    struct vlan_tag *tag;

                    data -= VLAN_TAG_LEN;
                    memmove((void *) data, data + VLAN_TAG_LEN, vlan_offset);

                    tag = (struct vlan_tag *) (data + vlan_offset);
                    tag->vlan_tpid = htons(ETH_P_8021Q);
                    tag->vlan_tci = htons(hdr.h2->tp_vlan_tci);

                    tp_snaplen += VLAN_TAG_LEN;
                    tp_len += VLAN_TAG_LEN;
                }

                verdict = DAQ_VERDICT_PASS;
                if (afpc->fcode.bf_insns && sfbpf_filter(afpc->fcode.bf_insns, data, tp_len, tp_snaplen) == 0)
                {
                    ignored_one = 1;
                    afpc->stats.packets_filtered++;
                    goto send_packet;
                }
                got_one = 1;

                daqhdr.ts.tv_sec = tp_sec;
                daqhdr.ts.tv_usec = tp_usec;
                daqhdr.caplen = tp_snaplen;
                daqhdr.pktlen = tp_len;
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
                    afpc->stats.verdicts[verdict]++;
                    verdict = verdict_translation_table[verdict];
                }
                afpc->stats.packets_received++;
                c++;
send_packet:
                if (verdict == DAQ_VERDICT_PASS && instance->peer)
                {
                    AFPacketEntry *entry = instance->peer->tx_ring.cursor;
                    int rc;

                    if (entry->hdr.h2->tp_status == TP_STATUS_AVAILABLE)
                    {
                        memcpy(entry->hdr.raw + TPACKET_ALIGN(instance->peer->tp_hdrlen), data, tp_snaplen);
                        entry->hdr.h2->tp_len = tp_snaplen;
                        entry->hdr.h2->tp_status = TP_STATUS_SEND_REQUEST;
                        rc = send(instance->peer->fd, NULL, 0, 0);
                        instance->peer->tx_ring.cursor = entry->next;
                    }
                    /* Else, don't forward the packet... */
                }
                /* Release the TPACKET buffer back to the kernel. */
                switch (instance->tp_version)
                {
                    case TPACKET_V2:
                        hdr.h2->tp_status = TP_STATUS_KERNEL;
                        break;
                }
                instance->rx_ring.cursor = instance->rx_ring.cursor->next;
            }
        }
        if (!got_one && !ignored_one)
        {
            for (i = 0, instance = afpc->instances; instance; i++, instance = instance->next)
            {
                pfd[i].fd = instance->fd;
                pfd[i].revents = 0;
                pfd[i].events = POLLIN;
            }
            ret = poll(pfd, afpc->intf_count, afpc->timeout);
            /* If we were interrupted by a signal, start the loop over.  The user should call daq_breakloop to actually exit. */
            if (ret < 0 && errno != EINTR)
            {
                DPE(afpc->errbuf, "%s: Poll failed: %s (%d)", __FUNCTION__, strerror(errno), errno);
                return DAQ_ERROR;
            }
            /* If the poll times out, return control to the caller. */
            if (ret == 0)
                break;
            /* If some number of of sockets have events returned, check them all for badness. */
            if (ret > 0)
            {
                for (i = 0; i < afpc->intf_count; i++)
                {
                    if (pfd[i].revents & (POLLHUP | POLLRDHUP | POLLERR | POLLNVAL))
                    {
                        if (pfd[i].revents & (POLLHUP | POLLRDHUP))
                            DPE(afpc->errbuf, "%s: Hang-up on a packet socket", __FUNCTION__);
                        else if (pfd[i].revents & POLLERR)
                            DPE(afpc->errbuf, "%s: Encountered error condition on a packet socket", __FUNCTION__);
                        else if (pfd[i].revents & POLLNVAL)
                            DPE(afpc->errbuf, "%s: Invalid polling request on a packet socket", __FUNCTION__);
                        return DAQ_ERROR;
                    }
                }
            }
        }
    }
    return 0;
}

static int afpacket_daq_inject(void *handle, const DAQ_PktHdr_t *hdr, const uint8_t *packet_data, uint32_t len, int reverse)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;
    AFPacketInstance *instance;
    AFPacketEntry *entry;

    /* Find the instance that the packet was received on. */
    for (instance = afpc->instances; instance; instance = instance->next)
    {
        if (instance->index == hdr->ingress_index)
            break;
    }

    if (!instance || (!reverse && !(instance = instance->peer)))
        return DAQ_ERROR;

    entry = instance->tx_ring.cursor;
    if (entry->hdr.h2->tp_status == TP_STATUS_AVAILABLE)
    {
        memcpy(entry->hdr.raw + TPACKET_ALIGN(instance->tp_hdrlen), packet_data, len);
        entry->hdr.h2->tp_len = len;
        entry->hdr.h2->tp_status = TP_STATUS_SEND_REQUEST;
        instance->tx_ring.cursor = entry->next;
        if (send(instance->fd, NULL, 0, 0) < 0)
        {
            DPE(afpc->errbuf, "%s: Error sending packet: %s (%d)", __FUNCTION__, strerror(errno), errno);
            return DAQ_ERROR;
        }
        afpc->stats.packets_injected++;
    }

    return DAQ_SUCCESS;
}

static int afpacket_daq_breakloop(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    afpc->break_loop = 1;

    return DAQ_SUCCESS;
}

static int afpacket_daq_stop(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    af_packet_close(afpc);

    return DAQ_SUCCESS;
}

static void afpacket_daq_shutdown(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    af_packet_close(afpc);
    if (afpc->device)
        free(afpc->device);
    if (afpc->filter)
        free(afpc->filter);
    free(afpc);
}

static DAQ_State afpacket_daq_check_status(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    return afpc->state;
}

static int afpacket_daq_get_stats(void *handle, DAQ_Stats_t *stats)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    update_hw_stats(afpc);
    memcpy(stats, &afpc->stats, sizeof(DAQ_Stats_t));

    return DAQ_SUCCESS;
}

static void afpacket_daq_reset_stats(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    reset_stats(afpc);
}

static int afpacket_daq_get_snaplen(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    return afpc->snaplen;
}

static uint32_t afpacket_daq_get_capabilities(void *handle)
{
    return DAQ_CAPA_BLOCK | DAQ_CAPA_REPLACE | DAQ_CAPA_INJECT | DAQ_CAPA_UNPRIV_START | DAQ_CAPA_BREAKLOOP | DAQ_CAPA_BPF | DAQ_CAPA_DEVICE_INDEX;
}

static int afpacket_daq_get_datalink_type(void *handle)
{
    return DLT_EN10MB;
}

static const char *afpacket_daq_get_errbuf(void *handle)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    return afpc->errbuf;
}

static void afpacket_daq_set_errbuf(void *handle, const char *string)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;

    if (!string)
        return;

    DPE(afpc->errbuf, "%s", string);
}

static int afpacket_daq_get_device_index(void *handle, const char *device)
{
    AFPacket_Context_t *afpc = (AFPacket_Context_t *) handle;
    AFPacketInstance *instance;

    for (instance = afpc->instances; instance; instance = instance->next)
    {
        if (!strcmp(device, instance->name))
            return instance->index;
    }

    return DAQ_ERROR_NODEV;
}

#ifdef BUILDING_SO
DAQ_SO_PUBLIC const DAQ_Module_t DAQ_MODULE_DATA =
#else
const DAQ_Module_t afpacket_daq_module_data =
#endif
{
    .api_version = DAQ_API_VERSION,
    .module_version = DAQ_AFPACKET_VERSION,
    .name = "afpacket",
    .type = DAQ_TYPE_INTF_CAPABLE | DAQ_TYPE_INLINE_CAPABLE | DAQ_TYPE_MULTI_INSTANCE,
    .initialize = afpacket_daq_initialize,
    .set_filter = afpacket_daq_set_filter,
    .start = afpacket_daq_start,
    .acquire = afpacket_daq_acquire,
    .inject = afpacket_daq_inject,
    .breakloop = afpacket_daq_breakloop,
    .stop = afpacket_daq_stop,
    .shutdown = afpacket_daq_shutdown,
    .check_status = afpacket_daq_check_status,
    .get_stats = afpacket_daq_get_stats,
    .reset_stats = afpacket_daq_reset_stats,
    .get_snaplen = afpacket_daq_get_snaplen,
    .get_capabilities = afpacket_daq_get_capabilities,
    .get_datalink_type = afpacket_daq_get_datalink_type,
    .get_errbuf = afpacket_daq_get_errbuf,
    .set_errbuf = afpacket_daq_set_errbuf,
    .get_device_index = afpacket_daq_get_device_index,
    .modify_flow = NULL,
    .hup_prep = NULL,
    .hup_apply = NULL,
    .hup_post = NULL,
};
