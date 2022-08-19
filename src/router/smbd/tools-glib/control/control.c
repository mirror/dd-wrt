// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <getopt.h>
#include <fcntl.h>
#include <errno.h>

#include "ksmbdtools.h"
#include "version.h"

static void usage(int status)
{
	fprintf(stderr,
		"Usage: ksmbd.control {-s | -d COMPONENT | -c | -V | -h}\n");

	if (status != EXIT_SUCCESS)
		fprintf(stderr, "Try 'ksmbd.control --help' for more information.\n");
	else
		fprintf(stderr,
			"Control ksmbd.mountd user mode and ksmbd kernel mode daemons.\n"
			"\n"
			"Mandatory arguments to long options are mandatory for short options too.\n"
			"  -s, --shutdown           shutdown ksmbd.mountd and ksmbd and exit\n"
			"  -d, --debug=COMPONENT    toggle debug printing for COMPONENT and exit;\n"
			"                           COMPONENT is 'all', 'smb', 'auth', 'vfs',\n"
			"                           'oplock', 'ipc', 'conn', or 'rdma';\n"
			"                           output also status of all components;\n"
			"                           enabled components are enclosed in brackets\n"
			"  -c, --ksmbd-version      output ksmbd version information and exit\n"
			"  -V, --version            output version information and exit\n"
			"  -h, --help               display this help and exit\n"
			"\n"
			"ksmbd-tools home page: <https://github.com/cifsd-team/ksmbd-tools>\n");
}

static const struct option opts[] = {
	{"shutdown",		no_argument,		NULL,	's' },
	{"debug",		required_argument,	NULL,	'd' },
	{"ksmbd-version",	no_argument,		NULL,	'c' },
	{"version",		no_argument,		NULL,	'V' },
	{"help",		no_argument,		NULL,	'h' },
	{NULL,			0,			NULL,	 0  }
};

static int show_version(void)
{
	printf("ksmbd-tools version : %s\n", KSMBD_TOOLS_VERSION);
	return EXIT_SUCCESS;
}

static int ksmbd_control_shutdown(void)
{
	int fd, ret;

	terminate_ksmbd_daemon();

	fd = open("/sys/class/ksmbd-control/kill_server", O_WRONLY);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return fd;
	}

	ret = write(fd, "hard", 4);
	close(fd);
	return ret;
}

static int ksmbd_control_show_version(void)
{
	int fd, ret;
	char ver[255] = {0};

	fd = open("/sys/module/ksmbd/version", O_RDONLY);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return fd;
	}

	ret = read(fd, ver, 255);
	close(fd);
	if (ret != -1)
		pr_info("ksmbd version : %s\n", ver);
	return ret;
}

static int ksmbd_control_debug(char *comp)
{
	int fd, ret;
	char buf[255] = {0};

	fd = open("/sys/class/ksmbd-control/debug", O_RDWR);
	if (fd < 0) {
		pr_err("open failed: %d\n", errno);
		return fd;
	}

	ret = write(fd, comp, strlen(comp));
	if (ret < 0)
		goto out;
	ret = lseek(fd, 0, SEEK_SET);
	if (ret < 0)
		goto out;
	ret = read(fd, buf, 255);
	if (ret < 0)
		goto out;

	pr_info("%s\n", buf);
out:
	close(fd);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int c;

	set_logger_app_name("ksmbd.control");

	if (getuid() != 0) {
		pr_err("Please try it as root.\n");
		return ret;
	}

	while ((c = getopt_long(argc, argv, "sd:cVh", opts, NULL)) != EOF)
		switch (c) {
		case 's':
			ksmbd_control_shutdown();
			goto out;
		case 'd':
			ret = ksmbd_control_debug(optarg);
			goto out;
		case 'c':
			ret = ksmbd_control_show_version();
			goto out;
		case 'V':
			ret = show_version();
			goto out;
		case 'h':
			ret = EXIT_SUCCESS;
			/* Fall through */
		case '?':
		default:
			usage(ret);
			goto out;
		}

	if (argc < 2 || argc > optind)
		usage(ret);
out:
	return ret;
}
