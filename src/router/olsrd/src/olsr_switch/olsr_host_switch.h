/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#ifndef _OLSR_HOST_SWITCH
#define _OLSR_HOST_SWITCH

#include "olsr_types.h"

#define OHS_TCP_PORT 10150

#define OHS_VERSION "0.1"

#define OHS_DEFAULT_OLSRD_PATH "./olsrd"

#define OHS_OUT_OF_MEMORY(s) do { printf("ohsd: out of memory \"%s\"!\n", s); ohs_close(0); } while (0)

#ifdef _WIN32
int __stdcall ohs_close(unsigned long signal) __attribute__ ((noreturn));
#else /* _WIN32 */
void ohs_close(int) __attribute__ ((noreturn));
#endif /* _WIN32 */

struct ohs_ip_link {
  union olsr_ip_addr dst;
  uint8_t quality;                     /* 0 - 100 */
  struct ohs_ip_link *next;
};

struct ohs_connection {
  union olsr_ip_addr ip_addr;
  int socket;
  uint32_t rx;
  uint32_t tx;
  uint32_t linkcnt;
  struct ohs_ip_link *links;
  struct ohs_connection *next;
};

extern uint32_t logbits;

extern struct ohs_connection *ohs_conns;

#define LOG_DEFAULT 0x0
#define LOG_FORWARD 0x1
#define LOG_CONNECT 0x2
#define LOG_LINK    0x4

#ifdef _WIN32
int __stdcall SignalHandler(unsigned long);
#else /* _WIN32 */
void ohs_close(int);
#endif /* _WIN32 */

struct ohs_connection *get_client_by_addr(const union olsr_ip_addr *);

int ohs_delete_connection(struct ohs_connection *);

#endif /* _OLSR_HOST_SWITCH */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
