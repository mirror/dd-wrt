/***

rvnamed		- reverse DNS lookup daemon for the IPTraf network
		  statistics utility.

Version 2.6.1                      Parallel with IPTraf 2.6

Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1998-2001

rvnamed is a daemon designed to do reverse DNS lookups, but return the
IP address immediately while the lookup goes on in the background.
A process requesting the lookup issues a request, and will immediately
get a reply with the IP address.  Meanwhile, rvnamed will fork and do
the lookup.  The requesting process simply needs to reissue the request
until a full domain name is returned.

This program is designed to be used by the IPTraf program to minimize
blocking and allow smoother keyboard control and packet counting when
reverse DNS lookups are enabled.

rvnamed and IPTraf communicate with each other using the BSD UNIX domain
socket protocol.

This software is open-source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "rvnamed.h"
#include "dirs.h"

#define NUM_CACHE_ENTRIES 2048
#define TIME_TARGET_MAX 30

struct hosts {
    unsigned long addr;
    char fqdn[45];
    int ready;
};

static int fork_count = 0;
static int max_fork_count = 0;

/*
 * This is the classic zombie-preventer
 */

void childreap()
{
    signal(SIGCHLD, childreap);

    while (waitpid(-1, NULL, WNOHANG) > 0)
        fork_count--;
}

void auto_terminate()
{
    exit(2);
}

/*
 * Process reverse DNS request from the client
 */

void process_rvn_packet(struct rvn *rvnpacket)
{
    int ccfd;
    extern int h_errno;
    struct sockaddr_un ccsa;

    struct hostent *he;

    ccfd = socket(PF_UNIX, SOCK_DGRAM, 0);

    he = gethostbyaddr((char *) &(rvnpacket->saddr),
                       sizeof(struct in_addr), AF_INET);

    if (he == NULL)
        strcpy(rvnpacket->fqdn, inet_ntoa(rvnpacket->saddr));
    else {
        bzero(rvnpacket->fqdn, 45);
        strncpy(rvnpacket->fqdn, he->h_name, 44);
    }

    ccsa.sun_family = AF_UNIX;
    strcpy(ccsa.sun_path, CHILDSOCKNAME);

    sendto(ccfd, rvnpacket, sizeof(struct rvn), 0,
           (struct sockaddr *) &ccsa,
           sizeof(ccsa.sun_family) + strlen(ccsa.sun_path));
    close(ccfd);
}

/*
 * Check if name is already resolved and in the cache.
 */

int name_resolved(struct rvn *rvnpacket, struct hosts *hostlist,
                  unsigned int lastfree)
{
    unsigned int i = 0;

    while (i != lastfree) {
        if ((rvnpacket->saddr.s_addr == hostlist[i].addr)
            && (hostlist[i].ready == RESOLVED))
            return i;

        i++;
    }

    return -1;
}

/*
 * Return the resolution status (NOTRESOLVED, RESOLVING, RESOLVED) of
 * the given IP address
 */

int addrstat(struct rvn *rvnpacket, struct hosts *hostlist,
             unsigned int lastfree)
{
    unsigned int i = 0;

    while (i != lastfree) {
        if (rvnpacket->saddr.s_addr == hostlist[i].addr)
            break;

        i++;
    }

    if (i != lastfree)
        return hostlist[i].ready;

    return NOTRESOLVED;
}

void writervnlog(FILE * fd, char *msg)
{
    time_t now;
    char atime[TIME_TARGET_MAX] = "";

    now = time((time_t *) NULL);

    strcpy(atime, ctime(&now));
    atime[strlen(atime) - 1] = '\0';

    fprintf(fd, "%s: %s\n", atime, msg);
}

int main(void)
{
    int cfd;
    int ifd;

    struct hosts hostlist[NUM_CACHE_ENTRIES];
    char logmsg[160];

    unsigned int hostindex = 0;
    unsigned int lastfree = 0;
    unsigned int hi = 0;
    int readyidx = 0;
    int fr = 0;
    int maxlogged = 0;

    struct rvn rvnpacket;

    int br;

    int ss = 0;

    fd_set sockset;

    struct sockaddr_un csa, isa;        /* child and iptraf comm sockets */
    struct sockaddr_un fromaddr;
    int fromlen;

    FILE *logfile;

    extern int errno;
    pid_t pid;

    /* Daemonization Sequence */

#ifndef DEBUG
    switch (fork()) {
    case -1:
        exit(1);
    case 0:
        break;
    default:
        exit(0);
    }

    setsid();
    chdir("/");
#endif

    signal(SIGCHLD, childreap);

    logfile = fopen(RVNDLOGFILE, "a");

    if (logfile == NULL)
        logfile = fopen("/dev/null", "a");

    writervnlog(logfile, "******** rvnamed started ********");
    writervnlog(logfile, "Clearing socket names");

    /*
     * Get rid of any residue socket names in case of a previous
     * abormal termination of rvnamed.
     */

    unlink(CHILDSOCKNAME);
    unlink(IPTSOCKNAME);

    writervnlog(logfile, "Opening sockets");
    csa.sun_family = AF_UNIX;
    strcpy(csa.sun_path, CHILDSOCKNAME);

    isa.sun_family = AF_UNIX;
    strcpy(isa.sun_path, IPTSOCKNAME);

    cfd = socket(PF_UNIX, SOCK_DGRAM, 0);

    if (cfd < 0) {
        writervnlog(logfile,
                    "Unable to open child communication socket, aborting");
        exit(1);
    }
    if (bind
        (cfd, (struct sockaddr *) &csa,
         sizeof(csa.sun_family) + strlen(csa.sun_path)) < 0) {
        writervnlog(logfile,
                    "Error binding child communication socket, aborting");
        exit(1);
    }
    ifd = socket(PF_UNIX, SOCK_DGRAM, 0);

    if (ifd < 0) {
        writervnlog(logfile,
                    "Unable to open client communication socket, aborting");
        exit(1);
    }
    if (bind(ifd, (struct sockaddr *) &isa,
             sizeof(isa.sun_family) + strlen(isa.sun_path)) < 0) {
        writervnlog(logfile,
                    "Error binding client communication socket, aborting");
        exit(1);
    }
    while (1) {
        FD_ZERO(&sockset);
        FD_SET(cfd, &sockset);
        FD_SET(ifd, &sockset);

        do {
            ss = select(ifd + 1, &sockset, NULL, NULL,
                        (struct timeval *) NULL);
        } while ((ss < 0) && (errno != ENOMEM));

        if (errno == ENOMEM) {
            writervnlog(logfile,
                        "Fatal error: no memory for descriptor monitoring");
            close(ifd);
            close(cfd);
            fclose(logfile);
            exit(1);
        }
        /*
         * Code to process packets coming from the forked child.
         */

        if (FD_ISSET(cfd, &sockset)) {
            fromlen =
                sizeof(fromaddr.sun_family) + strlen(fromaddr.sun_path);
            br = recvfrom(cfd, &rvnpacket, sizeof(struct rvn), 0,
                          (struct sockaddr *) &fromaddr, &fromlen);

            if (br > 0) {
                hi = 0;

                while (hi <= lastfree) {
                    if (hostlist[hi].addr == rvnpacket.saddr.s_addr)
                        break;
                    hi++;
                }

                if (hi == lastfree) {   /* Address not in cache */
                    bzero(&(hostlist[hi]), sizeof(struct hosts));
                    hi = hostindex;
                    hostindex++;
                    if (hostindex == NUM_CACHE_ENTRIES)
                        hostindex = 0;

                    hostlist[hi].addr = rvnpacket.saddr.s_addr;
                }
                strncpy(hostlist[hi].fqdn, rvnpacket.fqdn, 44);

                hostlist[hi].ready = RESOLVED;
            }
        }
        /*
         * This code section processes packets received from the IPTraf
         * program.
         */

        if (FD_ISSET(ifd, &sockset)) {
            fromlen = sizeof(struct sockaddr_un);
            br = recvfrom(ifd, &rvnpacket, sizeof(struct rvn), 0,
                          (struct sockaddr *) &fromaddr, &fromlen);
            if (br > 0) {
                switch (rvnpacket.type) {
                case RVN_HELLO:
                    sendto(ifd, &rvnpacket, sizeof(struct rvn), 0,
                           (struct sockaddr *) &fromaddr,
                           sizeof(fromaddr.sun_family) +
                           strlen(fromaddr.sun_path));
                    break;
                case RVN_QUIT:
#ifndef DEBUG
                    writervnlog(logfile, "Received quit instruction");
                    writervnlog(logfile, "Closing sockets");
                    close(ifd);
                    close(cfd);
                    writervnlog(logfile, "Clearing socket names");
                    unlink(IPTSOCKNAME);
                    unlink(CHILDSOCKNAME);
                    sprintf(logmsg,
                            "rvnamed terminating: max processes spawned: %d",
                            max_fork_count);
                    writervnlog(logfile, logmsg);
                    writervnlog(logfile,
                                "******** rvnamed terminated ********");
                    fclose(logfile);
                    exit(0);
#endif
                case RVN_REQUEST:
                    readyidx =
                        name_resolved(&rvnpacket, hostlist, lastfree);
                    if (readyidx >= 0) {
                        rvnpacket.type = RVN_REPLY;
                        bzero(rvnpacket.fqdn, 45);
                        strncpy(rvnpacket.fqdn, hostlist[readyidx].fqdn,
                                44);
                        rvnpacket.ready = RESOLVED;

                        br = sendto(ifd, &rvnpacket, sizeof(struct rvn), 0,
                                    (struct sockaddr *) &fromaddr,
                                    sizeof(fromaddr.sun_family) +
                                    strlen(fromaddr.sun_path));
                    } else {

                        /* 
                         * Add this IP address to the cache if this is a
                         * new one.
                         */

                        if (addrstat(&rvnpacket, hostlist, lastfree) ==
                            NOTRESOLVED) {
                            fflush(logfile);    /* flush all data prior */
                            /* to fork() */

                            if (fork_count <= MAX_RVNAMED_CHILDREN) {
                                /*
                                 * If we can still fork(), we add the data
                                 * to the cache array, but we don't update
                                 * the indexes until after the fork()
                                 * succeeds.  If the fork() fails, we'll
                                 * just reuse this slot for the next query.
                                 *
                                 * This is so that if the fork() fails due
                                 * to a temporary condition, rvnamed won't
                                 * think it's RESOLVING while there isn't
                                 * any actual child doing the resolution
                                 * before the entry expires.
                                 *
                                 * However, we'll still tell IPTraf that the
                                 * address is RESOLVING.
                                 *
                                 */
                                hostlist[hostindex].addr =
                                    rvnpacket.saddr.s_addr;
                                hostlist[hostindex].ready = RESOLVING;

                                maxlogged = 0;
                                fr = fork();
                            } else {
                                fr = -1;
                                if (!maxlogged)
                                    writervnlog(logfile,
                                                "Maximum child process limit reached");
                                maxlogged = 1;
                            }

                            switch (fr) {
                            case 0:    /* spawned child */
                                fclose(logfile);        /* no logging in child */
                                close(ifd);     /* no comm with client */
                                pid = getpid();

                                /*
                                 * Set auto-terminate timeout
                                 */
                                signal(SIGALRM, auto_terminate);
                                alarm(300);
                                process_rvn_packet(&rvnpacket);
                                exit(0);
                            case -1:
                                if (!maxlogged)
                                    writervnlog(logfile,
                                                "Error on fork, returning IP address");
                                break;
                            default:   /* parent */
                                if (fork_count > max_fork_count)
                                    max_fork_count = fork_count;

                                /*
                                 * Increase cache indexes only if fork()
                                 * succeeded, otherwise the previously
                                 * allocated slots will be used for the
                                 * next query.
                                 */

                                hostindex++;

                                if (hostindex == NUM_CACHE_ENTRIES)
                                    hostindex = 0;

                                if (lastfree < NUM_CACHE_ENTRIES)
                                    lastfree++;

                                fork_count++;
                                break;
                            }
                        }
                        rvnpacket.type = RVN_REPLY;
                        bzero(rvnpacket.fqdn, 45);
                        strcpy(rvnpacket.fqdn, inet_ntoa(rvnpacket.saddr));
                        rvnpacket.ready = RESOLVING;

                        br = sendto(ifd, &rvnpacket, sizeof(struct rvn), 0,
                                    (struct sockaddr *) &fromaddr,
                                    sizeof(fromaddr.sun_family) +
                                    strlen(fromaddr.sun_path));

                    }
                }
            }
        }                       /* end block for packets from IPTraf */
    }
}
