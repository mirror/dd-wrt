/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

pktsize.c	- the packet size breakdown facility

***/

#include "iptraf-ng-compat.h"

#include "tui/winops.h"

#include "attrs.h"
#include "dirs.h"
#include "fltdefs.h"
#include "ifaces.h"
#include "packet.h"
#include "deskman.h"
#include "error.h"
#include "pktsize.h"
#include "options.h"
#include "timer.h"
#include "log.h"
#include "logvars.h"
#include "promisc.h"

struct ifstat_brackets {
	unsigned int floor;
	unsigned int ceil;
	unsigned long count;
};

static void rotate_size_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_size_log);
}

static void write_size_log(struct ifstat_brackets *brackets,
			   unsigned long nsecs, char *ifname, unsigned int mtu,
			   FILE *logfile)
{
	char atime[TIME_TARGET_MAX];
	int i;

	genatime(time(NULL), atime);
	fprintf(logfile, "*** Packet Size Distribution, generated %s\n\n",
		atime);
	fprintf(logfile, "Interface: %s   MTU: %u\n\n", ifname, mtu);
	fprintf(logfile, "Packet Size (bytes)\tCount\n");

	for (i = 0; i <= 19; i++) {
		fprintf(logfile, "%u to %u:\t\t%lu\n", brackets[i].floor,
			brackets[i].ceil, brackets[i].count);
	}
	fprintf(logfile, "\nRunning time: %lu seconds\n", nsecs);
	fflush(logfile);
}

static int initialize_brackets(struct ifstat_brackets *brackets,
			       unsigned int *interval, int mtu,
			       WINDOW *win)
{
	int i;

	*interval = mtu / 20;	/* There are 20 packet size brackets */

	for (i = 0; i <= 19; i++) {
		brackets[i].floor = *interval * i + 1;
		brackets[i].ceil = *interval * (i + 1);
		brackets[i].count = 0;
	}

	brackets[19].ceil = mtu;

	for (i = 0; i <= 9; i++) {
		wattrset(win, STDATTR);
		wmove(win, i + 5, 2);
		wprintw(win, "%4u to %4u:", brackets[i].floor,
			brackets[i].ceil);
		wmove(win, i + 5, 23);
		wattrset(win, HIGHATTR);
		wprintw(win, "%8lu", 0);
	}

	for (i = 10; i <= 19; i++) {
		wattrset(win, STDATTR);
		wmove(win, (i - 10) + 5, 36);

		if (i != 19)
			wprintw(win, "%4u to %4u:", brackets[i].floor,
				brackets[i].ceil);
		else
			wprintw(win, "%4u to %4u+:", brackets[i].floor,
				brackets[i].ceil);

		wmove(win, (i - 10) + 5, 57);
		wattrset(win, HIGHATTR);
		wprintw(win, "%8lu", 0);
	}

	wattrset(win, STDATTR);
	mvwprintw(win, 17, 1,
		  "Interface MTU is %d bytes, not counting the data-link header",
		  mtu);
	mvwprintw(win, 18, 1,
		  "Maximum packet size is the MTU plus the data-link header length");
	mvwprintw(win, 19, 1,
		  "Packet size computations include data-link headers, if any");

	return 0;
}

static void update_size_distrib(unsigned int length,
				struct ifstat_brackets *brackets,
				unsigned int interval)
{
	unsigned int i;

	i = (length - 1) / interval;	/* minus 1 to keep interval
					   boundary lengths within the
					   proper brackets */

	if (i > 19)		/* This is for extras for MTU's not */
		i = 19;		/* divisible by 20 */

	brackets[i].count++;
}

static void print_size_distrib(struct ifstat_brackets *brackets, WINDOW *win)
{
	for (unsigned int i = 0; i <= 19; i++) {
		if (i < 10)
			wmove(win, i + 5, 23);
		else
			wmove(win, (i - 10) + 5, 57);

		wprintw(win, "%8lu", brackets[i].count);
	}
}

void packet_size_breakdown(char *ifname, time_t facilitytime)
{
	WINDOW *win;
	PANEL *panel;
	WINDOW *borderwin;
	PANEL *borderpanel;

	struct ifstat_brackets brackets[20];
	unsigned int interval;

	int ch;

	int mtu;

	int pkt_result;

	struct timeval tv;
	time_t starttime, startlog, timeint;
	time_t now;
	struct timeval updtime;

	int logging = options.logging;
	FILE *logfile = NULL;

	int fd;

	if (!dev_up(ifname)) {
		err_iface_down();
		goto err_unmark;
	}

	mtu = dev_get_mtu(ifname);
	if (mtu < 0) {
		write_error("Unable to obtain interface MTU");
		goto err_unmark;
	}

	borderwin = newwin(LINES - 2, COLS, 1, 0);
	borderpanel = new_panel(borderwin);

	wattrset(borderwin, BOXATTR);
	tx_box(borderwin, ACS_VLINE, ACS_HLINE);
	mvwprintw(borderwin, 0, 1, " Packet Distribution by Size ");

	win = newwin(LINES - 4, COLS - 2, 2, 1);
	panel = new_panel(win);

	tx_stdwinset(win);
	wtimeout(win, -1);
	wattrset(win, STDATTR);
	tx_colorwin(win);

	move(LINES - 1, 1);
	stdexitkeyhelp();

	initialize_brackets(brackets, &interval, mtu, win);

	mvwprintw(win, 1, 1, "Packet size brackets for interface %s", ifname);
	wattrset(win, BOXATTR);

	mvwprintw(win, 4, 1, "Packet Size (bytes)");
	mvwprintw(win, 4, 26, "Count");
	mvwprintw(win, 4, 36, "Packet Size (bytes)");
	mvwprintw(win, 4, 60, "Count");
	wattrset(win, HIGHATTR);

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			snprintf(current_logfile, 80, "%s-%s.log", PKTSIZELOG,
				 ifname);

			if (!daemonized)
				input_logfile(current_logfile, &logging);
		}
	}

	if (logging) {
		opentlog(&logfile, current_logfile);

		if (logfile == NULL)
			logging = 0;
	}
	if (logging) {
		signal(SIGUSR1, rotate_size_log);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** Packet size distribution facility started ********");
	}

	exitloop = 0;
	gettimeofday(&tv, NULL);
	updtime = tv;
	now = starttime = startlog = timeint = tv.tv_sec;

	LIST_HEAD(promisc);
	if (options.promisc) {
		promisc_init(&promisc, ifname);
		promisc_set_list(&promisc);
	}

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(fd == -1) {
		write_error("Unable to obtain monitoring socket");
		goto err;
	}
	if(dev_bind_ifname(fd, ifname) == -1) {
		write_error("Unable to bind interface on the socket");
		goto err_close;
	}

	PACKET_INIT(pkt);

	do {
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;

		if (screen_update_needed(&tv, &updtime)) {
			print_size_distrib(brackets, win);

			update_panels();
			doupdate();

			updtime = tv;
		}
		if (now - timeint >= 5) {
			printelapsedtime(starttime, now, LINES - 3, 1,
					 borderwin);
			timeint = now;
		}
		if (logging) {
			check_rotate_flag(&logfile);
			if ((now - startlog) >= options.logspan) {
				write_size_log(brackets, now - starttime,
					       ifname, mtu, logfile);
				startlog = now;
			}
		}

		if ((facilitytime != 0)
		    && (((now - starttime) / 60) >= facilitytime))
			exitloop = 1;

		if (packet_get(fd, &pkt, &ch, win) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR) {
			switch (ch) {
			case 12:
			case 'l':
			case 'L':
				tx_refresh_screen();
				break;
			case 'x':
			case 'X':
			case 'q':
			case 'Q':
			case 27:
			case 24:
				exitloop = 1;
			}
		}

		if (pkt.pkt_len <= 0)
			continue;

		pkt_result = packet_process(&pkt, NULL, NULL, NULL,
					    MATCH_OPPOSITE_USECONFIG, 0);

		if (pkt_result != PACKET_OK)
			continue;

		update_size_distrib(pkt.pkt_len, brackets, interval);
	} while (!exitloop);

err_close:
	close(fd);
err:
	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		write_size_log(brackets, now - starttime, ifname, mtu, logfile);
		writelog(logging, logfile,
			 "******** Packet size distribution facility stopped ********");
		fclose(logfile);
	}

	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	del_panel(panel);
	delwin(win);
	del_panel(borderpanel);
	delwin(borderwin);
err_unmark:
	strcpy(current_logfile, "");
}
