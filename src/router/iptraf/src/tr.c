/*
 * tr.c - Token Ring frame parsing code
 *
 * Based on the sources from the Linux kernel.
 *
 * Copyright (c) Gerard Paul Java 2002
 */

#include <asm/types.h>
#include <linux/if_tr.h>
#include <netinet/in.h>

unsigned int get_tr_ip_offset(unsigned char *pkt)
{
    struct trh_hdr *trh;
    unsigned int riflen = 0;

    trh = (struct trh_hdr *) pkt;

    /*
     * Check if this packet has TR routing information and get
     * its length.
     */
    if (trh->saddr[0] & TR_RII)
        riflen = (ntohs(trh->rcf) & TR_RCF_LEN_MASK) >> 8;

    return sizeof(struct trh_hdr) - TR_MAXRIFLEN + riflen +
        sizeof(struct trllc);
}
