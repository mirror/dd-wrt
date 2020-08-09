// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)


#include <stdio.h>
#include <stdbool.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <ev.h>

#include "server_socket.h"
#include "utils.h"
#include "packet.h"

volatile bool quit = false;

static void handle_signal(int sig)
{
    ev_break(EV_DEFAULT, EVBREAK_ALL);;
}

int signal_init(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    return 0;
}

int main (void)
{
	ctl_socket_init();
	packet_socket_init();

	ev_run(EV_DEFAULT, 0);

	packet_socket_cleanup();
	ctl_socket_cleanup();

	return 0;
}

