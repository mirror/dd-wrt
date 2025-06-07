/*
 * hugetop.c - utility to display system/process huge page information.
 *
 * zhenwei pi <pizhenwei@bytedance.com>
 *
 * Copyright © 2024      zhenwei pi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#include "c.h"
#include "fileutils.h"
#include "meminfo.h"
#include "nls.h"
#include "pids.h"
#include "strutils.h"
#include "units.h"

struct hg_state {
	unsigned long size;	/* KB */
	unsigned long nr_hugepages;
	unsigned long free_hugepages;
};

struct node_hg_states {
	char node[8];	/* nodeX, 8 bytes would be good enough */
	unsigned int nr_hg_state;
	struct hg_state *state;
};

struct nodes_hg_states {
	unsigned int nr_nodes;
	struct node_hg_states *nodes;
};

#define DEFAULT_COLS 80
#define DEFAULT_ROWS 24
static unsigned short cols = DEFAULT_COLS;
static unsigned short rows = DEFAULT_ROWS;

static int run_once;
static int numa;
static int human;
static struct termios saved_tty;
static long delay = 3;

enum pids_item Items[] = {
	PIDS_ID_PID,
	PIDS_CMD,
	PIDS_SMAP_HUGE_TLBPRV,
	PIDS_SMAP_HUGE_TLBSHR
};
#define ITEMS_COUNT (sizeof Items / sizeof *Items)

enum rel_items {
	EU_PID, EU_CMD, EU_SMAP_HUGE_TLBPRV, EU_SMAP_HUGE_TLBSHR
};

#define PIDS_GETINT(e) PIDS_VAL(EU_ ## e, s_int, stack)
#define PIDS_GETUNT(e) PIDS_VAL(EU_ ## e, u_int, stack)
#define PIDS_GETULL(e) PIDS_VAL(EU_ ## e, ull_int, stack)
#define PIDS_GETSTR(e) PIDS_VAL(EU_ ## e, str, stack)
#define PIDS_GETSCH(e) PIDS_VAL(EU_ ## e, s_ch, stack)
#define PIDS_GETSTV(e) PIDS_VAL(EU_ ## e, strv, stack)
#define PIDS_GETFLT(e) PIDS_VAL(EU_ ## e, real, stack)

static void setup_hugepage()
{
	struct meminfo_info *mem_info = NULL;
	struct stat statbuf;
	int ret;

	/* 1, verify "Hugepagesize" from /proc/meminfo. is huge pages supported on kernel building? */
	ret = procps_meminfo_new(&mem_info);
	if (ret) {
		fputs("Huge page not found or not supported", stdout);
		exit(-ret);
	}

	if (!MEMINFO_GET(mem_info, MEMINFO_MEM_HUGE_SIZE, ul_int)) {
		fputs("Huge page not found or not supported", stdout);
		exit(ENOTSUP);
	}

	procps_meminfo_unref(&mem_info);

	/* 2, verify /sys/devices/system/node/node<ID>/hugepages/hugepages-<size>/
	 *    per NUMA node huge page attributes got supported since Linux-v2.6.27(Dec-14-2009),
	 *    hugetop would not work on a lower kernel.
	 *    see Linux commit: 9a30523066cde7 ("hugetlb: add per node hstate attributes")
	 */
	ret = stat("/sys/devices/system/node/node0/hugepages", &statbuf);
	if (ret) {
		fputs("Per NUMA node huge page attributes not supported", stdout);
		exit(-ret);
	}
}

/*
 * term_size - set the globals 'cols' and 'rows' to the current terminal size
 */
static void term_size(int unusused __attribute__ ((__unused__)))
{
	struct winsize ws;

	if ((ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) && ws.ws_row > 10) {
		cols = ws.ws_col;
		rows = ws.ws_row;
	} else {
		cols = DEFAULT_COLS;
		rows = DEFAULT_ROWS;
	}
	if (run_once)
		rows = USHRT_MAX;
}

static void sigint_handler(int unused __attribute__ ((__unused__)))
{
	delay = 0;
}

static void parse_input(char c)
{
	switch (c) {
	case 'q':
	case 'Q':
		delay = 0;
		break;

	case 'n':
		numa = !numa;
		break;

	case 'H':
		human = !human;
		break;
	}
}

#define SYS_NODES "/sys/devices/system/node"

static unsigned long hg_read_attribute(const char *dir, const char *hg, const char *attr)
{
	char path[PATH_MAX] = { 0 };
	char buf[64] = { 0 };
	int fd;

	snprintf(path, sizeof(path), "%s/%s/%s", dir, hg, attr);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		printf("Failed to open %s\n", path);
		resizeterm(rows, cols);
		exit(errno);
	}

	if (read(fd, buf, sizeof(buf)) == -1) {
		printf("Failed to read %s\n", path);
		resizeterm(rows, cols);
		exit(errno);
	}

	close(fd);

	return atol(buf);
}

static int hg_state_cmp(const void *p1, const void *p2)
{
	const struct hg_state *state1 = p1;
	const struct hg_state *state2 = p2;

	return state1->size > state2->size;
}

static void hg_states_one_node(struct node_hg_states *node, const char *name)
{
	DIR *hg_dir;
	struct dirent *hg;
	char path[PATH_MAX] = { 0 };
	struct hg_state *state;

	memset(node, 0x00, sizeof(*node));
	strncpy(node->node, name, sizeof(node->node) - 1);

	/* Ex, scan /sys/devices/system/node/node0/hugepages */
	snprintf(path, sizeof(path), "%s/%s/hugepages", SYS_NODES, name);
	hg_dir = opendir(path);
	if (!hg_dir) {
		printf("Failed to open %s\n", path);
		resizeterm(rows, cols);
		exit(errno);
	}

	while ((hg = readdir(hg_dir))) {
		if (memcmp(hg->d_name, "hugepages-", 10))
			continue;

		node->state = realloc(node->state,
					sizeof(struct hg_state) * (node->nr_hg_state + 1));
		state = &node->state[node->nr_hg_state];
		node->nr_hg_state++;

		sscanf(hg->d_name, "hugepages-%ldkB", &state->size);
		state->nr_hugepages = hg_read_attribute(path, hg->d_name, "nr_hugepages");
		state->free_hugepages = hg_read_attribute(path, hg->d_name, "free_hugepages");
	}

	/* make sure the result in order */
	if (node->nr_hg_state > 1)
		qsort(node->state, node->nr_hg_state, sizeof(struct hg_state), hg_state_cmp);

	closedir(hg_dir);
}

/*
 * scan /sys/devices/system/node/node<ID>/hugepages/hugepages-<size>/
 */
static void hg_states_new(struct nodes_hg_states *nodes)
{
	DIR *nodes_dir;
	struct dirent *dirent;
	struct node_hg_states *node;

	nodes_dir = opendir(SYS_NODES);
	if (!nodes_dir) {
		fputs("Failed to open " SYS_NODES, stdout);
		resizeterm(rows, cols);
		exit(errno);
	}

	while ((dirent = readdir(nodes_dir))) {
		if ((dirent->d_type != DT_DIR) || memcmp(dirent->d_name, "node", 4))
			continue;

		nodes->nodes = realloc(nodes->nodes,
					 sizeof(struct node_hg_states) * (nodes->nr_nodes + 1));
		node = &nodes->nodes[nodes->nr_nodes];
		nodes->nr_nodes++;

		/* Ex, scan  /sys/devices/system/node/node0 */
		hg_states_one_node(node, dirent->d_name);
	}

	closedir(nodes_dir);
}

static void hg_states_free(struct nodes_hg_states *states)
{
	for (int n = 0; n < states->nr_nodes; n++) {
		struct node_hg_states *node = &states->nodes[n];

		free(node->state);
	}

	free(states->nodes);
}

#define PRINT_line(fmt, ...) if (run_once) printf(fmt, __VA_ARGS__); else printw(fmt, __VA_ARGS__)

static void print_node(struct node_hg_states *node, int numa)
{
	struct hg_state *state;
	char *line = calloc(cols, sizeof(char));
	int bytes;

	/* start build per node huge pages line. 'nodeX:' or 'node(s):' */
	if (numa)
		bytes = snprintf(line, cols, "%s:", node->node);
	else
		bytes = snprintf(line, cols, "node(s):");

	/* append ' 2.0Mi - xxx/yyy, 1.0Gi - mmm/nnn' */
	for (int i = 0; i < node->nr_hg_state; i++) {
		state = &node->state[i];
		bytes += snprintf(line + bytes, cols - bytes, " %s - %ld/%ld",
				scale_size(state->size, 3, 0, 1),
				state->free_hugepages, state->nr_hugepages);
		if (bytes >= cols)
			break;

		if (i < node->nr_hg_state - 1) {
			bytes += snprintf(line + bytes, cols - bytes, ",");
			if (bytes >= cols)
				break;
		}
	}

	PRINT_line("%s\n", line);

	free(line);
}

static void print_summary(void)
{
	struct nodes_hg_states nodes = { 0 };
	struct node_hg_states *node, *node0;
	time_t now;

	now = time(NULL);
	PRINT_line("%s - %s", program_invocation_short_name, ctime(&now));

	hg_states_new(&nodes);

	if (numa) {
		for (int n = 0; n < nodes.nr_nodes; n++) {
			node = &nodes.nodes[n];
			print_node(node, numa);
		}
	} else {
		/* merge node[1-n] into node[0] */
		node0 = &nodes.nodes[0];
		for (int n = 1; n < nodes.nr_nodes; n++) {
			node = &nodes.nodes[n];
			for (int i = 0; i < node->nr_hg_state; i++) {
				struct hg_state *state = &node->state[i];
				struct hg_state *state0 = &node0->state[i];

				state0->nr_hugepages += state->nr_hugepages;
				state0->free_hugepages += state->free_hugepages;
			}
		}

		print_node(node0, numa);
	}

	hg_states_free(&nodes);
}

static void print_headings(void)
{
	PRINT_line("%-78s\n", _("     PID     SHARED    PRIVATE COMMAND"));
}

static void print_procs(void)
{
	struct pids_info *info = NULL;
	struct pids_stack *stack;
	unsigned long long shared_hugepages;
	unsigned long long private_hugepages;
	char *line = calloc(cols, sizeof(char));
	int bytes = 0;

	procps_pids_new(&info, Items, ITEMS_COUNT);
	while ((stack = procps_pids_get(info, PIDS_FETCH_TASKS_ONLY))) {
		shared_hugepages = PIDS_GETULL(SMAP_HUGE_TLBSHR);
		private_hugepages = PIDS_GETULL(SMAP_HUGE_TLBPRV);

		/* no huge pages in use, skip it */
		if (shared_hugepages + private_hugepages == 0)
			continue;

		/* XXX: we can't PRINT_line("", scale_size(), scale_size()...), because scale_size()
		 *      uses a single static buffer, this statement overwrites buffer again.
		 */
		bytes = snprintf(line, cols, "%8d %10s",
				PIDS_GETINT(PID), scale_size(shared_hugepages, 2, 0, human));

		if (bytes < cols)
			snprintf(line + bytes, cols - bytes, " %10s %s",
				scale_size(private_hugepages, 2, 0, human), PIDS_GETSTR(CMD));

		PRINT_line("%s\n", line);
	}
	procps_pids_unref(&info);

	free(line);
}

static void __attribute__ ((__noreturn__)) usage(FILE * out)
{
	fputs(USAGE_HEADER, out);
	fprintf(out, _(" %s [options]\n"), program_invocation_short_name);
	fputs(USAGE_OPTIONS, out);
	fputs(_(" -d, --delay <secs>  delay updates\n"), out);
	fputs(_(" -n, --numa          display per NUMA nodes Huge pages information\n"), out);
	fputs(_(" -o, --once          only display once, then exit\n"), out);
	fputs(_(" -H, --human         display human-readable output\n"), out);
	fputs(USAGE_SEPARATOR, out);
	fputs(USAGE_HELP, out);
	fputs(USAGE_VERSION, out);

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int is_tty = isatty(STDIN_FILENO);
	int o;
	unsigned short old_rows;

	static const struct option longopts[] = {
		{ "delay",      required_argument, NULL, 'd' },
		{ "numa",       no_argument,       NULL, 'n' },
		{ "once",       no_argument,       NULL, 'o' },
		{ "human",      no_argument,       NULL, 'H' },
		{ "help",       no_argument,       NULL, 'h' },
		{ "version",    no_argument,       NULL, 'V' },
		{  NULL,        0,                 NULL, 0   }
	};

#ifdef HAVE_PROGRAM_INVOCATION_NAME
	program_invocation_name = program_invocation_short_name;
#endif
	setlocale (LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	atexit(close_stdout);

	while ((o = getopt_long(argc, argv, "d:noHhV", longopts, NULL)) != -1) {
		switch (o) {
			case 'd':
				errno = 0;
				delay = strtol_or_err(optarg, _("illegal delay"));
				if (delay < 1)
					errx(EXIT_FAILURE,
							_("delay must be positive integer"));
				break;
			case 'n':
				numa = 1;
				break;
			case 'o':
				run_once=1;
				delay = 0;
				break;
			case 'H':
				human = 1;
				break;
			case 'V':
				printf(PROCPS_NG_VERSION);
				return EXIT_SUCCESS;
			case 'h':
				usage(stdout);
			default:
				usage(stderr);

		}
	}

	setup_hugepage();

	if (!run_once) {
		is_tty = isatty(STDIN_FILENO);
		if (is_tty && tcgetattr(STDIN_FILENO, &saved_tty) == -1)
			warn(_("terminal setting retrieval"));

		old_rows = rows;
		term_size(0);
		initscr();
		resizeterm(rows, cols);
		signal(SIGWINCH, term_size);
		signal(SIGINT, sigint_handler);
	}

	do {
		struct timeval tv;
		fd_set readfds;
		char c;

		if (run_once) {
			print_summary();
			print_headings();
			print_procs();

			break;
		}

		if (old_rows != rows) {
			resizeterm(rows, cols);
			old_rows = rows;
		}

		move(0, 0);
		print_summary();
		attron(A_REVERSE);
		print_headings();
		attroff(A_REVERSE);
		print_procs();

		refresh();
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		tv.tv_sec = delay;
		tv.tv_usec = 0;
		if (select(STDOUT_FILENO, &readfds, NULL, NULL, &tv) > 0) {
			if (read(STDIN_FILENO, &c, 1) != 1)
				break;
			parse_input(c);
		}

		erase();
	} while (delay);

	if (!run_once) {
		if (is_tty)
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_tty);
		endwin();
	}

	return 0;
}
