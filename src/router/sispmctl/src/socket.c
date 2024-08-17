/*
    Copyright (C) 2003 Evan Buswell

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "config.h"
#ifdef HAVE_SYS_ETHERNET_H
#include <sys/ethernet.h>
#endif
#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif
#include <usb.h>
#include "sispm_ctl.h"
#include "socket.h"
#include "nethelp.h"

#ifndef WEBLESS
int listenport = LISTENPORT;

void l_listen(int *sock, struct usb_device *dev, int devnum)
{
	int i;
	int s;
	int connected = 0;
	int junk = 0;
	char *oob;
	char *buffer;
	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = 250000000; /* a quarter second */

	oob = (char *)malloc(32);
	buffer = (char *)malloc(BUFFERSIZE + 4);

	if (debug)
		fprintf(stderr, "Listening for local provider on port %d...\n", listenport);
	listen(*sock, 1); /* We only get one connection on this port.
				   Everything else is refused. */
	while (1) {
		while ((s = accept(*sock, NULL, NULL)) == -1) {
			sleep(1);
			/* retry after error.  Really bad errors shouldn't happen. */
		}
		if (debug)
			fprintf(stderr, "Provider connected.\n");

		connected = 1;

		while (connected) {
			if ((recv(s, oob, 32, MSG_OOB | MSG_DONTWAIT) > 0) && strncmp(oob, "flush", 5))
				fprintf(stderr, "OUT-OF-BAND MESSAGE 1");

			memset(buffer, 0, BUFFERSIZE + 4);
			i = recv(s, buffer, BUFFERSIZE, 0);
			if (i == -1 || i == 0) {
				if ((i == -1) && (errno != EAGAIN) && (errno != EINTR)) {
					if (junk != 0) {
						fprintf(stderr, "%d bytes\n", junk);
						junk = 0;
					}
					/* wait for a new connection */
					perror("Lost provider connection");
					close(s);
					connected = 0;
				}
				/* see if provider is still there */
				i = sock_write_bytes(s, (unsigned char *)"ping", 4);
				/*
				 * We get tcp acks, so there's no need to send a pong from the provider
				 */
				if ((i == -1) && (errno != EINTR)) {
					if (junk != 0) {
						fprintf(stderr, "%d bytes\n", junk);
						junk = 0;
					}
					/* wait for a new connection */
					perror("Lost provider connection");
					close(s);
					connected = 0;
				}
				nanosleep(&waittime, NULL);
			} else {
				process(s, buffer, dev, devnum);
				close(s);
				connected = 0;
			}
		}
	}
}

int *socket_init(char *bind_arg)
{
	int *s;
	int on = 1;
	size_t mtu = ETHERMTU;
	struct sockaddr_in addr;
	uint32_t bind_addr = 0;
	int result = 0;

	/* bind a socket to listen on */
	s = (int *)malloc(sizeof(int));
	if (s == NULL)
		return (NULL);

	/* locate socket */
	*s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*s == -1) {
		perror("Socket cannot be opened");
		free(s);
		return (NULL);
	}

	/* set socket options */
	if (setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int)) == -1) {
		perror("Socket option cannot be set");
		goto socket_error;
	}
	if (setsockopt(*s, SOL_SOCKET, SO_RCVBUF, &mtu, sizeof(size_t)) == -1) {
		perror("Socket option cannot be set");
		goto socket_error;
	}

	/* set socket essentials */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listenport);
	if (bind_arg != 0) {
		printf("Try to bind to %s\n", bind_arg);
		result = inet_pton(AF_INET, bind_arg, (void *)&bind_addr);
		if (result < 0) {
			perror("Inet_pton for given bind address failed");
			goto socket_error;
		} else if (result == 0) {
			fprintf(stderr, "Given bind address is not a valid IPv4 address: %s\n", bind_arg);
			goto socket_error;
		}
	} else {
		bind_addr = INADDR_ANY;
	}
	addr.sin_addr.s_addr = (uint32_t)bind_addr;

	/* bind socket now */
	if (bind(*s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		perror("Bind failed");
		goto socket_error;
	}

	return (s);

socket_error:
	close(*s);
	return NULL;
}
#endif // !WEBLESS
