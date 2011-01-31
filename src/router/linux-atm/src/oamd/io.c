/* io.c - I/O operations */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <net/if.h>
#include <netinet/in.h>
#include <atm.h>
#define _LINUX_NETDEVICE_H	/* glibc2 */
#include <linux/types.h>
#include <linux/if_arp.h>

#include "atmd.h"

#include "io.h"
#include "oam.h"

#define COMPONENT "IO"

static int kernel;
extern int run_fsm;

/* ----- kernel interface -------------------------------------------------- */

void
open_kernel(void)
{
	if ((kernel = socket(PF_ATMSVC, SOCK_DGRAM, 0)) < 0)
		diag(COMPONENT, DIAG_FATAL, "socket: %s", strerror(errno));
	if (ioctl(kernel, ATMOAMD_CTRL, 0) < 0)
		diag(COMPONENT, DIAG_FATAL, "ioctl ATMOAMD_CTRL: %s",
		     strerror(errno));
}

int
send_kernel(struct atmoam_ctrl *ctrl)
{
	int size;

	size = write(kernel, ctrl, sizeof (struct atmoam_ctrl));
	if (size < 0) {
		diag(COMPONENT, DIAG_DEBUG, "write kernel: %s",
		     strerror(errno));
		return -errno;
	}
	return size;
}

void
recv_kernel(void)
{
	struct atmoam_ctrl ctrl;
	int size;

	size = read(kernel, &ctrl, sizeof (ctrl));
	if (size < 0) {
		diag(COMPONENT, DIAG_ERROR, "read kernel: %s", strerror(errno));
		return;
	}

	diag(COMPONENT, DIAG_DEBUG,
	     "OAM %s Cell received on Intf %d VPI/VCI %d/%d (Vcc %p)",
	     ctrl.pti == 5 ? "F5-E2E" : (ctrl.pti == 4 ? "F5-SEG" : "UNK"),
	     ctrl.number, ctrl.vpi, ctrl.vci, *(struct atm_vcc **) &ctrl.vcc);

	oam_process(&ctrl);
}

void
close_kernel(void)
{
	close(kernel);
}

void
poll_loop(void)
{
	struct pollfd pollfds;

	pollfds.fd = kernel;
	pollfds.events = POLLIN;	/* Que eventos queremos, solo entradas */

	for (;;) {
		oam_state_print();

		if (run_fsm) {
			oam_fsm();
			run_fsm = 0;
		}

		switch (poll(&pollfds, 1, 10000)) {
		case 0:
			break;
		case -1:
			if (errno != EINTR)
				diag(COMPONENT, DIAG_ERROR, "poll loop...%s",
				       strerror(errno));
			break;
		default:
			if (pollfds.revents && POLLIN)
				recv_kernel();
		}
	}
}
