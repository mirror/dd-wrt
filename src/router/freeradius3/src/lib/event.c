/*
 * event.c	Non-thread-safe event handling, specific to a RADIUS
 *		server.
 *
 * Version:	$Id: 25ec7ddc2dcb3dc4fc05c1ad0538745ad25c9088 $
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *  Copyright 2007  The FreeRADIUS server project
 *  Copyright 2007  Alan DeKok <aland@ox.org>
 */

RCSID("$Id: 25ec7ddc2dcb3dc4fc05c1ad0538745ad25c9088 $")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/heap.h>
#include <freeradius-devel/event.h>

#include <pthread.h>

#ifdef HAVE_KQUEUE
#ifndef HAVE_SYS_EVENT_H
#error kqueue requires <sys/event.h>

#else
#include <sys/event.h>
#endif
#endif	/* HAVE_KQUEUE */

typedef struct fr_event_fd_t {
	int			fd;

	fr_event_fd_handler_t	handler;
	fr_event_fd_handler_t	write_handler;
	void			*ctx;
} fr_event_fd_t;

#define FR_EV_MAX_EVENTS (512)

int fr_ev_max_fds = FR_EV_MAX_EVENTS;

#undef USEC
#define USEC (1000000)

struct fr_event_list_t {
	fr_heap_t	*times;

	int		exit;

	fr_event_status_t status;

	struct timeval  now;
	bool		dispatch;

	int		num_readers;
#ifndef HAVE_KQUEUE
	int		max_readers;
	int		max_fd;

	fd_set		read_fds;
	fd_set		write_fds;
#else
	int		kq;
	struct kevent	events[FR_EV_MAX_EVENTS]; /* so it doesn't go on the stack every time */
#endif

	fr_event_fd_t	readers[1];
};

/*
 *	Internal structure for managing events.
 */
struct fr_event_t {
	fr_event_callback_t	callback;
	void			*ctx;
	struct timeval		when;
	fr_event_t		**parent;
	int			heap;
};


static int fr_event_list_time_cmp(void const *one, void const *two)
{
	fr_event_t const *a = one;
	fr_event_t const *b = two;

	if (a->when.tv_sec < b->when.tv_sec) return -1;
	if (a->when.tv_sec > b->when.tv_sec) return +1;

	if (a->when.tv_usec < b->when.tv_usec) return -1;
	if (a->when.tv_usec > b->when.tv_usec) return +1;

	return 0;
}


static int _event_list_free(fr_event_list_t *list)
{
	fr_event_list_t *el = list;
	fr_event_t *ev;

	while ((ev = fr_heap_peek(el->times)) != NULL) {
		fr_event_delete(el, &ev);
	}

	fr_heap_delete(el->times);

#ifdef HAVE_KQUEUE
	close(el->kq);
#endif

	return 0;
}


fr_event_list_t *fr_event_list_create(TALLOC_CTX *ctx, fr_event_status_t status)
{
	int i;
	fr_event_list_t *el;

	el = (fr_event_list_t *) talloc_zero_array(ctx, uint8_t, sizeof(fr_event_list_t) + fr_ev_max_fds * sizeof(el->readers[0]));
	if (!fr_assert(el)) {
		return NULL;
	}
	talloc_set_destructor(el, _event_list_free);
	talloc_set_type(el, fr_event_list_t);

	el->times = fr_heap_create(fr_event_list_time_cmp, offsetof(fr_event_t, heap));
	if (!el->times) {
		talloc_free(el);
		return NULL;
	}

	for (i = 0; i < fr_ev_max_fds; i++) {
		el->readers[i].fd = -1;
	}

#ifndef HAVE_KQUEUE
	el->max_fd = 0;
	FD_ZERO(&el->read_fds);
	FD_ZERO(&el->write_fds);
#else
	el->kq = kqueue();
	if (el->kq < 0) {
		talloc_free(el);
		return NULL;
	}
#endif

	el->status = status;

	return el;
}

int fr_event_list_num_fds(fr_event_list_t *el)
{
	if (!el) return 0;

	return el->num_readers;
}

int fr_event_list_num_elements(fr_event_list_t *el)
{
	if (!el) return 0;

	return fr_heap_num_elements(el->times);
}


int fr_event_delete(fr_event_list_t *el, fr_event_t **parent)
{
	int ret;

	fr_event_t *ev;

	if (!el || !parent || !*parent) return 0;

#ifndef NDEBUG
	/*
	 *  Validate the event_t struct to detect memory issues early.
	 */
	ev = talloc_get_type_abort(*parent, fr_event_t);

#else
	ev = *parent;
#endif

	if (ev->parent) {
		fr_assert(*(ev->parent) == ev);
		*ev->parent = NULL;
	}
	*parent = NULL;

	ret = fr_heap_extract(el->times, ev);
	fr_assert(ret == 1);	/* events MUST be in the heap */
	talloc_free(ev);

	return ret;
}


int fr_event_insert(fr_event_list_t *el, fr_event_callback_t callback, void *ctx, struct timeval *when,
		    fr_event_t **parent)
{
	fr_event_t *ev;

	if (!el) {
		fr_strerror_printf("Invalid arguments (NULL event list)");
		return 0;
	}

	if (!callback) {
		fr_strerror_printf("Invalid arguments (NULL callback)");
		return 0;
	}

	if (!when || (when->tv_usec >= USEC)) {
		fr_strerror_printf("Invalid arguments (time)");
		return 0;
	}

	if (!parent) {
		fr_strerror_printf("Invalid arguments (NULL parent)");
		return 0;
	}

	/*
	 *	If there is an event, re-use it instead of freeing it
	 *	and allocating a new one.
	 */
	if (*parent) {
		int ret;

#ifndef NDEBUG
		ev = talloc_get_type_abort(*parent, fr_event_t);
#else
		ev = *parent;
#endif

		ret = fr_heap_extract(el->times, ev);
		fr_assert(ret == 1);	/* events MUST be in the heap */

		memset(ev, 0, sizeof(*ev));
	} else {
		ev = talloc_zero(el, fr_event_t);
		if (!ev) return 0;
	}

	ev->callback = callback;
	ev->ctx = ctx;
	ev->when = *when;
	ev->parent = parent;

	if (!fr_heap_insert(el->times, ev)) {
		talloc_free(ev);
		return 0;
	}

	*parent = ev;
	return 1;
}


int fr_event_run(fr_event_list_t *el, struct timeval *when)
{
	fr_event_callback_t callback;
	void *ctx;
	fr_event_t *ev;

	if (!el) return 0;

	if (fr_heap_num_elements(el->times) == 0) {
		when->tv_sec = 0;
		when->tv_usec = 0;
		return 0;
	}

	ev = fr_heap_peek(el->times);
	if (!ev) {
		when->tv_sec = 0;
		when->tv_usec = 0;
		return 0;
	}

#ifndef NDEBUG
	ev = talloc_get_type_abort(ev, fr_event_t);
#endif

	/*
	 *	See if it's time to do this one.
	 */
	if ((ev->when.tv_sec > when->tv_sec) ||
	    ((ev->when.tv_sec == when->tv_sec) &&
	     (ev->when.tv_usec > when->tv_usec))) {
		*when = ev->when;
		return 0;
	}

	callback = ev->callback;
	ctx = ev->ctx;

	/*
	 *	Delete the event before calling it.
	 */
	fr_event_delete(el, ev->parent);

	callback(ctx);
	return 1;
}


int fr_event_now(fr_event_list_t *el, struct timeval *when)
{
	if (!when) return 0;

	if (el && el->dispatch) {
		*when = el->now;
	} else {
		gettimeofday(when, NULL);
	}

	return 1;
}


int fr_event_fd_insert(fr_event_list_t *el, int type, int fd,
		       fr_event_fd_handler_t handler, void *ctx)
{
	int i;
	fr_event_fd_t *ef;

	if (!el) {
		fr_strerror_printf("Invalid arguments (NULL event list)");
		return 0;
	}

	if (!handler) {
		fr_strerror_printf("Invalid arguments (NULL handler)");
		return 0;
	}

	if (!ctx) {
		fr_strerror_printf("Invalid arguments (NULL ctx)");
		return 0;
	}

	if (fd < 0) {
		fr_strerror_printf("Invalid arguments (bad FD %i)", fd);
		return 0;
	}

	if (type != 0) {
		fr_strerror_printf("Invalid type %i", type);
		return 0;
	}

	if (el->num_readers >= fr_ev_max_fds) {
		fr_strerror_printf("Too many readers");
		return 0;
	}
	ef = NULL;

#ifdef HAVE_KQUEUE
	/*
	 *	We need to store TWO fields with the event.  kqueue
	 *	only lets us store one.  If we put the two fields into
	 *	a malloc'd structure, that would help.  Except that
	 *	kqueue can silently delete the event when the socket
	 *	is closed, and not give us the opportunity to free it.
	 *	<sigh>
	 *
	 *	The solution is to put the fields into an array, and
	 *	do a linear search on addition/deletion of the FDs.
	 *	However, to avoid MOST linear issues, we start off the
	 *	search at "FD" offset.  Since FDs are unique, AND
	 *	usually less than 256, we do "FD & 0xff", which is a
	 *	good guess, and makes the lookups mostly O(1).
	 */
	for (i = 0; i < fr_ev_max_fds; i++) {
		int j;
		struct kevent evset;

		j = (i + fd) & (fr_ev_max_fds - 1);

		if (el->readers[j].fd >= 0) continue;

		/*
		 *	We want to read from the FD.
		 */
		EV_SET(&evset, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &el->readers[j]);
		if (kevent(el->kq, &evset, 1, NULL, 0, NULL) < 0) {
			fr_strerror_printf("Failed inserting event for FD %i: %s", fd, fr_syserror(errno));
			return 0;
		}

		ef = &el->readers[j];
		el->num_readers++;
		break;
	}

#else  /* HAVE_KQUEUE */

	/*
	 *	select() has limits.
	 */
	if (fd > FD_SETSIZE) {
		fprintf(stderr, "FD is larger than FD_SETSIZE");
		return 0;
	}

	for (i = 0; i <= el->max_readers; i++) {
		/*
		 *	Be fail-safe on multiple inserts.
		 */
		if (el->readers[i].fd == fd) {
			if ((el->readers[i].handler != handler) ||
			    (el->readers[i].ctx != ctx)) {
				fr_strerror_printf("Multiple handlers for same FD");
				return 0;
			}

			/*
			 *	No change.
			 */
			return 1;
		}

		if (el->readers[i].fd < 0) {
			ef = &el->readers[i];
			el->num_readers++;

			if (i == el->max_readers) el->max_readers = i + 1;

			FD_SET(fd, &el->read_fds);
			if (el->max_fd <= fd) el->max_fd = fd;
			break;
		}
	}
#endif

	if (!ef) {
		fr_strerror_printf("Failed assigning FD");
		return 0;
	}

	ef->fd = fd;
	ef->handler = handler;
	ef->ctx = ctx;

	return 1;
}

int fr_event_fd_write_handler(fr_event_list_t *el, int type, int fd,
			      fr_event_fd_handler_t write_handler, void *ctx)
{
	int i;

	if (!el || (fd < 0)) return 0;

	if (type != 0) return 0;

#ifdef HAVE_KQUEUE
	for (i = 0; i < fr_ev_max_fds; i++) {
		int j;
		struct kevent evset;

		j = (i + fd) & (fr_ev_max_fds - 1);

		if (el->readers[j].fd != fd) continue;

		fr_assert(ctx = el->readers[j].ctx);

		/*
		 *	Tell us when the socket is ready for writing
		 */
		if (write_handler) {
			fr_assert(!el->readers[j].write_handler);

			el->readers[j].write_handler = write_handler;

			EV_SET(&evset, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &el->readers[j]);
		} else {
			fr_assert(el->readers[j].write_handler);

			el->readers[j].write_handler = NULL;

			EV_SET(&evset, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		}
		if (kevent(el->kq, &evset, 1, NULL, 0, NULL) < 0) {
			fr_strerror_printf("Failed inserting event for FD %i: %s", fd, fr_syserror(errno));
			return 0;
		}

		return 1;
	}

#else

	for (i = 0; i < el->max_readers; i++) {
		if (el->readers[i].fd != fd) continue;

		fr_assert(ctx = el->readers[i].ctx);
		el->readers[i].write_handler = write_handler;

		if (write_handler) {
			FD_SET(fd, &el->write_fds); /* fd MUST already be in the set of readers! */
		}  else {
			FD_CLR(fd, &el->write_fds);
		}
		return 1;
	}
#endif	/* HAVE_KQUEUE */

	return 0;
}

int fr_event_fd_delete(fr_event_list_t *el, int type, int fd)
{
	int i;

	if (!el || (fd < 0)) return 0;

	if (type != 0) return 0;

#ifdef HAVE_KQUEUE
	for (i = 0; i < fr_ev_max_fds; i++) {
		int j;
		struct kevent evset;

		j = (i + fd) & (fr_ev_max_fds - 1);

		if (el->readers[j].fd != fd) continue;

		/*
		 *	Tell the kernel to delete it from the list.
		 *
		 *	The caller MAY have closed it, in which case
		 *	the kernel has removed it from the list.  So
		 *	we ignore the return code from kevent().
		 */
		EV_SET(&evset, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		(void) kevent(el->kq, &evset, 1, NULL, 0, NULL);

		/*
		 *	Delete the write handler if it exits.
		 */
		if (el->readers[j].write_handler) {
			EV_SET(&evset, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
			(void) kevent(el->kq, &evset, 1, NULL, 0, NULL);
		}

		el->readers[j].fd = -1;
		el->num_readers--;

		return 1;
	}

#else
	for (i = 0; i < el->max_readers; i++) {
		if (el->readers[i].fd == fd) {
			el->readers[i].fd = -1;
			el->num_readers--;

			if ((i + 1) == el->max_readers) el->max_readers = i;
			FD_CLR(fd, &el->read_fds);
			FD_CLR(fd, &el->write_fds);

			/*
			 *	@todo - update el->max_fd, too.
			 */
			return 1;
		}
	}
#endif	/* HAVE_KQUEUE */

	return 0;
}


void fr_event_loop_exit(fr_event_list_t *el, int code)
{
	if (!el) return;

	el->exit = code;
}

bool fr_event_loop_exiting(fr_event_list_t *el)
{
	return (el->exit != 0);
}

int fr_event_loop(fr_event_list_t *el)
{
	int i, rcode;
	struct timeval when, *wake;
#ifdef HAVE_KQUEUE
	struct timespec ts_when, *ts_wake;
#else
	fd_set read_fds, write_fds;
#endif

	el->exit = 0;
	el->dispatch = true;

	while (!el->exit) {
		/*
		 *	Find the first event.  If there's none, we wait
		 *	on the socket forever.
		 */
		when.tv_sec = 0;
		when.tv_usec = 0;

		if (fr_heap_num_elements(el->times) > 0) {
			fr_event_t *ev;

			ev = fr_heap_peek(el->times);
			if (!ev) {
				fr_exit_now(42);
			}

			gettimeofday(&el->now, NULL);

			if (timercmp(&el->now, &ev->when, <)) {
				when = ev->when;
				when.tv_sec -= el->now.tv_sec;

				if (when.tv_sec > 0) {
					when.tv_sec--;
					when.tv_usec += USEC;
				} else {
					when.tv_sec = 0;
				}
				when.tv_usec -= el->now.tv_usec;
				if (when.tv_usec >= USEC) {
					when.tv_usec -= USEC;
					when.tv_sec++;
				}
			} else { /* we've passed the event time */
				when.tv_sec = 0;
				when.tv_usec = 0;
			}

			wake = &when;
		} else {
			wake = NULL;
		}

		/*
		 *	Tell someone what the status is.
		 */
		if (el->status) el->status(wake);

#ifndef HAVE_KQUEUE
		read_fds = el->read_fds;
		write_fds = el->write_fds;
		rcode = select(el->max_fd + 1, &read_fds, &write_fds, NULL, wake);
		if ((rcode < 0) && (errno != EINTR)) {
			fr_strerror_printf("Failed in select: %s", fr_syserror(errno));
			el->dispatch = false;
			return -1;
		}

#else  /* HAVE_KQUEUE */

		if (wake) {
			ts_wake = &ts_when;
			ts_when.tv_sec = when.tv_sec;
			ts_when.tv_nsec = when.tv_usec * 1000;

		} else {
			ts_wake = NULL;
		}

		rcode = kevent(el->kq, NULL, 0, el->events, FR_EV_MAX_EVENTS, ts_wake);
#endif	/* HAVE_KQUEUE */

		if (fr_heap_num_elements(el->times) > 0) {
			do {
				gettimeofday(&el->now, NULL);
				when = el->now;
			} while (fr_event_run(el, &when) == 1);
		}

		if (rcode <= 0) continue;

#ifndef HAVE_KQUEUE
		/*
		 *	Loop over all of the sockets to see if there's
		 *	an event for that socket.
		 */
		for (i = 0; i < el->max_readers; i++) {
			fr_event_fd_t *ef = &el->readers[i];

			if (ef->fd < 0) continue;

			/*
			 *	Check if the socket is available for writing.
			 */
			if (ef->write_handler && FD_ISSET(ef->fd, &write_fds)) {
				ef->write_handler(el, ef->fd, ef->ctx);
			}

			/*
			 *	Re-check the fd - the write_handler may have closed it.
			 */
			if (ef->fd < 0) continue;
			if (!FD_ISSET(ef->fd, &read_fds)) continue;

			ef->handler(el, ef->fd, ef->ctx);
		}

#else  /* HAVE_KQUEUE */

		/*
		 *	Loop over all of the events, servicing them.
		 */
		for (i = 0; i < rcode; i++) {
			fr_event_fd_t *ef = el->events[i].udata;

			if (el->events[i].flags & EV_EOF) {
				/*
				 *	FIXME: delete the handler
				 *	here, and fix process.c to not
				 *	call fr_event_fd_delete().
				 *	It's cleaner.
				 *
				 *	Call the handler, which SHOULD
				 *	delete the connection.
				 */
				ef->handler(el, ef->fd, ef->ctx);
				continue;
			}

			if (el->events[i].filter == EVFILT_WRITE) {
				ef->write_handler(el, ef->fd, ef->ctx);
				continue;
			}

			/*
			 *	Else it's our event.  We only set
			 *	EVFILT_READ, so it must be a read
			 *	event.
			 */
			ef->handler(el, ef->fd, ef->ctx);
		}
#endif	/* HAVE_KQUEUE */
	}

	el->dispatch = false;
	return el->exit;
}


#ifdef TESTING

/*
 *  cc -g -I .. -c rbtree.c -o rbtree.o && cc -g -I .. -c isaac.c -o isaac.o && cc -DTESTING -I .. -c event.c  -o event_mine.o && cc event_mine.o rbtree.o isaac.o -o event
 *
 *  ./event
 *
 *  And hit CTRL-S to stop the output, CTRL-Q to continue.
 *  It normally alternates printing the time and sleeping,
 *  but when you hit CTRL-S/CTRL-Q, you should see a number
 *  of events run right after each other.
 *
 *  OR
 *
 *   valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./event
 */

static void print_time(void *ctx)
{
	struct timeval *when = ctx;

	printf("%d.%06d\n", when->tv_sec, when->tv_usec);
	fflush(stdout);
}

static fr_randctx rand_pool;

static uint32_t event_rand(void)
{
	uint32_t num;

	num = rand_pool.randrsl[rand_pool.randcnt++ & 0xff];
	if (rand_pool.randcnt == 256) {
		fr_isaac(&rand_pool);
		rand_pool.randcnt = 0;
	}

	return num;
}


#define MAX 100
int main(int argc, char **argv)
{
	int i, rcode;
	struct timeval array[MAX];
	struct timeval now, when;
	fr_event_list_t *el;

	el = fr_event_list_create(NULL, NULL);
	if (!el) exit(1);

	memset(&rand_pool, 0, sizeof(rand_pool));
	rand_pool.randrsl[1] = time(NULL);

	fr_randinit(&rand_pool, 1);
	rand_pool.randcnt = 0;

	gettimeofday(&array[0], NULL);
	for (i = 1; i < MAX; i++) {
		array[i] = array[i - 1];

		array[i].tv_usec += event_rand() & 0xffff;
		if (array[i].tv_usec > 1000000) {
			array[i].tv_usec -= 1000000;
			array[i].tv_sec++;
		}
		fr_event_insert(el, print_time, &array[i], &array[i]);
	}

	while (fr_event_list_num_elements(el)) {
		gettimeofday(&now, NULL);
		when = now;
		if (!fr_event_run(el, &when)) {
			int delay = (when.tv_sec - now.tv_sec) * 1000000;
			delay += when.tv_usec;
			delay -= now.tv_usec;

			printf("\tsleep %d\n", delay);
			fflush(stdout);
			usleep(delay);
		}
	}

	talloc_free(el);

	return 0;
}
#endif
