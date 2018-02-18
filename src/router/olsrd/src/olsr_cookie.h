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

#include "olsr_types.h"
#include "common/list.h"

#ifndef _OLSR_COOKIE_H
#define _OLSR_COOKIE_H

#define COOKIE_ID_MAX  50       /* maximum number of cookies in the system */

typedef enum olsr_cookie_type_ {
  OLSR_COOKIE_TYPE_MIN,
  OLSR_COOKIE_TYPE_MEMORY,
  OLSR_COOKIE_TYPE_TIMER,
  OLSR_COOKIE_TYPE_MAX
} olsr_cookie_type;

/*
 * This is a cookie. A cookie is a tool aimed for olsrd developers.
 * It is used for tracking resource usage in the system and also
 * for locating memory corruption.
 */
struct olsr_cookie_info {
  olsr_cookie_t ci_id;                 /* ID */
  char *ci_name;                       /* Name */
  olsr_cookie_type ci_type;            /* Type of cookie */
  size_t ci_size;                      /* Fixed size for block allocations */
  unsigned int ci_usage;               /* Stats, resource usage */
  unsigned int ci_changes;             /* Stats, resource churn */
  struct list_node ci_free_list;       /* List head for recyclable blocks */
  unsigned int ci_free_list_usage;     /* Length of free list */
};

#define COOKIE_FREE_LIST_THRESHOLD 10   /* Blocks / Percent  */

/*
 * Small brand which gets appended on the end of every block allocation.
 * Helps to detect memory corruption, like overruns, double frees.
 */
struct olsr_cookie_mem_brand {
  char cmb_sig[6];
  olsr_cookie_t cmb_id;
};

/* Externals. */
extern struct olsr_cookie_info *olsr_alloc_cookie(const char *, olsr_cookie_type);
extern void olsr_free_cookie(struct olsr_cookie_info *);
extern void olsr_delete_all_cookies(void);
extern char *olsr_cookie_name(olsr_cookie_t);
extern void olsr_cookie_set_memory_size(struct olsr_cookie_info *, size_t);
extern void olsr_cookie_usage_incr(olsr_cookie_t);
extern void olsr_cookie_usage_decr(olsr_cookie_t);

extern void *olsr_cookie_malloc(struct olsr_cookie_info *);
extern void olsr_cookie_free(struct olsr_cookie_info *, void *);

#endif /* _OLSR_COOKIE_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
