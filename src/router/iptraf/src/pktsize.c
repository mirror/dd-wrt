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

#define SIZES 20

struct psizetab {
	WINDOW *win;
	PANEL *panel;
	WINDOW *borderwin;
	PANEL *borderpanel;

	unsigned long size_in[SIZES + 1];	/* +1 for oversized count */
	unsigned long size_out[SIZES + 1];
	unsigned int mtu;
	unsigned int interval;
	unsigned int maxsize_in;
	unsigned int maxsize_out;

};

static void rotate_size_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_size_log);
}

static void write_size_log(struct psizetab *table, unsigned long nsecs,
			   char *ifname, FILE *logfile)
{
	char atime[TIME_TARGET_MAX];
	int i;

	genatime(time(NULL), atime);
	fprintf(logfile, "*** Packet Size Distribution, generated %s\n\n",
		atime);
	fprintf(logfile, "Interface: %s   MTU: %u\n\n", ifname, table->mtu);
	fprintf(logfile, "Packet Size (bytes)\tIn\t\tOut\n");

	for (i = 0; i < SIZES; i++) {
		fprintf(logfile, "%u to %u:\t\t%8lu\t%8lu\n",
				table->interval * i + 1,
				table->interval * (i + 1),
				table->size_in[i],
				table->size_out[i]);
	}
	fprintf(logfile, "\nRunning time: %lu seconds\n", nsecs);
	fflush(logfile);
}

static void psizetab_init(struct psizetab *table, char *ifname)
{
	table->borderwin = newwin(LINES - 2, COLS, 1, 0);
	table->borderpanel = new_panel(table->borderwin);

	wattrset(table->borderwin, BOXATTR);
	tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
	mvwprintw(table->borderwin, 0, 1, " Packet Distribution by Size for interface %s ", ifname);

	table->win = newwin(LINES - 4, COLS - 2, 2, 1);
	table->panel = new_panel(table->win);

	tx_stdwinset(table->win);
	wtimeout(table->win, -1);
	wattrset(table->win, STDATTR);
	tx_colorwin(table->win);

	wattrset(table->win, BOXATTR);
	mvwprintw(table->win, 1, 1, "Packet Size (bytes)");
	mvwprintw(table->win, 1, 23, "In      Out");
	mvwprintw(table->win, 1, 42, "Packet Size (bytes)");
	mvwprintw(table->win, 1, 64, "In      Out");
	wattrset(table->win, HIGHATTR);

	move(LINES - 1, 1);
	stdexitkeyhelp();

	update_panels();
	doupdate();
}

static void psizetab_destroy(struct psizetab *table)
{
	del_panel(table->panel);
	delwin(table->win);

	del_panel(table->borderpanel);
	delwin(table->borderwin);

	update_panels();
	doupdate();
}

static void sizes_init(struct psizetab *table, unsigned int mtu)
{
	table->mtu = mtu;

	unsigned int interval = mtu / SIZES;

	table->interval = interval;

	wattrset(table->win, STDATTR);
	for(unsigned int i = 0; i < SIZES; i++) {
		int row, column;

		table->size_in[i] = 0UL;	/* initialize counters */
		table->size_out[i] = 0UL;

		if (i < SIZES / 2) {
			row = i + 2;
			column = 1;
		} else {
			row = (i - 10) + 2;
			column = 42;
		}
		mvwprintw(table->win, row, column, "%5u to %5u:",
			  interval * i + 1, interval * (i + 1));
	}

	table->size_in[SIZES] = 0UL;	/* initialize oversized counters */
	table->size_out[SIZES] = 0UL;
	table->maxsize_in = 0UL;	/* initialize maxsize counters */
	table->maxsize_out = 0UL;

	mvwprintw(table->win, 12, 47, "oversized:");
	mvwprintw(table->win, 14, 1, "max packet size in (bytes):");
	mvwprintw(table->win, 15, 1, "max packet size out (bytes):");

	mvwprintw(table->win, 17, 1,
		  "Interface MTU is %u bytes, not counting the data-link header.",
		  table->mtu);
	mvwprintw(table->win, 18, 1,
		  "Maximum packet size is the MTU plus the data-link header length, but can be");
	mvwprintw(table->win, 19, 1,
		  "   bigger due to various offloading techniques of the interface.");
	mvwprintw(table->win, 20, 1,
		  "Packet size computations include data-link headers, if any.");
}

static void update_size_distrib(struct psizetab *table, struct pkt_hdr *pkt)
{
	/* -1 is to keep interval boundary lengths within the proper brackets */
	unsigned int i = (pkt->pkt_len - 1) / table->interval;

	if (i > SIZES)
		i = SIZES;	/* last entry is for lengths > MTU */

	if (pkt->from->sll_pkttype == PACKET_OUTGOING) {
		table->size_out[i]++;
		if (table->maxsize_out < pkt->pkt_len)
			table->maxsize_out = pkt->pkt_len;
	} else {
		table->size_in[i]++;
		if (table->maxsize_in < pkt->pkt_len)
			table->maxsize_in = pkt->pkt_len;
	}
}

static void print_size_distrib(struct psizetab *table)
{
	wattrset(table->win, HIGHATTR);
	for (unsigned int i = 0; i < SIZES + 1; i++) {	/* include oversized */
		if (i < 10)
			wmove(table->win, i + 2, 17);
		else
			wmove(table->win, (i - 10) + 2, 58);

		wprintw(table->win, "%8lu %8lu",
			table->size_in[i], table->size_out[i]);
	}
	mvwprintw(table->win, 14, 33, "%5u", table->maxsize_in);
	mvwprintw(table->win, 15, 33, "%5u", table->maxsize_out);
}

static void psize_process_key(int ch)
{
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

static void psize_process_packet(struct psizetab *table, struct pkt_hdr *pkt)
{
	int pkt_result = packet_process(pkt, NULL, NULL, NULL,
					MATCH_OPPOSITE_USECONFIG, 0);

	if (pkt_result == PACKET_OK)
		update_size_distrib(table, pkt);
}

void packet_size_breakdown(char *ifname, time_t facilitytime)
{
	int ch;

	int logging = options.logging;
	FILE *logfile = NULL;

	struct psizetab table;

	int fd;

	struct pkt_hdr pkt;

	unsigned long dropped = 0UL;

	if (!dev_up(ifname)) {
		err_iface_down();
		return;
	}

	psizetab_init(&table, ifname);

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

	int mtu = dev_get_mtu(ifname);
	if (mtu < 0) {
		write_error("Unable to obtain interface MTU");
		goto err_close;
	}

	sizes_init(&table, mtu);

	print_size_distrib(&table);
	update_panels();
	doupdate();

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

	packet_init(&pkt);

	exitloop = 0;

	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval last_time = now;
	struct timeval last_update = now;

	time_t starttime = now.tv_sec;
	time_t endtime = INT_MAX;
	if (facilitytime != 0)
		endtime = now.tv_sec + facilitytime * 60;

	time_t log_next = INT_MAX;
	if (logging)
		log_next = now.tv_sec + options.logspan;

	while (!exitloop) {
		gettimeofday(&now, NULL);

		if (now.tv_sec > last_time.tv_sec) {
			printelapsedtime(now.tv_sec - starttime, 1, table.borderwin);

			dropped += packet_get_dropped(fd);
			print_packet_drops(dropped, table.borderwin, 49);

			if (logging && (now.tv_sec > log_next)) {
				check_rotate_flag(&logfile);
				write_size_log(&table, now.tv_sec - starttime,
					       ifname, logfile);
				log_next = now.tv_sec + options.logspan;
			}

			if (now.tv_sec > endtime)
				exitloop = 1;

			last_time = now;
		}

		if (screen_update_needed(&now, &last_update)) {
			print_size_distrib(&table);

			update_panels();
			doupdate();

			last_update = now;
		}

		if (packet_get(fd, &pkt, &ch, table.win) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR)
			psize_process_key(ch);

		if (pkt.pkt_len > 0)
			psize_process_packet(&table, &pkt);
	}

	packet_destroy(&pkt);

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		write_size_log(&table, time(NULL) - starttime, ifname, logfile);
		writelog(logging, logfile,
			 "******** Packet size distribution facility stopped ********");
		fclose(logfile);
	}
	strcpy(current_logfile, "");

err_close:
	close(fd);
err:
	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	psizetab_destroy(&table);
}
