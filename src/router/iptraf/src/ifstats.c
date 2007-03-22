
/***

ifstats.c	- the interface statistics module
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997-2002

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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <winops.h>
#include <labels.h>
#include <listbox.h>
#include <msgboxes.h>
#include "ifstats.h"
#include "ifaces.h"
#include "isdntab.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "packet.h"
#include "options.h"
#include "log.h"
#include "dirs.h"
#include "deskman.h"
#include "ipcsum.h"
#include "attrs.h"
#include "serv.h"
#include "timer.h"
#include "instances.h"
#include "mode.h"
#include "logvars.h"
#include "promisc.h"
#include "error.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

extern int exitloop;
extern int daemonized;

/* from log.c, applicable only to this module */

void writegstatlog(struct iftab *table, int unit, unsigned long nsecs,
                   FILE * logfile);
void writedstatlog(char *ifname, int unit, float activity, float pps,
                   float peakactivity, float peakpps,
                   float peakactivity_in, float peakpps_in,
                   float peakactivity_out, float peakpps_out,
                   struct iftotals *ts, unsigned long nsecs,
                   FILE * logfile);

/*
 * USR1 log-rotation signal handlers
 */

void rotate_gstat_log()
{
    rotate_flag = 1;
    strcpy(target_logname, GSTATLOG);
    signal(SIGUSR1, rotate_gstat_log);
}

void rotate_dstat_log()
{
    rotate_flag = 1;
    strcpy(target_logname, current_logfile);
    signal(SIGUSR1, rotate_dstat_log);
}


/*
 * Function to check if an interface is already in the interface list.
 * This eliminates duplicate interface entries due to aliases
 */

int ifinlist(struct iflist *list, char *ifname)
{
    struct iflist *ptmp = list;
    int result = 0;

    while ((ptmp != NULL) && (result == 0)) {
        result = (strcmp(ifname, ptmp->ifname) == 0);
        ptmp = ptmp->next_entry;
    }

    return result;
}

/*
 * Initialize the list of interfaces.  This linked list is used in the
 * selection boxes as well as in the general interface statistics screen.
 *
 * This function parses the /proc/net/dev file and grabs the interface names
 * from there.  The SIOGIFFLAGS ioctl() call is used to determine whether the
 * interfaces are active.  Inactive interfaces are omitted from selection
 * lists.
 */

void initiflist(struct iflist **list)
{
    FILE *fd;
    char buf[161];
    char ifname[10];
    struct iflist *itmp = NULL;
    struct iflist *tail = NULL;
    unsigned int index = 0;
    int resp;

    *list = NULL;

    fd = open_procnetdev();
    if (fd == NULL) {
        tx_errbox("Unable to obtain interface list", ANYKEY_MSG, &resp);
        return;
    }

    do {
        strcpy(buf, "");
        get_next_iface(fd, ifname);
        if (strcmp(ifname, "") != 0) {
            if (!(iface_supported(ifname)))
                continue;

            if (ifinlist(*list, ifname))        /* ignore entry if already in */
                continue;       /* interface list */

            /*
             * Check if the interface is actually up running.  This prevents
             * inactive devices in /proc/net/dev from actually appearing in
             * interface lists used by IPTraf.
             */

            if (!iface_up(ifname))
                continue;

            /*
             * At this point, the interface is now sure to be up and running.
             */

            itmp = malloc(sizeof(struct iflist));
            bzero(itmp, sizeof(struct iflist));
            strcpy(itmp->ifname, ifname);
            index++;
            itmp->index = index;

            if (*list == NULL) {
                *list = itmp;
                itmp->prev_entry = NULL;
            } else {
                tail->next_entry = itmp;
                itmp->prev_entry = tail;
            }

            tail = itmp;
            itmp->next_entry = NULL;
        }
    } while (strcmp(ifname, "") != 0);

    fclose(fd);
}

void positionptr(struct iftab *table, struct iflist **ptmp, char *ifname)
{
    struct iflist *plast = NULL;
    int ok = 0;

    *ptmp = table->head;

    while ((*ptmp != NULL) && (!ok)) {
        ok = (strcmp((*ptmp)->ifname, ifname) == 0);

        if (!ok) {
            if ((*ptmp)->next_entry == NULL)
                plast = *ptmp;

            *ptmp = (*ptmp)->next_entry;
        }
    }

    if (*ptmp == NULL) {
        *ptmp = malloc(sizeof(struct iflist));
        bzero(*ptmp, sizeof(struct iflist));
        (*ptmp)->index = plast->index + 1;
        plast->next_entry = *ptmp;
        (*ptmp)->prev_entry = plast;
        (*ptmp)->next_entry = NULL;
        strcpy((*ptmp)->ifname, ifname);

        if ((*ptmp)->index <= LINES - 4)
            table->lastvisible = *ptmp;
    }
}

void destroyiflist(struct iflist *list)
{
    struct iflist *ctmp;
    struct iflist *ptmp;

    if (list != NULL) {
        ptmp = list;
        ctmp = ptmp->next_entry;

        do {
            free(ptmp);
            ptmp = ctmp;
            if (ctmp != NULL)
                ctmp = ctmp->next_entry;
        } while (ptmp != NULL);
    }
}

void no_ifaces_error(void)
{
    write_error
        ("No active interfaces.  Check their status or the /proc filesystem",
         daemonized);
}

void updaterates(struct iftab *table, int unit, time_t starttime,
                 time_t now, unsigned int idx)
{
    struct iflist *ptmp = table->firstvisible;

    wattrset(table->statwin, HIGHATTR);
    do {
        wmove(table->statwin, ptmp->index - idx, 52 * COLS / 80);
        if (unit == KBITS) {
            ptmp->rate =
                ((float) (ptmp->spanbr * 8 / 1000)) /
                ((float) (now - starttime));
            wprintw(table->statwin, "%8.2f kbits/sec", ptmp->rate);
        } else {
            ptmp->rate =
                ((float) (ptmp->spanbr / 1024)) /
                ((float) (now - starttime));
            wprintw(table->statwin, "%8.2f kbytes/sec", ptmp->rate);
        }

        if (ptmp->rate > ptmp->peakrate)
            ptmp->peakrate = ptmp->rate;

        ptmp->spanbr = 0;
        ptmp = ptmp->next_entry;
    } while (ptmp != table->lastvisible->next_entry);
}

void printifentry(struct iflist *ptmp, WINDOW * win, unsigned int idx)
{
    unsigned int target_row;

    if ((ptmp->index < idx) || (ptmp->index > idx + (LINES - 5)))
        return;

    target_row = ptmp->index - idx;

    wattrset(win, STDATTR);
    wmove(win, target_row, 1);
    wprintw(win, "%s", ptmp->ifname);
    wattrset(win, HIGHATTR);
    wmove(win, target_row, 12 * COLS / 80);
    printlargenum(ptmp->total, win);
    wmove(win, target_row, 22 * COLS / 80);
    printlargenum(ptmp->iptotal, win);
    wmove(win, target_row, 32 * COLS / 80);
    printlargenum(ptmp->noniptotal, win);
    wmove(win, target_row, 42 * COLS / 80);
    wprintw(win, "%8lu", ptmp->badtotal);
}

void preparescreen(struct iftab *table)
{
    struct iflist *ptmp = table->head;
    unsigned int i = 1;

    unsigned int winht = LINES - 4;

    table->firstvisible = table->head;

    do {
        printifentry(ptmp, table->statwin, 1);

        if (i <= winht)
            table->lastvisible = ptmp;

        ptmp = ptmp->next_entry;
        i++;
    } while ((ptmp != NULL) && (i <= winht));
}

void labelstats(WINDOW * win)
{
    wmove(win, 0, 1);
    wprintw(win, " Iface ");
    wmove(win, 0, 16 * COLS / 80);
    wprintw(win, " Total ");
    wmove(win, 0, 29 * COLS / 80);
    wprintw(win, " IP ");
    wmove(win, 0, 36 * COLS / 80);
    wprintw(win, " NonIP ");
    wmove(win, 0, 45 * COLS / 80);
    wprintw(win, " BadIP ");
    wmove(win, 0, 55 * COLS / 80);
    wprintw(win, " Activity ");
}

void initiftab(struct iftab *table)
{
    table->borderwin = newwin(LINES - 2, COLS, 1, 0);
    table->borderpanel = new_panel(table->borderwin);

    move(LINES - 1, 1);
    scrollkeyhelp();
    stdexitkeyhelp();
    wattrset(table->borderwin, BOXATTR);
    tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
    labelstats(table->borderwin);
    table->statwin = newwin(LINES - 4, COLS - 2, 2, 1);
    table->statpanel = new_panel(table->statwin);
    tx_stdwinset(table->statwin);
    wtimeout(table->statwin, -1);
    wattrset(table->statwin, STDATTR);
    tx_colorwin(table->statwin);
    wattrset(table->statwin, BOXATTR);
    wmove(table->borderwin, LINES - 3, 32 * COLS / 80);
    wprintw(table->borderwin,
            " Total, IP, NonIP, and BadIP are packet counts ");
}

/*
 * Scrolling routines for the general interface statistics window
 */

void scrollgstatwin(struct iftab *table, int direction, unsigned int *idx)
{
    char buf[255];
    sprintf(buf, "%%%dc", COLS - 2);
    wattrset(table->statwin, STDATTR);
    if (direction == SCROLLUP) {
        if (table->lastvisible->next_entry != NULL) {
            wscrl(table->statwin, 1);
            table->lastvisible = table->lastvisible->next_entry;
            table->firstvisible = table->firstvisible->next_entry;
            (*idx)++;
            wmove(table->statwin, LINES - 5, 0);
            scrollok(table->statwin, 0);
            wprintw(table->statwin, buf, ' ');
            scrollok(table->statwin, 1);
            printifentry(table->lastvisible, table->statwin, *idx);
        }
    } else {
        if (table->firstvisible != table->head) {
            wscrl(table->statwin, -1);
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;
            (*idx)--;
            wmove(table->statwin, 0, 0);
            wprintw(table->statwin, buf, ' ');
            printifentry(table->firstvisible, table->statwin, *idx);
        }
    }
}

void pagegstatwin(struct iftab *table, int direction, int *idx)
{
    int i = 1;

    if (direction == SCROLLUP) {
        while ((i <= LINES - 5)
               && (table->lastvisible->next_entry != NULL)) {
            i++;
            scrollgstatwin(table, direction, idx);
        }
    } else {
        while ((i <= LINES - 5) && (table->firstvisible != table->head)) {
            i++;
            scrollgstatwin(table, direction, idx);
        }
    }
}


/*
 * The general interface statistics function
 */

void ifstats(const struct OPTIONS *options, struct filterstate *ofilter,
             int facilitytime)
{
    int logging = options->logging;
    struct iftab table;

    char buf[MAX_PACKET_SIZE];
    char *packet;
    int pkt_result = 0;

    struct sockaddr_ll fromaddr;
    unsigned short linktype;

    struct iflist *ptmp = NULL;

    unsigned int idx = 1;

    int fd;
    FILE *logfile = NULL;

    int br;
    char ifname[10];

    int ch;

    struct timeval tv;
    unsigned long starttime = 0;
    unsigned long statbegin = 0;
    unsigned long now = 0;
    unsigned long long unow = 0;
    unsigned long startlog = 0;
    unsigned long updtime = 0;
    unsigned long long updtime_usec = 0;

    struct promisc_states *promisc_list;

    if (!facility_active(GSTATIDFILE, ""))
        mark_facility(GSTATIDFILE, "general interface statistics", "");
    else {
        write_error
            ("General interface stats already active in another process",
             daemonized);
        return;
    }

    initiflist(&(table.head));
    if (table.head == NULL) {
        no_ifaces_error();
        unmark_facility(GSTATIDFILE, "");
        return;
    }

    initiftab(&table);
    open_socket(&fd);

    if (fd < 0) {
        unmark_facility(GSTATIDFILE, "");
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

    if (logging) {
        if (strcmp(current_logfile, "") == 0) {
            strcpy(current_logfile, GSTATLOG);

            if (!daemonized)
                input_logfile(current_logfile, &logging);
        }
    }

    if (logging) {
        opentlog(&logfile, GSTATLOG);

        if (logfile == NULL)
            logging = 0;
    }
    if (logging)
        signal(SIGUSR1, rotate_gstat_log);

    rotate_flag = 0;
    writelog(logging, logfile,
             "******** General interface statistics started ********");

    if (table.head != NULL) {
        preparescreen(&table);

        update_panels();
        doupdate();

        isdnfd = -1;
        exitloop = 0;
        gettimeofday(&tv, NULL);
        starttime = startlog = statbegin = tv.tv_sec;

        while (!exitloop) {
            gettimeofday(&tv, NULL);
            now = tv.tv_sec;
            unow = tv.tv_sec * 1e+6 + tv.tv_usec;

            if ((now - starttime) >= 5) {
                updaterates(&table, options->actmode, starttime, now, idx);
                printelapsedtime(statbegin, now, LINES - 3, 1,
                                 table.borderwin);
                starttime = now;
            }
            if (((now - startlog) >= options->logspan) && (logging)) {
                writegstatlog(&table, options->actmode,
                              time((time_t *) NULL) - statbegin, logfile);
                startlog = now;
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
                && (((now - statbegin) / 60) >= facilitytime))
                exitloop = 1;

            getpacket(fd, buf, &fromaddr, &ch, &br, ifname, table.statwin);

            if (ch != ERR) {
                switch (ch) {
                case KEY_UP:
                    scrollgstatwin(&table, SCROLLDOWN, &idx);
                    break;
                case KEY_DOWN:
                    scrollgstatwin(&table, SCROLLUP, &idx);
                    break;
                case KEY_PPAGE:
                case '-':
                    pagegstatwin(&table, SCROLLDOWN, &idx);
                    break;
                case KEY_NPAGE:
                case ' ':
                    pagegstatwin(&table, SCROLLUP, &idx);
                    break;
                case 12:
                case 'l':
                case 'L':
                    tx_refresh_screen();
                    break;

                case 'Q':
                case 'q':
                case 'X':
                case 'x':
                case 27:
                case 24:
                    exitloop = 1;
                    break;
                }
            }
            if (br > 0) {
                pkt_result =
                    processpacket(buf, &packet, &br, NULL, NULL, NULL,
                                  &fromaddr, &linktype, ofilter,
                                  MATCH_OPPOSITE_USECONFIG, ifname, NULL);

                if (pkt_result != PACKET_OK
                    && pkt_result != MORE_FRAGMENTS)
                    continue;

                positionptr(&table, &ptmp, ifname);

                ptmp->total++;

                ptmp->spanbr += br;
                ptmp->br += br;

                if (fromaddr.sll_protocol == ETH_P_IP) {
                    ptmp->iptotal++;

                    if (pkt_result == CHECKSUM_ERROR) {
                        (ptmp->badtotal)++;
                        continue;
                    }
                } else {
                    (ptmp->noniptotal)++;
                }
                printifentry(ptmp, table.statwin, idx);
            }

        }

        close(fd);
    }

    if ((options->promisc) && (is_last_instance())) {
        load_promisc_list(&promisc_list);
        srpromisc(0, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, -1);

    del_panel(table.statpanel);
    delwin(table.statwin);
    del_panel(table.borderpanel);
    delwin(table.borderwin);
    update_panels();
    doupdate();

    if (logging) {
        signal(SIGUSR1, SIG_DFL);
        writegstatlog(&table, options->actmode,
                      time((time_t *) NULL) - statbegin, logfile);
        writelog(logging, logfile,
                 "******** General interface statistics stopped ********");
        fclose(logfile);
    }
    destroyiflist(table.head);
    pkt_cleanup();
    unmark_facility(GSTATIDFILE, "");
    strcpy(current_logfile, "");
}


void printdetlabels(WINDOW * win, struct iftotals *totals)
{
    wattrset(win, BOXATTR);
    mvwprintw(win, 2, 14,
              "  Total      Total    Incoming   Incoming    Outgoing   Outgoing");
    mvwprintw(win, 3, 14,
              "Packets      Bytes     Packets      Bytes     Packets      Bytes");
    wattrset(win, STDATTR);
    mvwprintw(win, 4, 2, "Total:");
    mvwprintw(win, 5, 2, "IP:");
    mvwprintw(win, 6, 2, "TCP:");
    mvwprintw(win, 7, 2, "UDP:");
    mvwprintw(win, 8, 2, "ICMP:");
    mvwprintw(win, 9, 2, "Other IP:");
    mvwprintw(win, 10, 2, "Non-IP:");
    mvwprintw(win, 13, 2, "Total rates:");
    mvwprintw(win, 16, 2, "Incoming rates:");
    mvwprintw(win, 19, 2, "Outgoing rates:");

    mvwprintw(win, 13, 45, "Broadcast packets:");
    mvwprintw(win, 14, 45, "Broadcast bytes:");
    mvwprintw(win, 18, 45, "IP checksum errors:");

    update_panels();
    doupdate();
}

void printstatrow(WINDOW * win, int row,
                  unsigned long long total, unsigned long long btotal,
                  unsigned long long total_in,
                  unsigned long long btotal_in,
                  unsigned long long total_out,
                  unsigned long long btotal_out)
{
    wmove(win, row, 12);
    printlargenum(total, win);
    wmove(win, row, 23);
    printlargenum(btotal, win);
    wmove(win, row, 35);
    printlargenum(total_in, win);
    wmove(win, row, 46);
    printlargenum(btotal_in, win);
    wmove(win, row, 58);
    printlargenum(total_out, win);
    wmove(win, row, 69);
    printlargenum(btotal_out, win);
}

void printdetails(struct iftotals *totals, WINDOW * win)
{
    wattrset(win, HIGHATTR);

    /* Print totals on the IP protocols */

    printstatrow(win, 4, totals->total, totals->bytestotal,
                 totals->total_in, totals->bytestotal_in,
                 totals->total_out, totals->bytestotal_out);

    printstatrow(win, 5, totals->iptotal, totals->ipbtotal,
                 totals->iptotal_in, totals->ipbtotal_in,
                 totals->iptotal_out, totals->ipbtotal_out);

    printstatrow(win, 6, totals->tcptotal, totals->tcpbtotal,
                 totals->tcptotal_in, totals->tcpbtotal_in,
                 totals->tcptotal_out, totals->tcpbtotal_out);

    printstatrow(win, 7, totals->udptotal, totals->udpbtotal,
                 totals->udptotal_in, totals->udpbtotal_in,
                 totals->udptotal_out, totals->udpbtotal_out);

    printstatrow(win, 8, totals->icmptotal, totals->icmpbtotal,
                 totals->icmptotal_in, totals->icmpbtotal_in,
                 totals->icmptotal_out, totals->icmpbtotal_out);

    printstatrow(win, 9, totals->othtotal, totals->othbtotal,
                 totals->othtotal_in, totals->othbtotal_in,
                 totals->othtotal_out, totals->othbtotal_out);

    /* Print non-IP totals */

    printstatrow(win, 10, totals->noniptotal, totals->nonipbtotal,
                 totals->noniptotal_in, totals->nonipbtotal_in,
                 totals->noniptotal_out, totals->nonipbtotal_out);

    /* Broadcast totals */

    wmove(win, 13, 67);
    printlargenum(totals->bcast, win);
    wmove(win, 14, 67);
    printlargenum(totals->bcastbytes, win);

    /* Bad packet count */

    mvwprintw(win, 18, 68, "%8lu", totals->badtotal);
}


/*
 * The detailed interface statistics function
 */

void detstats(char *iface, const struct OPTIONS *options, int facilitytime,
              struct filterstate *ofilter)
{
    int logging = options->logging;

    WINDOW *statwin;
    PANEL *statpanel;

    char buf[MAX_PACKET_SIZE];
    char *packet;
    struct iphdr *ipacket = NULL;
    char *tpacket;
    unsigned int iphlen;

    char ifname[10];
    struct sockaddr_ll fromaddr;
    unsigned short linktype;

    int fd;
    int br;
    int framelen = 0;
    int pkt_result = 0;

    FILE *logfile = NULL;

    unsigned int iplen = 0;

    struct iftotals totals;

    int ch;

    struct timeval tv;
    unsigned long updtime = 0;
    unsigned long long updtime_usec = 0;
    unsigned long starttime, now;
    unsigned long statbegin, startlog;
    unsigned long rate_interval;
    unsigned long long unow;

    float spanbr = 0;
    float spanpkt = 0;
    float spanbr_in = 0;
    float spanbr_out = 0;
    float spanpkt_in = 0;
    float spanpkt_out = 0;

    float activity = 0;
    float activity_in = 0;
    float activity_out = 0;
    float peakactivity = 0;
    float peakactivity_in = 0;
    float peakactivity_out = 0;

    float pps = 0;
    float peakpps = 0;
    float pps_in = 0;
    float pps_out = 0;
    float peakpps_in = 0;
    float peakpps_out = 0;

    char unitstring[7];

    struct promisc_states *promisc_list;
    char err_msg[80];

#ifdef ACTIVATE_GRAPHING
    FILE *graphing_fd;
    unsigned long last_graph_time;
    unsigned long graph_interval;

    float graph_span_pkts = 0;
    float graph_span_bytes = 0;
    float graph_span_pkts_in = 0;
    float graph_span_bytes_in = 0;
    float graph_span_pkts_out = 0;
    float graph_span_bytes_out = 0;
#endif

    /*
     * Mark this facility
     */

    if (!facility_active(DSTATIDFILE, iface))
        mark_facility(DSTATIDFILE, "detailed interface statistics", iface);
    else {
        snprintf(err_msg, 80,
                 "Detailed interface stats already monitoring %s", iface);
        write_error(err_msg, daemonized);
        return;
    }

    open_socket(&fd);

    if (fd < 0) {
        unmark_facility(DSTATIDFILE, iface);
        return;
    }
    if (!iface_supported(iface)) {
        err_iface_unsupported();
        unmark_facility(DSTATIDFILE, iface);
        return;
    }
    if (!iface_up(iface)) {
        err_iface_down();
        unmark_facility(DSTATIDFILE, iface);
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

    move(LINES - 1, 1);
    stdexitkeyhelp();
    statwin = newwin(LINES - 2, COLS, 1, 0);
    statpanel = new_panel(statwin);
    tx_stdwinset(statwin);
    wtimeout(statwin, -1);
    wattrset(statwin, BOXATTR);
    tx_colorwin(statwin);
    tx_box(statwin, ACS_VLINE, ACS_HLINE);
    wmove(statwin, 0, 1);
    wprintw(statwin, " Statistics for %s ", iface);
    wattrset(statwin, STDATTR);
    update_panels();
    doupdate();

    bzero(&totals, sizeof(struct iftotals));

    if (logging) {
        if (strcmp(current_logfile, "") == 0) {
            snprintf(current_logfile, 64, "%s-%s.log", DSTATLOG, iface);

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
        signal(SIGUSR1, rotate_dstat_log);

    rotate_flag = 0;
    writelog(logging, logfile,
             "******** Detailed interface statistics started ********");

    printdetlabels(statwin, &totals);
    printdetails(&totals, statwin);
    update_panels();
    doupdate();

    spanbr = 0;

    gettimeofday(&tv, NULL);
    starttime = startlog = statbegin = tv.tv_sec;

#ifdef ACTIVATE_GRAPHING
    last_graph_time = starttime;
#endif

    leaveok(statwin, TRUE);

    isdnfd = -1;
    exitloop = 0;
    dispmode(options->actmode, unitstring);

    /*
     * Data-gathering loop
     */

    while (!exitloop) {
        gettimeofday(&tv, NULL);
        now = tv.tv_sec;
        unow = tv.tv_sec * 1e+6 + tv.tv_usec;

        rate_interval = now - starttime;

        if (rate_interval >= 5) {
            wattrset(statwin, BOXATTR);
            printelapsedtime(statbegin, now, LINES - 3, 1, statwin);
            if (options->actmode == KBITS) {
                activity =
                    (float) (spanbr * 8 / 1000) / (float) rate_interval;
                activity_in =
                    (float) (spanbr_in * 8 / 1000) / (float) rate_interval;
                activity_out =
                    (float) (spanbr_out * 8 / 1000) /
                    (float) rate_interval;
            } else {
                activity = (float) (spanbr / 1024) / (float) rate_interval;
                activity_in =
                    (float) (spanbr_in / 1024) / (float) rate_interval;
                activity_out =
                    (float) (spanbr_out / 1024) / (float) rate_interval;
            }

            pps = (float) (spanpkt) / (float) (now - starttime);
            pps_in = (float) (spanpkt_in) / (float) (now - starttime);
            pps_out = (float) (spanpkt_out) / (float) (now - starttime);

            spanbr = spanbr_in = spanbr_out = 0;
            spanpkt = spanpkt_in = spanpkt_out = 0;
            starttime = now;

            wattrset(statwin, HIGHATTR);
            mvwprintw(statwin, 13, 19, "%8.1f %s/sec", activity,
                      unitstring);
            mvwprintw(statwin, 14, 19, "%8.1f packets/sec", pps);
            mvwprintw(statwin, 16, 19, "%8.1f %s/sec", activity_in,
                      unitstring);
            mvwprintw(statwin, 17, 19, "%8.1f packets/sec", pps_in);
            mvwprintw(statwin, 19, 19, "%8.1f %s/sec", activity_out,
                      unitstring);
            mvwprintw(statwin, 20, 19, "%8.1f packets/sec", pps_out);

            if (activity > peakactivity)
                peakactivity = activity;

            if (activity_in > peakactivity_in)
                peakactivity_in = activity_in;

            if (activity_out > peakactivity_out)
                peakactivity_out = activity_out;

            if (pps > peakpps)
                peakpps = pps;

            if (pps_in > peakpps_in)
                peakpps_in = pps_in;

            if (pps_out > peakpps_out)
                peakpps_out = pps_out;
        }
        if ((now - startlog) >= options->logspan && logging) {
            writedstatlog(iface, options->actmode, activity, pps,
                          peakactivity, peakpps,
                          peakactivity_in, peakpps_in,
                          peakactivity_out, peakpps_out, &totals,
                          time((time_t *) NULL) - statbegin, logfile);

            startlog = now;
        }
#ifdef ACTIVATE_GRAPHING
        graph_interval = now - last_graph_time;
        if (daemonized && graph_interval >= 60
            && graphing_logfile[0] != '\0') {
            graphing_fd = fopen(graphing_logfile, "w");
            if (graphing_fd == NULL) {
                write_error
                    ("Unable to open raw logfile, raw logging diabled", 1);
                graphing_logfile[0] = '\0';
            } else {
                fprintf(graphing_fd, "%lu %8.2f %8.2f %8.2f %8.2f\n",
                        now,
                        (float) graph_span_pkts_out /
                        (float) graph_interval,
                        (float) (graph_span_bytes_out * 8 / 1000) /
                        (float) graph_interval,
                        (float) graph_span_pkts_in /
                        (float) graph_interval,
                        (float) (graph_span_bytes_in * 8 / 1000) /
                        (float) graph_interval);

                fclose(graphing_fd);
                last_graph_time = now;
                graph_span_pkts_out = 0;
                graph_span_bytes_out = 0;
                graph_span_pkts_in = 0;
                graph_span_bytes_in = 0;
            }
        }
#endif

        if (((options->updrate == 0)
             && (unow - updtime_usec >= DEFAULT_UPDATE_DELAY))
            || ((options->updrate != 0)
                && (now - updtime >= options->updrate))) {
            printdetails(&totals, statwin);
            update_panels();
            doupdate();
            updtime_usec = unow;
            updtime = now;
        }
        check_rotate_flag(&logfile, logging);

        if ((facilitytime != 0)
            && (((now - statbegin) / 60) >= facilitytime))
            exitloop = 1;

        getpacket(fd, buf, &fromaddr, &ch, &br, ifname, statwin);

        if (ch != ERR) {
            switch (ch) {
            case 12:
            case 'l':
            case 'L':
                tx_refresh_screen();
                break;

            case 'Q':
            case 'q':
            case 'X':
            case 'x':
            case 24:
            case 27:
                exitloop = 1;
                break;
            }
        }
        if (br > 0) {
            framelen = br;
            pkt_result = processpacket(buf, &packet, &br, NULL,
                                       NULL, NULL, &fromaddr,
                                       &linktype, ofilter,
                                       MATCH_OPPOSITE_USECONFIG, ifname,
                                       iface);

            if (pkt_result != PACKET_OK && pkt_result != MORE_FRAGMENTS)
                continue;

            totals.total++;
            totals.bytestotal += framelen;

            if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                totals.total_out++;
                totals.bytestotal_out += framelen;
                spanbr_out += framelen;
                spanpkt_out++;
            } else {
                totals.total_in++;
                totals.bytestotal_in += framelen;
                spanbr_in += framelen;
                spanpkt_in++;
            }

            if (fromaddr.sll_pkttype == PACKET_BROADCAST) {
                totals.bcast++;
                totals.bcastbytes += framelen;
            }

            spanbr += framelen;
            spanpkt++;

            if (fromaddr.sll_protocol == ETH_P_IP) {
                if (pkt_result == CHECKSUM_ERROR) {
                    totals.badtotal++;
                    continue;
                }

                ipacket = (struct iphdr *) packet;
                iphlen = ipacket->ihl * 4;
                tpacket = packet + iphlen;
                iplen = ntohs(ipacket->tot_len);

                totals.iptotal++;
                totals.ipbtotal += iplen;

#ifdef ACTIVATE_GRAPHING
                graph_span_pkts++;
                graph_span_bytes += framelen;
#endif

                if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                    totals.iptotal_out++;
                    totals.ipbtotal_out += iplen;
#ifdef ACTIVATE_GRAPHING
                    graph_span_pkts_out++;
                    graph_span_bytes_out += framelen;
#endif
                } else {
                    totals.iptotal_in++;
                    totals.ipbtotal_in += iplen;
#ifdef ACTIVATE_GRAPHING
                    graph_span_pkts_in++;
                    graph_span_bytes_in += framelen;
#endif
                }

                switch (ipacket->protocol) {
                case IPPROTO_TCP:
                    totals.tcptotal++;
                    totals.tcpbtotal += iplen;

                    if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                        totals.tcptotal_out++;
                        totals.tcpbtotal_out += iplen;
                    } else {
                        totals.tcptotal_in++;
                        totals.tcpbtotal_in += iplen;
                    }
                    break;
                case IPPROTO_UDP:
                    totals.udptotal++;
                    totals.udpbtotal += iplen;

                    if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                        totals.udptotal_out++;
                        totals.udpbtotal_out += iplen;
                    } else {
                        totals.udptotal_in++;
                        totals.udpbtotal_in += iplen;
                    }
                    break;
                case IPPROTO_ICMP:
                    totals.icmptotal++;
                    totals.icmpbtotal += iplen;

                    if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                        totals.icmptotal_out++;
                        totals.icmpbtotal_out += iplen;
                    } else {
                        totals.icmptotal_in++;
                        totals.icmpbtotal_in += iplen;
                    }
                    break;
                default:
                    totals.othtotal++;
                    totals.othbtotal += iplen;

                    if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                        totals.othtotal_out++;
                        totals.othbtotal_out += iplen;
                    } else {
                        totals.othtotal_in++;
                        totals.othbtotal_in += iplen;
                    }
                    break;
                }
            } else {
                totals.noniptotal++;
                totals.nonipbtotal += br;

                if (fromaddr.sll_pkttype == PACKET_OUTGOING) {
                    totals.noniptotal_out++;
                    totals.nonipbtotal_out += br;
                } else {
                    totals.noniptotal_in++;
                    totals.nonipbtotal_in += br;
                }
            }
        }
    }

    close(fd);

    if ((options->promisc) && (is_last_instance())) {
        load_promisc_list(&promisc_list);
        srpromisc(0, promisc_list);
        destroy_promisc_list(&promisc_list);
    }

    adjust_instance_count(PROCCOUNTFILE, -1);

    if (logging) {
        signal(SIGUSR1, SIG_DFL);
        writedstatlog(iface, options->actmode, activity, pps,
                      peakactivity, peakpps,
                      peakactivity_in, peakpps_in,
                      peakactivity_out, peakpps_out, &totals,
                      time((time_t *) NULL) - statbegin, logfile);
        writelog(logging, logfile,
                 "******** Detailed interface statistics stopped ********");
        fclose(logfile);
    }

    del_panel(statpanel);
    delwin(statwin);
    unmark_facility(DSTATIDFILE, iface);
    strcpy(current_logfile, "");
    pkt_cleanup();
    update_panels();
    doupdate();
}

void selectiface(char *ifname, int withall, int *aborted)
{
    int ch;

    struct iflist *list;
    struct iflist *ptmp;

    struct scroll_list scrolllist;

    initiflist(&list);

    if (list == NULL) {
        no_ifaces_error();
        *aborted = 1;
        return;
    }

    if ((withall) && (list != NULL)) {
        ptmp = malloc(sizeof(struct iflist));
        strcpy(ptmp->ifname, "All interfaces");

        ptmp->prev_entry = NULL;
        list->prev_entry = ptmp;
        ptmp->next_entry = list;
        list = ptmp;
    }
    tx_listkeyhelp(STDATTR, HIGHATTR);

    ptmp = list;

    tx_init_listbox(&scrolllist, 24, 14, (COLS - 24) / 2 - 9,
                    (LINES - 14) / 2, STDATTR, BOXATTR, BARSTDATTR,
                    HIGHATTR);

    tx_set_listbox_title(&scrolllist, "Select Interface", 1);

    while (ptmp != NULL) {
        tx_add_list_entry(&scrolllist, (char *) ptmp, ptmp->ifname);
        ptmp = ptmp->next_entry;
    }

    tx_show_listbox(&scrolllist);
    tx_operate_listbox(&scrolllist, &ch, aborted);
    tx_close_listbox(&scrolllist);

    if (!(*aborted) && (list != NULL)) {
        ptmp = (struct iflist *) scrolllist.textptr->nodeptr;
        if ((withall) && (ptmp->prev_entry == NULL))    /* All Interfaces */
            strcpy(ifname, "");
        else
            strcpy(ifname, ptmp->ifname);
    }

    tx_destroy_list(&scrolllist);
    destroyiflist(list);
    update_panels();
    doupdate();
}
