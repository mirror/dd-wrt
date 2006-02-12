/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: defs.h,v 1.44 2005/03/06 19:33:35 kattemat Exp $
 */

#ifndef OLSR_PLUGIN

#ifndef _OLSR_DEFS
#define _OLSR_DEFS

/* Common includes */
#include <sys/time.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "olsr_protocol.h"
#include "net.h" /* IPaddr -> string conversions is used by everyone */
#include "olsr_cfg.h"

#define VERSION "0.4.9"
#define SOFTWARE_VERSION "olsr.org - " VERSION
#define OLSRD_VERSION_DATE "       *** " SOFTWARE_VERSION " (" __DATE__ ") ***\n"

#define OLSRD_CONF_FILE_NAME "olsrd.conf"
#define OLSRD_GLOBAL_CONF_FILE "/etc/" OLSRD_CONF_FILE_NAME

#define	HOPCNT_MAX		32	/* maximum hops number */
#define	MAXMESSAGESIZE		1500	/* max broadcast size */
#define UDP_IPV4_HDRSIZE        28
#define UDP_IPV6_HDRSIZE        48
#define MAX_IFS                 16

/* Debug helper macro */
#ifdef DEBUG
#define olsr_debug(lvl,format,args...) \
   OLSR_PRINTF(lvl, "%s (%s:%d): ", __func__, __FILE__, __LINE__); \
   OLSR_PRINTF(lvl, format, ##args);
#endif

FILE *debug_handle;

#ifdef NODEBUG
#define OLSR_PRINTF(lvl, format, args...) \
   { }
#else
#define OLSR_PRINTF(lvl, format, args...) \
   { \
     if((olsr_cnf->debug_level >= lvl) && debug_handle) \
        fprintf(debug_handle, format, ##args); \
   }
#endif

/* Provides a timestamp s1 milliseconds in the future
   according to system ticks returned by times(2) */
#define GET_TIMESTAMP(s1) \
        now_times + ((s1) / system_tick_divider)

#define TIMED_OUT(s1) \
        ((int)((s1) - now_times) < 0)


/*
 * Queueing macros
 */

/* First "argument" is NOT a pointer! */

#define QUEUE_ELEM(pre, new) \
        pre.next->prev = new; \
        new->next = pre.next; \
        new->prev = &pre; \
        pre.next = new

#define DEQUEUE_ELEM(elem) \
	elem->prev->next = elem->next; \
	elem->next->prev = elem->prev


/*
 * Global olsrd configuragtion
 */

struct olsrd_config *olsr_cnf;

/* Global tick resolution */
olsr_u16_t system_tick_divider;

int exit_value; /* Global return value for process termination */


/* Timer data */
clock_t now_times;              /* current idea of times(2) reported uptime */
struct timeval now;		/* current idea of time */
struct tm *nowtm;		/* current idea of time (in tm) */

olsr_bool disp_pack_in;         /* display incoming packet content? */
olsr_bool disp_pack_out;        /* display outgoing packet content? */

olsr_bool del_gws;

/*
 * Timer values
 */

float will_int;
float max_jitter;

size_t ipsize;

/* Main address of this node */
union olsr_ip_addr main_addr;

/* OLSR UPD port */
int olsr_udp_port;

/* The socket used for all ioctls */
int ioctl_s;

/* routing socket */
#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__
int rts;
#endif

float max_tc_vtime;

clock_t fwdtimer[MAX_IFS];	/* forwarding timer */

int minsize;

olsr_bool changes;                /* is set if changes occur in MPRS set */ 

/* TC empty message sending */
clock_t send_empty_tc;

/*
 *IPC functions
 *These are moved to a plugin soon
 * soon... duh!
 */

int
ipc_init(void);

int
ipc_input(int);

int
shutdown_ipc(void);

int
ipc_output(struct olsr *);

int
ipc_send_net_info(void);

int
ipc_route_send_rtentry(union olsr_ip_addr*, union olsr_ip_addr *, int, int, char *);



#endif
#endif
