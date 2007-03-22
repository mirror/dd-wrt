/***

itrafmon.c - the IP traffic monitor module
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

#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <winops.h>
#include <labels.h>
#include "options.h"
#include "tcptable.h"
#include "othptab.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "isdntab.h"
#include "packet.h"
#include "ifaces.h"
#include "promisc.h"
#include "deskman.h"
#include "error.h"
#include "attrs.h"
#include "log.h"
#include "revname.h"
#include "rvnamed.h"
#include "dirs.h"
#include "timer.h"
#include "ipfrag.h"
#include "instances.h"
#include "logvars.h"
#include "bar.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

extern int exitloop;
extern int daemonized;

void writetcplog(int logging, FILE * fd, struct tcptableent *entry,
                 unsigned int pktlen, int mac, char *message);
void write_tcp_unclosed(int logging, FILE * fd, struct tcptable *table);

void rotate_ipmon_log()
{
    rotate_flag = 1;
    strcpy(target_logname, current_logfile);
    signal(SIGUSR1, rotate_ipmon_log);
}

/* Hot key indicators for the bottom line */

void ipmonhelp()
{
    move(LINES - 1, 1);
    tx_printkeyhelp("Up/Dn/PgUp/PgDn", "-scroll  ", stdscr, HIGHATTR,
                    STATUSBARATTR);
    move(LINES - 1, 43);
    tx_printkeyhelp("W", "-chg actv win  ", stdscr, HIGHATTR,
                    STATUSBARATTR);
    tx_printkeyhelp("S", "-sort TCP  ", stdscr, HIGHATTR, STATUSBARATTR);
    stdexitkeyhelp();
};

void uniq_help(int what)
{
    move(LINES - 1, 25);
    if (!what)
        tx_printkeyhelp("M", "-more TCP info   ", stdscr, HIGHATTR,
                        STATUSBARATTR);
    else
        tx_printkeyhelp("Lft/Rt", "-vtcl scrl  ", stdscr, HIGHATTR,
                        STATUSBARATTR);
}

/* Mark general packet count indicators */

void prepare_statwin(WINDOW * win)
{
    wattrset(win, IPSTATLABELATTR);
    wmove(win, 0, 1);
    wprintw(win, "Pkts captured (all interfaces):");
    mvwaddch(win, 0, 45 * COLS / 80, ACS_VLINE);
}

void markactive(int curwin, WINDOW * tw, WINDOW * ow)
{
    WINDOW *win1;
    WINDOW *win2;
    int x1, y1, x2, y2;

    if (!curwin) {
        win1 = tw;
        win2 = ow;
    } else {
        win1 = ow;
        win2 = tw;
    }

    getmaxyx(win1, y1, x1);
    getmaxyx(win2, y2, x2);

    wmove(win1, --y1, COLS - 10);
    wattrset(win1, ACTIVEATTR);
    wprintw(win1, " Active ");
    wattrset(win1, BOXATTR);
    wmove(win2, --y2, COLS - 10);
    whline(win2, ACS_HLINE, 8);
}

void show_stats(WINDOW * win, unsigned long long total)
{
    wattrset(win, IPSTATATTR);
    wmove(win, 0, 35 * COLS / 80);
    printlargenum(total, win);
}


/* 
 * Scrolling and paging routines for the upper (TCP) window
 */

void scrollupperwin(struct tcptable *table, int direction,
                    unsigned long *idx, int mode)
{
    char sp_buf[10];
    sprintf(sp_buf, "%%%dc", COLS - 2);
    wattrset(table->tcpscreen, STDATTR);
    if (direction == SCROLLUP) {
        if (table->lastvisible != table->tail) {
            wscrl(table->tcpscreen, 1);
            table->lastvisible = table->lastvisible->next_entry;
            table->firstvisible = table->firstvisible->next_entry;
            (*idx)++;
            wmove(table->tcpscreen, table->imaxy - 1, 0);
            scrollok(table->tcpscreen, 0);
            wprintw(table->tcpscreen, sp_buf, ' ');
            scrollok(table->tcpscreen, 1);
            printentry(table, table->lastvisible, *idx, mode);
        }
    } else {
        if (table->firstvisible != table->head) {
            wscrl(table->tcpscreen, -1);
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;
            (*idx)--;
            wmove(table->tcpscreen, 0, 0);
            wprintw(table->tcpscreen, sp_buf, ' ');
            printentry(table, table->firstvisible, *idx, mode);
        }
    }
}

void pageupperwin(struct tcptable *table, int direction,
                  unsigned long *idx, int mode)
{
    int i = 1;

    wattrset(table->tcpscreen, STDATTR);
    if (direction == SCROLLUP) {
        while ((i <= table->imaxy - 3)
               && (table->lastvisible != table->tail)) {
            i++;
            table->firstvisible = table->firstvisible->next_entry;
            table->lastvisible = table->lastvisible->next_entry;
            (*idx)++;
        }
    } else {
        while ((i <= table->imaxy - 3)
               && (table->firstvisible != table->head)) {
            i++;
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;
            (*idx)--;
        }
    }
}

/*
 * Scrolling and paging routines for the lower (non-TCP) window.
 */

void scrolllowerwin(struct othptable *table, int direction)
{
    if (direction == SCROLLUP) {
        if (table->lastvisible != table->tail) {
            wscrl(table->othpwin, 1);
            table->lastvisible = table->lastvisible->next_entry;
            table->firstvisible = table->firstvisible->next_entry;

            if (table->htstat == HIND) {        /* Head indicator on? */
                wmove(table->borderwin, table->obmaxy - 1, 1);
                whline(table->borderwin, ACS_HLINE, 8);
                table->htstat = NOHTIND;
            }
            printothpentry(table, table->lastvisible, table->oimaxy - 1,
                           0, (FILE *) NULL);
        }
    } else {
        if (table->firstvisible != table->head) {
            wscrl(table->othpwin, -1);
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;

            if (table->htstat == TIND) {        /* Tail indicator on? */
                wmove(table->borderwin, table->obmaxy - 1, 1);
                whline(table->borderwin, ACS_HLINE, 8);
                table->htstat = NOHTIND;
            }
            printothpentry(table, table->firstvisible, 0,
                           0, (FILE *) NULL);
        }
    }
}

void pagelowerwin(struct othptable *table, int direction)
{
    int i = 1;

    if (direction == SCROLLUP) {
        while ((i <= table->oimaxy - 2)
               && (table->lastvisible != table->tail)) {
            i++;
            table->firstvisible = table->firstvisible->next_entry;
            table->lastvisible = table->lastvisible->next_entry;

            if (table->htstat == HIND) {        /* Head indicator on? */
                wmove(table->borderwin, table->obmaxy - 1, 1);
                whline(table->borderwin, ACS_HLINE, 8);
                table->htstat = NOHTIND;
            }
        }
    } else {
        while ((i <= table->oimaxy - 2)
               && (table->firstvisible != table->head)) {
            i++;
            table->firstvisible = table->firstvisible->prev_entry;
            table->lastvisible = table->lastvisible->prev_entry;

            if (table->htstat == TIND) {        /* Tail indicator on? */
                wmove(table->borderwin, table->obmaxy - 1, 1);
                whline(table->borderwin, ACS_HLINE, 8);
                table->htstat = NOHTIND;
            }
        }
    }
}

/*
 * Pop up sorting key window
 */

void show_tcpsort_win(WINDOW ** win, PANEL ** panel)
{
    *win = newwin(9, 35, (LINES - 8) / 2, COLS - 40);
    *panel = new_panel(*win);

    wattrset(*win, DLGBOXATTR);
    tx_colorwin(*win);
    tx_box(*win, ACS_VLINE, ACS_HLINE);
    wattrset(*win, DLGTEXTATTR);
    mvwprintw(*win, 2, 2, "Select sort criterion");
    wmove(*win, 4, 2);
    tx_printkeyhelp("P", " - sort by packet count", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 5, 2);
    tx_printkeyhelp("B", " - sort by byte count", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    wmove(*win, 6, 2);
    tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
                    DLGTEXTATTR);
    update_panels();
    doupdate();
}

/*
 * Routine to swap two TCP entries.  p1 and p2 are pointers to TCP entries,
 * but p1 must be ahead of p2.  It's a linked list thing.
 */
void swap_tcp_entries(struct tcptable *table,
                      struct tcptableent *p1, struct tcptableent *p2)
{
    struct tcptableent *p2nextsaved;
    struct tcptableent *p1prevsaved;
    unsigned int tmp;

    if (p1 == p2)
        return;

    tmp = p1->index;
    p1->index = p2->index;
    p2->index = tmp;

    p1->next_entry->index = p1->index + 1;
    p2->next_entry->index = p2->index + 1;

    if (p1->prev_entry != NULL)
        p1->prev_entry->next_entry = p2;
    else
        table->head = p2;

    if (p2->next_entry->next_entry != NULL)
        p2->next_entry->next_entry->prev_entry = p1->next_entry;
    else
        table->tail = p1->next_entry;

    p2nextsaved = p2->next_entry->next_entry;
    p1prevsaved = p1->prev_entry;

    if (p1->next_entry->next_entry == p2) {     /* swapping adjacent entries */
        p2->next_entry->next_entry = p1;
        p1->prev_entry = p2->next_entry;
    } else {
        p2->next_entry->next_entry = p1->next_entry->next_entry;
        p1->prev_entry = p2->prev_entry;
        p2->prev_entry->next_entry = p1;
        p1->next_entry->next_entry->prev_entry = p2->next_entry;
    }

    p2->prev_entry = p1prevsaved;
    p1->next_entry->next_entry = p2nextsaved;
}

unsigned long long qt_getkey(struct tcptableent *entry, int ch)
{
    if (ch == 'B')
        return (max(entry->bcount, entry->oth_connection->bcount));

    return (max(entry->pcount, entry->oth_connection->pcount));
}

struct tcptableent *qt_partition(struct tcptable *table,
                                 struct tcptableent **low,
                                 struct tcptableent **high,
                                 int ch, struct OPTIONS *opts,
                                 int logging, FILE * logfile, int *nomem)
{
    struct tcptableent *pivot = *low;

    struct tcptableent *left = *low;
    struct tcptableent *right = *high;
    struct tcptableent *ptmp;

    unsigned long long pivot_value;

    time_t now;

    pivot_value = qt_getkey(pivot, ch);

    now = time(NULL);

    while (left->index < right->index) {
        while ((qt_getkey(left, ch) >= pivot_value)
               && (left->next_entry->next_entry != NULL)) {

            /*
             * Might as well check out timed out entries here too.
             */
            if ((opts->timeout > 0)
                && ((now - left->lastupdate) / 60 > opts->timeout)
                && (!(left->inclosed))) {
                left->timedout = left->oth_connection->timedout = 1;
                addtoclosedlist(table, left, nomem);

                if (logging)
                    write_timeout_log(logging, logfile, left, opts);
            }

            left = left->next_entry->next_entry;
        }

        while (qt_getkey(right, ch) < pivot_value) {
            /*
             * Might as well check out timed out entries here too.
             */
            if ((opts->timeout > 0)
                && ((now - right->lastupdate) / 60 > opts->timeout)
                && (!(right->inclosed))) {
                right->timedout = right->oth_connection->timedout = 1;
                addtoclosedlist(table, right, nomem);

                if (logging)
                    write_timeout_log(logging, logfile, right, opts);
            }
            right = right->prev_entry->prev_entry;
        }

        if (left->index < right->index) {
            swap_tcp_entries(table, left, right);

            if (*low == left)
                *low = right;

            if (*high == right)
                *high = left;

            ptmp = left;
            left = right;
            right = ptmp;
        }
    }
    swap_tcp_entries(table, pivot, right);

    if (*low == pivot)
        *low = right;

    if (*high == right)
        *high = pivot;

    return pivot;
}

/*
 * Quicksort the TCP entries.
 */
void quicksort_tcp_entries(struct tcptable *table, struct tcptableent *low,
                           struct tcptableent *high, int ch,
                           struct OPTIONS *opts,
                           int logging, FILE * logfile, int *nomem)
{
    struct tcptableent *pivot;

    if ((high == NULL) || (low == NULL))
        return;

    if (high->index > low->index) {
        pivot = qt_partition(table, &low, &high, ch, opts,
                             logging, logfile, nomem);

        if (pivot->prev_entry != NULL)
            quicksort_tcp_entries(table, low,
                                  pivot->prev_entry->prev_entry, ch, opts,
                                  logging, logfile, nomem);

        quicksort_tcp_entries(table, pivot->next_entry->next_entry, high,
                              ch, opts, logging, logfile, nomem);
    }
}

/* 
 * This function sorts the TCP window.  The old exchange sort has been
 * replaced with a Quicksort algorithm.
 */

void sortipents(struct tcptable *table, unsigned long *idx, int ch,
                int mode, int logging, FILE * logfile,
                time_t timeout, int *nomem, struct OPTIONS *opts)
{
    struct tcptableent *tcptmp1;
    unsigned int idxtmp;

    if ((table->head == NULL)
        || (table->head->next_entry->next_entry == NULL))
        return;

    ch = toupper(ch);

    if ((ch != 'P') && (ch != 'B'))
        return;

    quicksort_tcp_entries(table, table->head, table->tail->prev_entry,
                          ch, opts, logging, logfile, nomem);

    update_panels();
    doupdate();
    tx_colorwin(table->tcpscreen);

    tcptmp1 = table->firstvisible = table->head;
    *idx = 1;
    idxtmp = 0;

    while ((tcptmp1 != NULL) && (idxtmp <= table->imaxy - 1)) {
        if (idxtmp++ <= table->imaxy - 1)
            table->lastvisible = tcptmp1;
        tcptmp1 = tcptmp1->next_entry;
    }

}

/*
 * Attempt to communicate with rvnamed, and if it doesn't respond, try
 * to start it.
 */

int checkrvnamed(void)
{
    int execstat = 0;
    pid_t cpid = 0;
    int cstat;
    extern int errno;

    indicate("Trying to communicate with reverse lookup server");
    if (!rvnamedactive()) {
        indicate("Starting reverse lookup server");

        if ((cpid = fork()) == 0) {
            execstat = execl(RVNDFILE, NULL);

            /* 
             * execl() never returns, so if we reach this point, we have
             * a problem.
             */

            _exit(1);
        } else if (cpid == -1) {
            write_error("Can't spawn new process; lookups will block",
                        daemonized);
            return 0;
        } else {
            while (waitpid(cpid, &cstat, 0) < 0)
                if (errno != EINTR)
                    break;

            if (WEXITSTATUS(cstat) == 1) {
                write_error("Can't start rvnamed; lookups will block",
                            daemonized);
                return 0;
            } else {
                sleep(1);
                return 1;
            }
        }
    }
    return 1;
}

void update_flowrate(WINDOW * win, struct tcptableent *entry, time_t now,
                     int *cleared, int mode)
{
    float rate = 0;
    char units[10];

    wattrset(win, IPSTATLABELATTR);
    mvwprintw(win, 0, COLS * 47 / 80, "TCP flow rate: ");
    wattrset(win, IPSTATATTR);
    if (mode == KBITS) {
        strcpy(units, "kbits/s");
        rate =
            (float) (entry->spanbr * 8 / 1000) / (float) (now -
                                                          entry->
                                                          starttime);
    } else {
        strcpy(units, "kbytes/s");
        rate =
            (float) (entry->spanbr / 1024) / (float) (now -
                                                      entry->starttime);
    }
    mvwprintw(win, 0, COLS * 50 / 80 + 13, "%8.2f %s", rate, units);
    entry->spanbr = 0;
    *cleared = 0;
}

/* 
 * The IP Traffic Monitor
 */

void ipmon(struct OPTIONS *options,
           struct filterstate *ofilter, int facilitytime, char *ifptr)
{
    int logging = options->logging;
    struct sockaddr_ll fromaddr;        /* iface info */
    unsigned short linktype;    /* data link type */
    int fd;                     /* raw socket */
    char tpacket[MAX_PACKET_SIZE];      /* raw packet data */
    char *packet = NULL;        /* network packet ptr */
    struct iphdr *ippacket;
    struct tcphdr *transpacket; /* IP-encapsulated packet */
    unsigned int sport = 0, dport = 0;  /* TCP/UDP port values */
    char sp_buf[10];

    unsigned long screen_idx = 1;

    struct timeval tv;
    unsigned long starttime = 0;
    unsigned long now = 0;
    unsigned long timeint = 0;
    unsigned long updtime = 0;
    unsigned long long updtime_usec = 0;
    unsigned long long unow = 0;
    unsigned long closedint = 0;

    WINDOW *statwin;
    PANEL *statpanel;

    WINDOW *sortwin;
    PANEL *sortpanel;

    FILE *logfile = NULL;

    int curwin = 0;

    int readlen;
    char ifname[10];

    unsigned long long total_pkts = 0;

    unsigned int br;            /* bytes read.  Differs from readlen */
    /* only when packets fragmented */
    unsigned int iphlen;
    unsigned int totalhlen;

    struct tcptable table;
    struct tcptableent *tcpentry;
    struct tcptableent *tmptcp;
    int mode = 0;

    struct othptable othptbl;
    struct othptabent *othpent;

    int p_sstat = 0, p_dstat = 0;       /* Reverse lookup statuses prior to */
    /* reattempt in updateentry() */
    int pkt_result = 0;         /* Non-IP filter ok */

    int fragment = 0;           /* Set to 1 if not first fragment */

    int ch;
    int keymode = 0;
    char msgstring[80];
    int nomem = 0;

    struct promisc_states *promisc_list;

    int rvnfd = 0;

    int instance_id;
    int revlook = options->revlook;
    int statcleared = 0;
    int wasempty = 1;

    const int statx = COLS * 47 / 80;

    /* 
     * Mark this instance of the traffic monitor
     */

    if (!facility_active(IPMONIDFILE, ifptr))
        mark_facility(IPMONIDFILE, "IP traffic monitor", ifptr);
    else {
        snprintf(msgstring, 80,
                 "IP Traffic Monitor already listening on %s",
                 gen_iface_msg(ifptr));
        write_error(msgstring, daemonized);
        return;
    }

    if (ifptr != NULL) {
        if (!iface_supported(ifptr)) {
            err_iface_unsupported();
            unmark_facility(IPMONIDFILE, ifptr);
            return;
        }
        if (!iface_up(ifptr)) {
            err_iface_down();
            unmark_facility(IPMONIDFILE, ifptr);
            return;
        }
    }

    open_socket(&fd);

    if (fd < 0) {
        unmark_facility(IPMONIDFILE, ifptr);
        return;
    }

    if (options->promisc) {
        if (first_active_facility()) {
            init_promisc_list(&promisc_list);
            save_promisc_list(promisc_list);
            srpromisc(1, promisc_list);
        }
    }

    /*
     * Adjust instance counters
     */

    adjust_instance_count(PROCCOUNTFILE, 1);
    instance_id = adjust_instance_count(ITRAFMONCOUNTFILE, 1);
    strncpy(active_facility_countfile, ITRAFMONCOUNTFILE, 64);

    init_tcp_table(&table);
    init_othp_table(&othptbl, options->mac);

    statwin = newwin(1, COLS, LINES - 2, 0);
    statpanel = new_panel(statwin);
    wattrset(statwin, IPSTATLABELATTR);
    wmove(statwin, 0, 0);
    sprintf(sp_buf, "%%%dc", COLS);
    wprintw(statwin, sp_buf, ' ');
    prepare_statwin(statwin);
    show_stats(statwin, 0);
    markactive(curwin, table.borderwin, othptbl.borderwin);
    update_panels();
    doupdate();

    if (revlook) {
        if (checkrvnamed())
            open_rvn_socket(&rvnfd);
    } else
        rvnfd = 0;

    ipmonhelp();
    uniq_help(0);

    update_panels();
    doupdate();

    if (options->servnames)
        setservent(1);

    /*
     * Try to open log file if logging activated.  Turn off logging
     * (for this session only) if an error was discovered in opening
     * the log file.  Configuration setting is kept.  Who knows, the
     * situation may be corrected later.
     */

    if (logging) {
        if (strcmp(current_logfile, "") == 0) {
            strncpy(current_logfile,
                    gen_instance_logname(IPMONLOG, instance_id), 80);

            if (!daemonized)
                input_logfile(current_logfile, &logging);
        }
    }

    if (logging)
        opentlog(&logfile, current_logfile);

    if (logfile == NULL)
        logging = 0;

    if (logging)
        signal(SIGUSR1, rotate_ipmon_log);

    setprotoent(1);

    rotate_flag = 0;
    writelog(logging, logfile,
             "******** IP traffic monitor started ********");

    isdnfd = -1;
    exitloop = 0;
    gettimeofday(&tv, NULL);
    starttime = timeint = closedint = tv.tv_sec;

    while (!exitloop) {
        gettimeofday(&tv, NULL);
        now = tv.tv_sec;
        unow = tv.tv_sec * 1e+06 + tv.tv_usec;

        /* 
         * Print timer at bottom of screen
         */

        if (now - timeint >= 5) {
            printelapsedtime(starttime, now, othptbl.obmaxy - 1, 15,
                             othptbl.borderwin);
            timeint = now;
        }

        /*
         * Automatically clear closed/timed out entries
         */

        if ((options->closedint != 0) &&
            ((now - closedint) / 60 >= options->closedint)) {
            flushclosedentries(&table, &screen_idx,
                               logging, logfile, options);
            refreshtcpwin(&table, screen_idx, mode);
            closedint = now;
        }

        /*
         * Update screen at configured intervals.
         */

        if (((options->updrate != 0)
             && (now - updtime >= options->updrate))
            || ((options->updrate == 0)
                && (unow - updtime_usec >= DEFAULT_UPDATE_DELAY))) {
            update_panels();
            doupdate();
            updtime = now;
            updtime_usec = unow;
        }

        /*
         * If highlight bar is on some entry, update the flow rate
         * indicator after five seconds.
         */
        if (table.barptr != NULL) {
            if ((now - table.barptr->starttime) >= 5) {
                update_flowrate(statwin, table.barptr, now, &statcleared,
                                options->actmode);
                table.barptr->starttime = now;
            }
        } else {
            wattrset(statwin, IPSTATATTR);
            mvwprintw(statwin, 0, statx, "No TCP entries              ");
        }

        /*
         * Terminate facility should a lifetime be specified at the
         * command line
         */
        if ((facilitytime != 0)
            && (((now - starttime) / 60) >= facilitytime))
            exitloop = 1;

        /*
         * Close and rotate log file if signal was received
         */
        if ((rotate_flag == 1) && (logging)) {
            announce_rotate_prepare(logfile);
            write_tcp_unclosed(logging, logfile, &table);
            rotate_logfile(&logfile, target_logname);
            announce_rotate_complete(logfile);
            rotate_flag = 0;
        }

        getpacket(fd, tpacket, &fromaddr, &ch, &readlen, ifname,
                  table.tcpscreen);

        if (ch != ERR) {
            if (keymode == 0) {
                switch (ch) {
                case KEY_UP:
                    if (!curwin) {
                        if (table.barptr != NULL) {
                            if (table.barptr->prev_entry != NULL) {
                                tmptcp = table.barptr;
                                set_barptr((char **) &(table.barptr),
                                           (char *) table.barptr->
                                           prev_entry,
                                           &(table.barptr->prev_entry->
                                             starttime),
                                           (char *) &(table.barptr->
                                                      prev_entry->spanbr),
                                           sizeof(unsigned long), statwin,
                                           &statcleared, statx);

                                printentry(&table, tmptcp, screen_idx,
                                           mode);

                                if (table.baridx == 1)
                                    scrollupperwin(&table, SCROLLDOWN,
                                                   &screen_idx, mode);
                                else
                                    (table.baridx)--;

                                printentry(&table, table.barptr,
                                           screen_idx, mode);
                            }
                        }
                    } else
                        scrolllowerwin(&othptbl, SCROLLDOWN);
                    break;
                case KEY_DOWN:
                    if (!curwin) {
                        if (table.barptr != NULL) {
                            if (table.barptr->next_entry != NULL) {
                                tmptcp = table.barptr;
                                set_barptr((char **) &(table.barptr),
                                           (char *) table.barptr->
                                           next_entry,
                                           &(table.barptr->next_entry->
                                             starttime),
                                           (char *) &(table.barptr->
                                                      next_entry->spanbr),
                                           sizeof(unsigned long), statwin,
                                           &statcleared, statx);
                                printentry(&table, tmptcp, screen_idx,
                                           mode);

                                if (table.baridx == table.imaxy)
                                    scrollupperwin(&table, SCROLLUP,
                                                   &screen_idx, mode);
                                else
                                    (table.baridx)++;

                                printentry(&table, table.barptr,
                                           screen_idx, mode);
                            }
                        }
                    } else
                        scrolllowerwin(&othptbl, SCROLLUP);
                    break;
                case KEY_RIGHT:
                    if (curwin) {
                        if (othptbl.strindex != VSCRL_OFFSET)
                            othptbl.strindex = VSCRL_OFFSET;

                        refresh_othwindow(&othptbl);
                    }
                    break;
                case KEY_LEFT:
                    if (curwin) {
                        if (othptbl.strindex != 0)
                            othptbl.strindex = 0;

                        refresh_othwindow(&othptbl);
                    }
                    break;
                case KEY_PPAGE:
                case '-':
                    if (!curwin) {
                        if (table.barptr != NULL) {
                            pageupperwin(&table, SCROLLDOWN, &screen_idx,
                                         mode);
                            set_barptr((char **) &(table.barptr),
                                       (char *) table.lastvisible,
                                       &(table.lastvisible->starttime),
                                       (char *) &(table.lastvisible->
                                                  spanbr),
                                       sizeof(unsigned long), statwin,
                                       &statcleared, statx);
                            table.baridx =
                                table.lastvisible->index - screen_idx + 1;
                            refreshtcpwin(&table, screen_idx, mode);
                        }
                    } else {
                        pagelowerwin(&othptbl, SCROLLDOWN);
                        refresh_othwindow(&othptbl);
                    }
                    break;
                case KEY_NPAGE:
                case ' ':
                    if (!curwin) {
                        if (table.barptr != NULL) {
                            pageupperwin(&table, SCROLLUP, &screen_idx,
                                         mode);
                            set_barptr((char **) &(table.barptr),
                                       (char *) table.firstvisible,
                                       &(table.firstvisible->starttime),
                                       (char *) &(table.firstvisible->
                                                  spanbr),
                                       sizeof(unsigned long), statwin,
                                       &statcleared, statx);
                            table.baridx = 1;
                            refreshtcpwin(&table, screen_idx, mode);
                        }
                    } else {
                        pagelowerwin(&othptbl, SCROLLUP);
                        refresh_othwindow(&othptbl);
                    }
                    break;
                case KEY_F(6):
                case 'w':
                case 'W':
                case 9:
                    curwin = !curwin;
                    markactive(curwin, table.borderwin, othptbl.borderwin);
                    uniq_help(curwin);
                    break;
                case 'm':
                case 'M':
                    if (!curwin) {
                        mode = (mode + 1) % 3;
                        if ((mode == 1) && (!options->mac))
                            mode = 2;
                        refreshtcpwin(&table, screen_idx, mode);
                    }
                    break;
                case 12:
                case 'l':
                case 'L':
                    tx_refresh_screen();
                    break;

                case 'F':
                case 'f':
                case 'c':
                case 'C':
                    flushclosedentries(&table, &screen_idx,
                                       logging, logfile, options);
                    refreshtcpwin(&table, screen_idx, mode);
                    break;
                case 's':
                case 'S':
                    keymode = 1;
                    show_tcpsort_win(&sortwin, &sortpanel);
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
            } else if (keymode == 1) {
                keymode = 0;
                del_panel(sortpanel);
                delwin(sortwin);
                show_sort_statwin(&sortwin, &sortpanel);
                update_panels();
                doupdate();
                sortipents(&table, &screen_idx, ch, mode, logging, logfile,
                           options->timeout, &nomem, options);

                if (table.barptr != NULL) {
                    set_barptr((char **) &(table.barptr),
                               (char *) table.firstvisible,
                               &(table.firstvisible->starttime),
                               (char *) &(table.firstvisible->spanbr),
                               sizeof(unsigned long), statwin,
                               &statcleared, statx);
                    table.baridx = 1;
                }
                refreshtcpwin(&table, screen_idx, mode);
                del_panel(sortpanel);
                delwin(sortwin);
                update_panels();
                doupdate();
            }
        }

        if (readlen > 0) {
            total_pkts++;
            show_stats(statwin, total_pkts);

            pkt_result = processpacket((char *) tpacket, &packet, &readlen,
                                       &br, &sport, &dport, &fromaddr,
                                       &linktype, ofilter,
                                       MATCH_OPPOSITE_ALWAYS, ifname,
                                       ifptr);

            if (pkt_result != PACKET_OK)
                continue;

            if (fromaddr.sll_protocol != ETH_P_IP) {
                othpent = add_othp_entry(&othptbl, &table,
                                         0, 0, NOT_IP,
                                         fromaddr.sll_protocol,
                                         linktype, (char *) tpacket,
                                         (char *) packet, br, ifname, 0, 0,
                                         0, logging, logfile,
                                         options->servnames, 0, &nomem);
                continue;
            } else {
                ippacket = (struct iphdr *) packet;
                iphlen = ippacket->ihl * 4;
                transpacket = (struct tcphdr *) (packet + iphlen);

                if (ippacket->protocol == IPPROTO_TCP) {

                    tcpentry =
                        in_table(&table, ippacket->saddr, ippacket->daddr,
                                 ntohs(sport), ntohs(dport), ifname,
                                 logging, logfile, &nomem, options);

                    /* 
                     * Add a new entry if it doesn't exist, and, 
                     * to reduce the chances of stales, not a FIN.
                     */

                    if ((ntohs(ippacket->frag_off) & 0x3fff) == 0) {    /* first frag only */
                        totalhlen = iphlen + transpacket->doff * 4;

                        if ((tcpentry == NULL) && (!(transpacket->fin))) {

                            /*
                             * Ok, so we have a packet.  Add it if this connection
                             * is not yet closed, or if it is a SYN packet.
                             */

                            if (!nomem) {
                                wasempty = (table.head == NULL);
                                tcpentry = addentry(&table, (unsigned long)
                                                    ippacket->saddr,
                                                    (unsigned long)
                                                    ippacket->daddr, sport,
                                                    dport,
                                                    ippacket->protocol,
                                                    ifname, &revlook,
                                                    rvnfd,
                                                    options->servnames,
                                                    &nomem);

                                if (tcpentry != NULL) {
                                    printentry(&table,
                                               tcpentry->oth_connection,
                                               screen_idx, mode);

                                    if (wasempty) {
                                        set_barptr((char **)
                                                   &(table.barptr),
                                                   (char *) table.
                                                   firstvisible,
                                                   &(table.firstvisible->
                                                     starttime),
                                                   (char *) &(table.
                                                              firstvisible->
                                                              spanbr),
                                                   sizeof(unsigned long),
                                                   statwin, &statcleared,
                                                   statx);
                                        table.baridx = 1;
                                    }

                                    if ((table.barptr == tcpentry) ||
                                        (table.barptr ==
                                         tcpentry->oth_connection))
                                        set_barptr((char **)
                                                   &(table.barptr),
                                                   (char *) table.barptr,
                                                   &(table.barptr->
                                                     starttime),
                                                   (char *) &(table.
                                                              barptr->
                                                              spanbr),
                                                   sizeof(unsigned long),
                                                   statwin, &statcleared,
                                                   statx);
                                }
                            }
                        }
                    }
                    /* 
                     * If we had an addentry() success, we should have no
                     * problem here.  Same thing if we had a table lookup
                     * success.
                     */

                    if (tcpentry != NULL) {
                        /* 
                         * Don't bother updating the entry if the connection
                         * has been previously reset.  (Does this really
                         * happen in practice?)
                         */

                        if (!(tcpentry->stat & FLAG_RST)) {
                            if (revlook) {
                                p_sstat = tcpentry->s_fstat;
                                p_dstat = tcpentry->d_fstat;
                            }
                            updateentry(&table, tcpentry, transpacket,
                                        tpacket, linktype, readlen, br,
                                        ippacket->frag_off, logging,
                                        &revlook, rvnfd, options, logfile,
                                        &nomem);

                            /*
                             * Log first packet of a TCP connection except if
                             * it's a RST, which was already logged earlier in
                             * updateentry()
                             */

                            if ((tcpentry->pcount == 1) &&
                                (!(tcpentry->stat & FLAG_RST))
                                && (logging)) {
                                strcpy(msgstring, "first packet");
                                if (transpacket->syn)
                                    strcat(msgstring, " (SYN)");

                                writetcplog(logging, logfile,
                                            tcpentry, readlen,
                                            options->mac, msgstring);
                            }

                            if ((revlook)
                                && (((p_sstat != RESOLVED)
                                     && (tcpentry->s_fstat == RESOLVED))
                                    || ((p_dstat != RESOLVED)
                                        && (tcpentry->d_fstat ==
                                            RESOLVED)))) {
                                clearaddr(&table, tcpentry, screen_idx);
                                clearaddr(&table, tcpentry->oth_connection,
                                          screen_idx);
                            }
                            printentry(&table, tcpentry, screen_idx, mode);

                            /*
                             * Special cases: Update other direction if it's
                             * an ACK in response to a FIN. 
                             *
                             *         -- or --
                             *
                             * Addresses were just resolved for the other
                             * direction, so we should also do so here.
                             */

                            if (((tcpentry->oth_connection->finsent == 2) &&    /* FINed and ACKed */
                                 (ntohl(transpacket->seq) ==
                                  tcpentry->oth_connection->finack))
                                || ((revlook)
                                    && (((p_sstat != RESOLVED)
                                         && (tcpentry->s_fstat ==
                                             RESOLVED))
                                        || ((p_dstat != RESOLVED)
                                            && (tcpentry->d_fstat ==
                                                RESOLVED)))))
                                printentry(&table,
                                           tcpentry->oth_connection,
                                           screen_idx, mode);
                        }
                    }
                } else {        /* now for the other IP protocols */
                    fragment = ((ntohs(ippacket->frag_off) & 0x1fff) != 0);

                    if (ippacket->protocol == IPPROTO_ICMP) {

                        /*
                         * Cancel the corresponding TCP entry if an ICMP
                         * Destination Unreachable or TTL Exceeded message
                         * is received.
                         */

                        if (((struct icmphdr *) transpacket)->type ==
                            ICMP_DEST_UNREACH)
                            process_dest_unreach(&table,
                                                 (char *) transpacket,
                                                 ifname, &nomem);

                    }

                    othpent =
                        add_othp_entry(&othptbl, &table, ippacket->saddr,
                                       ippacket->daddr, IS_IP,
                                       ippacket->protocol, linktype,
                                       (char *) tpacket,
                                       (char *) transpacket, readlen,
                                       ifname, &revlook, rvnfd,
                                       options->timeout, logging, logfile,
                                       options->servnames, fragment,
                                       &nomem);
                }
            }
        }
    }

    if (get_instance_count(ITRAFMONCOUNTFILE) <= 1)
        killrvnamed();

    if (options->servnames)
        endservent();

    endprotoent();
    close_rvn_socket(rvnfd);

    if ((options->promisc) && (is_last_instance())) {
        load_promisc_list(&promisc_list);
        srpromisc(0, promisc_list);
        destroy_promisc_list(&promisc_list);
    }
    adjust_instance_count(PROCCOUNTFILE, -1);
    adjust_instance_count(ITRAFMONCOUNTFILE, -1);

    attrset(STDATTR);
    mvprintw(0, COLS - 20, "                    ");
    del_panel(table.tcppanel);
    del_panel(table.borderpanel);
    del_panel(othptbl.othppanel);
    del_panel(othptbl.borderpanel);
    del_panel(statpanel);
    update_panels();
    doupdate();
    delwin(table.tcpscreen);
    delwin(table.borderwin);
    delwin(othptbl.othpwin);
    delwin(othptbl.borderwin);
    delwin(statwin);
    close(fd);
    destroytcptable(&table);
    destroyothptable(&othptbl);
    pkt_cleanup();
    writelog(logging, logfile,
             "******** IP traffic monitor stopped ********\n");
    unmark_facility(IPMONIDFILE, ifptr);
    if (logfile != NULL)
        fclose(logfile);

    strcpy(current_logfile, "");

    signal(SIGUSR1, SIG_DFL);

    return;
}
