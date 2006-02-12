/*
 *	BIRD Object Locks
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_LOCKS_H_
#define _BIRD_LOCKS_H_

#include "lib/resource.h"
#include "lib/event.h"

/*
 *  The object locks are used for controlling exclusive access
 *  to various physical resources like UDP ports on specific devices.
 *  When you want to access such resource, you ask for a object lock
 *  structure, fill in specification of the object and your function
 *  you want to have called when the object is available and invoke
 *  olock_acquire() afterwards. When the object becomes free, the lock
 *  manager calls your function. To free the object lock, just call rfree
 *  on its resource.
 */

struct object_lock {
  resource r;
  ip_addr addr;		/* Identification of a object: IP address */
  unsigned int type;	/* ... object type (OBJLOCK_xxx) */
  struct iface *iface;	/* ... interface */
  unsigned int port;	/* ... port number */
  void (*hook)(struct object_lock *);	/* Called when the lock succeeds */
  void *data;		/* User data */
  /* ... internal to lock manager, don't touch ... */
  node n;		/* Node in list of olocks */
  int state;		/* OLOCK_STATE_xxx */
  list waiters;		/* Locks waiting for the same resource */
};

struct object_lock *olock_new(pool *);
void olock_acquire(struct object_lock *);
void olock_init(void);

#define OBJLOCK_UDP 1			/* UDP port */
#define OBJLOCK_TCP 2			/* TCP port */
#define OBJLOCK_IP 3			/* IP protocol */

#define OLOCK_STATE_FREE 0
#define OLOCK_STATE_LOCKED 1
#define OLOCK_STATE_WAITING 2
#define OLOCK_STATE_EVENT 3		/* waiting for unlock processing */

#endif
