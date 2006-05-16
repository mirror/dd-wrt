
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
 * $Id: olsrd_plugin.h,v 1.13 2005/02/20 17:33:43 kattemat Exp $
 */

#ifndef _OLSRD_PLUGIN_DEFS
#define _OLSRD_PLUGIN_DEFS


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "olsr_plugin_io.h"
#include "olsr_types.h"

#define PLUGIN_NAME    "OLSRD dynamic gateway plugin"
#define PLUGIN_VERSION "0.4"
#define PLUGIN_AUTHOR   "Various artists"
#define MOD_DESC PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_AUTHOR
#define PLUGIN_INTERFACE_VERSION 2

/****************************************************************************
 *                Function pointers to functions in olsrd                   *
 *              These allow direct access to olsrd functions                *
 ****************************************************************************/

/* The multi-purpose funtion. All other functions are fetched trough this */
int (*olsr_plugin_io)(int, void *, size_t);

/* Register a scheduled event */
int (*olsr_register_scheduler_event)(void (*)(void), void*, float, float, olsr_u8_t *);

/* olsrd printf wrapper */
int (*olsr_printf)(int, char *, ...);

/* Add hna net IPv4 */
void (*add_local_hna4_entry)(union olsr_ip_addr *, union hna_netmask *);

/* Remove hna net IPv4 */
int (*remove_local_hna4_entry)(union olsr_ip_addr *, union hna_netmask *);


/****************************************************************************
 *                             Data from olsrd                              *
 *           NOTE THAT POINTERS POINT TO THE DATA USED BY OLSRD!            *
 *               NEVER ALTER DATA POINTED TO BY THESE POINTERS              * 
 *                   UNLESS YOU KNOW WHAT YOU ARE DOING!!!                  *
 ****************************************************************************/

/* These two are set automatically by olsrd at load time */
int                ipversion;  /* IPversion in use */
union olsr_ip_addr *main_addr; /* Main address */


size_t             ipsize;     /* Size of the ipadresses used */
struct timeval     *now;       /* the olsrds schedulers idea of current time */


/****************************************************************************
 *                Functions that the plugin MUST provide                    *
 ****************************************************************************/


/* Initialization function */
int
olsr_plugin_init(void);

/* Destructor function */
void
olsr_plugin_exit(void);

/* Mulitpurpose funtion */
int
plugin_io(int, void *, size_t);

int
register_olsr_param(char *, char *);

int 
get_plugin_interface_version(void);

#endif
