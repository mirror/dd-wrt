
/***

tcptable.c - table manipulation routines for the IP monitor
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

#include <winops.h>
#include "options.h"
#include "tcptable.h"
#include "deskman.h"
#include "attrs.h"
#include "log.h"
#include "revname.h"
#include "rvnamed.h"

#define MSGSTRING_MAX	320

unsigned int bmaxy = 0;
unsigned int imaxy = 0;

void convmacaddr(char *addr, char *result);
void writetcplog(int logging, FILE * fd, struct tcptableent *entry,
                 unsigned int pktlen, int mac, char *message);

void setlabels(WINDOW * win, int mode)
{
    wmove(win, 0, 42 * COLS / 80);
    whline(win, ACS_HLINE, 23 * COLS / 80);

    if (mode == 0) {
        wmove(win, 0, 47 * COLS / 80);
        wprintw(win, " Packets ");
        wmove(win, 0, 59 * COLS / 80);
        wprintw(win, " Bytes ");
    } else if (mode == 1) {
        mvwprintw(win, 0, 47 * COLS / 80, " Source MAC Addr ");
    } else if (mode == 2) {
        wmove(win, 0, 45 * COLS / 80);
        wprintw(win, " Pkt Size ");
        wmove(win, 0, 56 * COLS / 80);
        wprintw(win, " Win Size ");
    }
}

/*
 * The hash function for the TCP hash table
 */

unsigned int tcp_hash(unsigned long saddr, unsigned int sport,
                      unsigned long daddr, unsigned int dport,
                      char *ifname)
{
    int i;
    int ifsum = 0;

    for (i = 0; i <= strlen(ifname) - 1; i++)
        ifsum += ifname[i];

    return ((ifsum + (4 * saddr) + (3 * sport) +
             (2 * daddr) + dport) % ENTRIES_IN_HASH_TABLE);
}

void print_tcp_num_entries(struct tcptable *table)
{
    mvwprintw(table->borderwin, table->bmaxy - 1, 1, " TCP: %6u entries ",
              table->count);
}


void init_tcp_table(struct tcptable *table)
{
    int i;

    table->bmaxy = LINES * 0.6; /* 60% of total screen */
    table->imaxy = table->bmaxy - 2;

    table->borderwin = newwin(table->bmaxy, COLS, 1, 0);
    table->borderpanel = new_panel(table->borderwin);

    wattrset(table->borderwin, BOXATTR);
    tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
    wmove(table->borderwin, 0, 1);
    wprintw(table->borderwin, " TCP Connections (Source Host:Port) ");

    setlabels(table->borderwin, 0);     /* initially use mode 0 */

    wmove(table->borderwin, 0, 65 * COLS / 80);
    wprintw(table->borderwin, " Flags ");
    wmove(table->borderwin, 0, 72 * COLS / 80);
    wprintw(table->borderwin, " Iface ");
    update_panels();
    doupdate();

    table->head = table->tail = NULL;
    table->firstvisible = table->lastvisible = NULL;
    table->tcpscreen = newwin(table->imaxy, COLS - 2, 2, 1);
    table->tcppanel = new_panel(table->tcpscreen);
    table->closedentries = table->closedtail = NULL;
    wattrset(table->tcpscreen, BOXATTR);
    tx_colorwin(table->tcpscreen);
    table->lastpos = 0;
    table->count = 0;

    wtimeout(table->tcpscreen, -1);
    tx_stdwinset(table->tcpscreen);
    print_tcp_num_entries(table);

    /* 
     * Initialize hash table to nulls
     */

    for (i = 0; i <= ENTRIES_IN_HASH_TABLE - 1; i++) {
        table->hash_table[i] = NULL;
        table->hash_tails[i] = NULL;
    }
    table->barptr = NULL;
    table->baridx = 0;
}

/*
 * Add a TCP entry to the hash table.
 */

int add_tcp_hash_entry(struct tcptable *table, struct tcptableent *entry)
{
    unsigned int hp;            /* hash position in table */
    struct tcp_hashentry *ptmp;

    hp = tcp_hash(entry->saddr.s_addr, entry->sport,
                  entry->daddr.s_addr, entry->dport, entry->ifname);

    ptmp = malloc(sizeof(struct tcp_hashentry));
    bzero(ptmp, sizeof(struct tcp_hashentry));

    if (ptmp == NULL)
        return 1;

    /*
     * Add backpointer from screen node to hash node for deletion later
     * (Actually point to its predecessor coz of the header cell).
     */

    entry->hash_node = ptmp;

    /* 
     * Update hash node and add it to list.
     */

    ptmp->tcpnode = entry;
    ptmp->hp = hp;

    if (table->hash_table[hp] == NULL) {
        ptmp->prev_entry = NULL;
        table->hash_table[hp] = ptmp;
        ptmp->index = 1;
    }

    if (table->hash_tails[hp] != NULL) {
        table->hash_tails[hp]->next_entry = ptmp;
        ptmp->prev_entry = table->hash_tails[hp];
        ptmp->index = ptmp->prev_entry->index + 1;
    }
    table->hash_tails[hp] = ptmp;
    ptmp->next_entry = NULL;

    return 0;
}

/*
 * Delete a hash table node
 */

void del_tcp_hash_node(struct tcptable *table, struct tcptableent *entry)
{
    struct tcp_hashentry *ptmp;

    ptmp = entry->hash_node;    /* ptmp now points to the target */

    /*
     * If the targeted node is the last entry, adjust the corresponding tail
     * pointer to the preceeding node;
     */

    if (ptmp->next_entry == NULL)
        table->hash_tails[ptmp->hp] = ptmp->prev_entry;

    if (ptmp->prev_entry != NULL)
        ptmp->prev_entry->next_entry = ptmp->next_entry;
    else
        table->hash_table[ptmp->hp] = ptmp->next_entry;

    if (ptmp->next_entry != NULL)
        ptmp->next_entry->prev_entry = ptmp->prev_entry;

    free(ptmp);
}

/*
 * Add a new entry to the TCP screen table
 */

struct tcptableent *addentry(struct tcptable *table,
                             unsigned long int saddr,
                             unsigned long int daddr,
                             unsigned int sport, unsigned int dport,
                             int protocol,
                             char *ifname, int *rev_lookup,
                             int rvnfd, int servnames, int *nomem)
{
    struct tcptableent *new_entry;
    struct closedlist *ctemp;

    /* 
     * Allocate and attach a new node if no closed entries found
     */

    if (table->closedentries == NULL) {
        new_entry = malloc(sizeof(struct tcptableent));

        if (new_entry != NULL)
            new_entry->oth_connection = malloc(sizeof(struct tcptableent));

        if ((new_entry->oth_connection == NULL) || (new_entry == NULL)) {
            printnomem();
            *nomem = 1;
            return NULL;
        }
        new_entry->oth_connection->oth_connection = new_entry;

        if (table->head == NULL) {
            new_entry->prev_entry = NULL;
            table->head = new_entry;

            table->firstvisible = new_entry;
        }
        if (table->tail != NULL) {
            table->tail->next_entry = new_entry;
            new_entry->prev_entry = table->tail;
        }
        table->lastpos++;
        new_entry->index = table->lastpos;
        table->lastpos++;
        new_entry->oth_connection->index = table->lastpos;

        table->tail = new_entry->oth_connection;
        new_entry->next_entry = new_entry->oth_connection;
        new_entry->next_entry->prev_entry = new_entry;
        new_entry->next_entry->next_entry = NULL;


        if (new_entry->oth_connection->index <=
            table->firstvisible->index + (table->imaxy - 1))
            table->lastvisible = new_entry->oth_connection;
        else if (new_entry->index <=
                 table->firstvisible->index + (table->imaxy - 1))
            table->lastvisible = new_entry;

        new_entry->reused = new_entry->oth_connection->reused = 0;
        table->count++;

        print_tcp_num_entries(table);
    } else {
        /*
         * If we reach this point, we're allocating off the list of closed
         * entries.  In this case, we take the top entry, let the new_entry
         * variable point to whatever the top is pointing to.  The new_entry's
         * oth_connection also points to the reused entry's oth_connection
         */

        new_entry = table->closedentries->closedentry;
        new_entry->oth_connection = table->closedentries->pair;

        ctemp = table->closedentries;
        table->closedentries = table->closedentries->next_entry;
        free(ctemp);

        /*
         * Mark the closed list's tail as NULL if we use the last entry
         * in the list to prevent a dangling reference.
         */

        if (table->closedentries == NULL)
            table->closedtail = NULL;

        new_entry->reused = new_entry->oth_connection->reused = 1;

        /*
         * Delete the old hash entries for this reallocated node;
         */

        del_tcp_hash_node(table, new_entry);
        del_tcp_hash_node(table, new_entry->oth_connection);
    }

    /* 
     * Fill in address fields with raw IP addresses
     */

    new_entry->saddr.s_addr = new_entry->oth_connection->daddr.s_addr =
        saddr;
    new_entry->daddr.s_addr = new_entry->oth_connection->saddr.s_addr =
        daddr;
    new_entry->protocol = protocol;

    /*
     * Initialize count fields
     */

    new_entry->pcount = new_entry->bcount = 0;
    new_entry->win = new_entry->psize = 0;
    new_entry->timedout = new_entry->oth_connection->timedout = 0;
    new_entry->oth_connection->pcount = new_entry->oth_connection->bcount =
        0;
    new_entry->oth_connection->win = new_entry->oth_connection->psize = 0;

    /*
     * Store interface name
     */

    strcpy(new_entry->ifname, ifname);
    strcpy(new_entry->oth_connection->ifname, ifname);

    /*
     * Zero out MAC address fields
     */

    bzero(new_entry->smacaddr, 15);
    bzero(new_entry->oth_connection->smacaddr, 15);

    /*
     * Set raw port numbers
     */

    new_entry->sport = new_entry->oth_connection->dport = ntohs(sport);
    new_entry->dport = new_entry->oth_connection->sport = ntohs(dport);

    new_entry->stat = new_entry->oth_connection->stat = 0;

    new_entry->s_fstat = revname(rev_lookup, &(new_entry->saddr),
                                 new_entry->s_fqdn, rvnfd);
    new_entry->d_fstat = revname(rev_lookup, &(new_entry->daddr),
                                 new_entry->d_fqdn, rvnfd);

    /*
     * Set port service names (where applicable)
     */

    servlook(servnames, sport, IPPROTO_TCP, new_entry->s_sname, 10);
    servlook(servnames, dport, IPPROTO_TCP, new_entry->d_sname, 10);

    strcpy(new_entry->oth_connection->s_sname, new_entry->d_sname);
    strcpy(new_entry->oth_connection->d_sname, new_entry->s_sname);

    strcpy(new_entry->oth_connection->d_fqdn, new_entry->s_fqdn);
    strcpy(new_entry->oth_connection->s_fqdn, new_entry->d_fqdn);
    new_entry->oth_connection->s_fstat = new_entry->d_fstat;
    new_entry->oth_connection->d_fstat = new_entry->s_fstat;

    if (new_entry->index < new_entry->oth_connection->index) {
        new_entry->half_bracket = ACS_ULCORNER;
        new_entry->oth_connection->half_bracket = ACS_LLCORNER;
    } else {
        new_entry->half_bracket = ACS_LLCORNER;
        new_entry->oth_connection->half_bracket = ACS_ULCORNER;
    }

    new_entry->inclosed = new_entry->oth_connection->inclosed = 0;
    new_entry->finack = new_entry->oth_connection->finack = 0;
    new_entry->finsent = new_entry->oth_connection->finsent = 0;
    new_entry->partial = new_entry->oth_connection->partial = 0;
    new_entry->spanbr = new_entry->oth_connection->spanbr = 0;
    new_entry->conn_starttime = new_entry->oth_connection->conn_starttime =
        time(NULL);

    /*
     * Mark flow rate start time and byte counter for flow computation
     * if the highlight bar is on either flow of the new connection.
     */
    if (table->barptr == new_entry) {
        new_entry->starttime = time(NULL);
        new_entry->spanbr = 0;
    } else if (table->barptr == new_entry->oth_connection) {
        new_entry->oth_connection->starttime = time(NULL);
        new_entry->oth_connection->spanbr = 0;
    }

    /*
     * Add entries to hash table
     */

    *nomem = add_tcp_hash_entry(table, new_entry);
    *nomem = add_tcp_hash_entry(table, new_entry->oth_connection);

    return new_entry;
}

void addtoclosedlist(struct tcptable *table, struct tcptableent *entry,
                     int *nomem)
{
    struct closedlist *ctemp;

    ctemp = malloc(sizeof(struct closedlist));

    if (ctemp == NULL) {
        printnomem();
        *nomem = 1;
        return;
    }
    /*
     * Point to closed entries
     */
    ctemp->closedentry = entry;
    ctemp->pair = entry->oth_connection;
    entry->inclosed = entry->oth_connection->inclosed = 1;

    /*
     * Add node to closed entry list.
     */

    if (table->closedtail != NULL)
        table->closedtail->next_entry = ctemp;

    table->closedtail = ctemp;
    table->closedtail->next_entry = NULL;

    if (table->closedentries == NULL)
        table->closedentries = ctemp;

}

char *tcplog_flowrate_msg(struct tcptableent *entry, struct OPTIONS *opts)
{
    char rateunit[10];
    float rate = 0;
    static char message[60];
    time_t interval;

    interval = time(NULL) - entry->conn_starttime;

    if (opts->actmode == KBITS) {
        strcpy(rateunit, "kbits/s");

        if (interval > 0)
            rate = (float) (entry->bcount * 8 / 1000) / (float) interval;
        else
            rate = (float) (entry->bcount * 8 / 1000);
    } else {
        strcpy(rateunit, "kbytes/s");

        if (interval > 0)
            rate = (float) (entry->bcount / 1024) / (float) interval;
        else
            rate = (float) (entry->bcount / 1024);
    }

    snprintf(message, 60, "avg flow rate %.2f %s", rate, rateunit);
    return message;
}

void write_timeout_log(int logging, FILE * logfile,
                       struct tcptableent *tcpnode, struct OPTIONS *opts)
{
    char msgstring[MSGSTRING_MAX];

    if (logging) {
        snprintf(msgstring, MSGSTRING_MAX,
                 "TCP; Connection %s:%s to %s:%s timed out, %lu packets, %lu bytes, %s; opposite direction %lu packets, %lu bytes, %s",
                 tcpnode->s_fqdn, tcpnode->s_sname,
                 tcpnode->d_fqdn, tcpnode->d_sname,
                 tcpnode->pcount, tcpnode->bcount,
                 tcplog_flowrate_msg(tcpnode, opts),
                 tcpnode->oth_connection->pcount,
                 tcpnode->oth_connection->bcount,
                 tcplog_flowrate_msg(tcpnode->oth_connection, opts));
        writelog(logging, logfile, msgstring);
    }
}

struct tcptableent *in_table(struct tcptable *table, unsigned long saddr,
                             unsigned long daddr, unsigned int sport,
                             unsigned int dport, char *ifname,
                             int logging, FILE * logfile,
                             int *nomem, struct OPTIONS *opts)
{
    struct tcp_hashentry *hashptr;
    unsigned int hp;

    int hastimeouts = 0;

    time_t now;
    time_t timeout;

    if (opts != NULL)
        timeout = opts->timeout;
    else
        timeout = 0;

    if (table->head == NULL) {
        return 0;
    }
    /*
     * Determine hash table index for this set of addresses and ports
     */

    hp = tcp_hash(saddr, sport, daddr, dport, ifname);
    hashptr = table->hash_table[hp];

    while (hashptr != NULL) {
        if ((hashptr->tcpnode->saddr.s_addr == saddr) &&
            (hashptr->tcpnode->daddr.s_addr == daddr) &&
            (hashptr->tcpnode->sport == sport) &&
            (hashptr->tcpnode->dport == dport) &&
            (strcmp(hashptr->tcpnode->ifname, ifname) == 0))
            break;

        now = time(NULL);

        /*
         * Add the timed out entries to the closed list in case we didn't
         * find any closed ones.
         */

        if ((timeout > 0)
            && ((now - hashptr->tcpnode->lastupdate) / 60 > timeout)
            && (!(hashptr->tcpnode->inclosed))) {
            hashptr->tcpnode->timedout = 1;
            hashptr->tcpnode->oth_connection->timedout = 1;
            addtoclosedlist(table, hashptr->tcpnode, nomem);
            if (!(*nomem))
                hastimeouts = 1;

            if (logging)
                write_timeout_log(logging, logfile, hashptr->tcpnode,
                                  opts);
        }
        hashptr = hashptr->next_entry;
    }

    if (hashptr != NULL) {      /* needed to avoid SIGSEGV */
        if ((((hashptr->tcpnode->finsent == 2) &&
              (hashptr->tcpnode->oth_connection->finsent == 2))) ||
            (((hashptr->tcpnode->stat & FLAG_RST) ||
              (hashptr->tcpnode->oth_connection->stat & FLAG_RST)))) {
            return NULL;
        } else {
            return hashptr->tcpnode;
        }
    } else {
        return NULL;
    }
}


/*
 * Update the TCP status record should an applicable packet arrive.
 */

void updateentry(struct tcptable *table, struct tcptableent *tableentry,
                 struct tcphdr *transpacket, char *packet, int linkproto,
                 unsigned long packetlength,
                 unsigned int bcount, unsigned int fragofs, int logging,
                 int *revlook, int rvnfd,
                 struct OPTIONS *opts, FILE * logfile, int *nomem)
{
    char msgstring[MSGSTRING_MAX];
    char newmacaddr[15];

    if (tableentry->s_fstat != RESOLVED) {
        tableentry->s_fstat = revname(revlook, &(tableentry->saddr),
                                      tableentry->s_fqdn, rvnfd);
        strcpy(tableentry->oth_connection->d_fqdn, tableentry->s_fqdn);
        tableentry->oth_connection->d_fstat = tableentry->s_fstat;
    }
    if (tableentry->d_fstat != RESOLVED) {
        tableentry->d_fstat = revname(revlook, &(tableentry->daddr),
                                      tableentry->d_fqdn, rvnfd);
        strcpy(tableentry->oth_connection->s_fqdn, tableentry->d_fqdn);
        tableentry->oth_connection->s_fstat = tableentry->d_fstat;
    }
    tableentry->pcount++;
    tableentry->bcount += bcount;
    tableentry->psize = packetlength;
    tableentry->spanbr += bcount;

    if (opts->mac) {
        bzero(newmacaddr, 15);

        if ((linkproto == LINK_ETHERNET) || (linkproto == LINK_PLIP)) {
            convmacaddr(((struct ethhdr *) packet)->h_source, newmacaddr);
        } else if (linkproto == LINK_FDDI) {
            convmacaddr(((struct fddihdr *) packet)->saddr, newmacaddr);
        } else if (linkproto == LINK_TR) {
            convmacaddr(((struct trh_hdr *) packet)->saddr, newmacaddr);
        }

        if (tableentry->smacaddr[0] != '\0') {
            if (strcmp(tableentry->smacaddr, newmacaddr) != 0) {
                snprintf(msgstring, MSGSTRING_MAX,
                         "TCP; %s; from %s:%s to %s:%s: new source MAC address %s (previously %s)",
                         tableentry->ifname, tableentry->s_fqdn,
                         tableentry->s_sname, tableentry->d_fqdn,
                         tableentry->d_sname, newmacaddr,
                         tableentry->smacaddr);
                writelog(logging, logfile, msgstring);
                strcpy(tableentry->smacaddr, newmacaddr);
            }
        } else
            strcpy(tableentry->smacaddr, newmacaddr);
    }

    /*
     * If this is not the first TCP fragment, skip interpretation of the
     * TCP header.
     */

    if ((ntohs(fragofs) & 0x1fff) != 0) {
        tableentry->lastupdate = tableentry->oth_connection->lastupdate =
            time(NULL);
        return;
    }
    /*
     * At this point, we have a TCP header, and we proceed to process it.
     */

    if (tableentry->pcount == 1) {
        if ((transpacket->syn) || (transpacket->rst))
            tableentry->partial = 0;
        else
            tableentry->partial = 1;
    }
    tableentry->win = ntohs(transpacket->window);

    tableentry->stat = 0;

    if (transpacket->syn)
        tableentry->stat |= FLAG_SYN;

    if (transpacket->ack) {
        tableentry->stat |= FLAG_ACK;

        /*
         * The following sequences are used when the ACK is in response to
         * a FIN (see comments for FIN below).  If the opposite direction
         * already has its indicator set to 1 (FIN sent, not ACKed), and
         * the incoming ACK has the same sequence number as the previously
         * stored FIN's ack number (i.e. the ACK in response to the opposite
         * flow's FIN), the opposite direction's state is set to 2 (FIN sent
         * and ACKed).
         */

        if ((tableentry->oth_connection->finsent == 1) &&
            (ntohl(transpacket->seq) ==
             tableentry->oth_connection->finack)) {
            tableentry->oth_connection->finsent = 2;

            if (logging) {
                writetcplog(logging, logfile, tableentry,
                            tableentry->psize, opts->mac,
                            "FIN acknowleged");
            }
        }
    }
    /*
     * The closing sequence is similar, but not identical to the TCP close
     * sequence described in the RFC.  This sequence is primarily cosmetic.
     *
     * When a FIN is sent in a direction, a state indicator is set to 1,
     * to indicate a FIN sent, but not ACKed yet.  For comparison later,
     * the acknowlegement number is also saved in the entry.  See comments
     * in ACK above.
     */

    if (transpacket->fin) {

        /*
         * First, we check if the opposite direction has no counts, in which
         * case we simply mark the entire connection available for reuse.
         * This is in case packets from a machine pass an interface, but
         * on the return, completely bypasses any interface on our machine.
         *
         * Q: Could such a situation really happen in practice?  I managed to
         * do it but under *really* ridiculous circumstances.
         *
         * A: (as of version 2.5.0, June 2001): Yes this DOES happen in
         * practice.  Unidirectional satellite feeds can send data straight
         * to a remote network using you as your upstream.
         */

        if (tableentry->oth_connection->pcount == 0)
            addtoclosedlist(table, tableentry, nomem);
        else {

            /*
             * That aside, mark the direction as being done, and make it
             * ready for a complete close upon receipt of an ACK.  We save
             * the acknowlegement number for identification of the proper
             * ACK packet when it arrives in the other direction.
             */

            tableentry->finsent = 1;
            tableentry->finack = ntohl(transpacket->ack_seq);
        }
        if (logging) {
            sprintf(msgstring, "FIN sent; %lu packets, %lu bytes, %s",
                    tableentry->pcount, tableentry->bcount,
                    tcplog_flowrate_msg(tableentry, opts));

            writetcplog(logging, logfile, tableentry, tableentry->psize,
                        opts->mac, msgstring);
        }
    }
    if (transpacket->rst) {
        tableentry->stat |= FLAG_RST;
        if (!(tableentry->inclosed))
            addtoclosedlist(table, tableentry, nomem);

        if (logging) {
            snprintf(msgstring, MSGSTRING_MAX,
                     "Connection reset; %lu packets, %lu bytes, %s; opposite direction %lu packets, %lu bytes; %s",
                     tableentry->pcount, tableentry->bcount,
                     tcplog_flowrate_msg(tableentry, opts),
                     tableentry->oth_connection->pcount,
                     tableentry->oth_connection->bcount,
                     tcplog_flowrate_msg(tableentry->oth_connection,
                                         opts));
            writetcplog(logging, logfile, tableentry, tableentry->psize,
                        opts->mac, msgstring);
        }
    }
    if (transpacket->psh)
        tableentry->stat |= FLAG_PSH;

    if (transpacket->urg)
        tableentry->stat |= FLAG_URG;

    tableentry->lastupdate = tableentry->oth_connection->lastupdate =
        time(NULL);
    /*
     * Shall we add this entry to the closed entry list?  If both
     * directions have their state indicators set to 2, or one direction
     * is set to 2, and the other 1, that's it.
     */

    if ((!tableentry->inclosed) &&
        (((tableentry->finsent
           == 2) && ((tableentry->oth_connection->finsent == 1)
                     || (tableentry->oth_connection->finsent == 2)))
         || ((tableentry->oth_connection->finsent == 2)
             && ((tableentry->finsent == 1)
                 || (tableentry->finsent == 2)))))
        addtoclosedlist(table, tableentry, nomem);

}

/*
 * Clears out the resolved IP addresses from the window.  This prevents
 * overlapping port numbers (in cases where the resolved DNS name is shorter
 * than its IP address), that may cause the illusion of large ports.  Plus,
 * such output, while may be interpreted by people with a little know-how,
 * is just plain wrong.
 *
 * Returns immediately if the entry is not visible in the window.
 */

void clearaddr(struct tcptable *table, struct tcptableent *tableentry,
               unsigned int screen_idx)
{
    unsigned int target_row;

    if ((tableentry->index < screen_idx) ||
        (tableentry->index > screen_idx + (table->imaxy - 1)))
        return;

    target_row = (tableentry->index) - screen_idx;

    wmove(table->tcpscreen, target_row, 1);
    wprintw(table->tcpscreen, "%44c", ' ');
}

/*
 * Display a TCP connection line.  Returns immediately if the entry is
 * not visible in the window.
 */

void printentry(struct tcptable *table, struct tcptableent *tableentry,
                unsigned int screen_idx, int mode)
{
    char stat[7] = "";
    unsigned int target_row;
    char sp_buf[MSGSTRING_MAX];
    int normalattr;
    int highattr;

    /*
     * Set appropriate attributes for this entry
     */

    if (table->barptr == tableentry) {
        normalattr = BARSTDATTR;
        highattr = BARHIGHATTR;
    } else {
        normalattr = STDATTR;
        highattr = HIGHATTR;
    }

    if ((tableentry->index < screen_idx) ||
        (tableentry->index > screen_idx + (table->imaxy - 1)))
        return;

    target_row = (tableentry->index) - screen_idx;

    /* clear the data if it's a reused entry */

    wattrset(table->tcpscreen, PTRATTR);
    wmove(table->tcpscreen, target_row, 2);
    if (tableentry->reused) {
        scrollok(table->tcpscreen, 0);
        sprintf(sp_buf, "%%%dc", COLS - 4);
        wprintw(table->tcpscreen, sp_buf, ' ');
        scrollok(table->tcpscreen, 1);
        tableentry->reused = 0;
        wmove(table->tcpscreen, target_row, 1);
    }
    /* print half of connection indicator bracket */

    wmove(table->tcpscreen, target_row, 0);
    waddch(table->tcpscreen, tableentry->half_bracket);

    /* proceed with the actual entry */

    wattrset(table->tcpscreen, normalattr);
    sprintf(sp_buf, "%%%dc", COLS - 5);
    mvwprintw(table->tcpscreen, target_row, 2, sp_buf, ' ');

    sprintf(sp_buf, "%%.%ds:%%.%ds", 32 * COLS / 80, 10);

    wmove(table->tcpscreen, target_row, 1);
    wprintw(table->tcpscreen, sp_buf, tableentry->s_fqdn,
            tableentry->s_sname);

    wattrset(table->tcpscreen, highattr);

    /* 
     * Print packet and byte counts or window size and packet size, depending
     * on the value of mode.
     */

    switch (mode) {
    case 0:
        wmove(table->tcpscreen, target_row, 47 * COLS / 80 - 2);
        if (tableentry->partial)
            wprintw(table->tcpscreen, ">");
        else
            wprintw(table->tcpscreen, "=");
        wprintw(table->tcpscreen, "%8u  ", tableentry->pcount);
        wmove(table->tcpscreen, target_row, 59 * COLS / 80 - 4);
        wprintw(table->tcpscreen, "%9u  ", tableentry->bcount);
        break;
    case 1:
        wmove(table->tcpscreen, target_row, 50 * COLS / 80);
        if (tableentry->smacaddr[0] == '\0')
            wprintw(table->tcpscreen, "    N/A    ");
        else
            wprintw(table->tcpscreen, "%s", tableentry->smacaddr);
        break;
    case 2:
        wmove(table->tcpscreen, target_row, 45 * COLS / 80 + 3);
        wprintw(table->tcpscreen, "%5u  ", tableentry->psize);
        wmove(table->tcpscreen, target_row, 56 * COLS / 80 - 1);
        wprintw(table->tcpscreen, "%9u  ", tableentry->win);
    }

    wattrset(table->tcpscreen, normalattr);

    if (tableentry->finsent == 1)
        strcpy(stat, "DONE  ");
    else if (tableentry->finsent == 2)
        strcpy(stat, "CLOSED");
    else if (tableentry->stat & FLAG_RST)
        strcpy(stat, "RESET ");
    else {
        if (tableentry->stat & FLAG_SYN)
            strcat(stat, "S");
        else
            strcat(stat, "-");

        if (tableentry->stat & FLAG_PSH)
            strcat(stat, "P");
        else
            strcat(stat, "-");

        if (tableentry->stat & FLAG_ACK)
            strcat(stat, "A");
        else
            strcat(stat, "-");

        if (tableentry->stat & FLAG_URG)
            strcat(stat, "U");
        else
            strcat(stat, "-");

        strcat(stat, " ");
    }

    wmove(table->tcpscreen, target_row, 65 * COLS / 80);
    wprintw(table->tcpscreen, "%s", stat);
    wmove(table->tcpscreen, target_row, 72 * COLS / 80);
    wprintw(table->tcpscreen, "%s", tableentry->ifname);
}

/*
 * Redraw the TCP window
 */

void refreshtcpwin(struct tcptable *table, unsigned int idx, int mode)
{
    struct tcptableent *ptmp;

    setlabels(table->borderwin, mode);
    wattrset(table->tcpscreen, STDATTR);
    tx_colorwin(table->tcpscreen);
    ptmp = table->firstvisible;

    while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
        printentry(table, ptmp, idx, mode);
        ptmp = ptmp->next_entry;
    }

    wmove(table->borderwin, table->bmaxy - 1, 1);

    print_tcp_num_entries(table);

    update_panels();
    doupdate();
}

void destroy_closed_entries(struct tcptable *table)
{
    struct closedlist *closedtemp;
    struct closedlist *closedtemp_next;

    if (table->closedentries != NULL) {
        closedtemp = table->closedentries;
        closedtemp_next = table->closedentries->next_entry;

        while (closedtemp != NULL) {
            free(closedtemp);
            closedtemp = closedtemp_next;
            if (closedtemp_next != NULL)
                closedtemp_next = closedtemp_next->next_entry;
        }

        table->closedentries = NULL;
        table->closedtail = NULL;
    }
}

/*
 * Kill the entire TCP table
 */
void destroytcptable(struct tcptable *table)
{
    struct tcptableent *ctemp;
    struct tcptableent *c_next_entry;
    struct tcp_hashentry *hashtemp;
    struct tcp_hashentry *hashtemp_next;

    unsigned int i;

    /*
     * Destroy main TCP table
     */

    if (table->head != NULL) {
        ctemp = table->head;
        c_next_entry = table->head->next_entry;

        while (ctemp != NULL) {
            free(ctemp);
            ctemp = c_next_entry;

            if (c_next_entry != NULL)
                c_next_entry = c_next_entry->next_entry;
        }
    }
    /*
     * Destroy list of closed entries
     */

    destroy_closed_entries(table);

    /*
     * Destroy hash table
     */

    for (i = 0; i <= ENTRIES_IN_HASH_TABLE - 1; i++) {
        if (table->hash_table[i] != NULL) {
            hashtemp = table->hash_table[i];
            hashtemp_next = table->hash_table[i]->next_entry;

            while (hashtemp != NULL) {
                free(hashtemp);
                hashtemp = hashtemp_next;

                if (hashtemp_next != NULL)
                    hashtemp_next = hashtemp_next->next_entry;
            }
        }
    }
}

/*
 * Kill an entry from the TCP table
 */

void destroy_tcp_entry(struct tcptable *table, struct tcptableent *ptmp)
{
    if (ptmp->prev_entry != NULL)
        ptmp->prev_entry->next_entry = ptmp->next_entry;
    else
        table->head = ptmp->next_entry;

    if (ptmp->next_entry != NULL)
        ptmp->next_entry->prev_entry = ptmp->prev_entry;
    else
        table->tail = ptmp->prev_entry;

    free(ptmp);

    if (table->head == NULL) {
        table->firstvisible = NULL;
        table->lastvisible = NULL;
    }
}

/*
 * Kill all closed entries from the table, and clear the list of closed
 * entries.
 */

void flushclosedentries(struct tcptable *table,
                        unsigned long *screen_idx,
                        int logging, FILE * logfile, struct OPTIONS *opts)
{
    struct tcptableent *ptmp = table->head;
    struct tcptableent *ctmp = NULL;
    unsigned long idx = 1;
    time_t now;
    time_t lastupdated = 0;

    while (ptmp != NULL) {
        now = time(NULL);
        lastupdated = (now - ptmp->lastupdate) / 60;

        if ((ptmp->inclosed) || (lastupdated > opts->timeout)) {
            ctmp = ptmp;
            /*
             * Mark and flush timed out TCP entries.
             */
            if (lastupdated > opts->timeout) {
                if ((!(ptmp->timedout)) && (!(ptmp->inclosed))) {
                    write_timeout_log(logging, logfile, ptmp, opts);
                    ptmp->timedout = ptmp->oth_connection->timedout = 1;
                }
            }

            /*
             * Advance to next entry and destroy target entry.
             */
            ptmp = ptmp->next_entry;

            /*
             * If the targeted entry is highlighted, and the next entry is
             * not NULL (we're still in the list) we move the bar pointer to
             * the next entry otherwise we move it to the previous entry.
             */
            if (ptmp != NULL) {
                if (table->barptr == ctmp) {
                    table->barptr = ptmp;
                }
            } else {
                if (table->barptr == ctmp) {
                    table->barptr = table->barptr->prev_entry;
                }
            }

            /*
             * Do the dirty deed
             */
            del_tcp_hash_node(table, ctmp);
            destroy_tcp_entry(table, ctmp);

            /*
             * Adjust screen index if the deleted entry was "above"
             * the screen.
             */
            if (idx < *screen_idx)
                (*screen_idx)--;
        } else {
            /*
             * Set the first visible pointer once the index matches
             * the screen index.
             */
            if (idx == *screen_idx)
                table->firstvisible = ptmp;

            /*
             * Keep setting the last visible pointer until the scan
             * index "leaves" the screen
             */
            if (idx <= (*screen_idx) + (table->imaxy - 1))
                table->lastvisible = ptmp;

            ptmp->index = idx;
            idx++;
            ptmp = ptmp->next_entry;
        }
    }

    table->lastpos = idx - 1;
    table->count = table->lastpos / 2;
    destroy_closed_entries(table);

    /*
     * Shift entries down if the deletion causes the last entry to
     * occupy anywhere other than the last line of the TCP display
     * window.
     */

    if (table->head != NULL) {
        /*
         * Point screen index to the last table entry if the tail entry is
         * "above" the screen index.  Set the firstvisible pointer to that
         * as well.
         */
        if (table->tail->index < *screen_idx) {
            *screen_idx = table->tail->index;
            table->firstvisible = table->tail;
        }

        /*
         * Move the screen index and firstvisible entry up until the tail
         * hits the bottom of the window (tail is at screen index plus
         * screen length minus 1) or the firstvisible pointer hits the
         * head of the table.  The highlight bar should "go along" with
         * the shifting.
         */
        while ((table->tail->index < *screen_idx + table->imaxy - 1) &&
               (table->firstvisible->prev_entry != NULL)) {
            table->firstvisible = table->firstvisible->prev_entry;
            (*screen_idx)--;
        }

        /*
         * Set the bar position index once everything's done.
         */
        table->baridx = table->barptr->index - *screen_idx + 1;
    }
}
