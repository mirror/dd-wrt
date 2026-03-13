#include <libubox/uloop.h>
#include "includes.h"
#include "common.h"
#include "eloop.h"

static void eloop_uloop_event_cb(int sock, void *eloop_ctx, void *sock_ctx)
{
}

static void eloop_uloop_fd_cb(struct uloop_fd *fd, unsigned int events)
{
	unsigned int changed = events ^ fd->flags;

	if (changed & ULOOP_READ) {
		if (events & ULOOP_READ)
			eloop_register_sock(fd->fd, EVENT_TYPE_READ, eloop_uloop_event_cb, fd, fd);
		else
			eloop_unregister_sock(fd->fd, EVENT_TYPE_READ);
	}

	if (changed & ULOOP_WRITE) {
		if (events & ULOOP_WRITE)
			eloop_register_sock(fd->fd, EVENT_TYPE_WRITE, eloop_uloop_event_cb, fd, fd);
		else
			eloop_unregister_sock(fd->fd, EVENT_TYPE_WRITE);
	}
}

static bool uloop_timeout_poll_handler(struct os_reltime *tv, bool tv_set)
{
	struct os_reltime tv_uloop;
	int timeout_ms = uloop_get_next_timeout();

	if (timeout_ms < 0)
		return false;

	tv_uloop.sec = timeout_ms / 1000;
	tv_uloop.usec = (timeout_ms % 1000) * 1000;

	if (!tv_set || os_reltime_before(&tv_uloop, tv)) {
		*tv = tv_uloop;
		return true;
	}

	return false;
}

static void uloop_poll_handler(void)
{
	uloop_run_timeout(0);
}

void eloop_add_uloop(void)
{
	static bool init_done = false;

	if (!init_done) {
		uloop_init();
		uloop_fd_set_cb = eloop_uloop_fd_cb;
		init_done = true;
	}

	eloop_register_cb(uloop_poll_handler, uloop_timeout_poll_handler);
}
