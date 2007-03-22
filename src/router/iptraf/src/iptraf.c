/*
===========================================================================
IPTraf
An IP Network Statistics Utility
Written by Gerard Paul Java <riker@seul.org>
Copyright (c) Gerard Paul Java 1997-2004

Version 3.0.0
Main Module

---------------------------------------------------------------------------
This software is open-source; you may redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.
---------------------------------------------------------------------------
*/

#define MAIN_MODULE

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <curses.h>
#include <panel.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <menurt.h>
#include <winops.h>
#include <msgboxes.h>
#include "dirs.h"
#include "deskman.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "fltmgr.h"
#include "fltedit.h"
#include "ifstats.h"
#include "serv.h"
#include "options.h"
#include "promisc.h"
#include "externs.h"
#include "instances.h"
#include "tcptable.h"
#include "log.h"
#include "attrs.h"
#include "rvnamed.h"
#include "logvars.h"
#include "getpath.h"

#define WITHALL 1
#define WITHOUTALL 0

const char *ALLSPEC = "all";

/*
 * Important globals used throughout the
 * program.
 */
int exitloop = 0;
int daemonized = 0;
int facility_running = 0;
int is_first_instance;
char active_facility_lockfile[64];
char active_facility_countfile[64];
int accept_unsupported_interfaces = 0;
char graphing_filter[80];

extern void about();

void press_enter_to_continue(void)
{
    fprintf(stderr, "Press Enter to continue.\n");
    getchar();
}

void clearfiles(char *prefix, char *directory)
{
    DIR *dir;
    struct dirent *dir_entry;
    char target_name[80];

    dir = opendir(directory);

    if (dir == NULL) {
        fprintf(stderr, "\nUnable to read directory %s\n%s\n",
                directory, strerror(errno));
        press_enter_to_continue();
        return;
    }

    do {
        dir_entry = readdir(dir);
        if (dir_entry != NULL) {
            if (strncmp(dir_entry->d_name, prefix, strlen(prefix)) == 0) {
                snprintf(target_name, 80, "%s/%s", directory,
                         dir_entry->d_name);
                unlink(target_name);
            }
        }
    } while (dir_entry != NULL);

    closedir(dir);
}

void removetags(void)
{
    clearfiles("iptraf", LOCKDIR);
}

void remove_sockets(void)
{
    clearfiles(SOCKET_PREFIX, WORKDIR);
}

/*
 * Handlers for the TERM signal and HUP signals.  There's nothing we can do
 * for the KILL.
 */

void term_signal_handler(int signo)
{
    erase();
    refresh();
    endwin();

    if (signo != SIGHUP)
        fprintf(stderr, "IPTraf process %u exiting on signal %d\n\n",
                getpid(), signo);

    if (active_facility_lockfile[0] != '\0') {
        unlink(active_facility_lockfile);
        adjust_instance_count(PROCCOUNTFILE, -1);
        if (active_facility_countfile[0] != '\0')
            adjust_instance_count(active_facility_countfile, -1);
    }

    if (is_first_instance)
        unlink(IPTIDFILE);

    exit(1);
}

/* 
 * Handler for the SIGSEGV, Segmentation Fault.  Tries to clear the screen
 * and issue a better message than "Segmentation fault".  May not always
 * clean up properly.
 */

void segvhandler()
{
    erase();
    refresh();
    endwin();
    fprintf(stderr, "Fatal: memory allocation error\n\n");
    fprintf(stderr,
            "If you suspect a bug, please report the exact circumstances under which this\n");
    fprintf(stderr,
            "error was generated.  If possible, include gdb or strace data which may point\n");
    fprintf(stderr,
            "out where the error occured.  Bug reports may be sent in to iptraf@seul.org.\n\n");
    fprintf(stderr,
            "An attempt will be made to clear all lock files, but if stale lock files\n");
    fprintf(stderr,
            "remain, exit all other instances of IPTraf and restart with the -f\n");
    fprintf(stderr, "command-line parameter.\n\n");
    fprintf(stderr, "IPTraf process %u aborting on signal 11.\n\n",
            getpid());

    if (active_facility_lockfile[0] != '\0')
        unlink(active_facility_lockfile);

    if (is_first_instance)
        unlink(IPTIDFILE);

    if (active_facility_lockfile[0] != '\0') {
        unlink(active_facility_lockfile);
        adjust_instance_count(PROCCOUNTFILE, -1);
        if (active_facility_countfile[0] != '\0')
            adjust_instance_count(active_facility_countfile, -1);
    }

    exit(2);
}

/*
 * USR2 handler.  Used to normally exit a daemonized facility.
 */

void term_usr2_handler()
{
    exitloop = 1;
}

void init_break_menu(struct MENU *break_menu)
{
    tx_initmenu(break_menu, 6, 20, (LINES - 6) / 2, COLS / 2,
                BOXATTR, STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR,
                DESCATTR);
    tx_additem(break_menu, " By packet ^s^ize",
               "Displays packet counts by packet size range");
    tx_additem(break_menu, " By TCP/UDP ^p^ort",
               "Displays packet and byte counts by service port");
    tx_additem(break_menu, NULL, NULL);
    tx_additem(break_menu, " E^x^it menu", "Return to main menu");
}

/*
 * Get the ball rolling: The program interface routine.
 */

void program_interface(struct OPTIONS *options,
                       int opt, char *optarg, int facilitytime)
{
    struct MENU menu;
    struct MENU break_menu;

    int endloop = 0;
    int row = 1;
    int break_row = 1;
    int aborted;
    int break_aborted;

    struct filterstate ofilter;
    struct ffnode *fltfiles;

    char ifname[10];
    char *ifptr = NULL;
    struct porttab *ports;

    draw_desktop();

    attrset(STATUSBARATTR);
    mvprintw(0, 1, "IPTraf");

    /*
     * Load saved filter or graphing filter if specified
     */
    if (graphing_logfile[0] != '\0') {
        loadfilterlist(&fltfiles);
        memset(&ofilter, 0, sizeof(struct filterstate));
        loadfilter(pickfilterbyname(fltfiles, graphing_filter),
                   &(ofilter.fl), FLT_RESOLVE);
    } else {
        loadfilters(&ofilter);
        indicate("");
    }

    loadaddports(&ports);

    if (opt == 0) {
        attrset(STATUSBARATTR);
        mvprintw(LINES - 1, 1, PLATFORM);
        about();

        tx_initmenu(&menu, 13, 35, (LINES - 14) / 2, (COLS - 35) / 2,
                    BOXATTR, STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR,
                    DESCATTR);

        tx_additem(&menu, " IP traffic ^m^onitor",
                   "Displays current IP traffic information");
        tx_additem(&menu, " General interface ^s^tatistics",
                   "Displays some statistics for attached interfaces");
        tx_additem(&menu, " ^D^etailed interface statistics",
                   "Displays more statistics for a selected interface");
        tx_additem(&menu, " Statistical ^b^reakdowns...",
                   "Facilities for traffic counts by packet size or TCP/UDP port");
        tx_additem(&menu, " ^L^AN station monitor",
                   "Displays statistics on detected LAN stations");
        tx_additem(&menu, NULL, NULL);
        tx_additem(&menu, " ^F^ilters...",
                   "Allows you to select traffic display and logging criteria");
        tx_additem(&menu, NULL, NULL);
        tx_additem(&menu, " C^o^nfigure...",
                   "Set various program options");
        tx_additem(&menu, NULL, NULL);
        tx_additem(&menu, " E^x^it", "Exits program");

        endloop = 0;

        do {
            tx_showmenu(&menu);
            tx_operatemenu(&menu, &row, &aborted);

            switch (row) {
            case 1:
                selectiface(ifname, WITHALL, &aborted);
                if (!aborted) {
                    if (strcmp(ifname, "") != 0)
                        ifptr = ifname;
                    else
                        ifptr = NULL;

                    ipmon(options, &ofilter, 0, ifptr);
                }
                break;
            case 2:
                ifstats(options, &ofilter, 0);
                break;
            case 3:
                selectiface(ifname, WITHOUTALL, &aborted);
                if (!aborted)
                    detstats(ifname, options, 0, &ofilter);
                break;
            case 4:
                break_row = 1;
                init_break_menu(&break_menu);
                tx_showmenu(&break_menu);
                tx_operatemenu(&break_menu, &break_row, &break_aborted);

                switch (break_row) {
                case 1:
                    selectiface(ifname, WITHOUTALL, &aborted);
                    if (!aborted)
                        packet_size_breakdown(options, ifname, 0,
                                              &ofilter);
                    break;
                case 2:
                    selectiface(ifname, WITHOUTALL, &aborted);
                    if (!aborted)
                        servmon(ifname, ports, options, 0, &ofilter);
                    break;
                case 4:
                    break;
                }
                tx_destroymenu(&break_menu);
                break;
            case 5:
                selectiface(ifname, WITHALL, &aborted);
                if (!aborted) {
                    if (strcmp(ifname, "") != 0)
                        ifptr = ifname;
                    else
                        ifptr = NULL;
                    hostmon(options, 0, ifptr, &ofilter);
                }
                break;
            case 7:
                config_filters(&ofilter);
                savefilters(&ofilter);
                break;
            case 9:
                setoptions(options, &ports);
                saveoptions(options);
                break;
            case 11:
                endloop = 1;
                break;
            }
        } while (!endloop);

        tx_destroymenu(&menu);
    } else {
        switch (opt) {
        case 'i':
            if ((strcmp(optarg, ALLSPEC) == 0)
                || (strcmp(optarg, "") == 0))
                ifptr = NULL;
            else
                ifptr = optarg;

            ipmon(options, &ofilter, facilitytime, ifptr);
            break;
        case 'g':
            ifstats(options, &ofilter, facilitytime);
            break;
        case 'd':
            detstats(optarg, options, facilitytime, &ofilter);
            break;
        case 's':
            servmon(optarg, ports, options, facilitytime, &ofilter);
            break;
        case 'z':
            packet_size_breakdown(options, optarg, facilitytime, &ofilter);
            break;
        case 'l':
            if ((strcmp(optarg, ALLSPEC) == 0)
                || (strcmp(optarg, "") == 0))
                ifptr = NULL;
            else
                ifptr = optarg;

            hostmon(options, facilitytime, ifptr, &ofilter);
            break;
        }
    }

    destroyporttab(ports);
    erase();
    update_panels();
    doupdate();
}


/*
 * Command-line help facility.
 */

void commandhelp()
{
    printf("\nSyntax:\n");
    printf
        ("    iptraf [ -f ] [ { -i iface | -g | -d iface | -s iface | -z iface |\n");
    printf
        ("           -l iface } [ -t timeout ] [ -B ] [ -L logfile ] [-I interval] ] \n\n");
    printf
        ("Issue the iptraf command with no parameters for menu-driven operation.\n");
    printf("These options can also be supplied to the command:\n\n");
    printf
        ("-i iface    - start the IP traffic monitor (use \"-i all\" for all interfaces)\n");
    printf("-g          - start the general interface statistics\n");
    printf
        ("-d iface    - start the detailed statistics facility on an interface\n");
    printf
        ("-s iface    - start the TCP and UDP monitor on an interface\n");
    printf("-z iface    - shows the packet size counts on an interface\n");
    printf
        ("-l iface    - start the LAN station monitor (\"-l all\" for all LAN interfaces)\n");
    printf
        ("-B          - run in background (use only with one of the above parameters)\n");
    printf
        ("-t timeout  - when used with one of the above parameters, tells\n");
    printf
        ("              the facility to run only for the specified number of\n");
    printf("              minutes (timeout)\n");
    printf
        ("-L logfile  - specifies an alternate log file for any direct invocation\n");
    printf
        ("              of a facility from the command line.  The log is placed in\n");
    printf("              %s if path is not specified.\n", LOGDIR);
    printf
        ("-I interval - specifies the log interval for all facilities except the IP\n");
    printf("              traffic monitor.  Value is in minutes.\n");
    printf
        ("-f          - clear all locks and counters.  Use with great caution.\n");
    printf
        ("              Normally used to recover from an abnormal termination.\n\n");
    printf("IPTraf %s Copyright (c) Gerard Paul Java 1997-2004\n",
           VERSION);
}

int first_instance()
{
    int fd;

    fd = open(IPTIDFILE, O_RDONLY);

    if (fd < 0)
        return !0;
    else {
        close(fd);
        return 0;
    }
}

void mark_first_instance()
{
    int fd;

    fd = open(IPTIDFILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "\nWarning: unable to tag this process\r\n");
        press_enter_to_continue();
        return;
    }
    close(fd);
}

/*
 * Main routine
 */

int main(int argc, char **argv)
{
    struct OPTIONS options;
    int opt = 0;
    int command = 0;
    char keyparm[12];
    int facilitytime = 0;
    int current_log_interval;

#ifndef ALLOWUSERS
    if (getuid() != 0) {
        fprintf(stderr, "\nIPTraf Version %s\n", VERSION);
        fprintf(stderr, "Copyright (c) Gerard Paul Java 1997-2004l\n\n");
        fprintf(stderr,
                "This program can be run only by the system administrator\n\n");
        exit(1);
    }
#endif

    strcpy(current_logfile, "");
    strcpy(graphing_logfile, "");
    strcpy(graphing_filter, "");

    /*
     * Parse command line
     */

    if (argc > 1) {
        do {
            opterr = 0;
            opt = getopt(argc, argv, "i:gd:s:z:l:hfqt:BL:uI:G:F:");

            if (opt == 'h') {
                commandhelp();
                exit(0);
            } else if (opt == 'f') {
                removetags();
                remove_sockets();
            } else if (opt == 't') {
                facilitytime = atoi(optarg);
                if (facilitytime == 0) {
                    fprintf(stderr, "\nInvalid time value\n\n");
                    exit(1);
                }
            } else if (opt == 'B') {
                daemonized = 1;
                setenv("TERM", "linux", 1);
            } else if (opt == 'L') {
                if (strchr(optarg, '/') != NULL)
                    strncpy(current_logfile, optarg, 80);
                else
                    strncpy(current_logfile, get_path(T_LOGDIR, optarg),
                            80);
            } else if (opt == 'q') {
                /* -q parameter now ignored, maintained for compatibility */
            } else if (opt == 'u') {
                accept_unsupported_interfaces = 1;
            } else if (opt == 'I') {
                current_log_interval = atoi(optarg);
                if (current_log_interval == 0)
                    fprintf(stderr, "Invalid log interval value\n");

                exit(1);
            } else if (opt == 'G') {
                if (strchr(optarg, '/') != NULL)
                    strncpy(graphing_logfile, optarg, 80);
                else
                    strncpy(graphing_logfile, get_path(T_LOGDIR, optarg),
                            80);

                daemonized = 1;
            } else if (opt == 'F') {
                strncpy(graphing_filter, optarg, 80);
            } else if (opt == '?') {
                fprintf(stderr,
                        "\nInvalid option or missing parameter, use iptraf -h for help\n\n");
                exit(1);
            } else if (opt != -1) {
                if (optarg != 0) {
                    bzero(keyparm, 12);
                    strncpy(keyparm, optarg, 11);
                } else
                    strcpy(keyparm, "");

                command = opt;
            }
        } while ((opt != '?') && (opt != -1));
    }
    is_first_instance = first_instance();

    if ((getenv("TERM") == NULL) && (!daemonized)) {
        fprintf(stderr, "Your TERM variable is not set.\n");
        fprintf(stderr, "Please set it to an appropriate value.\n");
        exit(1);
    }

    if (graphing_logfile[0] != '\0' && graphing_filter[0] == '\0') {
        fprintf(stderr, "Specify an IP filter name with -F\n");
        exit(1);
    }

    loadoptions(&options);

    /*
     * If a facility is directly invoked from the command line, check for
     * a daemonization request
     */

    if ((daemonized) && (command != 0)) {
        switch (fork()) {
        case 0:                /* child */
            setsid();
            freopen("/dev/null", "w", stdout);  /* redirect std output */
            freopen("/dev/null", "r", stdin);   /* redirect std input */
            freopen("/dev/null", "w", stderr);  /* redirect std error */
            signal(SIGUSR2, (void *) term_usr2_handler);

            if (graphing_logfile[0] != '\0')
                options.logging = 0;    /* if raw logging is specified */
            else                /* then standard logging is disabled */
                options.logging = 1;
            break;
        case -1:               /* error */
            fprintf(stderr,
                    "\nFork error, IPTraf cannot run in background\n\n");
            exit(1);
        default:               /* parent */
            exit(0);
        }
    }
#ifdef SIMDAEMON
    daemonized = 1;
    freopen("/dev/null", "w", stdout);  /* redirect std output */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stderr);
#endif

    initscr();

    if ((LINES < 24) || (COLS < 80)) {
        endwin();
        fprintf(stderr,
                "\nThis program requires a screen size of at least 80 columns by 24 lines\n");
        fprintf(stderr, "Please resize your window\n\n");
        exit(1);
    }

    mark_first_instance();

    signal(SIGTERM, (void *) term_signal_handler);
    signal(SIGHUP, (void *) term_signal_handler);
    signal(SIGSEGV, (void *) segvhandler);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);

    start_color();
    standardcolors(options.color);
    noecho();
    nonl();
    cbreak();

#ifndef DEBUG
    curs_set(0);
#endif

    /*
     * Set logfilename variable to NULL if -L was specified without an
     * appropriate facility on the command line.
     */

    if (command == 0)
        strcpy(current_logfile, "");

    /*
     * If by this time the logfile is still acceptable, obtain the
     * logspan from the command line if so specified.
     */

    if (current_logfile[0] != '\0') {
        options.logging = 1;
        if (current_log_interval != 0) {
            options.logspan = current_log_interval;
        }
    }

    program_interface(&options, command, keyparm, facilitytime);

    endwin();

    if (is_first_instance)
        unlink(IPTIDFILE);

    return (0);
}
