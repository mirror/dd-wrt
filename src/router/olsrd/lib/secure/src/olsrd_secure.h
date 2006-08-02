/*
 * Secure OLSR plugin
 * http://www.olsr.org
 *
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following 
 * conditions are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsrd, olsr.org nor the names of its 
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
 *POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Dynamic linked library example for UniK OLSRd
 */

#ifndef _OLSRD_PLUGIN_TEST
#define _OLSRD_PLUGIN_TEST

#include "secure_messages.h"

#include "hashing.h"


#define KEYFILE "/root/.olsr/olsrd_secure_key"

/* Schemes */
#define ONE_CHECKSUM          1

/* Algorithm definitions */
#define SHA1_INCLUDING_KEY   1
#define MD5_INCLUDING_KEY   2

#ifdef USE_OPENSSL
#define SIGNATURE_SIZE 20
#else
#define SIGNATURE_SIZE 16
#endif

#define KEYLENGTH      16

#define UPPER_DIFF 3
#define LOWER_DIFF -3

char aes_key[16];
/* Seconds of slack allowed */
#define SLACK 3

/* Timestamp node */
struct stamp
{
  union olsr_ip_addr addr;
  /* Timestamp difference */
  int diff;
  olsr_u32_t challenge;
  olsr_u8_t validated;
  clock_t valtime; /* Validity time */
  clock_t conftime; /* Reconfiguration time */
  struct stamp *prev;
  struct stamp *next;
};

/* Seconds to cache a valid timestamp entry */
#define TIMESTAMP_HOLD_TIME 30
/* Seconds to cache a not verified timestamp entry */
#define EXCHANGE_HOLD_TIME 5

struct stamp timestamps[HASHSIZE];

char checksum_cache[512 + KEYLENGTH];

/* Input interface */
struct interface *olsr_in_if;

/* Timeout function to register with the sceduler */
void
olsr_timeout(void);


/* Event function to register with the sceduler */
void
olsr_event(void);

int
send_challenge(union olsr_ip_addr *);

int
ifchange(struct interface *, int);

int
send_cres(union olsr_ip_addr *, union olsr_ip_addr *, olsr_u32_t, struct stamp *);

int
send_rres(union olsr_ip_addr *, union olsr_ip_addr *, olsr_u32_t);

int
parse_challenge(char *);

int
parse_cres(char *);

int
parse_rres(char *);

int
check_auth(char *, int *);

void
ipc_action(int);

int
ipc_send(char *, int);

int
add_signature(char *, int*);

int
validate_packet(char *, int*);

void
packet_parser(int);

void
timeout_timestamps(void*);

int
check_timestamp(union olsr_ip_addr *, time_t);

struct stamp *
lookup_timestamp_entry(union olsr_ip_addr *);

int
read_key_from_file(char *);

int
secure_plugin_init(void);

void
secure_plugin_exit(void);

int
plugin_ipc_init(void);

#endif
