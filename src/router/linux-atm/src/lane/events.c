/*
 * Event handler
 *
 * $Id: events.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

/* Local includes */
#include "events.h"
#include "load.h"
#include "dump.h"
#include "mem.h"
#include "timers.h"
#include "connect.h"

/* Type definitions */
typedef struct _EventList_t {
  const Event_t *event;
  struct _EventList_t *next;
  struct _EventList_t *prev;
} EventList_t;

typedef struct _HFList_t {
  const char *name;
  HandlerFunc_t h;
  void *funcdata;
  struct _HFList_t *next;
} HFList_t;

typedef struct _FDList_t {
  int fd;
  void *data;
  struct _FDList_t *next;
} FDList_t;

/* Local function prototypes */
static void events_init0(void);
static void events_init1(void);
static void events_dump(void);
static void events_release(void);
static void event_get_fds(fd_set *dest);

/* Data */
static const char *rcsid="$Id: events.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $";

const Unit_t events_unit = {
  "events",
  &events_init0,
  &events_init1,
  &events_dump,
  &events_release
};

/*static mutex_t event_mutex;*/
static EventList_t *eventhead, *eventtail;
static HFList_t *handlers[CE_MAX + 1];
static FDList_t *fdlist;

/* Functions */

/* Initialize local data */
static void
events_init0(void)
{
  unsigned int i;

  eventhead = NULL;
  eventtail = NULL;
  for (i = 0; i <= CE_MAX; i++) {
    handlers[i] = NULL;
  }
  fdlist = NULL;
}

/* Initialization for data that needs other units */
static void
events_init1(void)
{
  set_var_str(&events_unit, "version", rcsid);
  Debug_unit(&events_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
events_dump(void)
{
  const EventList_t *tmp;
  const HFList_t *htmp;
  const FDList_t *ftmp;
  unsigned int i;

  for (tmp = eventhead; tmp != NULL; tmp = tmp->next) {
    assert(tmp->event != NULL);
    assert(tmp->event->unit != NULL);
    assert(tmp->event->unit->name != NULL);
    Debug_unit(&events_unit, "unit %s type %s data 0x%x", tmp->event->unit->name, dump_event_type(tmp->event->type), tmp->event->data);
  }
  for (i = 0; i <= CE_MAX; i++) {
    for (htmp = handlers[i]; htmp != NULL; htmp = htmp->next) {
      Debug_unit(&events_unit, "type %s func 0x%x name %s data 0x%x", dump_event_type(i), htmp->h, htmp->name, htmp->funcdata);
    }
  }
  for (ftmp = fdlist; ftmp != NULL; ftmp = ftmp->next) {
    Debug_unit(&events_unit, "polled fd %d", ftmp->fd);
  }
}

/* Release allocated memory, close files etc. */
static void
events_release(void)
{
  unsigned int i;
  HFList_t *htmp;
  FDList_t *ftmp;
  EventList_t *tmp;

  /* Lock list */
  for (tmp = eventhead; tmp != NULL; ) {
    assert(eventhead->event != NULL);
    assert(eventhead->event->unit != NULL);
    assert(eventhead->event->unit->name != NULL);
    Debug_unit(&events_unit, "discarding unread event, unit %s type %s data 0x%x", eventhead->event->unit->name, dump_event_type(eventhead->event->type), eventhead->event->data);
    tmp = eventhead->next;
    mem_free(&events_unit, eventhead->event);
    mem_free(&events_unit, eventhead);
    eventhead = tmp;
  }
  eventtail = NULL;
  /* Unlock list */
  for (i = 0; i <= CE_MAX; i++) {
    for (htmp = handlers[i]; htmp != NULL; htmp = htmp->next) {
      handlers[i] = htmp->next;
      mem_free(&events_unit, htmp);
    }
  }
  for (ftmp = fdlist; ftmp != NULL; ftmp = ftmp->next) {
    printf("Closing:%d\n",ftmp->fd);
    close(ftmp->fd);
    fdlist = ftmp->next;
    mem_free(&events_unit, ftmp);
  }
}

void
add_event_handler(EventType_t type, HandlerFunc_t func, const char *name, void *funcdata)
{
  HFList_t *tmp;

  tmp = (HFList_t *)mem_alloc(&events_unit, sizeof(HFList_t));
  tmp->name = name;
  tmp->h = func;
  tmp->funcdata = funcdata;
  assert(type <= CE_MAX);
  tmp->next = handlers[type];
  
  handlers[type] = tmp;
}

static const char *typetable[CE_MAX + 1] = {
  "New SVC",
  "SVC closed",
  "Data arrived",
  "Timer expired",
  "Dump",
  "Restart",
  "Termination"
};

/* Return event type as string */
const char *
dump_event_type(EventType_t type)
{
  assert(type >= CE_SVC_CLOSE && type <= CE_MAX);
  return typetable[type];
}

/* Get next event from queue */
const Event_t *
event_get_next(void)
{
  const Event_t *event = NULL;
  EventList_t *tmplist;
  const FDList_t *tmp;
  int poll_ret;
  Timer_t *soonest;
  fd_set to_select;
  struct timeval *rolex;

  while (eventtail == NULL && sig_hup == 0 && 
	 sig_usr1 == 0 && sig_alarm == 0 &&
	 sig_usr2 == 0) {
    /* No events waiting, sleep on select() or poll() */
    if (fdlist != NULL) {
      soonest = timer_find_soonest(&events_unit);
      if (soonest != NULL) {
	rolex = (struct timeval *)mem_alloc(&events_unit,
					    sizeof(struct timeval));
	rolex->tv_sec = soonest->alarm_time - time(NULL);
	rolex->tv_usec = 0;
	Debug_unit(&events_unit, "Sleeping %d s...", rolex->tv_sec);
      }
      else {
	rolex = NULL;
	Debug_unit(&events_unit, "Sleeping forever...");
      }
      event_get_fds(&to_select);
      poll_ret = select(FD_SETSIZE, &to_select, NULL, NULL, rolex);
      if (rolex)
	mem_free(&events_unit,rolex);
      Debug_unit(&events_unit, "Select: %d", poll_ret);
      switch (poll_ret) {
      case -1:
	/* Error occurred */
	dump_error(&events_unit, "select");
	break;
      case 0:
	/* Timeout */
	timer_ack(&events_unit, soonest);
	event_put(&events_unit, CE_TIMER, (void*)soonest->data);
	break;
      default:
	/* Data arrival / SVC creation */
	Debug_unit(&events_unit,"fdlist:%d",fdlist);
	for (tmp = fdlist; tmp != NULL; tmp = tmp->next) {
	  dump_printf(EL_DEBUG,"FD:%d",tmp->fd);
	  if (FD_ISSET(tmp->fd, &to_select)) {
	    Debug_unit(&events_unit, "Event on fd %d", tmp->fd);
	    conn_set_active(tmp->data, tmp->fd);
	    event_put(&events_unit, CE_DATA, tmp->data);
	  } 	  
	}
	break;
      }
    }
    else {
      Debug_unit(&events_unit, "No fds, restarting after 10 s");
      sleep(10);
      event_put(&events_unit, CE_RESTART, NULL);
    }
  }
  if (sig_hup != 0) {
      event_put(&events_unit, CE_RESTART, NULL);
      sig_hup = 0;
  }
  if (sig_usr1 != 0) {
      event_put(&events_unit, CE_DUMP, NULL);
      sig_usr1 = 0;
  }
  if (sig_usr2 != 0) {
      event_put(&events_unit, CE_EXIT, NULL);
      sig_usr2 = 0;
  }
  if (sig_alarm != 0) {
      event_put(&events_unit, CE_TIMER, NULL);
      sig_alarm = 0;
  }
  Debug_unit(&events_unit, "get event %s", dump_event_type(eventtail->event->type));
  /* Lock list */
  event = eventhead->event;
  if (eventhead->next != NULL) {
    eventhead->next->prev = NULL;
  }
  else {
    /* This was the last */
    eventtail = NULL;
  }
  tmplist = eventhead;
  eventhead = eventhead->next;
  /* Unlock list */
  mem_free(&events_unit, tmplist);
  return event;
}

/* Add event to queue */
void
event_put(const Unit_t *unit, EventType_t type, void *data)
{
  Event_t *event;
  EventList_t *tmplist;

  Debug_unit(&events_unit, "unit %s puts event %s", unit->name, dump_event_type(type));
  event = (Event_t *)mem_alloc(&events_unit, sizeof(Event_t));
  event->unit = unit;
  event->type = type;
  event->data = data;

  tmplist = (EventList_t *)mem_alloc(&events_unit, sizeof(EventList_t));
  tmplist->event = event;
  tmplist->next = NULL;
  /* Lock list */
  tmplist->prev = eventtail;

  if (eventtail != NULL) {
    eventtail->next = tmplist;
  }
  else {
    /* This is the first one */
    eventhead = tmplist;
  }
  eventtail = tmplist;
  /* Unlock list */
}

/* Call handlers until one returns nonzero */
int
dispatch_handlers(const Event_t *event)
{
  int handled = 0;
  const HFList_t *htmp;

  assert(event != NULL);
  for (htmp = handlers[event->type]; htmp != NULL; htmp = htmp->next) {
    Debug_unit(&events_unit, "trying handler %s", htmp->name);
    handled = (htmp->h(event, htmp->funcdata));
    if (handled != 0) {
      Debug_unit(&events_unit, "success.");
      break;
    }
    else {
      Debug_unit(&events_unit, "failed.");
    }
  }
  return handled;
}

static void
event_get_fds(fd_set *dest)
{
  FDList_t *tmp;

  FD_ZERO(dest);

  for(tmp = fdlist; tmp != NULL; tmp = tmp->next)
    FD_SET(tmp->fd, dest);
}

void
event_add_fd(int fd, void *data)
{
  FDList_t *tmp;

  tmp = mem_alloc(&events_unit, sizeof(FDList_t));
  tmp->fd = fd;
  tmp->data = data;
  tmp->next = fdlist;
  fdlist = tmp;
}

void
event_remove_fd(int fd)
{
  FDList_t *tmp, *prev = NULL;

  for (tmp = fdlist; tmp != NULL; prev = tmp, tmp = tmp->next) {
    if (tmp->fd == fd) {
      break;
    }
  }

  if (tmp == NULL) {
    Debug_unit(&events_unit, "Could not find fd %d for removal", fd);
    return;
  }
  else {
    if (prev != NULL) {
      prev->next = tmp->next;
    }
    else {
      fdlist = tmp->next;
    }
    mem_free(&events_unit, tmp);
  }
}
