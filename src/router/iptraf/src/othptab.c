/***

othptab.c - non-TCP protocol display module
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997, 1998

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <asm/types.h>
#include <linux/if_ether.h>
#include <linux/if_tr.h>
#include <linux/if_fddi.h>
#include <winops.h>
#include "arphdr.h"
#include "options.h"
#include "tcptable.h"
#include "othptab.h"
#include "deskman.h"
#include "attrs.h"
#include "log.h"
#include "revname.h"
#include "rvnamed.h"
#include "links.h"

#define MSGSTRING_MAX	240
#define SHORTSTRING_MAX	40

void convmacaddr(char *addr, char *result);     /* external; from hostmon.c */
void writeothplog(int logging, FILE * fd, char *protname,
                  char *description, char *additional, int is_ip,
                  int withmac, struct othptabent *entry);

void init_othp_table(struct othptable *table, int mac)
{
    unsigned int winht;
    unsigned int wintop;
    unsigned int obmaxx;

    winht = LINES - (LINES * 0.6) - 2;
    wintop = (LINES * 0.6) + 1;

    table->count = 0;
    table->lastpos = 0;
    table->strindex = 0;
    table->htstat = NOHTIND;
    table->head = table->tail = NULL;
    table->firstvisible = table->lastvisible = NULL;
    table->borderwin = newwin(winht, COLS, wintop, 0);
    table->borderpanel = new_panel(table->borderwin);
    wattrset(table->borderwin, BOXATTR);
    tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);

    table->head = table->tail = NULL;
    table->othpwin = newwin(winht - 2, COLS - 2, wintop + 1, 1);
    table->othppanel = new_panel(table->othpwin);
    wattrset(table->othpwin, STDATTR);
    tx_colorwin(table->othpwin);
    update_panels();
    doupdate();

    tx_stdwinset(table->othpwin);
    getmaxyx(table->borderwin, table->obmaxy, obmaxx);
    table->oimaxy = table->obmaxy - 2;

    table->mac = mac;
}

void process_dest_unreach(struct tcptable *table, char *packet,
                          char *ifname, int *nomem)
{
    struct iphdr *ip;
    struct tcphdr *tcp;
    struct tcptableent *tcpentry;

    ip = (struct iphdr *) (packet + 8);

    if (ip->protocol != IPPROTO_TCP)
        return;

    tcp = (struct tcphdr *) (packet + 8 + (ip->ihl * 4));

    /* 
     * We really won't be making use of nomem here.  Timeout checking
     * won't be performed either, so we just pass NULL as the pointer
     * to the configuration structure.  in_table() will recognize this
     * and set its internal timeout variable to 0.
     */

    tcpentry = in_table(table, ip->saddr, ip->daddr,
                        ntohs(tcp->source), ntohs(tcp->dest), ifname,
                        0, NULL, nomem, NULL);

    if (tcpentry != NULL) {
        tcpentry->stat = tcpentry->oth_connection->stat = FLAG_RST;
        addtoclosedlist(table, tcpentry, nomem);
    }
}

struct othptabent *add_othp_entry(struct othptable *table,
                                  struct tcptable *tcptab,
                                  unsigned long saddr, unsigned long daddr,
                                  int is_ip, int protocol,
                                  unsigned short linkproto, char *packet,
                                  char *packet2, unsigned int br,
                                  char *ifname, int *rev_lookup, int rvnfd,
                                  unsigned int tm, int logging,
                                  FILE * logfile, int servnames,
                                  int fragment, int *nomem)
{
    struct othptabent *new_entry;
    struct othptabent *temp;
    struct in_addr isaddr, idaddr;

    new_entry = malloc(sizeof(struct othptabent));

    if (new_entry == NULL) {
        printnomem();
        *nomem = 1;
        return NULL;
    }
    bzero(new_entry, sizeof(struct othptabent));

    new_entry->is_ip = is_ip;
    new_entry->fragment = fragment;

    if ((table->mac) || (!is_ip)) {
        if ((linkproto == LINK_ETHERNET) || (linkproto == LINK_PLIP)) {
            convmacaddr(((struct ethhdr *) packet)->h_source,
                        new_entry->smacaddr);
            convmacaddr(((struct ethhdr *) packet)->h_dest,
                        new_entry->dmacaddr);
        } else if (linkproto == LINK_FDDI) {
            convmacaddr(((struct fddihdr *) packet)->saddr,
                        new_entry->smacaddr);
            convmacaddr(((struct fddihdr *) packet)->daddr,
                        new_entry->dmacaddr);
        } else if (linkproto == LINK_TR) {
            convmacaddr(((struct trh_hdr *) packet)->saddr,
                        new_entry->smacaddr);
            convmacaddr(((struct trh_hdr *) packet)->daddr,
                        new_entry->dmacaddr);
        }
    }

    if (is_ip) {
        new_entry->saddr = isaddr.s_addr = saddr;
        new_entry->daddr = idaddr.s_addr = daddr;

        revname(rev_lookup, &isaddr, new_entry->s_fqdn, rvnfd);
        revname(rev_lookup, &idaddr, new_entry->d_fqdn, rvnfd);

        if (!fragment) {
            if (protocol == IPPROTO_ICMP) {
                new_entry->un.icmp.type =
                    ((struct icmphdr *) packet2)->type;
                new_entry->un.icmp.code =
                    ((struct icmphdr *) packet2)->code;
            } else if (protocol == IPPROTO_UDP) {
                servlook(servnames, ((struct udphdr *) packet2)->source,
                         IPPROTO_UDP, new_entry->un.udp.s_sname, 10);
                servlook(servnames, ((struct udphdr *) packet2)->dest,
                         IPPROTO_UDP, new_entry->un.udp.d_sname, 10);
            } else if (protocol == IPPROTO_OSPFIGP) {
                new_entry->un.ospf.type =
                    ((struct ospfhdr *) packet2)->ospf_type;
                new_entry->un.ospf.area =
                    ntohl(((struct ospfhdr *) packet2)->ospf_areaid.
                          s_addr);
                strcpy(new_entry->un.ospf.routerid,
                       inet_ntoa(((struct ospfhdr *)
                                  packet2)->ospf_routerid));
            }
        }
    } else {
        new_entry->linkproto = linkproto;

        if (protocol == ETH_P_ARP) {
            new_entry->un.arp.opcode = ((struct arp_hdr *) packet2)->ar_op;
            memcpy(&(new_entry->un.arp.src_ip_address),
                   &(((struct arp_hdr *) packet2)->ar_sip), 4);
            memcpy(&(new_entry->un.arp.dest_ip_address),
                   &(((struct arp_hdr *) packet2)->ar_tip), 4);
        } else if (protocol == ETH_P_RARP) {
            new_entry->un.rarp.opcode = ((struct arphdr *) packet2)->ar_op;
            memcpy(&(new_entry->un.rarp.src_mac_address),
                   &(((struct arp_hdr *) packet2)->ar_sha), 6);
            memcpy(&(new_entry->un.rarp.dest_mac_address),
                   &(((struct arp_hdr *) packet2)->ar_tha), 6);
        }
    }

    new_entry->protocol = protocol;
    strcpy(new_entry->iface, ifname);

    new_entry->pkt_length = br;

    if (table->head == NULL) {
        new_entry->prev_entry = NULL;
        table->head = new_entry;
        table->firstvisible = new_entry;
    }
    /*
     * Max number of entries in the lower window is 512.  Upon reaching
     * this figure, oldest entries are thrown out.
     */

    if (table->count == 512) {
        if (table->firstvisible == table->head) {
            wscrl(table->othpwin, 1);
            printothpentry(table, table->lastvisible->next_entry,
                           table->oimaxy - 1, logging, logfile);
            table->firstvisible = table->firstvisible->next_entry;
            table->lastvisible = table->lastvisible->next_entry;
        }
        temp = table->head;
        table->head = table->head->next_entry;
        table->head->prev_entry = NULL;
        free(temp);
    } else
        table->count++;

    if (table->tail != NULL) {
        new_entry->prev_entry = table->tail;
        table->tail->next_entry = new_entry;
    }
    table->tail = new_entry;
    new_entry->next_entry = NULL;

    table->lastpos++;
    new_entry->index = table->lastpos;

    if (table->count <= table->oimaxy) {
        table->lastvisible = new_entry;
        printothpentry(table, new_entry, table->count - 1, logging,
                       logfile);
    } else if (table->lastvisible == table->tail->prev_entry) {
        wscrl(table->othpwin, 1);
        table->firstvisible = table->firstvisible->next_entry;
        table->lastvisible = table->tail;
        printothpentry(table, new_entry, table->oimaxy - 1, logging,
                       logfile);
    }
    return new_entry;
}

/*
 * Function to retrieve non-IP packet tags.  No further details are
 * provided beyond the type.
 */

char *packetlookup(unsigned int protocol)
{
    unsigned int i = 0;
    static struct packetstruct packettypes[] = {
        {"DEC MOP dump/load", 0x6001},
        {"DEC MOP remote console", 0x6002},
        {"DEC DECnet Phase IV", 0x6003},
        {"DEC LAT", 0x6004},
        {"DEC DECnet Diagnostics", 0x6005},
        {"DEC DECnet Customer Use", 0x6006},
        {"DEC DECnet SCA", 0x6007},
        {"IPX", 0x8137},
        {NULL, 0x0}
    };


    while ((packettypes[i].packet_name != NULL)
           && (packettypes[i].protocol != protocol))
        i++;

    return packettypes[i].packet_name;

}

void printothpentry(struct othptable *table, struct othptabent *entry,
                    unsigned int target_row, int logging, FILE * logfile)
{
    char protname[SHORTSTRING_MAX];
    char description[SHORTSTRING_MAX];
    char additional[MSGSTRING_MAX];
    char msgstring[MSGSTRING_MAX];
    char scratchpad[MSGSTRING_MAX];
    char sp_buf[SHORTSTRING_MAX];
    char *startstr;

    char *packet_type;

    struct in_addr saddr;
    char rarp_mac_addr[15];

    unsigned int unknown = 0;

    struct protoent *protptr;
    sprintf(sp_buf, "%%%dc", COLS - 2);

    wmove(table->borderwin, table->obmaxy - 1, 1);
    if ((table->lastvisible == table->tail) && (table->htstat != TIND) &&
        (table->count >= table->oimaxy)) {
        wprintw(table->borderwin, " Bottom ");
        table->htstat = TIND;
    } else if ((table->firstvisible == table->head)
               && (table->htstat != HIND)) {
        wprintw(table->borderwin, " Top ");
        table->htstat = HIND;
    }
    if (!(entry->is_ip)) {
        wmove(table->othpwin, target_row, 0);
        scrollok(table->othpwin, 0);
        wattrset(table->othpwin, UNKNATTR);
        wprintw(table->othpwin, sp_buf, ' ');
        scrollok(table->othpwin, 1);
        wmove(table->othpwin, target_row, 1);

        switch (entry->protocol) {
        case ETH_P_ARP:
            sprintf(msgstring, "ARP ");
            switch (ntohs(entry->un.arp.opcode)) {
            case ARPOP_REQUEST:
                strcat(msgstring, "request for ");
                memcpy(&(saddr.s_addr), entry->un.arp.dest_ip_address, 4);
                break;
            case ARPOP_REPLY:
                strcat(msgstring, "reply from ");
                memcpy(&(saddr.s_addr), entry->un.arp.src_ip_address, 4);
                break;
            }

            sprintf(scratchpad, inet_ntoa(saddr));
            strcat(msgstring, scratchpad);
            wattrset(table->othpwin, ARPATTR);
            break;
        case ETH_P_RARP:
            sprintf(msgstring, "RARP ");
            memset(rarp_mac_addr, 0, 15);
            switch (ntohs(entry->un.rarp.opcode)) {
            case ARPOP_RREQUEST:
                strcat(msgstring, "request for ");
                convmacaddr(entry->un.rarp.dest_mac_address,
                            rarp_mac_addr);
                break;
            case ARPOP_RREPLY:
                strcat(msgstring, "reply from ");
                convmacaddr(entry->un.rarp.src_mac_address, rarp_mac_addr);
                break;
            }

            sprintf(scratchpad, rarp_mac_addr);
            strcat(msgstring, scratchpad);
            wattrset(table->othpwin, ARPATTR);
            break;
        default:
            packet_type = packetlookup(entry->protocol);
            if (packet_type == NULL)
                sprintf(msgstring, "Non-IP (0x%x)", entry->protocol);
            else
                sprintf(msgstring, "Non-IP (%s)", packet_type);

            wattrset(table->othpwin, UNKNATTR);
        }

        strcpy(protname, msgstring);
        sprintf(scratchpad, " (%u bytes)", entry->pkt_length);
        strcat(msgstring, scratchpad);

        if ((entry->linkproto == LINK_ETHERNET) ||
            (entry->linkproto == LINK_PLIP) ||
            (entry->linkproto == LINK_FDDI)) {
            sprintf(scratchpad, " from %s to %s on %s",
                    entry->smacaddr, entry->dmacaddr, entry->iface);

            strcat(msgstring, scratchpad);
        }
        startstr = msgstring + table->strindex;
        waddnstr(table->othpwin, startstr, COLS - 4);
        writeothplog(logging, logfile, protname, "", "", 0, 0, entry);
        return;
    }
    strcpy(additional, "");
    strcpy(description, "");

    switch (entry->protocol) {
    case IPPROTO_UDP:
        wattrset(table->othpwin, UDPATTR);
        strcpy(protname, "UDP");
        break;
    case IPPROTO_ICMP:
        wattrset(table->othpwin, STDATTR);
        strcpy(protname, "ICMP");
        break;
    case IPPROTO_OSPFIGP:
        wattrset(table->othpwin, OSPFATTR);
        strcpy(protname, "OSPF");
        break;
    case IPPROTO_IGP:
        wattrset(table->othpwin, IGPATTR);
        strcpy(protname, "IGP");
        break;
    case IPPROTO_IGMP:
        wattrset(table->othpwin, IGMPATTR);
        strcpy(protname, "IGMP");
        break;
    case IPPROTO_IGRP:
        wattrset(table->othpwin, IGRPATTR);
        strcpy(protname, "IGRP");
        break;
    case IPPROTO_GRE:
        wattrset(table->othpwin, GREATTR);
        strcpy(protname, "GRE");
        break;
    default:
        wattrset(table->othpwin, UNKNIPATTR);
        protptr = getprotobynumber(entry->protocol);
        if (protptr != NULL) {
            sprintf(protname, protptr->p_aliases[0]);
        } else {
            sprintf(protname, "IP protocol");
            unknown = 1;
        }
    }

    if (!(entry->fragment)) {
        if (entry->protocol == IPPROTO_ICMP) {
            switch (entry->un.icmp.type) {
            case ICMP_ECHOREPLY:
                strcpy(description, "echo rply");
                break;
            case ICMP_ECHO:
                strcpy(description, "echo req");
                break;
            case ICMP_DEST_UNREACH:
                strcpy(description, "dest unrch");
                switch (entry->un.icmp.code) {
                case ICMP_NET_UNREACH:
                    strcpy(additional, "ntwk");
                    break;
                case ICMP_HOST_UNREACH:
                    strcpy(additional, "host");
                    break;
                case ICMP_PROT_UNREACH:
                    strcpy(additional, "proto");
                    break;
                case ICMP_PORT_UNREACH:
                    strcpy(additional, "port");
                    break;
                case ICMP_FRAG_NEEDED:
                    strcpy(additional, "DF set");
                    break;
                case ICMP_SR_FAILED:
                    strcpy(additional, "src rte fail");
                    break;
                case ICMP_NET_UNKNOWN:
                    strcpy(additional, "net unkn");
                    break;
                case ICMP_HOST_UNKNOWN:
                    strcpy(additional, "host unkn");
                    break;
                case ICMP_HOST_ISOLATED:
                    strcpy(additional, "src isltd");
                    break;
                case ICMP_NET_ANO:
                    strcpy(additional, "net comm denied");
                    break;
                case ICMP_HOST_ANO:
                    strcpy(additional, "host comm denied");
                    break;
                case ICMP_NET_UNR_TOS:
                    strcpy(additional, "net unrch for TOS");
                    break;
                case ICMP_HOST_UNR_TOS:
                    strcpy(additional, "host unrch for TOS");
                    break;
                case ICMP_PKT_FILTERED:
                    strcpy(additional, "pkt fltrd");
                    break;
                case ICMP_PREC_VIOLATION:
                    strcpy(additional, "prec violtn");
                    break;
                case ICMP_PREC_CUTOFF:
                    strcpy(additional, "prec cutoff");
                    break;
                }

                break;
            case ICMP_SOURCE_QUENCH:
                strcpy(description, "src qnch");
                break;
            case ICMP_REDIRECT:
                strcpy(description, "redirct");
                break;
            case ICMP_TIME_EXCEEDED:
                strcpy(description, "time excd");
                break;
            case ICMP_PARAMETERPROB:
                strcpy(description, "param prob");
                break;
            case ICMP_TIMESTAMP:
                strcpy(description, "timestmp req");
                break;
            case ICMP_INFO_REQUEST:
                strcpy(description, "info req");
                break;
            case ICMP_INFO_REPLY:
                strcpy(description, "info rep");
                break;
            case ICMP_ADDRESS:
                strcpy(description, "addr mask req");
                break;
            case ICMP_ADDRESSREPLY:
                strcpy(description, "addr mask rep");
                break;
            default:
                strcpy(description, "bad/unkn");
                break;
            }

        } else if (entry->protocol == IPPROTO_OSPFIGP) {
            switch (entry->un.ospf.type) {
            case OSPF_TYPE_HELLO:
                strcpy(description, "hlo");
                break;
            case OSPF_TYPE_DB:
                strcpy(description, "DB desc");
                break;
            case OSPF_TYPE_LSR:
                strcpy(description, "LSR");
                break;
            case OSPF_TYPE_LSU:
                strcpy(description, "LSU");
                break;
            case OSPF_TYPE_LSA:
                strcpy(description, "LSA");
                break;
            }
            sprintf(additional, "a=%lu r=%s", entry->un.ospf.area,
                    entry->un.ospf.routerid);
        }
    } else
        strcpy(description, "fragment");

    strcpy(msgstring, protname);
    strcat(msgstring, " ");

    if (strcmp(description, "") != 0) {
        strcat(msgstring, description);
        strcat(msgstring, " ");
    }
    if (strcmp(additional, "") != 0) {
        sprintf(scratchpad, "(%s) ", additional);
        strcat(msgstring, scratchpad);
    }
    if (unknown) {
        sprintf(scratchpad, "%u ", entry->protocol);
        strcat(msgstring, scratchpad);
    }
    sprintf(scratchpad, "(%u bytes) ", entry->pkt_length);
    strcat(msgstring, scratchpad);

    if ((entry->protocol == IPPROTO_UDP) && (!(entry->fragment))) {
        sprintf(scratchpad, "from %.25s:%s to %.25s:%s",
                entry->s_fqdn, entry->un.udp.s_sname,
                entry->d_fqdn, entry->un.udp.d_sname);
    } else {
        sprintf(scratchpad, "from %.25s to %.25s", entry->s_fqdn,
                entry->d_fqdn);
    }

    strcat(msgstring, scratchpad);

    if (((entry->smacaddr)[0] != '\0') && (table->mac)) {
        snprintf(scratchpad, MSGSTRING_MAX, " (src HWaddr %s)",
                 entry->smacaddr);
        strcat(msgstring, scratchpad);
    }
    strcat(msgstring, " on ");
    strcat(msgstring, entry->iface);

    wmove(table->othpwin, target_row, 0);
    scrollok(table->othpwin, 0);
    wprintw(table->othpwin, sp_buf, ' ');
    scrollok(table->othpwin, 1);
    wmove(table->othpwin, target_row, 1);
    startstr = msgstring + table->strindex;
    waddnstr(table->othpwin, startstr, COLS - 4);

    if (logging)
        writeothplog(logging, logfile, protname, description, additional,
                     1, table->mac, entry);
}

void refresh_othwindow(struct othptable *table)
{
    int target_row = 0;
    struct othptabent *entry;

    wattrset(table->othpwin, STDATTR);
    tx_colorwin(table->othpwin);

    entry = table->firstvisible;

    while ((entry != NULL) && (entry != table->lastvisible->next_entry)) {
        printothpentry(table, entry, target_row, 0, (FILE *) NULL);
        target_row++;
        entry = entry->next_entry;
    }

    update_panels();
    doupdate();
}

void destroyothptable(struct othptable *table)
{
    struct othptabent *ctemp;
    struct othptabent *ctemp_next;

    if (table->head != NULL) {
        ctemp = table->head;
        ctemp_next = table->head->next_entry;

        while (ctemp != NULL) {
            free(ctemp);
            ctemp = ctemp_next;

            if (ctemp_next != NULL)
                ctemp_next = ctemp_next->next_entry;
        }
    }
}
