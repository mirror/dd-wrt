/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/* @file ndsctl_thread.c
    @brief Monitoring and control of opennds, server part
    @author Copyright (C) 2004 Alexandre Carmel-Veilleux <acv@acv.ca>
    @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "common.h"
#include "util.h"
#include "conf.h"
#include "debug.h"
#include "auth.h"
#include "safe.h"
#include "client_list.h"
#include "fw_iptables.h"
#include "main.h"

#include "ndsctl_thread.h"
#include "http_microhttpd_utils.h"

#define MAX_EVENT_SIZE 30

// Defined in clientlist.c
extern pthread_mutex_t client_list_mutex;
extern pthread_mutex_t config_mutex;

static int ndsctl_handler(int fd);
static void ndsctl_block(FILE *fp, char *arg);
static void ndsctl_unblock(FILE *fp, char *arg);
static void ndsctl_allow(FILE *fp, char *arg);
static void ndsctl_unallow(FILE *fp, char *arg);
static void ndsctl_trust(FILE *fp, char *arg);
static void ndsctl_untrust(FILE *fp, char *arg);
static void ndsctl_auth(FILE *fp, char *arg);
static void ndsctl_deauth(FILE *fp, char *arg);
static void ndsctl_debuglevel(FILE *fp, char *arg);

static int socket_set_non_blocking(int sockfd);

/* Launches a thread that monitors the control socket for request
 @param arg Must contain a pointer to a string containing the Unix domain socket to open
 @todo This thread loops infinitely, need a watchdog to verify that it is still running?
 */
void*
thread_ndsctl(void *arg)
{
	int sock, fd, epoll_fd;
	const char *sock_name;
	struct sockaddr_un sa_un;
	socklen_t len;
	struct epoll_event ev;
	struct epoll_event *events;
	int current_fd_count;
	int number_of_count;
	int i;

	debug(LOG_DEBUG, "Starting ndsctl thread");

	memset(&sa_un, 0, sizeof(sa_un));
	sock_name = (char *)arg;
	debug(LOG_DEBUG, "Socket name: %s", sock_name);

	if (strlen(sock_name) > (sizeof(sa_un.sun_path) - 1)) {
		debug(LOG_ERR, "NDSCTL socket name too long");
		exit(1);
	}

	// Use AF_UNIX, not PF_UNIX, AF_LOCAL or PF_LOCAL
	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	// If socket file exists, delete it..
	unlink(sock_name);

	strcpy(sa_un.sun_path, sock_name);
	sa_un.sun_family = AF_UNIX;

	debug(LOG_DEBUG, "Binding socket [%s] Socket descriptor [%d]", sa_un.sun_path, sock);

	if (bind(sock, (struct sockaddr *)&sa_un, strlen(sock_name) + sizeof(sa_un.sun_family))) {
		debug(LOG_ERR, "Could not bind control socket: [%s] Terminating...", strerror(errno));
		pthread_exit(NULL);
	}

	if (listen(sock, 5)) {
		debug(LOG_ERR, "Could not listen on control socket: [%s] Terminating...", strerror(errno));
		pthread_exit(NULL);
	}

	memset(&ev, 0, sizeof(struct epoll_event));
	epoll_fd = epoll_create(MAX_EVENT_SIZE);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sock;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev) < 0) {
		debug(LOG_ERR, "Could not insert socket fd to epoll set: [%s] Terminating...", strerror(errno));
		pthread_exit(NULL);
	}

	events = (struct epoll_event*) calloc(MAX_EVENT_SIZE, sizeof(struct epoll_event));

	if (!events) {
		close(sock);
		pthread_exit(NULL);
	}

	current_fd_count = 1;

	debug(LOG_DEBUG, "Entering ndsctl thread loop");

	while (1) {
		memset(&sa_un, 0, sizeof(sa_un));
		len = (socklen_t) sizeof(sa_un);

		number_of_count = epoll_wait(epoll_fd, events, current_fd_count, -1);

		if (number_of_count == -1) {
			// interupted is not an error
			if (errno == EINTR)
				continue;

			debug(LOG_ERR, "Failed to wait epoll events: %s", strerror(errno));
			free(events);
			pthread_exit(NULL);
		}

		for (i = 0; i < number_of_count; i++) {

			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
					(!(events[i].events & EPOLLIN))) {
				debug(LOG_ERR, "Socket is not ready for communication : %s", strerror(errno));

				if (events[i].data.fd > 0) {
					shutdown(events[i].data.fd, 2);
					close(events[i].data.fd);
					events[i].data.fd = 0;
				}
				continue;
			}

			if (events[i].data.fd == sock) {
				if ((fd = accept(events[i].data.fd, (struct sockaddr *)&sa_un, &len)) == -1) {
					debug(LOG_ERR, "Accept failed on control socket: %s", strerror(errno));
					free(events);
					pthread_exit(NULL);
				} else {
					socket_set_non_blocking(fd);
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = fd;

					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
						debug(LOG_ERR, "Could not insert socket fd to epoll set: %s", strerror(errno));
						free(events);
						pthread_exit(NULL);
					}

					current_fd_count += 1;
				}

			} else {
				if (ndsctl_handler(events[i].data.fd)) {
					free(events);
					pthread_exit(NULL);
				}
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
				current_fd_count -= 1;

				// socket was closed on 'ndsctl_handler'
				if (events[i].data.fd > 0) {
					events[i].data.fd = 0;
				}
			}
		}
	}

	return NULL;
}

static int
ndsctl_handler(int fd)
{
	int done, i, ret = 0;
	char request[MAX_BUF];
	//ssize_t read_bytes, len;
	int read_bytes, len;
	FILE* fp;

	debug(LOG_DEBUG, "Entering thread_ndsctl_handler....");

	// Init variables
	read_bytes = 0;
	done = 0;
	memset(request, 0, sizeof(request));
	fp = fdopen(fd, "w");

	debug(LOG_DEBUG, "Read bytes and stuff from socket descriptor [%d], pointer [%lu]", fd, fp);

	// Read....
	while (!done && read_bytes < (sizeof(request) - 1)) {
		len = read(fd, request + read_bytes, sizeof(request) - read_bytes);

		// Have we gotten a command yet?
		for (i = read_bytes; i < (read_bytes + len); i++) {
			if (request[i] == '\r' || request[i] == '\n') {
				request[i] = '\0';
				done = 1;
			}
		}

		// Increment position
		read_bytes += len;
	}

	debug(LOG_DEBUG, "ndsctl request received: [%s]", request);

	if (strncmp(request, "status", 6) == 0) {
		ndsctl_status(fp);
	} else if (strncmp(request, "json", 4) == 0) {
		ndsctl_json(fp, (request + 5));
	} else if (strncmp(request, "stop", 4) == 0) {
		// tell the caller to stop the thread
		ret = 1;
	} else if (strncmp(request, "block", 5) == 0) {
		ndsctl_block(fp, (request + 6));
	} else if (strncmp(request, "unblock", 7) == 0) {
		ndsctl_unblock(fp, (request + 8));
	} else if (strncmp(request, "allow", 5) == 0) {
		ndsctl_allow(fp, (request + 6));
	} else if (strncmp(request, "unallow", 7) == 0) {
		ndsctl_unallow(fp, (request + 8));
	} else if (strncmp(request, "trust", 5) == 0) {
		ndsctl_trust(fp, (request + 6));
	} else if (strncmp(request, "untrust", 7) == 0) {
		ndsctl_untrust(fp, (request + 8));
	} else if (strncmp(request, "auth", 4) == 0) {
		ndsctl_auth(fp, (request + 5));
	} else if (strncmp(request, "deauth", 6) == 0) {
		ndsctl_deauth(fp, (request + 7));
	} else if (strncmp(request, "debuglevel", 10) == 0) {
		ndsctl_debuglevel(fp, (request + 11));
	}

	if (!done) {
		debug(LOG_ERR, "Invalid ndsctl request.");
	}

	debug(LOG_DEBUG, "ndsctl request processed: [%s]", request);
	debug(LOG_DEBUG, "Exiting thread_ndsctl_handler....");

	// Close and flush fp, also closes underlying fd
	fclose(fp);
	return ret;
}

static void
ndsctl_auth(FILE *fp, char *arg)
{
	s_config *config = config_get_config();
	t_client *client;
	unsigned id;
	int rc = -1;
	int seconds = 60 * config->session_timeout;
	int custom_seconds;
	int uploadrate = config->upload_rate;
	int downloadrate = config->download_rate;
	unsigned long long int uploadquota = config->upload_quota;
	unsigned long long int downloadquota = config->download_quota;
	char *libcmd;
	char *msg;
	char customdata[256] = {0};
	char *argcopy;
	const char *arg2;
	const char *arg3;
	const char *arg4;
	const char *arg5;
	const char *arg6;
	const char *arg7;
	const char *arg8;
	char *ptr;
	const char *ipclient;
	const char *macclient;
	time_t now = time(NULL);

	debug(LOG_DEBUG, "Entering ndsctl_auth [%s]", arg);

	argcopy=strdup(arg);

	// arg2 = ip|mac|tok
	arg2 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg2 [%s]", arg2);

	// arg3 = scheduled duration (minutes) until deauth
	arg3 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg3 [%s]", arg3);

	if (arg3 != NULL) {
		custom_seconds = 60 * strtol(arg3, &ptr, 10);
		if (custom_seconds > 0) {
			seconds = custom_seconds;
		}
	}
	debug(LOG_DEBUG, "Client session duration [%d] seconds", seconds);

	// arg4 = upload rate (kb/s)
	arg4 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg4 [%s]", arg4);

	if (arg4 != NULL) {
		uploadrate = strtol(arg4, &ptr, 10);
	}

	// arg5 = download rate (kb/s)
	arg5 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg5 [%s]", arg5);

	if (arg5 != NULL) {
		downloadrate = strtol(arg5, &ptr, 10);
	}

	// arg6 = upload quota (kB)
	arg6 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg6 [%s]", arg6);

	if (arg6 != NULL) {
		uploadquota = strtoll(arg6, &ptr, 10);
	}

	// arg7 = download quota (kB)
	arg7 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg7 [%s]", arg7);

	if (arg7 != NULL) {
		downloadquota = strtoll(arg7, &ptr, 10);
	}

	// arg8 = custom data string - max 256 characters
	arg8 = strsep(&argcopy, ",");
	debug(LOG_DEBUG, "arg8 [%s]", arg8);

	if (arg8 != NULL) {
	snprintf(customdata, sizeof(customdata), "%s", arg8);
	debug(LOG_DEBUG, "customdata [%s]", customdata);
	}

	LOCK_CLIENT_LIST();
	debug(LOG_DEBUG, "find in client list - arg2: [%s]", arg2);
	client = client_list_find_by_any(arg2, arg2, arg2);
	id = client ? client->id : 0;
	debug(LOG_DEBUG, "client id: [%d]", id);

	if (!id  && config->allow_preemptive_authentication == 1) {
		// Client is neither preauthenticated nor authenticated
		// If Preemptive authentication is enabled we should try to auth by mac
		debug(LOG_DEBUG, "Client is not in client list.");
		// Build command to get client mac and ip
		safe_asprintf(&libcmd, "/usr/lib/opennds/libopennds.sh clientaddress \"%s\"", arg2 );

		msg = safe_calloc(64);
		rc = execute_ret_url_encoded(msg, 64 - 1, libcmd);
		free(libcmd);

		if (rc == 0) {
			debug(LOG_DEBUG, "Client ip/mac: %s", msg);

			if (strcmp(msg, "-") == 0) {
				debug(LOG_DEBUG, "Client [%s] is not connected", arg2);
			} else {
				ipclient = strtok(msg, " ");
				macclient = strtok(NULL, " ");
				debug(LOG_DEBUG, "Client ip [%s], mac [%s]", ipclient, macclient);

				// check if client ip is on our subnet
				safe_asprintf(&libcmd, "/usr/lib/opennds/libopennds.sh get_interface_by_ip \"%s\"", ipclient);
				msg = safe_calloc(64);
				rc = execute_ret_url_encoded(msg, 64 - 1, libcmd);
				free(libcmd);

				if (rc == 0) {

					if (strcmp(config->gw_interface, msg) == 0) {
						debug(LOG_DEBUG, "Pre-emptive Authentication: Client [%s] is on our subnet using interface [%s]", ipclient, msg);

						client = client_list_add_client(macclient, ipclient);

						if (client) {
							id = client ? client->id : 0;
							debug(LOG_DEBUG, "client id: [%d]", id);
							client->client_type = "preemptive";

							// log the preemptive authentication
							safe_asprintf(&libcmd,
								"/usr/lib/opennds/libopennds.sh write_log \"mac=%s, ip=%s, client_type=%s\"",
								macclient,
								ipclient,
								client->client_type
							);

							msg = safe_calloc(64);
							rc = execute_ret_url_encoded(msg, 64 - 1, libcmd);
							free(libcmd);
						}

					} else {
						debug(LOG_NOTICE, "Pre-emptive Authentication: Client ip address [%s] is  NOT on our subnet", ipclient);
						id = 0;
					}
				} else {
					debug(LOG_DEBUG, "ip subnet test failed: Continuing...");
				}
			}
		} else {
			debug(LOG_DEBUG, "Client connection not found: Continuing...");
			rc = -1;
		}
	}

	if (id) {

		if (strcmp(fw_connection_state_as_string(client->fw_connection_state), "Preauthenticated") == 0) {
			// set client values
			client->session_start = now;

			if (seconds > 0) {
				client->session_end = now + seconds;
			} else {
				client->session_end = 0;
			}

			client->upload_rate = uploadrate;
			client->download_rate = downloadrate;
			client->upload_quota = uploadquota;
			client->download_quota = downloadquota;

			debug(LOG_DEBUG, "ndsctl_thread: client session start time [ %lu ], end time [ %lu ]", now, client->session_end);

			rc = auth_client_auth_nolock(id, "ndsctl_auth", customdata);
		}
	} else {
		// Client is neither preauthenticated nor authenticated
		// If Preemptive authentication is enabled we should have tried to auth by mac
		debug(LOG_DEBUG, "Client is not in client list.");
		rc = -1;
	}

	UNLOCK_CLIENT_LIST();

	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_auth...");
}

static void
ndsctl_deauth(FILE *fp, char *arg)
{
	t_client *client;
	unsigned id;
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_deauth [%s]", arg);

	LOCK_CLIENT_LIST();
	client = client_list_find_by_any(arg, arg, arg);
	id = client ? client->id : 0;
	UNLOCK_CLIENT_LIST();

	if (id) {
		rc = auth_client_deauth(id, "ndsctl_deauth");
	} else {
		debug(LOG_DEBUG, "Client not found.");
		rc = -1;
	}

	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_deauth...");
}

static void
ndsctl_block(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_block [%s]", arg);

	rc = auth_client_block(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_block.");
}

static void
ndsctl_unblock(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_unblock [%s]", arg);

	rc = auth_client_unblock(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_unblock.");
}

static void
ndsctl_allow(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_allow [%s]", arg);

	rc = auth_client_allow(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_allow.");
}

static void
ndsctl_unallow(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_unallow [%s]", arg);

	rc = auth_client_unallow(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_unallow.");
}

static void
ndsctl_trust(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_trust [%s]", arg);

	rc = auth_client_trust(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_trust.");
}

static void
ndsctl_untrust(FILE *fp, char *arg)
{
	int rc;

	debug(LOG_DEBUG, "Entering ndsctl_untrust [%s]", arg);

	rc = auth_client_untrust(arg);
	if (rc == 0) {
		fprintf(fp, "Yes");
	} else {
		fprintf(fp, "No");
	}

	debug(LOG_DEBUG, "Exiting ndsctl_untrust.");
}

static void
ndsctl_debuglevel(FILE *fp, char *arg)
{
	debug(LOG_DEBUG, "Entering ndsctl_debuglevel [%s]", arg);

	LOCK_CONFIG();

	if (!set_debuglevel(arg)) {
		fprintf(fp, "Yes");
		debug(LOG_NOTICE, "Set debug debuglevel to %s.", arg);
	} else {
		fprintf(fp, "No");
	}

	UNLOCK_CONFIG();

	debug(LOG_DEBUG, "Exiting ndsctl_debuglevel.");
}

static int
socket_set_non_blocking(int sockfd)
{
	int rc;

	rc = fcntl(sockfd, F_GETFL, 0);

	if (rc) {
		rc |= O_NONBLOCK;
		rc = fcntl(sockfd, F_SETFL, rc);
	}

	return rc;
}
