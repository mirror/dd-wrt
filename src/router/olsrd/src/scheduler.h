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


#ifndef _OLSR_SCHEDULER
#define _OLSR_SCHEDULER

#include "common/list.h"

#include "olsr_types.h"

#include <time.h>

/* Some defs for juggling with timers */
#define MSEC_PER_SEC 1000
#define USEC_PER_SEC 1000000
#define NSEC_PER_USEC 1000
#define USEC_PER_MSEC 1000

#define TIMER_WHEEL_SLOTS 1024
#define TIMER_WHEEL_MASK (TIMER_WHEEL_SLOTS - 1)

typedef void (*timer_cb_func) (void *); /* callback function */

/*
 * Our timer implementation is a based on individual timers arranged in
 * a double linked list hanging of hash containers called a timer wheel slot.
 * For every timer a timer_entry is created and attached to the timer wheel slot.
 * When the timer fires, the timer_cb function is called with the
 * context pointer.
 * The implementation supports periodic and oneshot timers.
 * For a periodic timer the timer_period field is set to non zero,
 * which causes the timer to run forever until manually stopped.
 */
struct timer_entry {
  struct list_node timer_list;         /* Wheel membership */
  uint32_t timer_clock;                /* when timer shall fire (absolute time) */
  unsigned int timer_period;           /* set for periodical timers (relative time) */
  struct olsr_cookie_info *timer_cookie;       /* used for diag stuff */
  uint8_t timer_jitter_pct;            /* the jitter expressed in percent */
  uint8_t timer_flags;                 /* misc flags */
  unsigned int timer_random;           /* cache random() result for performance reasons */
  timer_cb_func timer_cb;              /* callback function */
  void *timer_cb_context;              /* context pointer */
};

/* INLINE to recast from timer_list back to timer_entry */
LISTNODE2STRUCT(list2timer, struct timer_entry, timer_list);

#define OLSR_TIMER_ONESHOT    0 /* One shot timer */
#define OLSR_TIMER_PERIODIC   1 /* Periodic timer */

/* Timer flags */
#define OLSR_TIMER_RUNNING  ( 1u << 0)   /* this timer is running */
#define OLSR_TIMER_REMOVED  ( 1u << 1)   /* this timer is tagged for removal */

/* Timers */
void olsr_init_timers(void);
void olsr_flush_timers(void);
void olsr_set_timer (struct timer_entry **, unsigned int, uint8_t, bool, timer_cb_func, void *, struct olsr_cookie_info *);
struct timer_entry *olsr_start_timer (unsigned int, uint8_t, bool, timer_cb_func, void *, struct olsr_cookie_info *);
void olsr_change_timer(struct timer_entry *, unsigned int, uint8_t, bool);
void olsr_stop_timer (struct timer_entry *);

/* Printing timestamps */
const char *olsr_clock_string(uint32_t);
const char *olsr_wallclock_string(void);

/* Main scheduler loop */
void olsr_scheduler(void);
void olsr_scheduler_stop(void);

/*
 * Provides a timestamp s1 milliseconds in the future
 */
#define GET_TIMESTAMP(s1)	olsr_getTimestamp(s1)

/* Compute the time in milliseconds when a timestamp will expire. */
#define TIME_DUE(s1)    olsr_getTimeDue(s1)

/* Returns TRUE if a timestamp is expired */
#define TIMED_OUT(s1)	  olsr_isTimedOut(s1)

/* Timer data */
extern uint32_t now_times;     /* current idea of times(2) reported uptime */


#define SP_PR_READ		0x01
#define SP_PR_WRITE		0x02

#define SP_IMM_READ		0x04
#define SP_IMM_WRITE		0x08


typedef void (*socket_handler_func) (int fd, void *data, unsigned int flags);


struct olsr_socket_entry {
  int fd;
  socket_handler_func process_immediate;
  socket_handler_func process_pollrate;
  void *data;
  unsigned int flags;
  struct list_node socket_node;
};

LISTNODE2STRUCT(list2socket, struct olsr_socket_entry, socket_node);

/* deletion safe macro for socket list traversal */
#define OLSR_FOR_ALL_SOCKETS(socket) \
{ \
  struct list_node *_socket_node, *_next_socket_node; \
  for (_socket_node = socket_head.next; \
    _socket_node != &socket_head; \
    _socket_node = _next_socket_node) { \
    _next_socket_node = _socket_node->next; \
    socket = list2socket(_socket_node);
#define OLSR_FOR_ALL_SOCKETS_END(socket) }}

uint32_t olsr_times(void);
uint32_t olsr_getTimestamp (uint32_t s);
int32_t olsr_getTimeDue (uint32_t s);
bool olsr_isTimedOut (uint32_t s);

void add_olsr_socket (int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, void *data, unsigned int flags);
int remove_olsr_socket (int fd, socket_handler_func pf_pr, socket_handler_func pf_imm);
void olsr_flush_sockets(void);
void enable_olsr_socket (int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, unsigned int flags);
void disable_olsr_socket (int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, unsigned int flags);

/*
 * a wrapper around times(2). times(2) has the problem, that it may return -1
 * in case of an err (e.g. EFAULT on the parameter) or immediately before an
 * overrun (though it is not en error) just because the jiffies (or whatever
 * the underlying kernel calls the smallest accountable time unit) are
 * inherently "unsigned" (and always incremented).
 */


#endif /* _OLSR_SCHEDULER */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
