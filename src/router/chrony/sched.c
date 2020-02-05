/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011, 2013-2016
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  This file contains the scheduling loop and the timeout queue.

  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "sched.h"
#include "memory.h"
#include "util.h"
#include "local.h"
#include "logging.h"

/* ================================================== */

/* Flag indicating that we are initialised */
static int initialised = 0;

/* ================================================== */

/* One more than the highest file descriptor that is registered */
static unsigned int one_highest_fd;

#ifndef FD_SETSIZE
/* If FD_SETSIZE is not defined, assume that fd_set is implemented
   as a fixed size array of bits, possibly embedded inside a record */
#define FD_SETSIZE (sizeof(fd_set) * 8)
#endif

typedef struct {
  SCH_FileHandler       handler;
  SCH_ArbitraryArgument arg;
  int                   events;
} FileHandlerEntry;

static ARR_Instance file_handlers;

/* Timestamp when last select() returned */
static struct timespec last_select_ts, last_select_ts_raw;
static double last_select_ts_err;

/* ================================================== */

/* Variables to handler the timer queue */

typedef struct _TimerQueueEntry
{
  struct _TimerQueueEntry *next; /* Forward and back links in the list */
  struct _TimerQueueEntry *prev;
  struct timespec ts;           /* Local system time at which the
                                   timeout is to expire.  Clearly this
                                   must be in terms of what the
                                   operating system thinks of as
                                   system time, because it will be an
                                   argument to select().  Therefore,
                                   any fudges etc that our local time
                                   driver module would apply to time
                                   that we pass to clients etc doesn't
                                   apply to this. */
  SCH_TimeoutID id;             /* ID to allow client to delete
                                   timeout */
  SCH_TimeoutClass class;       /* The class that the epoch is in */
  SCH_TimeoutHandler handler;   /* The handler routine to use */
  SCH_ArbitraryArgument arg;    /* The argument to pass to the handler */

} TimerQueueEntry;

/* The timer queue.  We only use the next and prev entries of this
   record, these chain to the real entries. */
static TimerQueueEntry timer_queue;
static unsigned long n_timer_queue_entries;
static SCH_TimeoutID next_tqe_id;

/* Pointer to head of free list */
static TimerQueueEntry *tqe_free_list = NULL;

/* Timestamp when was last timeout dispatched for each class */
static struct timespec last_class_dispatch[SCH_NumberOfClasses];

/* ================================================== */

static int need_to_exit;

/* ================================================== */

static void
handle_slew(struct timespec *raw,
            struct timespec *cooked,
            double dfreq,
            double doffset,
            LCL_ChangeType change_type,
            void *anything);

/* ================================================== */

void
SCH_Initialise(void)
{
  file_handlers = ARR_CreateInstance(sizeof (FileHandlerEntry));

  n_timer_queue_entries = 0;
  next_tqe_id = 0;

  timer_queue.next = &timer_queue;
  timer_queue.prev = &timer_queue;

  need_to_exit = 0;

  LCL_AddParameterChangeHandler(handle_slew, NULL);

  LCL_ReadRawTime(&last_select_ts_raw);
  last_select_ts = last_select_ts_raw;

  initialised = 1;
}


/* ================================================== */

void
SCH_Finalise(void) {
  ARR_DestroyInstance(file_handlers);

  initialised = 0;
}

/* ================================================== */

void
SCH_AddFileHandler
(int fd, int events, SCH_FileHandler handler, SCH_ArbitraryArgument arg)
{
  FileHandlerEntry *ptr;

  assert(initialised);
  assert(events);
  assert(fd >= 0);
  
  if (fd >= FD_SETSIZE)
    LOG_FATAL("Too many file descriptors");

  /* Resize the array if the descriptor is highest so far */
  while (ARR_GetSize(file_handlers) <= fd) {
    ptr = ARR_GetNewElement(file_handlers);
    ptr->handler = NULL;
    ptr->arg = NULL;
    ptr->events = 0;
  }

  ptr = ARR_GetElement(file_handlers, fd);

  /* Don't want to allow the same fd to register a handler more than
     once without deleting a previous association - this suggests
     a bug somewhere else in the program. */
  assert(!ptr->handler);

  ptr->handler = handler;
  ptr->arg = arg;
  ptr->events = events;

  if (one_highest_fd < fd + 1)
    one_highest_fd = fd + 1;
}


/* ================================================== */

void
SCH_RemoveFileHandler(int fd)
{
  FileHandlerEntry *ptr;

  assert(initialised);

  ptr = ARR_GetElement(file_handlers, fd);

  /* Check that a handler was registered for the fd in question */
  assert(ptr->handler);

  ptr->handler = NULL;
  ptr->arg = NULL;
  ptr->events = 0;

  /* Find new highest file descriptor */
  while (one_highest_fd > 0) {
    ptr = ARR_GetElement(file_handlers, one_highest_fd - 1);
    if (ptr->handler)
      break;
    one_highest_fd--;
  }
}

/* ================================================== */

void
SCH_SetFileHandlerEvent(int fd, int event, int enable)
{
  FileHandlerEntry *ptr;

  ptr = ARR_GetElement(file_handlers, fd);

  if (enable)
    ptr->events |= event;
  else
    ptr->events &= ~event;
}

/* ================================================== */

void
SCH_GetLastEventTime(struct timespec *cooked, double *err, struct timespec *raw)
{
  if (cooked) {
    *cooked = last_select_ts;
    if (err)
      *err = last_select_ts_err;
  }
  if (raw)
    *raw = last_select_ts_raw;
}

/* ================================================== */

#define TQE_ALLOC_QUANTUM 32

static TimerQueueEntry *
allocate_tqe(void)
{
  TimerQueueEntry *new_block;
  TimerQueueEntry *result;
  int i;
  if (tqe_free_list == NULL) {
    new_block = MallocArray(TimerQueueEntry, TQE_ALLOC_QUANTUM);
    for (i=1; i<TQE_ALLOC_QUANTUM; i++) {
      new_block[i].next = &(new_block[i-1]);
    }
    new_block[0].next = NULL;
    tqe_free_list = &(new_block[TQE_ALLOC_QUANTUM - 1]);
  }

  result = tqe_free_list;
  tqe_free_list = tqe_free_list->next;
  return result;
}

/* ================================================== */

static void
release_tqe(TimerQueueEntry *node)
{
  node->next = tqe_free_list;
  tqe_free_list = node;
}

/* ================================================== */

static SCH_TimeoutID
get_new_tqe_id(void)
{
  TimerQueueEntry *ptr;

try_again:
  next_tqe_id++;
  if (!next_tqe_id)
    goto try_again;

  /* Make sure the ID isn't already used */
  for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next)
    if (ptr->id == next_tqe_id)
      goto try_again;

  return next_tqe_id;
}

/* ================================================== */

SCH_TimeoutID
SCH_AddTimeout(struct timespec *ts, SCH_TimeoutHandler handler, SCH_ArbitraryArgument arg)
{
  TimerQueueEntry *new_tqe;
  TimerQueueEntry *ptr;

  assert(initialised);

  new_tqe = allocate_tqe();

  new_tqe->id = get_new_tqe_id();
  new_tqe->handler = handler;
  new_tqe->arg = arg;
  new_tqe->ts = *ts;
  new_tqe->class = SCH_ReservedTimeoutValue;

  /* Now work out where to insert the new entry in the list */
  for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next) {
    if (UTI_CompareTimespecs(&new_tqe->ts, &ptr->ts) == -1) {
      /* If the new entry comes before the current pointer location in
         the list, we want to insert the new entry just before ptr. */
      break;
    }
  }

  /* At this stage, we want to insert the new entry immediately before
     the entry identified by 'ptr' */

  new_tqe->next = ptr;
  new_tqe->prev = ptr->prev;
  ptr->prev->next = new_tqe;
  ptr->prev = new_tqe;

  n_timer_queue_entries++;

  return new_tqe->id;
}

/* ================================================== */
/* This queues a timeout to elapse at a given delta time relative to
   the current (raw) time */

SCH_TimeoutID
SCH_AddTimeoutByDelay(double delay, SCH_TimeoutHandler handler, SCH_ArbitraryArgument arg)
{
  struct timespec now, then;

  assert(initialised);
  assert(delay >= 0.0);

  LCL_ReadRawTime(&now);
  UTI_AddDoubleToTimespec(&now, delay, &then);
  if (UTI_CompareTimespecs(&now, &then) > 0) {
    LOG_FATAL("Timeout overflow");
  }

  return SCH_AddTimeout(&then, handler, arg);

}

/* ================================================== */

SCH_TimeoutID
SCH_AddTimeoutInClass(double min_delay, double separation, double randomness,
                      SCH_TimeoutClass class,
                      SCH_TimeoutHandler handler, SCH_ArbitraryArgument arg)
{
  TimerQueueEntry *new_tqe;
  TimerQueueEntry *ptr;
  struct timespec now;
  double diff, r;
  double new_min_delay;

  assert(initialised);
  assert(min_delay >= 0.0);
  assert(class < SCH_NumberOfClasses);

  if (randomness > 0.0) {
    uint32_t rnd;

    UTI_GetRandomBytes(&rnd, sizeof (rnd));
    r = rnd * (randomness / (uint32_t)-1) + 1.0;
    min_delay *= r;
    separation *= r;
  }
  
  LCL_ReadRawTime(&now);
  new_min_delay = min_delay;

  /* Check the separation from the last dispatched timeout */
  diff = UTI_DiffTimespecsToDouble(&now, &last_class_dispatch[class]);
  if (diff < separation && diff >= 0.0 && diff + new_min_delay < separation) {
    new_min_delay = separation - diff;
  }

  /* Scan through list for entries in the same class and increase min_delay
     if necessary to keep at least the separation away */
  for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next) {
    if (ptr->class == class) {
      diff = UTI_DiffTimespecsToDouble(&ptr->ts, &now);
      if (new_min_delay > diff) {
        if (new_min_delay - diff < separation) {
          new_min_delay = diff + separation;
        }
      } else {
        if (diff - new_min_delay < separation) {
          new_min_delay = diff + separation;
        }
      }
    }
  }

  for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next) {
    diff = UTI_DiffTimespecsToDouble(&ptr->ts, &now);
    if (diff > new_min_delay) {
      break;
    }
  }

  /* We have located the insertion point */
  new_tqe = allocate_tqe();

  new_tqe->id = get_new_tqe_id();
  new_tqe->handler = handler;
  new_tqe->arg = arg;
  UTI_AddDoubleToTimespec(&now, new_min_delay, &new_tqe->ts);
  new_tqe->class = class;

  new_tqe->next = ptr;
  new_tqe->prev = ptr->prev;
  ptr->prev->next = new_tqe;
  ptr->prev = new_tqe;
  n_timer_queue_entries++;

  return new_tqe->id;
}

/* ================================================== */

void
SCH_RemoveTimeout(SCH_TimeoutID id)
{
  TimerQueueEntry *ptr;

  assert(initialised);

  if (!id)
    return;

  for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next) {

    if (ptr->id == id) {
      /* Found the required entry */
      
      /* Unlink from the queue */
      ptr->next->prev = ptr->prev;
      ptr->prev->next = ptr->next;
      
      /* Decrement entry count */
      --n_timer_queue_entries;
      
      /* Release memory back to the operating system */
      release_tqe(ptr);

      return;
    }
  }

  /* Catch calls with invalid non-zero ID */
  assert(0);
}

/* ================================================== */
/* Try to dispatch any timeouts that have already gone by, and
   keep going until all are done.  (The earlier ones may take so
   long to do that the later ones come around by the time they are
   completed). */

static void
dispatch_timeouts(struct timespec *now) {
  TimerQueueEntry *ptr;
  SCH_TimeoutHandler handler;
  SCH_ArbitraryArgument arg;
  int n_done = 0, n_entries_on_start = n_timer_queue_entries;

  while (1) {
    LCL_ReadRawTime(now);

    if (!(n_timer_queue_entries > 0 &&
          UTI_CompareTimespecs(now, &timer_queue.next->ts) >= 0)) {
      break;
    }

    ptr = timer_queue.next;

    last_class_dispatch[ptr->class] = *now;

    handler = ptr->handler;
    arg = ptr->arg;

    SCH_RemoveTimeout(ptr->id);

    /* Dispatch the handler */
    (handler)(arg);

    /* Increment count of timeouts handled */
    ++n_done;

    /* If more timeouts were handled than there were in the timer queue on
       start and there are now, assume some code is scheduling timeouts with
       negative delays and abort.  Make the actual limit higher in case the
       machine is temporarily overloaded and dispatching the handlers takes
       more time than was delay of a scheduled timeout. */
    if (n_done > n_timer_queue_entries * 4 &&
        n_done > n_entries_on_start * 4) {
      LOG_FATAL("Possible infinite loop in scheduling");
    }
  }
}

/* ================================================== */

/* nfd is the number of bits set in all fd_sets */

static void
dispatch_filehandlers(int nfd, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
  FileHandlerEntry *ptr;
  int fd;
  
  for (fd = 0; nfd && fd < one_highest_fd; fd++) {
    if (except_fds && FD_ISSET(fd, except_fds)) {
      /* This descriptor has an exception, dispatch its handler */
      ptr = (FileHandlerEntry *)ARR_GetElement(file_handlers, fd);
      if (ptr->handler)
        (ptr->handler)(fd, SCH_FILE_EXCEPTION, ptr->arg);
      nfd--;

      /* Don't try to read from it now */
      if (read_fds && FD_ISSET(fd, read_fds)) {
        FD_CLR(fd, read_fds);
        nfd--;
      }
    }

    if (read_fds && FD_ISSET(fd, read_fds)) {
      /* This descriptor can be read from, dispatch its handler */
      ptr = (FileHandlerEntry *)ARR_GetElement(file_handlers, fd);
      if (ptr->handler)
        (ptr->handler)(fd, SCH_FILE_INPUT, ptr->arg);
      nfd--;
    }

    if (write_fds && FD_ISSET(fd, write_fds)) {
      /* This descriptor can be written to, dispatch its handler */
      ptr = (FileHandlerEntry *)ARR_GetElement(file_handlers, fd);
      if (ptr->handler)
        (ptr->handler)(fd, SCH_FILE_OUTPUT, ptr->arg);
      nfd--;
    }
  }
}

/* ================================================== */

static void
handle_slew(struct timespec *raw,
            struct timespec *cooked,
            double dfreq,
            double doffset,
            LCL_ChangeType change_type,
            void *anything)
{
  TimerQueueEntry *ptr;
  double delta;
  int i;

  if (change_type != LCL_ChangeAdjust) {
    /* Make sure this handler is invoked first in order to not shift new timers
       added from other handlers */
    assert(LCL_IsFirstParameterChangeHandler(handle_slew));

    /* If a step change occurs, just shift all raw time stamps by the offset */
    
    for (ptr = timer_queue.next; ptr != &timer_queue; ptr = ptr->next) {
      UTI_AddDoubleToTimespec(&ptr->ts, -doffset, &ptr->ts);
    }

    for (i = 0; i < SCH_NumberOfClasses; i++) {
      UTI_AddDoubleToTimespec(&last_class_dispatch[i], -doffset, &last_class_dispatch[i]);
    }

    UTI_AddDoubleToTimespec(&last_select_ts_raw, -doffset, &last_select_ts_raw);
  }

  UTI_AdjustTimespec(&last_select_ts, cooked, &last_select_ts, &delta, dfreq, doffset);
}

/* ================================================== */

static void
fill_fd_sets(fd_set **read_fds, fd_set **write_fds, fd_set **except_fds)
{
  FileHandlerEntry *handlers;
  fd_set *rd, *wr, *ex;
  int i, n, events;

  n = ARR_GetSize(file_handlers);
  handlers = ARR_GetElements(file_handlers);
  rd = wr = ex = NULL;

  for (i = 0; i < n; i++) {
    events = handlers[i].events;

    if (!events)
      continue;

    if (events & SCH_FILE_INPUT) {
      if (!rd) {
        rd = *read_fds;
        FD_ZERO(rd);
      }
      FD_SET(i, rd);
    }

    if (events & SCH_FILE_OUTPUT) {
      if (!wr) {
        wr = *write_fds;
        FD_ZERO(wr);
      }
      FD_SET(i, wr);
    }

    if (events & SCH_FILE_EXCEPTION) {
      if (!ex) {
        ex = *except_fds;
        FD_ZERO(ex);
      }
      FD_SET(i, ex);
    }
  }

  if (!rd)
    *read_fds = NULL;
  if (!wr)
    *write_fds = NULL;
  if (!ex)
    *except_fds = NULL;
}

/* ================================================== */

#define JUMP_DETECT_THRESHOLD 10

static int
check_current_time(struct timespec *prev_raw, struct timespec *raw, int timeout,
                   struct timeval *orig_select_tv,
                   struct timeval *rem_select_tv)
{
  struct timespec elapsed_min, elapsed_max, orig_select_ts, rem_select_ts;
  double step, elapsed;

  UTI_TimevalToTimespec(orig_select_tv, &orig_select_ts);

  /* Get an estimate of the time spent waiting in the select() call. On some
     systems (e.g. Linux) the timeout timeval is modified to return the
     remaining time, use that information. */
  if (timeout) {
    elapsed_max = elapsed_min = orig_select_ts;
  } else if (rem_select_tv && rem_select_tv->tv_sec >= 0 &&
             rem_select_tv->tv_sec <= orig_select_tv->tv_sec &&
             (rem_select_tv->tv_sec != orig_select_tv->tv_sec ||
              rem_select_tv->tv_usec != orig_select_tv->tv_usec)) {
    UTI_TimevalToTimespec(rem_select_tv, &rem_select_ts);
    UTI_DiffTimespecs(&elapsed_min, &orig_select_ts, &rem_select_ts);
    elapsed_max = elapsed_min;
  } else {
    if (rem_select_tv)
      elapsed_max = orig_select_ts;
    else
      UTI_DiffTimespecs(&elapsed_max, raw, prev_raw);
    UTI_ZeroTimespec(&elapsed_min);
  }

  if (last_select_ts_raw.tv_sec + elapsed_min.tv_sec >
      raw->tv_sec + JUMP_DETECT_THRESHOLD) {
    LOG(LOGS_WARN, "Backward time jump detected!");
  } else if (prev_raw->tv_sec + elapsed_max.tv_sec + JUMP_DETECT_THRESHOLD <
             raw->tv_sec) {
    LOG(LOGS_WARN, "Forward time jump detected!");
  } else {
    return 1;
  }

  step = UTI_DiffTimespecsToDouble(&last_select_ts_raw, raw);
  elapsed = UTI_TimespecToDouble(&elapsed_min);
  step += elapsed;

  /* Cooked time may no longer be valid after dispatching the handlers */
  LCL_NotifyExternalTimeStep(raw, raw, step, fabs(step));

  return 0;
}

/* ================================================== */

void
SCH_MainLoop(void)
{
  fd_set read_fds, write_fds, except_fds;
  fd_set *p_read_fds, *p_write_fds, *p_except_fds;
  int status, errsv;
  struct timeval tv, saved_tv, *ptv;
  struct timespec ts, now, saved_now, cooked;
  double err;

  assert(initialised);

  while (!need_to_exit) {
    /* Dispatch timeouts and fill now with current raw time */
    dispatch_timeouts(&now);
    saved_now = now;
    
    /* The timeout handlers may request quit */
    if (need_to_exit)
      break;

    /* Check whether there is a timeout and set it up */
    if (n_timer_queue_entries > 0) {
      UTI_DiffTimespecs(&ts, &timer_queue.next->ts, &now);
      assert(ts.tv_sec > 0 || ts.tv_nsec > 0);

      UTI_TimespecToTimeval(&ts, &tv);
      ptv = &tv;
      saved_tv = tv;
    } else {
      ptv = NULL;
      saved_tv.tv_sec = saved_tv.tv_usec = 0;
    }

    p_read_fds = &read_fds;
    p_write_fds = &write_fds;
    p_except_fds = &except_fds;
    fill_fd_sets(&p_read_fds, &p_write_fds, &p_except_fds);

    /* if there are no file descriptors being waited on and no
       timeout set, this is clearly ridiculous, so stop the run */
    if (!ptv && !p_read_fds && !p_write_fds)
      LOG_FATAL("Nothing to do");

    status = select(one_highest_fd, p_read_fds, p_write_fds, p_except_fds, ptv);
    errsv = errno;

    LCL_ReadRawTime(&now);
    LCL_CookTime(&now, &cooked, &err);

    /* Check if the time didn't jump unexpectedly */
    if (!check_current_time(&saved_now, &now, status == 0, &saved_tv, ptv)) {
      /* Cook the time again after handling the step */
      LCL_CookTime(&now, &cooked, &err);
    }

    last_select_ts_raw = now;
    last_select_ts = cooked;
    last_select_ts_err = err;

    if (status < 0) {
      if (!need_to_exit && errsv != EINTR) {
        LOG_FATAL("select() failed : %s", strerror(errsv));
      }
    } else if (status > 0) {
      /* A file descriptor is ready for input or output */
      dispatch_filehandlers(status, p_read_fds, p_write_fds, p_except_fds);
    } else {
      /* No descriptors readable, timeout must have elapsed.
       Therefore, tv must be non-null */
      assert(ptv);

      /* There's nothing to do here, since the timeouts
         will be dispatched at the top of the next loop
         cycle */

    }
  }         
}

/* ================================================== */

void
SCH_QuitProgram(void)
{
  need_to_exit = 1;
}

/* ================================================== */

