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
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 * $Id$ GNU
 */

#ifndef SCREEN_SCHED_H
#define SCREEN_SCHED_H

#include <stdbool.h>
#include <sys/time.h>

typedef enum {
	EV_TIMEOUT	= 0,
	EV_READ		= 1,
	EV_WRITE	= 2,
	EV_ALWAYS	= 3
} EventType;

typedef struct Event Event;
struct Event {
	Event *next;
	void (*handler) (Event *, void *);
	void *data;
	int fd;
	EventType type;
	int priority;
	int timeout;		/* timeout in milliseconds */
	bool queued;		/* in evs queue */
	int *condpos;		/* only active if condpos - condneg > 0 */
	int *condneg;
};

void evenq (Event *);
void evdeq (Event *);
void SetTimeout (Event *, int);
void sched (void) __attribute__((__noreturn__));

#endif /* SCREEN_SCHED_H */
