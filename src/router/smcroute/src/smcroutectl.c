/* Client for smcrouted, not needed if only using smcroute.conf
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"

#include <err.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sysexits.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "msg.h"
#include "util.h"

static const char version_info[] = PACKAGE_NAME " v" PACKAGE_VERSION;

static char *ident = PACKAGE;
static char *sock_file = NULL;
static char *prognm = NULL;
static int   heading = 1;
static int   detail = 0;
static int   plain = 0;
static int   help = 0;

struct arg {
	char *name;
	int   min_args;		/* 0: command takes no arguments */
	int   val;
	char *arg;
	char *help;
	char *example;		/* optional */
	int   has_detail;
} args[] = {
	{ NULL,      0, 'b', NULL,   "Batch mode, read commands from stdin", NULL, 0 },
	{ NULL,      0, 'd', NULL,   "Detailed output in show command", NULL, 0 },
	{ NULL,      1, 'i', "NAME", "Identity of routing daemon instance, default: " PACKAGE, "foo", 0 },
	{ NULL,      1, 'I', "NAME", NULL, NULL, 0 }, /* Alias, compat with older versions */
	{ NULL,      0, 'p', NULL,   "Use plain table headings, no ctrl chars", NULL, 0 },
	{ NULL,      0, 't', NULL,   "Skip table heading in show command", NULL, 0 },
	{ NULL,      1, 'u', "FILE", "UNIX domain socket for daemon, default: " RUNSTATEDIR "/" PACKAGE ".sock", "/tmp/foo.sock", 0 },
	{ "help",    0, 'h', NULL,   "Show help text", NULL, 0 },
	{ "version", 0, 'v', NULL,   "Show program version and support information", NULL, 0 },
	{ "flush" ,  0, 'F', NULL,   "Flush all dynamically installed (*,G) multicast routes", NULL, 0 },
	{ "kill",    0, 'k', NULL,   "Kill running daemon", NULL, 0 },
	{ "reload",  0, 'H', NULL,   "Reload .conf file, like SIGHUP", NULL, 0 },
	{ "restart", 0, 'H', NULL,   NULL, NULL, 0 }, /* Alias, compat with older versions */
	{ "show",    0, 's', NULL,   "Show status of routes, joined groups, interfaces, etc.", NULL, 1 },
	{ "add",     3, 'a', NULL,   "Add a multicast route",    "eth0 192.168.2.42 225.1.2.3 eth1 eth2", 0 },
	{ "remove",  2, 'r', NULL,   "Remove a multicast route", "eth0 192.168.2.42 225.1.2.3", 0 },
	{ "del",     2, 'r', NULL,   NULL, NULL, 0 }, /* Alias */
	{ "join",    2, 'j', NULL,   "Join multicast group on an interface", "eth0 225.1.2.3", 0 },
	{ "leave",   2, 'l', NULL,   "Leave joined multicast group",         "eth0 225.1.2.3", 0 },
	{ NULL, 0, 0, NULL, NULL, NULL, 0 }
};


/*
 * Build IPC message to send to the daemon using @cmd and @count
 * number of arguments from @argv.
 */
static struct ipc_msg *msg_create(uint16_t cmd, char *argv[], size_t count)
{
	struct ipc_msg *msg;
	size_t len = 0;
	size_t i, sz;
	char *ptr;

	for (i = 0; i < count; i++)
		len += strlen(argv[i]) + 1;

	sz = sizeof(struct ipc_msg) + len + 1;
	if (sz > MX_CMDPKT_SZ) {
		errno = EMSGSIZE;
		return NULL;
	}

	msg = calloc(1, sz);
	if (!msg)
		return NULL;

	msg->len   = sz;
	msg->cmd   = cmd;
	msg->count = count;

	ptr = (char *)msg->argv;
	for (i = 0; i < count; i++) {
		len = strlen(argv[i]) + 1;
		ptr = memcpy(ptr, argv[i], len) + len;
	}
	*ptr = '\0';	/* '\0' behind last string */

	return msg;
}

#define ESC "\033"
static int get_width(void)
{
	int ret = 79;
#ifdef HAVE_TERMIOS_H
	struct pollfd fd = { STDIN_FILENO, POLLIN, 0 };
	struct termios tc, saved;
	char buf[42];

	memset(buf, 0, sizeof(buf));
	tcgetattr(STDERR_FILENO, &tc);
	saved = tc;
	tc.c_cflag |= (CLOCAL | CREAD);
	tc.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tcsetattr(STDERR_FILENO, TCSANOW, &tc);
	fprintf(stderr, ESC "7" ESC "[r" ESC "[999;999H" ESC "[6n");

	if (poll(&fd, 1, 300) > 0) {
		int row, col;

		if (scanf(ESC "[%d;%dR", &row, &col) == 2)
			ret = col;
	}

	fprintf(stderr, ESC "8");
	tcsetattr(STDERR_FILENO, TCSANOW, &saved);
#endif
	return ret;
}

static void print(char *line, int indent)
{
	int type = 0;
	int i, len;

	chomp(line);

	/* Table headings, or repeat headers, end with a '=' */
	len = (int)strlen(line) - 1;
	if (len > 0) {
		if (line[len] == '_')
			type = 1;
		if (line[len] == '=')
			type = 2;

		if (type) {
			if (!heading)
				return;
			line[len] = 0;
		}
	}

	switch (type) {
	case 1:
		if (!plain) {
			fprintf(stdout, "\e[4m%*s\e[0m\n%s\n", get_width(), "", line);
			return;

		}

		len = len < 79 ? 79 : len;
		for (i = 0; i < len; i++)
			fputc('_', stdout);
		fprintf(stdout, "\n%*s%s\n", indent, "", line);
		break;

	case 2:
		if (!plain) {
			len = get_width() - len;
			fprintf(stdout, "\e[7m%s%*s\e[0m\n", line, len, "");
			return;
		}

		len = len < 79 ? 79 : len;
		for (i = 0; i < len; i++)
			fputc('=', stdout);
		fprintf(stdout, "\n%*s%s\n", indent, "", line);
		for (i = 0; i < len; i++)
			fputc('=', stdout);
		fputs("\n", stdout);
		break;

	default:
		puts(line);
		break;
	}
}

/*
 * Connects to the IPC socket of the server
 */
static int ipc_connect(char *path)
{
	struct sockaddr_un sa;
	socklen_t len;
	int sd;

	sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd < 0)
		return -1;

#ifdef HAVE_SOCKADDR_UN_SUN_LEN
	sa.sun_len = 0;	/* <- correct length is set by the OS */
#endif
	sa.sun_family = AF_UNIX;
	if (!path)
		snprintf(sa.sun_path, sizeof(sa.sun_path), "%s/%s.sock", RUNSTATEDIR, ident);
	else
		snprintf(sa.sun_path, sizeof(sa.sun_path), "%s", path);

	len = offsetof(struct sockaddr_un, sun_path) + strlen(sa.sun_path);
	if (connect(sd, (struct sockaddr *)&sa, len) < 0) {
		int err = errno;

		if (ENOENT == errno)
			warnx("Cannot find IPC socket %s", sa.sun_path);

		close(sd);
		errno = err;

		return -1;
	}

	return sd;
}

static int ipc_command(uint16_t cmd, char *argv[], size_t count)
{
	char buf[MX_CMDPKT_SZ + 1];
	struct ipc_msg *msg;
	int retries = 30;
	int result = 0;
	ssize_t total;
	ssize_t len;
	FILE *fp;
	int sd;

	msg = msg_create(cmd, argv, count);
	if (!msg) {
		warn("Failed constructing IPC command");
		return 1;
	}

	while ((sd = ipc_connect(sock_file)) < 0) {
		switch (errno) {
		case EACCES:
			warnx("Need root privileges to connect to daemon");
			break;

		case ECONNREFUSED:
			if (--retries) {
				usleep(100000);
				continue;
			}

			warnx("Daemon not running");
			break;

		case ENOENT:
			if (!sock_file) {
				warnx("Daemon may be running with another -i NAME");
				break;
			}
			/* fallthrough */

		default:
			warn("Failed connecting to daemon");
			break;
		}

		free(msg);
		return 1;
	}

	/* Send command */
	if (write(sd, msg, msg->len) != (ssize_t)msg->len) {
		warn("Communication with daemon failed");
		close(sd);
		free(msg);

		return 1;
	}

	fp = tempfile();
	if (!fp) {
		close(sd);
		free(msg);
		err(EX_OSERR, "Failed creating tempfile()");
	}

	total = 0;
	while ((len = read(sd, buf, sizeof(buf) - 1)) > 0) {
		total += len;
		buf[len] = 0;
		fwrite(buf, len, 1, fp);
	}
	rewind(fp);

	if (total > 1) {
		if (cmd == 'S' || cmd == 's') {
			while (fgets(buf, sizeof(buf), fp))
				print(buf, 0);
		} else {
			if (fgets(buf, sizeof(buf), fp))
				warnx("%s", buf);
			result = 1;
		}
	}

	fclose(fp);
	close(sd);
	free(msg);

	return result;
}

static int usage(int code)
{
	int i;

	printf("Usage:\n  %s [OPTIONS] CMD [ARGS]\n\n", prognm);

	printf("Options:\n");
	for (i = 0; args[i].val; i++) {
		if (!args[i].help)
			continue;

		if (args[i].name)
			continue;

		printf("  -%c %-10s %s\n", args[i].val, args[i].arg ? args[i].arg : "", args[i].help);
	}

	printf("\nCommands:\n");
	for (i = 0; args[i].val; i++) {
		if (!args[i].help)
			continue;

		if (!args[i].name)
			continue;

		printf("  %-7s %s  %s\n", args[i].name,
		       args[i].min_args ? "ARGS" : "    ", args[i].help);
	}

	printf("\nArguments:\n"
	       "         <---------- INBOUND ------------>  <- OUTBOUND ->\n"
	       "  add    IIF [SOURCE-IP[/LEN]] GROUP[/LEN]  OIF [OIF ... ]\n"
	       "  remove IIF [SOURCE-IP[/LEN]] GROUP[/LEN]\n"
	       "\n"
	       "  join   IIF [SOURCE-IP[/LEN]] GROUP[/LEN]\n"
	       "  leave  IIF [SOURCE-IP[/LEN]] GROUP[/LEN]\n"
	       "\n"
	       "  show   interfaces    Show configured multicast interfaces\n"
	       "  show   groups        Show joined multicast groups\n"
	       "  show   routes        Show (*,G) and (S,G) multicast routes, default\n"
	       "\n"
	       "Note:\n"
	       "  Inbound (IIF) and outbound (OIF) interfaces can be either an interface\n"
	       "  name or a wildcard.  E.g., \"eth+\" matches eth0, eth15, etc.\n"
	       "\n");

	return code;
}

static int version(void)
{
	puts(version_info);
	printf("\n"
	       "Bug report address: %s\n", PACKAGE_BUGREPORT);
#ifdef PACKAGE_URL
	printf("Project homepage:   %s\n", PACKAGE_URL);
#endif
	return 0;
}

static int parse(int pos, int argc, char *argv[])
{
	struct arg *cmd = NULL;
	int status = 0;
	int c;

	while (pos < argc && !cmd) {
		char *arg = argv[pos];
		int i;

		for (i = 0; args[i].val; i++) {
			char    *nm = args[i].name;
			size_t  len;

			if (!nm)
				continue;

			len = MIN(strlen(nm), strlen(arg));
			if (strncmp(arg, nm, len))
				continue;

			c = args[i].val;
			switch (c) {
			case 'h':
				help++;
				break;

			case 'v':
				return version();

			default:
				cmd = &args[i];
				if (help)
					goto help;
				if (argc - (pos + 1) < args[i].min_args) {
					warnx("Not enough arguments to command %s", nm);
					status = 1;
					goto help;
				}
				break;
			}

			break;	/* Next arg */
		}
		pos++;
	}

	if (help) {
		if (!cmd)
			return usage(0);
	help:
		while (!cmd->help)
			cmd--;
		printf("Help:\n"
		       "  %s\n\n"
		       "Example:\n"
		       "  %s %s %s\n\n", cmd->help,
		       prognm, cmd->name, cmd->example ? cmd->example : "");
		return status;
	}

	if (!cmd)
		return ipc_command(detail ? 'S' : 's', NULL, 0);

	c = cmd->val;
	if (detail && cmd->has_detail)
		c -= 0x20;

	return ipc_command(c, &argv[pos], argc - pos);
}

static int batch(void)
{
	char line[512];
	int rc = 0;

	while (fgets(line, sizeof(line), stdin)) {
		char *ptr, *token, *args[10];
		int num = 0;

		ptr = chomp(line);
		if (ptr[0] == '#')
			continue;

		while (num < 9 && (token = strsep(&ptr, " \t")))
			args[num++] = token;

		if (!num)
			continue;

		rc += parse(0, num, args);
	}

	return rc;
}

static char *progname(const char *arg0)
{
	char *nm;

	nm = strrchr(arg0, '/');
	if (nm)
		nm++;
	else
		nm = (char *)arg0;

	return nm;
}

int main(int argc, char *argv[])
{
	int batch_mode = 0;
	int c;

	prognm = progname(argv[0]);
	while ((c = getopt(argc, argv, "bdhI:i:ptu:v")) != EOF) {
		switch (c) {
		case 'b':
			batch_mode = 1;
			break;

		case 'd':
			detail++;
			break;

		case 'h':
			help++;
			break;

		case 'I':	/* compat with previous versions */
		case 'i':
			ident = optarg;
			break;

		case 'p':
			plain = 1;
			break;

		case 't':
			heading = 0;
			break;

		case 'u':
			sock_file = optarg;
			break;

		case 'v':
			return version();

		default:
			return usage(1);
		}
	}

	if (batch_mode)
		return batch();

	return parse(optind, argc, argv);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
