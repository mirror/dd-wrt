/*
 * BGP-4 Finite State Machine   
 * From RFC1771 [A Border Gateway Protocol 4 (BGP-4)]
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_BGP_FSM_H
#define _ZEBRA_BGP_FSM_H

/* Macro for BGP read add */
#define BGP_READ_ON(T,F,V) \
do { \
  if (!(T)) \
    (T) = thread_add_read (master, (F), peer, (V)); \
} while (0)

/* Macro for BGP read off. */
#define BGP_READ_OFF(X) \
do { \
  if (X) \
    { \
      thread_cancel (X); \
      (X) = NULL; \
    } \
} while (0)

/* Macro for BGP write add */
#define BGP_WRITE_ON(T,F,V) \
do { \
  if (!(T)) \
    (T) = thread_add_write (master, (F), peer, (V)); \
} while (0)

/* Macro for BGP write turn off. */
#define BGP_WRITE_OFF(X) \
do { \
  if (X) \
    { \
      thread_cancel (X); \
      (X) = NULL; \
    } \
} while (0)

/* Macro for timer turn on. */
#define BGP_TIMER_ON(T,F,V) \
do { \
  if (!(T)) \
    (T) = thread_add_timer (master, (F), peer, (V)); \
} while (0)

/* Macro for timer turn off. */
#define BGP_TIMER_OFF(X) \
do { \
  if (X) \
    { \
      thread_cancel (X); \
      (X) = NULL; \
    } \
} while (0)

#define BGP_EVENT_ADD(P,E) \
    thread_add_event (master, bgp_event, (P), (E))

#define BGP_EVENT_DELETE(P) \
    thread_cancel_event (master, (P))

/* Prototypes. */
int bgp_event (struct thread *);
void bgp_timer_set (struct peer *);
int bgp_stop (struct peer *peer);
void fsm_change_status (struct peer *peer, int status);

#endif /* _ZEBRA_BGP_FSM_H */
