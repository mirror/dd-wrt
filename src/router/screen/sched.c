/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include <sys/types.h>
#if !defined(sun) && !defined(B43) && !defined(ISC) && !defined(pyr) && !defined(_CX_UX)
# include <time.h>
#endif
#include <sys/time.h>

#include "config.h"
#include "screen.h"
#include "extern.h"

static struct event *evs;
static struct event *tevs;
static struct event *nextev;
static int calctimeout;

static struct event *calctimo __P((void));
#if (defined(sgi) && defined(SVR4)) || defined(__osf__) || defined(M_UNIX)
static int sgihack __P((void));
#endif

void
evenq(ev)
struct event *ev;
{
  struct event *evp, **evpp;
  debug3("New event fd %d type %d queued %d\n", ev->fd, ev->type, ev->queued);
  if (ev->queued)
    return;
  evpp = &evs;
  if (ev->type == EV_TIMEOUT)
    {
      calctimeout = 1;
      evpp = &tevs;
    }
  for (; (evp = *evpp); evpp = &evp->next)
    if (ev->pri > evp->pri)
      break;
  ev->next = evp;
  *evpp = ev;
  ev->queued = 1;
}

void
evdeq(ev)
struct event *ev;
{
  struct event *evp, **evpp;
  debug3("Deq event fd %d type %d queued %d\n", ev->fd, ev->type, ev->queued);
  if (!ev->queued)
    return;
  evpp = &evs;
  if (ev->type == EV_TIMEOUT)
    {
      calctimeout = 1;
      evpp = &tevs;
    }
  for (; (evp = *evpp); evpp = &evp->next)
    if (evp == ev)
      break;
  ASSERT(evp);
  *evpp = ev->next;
  ev->queued = 0;
  if (ev == nextev)
    nextev = nextev->next;
}

static struct event *
calctimo()
{
  struct event *ev, *min;
  long mins;

  if ((min = tevs) == 0)
    return 0;
  mins = min->timeout.tv_sec;
  for (ev = tevs->next; ev; ev = ev->next)
    {
      ASSERT(ev->type == EV_TIMEOUT);
      if (mins < ev->timeout.tv_sec)
	continue;
      if (mins > ev->timeout.tv_sec || min->timeout.tv_usec > ev->timeout.tv_usec)
	{
	  min = ev;
	  mins = ev->timeout.tv_sec;
	}
    }
  return min;
}

void
sched()
{
  struct event *ev;
  fd_set r, w, *set;
  struct event *timeoutev = 0;
  struct timeval timeout;
  int nsel;

  for (;;)
    {
      if (calctimeout)
	timeoutev = calctimo();
      if (timeoutev)
	{
	  gettimeofday(&timeout, NULL);
	  /* tp - timeout */
	  timeout.tv_sec = timeoutev->timeout.tv_sec - timeout.tv_sec;
	  timeout.tv_usec = timeoutev->timeout.tv_usec - timeout.tv_usec;
	  if (timeout.tv_usec < 0)
	    {
	      timeout.tv_usec += 1000000;
	      timeout.tv_sec--;
	    }
	  if (timeout.tv_sec < 0)
	    {
	      timeout.tv_usec = 0;
	      timeout.tv_sec = 0;
	    }
	}
#ifdef DEBUG
      debug("waiting for events");
      if (timeoutev)
        debug2(" timeout %d secs %d usecs", timeout.tv_sec, timeout.tv_usec);
      debug(":\n");
      for (ev = evs; ev; ev = ev->next)
        debug3(" - fd %d type %d pri %d\n", ev->fd, ev->type, ev->pri);
      if (tevs)
        debug("timed events:\n");
      for (ev = tevs; ev; ev = ev->next)
        debug3(" - pri %d sec %d usec %d\n", ev->pri, ev->timeout.tv_sec, ev->timeout.tv_usec);
#endif

      FD_ZERO(&r);
      FD_ZERO(&w);
      for (ev = evs; ev; ev = ev->next)
        {
	  if (ev->condpos && *ev->condpos <= (ev->condneg ? *ev->condneg : 0))
	    {
	      debug2(" - cond ev fd %d type %d failed\n", ev->fd, ev->type);
	      continue;
	    }
	  if (ev->type == EV_READ)
	    FD_SET(ev->fd, &r);
	  else if (ev->type == EV_WRITE)
	    FD_SET(ev->fd, &w);
        }

#ifdef DEBUG
      debug("readfds:");
      for (nsel = 0; nsel < FD_SETSIZE; nsel++)
	if (FD_ISSET(nsel, &r))
	  debug1(" %d", nsel);
      debug("\n");
      debug("writefds:");
      for (nsel = 0; nsel < FD_SETSIZE; nsel++)
	if (FD_ISSET(nsel, &w))
	  debug1(" %d", nsel);
      debug("\n");
#endif

      nsel = select(FD_SETSIZE, &r, &w, (fd_set *)0, timeoutev ? &timeout : (struct timeval *) 0);
      if (nsel < 0)
	{
	  if (errno != EINTR)
	    {
#if defined(sgi) && defined(SVR4)
	      if (errno == EIO && sgihack())
		continue;
#endif
#if defined(__osf__) || defined(M_UNIX)
	      /* OSF/1 3.x, SCO bug: EBADF */
	      /* OSF/1 4.x bug: EIO */
	      if ((errno == EIO || errno == EBADF) && sgihack())
		continue;
#endif
	      Panic(errno, "select");
	    }
	  nsel = 0;
	}
      else if (nsel == 0)	/* timeout */
	{
	  debug("TIMEOUT!\n");
	  ASSERT(timeoutev);
	  evdeq(timeoutev);
	  timeoutev->handler(timeoutev, timeoutev->data);
	}
#ifdef SELECT_BROKEN
      /*
       * Sequents select emulation counts a descriptor which is
       * readable and writeable only as one hit. Waaaaa.
       */
      if (nsel)
        nsel = 2 * FD_SETSIZE;
#endif

      for (ev = evs; ev; ev = nextev)
        {
          nextev = ev->next;
	  if (ev->type != EV_ALWAYS)
	    {
	      set = ev->type == EV_READ ? &r : &w;
	      if (nsel == 0 || !FD_ISSET(ev->fd, set))
		continue;
	      nsel--;
	    }
	  if (ev->condpos && *ev->condpos <= (ev->condneg ? *ev->condneg : 0))
	    continue;
	  debug2(" + hit ev fd %d type %d!\n", ev->fd, ev->type);
	  ev->handler(ev, ev->data);
        }
    }
}

void
SetTimeout(ev, timo)
struct event *ev;
int timo;
{
  ASSERT(ev->type == EV_TIMEOUT);
  debug2("event %x new timeout %d ms\n", ev, timo);
  gettimeofday(&ev->timeout, NULL);
  ev->timeout.tv_sec  += timo / 1000;
  ev->timeout.tv_usec += (timo % 1000) * 1000;
  if (ev->timeout.tv_usec > 1000000)
    {
      ev->timeout.tv_usec -= 1000000;
      ev->timeout.tv_sec++;
    }
  if (ev->queued)
    calctimeout = 1;
}


#if (defined(sgi) && defined(SVR4)) || defined(__osf__) || defined(M_UNIX)

extern struct display *display, *displays;
static int sgihack()
{
  fd_set r, w;
  struct timeval tv;

  debug("IRIX5.2 workaround: searching for bad display\n");
  for (display = displays; display; )
    {
      FD_ZERO(&r);
      FD_ZERO(&w);
      FD_SET(D_userfd, &r);
      FD_SET(D_userfd, &w);
      tv.tv_sec = tv.tv_usec = 0;
      if (select(FD_SETSIZE, &r, &w, (fd_set *)0, &tv) == -1)
	{
	  if (errno == EINTR)
	    continue;
	  Hangup();	/* goodbye display */
	  return 1;
	}
      display = display->d_next;
    }
  return 0;
}

#endif
