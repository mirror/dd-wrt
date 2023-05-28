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

/** @file ndsctl.c
    @brief Monitoring and control of opennds, client part
    @author Copyright (C) 2004 Alexandre Carmel-Veilleux <acv@acv.ca>
    @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
    @author Copyright (C) 2021 ndsctl_lock() and ndsctl_unlock() based on code by Linus Lüssing <ll@simonwunderlich.de>
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <ctype.h>

#include "ndsctl.h"
#include "common.h"

int safe_asprintf(char **strp, const char *fmt, ...);
int safe_vasprintf(char **strp, const char *fmt, va_list ap);

struct argument {
	const char *cmd;
	const char *ifyes;
	const char *ifno;
};

int safe_asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = safe_vasprintf(strp, fmt, ap);
	va_end(ap);

	return (retval);
}

int safe_vasprintf(char **strp, const char *fmt, va_list ap)
{
	int retval;

	retval = vasprintf(strp, fmt, ap);

	if (retval == -1) {
		printf("Failed: Memory allocation error");
		exit (1);
	}

	return (retval);
}

int b64decode(char *buf, int blen, const void *src, int slen)
{
	const unsigned char *str = src;
	unsigned int cout = 0;
	unsigned int cin = 0;
	int len = 0;
	int i = 0;

	for (i = 0; (i <= slen) && (str[i] != 0); i++) {
		cin = str[i];

		if ((cin >= '0') && (cin <= '9'))
			cin = cin - '0' + 52;
		else if ((cin >= 'A') && (cin <= 'Z'))
			cin = cin - 'A';
		else if ((cin >= 'a') && (cin <= 'z'))
			cin = cin - 'a' + 26;
		else if (cin == '+')
			cin = 62;
		else if (cin == '/')
			cin = 63;
		else if (cin == '=')
			cin = 0;
		else
			continue;

		cout = (cout << 6) | cin;

		if ((i % 4) != 3)
			continue;

		if ((len + 3) >= blen)
			break;

		buf[len++] = (char)(cout >> 16);
		buf[len++] = (char)(cout >> 8);
		buf[len++] = (char)(cout);
	}

	buf[len++] = 0;
	return len;
}

int b64encode(char *buf, int blen, const char *src, int slen)
{
	int  i;
	int  v;
	int len = 0;
	const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for (i=0, len=0; i<slen; i+=3, len+=4) {
		if ((len+4) <= blen) {
			v = src[i];
			v = i+1 < slen ? v << 8 | src[i+1] : v << 8;
			v = i+2 < slen ? v << 8 | src[i+2] : v << 8;

			buf[len]   = b64chars[(v >> 18) & 0x3F];
			buf[len+1] = b64chars[(v >> 12) & 0x3F];

			if (i+1 < slen) {
				buf[len+2] = b64chars[(v >> 6) & 0x3F];
			} else {
				buf[len+2] = '=';
			}

			if (i+2 < slen) {
				buf[len+3] = b64chars[v & 0x3F];
			} else {
				buf[len+3] = '=';
			}
		} else {
			break;
		}
	}

	return (i == slen) ? (len + 4) : -1;
}


/** @internal
 * @brief Print usage
 *
 * Prints usage, called when ndsctl is run with -h or with an unknown option
 */
static void
usage(void)
{
	printf(
		"Usage: ndsctl [options] command [arguments]\n"
		"\n"
		"options:\n"
		"  -s <path>           Deprecated and ignored - Path to the socket, config setting is used instead\n"
		"  -h                  Print usage\n"
		"\n"
		"commands:\n"
		"  status\n"
		"	View the status of opennds\n\n"
		"  json	mac|ip|token(optional)\n"
		"	Display client list in json format\n"
		"	mac|ip|token is optional, if not specified, all clients are listed\n\n"
		"  stop\n"
		"	Stop the running opennds\n\n"
		"  auth mac|ip|token sessiontimeout(minutes) uploadrate(kb/s) downloadrate(kb/s) uploadquota(kB) downloadquota(kB) customstring\n"
		"	Authenticate client with specified mac, ip or token\n"
		"\n"
		"	sessiontimeout sets the session duration. Unlimited if 0, defaults to global setting if null (double quotes).\n"
		"	The client will be deauthenticated once the sessiontimout period has passed.\n"
		"\n"
		"	uploadrate and downloadrate are the maximum allowed data rates. Unlimited if 0, global setting if null (\"\").\n"
		"\n"
		"	uploadquota and downloadquota are the maximum volumes of data allowed. Unlimited if 0, global setting if null (\"\").\n"
		"\n"
		"	customstring is a custom string that will be passed to BinAuth.\n"
		"\n"
		"	Example: ndsctl auth 10.13.1.138 1400 300 1500 500000 1000000 \"This is a Custom String\"\n"
		"\n"
		"  deauth mac|ip|token\n"
		"	Deauthenticate user with specified mac, ip or token\n\n"
		"  block mac\n"
		"	Block the given MAC address\n\n"
		"  unblock mac\n"
		"	Unblock the given MAC address\n\n"
		"  allow mac\n"
		"	Allow the given MAC address\n\n"
		"  unallow mac\n"
		"	Unallow the given MAC address\n\n"
		"  trust mac\n"
		"	Trust the given MAC address\n\n"
		"  untrust mac\n"
		"	Untrust the given MAC address\n\n"
		"  debuglevel n\n"
		"	Set debug level to n (0=silent, 1=Normal, 2=Info, 3=debug)\n\n"
		"  b64decode \"string_to_decode\"\n"
		"	Base 64 decode the given string\n\n"
		"  b64encode \"string_to_encode\"\n"
		"	Base 64 encode the given string\n"
		"\n"
	);
}

static struct argument arguments[] = {
	{"json", NULL, NULL},
	{"status", NULL, NULL},
	{"stop", NULL, NULL},
	{"debuglevel", "Debug level set to %s.\n", "Failed to set debug level to %s.\n"},
	{"deauth", "Client %s deauthenticated.\n", "Client %s not found.\n"},
	{"auth", "Client %s authenticated.\n", "Failed to authenticate client %s.\n"},
	{"block", "MAC %s blocked.\n", "Failed to block MAC %s.\n"},
	{"unblock", "MAC %s unblocked.\n", "Failed to unblock MAC %s.\n"},
	{"allow", "MAC %s allowed.\n", "Failed to allow MAC %s.\n"},
	{"unallow", "MAC %s unallowed.\n", "Failed to unallow MAC %s.\n"},
	{"trust", "MAC %s trusted.\n", "Failed to trust MAC %s.\n"},
	{"untrust", "MAC %s untrusted.\n", "Failed to untrust MAC %s.\n"},
	{"b64decode", NULL, NULL},
	{"b64encode", NULL, NULL},
	{NULL, NULL, NULL}
};

static const struct argument*
find_argument(const char *cmd) {
	int i;

	for (i = 0; arguments[i].cmd; i++) {
		if (strcmp(arguments[i].cmd, cmd) == 0) {
			return &arguments[i];
		}
	}

	return NULL;
}

static int
connect_to_server(const char sock_name[])
{
	int sock;
	char lockfile[] = "/tmp/ndsctl.lock";
	struct sockaddr_un sa_un;

	// Connect to socket
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	memset(&sa_un, 0, sizeof(sa_un));
	sa_un.sun_family = AF_UNIX;
	strncpy(sa_un.sun_path, sock_name, (sizeof(sa_un.sun_path) - 1));

	if (connect(sock, (struct sockaddr *)&sa_un, strlen(sa_un.sun_path) + sizeof(sa_un.sun_family))) {
		fprintf(stderr, "ndsctl: opennds probably not started (Error: %s)\n", strerror(errno));
		remove(lockfile);
		return -1;
	}

	return sock;
}

static int
send_request(int sock, const char request[])
{
	ssize_t len, written;

	len = 0;
	while (len != strlen(request)) {
		written = write(sock, (request + len), strlen(request) - len);
		if (written == -1) {
			fprintf(stderr, "Write to opennds failed: %s\n", strerror(errno));
			exit(1);
		}
		len += written;
	}

	return((int)len);
}

/* Perform a ndsctl action, with server response Yes or No.
 * Action given by cmd, followed by config.param.
 * Responses printed to stdout, as formatted by ifyes or ifno.
 * config.param interpolated in format with %s directive if desired.
 */
static int
ndsctl_do(const char *socket, const struct argument *arg, const char *param)
{
	int sock;
	char buffer[MAX_BUF] = {0};
	char request[MAX_BUF *4 /3] = {0};
	int len, rlen;
	int ret;

	//setlogmask(LOG_UPTO (LOG_NOTICE));
	sock = connect_to_server(socket);

	if (sock < 0) {
		return 3;
	}

	if (strlen(param) > 0) {
		snprintf(request, sizeof(request), "%s %s\r\n\r\n", arg->cmd, param);
	} else {
		snprintf(request, sizeof(request), "%s\r\n\r\n", arg->cmd);
	}

	len = send_request(sock, request);

	if (arg->ifyes && arg->ifno) {
		len = 0;
		memset(buffer, 0, sizeof(buffer));
		while ((len < sizeof(buffer)) && ((rlen = read(sock, (buffer + len),
			(sizeof(buffer) - len))) > 0)) {
			len += rlen;
		}

		if (rlen < 0) {
			fprintf(stderr, "ndsctl: Error reading socket: %s\n", strerror(errno));
			ret = 3;
		} else if (strcmp(buffer, "Yes") == 0) {
			printf(arg->ifyes, param);
			ret = 0;
		} else if (strcmp(buffer, "No") == 0) {
			printf(arg->ifno, param);
			ret = 1;
		} else {
			fprintf(stderr, "ndsctl: Error: opennds sent an abnormal reply.\n");
			ret = 2;
		}
	} else {
		while ((len = read(sock, buffer, sizeof(buffer) - 1)) > 0) {
			buffer[len] = '\0';
			printf("%s", buffer);
		}
		ret = 0;
	}

	shutdown(sock, 2);
	close(sock);
	return ret;
}

int ndsctl_lock(char *mountpoint, int *lockfd)
{
	char *lockfile;

	// Open or create the lock file
	safe_asprintf(&lockfile, "%s/ndsctl.lock", mountpoint);
	*lockfd = open(lockfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	free(lockfile);

	if (*lockfd < 0) {
		// Critical error - must report to syslog
		openlog ("ndsctl", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
		syslog (LOG_ERR, "CRITICAL ERROR - Unable to open [%s]", lockfile);
		closelog ();
		return 5;
	}


	if (lockf(*lockfd, F_TLOCK, 0) == 0) {
		return 0;
	} else {

		if (errno != EACCES && errno != EAGAIN) {
			// persistent error
			// This is a Critical error - must report to syslog
			openlog ("ndsctl", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
			syslog (LOG_ERR, "CRITICAL ERROR - Unable to create lock on [%s]", lockfile);
			closelog ();
			close(*lockfd);
			return 6;
		}

		// This is a normal operating state, no need to report to syslog
		// It is the responsibility of the caller to check return code and retry if required
		close(*lockfd);
		return 4;
	}
}

void ndsctl_unlock(int *lockfd)
{
	if (lockf(*lockfd, F_ULOCK, 0) < 0) {
		printf(" Error - Unable to Unlock ndsctl/n");
	}

	close(*lockfd);
}


int
main(int argc, char **argv)
{
	const struct argument* arg;
	char *socket;
	int i = 1;
	int counter;
	char args[256] = {0};
	char argi[128] = {0};
	char str_b64[QUERYMAXLEN] = {0};
	char mountpoint[128] = {0};
	char socket_file[128] = {0};
	char *cmd;
	int ret;
	int lockfd;
	FILE *fd;

	// check arguments and take action:

	if (argc <= i) {
		usage();
		return 1;
	}

	if (strcmp(argv[1], "-s") == 0) {
		if (argc >= 4) {
			// -s option is deprecated - using config setting instead
			//socket = strdup(argv[2]);
			i = 3;
		} else {
			usage();
			return 1;
		}
	}

	if (strcmp(argv[i], "b64decode") == 0) {
		if(argv[i+1] == NULL) {
			return 1;
		} else {
			b64decode(str_b64, sizeof(str_b64), argv[i+1], strlen(argv[i+1]));
			printf("%s", str_b64);
			return 0;
		}
	}

	if (strcmp(argv[i], "b64encode") == 0) {
		if(argv[i+1] == NULL) {
			return 1;
		} else {
			b64encode(str_b64, sizeof(str_b64), argv[i+1], strlen(argv[i+1]));
			printf("%s", str_b64);
			return 0;
		}
	}


	if (strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	}


	// Get the tempfs mountpoint
	safe_asprintf(&cmd, "/usr/lib/opennds/libopennds.sh tmpfs");
	fd = popen(cmd, "r");
	if (fd == NULL) {
		printf("Unable to open library - Terminating");
		pclose(fd);
		exit(1);
	}
	free(cmd);

	if(fgets(mountpoint, sizeof(mountpoint), fd) == NULL) {
		printf("Unable to get mountpoint - Terminating");
		pclose(fd);
		exit(1);
	}
	pclose(fd);

	//Create lock
	ret = ndsctl_lock(mountpoint, &lockfd);

	if (ret > 0) {
		return ret;
	} 

	// Get the configured socket filename if there is one
	safe_asprintf(&cmd, "/usr/lib/opennds/libopennds.sh get_option_from_config ndsctlsocket");
	fd = popen(cmd, "r");
	if (fd == NULL) {
		printf("Unable to open library - Terminating");
		exit(1);
	}
	free(cmd);

	if(fgets(socket_file, sizeof(socket_file), fd) == NULL) {
		// Not set in config, so using default socket
		strcat(socket_file, DEFAULT_SOCKET_FILENAME);
	}
	pclose(fd);

	// Construct the full socket path/filename
	safe_asprintf(&socket, "%s/%s", mountpoint, socket_file);

	// check arguments that need socket access and take action:
	arg = find_argument(argv[i]);

	if (arg == NULL) {
		fprintf(stderr, "Unknown command: %s\n", argv[i]);
		ndsctl_unlock(&lockfd);
		free(socket);
		return 1;
	}

	// Collect command line arguments then send the command
	if (argc > i+1) {
		snprintf(args, sizeof(args), "%s", argv[i+1]);

		for (counter=i+2; counter < argc; counter++) {
			snprintf(argi, sizeof(argi), ",%s", argv[counter]);
			strncat(args, argi, sizeof(args)-1);
		}
	}

	ret = ndsctl_do(socket, arg, args);
	ndsctl_unlock(&lockfd);
	free(socket);
	return ret;
}


