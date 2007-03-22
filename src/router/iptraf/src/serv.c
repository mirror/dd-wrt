/***

serv.c  - TCP/UDP port statistics module
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

#include <curses.h>
#include <panel.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <listbox.h>
#include <labels.h>
#include <winops.h>
#include <input.h>
#include <msgboxes.h>
#include "tcphdr.h"
#include "dirs.h"
#include "ipcsum.h"
#include "deskman.h"
#include "isdntab.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "packet.h"
#include "ipfrag.h"
#include "ifaces.h"
#include "attrs.h"
#include "serv.h"
#include "servname.h"
#include "log.h"
#include "timer.h"
#include "promisc.h"
#include "options.h"
#include "instances.h"
#include "packet.h"
#include "logvars.h"
#include "error.h"
#include "bar.h"
#include "mode.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

#define LEFT 0
#define RIGHT 1

extern int exitloop;
extern int daemonized;

extern void writeutslog(struct portlistent *list, unsigned long nsecs,
                        int unit, FILE * logfile);

/*
 * SIGUSR1 logfile rotation signal handler
 */

void rotate_serv_log()
{
    rotate_flag = 1;
    strcpy(target_logname, current_logfile);
    signal(SIGUSR1, rotate_serv_log);
}

void initportlist(struct portlist *list)
{
    float screen_scale = ((float) COLS / 80 + 1) / 2;
    int scratchx;

    list->head = list->tail = list->barptr = NULL;
    list->firstvisible = list->lastvisible = NULL;
    list->count = 0;
    list->baridx = 0;

    list->borderwin = newwin(LINES - 3, COLS, 1, 0);
    list->borderpanel = new_panel(list->borderwin);
    wattrset(list->borderwin, BOXATTR);
    tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);

    wmove(list->borderwin, 0, 1 * screen_scale);
    wprintw(list->borderwin, " Proto/Port ");
    wmove(list->borderwin, 0, 22 * screen_scale);
    wprintw(list->borderwin, " Pkts ");
    wmove(list->borderwin, 0, 31 * screen_scale);
    wprintw(list->borderwin, " Bytes ");
    wmove(list->borderwin, 0, 40 * screen_scale);
    wprintw(list->borderwin, " PktsTo ");
    wmove(list->borderwin, 0, 49 * screen_scale);
    wprintw(list->borderwin, " BytesTo ");
    wmove(list->borderwin, 0, 58 * screen_scale);
    wprintw(list->borderwin, " PktsFrom ");
    wmove(list->borderwin, 0, 67 * screen_scale);
    wprintw(list->borderwin, " BytesFrom ");

    list->win = newwin(LINES - 5, COLS - 2, 2, 1);
    list->panel = new_panel(list->win);
    getmaxyx(list->win, list->imaxy, scratchx);

    tx_stdwinset(list->win);
    wtimeout(list->win, -1);
    wattrset(list->win, STDATTR);
    tx_colorwin(list->win);
    update_panels();
    doupdate();
}

struct portlistent *addtoportlist(struct portlist *list,
                                  unsigned int protocol, unsigned int port,
                                  int *nomem, int servnames)
{
    struct portlistent *ptemp;

    ptemp = malloc(sizeof(struct portlistent));

    if (ptemp == NULL) {
        printnomem();
        *nomem = 1;
        return NULL;
    }
    if (list->head == NULL) {
        ptemp->prev_entry = NULL;
        list->head = ptemp;
        list->firstvisible = ptemp;
    }

    if (list->tail != NULL) {
        list->tail->next_entry = ptemp;
        ptemp->prev_entry = list->tail;
    }
    list->tail = ptemp;
    ptemp->next_entry = NULL;

    ptemp->protocol = protocol;
    ptemp->port = port;         /* This is used in checks later. */

    /* 
     * Obtain appropriate service name
     */

    servlook(servnames, htons(port), protocol, ptemp->servname, 10);

    ptemp->count = ptemp->bcount = 0;
    ptemp->icount = ptemp->ibcount = 0;
    ptemp->ocount = ptemp->obcount = 0;

    list->count++;
    ptemp->idx = list->count;

    ptemp->proto_starttime = time(NULL);

    if (list->count <= LINES - 5)
        list->lastvisible = ptemp;

    wmove(list->borderwin, LINES - 4, 1);
    wprintw(list->borderwin, " %u entries ", list->count);

    return ptemp;
}

int portinlist(struct porttab *table, unsigned int port)
{
    struct porttab *ptmp = table;

    while (ptmp != NULL) {
        if (((ptmp->port_max == 0) && (ptmp->port_min == port)) ||
            ((port >= ptmp->port_min) && (port <= ptmp->port_max)))
            return 1;

        ptmp = ptmp->next_entry;
    }

    return 0;
}

int goodport(unsigned int port, struct porttab *table)
{
    return ((port < 1024) || (portinlist(table, port)));
}

struct portlistent *inportlist(struct portlist *list,
                               unsigned int protocol, unsigned int port)
{
    struct portlistent *ptmp = list->head;

    while (ptmp != NULL) {
        if ((ptmp->port == port) && (ptmp->protocol == protocol))
            return ptmp;

        ptmp = ptmp->next_entry;
    }

    return NULL;
}

void printportent(struct portlist *list, struct portlistent *entry,
                  unsigned int idx)
{
    unsigned int target_row;
    float screen_scale = ((float) COLS / 80 + 1) / 2;
    int tcplabelattr;
    int udplabelattr;
    int highattr;
    char sp_buf[10];

    if ((entry->idx < idx) || (entry->idx > idx + (LINES - 6)))
        return;

    target_row = entry->idx - idx;

    if (entry == list->barptr) {
        tcplabelattr = BARSTDATTR;
        udplabelattr = BARPTRATTR;
        highattr = BARHIGHATTR;
    } else {
        tcplabelattr = STDATTR;
        udplabelattr = PTRATTR;
        highattr = HIGHATTR;
    }

    wattrset(list->win, tcplabelattr);
    sprintf(sp_buf, "%%%dc", COLS - 2);
    scrollok(list->win, 0);
    mvwprintw(list->win, target_row, 0, sp_buf, ' ');
    scrollok(list->win, 1);

    wmove(list->win, target_row, 1);
    if (entry->protocol == IPPROTO_TCP) {
        wattrset(list->win, tcplabelattr);
        wprintw(list->win, "TCP");
    } else if (entry->protocol == IPPROTO_UDP) {
        wattrset(list->win, udplabelattr);
        wprintw(list->win, "UDP");
    }

    wprintw(list->win, "/%s          ", entry->servname);
    wattrset(list->win, highattr);
    wmove(list->win, target_row, 17 * screen_scale);
    printlargenum(entry->count, list->win);
    wmove(list->win, target_row, 27 * screen_scale);
    printlargenum(entry->bcount, list->win);
    wmove(list->win, target_row, 37 * screen_scale);
    printlargenum(entry->icount, list->win);
    wmove(list->win, target_row, 47 * screen_scale);
    printlargenum(entry->ibcount, list->win);
    wmove(list->win, target_row, 57 * screen_scale);
    printlargenum(entry->ocount, list->win);
    wmove(list->win, target_row, 67 * screen_scale);
    printlargenum(entry->obcount, list->win);
}

void destroyportlist(struct portlist *list)
{
    struct portlistent *ptmp = list->head;
    struct portlistent *ctmp = NULL;

    if (list->head != NULL)
        ctmp = list->head->next_entry;

    while (ptmp != NULL) {
        free(ptmp);
        ptmp = ctmp;

        if (ctmp != NULL)
            ctmp = ctmp->next_entry;
    }
}

void updateportent(struct portlist *list, unsigned int protocol,
                   unsigned int sport, unsigned int dport, int br,
                   unsigned int idx, struct porttab *ports, int servnames)
{
    struct portlistent *sport_listent = NULL;
    struct portlistent *dport_listent = NULL;
    int nomem = 0;

    if (goodport(sport, ports)) {
        sport_listent = inportlist(list, protocol, sport);

        if ((sport_listent == NULL) && (!nomem))
            sport_listent =
                addtoportlist(list, protocol, sport, &nomem, servnames);

        if (sport_listent == NULL)
            return;

        sport_listent->count++;
        sport_listent->bcount += br;
        sport_listent->spans.spanbr += br;

        sport_listent->obcount += br;
        sport_listent->spans.spanbr_out += br;
        sport_listent->ocount++;
    }

    if (goodport(dport, ports)) {
        dport_listent = inportlist(list, protocol, dport);

        if ((dport_listent == NULL) && (!nomem))
            dport_listent =
                addtoportlist(list, protocol, dport, &nomem, servnames);

        if (dport_listent == NULL)
            return;

        if (dport_listent != sport_listent) {
            dport_listent->count++;
            dport_listent->bcount += br;
            dport_listent->spans.spanbr += br;
        }

        dport_listent->ibcount += br;
        dport_listent->spans.spanbr_out += br;
        dport_listent->icount++;
    }
    if (sport_listent != NULL || dport_listent != NULL) {
        if (sport_listent != NULL)
            printportent(list, sport_listent, idx);
        if (dport_listent != NULL && dport_listent != sport_listent)
            printportent(list, dport_listent, idx);
    }
}

/*
 * Swap two port list entries.  p1 must be previous to p2.
 */

void swapportents(struct portlist *list,
                  struct portlistent *p1, struct portlistent *p2)
{
    register unsigned int tmp;
    struct portlistent *p1prevsaved;
    struct portlistent *p2nextsaved;

    if (p1 == p2)
        return;

    tmp = p1->idx;
    p1->idx = p2->idx;
    p2->idx = tmp;

    if (p1->prev_entry != NULL)
        p1->prev_entry->next_entry = p2;
    else
        list->head = p2;

    if (p2->next_entry != NULL)
        p2->next_entry->prev_entry = p1;
    else
        list->tail = p1;

    p2nextsaved = p2->next_entry;
    p1prevsaved = p1->prev_entry;

    if (p1->next_entry == p2) {
        p2->next_entry = p1;
        p1->prev_entry = p2;
    } else {
        p2->next_entry = p1->next_entry;
        p1->prev_entry = p2->prev_entry;
        p2->prev_entry->next_entry = p1;
        p1->next_entry->prev_entry = p2;
    }

    p2->prev_entry = p1prevsaved;
    p1->next_entry = p2nextsaved;
}

/*
 * Retrieve the appropriate sort criterion based on keystroke.
 */
unsigned long long qp_getkey(struct portlistent *entry, int ch)
{
    unsigned long long result = 0;

    switch (ch) {
    case 'R':
        result = entry->port;
        break;
    case 'B':
        result = entry->bcount;
        break;
    case 'O':
        result = entry->ibcount;
        break;
    case 'M':
        result = entry->obcount;
        break;
    case 'P':
        result = entry->count;
        break;
    case 'T':
        result = entry->icount;
        break;
    case 'F':
        result = entry->ocount;
        break;
    }

    return result;
}

/*
 * Refresh TCP/UDP service screen.
 */

void refresh_serv_screen(struct portlist *table, int idx)
{
    struct portlistent *ptmp = table->firstvisible;

    wattrset(table->win, STDATTR);
    tx_colorwin(table->win);

    while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
        printportent(table, ptmp, idx);
        ptmp = ptmp->next_entry;
    }
    update_panels();
    doupdate();
}


/*
 * Compare the sort criterion with the pivot value.  Receives a parameter
 * specifying whether the criterion is left or right of the pivot value.
 *
 * If criterion is the port number: return true if criterion is less than or
 *     equal to the pivot when the SIDE is left.  If SIDE is right, return
 *     true if the value is greater than the pivot.  This results in an
 *     ascending sort.
 *
 * If the criterion is a count: return true when the criterion is greater than
 *     or equal to the pivot when the SIDE is left, otherwise, when SIDE is
 *     right, return true if the value is less than the pivot.  This results
 *     in a descending sort. 
 */

int qp_compare(struct portlistent *entry, unsigned long long pv, int ch,
               int side)
{
    int result = 0;
    unsigned long long value;

    value = qp_getkey(entry, ch);

    if (ch == 'R') {
        if (side == LEFT)
            result = (value <= pv);
        else
            result = (value > pv);
    } else {
        if (side == LEFT)
            result = (value >= pv);
        else
            result = (value < pv);
    }

    return result;
}

/*
 * Partition port list such that a pivot is selected, and that all values
 * left of the pivot are less (or greater) than or equal to the pivot,
 * and that all values right of the pivot are greater (or less) than
 * the pivot.
 */
struct portlistent *qp_partition(struct portlist *table,
                                 struct portlistent **low,
                                 struct portlistent **high, int ch)
{
    struct portlistent *pivot = *low;

    struct portlistent *left = *low;
    struct portlistent *right = *high;
    struct portlistent *ptmp;

    unsigned long long pivot_value;

    pivot_value = qp_getkey(pivot, ch);

    while (left->idx < right->idx) {
        while ((qp_compare(left, pivot_value, ch, LEFT))
               && (left->next_entry != NULL))
            left = left->next_entry;

        while (qp_compare(right, pivot_value, ch, RIGHT))
            right = right->prev_entry;

        if (left->idx < right->idx) {
            swapportents(table, left, right);
            if (*low == left)
                *low = right;

            if (*high == right)
                *high = left;

            ptmp = left;
            left = right;
            right = ptmp;
        }
    }
    swapportents(table, pivot, right);

    if (*low == pivot)
        *low = right;

    if (*high == right)
        *high = pivot;

    return pivot;
}

/*
 * Quicksort for the port list.
 */
void quicksort_port_entries(struct portlist *table,
                            struct portlistent *low,
                            struct portlistent *high, int ch)
{
    struct portlistent *pivot;

    if ((high == NULL) || (low == NULL))
        return;

    if (high->idx > low->idx) {
        pivot = qp_partition(table, &low, &high, ch);

        quicksort_port_entries(table, low, pivot->prev_entry, ch);
        quicksort_port_entries(table, pivot->next_entry, high, ch);
    }
}

void sortportents(struct portlist *list, int *idx, int command)
{
    struct portlistent *ptemp1;
    unsigned int idxtmp;

    if (!(list->head))
        return;

    command = toupper(command);

    if ((command != 'R') && (command != 'B') && (command != 'O') &&
        (command != 'M') && (command != 'P') && (command != 'T') &&
        (command != 'F'))
        return;

    quicksort_port_entries(list, list->head, list->tail, command);

    ptemp1 = list->firstvisible = list->head;
    *idx = 1;
    idxtmp = 1;

    while ((ptemp1) && (idxtmp <= LINES - 5)) { /* printout */
        printportent(list, ptemp1, *idx);
        if (idxtmp <= LINES - 5)
            list->lastvisible = ptemp1;
        ptemp1 = ptemp1->next_entry;
        idxtmp++;
    }
}

void scrollservwin(struct portlist *table, int direction, int *idx)
{
    char sp_buf[10];
    sprintf(sp_buf, "%%%dc", COLS - 2);
    wattrset(table->win, STDATTR);
    if (direction == SCROLLUP) {
        if (table->lastvisible != table->tail) {
            wscrl(table->win, 1);
            table->lastvisible = table->lastvisible->next_entry;
            table->firstvisible = table->firstvisible->next_entry;
            (*idx)++;
            wmove(table->win, LINES - 6, 0);
            scrollok(table->win, 0);
            wprintw(table->win, sp_buf, ' ');
            scrollok(table->win, 1);
            printportent(table, table->lastvisible, *idx);
        }
    } else {
        if (table->firstvisible != table->head) {
            wscrl(table->win, -1);
            table->lastvisible = table->lastvisible->prev_entry;
            table->firstvisible = table->firstvisible->prev_entry;
            (*idx)--;
            wmove(table->win, 0, 0);
            wprintw(table->win, sp_buf, ' ');
            printportent(table, table->firstvisible, *idx);
        }
    }
}

void pageservwin(struct portlist *table, int direction, int *idx)
{
    int i = 1;

    if (direction == SCROLLUP) {
        while ((i <= LINES - 9) && (table->lastvisible != table->tail)) {
            i++;
            table->firstvisible = table->firstvisible->next_entry;
            table->lastvisible = table->lastvisible->next_entry;
            (*idx)++;
        }
    } else {
        while ((i <= LINES - 9) && (table->firstvisible != table->head)) {
            i++;
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;
            (*idx)--;
        }
    }
    refresh_serv_screen(table, *idx);
}

void show_portsort_keywin(WINDOW ** win, PANEL ** panel)
{
    *win = newwin(14, 35, (LINES - 10) / 2, COLS - 40);
    *panel = new_panel(*win);

    wattrset(*win, DLGBOXATTR);
    tx_colorwin(*win);
    tx_box(*win, ACS_VLINE, ACS_HLINE);

    wattrset(*win, DLGTEXTATTR);
    mvwprintw(*win, 2, 2, "Select sort criterion");
    wmove(*win, 4, 2);
    tx_printkeyhelp("R", " - port number", *win, DLGHIGHATTR, DLGTEXTATTR);
    wmove(*win, 5, 2);
    tx_printkeyhelp("P", " - total packets", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 6, 2);
    tx_printkeyhelp("B", " - total bytes", *win, DLGHIGHATTR, DLGTEXTATTR);
    wmove(*win, 7, 2);
    tx_printkeyhelp("T", " - packets to", *win, DLGHIGHATTR, DLGTEXTATTR);
    wmove(*win, 8, 2);
    tx_printkeyhelp("O", " - bytes to", *win, DLGHIGHATTR, DLGTEXTATTR);
    wmove(*win, 9, 2);
    tx_printkeyhelp("F", " - packets from", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 10, 2);
    tx_printkeyhelp("M", " - bytes from", *win, DLGHIGHATTR, DLGTEXTATTR);
    wmove(*win, 11, 2);
    tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    update_panels();
    doupdate();
}

void update_serv_rates(struct portlist *list, WINDOW * win, int actmode,
                       int *cleared)
{
    char act_unit[10];
    float inrate, outrate, totalrate;
    time_t now = time(NULL);

    dispmode(actmode, act_unit);

    if (actmode == KBITS) {
        inrate =
            (float) (list->barptr->spans.spanbr_in * 8 / 1000) /
            (float) (now - list->barptr->starttime);
        outrate =
            (float) (list->barptr->spans.spanbr_out * 8 / 1000) /
            (float) (now - list->barptr->starttime);
        totalrate =
            (float) (list->barptr->spans.spanbr * 8 / 1000) /
            (float) (now - list->barptr->starttime);
    } else {
        inrate =
            (float) (list->barptr->spans.spanbr_in / 1024) / (float) (now -
                                                                      list->
                                                                      barptr->
                                                                      starttime);
        outrate =
            (float) (list->barptr->spans.spanbr_out / 1024) /
            (float) (now - list->barptr->starttime);
        totalrate =
            (float) (list->barptr->spans.spanbr / 1024) / (float) (now -
                                                                   list->
                                                                   barptr->
                                                                   starttime);
    }

    wattrset(win, IPSTATLABELATTR);
    mvwprintw(win, 0, 1,
              "Protocol data rates (%s/s):           in             out            total",
              act_unit);
    wattrset(win, IPSTATATTR);
    mvwprintw(win, 0, 31, "%10.2f", inrate);
    mvwprintw(win, 0, 46, "%10.2f", outrate);
    mvwprintw(win, 0, 61, "%10.2f", totalrate);

    bzero(&(list->barptr->spans), sizeof(struct serv_spans));
    list->barptr->starttime = time(NULL);
    *cleared = 0;
}

/*
 * The TCP/UDP service monitor
 */

void servmon(char *ifname, struct porttab *ports,
             const struct OPTIONS *options, int facilitytime,
             struct filterstate *ofilter)
{
    int logging = options->logging;
    int fd;
    int pkt_result;

    char buf[MAX_PACKET_SIZE];
    char *ipacket;

    int keymode = 0;

    struct sockaddr_ll fromaddr;
    unsigned short linktype;
    int br;

    char iface[8];
    unsigned int idx = 1;

    unsigned int sport = 0;
    unsigned int dport = 0;

    struct timeval tv;
    unsigned long starttime, startlog, timeint;
    unsigned long now;
    unsigned long long unow;
    unsigned long updtime = 0;
    unsigned long long updtime_usec = 0;

    unsigned int tot_br;

    int ch;

    struct portlist list;
    struct portlistent *serv_tmp;
    int statcleared = 0;

    FILE *logfile = NULL;

    struct promisc_states *promisc_list;

    WINDOW *sortwin;
    PANEL *sortpanel;

    WINDOW *statwin;
    PANEL *statpanel;

    char msgstring[80];
    char sp_buf[10];

    const int statx = 1;

    /*
     * Mark this facility
     */

    if (!facility_active(TCPUDPIDFILE, ifname))
        mark_facility(TCPUDPIDFILE, "TCP/UDP monitor", ifname);
    else {
        snprintf(msgstring, 80, "TCP/UDP monitor already running on %s",
                 ifname);
        write_error(msgstring, daemonized);
        return;
    }

    open_socket(&fd);

    if (fd < 0) {
        unmark_facility(TCPUDPIDFILE, ifname);
        return;
    }
    if (!iface_supported(ifname)) {
        err_iface_unsupported();
        unmark_facility(TCPUDPIDFILE, ifname);
        return;
    }
    if (!iface_up(ifname)) {
        err_iface_down();
        unmark_facility(TCPUDPIDFILE, ifname);
        return;
    }

    if ((first_active_facility()) && (options->promisc)) {
        init_promisc_list(&promisc_list);
        save_promisc_list(promisc_list);
        srpromisc(1, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, 1);
    active_facility_countfile[0] = '\0';

    initportlist(&list);
    statwin = newwin(1, COLS, LINES - 2, 0);
    statpanel = new_panel(statwin);
    scrollok(statwin, 0);
    wattrset(statwin, IPSTATLABELATTR);
    sprintf(sp_buf, "%%%dc", COLS);
    mvwprintw(statwin, 0, 0, sp_buf, ' ');

    move(LINES - 1, 1);
    scrollkeyhelp();
    sortkeyhelp();
    stdexitkeyhelp();

    if (options->servnames)
        setservent(1);

    if (logging) {
        if (strcmp(current_logfile, "") == 0) {
            snprintf(current_logfile, 80, "%s-%s.log", TCPUDPLOG, ifname);

            if (!daemonized)
                input_logfile(current_logfile, &logging);
        }
    }

    if (logging) {
        opentlog(&logfile, current_logfile);

        if (logfile == NULL)
            logging = 0;
    }
    if (logging)
        signal(SIGUSR1, rotate_serv_log);

    rotate_flag = 0;
    writelog(logging, logfile,
             "******** TCP/UDP service monitor started ********");

    isdnfd = -1;
    exitloop = 0;
    gettimeofday(&tv, NULL);
    starttime = startlog = timeint = tv.tv_sec;

    wattrset(statwin, IPSTATATTR);
    mvwprintw(statwin, 0, 1, "No entries");
    update_panels();
    doupdate();

    while (!exitloop) {
        gettimeofday(&tv, NULL);
        now = tv.tv_sec;
        unow = tv.tv_sec * 1e+6 + tv.tv_usec;

        if (now - timeint >= 5) {
            printelapsedtime(starttime, now, LINES - 4, 20,
                             list.borderwin);
            timeint = now;
        }
        if (((now - startlog) >= options->logspan) && (logging)) {
            writeutslog(list.head, now - starttime, options->actmode,
                        logfile);
            startlog = now;
        }

        if (list.barptr != NULL) {
            if ((now - list.barptr->starttime) >= 5) {
                update_serv_rates(&list, statwin, options->actmode,
                                  &statcleared);
            }
        }

        if (((options->updrate != 0)
             && (now - updtime >= options->updrate))
            || ((options->updrate == 0)
                && (unow - updtime_usec >= DEFAULT_UPDATE_DELAY))) {
            update_panels();
            doupdate();
            updtime = now;
            updtime_usec = unow;
        }
        check_rotate_flag(&logfile, logging);

        if ((facilitytime != 0)
            && (((now - starttime) / 60) >= facilitytime))
            exitloop = 1;

        getpacket(fd, buf, &fromaddr, &ch, &br, iface, list.win);

        if (ch != ERR) {
            if (keymode == 0) {
                switch (ch) {
                case KEY_UP:
                    if (list.barptr != NULL) {
                        if (list.barptr->prev_entry != NULL) {
                            serv_tmp = list.barptr;
                            set_barptr((char **) &(list.barptr),
                                       (char *) list.barptr->prev_entry,
                                       &(list.barptr->prev_entry->
                                         starttime),
                                       (char *) &(list.barptr->prev_entry->
                                                  spans),
                                       sizeof(struct serv_spans), statwin,
                                       &statcleared, statx);
                            printportent(&list, serv_tmp, idx);

                            if (list.baridx == 1) {
                                scrollservwin(&list, SCROLLDOWN, &idx);
                            } else
                                list.baridx--;

                            printportent(&list, list.barptr, idx);
                        }
                    }
                    break;
                case KEY_DOWN:
                    if (list.barptr != NULL) {
                        if (list.barptr->next_entry != NULL) {
                            serv_tmp = list.barptr;
                            set_barptr((char **) &(list.barptr),
                                       (char *) list.barptr->next_entry,
                                       &(list.barptr->next_entry->
                                         starttime),
                                       (char *) &(list.barptr->next_entry->
                                                  spans),
                                       sizeof(struct serv_spans), statwin,
                                       &statcleared, statx);
                            printportent(&list, serv_tmp, idx);

                            if (list.baridx == list.imaxy) {
                                scrollservwin(&list, SCROLLUP, &idx);
                            } else
                                list.baridx++;

                            printportent(&list, list.barptr, idx);
                        }
                    }
                    break;
                case KEY_PPAGE:
                case '-':
                    if (list.barptr != NULL) {
                        pageservwin(&list, SCROLLDOWN, &idx);

                        set_barptr((char **) &(list.barptr),
                                   (char *) (list.lastvisible),
                                   &(list.lastvisible->starttime),
                                   (char *) &(list.lastvisible->spans),
                                   sizeof(struct serv_spans), statwin,
                                   &statcleared, statx);
                        list.baridx = list.lastvisible->idx - idx + 1;

                        refresh_serv_screen(&list, idx);
                    }
                    break;
                case KEY_NPAGE:
                case ' ':
                    if (list.barptr != NULL) {
                        pageservwin(&list, SCROLLUP, &idx);

                        set_barptr((char **) &(list.barptr),
                                   (char *) (list.firstvisible),
                                   &(list.firstvisible->starttime),
                                   (char *) &(list.firstvisible->spans),
                                   sizeof(struct serv_spans), statwin,
                                   &statcleared, statx);
                        list.baridx = 1;

                        refresh_serv_screen(&list, idx);
                    }
                    break;
                case 12:
                case 'l':
                case 'L':
                    tx_refresh_screen();
                    break;
                case 's':
                case 'S':
                    show_portsort_keywin(&sortwin, &sortpanel);
                    keymode = 1;
                    break;
                case 'q':
                case 'Q':
                case 'x':
                case 'X':
                case 27:
                case 24:
                    exitloop = 1;
                }
            } else if (keymode == 1) {
                del_panel(sortpanel);
                delwin(sortwin);
                sortportents(&list, &idx, ch);
                keymode = 0;
                if (list.barptr != NULL) {
                    set_barptr((char **) &(list.barptr),
                               (char *) list.firstvisible,
                               &(list.firstvisible->starttime),
                               (char *) &(list.firstvisible->spans),
                               sizeof(struct serv_spans), statwin,
                               &statcleared, statx);
                    list.baridx = 1;
                }
                refresh_serv_screen(&list, idx);
                update_panels();
                doupdate();
            }
        }

        if (br > 0) {
            pkt_result = processpacket(buf, &ipacket, &br, &tot_br,
                                       &sport, &dport, &fromaddr,
                                       &linktype, ofilter,
                                       MATCH_OPPOSITE_USECONFIG, iface,
                                       ifname);

            if (pkt_result != PACKET_OK)
                continue;

            if ((((struct iphdr *) ipacket)->protocol == IPPROTO_TCP)
                || (((struct iphdr *) ipacket)->protocol == IPPROTO_UDP)) {
                updateportent(&list, ((struct iphdr *) ipacket)->protocol,
                              ntohs(sport), ntohs(dport),
                              ntohs(((struct iphdr *) ipacket)->tot_len),
                              idx, ports, options->servnames);

                if ((list.barptr == NULL) && (list.head != NULL)) {
                    set_barptr((char **) &(list.barptr),
                               (char *) list.head, &(list.head->starttime),
                               (char *) &(list.head->spans),
                               sizeof(struct serv_spans), statwin,
                               &statcleared, statx);
                    list.baridx = 1;
                }
            }
        }
    }

    if (logging) {
        signal(SIGUSR1, SIG_DFL);
        writeutslog(list.head, time((time_t *) NULL) - starttime,
                    options->actmode, logfile);
        writelog(logging, logfile,
                 "******** TCP/UDP service monitor stopped ********");
        fclose(logfile);
    }
    if (options->servnames)
        endservent();

    if ((options->promisc) && (is_last_instance())) {
        load_promisc_list(&promisc_list);
        srpromisc(0, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, -1);

    del_panel(list.panel);
    delwin(list.win);
    del_panel(list.borderpanel);
    delwin(list.borderwin);
    del_panel(statpanel);
    delwin(statwin);
    unmark_facility(TCPUDPIDFILE, ifname);
    update_panels();
    doupdate();
    destroyportlist(&list);
    pkt_cleanup();
    strcpy(current_logfile, "");
}

void portdlg(unsigned int *port_min, int *port_max, int *aborted, int mode)
{
    WINDOW *bw;
    PANEL *bp;
    WINDOW *win;
    PANEL *panel;

    struct FIELDLIST list;

    bw = newwin(14, 50, (LINES - 14) / 2, (COLS - 50) / 2 - 10);
    bp = new_panel(bw);

    win = newwin(12, 48, (LINES - 14) / 2 + 1, (COLS - 50) / 2 - 9);
    panel = new_panel(win);

    wattrset(bw, DLGBOXATTR);
    tx_box(bw, ACS_VLINE, ACS_HLINE);

    wattrset(win, DLGTEXTATTR);
    tx_colorwin(win);
    tx_stdwinset(win);
    wtimeout(win, -1);

    mvwprintw(win, 1, 1, "Port numbers below 1024 are reserved for");
    mvwprintw(win, 2, 1, "TCP/IP services, and are normally the only");
    mvwprintw(win, 3, 1, "ones monitored by the TCP/UDP statistics");
    mvwprintw(win, 4, 1, "module.  If you wish to monitor a higher-");
    mvwprintw(win, 5, 1, "numbered port or range of ports, enter it");
    mvwprintw(win, 6, 1, "here.  Fill just the first field for a");
    mvwprintw(win, 7, 1, "single port, or both fields for a range.");

    wmove(win, 11, 1);
    tabkeyhelp(win);
    stdkeyhelp(win);

    tx_initfields(&list, 1, 20, (LINES - 14) / 2 + 10, (COLS - 50) / 2 - 8,
                  DLGTEXTATTR, FIELDATTR);
    mvwprintw(list.fieldwin, 0, 6, "to");

    tx_addfield(&list, 5, 0, 0, "");
    tx_addfield(&list, 5, 0, 9, "");

    tx_fillfields(&list, aborted);

    if (!(*aborted)) {
        *port_min = atoi(list.list->buf);
        *port_max = atoi(list.list->nextfield->buf);
    }
    del_panel(bp);
    delwin(bw);
    del_panel(panel);
    delwin(win);
    tx_destroyfields(&list);
}

void saveportlist(struct porttab *table)
{
    struct porttab *ptmp = table;
    int fd;
    int bw;
    int resp;

    fd = open(PORTFILE, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        tx_errbox("Unable to open port list file", ANYKEY_MSG, &resp);
        return;
    }
    while (ptmp != NULL) {
        bw = write(fd, &(ptmp->port_min), sizeof(unsigned int));
        bw = write(fd, &(ptmp->port_max), sizeof(unsigned int));

        if (bw < 0) {
            tx_errbox("Unable to write port/range entry", ANYKEY_MSG,
                      &resp);
            destroyporttab(table);
            close(fd);
            return;
        }
        ptmp = ptmp->next_entry;
    }

    close(fd);
}

int dup_portentry(struct porttab *table, unsigned int min,
                  unsigned int max)
{
    struct porttab *ptmp = table;

    while (ptmp != NULL) {
        if ((ptmp->port_min == min) && (ptmp->port_max == max))
            return 1;

        ptmp = ptmp->next_entry;
    }

    return 0;
}

void addmoreports(struct porttab **table)
{
    unsigned int port_min, port_max;
    int aborted;
    int resp;
    struct porttab *ptmp;

    portdlg(&port_min, &port_max, &aborted, 0);

    if (!aborted) {
        if (dup_portentry(*table, port_min, port_max))
            tx_errbox("Duplicate port/range entry", ANYKEY_MSG, &resp);
        else {
            ptmp = malloc(sizeof(struct porttab));

            ptmp->port_min = port_min;
            ptmp->port_max = port_max;
            ptmp->prev_entry = NULL;
            ptmp->next_entry = *table;

            if (*table != NULL)
                (*table)->prev_entry = ptmp;

            *table = ptmp;
            saveportlist(*table);
        }
    }
    update_panels();
    doupdate();
}

void loadaddports(struct porttab **table)
{
    int fd;
    struct porttab *ptemp;
    struct porttab *tail = NULL;
    int br;
    int resp;

    *table = NULL;

    fd = open(PORTFILE, O_RDONLY);
    if (fd < 0)
        return;

    do {
        ptemp = malloc(sizeof(struct porttab));

        br = read(fd, &(ptemp->port_min), sizeof(unsigned int));
        br = read(fd, &(ptemp->port_max), sizeof(unsigned int));

        if (br < 0) {
            tx_errbox("Error reading port list", ANYKEY_MSG, &resp);
            close(fd);
            destroyporttab(*table);
            return;
        }
        if (br > 0) {
            if (*table == NULL) {
                *table = ptemp;
                ptemp->prev_entry = NULL;
            }
            if (tail != NULL) {
                tail->next_entry = ptemp;
                ptemp->prev_entry = tail;
            }
            tail = ptemp;
            ptemp->next_entry = NULL;
        } else
            free(ptemp);

    } while (br > 0);

    close(fd);
}

void displayportentry(struct porttab *ptmp, WINDOW * win)
{
    wprintw(win, "%u", ptmp->port_min);
    if (ptmp->port_max != 0)
        wprintw(win, " to %u", ptmp->port_max);
}

void displayports(struct porttab **table, WINDOW * win)
{
    struct porttab *ptmp = *table;
    short i = 0;

    do {
        wmove(win, i, 2);
        displayportentry(ptmp, win);

        i++;
        ptmp = ptmp->next_entry;
    } while ((i < 18) && (ptmp != NULL));

    update_panels();
    doupdate();
}

void operate_portselect(struct porttab **table, struct porttab **node,
                        int *aborted)
{
    int ch = 0;
    struct scroll_list list;
    char listtext[20];

    tx_init_listbox(&list, 25, 22, (COLS - 25) / 2, (LINES - 22) / 2,
                    STDATTR, BOXATTR, BARSTDATTR, HIGHATTR);

    tx_set_listbox_title(&list, "Select Port/Range", 1);

    *node = *table;
    while (*node != NULL) {
        snprintf(listtext, 20, "%d to %d", (*node)->port_min,
                 (*node)->port_max);
        tx_add_list_entry(&list, (char *) *node, listtext);
        *node = (*node)->next_entry;
    }

    tx_show_listbox(&list);
    tx_operate_listbox(&list, &ch, aborted);

    if (!(*aborted))
        *node = (struct porttab *) list.textptr->nodeptr;

    tx_close_listbox(&list);
    tx_destroy_list(&list);
}

void selectport(struct porttab **table, struct porttab **node,
                int *aborted)
{
    int resp;

    if (*table == NULL) {
        tx_errbox("No custom ports", ANYKEY_MSG, &resp);
        return;
    }

    operate_portselect(table, node, aborted);
}

void delport(struct porttab **table, struct porttab *ptmp)
{
    if (ptmp != NULL) {
        if (ptmp == *table) {
            *table = (*table)->next_entry;
            if (*table != NULL)
                (*table)->prev_entry = NULL;
        } else {
            ptmp->prev_entry->next_entry = ptmp->next_entry;

            if (ptmp->next_entry != NULL)
                ptmp->next_entry->prev_entry = ptmp->prev_entry;
        }

        free(ptmp);
    }
}

void removeaport(struct porttab **table)
{
    unsigned int aborted;
    struct porttab *ptmp;

    selectport(table, &ptmp, &aborted);

    if (!aborted) {
        delport(table, ptmp);
        saveportlist(*table);
    }
}

void destroyporttab(struct porttab *table)
{
    struct porttab *ptemp = table;
    struct porttab *ctemp = NULL;

    if (ptemp != NULL)
        ctemp = ptemp->next_entry;

    while (ptemp != NULL) {
        free(ptemp);
        ptemp = ctemp;

        if (ctemp != NULL)
            ctemp = ctemp->next_entry;
    }
}
