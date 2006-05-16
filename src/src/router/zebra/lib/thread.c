/* Thread management routine
 * Copyright (C) 1998, 2000 Kunihiro Ishiguro <kunihiro@zebra.org>
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

/* #define DEBUG */

#include <zebra.h>

#include "thread.h"
#include "memory.h"
#include "log.h"

#ifdef DEBUG
void thread_master_debug (struct thread_master *);
#endif /* DEBUG */

/* Thread types. */
#define THREAD_READ  0
#define THREAD_WRITE 1
#define THREAD_TIMER 2
#define THREAD_EVENT 3
#define THREAD_READY 4
#define THREAD_UNUSED 5

/* Make thread master. */
struct thread_master *
thread_make_master ()
{
  struct thread_master *new;

  new = XMALLOC (MTYPE_THREAD_MASTER, sizeof (struct thread_master));
  bzero (new, sizeof (struct thread_master));

  return new;
}

/* Add a new thread to the list. */
static void
thread_list_add (struct thread_list *list, struct thread *thread)
{
  thread->next = NULL;
  thread->prev = list->tail;
  if (list->tail)
    list->tail->next = thread;
  else
    list->head = thread;
  list->tail = thread;
  list->count++;
}

/* Add a new thread to the list. */
void
thread_list_add_before (struct thread_list *list, 
			struct thread *point, 
			struct thread *thread)
{
  thread->next = point;
  thread->prev = point->prev;
  if (point->prev)
    point->prev->next = thread;
  else
    list->head = thread;
  point->prev = thread;
  list->count++;
}

/* Delete a thread from the list. */
struct thread *
thread_list_delete (struct thread_list *list, struct thread *thread)
{
  if (thread->next)
    thread->next->prev = thread->prev;
  else
    list->tail = thread->prev;
  if (thread->prev)
    thread->prev->next = thread->next;
  else
    list->head = thread->next;
  thread->next = thread->prev = NULL;
  list->count--;
  return thread;
}

/* Free all unused thread. */
static void
thread_clean_unuse (struct thread_master *m)
{
  struct thread *thread;

  thread = m->unuse.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->unuse, t);
      XFREE (MTYPE_THREAD, t);
      m->alloc--;
    }
}

/* Move thread to unuse list. */
static void
thread_add_unuse (struct thread_master *m, struct thread *thread)
{
  assert (m != NULL);
  assert (thread->next == NULL);
  assert (thread->prev == NULL);
  assert (thread->type == THREAD_UNUSED);
  thread_list_add (&m->unuse, thread);
}

/* Stop thread scheduler. */
void
thread_destroy_master (struct thread_master *m)
{
  struct thread *thread;

  thread = m->read.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->read, t);
      t->type = THREAD_UNUSED;
      thread_add_unuse (m, t);
    }

  thread = m->write.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->write, t);
      t->type = THREAD_UNUSED;
      thread_add_unuse (m, t);
    }

  thread = m->timer.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->timer, t);
      t->type = THREAD_UNUSED;
      thread_add_unuse (m, t);
    }

  thread = m->event.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->event, t);
      t->type = THREAD_UNUSED;
      thread_add_unuse (m, t);
    }

  thread = m->ready.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      thread_list_delete (&m->ready, t);
      t->type = THREAD_UNUSED;
      thread_add_unuse (m, t);
    }

  thread_clean_unuse (m);
  XFREE (MTYPE_THREAD_MASTER, m);
}

/* Delete top of the list and return it. */
struct thread *
thread_trim_head (struct thread_list *list)
{
  if (list->head)
    return thread_list_delete (list, list->head);
  return NULL;
}

/* Make new thread. */
struct thread *
thread_new (struct thread_master *m)
{
  struct thread *new;

  if (m->unuse.head)
    return (thread_trim_head (&m->unuse));

  new = XMALLOC (MTYPE_THREAD, sizeof (struct thread));
  bzero (new, sizeof (struct thread));
  m->alloc++;
  return new;
}

/* Add new read thread. */
struct thread *
thread_add_read (struct thread_master *m, 
		 int (*func)(struct thread *),
		 void *arg,
		 int fd)
{
  struct thread *thread;

  assert (m != NULL);

  if (FD_ISSET (fd, &m->readfd))
    {
      zlog (NULL, LOG_WARNING, "There is already read fd [%d]", fd);
      return NULL;
    }

  thread = thread_new (m);
  thread->type = THREAD_READ;
  thread->id = 0;
  thread->master = m;
  thread->func = func;
  thread->arg = arg;
  FD_SET (fd, &m->readfd);
  thread->u.fd = fd;
  thread_list_add (&m->read, thread);

  return thread;
}

/* Add new write thread. */
struct thread *
thread_add_write (struct thread_master *m,
		 int (*func)(struct thread *),
		 void *arg,
		 int fd)
{
  struct thread *thread;

  assert (m != NULL);

  if (FD_ISSET (fd, &m->writefd))
    {
      zlog (NULL, LOG_WARNING, "There is already write fd [%d]", fd);
      return NULL;
    }

  thread = thread_new (m);
  thread->type = THREAD_WRITE;
  thread->id = 0;
  thread->master = m;
  thread->func = func;
  thread->arg = arg;
  FD_SET (fd, &m->writefd);
  thread->u.fd = fd;
  thread_list_add (&m->write, thread);

  return thread;
}

/* timer compare */
static int
thread_timer_cmp (struct timeval a, struct timeval b)
{
  if (a.tv_sec > b.tv_sec) 
    return 1;
  if (a.tv_sec < b.tv_sec)
    return -1;
  if (a.tv_usec > b.tv_usec)
    return 1;
  if (a.tv_usec < b.tv_usec)
    return -1;
  return 0;
}

/* Add timer event thread. */
struct thread *
thread_add_timer (struct thread_master *m,
		  int (*func)(struct thread *),
		  void *arg,
		  long timer)
{
  struct timeval timer_now;
  struct thread *thread;
#ifndef TIMER_NO_SORT
  struct thread *tt;
#endif /* TIMER_NO_SORT */

  assert (m != NULL);

  thread = thread_new (m);
  thread->type = THREAD_TIMER;
  thread->id = 0;
  thread->master = m;
  thread->func = func;
  thread->arg = arg;

  /* Do we need jitter here? */
  gettimeofday (&timer_now, NULL);
  timer_now.tv_sec += timer;
  thread->u.sands = timer_now;

  /* Sort by timeval. */
#ifdef TIMER_NO_SORT
  thread_list_add (&m->timer, thread);
#else
  for (tt = m->timer.head; tt; tt = tt->next)
    if (thread_timer_cmp (thread->u.sands, tt->u.sands) <= 0)
      break;

  if (tt)
    thread_list_add_before (&m->timer, tt, thread);
  else
    thread_list_add (&m->timer, thread);
#endif /* TIMER_NO_SORT */

  return thread;
}

/* Add simple event thread. */
struct thread *
thread_add_event (struct thread_master *m,
		  int (*func)(struct thread *), 
		  void *arg,
		  int val)
{
  struct thread *thread;

  assert (m != NULL);

  thread = thread_new (m);
  thread->type = THREAD_EVENT;
  thread->id = 0;
  thread->master = m;
  thread->func = func;
  thread->arg = arg;
  thread->u.val = val;
  thread_list_add (&m->event, thread);

  return thread;
}

/* Cancel thread from scheduler. */
void
thread_cancel (struct thread *thread)
{
  /**/
  switch (thread->type)
    {
    case THREAD_READ:
      assert (FD_ISSET (thread->u.fd, &thread->master->readfd));
      FD_CLR (thread->u.fd, &thread->master->readfd);
      thread_list_delete (&thread->master->read, thread);
      break;
    case THREAD_WRITE:
      assert (FD_ISSET (thread->u.fd, &thread->master->writefd));
      FD_CLR (thread->u.fd, &thread->master->writefd);
      thread_list_delete (&thread->master->write, thread);
      break;
    case THREAD_TIMER:
      thread_list_delete (&thread->master->timer, thread);
      break;
    case THREAD_EVENT:
      thread_list_delete (&thread->master->event, thread);
      break;
    case THREAD_READY:
      thread_list_delete (&thread->master->ready, thread);
      break;
    default:
      break;
    }
  thread->type = THREAD_UNUSED;
  thread_add_unuse (thread->master, thread);

#ifdef DEBUG
  thread_master_debug (thread->master);
#endif /* DEBUG */
}

/* Delete all events which has argument value arg. */
void
thread_cancel_event (struct thread_master *m, void *arg)
{
  struct thread *thread;

  thread = m->event.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      if (t->arg == arg)
	{
	  thread_list_delete (&m->event, t);
	  t->type = THREAD_UNUSED;
	  thread_add_unuse (m, t);
	}
    }
}

/* for struct timeval */
#define TIMER_SEC_MICRO 1000000

/* timer sub */
struct timeval
thread_timer_sub (struct timeval a, struct timeval b)
{
  struct timeval ret;

  ret.tv_usec = a.tv_usec - b.tv_usec;
  ret.tv_sec = a.tv_sec - b.tv_sec;

  if (ret.tv_usec < 0) {
    ret.tv_usec += TIMER_SEC_MICRO;
    ret.tv_sec--;
  }

  return ret;
}

/* For debug use. */
void
thread_timer_dump (struct timeval tv)
{
  printf ("Timer : %ld:%ld\n", (long int) tv.tv_sec, (long int) tv.tv_usec);
}

/* Fetch next ready thread. */
struct thread *
thread_fetch (struct thread_master *m, struct thread *fetch)
{
  int ret;
  struct thread *thread;
  fd_set readfd;
  fd_set writefd;
  fd_set exceptfd;
  struct timeval timer_now;
  struct timeval timer_min;
  struct timeval *timer_wait;

  assert (m != NULL);

 retry:  /* When thread can't fetch try to find next thread again. */

  /* If there is event process it first. */
  while ((thread = thread_trim_head (&m->event)))
    {
      *fetch = *thread;
      thread->type = THREAD_UNUSED;
      thread_add_unuse (m, thread);
      return fetch;
    }

  /* If there is ready threads process them */
  while ((thread = thread_trim_head (&m->ready)))
    {
      *fetch = *thread;
      thread->type = THREAD_UNUSED;
      thread_add_unuse (m, thread);
      return fetch;
    }

  /* Calculate select wait timer. */
#ifdef TIMER_NO_SORT
  gettimeofday (&timer_now, NULL);

  timer_wait = NULL;
  for (thread = m->timer.head; thread; thread = thread->next)
    {
      if (! timer_wait)
	timer_wait = &thread->u.sands;
      else if (thread_timer_cmp (thread->u.sands, *timer_wait) < 0)
	timer_wait = &thread->u.sands;
    }

  if (m->timer.head)
    {
      timer_min = *timer_wait;
      timer_min = thread_timer_sub (timer_min, timer_now);
      if (timer_min.tv_sec < 0)
	{
	  timer_min.tv_sec = 0;
	  timer_min.tv_usec = 10;
	}
      timer_wait = &timer_min;
    }
  else
    {
      timer_wait = NULL;
    }
#else
  if (m->timer.head)
    {
      gettimeofday (&timer_now, NULL);
      timer_min = m->timer.head->u.sands;
      timer_min = thread_timer_sub (timer_min, timer_now);
      if (timer_min.tv_sec < 0)
	{
	  timer_min.tv_sec = 0;
	  timer_min.tv_usec = 10;
	}
      timer_wait = &timer_min;
    }
  else
    {
      timer_wait = NULL;
    }
#endif /* TIMER_NO_SORT */

  /* Call select function. */
  readfd = m->readfd;
  writefd = m->writefd;
  exceptfd = m->exceptfd;

#ifdef DEBUG
  {
    int i;
    printf ("readfd : ");
    for (i = 0; i < FD_SETSIZE; i++)
      if (FD_ISSET (i, &readfd))
	printf ("[%d] ", i);
    printf ("\n");

  }
  {
    struct thread *t;

    printf ("readms : ");
    for (t = m->read.head; t; t = t->next)
      printf ("[%d] ", t->u.fd);
    printf ("\n");
  }
#endif /* DEBUG */

  ret = select (FD_SETSIZE, &readfd, &writefd, &exceptfd, timer_wait);
  if (ret < 0)
    {
      if (errno != EINTR)
	{
	  /* Real error. */
	  zlog_warn ("select error: %s", strerror (errno));
	  assert (0);
	}
      /* Signal is coming. */
      goto retry;
    }

#ifdef DEBUG
  {
    int i;
    printf ("after select readfd : ");
    for (i = 0; i < FD_SETSIZE; i++)
      if (FD_ISSET (i, &readfd))
	printf ("[%d] ", i);
    printf ("\n");
  }
#endif /* DEBUG */

  /* Read thead. */
  thread = m->read.head;
  while (thread)
    {
      struct thread *t;
      
      t = thread;
      thread = t->next;

      if (FD_ISSET (t->u.fd, &readfd))
	{
	  assert (FD_ISSET (t->u.fd, &m->readfd));
	  FD_CLR(t->u.fd, &m->readfd);
	  thread_list_delete (&m->read, t);
	  thread_list_add (&m->ready, t);
	  t->type = THREAD_READY;
	}
    }
#ifdef DEBUG
  {
    struct thread *t;

    printf ("readms : ");
    for (t = m->read.head; t; t = t->next)
      printf ("[%d] ", t->u.fd);
    printf ("\n");
  }
#endif /* DEBUG */      

  /* Write thead. */
  thread = m->write.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      if (FD_ISSET (t->u.fd, &writefd))
	{
	  assert (FD_ISSET (t->u.fd, &m->writefd));
	  FD_CLR(t->u.fd, &m->writefd);
	  thread_list_delete (&m->write, t);
	  thread_list_add (&m->ready, t);
	  t->type = THREAD_READY;
	}
    }

  /* Exception thead. */
  /*...*/

  /* Timer update. */
  gettimeofday (&timer_now, NULL);

  thread = m->timer.head;
  while (thread)
    {
      struct thread *t;

      t = thread;
      thread = t->next;

      if (thread_timer_cmp (timer_now, t->u.sands) >= 0)
	{
	  thread_list_delete (&m->timer, t);
	  thread_list_add (&m->ready, t);
	  t->type = THREAD_READY;
	}
    }

  /* Return one event. */
  thread = thread_trim_head (&m->ready);

  /* There is no ready thread. */
  if (!thread)
    goto retry;

  *fetch = *thread;
  thread->type = THREAD_UNUSED;
  thread_add_unuse (m, thread);
#ifdef DEBUG
  thread_master_debug (m);
#endif /* DEBUG */
  
  return fetch;
}

/* List allocation and head/tail print out. */
void
thread_list_debug (struct thread_list *list)
{
  printf ("count [%d] head [%p] tail [%p]\n",
	  list->count, list->head, list->tail);
}

/* Debug print for thread_master. */
void
thread_master_debug (struct thread_master *m)
{
  printf ("-----------\n");
  printf ("readlist  : ");
  thread_list_debug (&m->read);
  printf ("writelist : ");
  thread_list_debug (&m->write);
  printf ("timerlist : ");
  thread_list_debug (&m->timer);
  printf ("eventlist : ");
  thread_list_debug (&m->event);
  printf ("unuselist : ");
  thread_list_debug (&m->unuse);
  printf ("total alloc: [%ld]\n", m->alloc);
  printf ("-----------\n");
}

/* Debug print for thread. */
void
thread_debug (struct thread *thread)
{
  printf ("Thread: ID [%ld] Type [%d] Next [%p]"
	  "Prev [%p] Func [%p] arg [%p] fd [%d]\n", 
	  thread->id, thread->type, thread->next,
	  thread->prev, thread->func, thread->arg, thread->u.fd);
}

/* Make unique thread id for non pthread version of thread manager. */
unsigned long int
thread_get_id ()
{
  static unsigned long int counter = 0;
  return ++counter;
}

/* Call thread ! */
void
thread_call (struct thread *thread)
{
  thread->id = thread_get_id ();
  (*thread->func) (thread);
}

/* Execute thread */
struct thread *
thread_execute (struct thread_master *m,
                int (*func)(struct thread *), 
                void *arg,
                int val)
{
  struct thread dummy; 

  memset (&dummy, 0, sizeof (struct thread));

  dummy.type = THREAD_EVENT;
  dummy.id = 0;
  dummy.master = (struct thread_master *)NULL;
  dummy.func = func;
  dummy.arg = arg;
  dummy.u.val = val;
  thread_call (&dummy);     /* execute immediately */

  return (struct thread *)NULL;
}
