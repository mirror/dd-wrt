/*
 * Thread management routine header.
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

#ifndef _ZEBRA_THREAD_H
#define _ZEBRA_THREAD_H

/* Linked list of thread. */
struct thread_list
{
  struct thread *head;
  struct thread *tail;
  int count;
};

/* Master of the theads. */
struct thread_master
{
  struct thread_list read;
  struct thread_list write;
  struct thread_list timer;
  struct thread_list event;
  struct thread_list ready;
  struct thread_list unuse;
  fd_set readfd;
  fd_set writefd;
  fd_set exceptfd;
  unsigned long alloc;
};

/* Thread itself. */
struct thread
{
  unsigned long id;
  unsigned char type;		/* thread type */
  struct thread *next;		/* next pointer of the thread */
  struct thread *prev;		/* previous pointer of the thread */
  struct thread_master *master;	/* pointer to the struct thread_master. */
  int (*func) (struct thread *); /* event function */
  void *arg;			/* event argument */
  union {
    int val;			/* second argument of the event. */
    int fd;			/* file descriptor in case of read/write. */
    struct timeval sands;	/* rest of time sands value. */
  } u;
};

/* Macros. */
#define THREAD_ARG(X) ((X)->arg)
#define THREAD_FD(X)  ((X)->u.fd)
#define THREAD_VAL(X) ((X)->u.val)

/* Prototypes. */
struct thread_master *thread_make_master ();

struct thread *
thread_add_read (struct thread_master *m, 
		 int (*func)(struct thread *),
		 void *arg,
		 int fd);

struct thread *
thread_add_write (struct thread_master *m,
		 int (*func)(struct thread *),
		 void *arg,
		 int fd);

struct thread *
thread_add_timer (struct thread_master *m,
		  int (*func)(struct thread *),
		  void *arg,
		  long timer);

struct thread *
thread_add_event (struct thread_master *m,
		  int (*func)(struct thread *), 
		  void *arg,
		  int val);

void
thread_cancel (struct thread *thread);

void
thread_cancel_event (struct thread_master *m, void *arg);

struct thread *
thread_fetch (struct thread_master *m, 
	      struct thread *fetch);

void
thread_call (struct thread *thread);

struct thread *
thread_execute (struct thread_master *m,
		  int (*func)(struct thread *), 
		  void *arg,
		  int val);

#endif /* _ZEBRA_THREAD_H */
