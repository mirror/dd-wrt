/*
 * Event handler
 *
 * $Id: events.h,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */

#ifndef EVENTS_H
#define EVENTS_H
/* System includes needed for types */

/* Local includes needed for types */
#include "units.h"

/* Type definitions */
typedef enum {
  CE_SVC_OPEN, CE_SVC_CLOSE, CE_DATA, CE_TIMER, CE_DUMP, CE_RESTART, CE_EXIT
} EventType_t;
#define CE_MAX CE_EXIT

typedef struct {
  const Unit_t *unit;
  EventType_t type;
  void *data;
} Event_t;

/* Event handlers should return nonzero if they swallowed the event */
typedef int (* HandlerFunc_t)(const Event_t *event, void *funcdata);

/* Global function prototypes */
void add_event_handler(EventType_t type, HandlerFunc_t func, const char *name, void *funcdata);
const Event_t *event_get_next(void);
void event_put(const Unit_t *unit, EventType_t type, void *data);
const char *dump_event_type(EventType_t type);
int dispatch_handlers(const Event_t *event);
void event_add_fd(int fd, void *data);
void event_remove_fd(int fd);

/* Global data */
extern const Unit_t events_unit;

#endif

