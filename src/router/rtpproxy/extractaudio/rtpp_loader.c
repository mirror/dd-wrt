/*
 * Copyright (c) 2007-2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rtp_info.h"
#include "rtpp_record_private.h"
#include "rtpp_util.h"
#include "rtpp_loader.h"

static int load_adhoc(struct rtpp_loader *loader, struct channels *,
  struct rtpp_session_stat *, enum origin);
static int load_pcap(struct rtpp_loader *loader, struct channels *,
  struct rtpp_session_stat *, enum origin);

static void
rtpp_loader_destroy(struct rtpp_loader *loader)
{

    close(loader->ifd);
    free(loader);
}

struct rtpp_loader *
rtpp_load(const char *path)
{
    struct rtpp_loader *rval;
    pcap_hdr_t *pcap_hdr;

    rval = malloc(sizeof(*rval));
    if (rval == NULL)
        return NULL;

    memset(rval, '\0', sizeof(*rval));

    rval->ifd = open(path, O_RDONLY);
    if (rval->ifd == -1) {
        free(rval);
        return NULL;
    }

    if (fstat(rval->ifd, &(rval->sb)) == -1 || rval->sb.st_size == 0) {
        close(rval->ifd);
        free(rval);
        return NULL;
    }

    rval->ibuf = mmap(NULL, rval->sb.st_size, PROT_READ, MAP_SHARED, rval->ifd, 0);
    if (rval->ibuf == MAP_FAILED) {
        close(rval->ifd);
        free(rval);
        return NULL;
    }

    rval->destroy = rtpp_loader_destroy;

    if (*(uint32_t *)(rval->ibuf) != PCAP_MAGIC) {
        rval->load = load_adhoc;
    } else {
        rval->load = load_pcap;
        if (rval->sb.st_size < sizeof(*pcap_hdr)) {
            warnx("invalid PCAP format");
            rval->destroy(rval);
            return NULL;
        }
        pcap_hdr = (pcap_hdr_t *)rval->ibuf;
        if (pcap_hdr->network != DLT_EN10MB && pcap_hdr->network != DLT_NULL) {
            warnx("unsupported data-link type in the PCAP: %d", pcap_hdr->network);
            rval->destroy(rval);
            return NULL;
        }
        if (pcap_hdr->version_major != PCAP_VER_MAJR || pcap_hdr->version_minor != PCAP_VER_MINR) {
            warnx("unsupported version of the PCAP: %d.%d", pcap_hdr->version_major, pcap_hdr->version_minor);
            rval->destroy(rval);
            return NULL;
        }

        rval->private.pcap_data.pcap_hdr = pcap_hdr;
        rval->ibuf += sizeof(*pcap_hdr);
        rval->sb.st_size -= sizeof(*pcap_hdr);
    }
    return rval;
}

static int
load_adhoc(struct rtpp_loader *loader, struct channels *channels,
  struct rtpp_session_stat *stat, enum origin origin)
{
    int pcount;
    unsigned char *cp;
    struct pkt_hdr_adhoc *pkt;
    struct packet *pack, *pp;
    struct channel *channel;
    struct session *sess;
    off_t st_size;

    pcount = 0;
    st_size = loader->sb.st_size;
    for (cp = loader->ibuf; cp < loader->ibuf + st_size; cp += pkt->plen) {
        pkt = (struct pkt_hdr_adhoc *)cp;
        cp += sizeof(*pkt);
        if (pkt->plen < sizeof(rtp_hdr_t))
            continue;
        pack = malloc(sizeof(*pack));
        if (rtp_packet_parse(cp, pkt->plen, &(pack->parsed)) != RTP_PARSER_OK) {
            /* XXX error handling */
            free(pack);
            continue;
        }
        pack->pkt = pkt;
        pack->rpkt = RPKT(pack);
        update_rtpp_stats(stat, pack->rpkt, &(pack->parsed), pkt->time);

        sess = session_lookup(channels, pack->rpkt->ssrc);
        if (sess == NULL) {
            channel = malloc(sizeof(*channel));
            memset(channel, 0, sizeof(*channel));
            channel->origin = origin;
            sess = &(channel->session);
            MYQ_INIT(sess);
            MYQ_INSERT_HEAD(sess, pack);
            channel_insert(channels, channel);
            pcount++;
            goto endloop;
        }

        /* Put packet it order */
        MYQ_FOREACH_REVERSE(pp, sess) {
            if (pp->parsed.seq == pack->parsed.seq) {
                /* Duplicate packet */
                free(pack);
                goto endloop;
            }
            if (pp->parsed.ts < pack->parsed.ts ||
              pp->parsed.seq < pack->parsed.seq) {
                MYQ_INSERT_AFTER(sess, pp, pack);
                pcount++;
                goto endloop;
            }
        }
        MYQ_INSERT_HEAD(sess, pack);
        pcount++;
endloop:
        continue;
    }
    if (cp != loader->ibuf + st_size) {
        warnx("invalid format, %d packets loaded", pcount);
        return -1;
    }
    return pcount;
}

static int
load_pcap(struct rtpp_loader *loader, struct channels *channels,
  struct rtpp_session_stat *stat, enum origin origin)
{
    int pcount;
    unsigned char *cp;
    struct packet *pack, *pp;
    struct channel *channel;
    struct session *sess;
    union pkt_hdr_pcap *pcap;
    int rtp_len;
    off_t st_size;
    int pcap_size, network;
    pcaprec_hdr_t *pcaprec_hdr;
    struct udpip *udpip;

    st_size = loader->sb.st_size;
    network = loader->private.pcap_data.pcap_hdr->network;

    pcount = 0;
    for (cp = loader->ibuf; cp < loader->ibuf + st_size; cp += rtp_len) {
        pcap = (void *)cp;

        if (network == DLT_NULL) {
            pcap_size = sizeof(struct pkt_hdr_pcap_null);
            pcaprec_hdr = &(pcap->null.pcaprec_hdr);
            udpip = &(pcap->null.udpip);
        } else {
            pcap_size = sizeof(struct pkt_hdr_pcap_en10t);
            pcaprec_hdr = &(pcap->en10t.pcaprec_hdr);
            udpip = &(pcap->en10t.udpip);
        }
        rtp_len = pcaprec_hdr->incl_len - (pcap_size - sizeof(*pcaprec_hdr));
        if (rtp_len < 0) {
            warnx("broken or truncated PCAP file");
            return -1;
        }
        cp += pcap_size;
        if (rtp_len < sizeof(rtp_hdr_t))
            continue;
        pack = malloc(sizeof(*pack) + sizeof(*pack->pkt));
        if (rtp_packet_parse(cp, rtp_len, &(pack->parsed)) != RTP_PARSER_OK) {
            /* XXX error handling */
            free(pack);
            continue;
        }
        pack->pkt = (struct pkt_hdr_adhoc *)&pack[1];
        pack->rpkt = (rtp_hdr_t *)cp;
        pack->pkt->time = ts2dtime(pcaprec_hdr->ts_sec, pcaprec_hdr->ts_usec);
        pack->pkt->plen = rtp_len;
        if (origin == O_CH) {
            pack->pkt->addr.in4.sin_family = AF_INET;
            pack->pkt->addr.in4.sin_port = ntohs(udpip->udphdr.uh_dport);
            pack->pkt->addr.in4.sin_addr = udpip->iphdr.ip_dst;
        } else {
            pack->pkt->addr.in4.sin_family = AF_INET;
            pack->pkt->addr.in4.sin_port = ntohs(udpip->udphdr.uh_sport);
            pack->pkt->addr.in4.sin_addr = udpip->iphdr.ip_src;
        }
        update_rtpp_stats(stat, pack->rpkt, &(pack->parsed), pack->pkt->time);

        sess = session_lookup(channels, pack->rpkt->ssrc);
        if (sess == NULL) {
            channel = malloc(sizeof(*channel));
            memset(channel, 0, sizeof(*channel));
            channel->origin = origin;
            sess = &(channel->session);
            MYQ_INIT(sess);
            MYQ_INSERT_HEAD(sess, pack);
            channel_insert(channels, channel);
            pcount++;
            goto endloop;
        }

        /* Put packet it order */
        MYQ_FOREACH_REVERSE(pp, sess) {
            if (pp->parsed.seq == pack->parsed.seq) {
                /* Duplicate packet */
                free(pack);
                goto endloop;
            }
            if (pp->parsed.ts < pack->parsed.ts ||
              pp->parsed.seq < pack->parsed.seq) {
                MYQ_INSERT_AFTER(sess, pp, pack);
                pcount++;
                goto endloop;
            }
        }
        MYQ_INSERT_HEAD(sess, pack);
        pcount++;
endloop:
        continue;
    }
    if (cp != loader->ibuf + st_size) {
        warnx("invalid format, %d packets loaded", pcount);
        return -1;
    }
    return pcount;
}
