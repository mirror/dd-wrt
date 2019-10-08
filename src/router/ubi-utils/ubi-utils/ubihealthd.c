#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <mtd/ubi-user.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <sys/random.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

#define PROGRAM_NAME "ubihealthd"

#include "libubi.h"
#include "common.h"

#ifndef UBI_IOCRPEB
#define UBI_IOCRPEB _IOW(UBI_IOC_MAGIC, 4, int32_t)
#endif

struct peb_state {
	int alive;
	int pnum;
	int last_errno;
};

static struct peb_state **peb_state_array;
static int peb_state_array_len;
static int cur_pos;
static const char *ubi_device = "/dev/ubi0";
static int ubi_fd;
static int interval_secs = 120;
static int nodaemon;

static const char opt_string[] = "d:i:fh";
static const struct option options[] = {
        {
                .name = "device",
                .has_arg = required_argument,
                .flag = NULL,
                .val = 'd'
        },
        {
                .name = "interval",
                .has_arg = required_argument,
                .flag = NULL,
                .val = 'i'
        },
	{
		.name = "help",
		.has_arg = no_argument,
		.flag = NULL,
		.val = 'h'
	},
	{ /* sentinel */ }
};

static void dolog(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (nodaemon)
		vfprintf(stderr, fmt, ap);
	else
		vsyslog(LOG_DAEMON | LOG_WARNING, fmt, ap);
	va_end(ap);
}

static void build_peb_list(void)
{
	int i, pos;
	struct peb_state *ps;

	peb_state_array = xmalloc(sizeof(ps) * peb_state_array_len);

	for (i = 0; i < peb_state_array_len; i++) {
		ps = xmalloc(sizeof(*ps));

		ps->pnum = i;
		ps->last_errno = 0;
		ps->alive = 1;

		peb_state_array[i] = ps;
	}

	/* Shuffle the list */
	for (i = 0; i < peb_state_array_len; i++) {
		pos = rand() % peb_state_array_len;

		ps = peb_state_array[pos];
		peb_state_array[pos] = peb_state_array[i];
		peb_state_array[i] = ps;
	}
}

static struct peb_state *__next_peb(void)
{
	struct peb_state *ps = peb_state_array[cur_pos];

	cur_pos++;
	if (cur_pos >= peb_state_array_len)
		cur_pos = 0;

	return ps;
}

static struct peb_state *next_peb(void)
{
	int i;
	struct peb_state *ps;

	/* Find next PEB in our list, skip bad PEBs */
	for (i = 0; i < peb_state_array_len; i++) {
		ps = __next_peb();
		if (ps->alive)
			return ps;
	}

	dolog("Fatal: All PEBs are gone?!\n");
	exit(1);

	return NULL;
}

static int process_one_peb(void)
{
	int rc;
	struct peb_state *ps = next_peb();

	rc = ioctl(ubi_fd, UBI_IOCRPEB, &ps->pnum);
	if (!rc)
		return 0;
	else
		rc = errno;

	switch (rc) {
	case EINVAL: {
		dolog("Unable to check PEB %i for unknown reason!\n", ps->pnum);
		break;
	}
	case ENOENT: {
		/* UBI ignores this PEB */
		ps->alive = 0;
		break;
	}
	case EBUSY: {
		if (ps->last_errno == rc)
			dolog("Warning: Unable to check PEB %i\n", ps->pnum);
		break;
	}
	case EAGAIN: {
		if (ps->last_errno == rc)
			dolog("Warning: PEB %i has bitflips, but cannot scrub!\n", ps->pnum);
		break;
	}
	case EUCLEAN: {
		/* Scrub happened */
		break;
	}
	case ENOTTY: {
		dolog("Fatal: Kernel does not support this interface. Too old kernel?\n");
		exit(1);
		break;
	}
	case ENODEV: {
		dolog("Fatal: UBI device vanished under us.\n");
		exit(1);
	}
	default:
		dolog("Warning: Unknown return code from kernel: %i\n", rc);
	}

	ps->last_errno = rc;

	return 0;
}

static int get_peb_count(void)
{
	libubi_t libubi = libubi_open();
	struct ubi_dev_info dev_info;

	if (!libubi) {
		fprintf(stderr, "Unable to init libubi, is UBI present?\n");
		exit(1);
	}

	if (ubi_get_dev_info(libubi, ubi_device, &dev_info)) {
		fprintf(stderr, "Fatal: Could not get ubi info for %s\n", ubi_device);
		exit(1);
	}

	libubi_close(libubi);

	return dev_info.total_lebs;
}

static void init_prng(void)
{
	int ret, seed;

	ret = getrandom(&seed, sizeof(seed), 0);
	if (ret != sizeof(seed)) {
		if (ret == -1)
			fprintf(stderr, "Unable to get random seed: %m\n");
		else
			fprintf(stderr, "Unable to get %zi bytes random seed\n", sizeof(seed));

		exit(1);
	}
	srand(seed);
}

int main (int argc, char *argv[])
{
	int c, i;

	while ((c = getopt_long(argc, argv, opt_string, options, &i)) != -1) {
		switch(c) {
		case 'd': {
			ubi_device = optarg;
			break;
		}
		case 'i': {
			interval_secs = atoi(optarg);
			if (!interval_secs) {
				fprintf(stderr, "Bad interval value! %s\n", optarg);
				exit(1);
			}
			break;
		}
		case 'f': {
			nodaemon = 1;
			break;
		}
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [ -d UBI_DEVICE ] [-i INTERVAL_SEC ] [ -f ]\n", argv[0]);
			exit(1);
			break;
		}
	}

	ubi_fd = open(ubi_device, O_RDONLY);
	if (ubi_fd == -1) {
		fprintf(stderr, "Fatal: Unable to open %s: %m\n", ubi_device);
		exit(1);
	}

	init_prng();

	peb_state_array_len = get_peb_count();
	build_peb_list();

	if (!nodaemon) {
		if (daemon(0, 0) == -1) {
			fprintf(stderr, "Unable to become a daemon: %m\n");
			exit(1);
		}
	}

	for (;;) {
		process_one_peb();
		sleep(interval_secs);
	}

	return 0;
}
