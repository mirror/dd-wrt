
/*
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Dynamic linked library example for UniK OLSRd
 */

#ifndef _OLSRD_PLUGIN_TEST
#define _OLSRD_PLUGIN_TEST

#include "olsrd_plugin.h"

#define IPC_PORT           8888

#define EMISSION_INTERVAL  2.5
/* Database entry */

struct pwrentry
{
  union olsr_ip_addr originator;  /* IP address of the node this entry describes */
  olsr_u8_t          source_type; /* Powersource of the node */
  olsr_u8_t          percentage;  /* Percentage of battery power left */
  olsr_u16_t         time_left;   /* Operation time left on battery */
  struct timeval     timer;       /* Validity time */
  struct pwrentry    *next;       /* Next element in line */
  struct pwrentry    *prev;       /* Previous elemnt in line */
};


/* The database - (using hashing) */
struct pwrentry list[HASHSIZE];


int has_apm;

/* set buffer to size of IPv6 message */
static char buffer[sizeof(struct olsrmsg6)];


/* Timeout function to register with the sceduler */
void
olsr_timeout(void);

/* Parser function to register with the sceduler */
void
olsr_parser(union olsr_message *, struct interface *, union olsr_ip_addr *);

/* Event function to register with the sceduler */
void
olsr_event(void *);

void
ipc_action(int fd);

int
get_powerstatus(struct powermsg *);

int
update_power_entry(union olsr_ip_addr *, struct powermsg *, double);

void
print_power_table(void);

#endif
