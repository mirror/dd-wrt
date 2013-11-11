/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

itrafmon.c - the IP traffic monitor module

***/

#include "iptraf-ng-compat.h"

#include "tui/labels.h"
#include "tui/winops.h"

#include "options.h"
#include "tcptable.h"
#include "othptab.h"
#include "fltdefs.h"
#include "packet.h"
#include "ifaces.h"
#include "promisc.h"
#include "deskman.h"
#include "error.h"
#include "attrs.h"
#include "log.h"
#include "revname.h"
#include "rvnamed.h"
#include "dirs.h"
#include "timer.h"
#include "ipfrag.h"
#include "logvars.h"
#include "itrafmon.h"
#include "sockaddr.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

static void rotate_ipmon_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_ipmon_log);
}

/* Hot key indicators for the bottom line */

static void ipmonhelp(void)
{
	move(LINES - 1, 1);
	tx_printkeyhelp("Up/Dn/PgUp/PgDn", "-scroll  ", stdscr, HIGHATTR,
			STATUSBARATTR);
	move(LINES - 1, 43);
	tx_printkeyhelp("W", "-chg actv win  ", stdscr, HIGHATTR,
			STATUSBARATTR);
	tx_printkeyhelp("S", "-sort TCP  ", stdscr, HIGHATTR, STATUSBARATTR);
	stdexitkeyhelp();
}

static void uniq_help(int what)
{
	move(LINES - 1, 25);
	if (!what)
		tx_printkeyhelp("M", "-more TCP info   ", stdscr, HIGHATTR,
				STATUSBARATTR);
	else
		tx_printkeyhelp("Lft/Rt", "-vtcl scrl  ", stdscr, HIGHATTR,
				STATUSBARATTR);
}

/* Mark general packet count indicators */

static void prepare_statwin(WINDOW * win)
{
	wattrset(win, IPSTATLABELATTR);
	wmove(win, 0, 1);
	wprintw(win, "Packets captured:");
	mvwaddch(win, 0, 45 * COLS / 80, ACS_VLINE);
}

static void markactive(int curwin, WINDOW * tw, WINDOW * ow)
{
	WINDOW *win1;
	WINDOW *win2;
	int x1 __unused, y1, x2 __unused, y2;

	if (!curwin) {
		win1 = tw;
		win2 = ow;
	} else {
		win1 = ow;
		win2 = tw;
	}

	getmaxyx(win1, y1, x1);
	getmaxyx(win2, y2, x2);

	wmove(win1, --y1, COLS - 10);
	wattrset(win1, ACTIVEATTR);
	wprintw(win1, " Active ");
	wattrset(win1, BOXATTR);
	wmove(win2, --y2, COLS - 10);
	whline(win2, ACS_HLINE, 8);
}

static void show_stats(WINDOW * win, unsigned long long total)
{
	wattrset(win, IPSTATATTR);
	wmove(win, 0, 35 * COLS / 80);
	printlargenum(total, win);
}


/*
 * Scrolling and paging routines for the upper (TCP) window
 */

static void scrollupperwin(struct tcptable *table, int direction,
			   unsigned long *idx, int mode)
{
	char sp_buf[10];

	sprintf(sp_buf, "%%%dc", COLS - 2);
	wattrset(table->tcpscreen, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			wscrl(table->tcpscreen, 1);
			table->lastvisible = table->lastvisible->next_entry;
			table->firstvisible = table->firstvisible->next_entry;
			(*idx)++;
			wmove(table->tcpscreen, table->imaxy - 1, 0);
			scrollok(table->tcpscreen, 0);
			wprintw(table->tcpscreen, sp_buf, ' ');
			scrollok(table->tcpscreen, 1);
			printentry(table, table->lastvisible, *idx, mode);
		}
	} else {
		if (table->firstvisible != table->head) {
			wscrl(table->tcpscreen, -1);
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			(*idx)--;
			wmove(table->tcpscreen, 0, 0);
			wprintw(table->tcpscreen, sp_buf, ' ');
			printentry(table, table->firstvisible, *idx, mode);
		}
	}
}

static void pageupperwin(struct tcptable *table, int direction,
			 unsigned long *idx)
{
	unsigned int i = 1;

	wattrset(table->tcpscreen, STDATTR);
	if (direction == SCROLLUP) {
		while ((i <= table->imaxy - 3)
		       && (table->lastvisible != table->tail)) {
			i++;
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;
			(*idx)++;
		}
	} else {
		while ((i <= table->imaxy - 3)
		       && (table->firstvisible != table->head)) {
			i++;
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			(*idx)--;
		}
	}
}

/*
 * Scrolling and paging routines for the lower (non-TCP) window.
 */

static void scrolllowerwin(struct othptable *table, int direction)
{
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			wscrl(table->othpwin, 1);
			table->lastvisible = table->lastvisible->next_entry;
			table->firstvisible = table->firstvisible->next_entry;

			if (table->htstat == HIND) {	/* Head indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
			printothpentry(table, table->lastvisible,
				       table->oimaxy - 1, 0, NULL);
		}
	} else {
		if (table->firstvisible != table->head) {
			wscrl(table->othpwin, -1);
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			if (table->htstat == TIND) {	/* Tail indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
			printothpentry(table, table->firstvisible, 0, 0, NULL);
		}
	}
}

static void pagelowerwin(struct othptable *table, int direction)
{
	unsigned int i = 1;

	if (direction == SCROLLUP) {
		while ((i <= table->oimaxy - 2)
		       && (table->lastvisible != table->tail)) {
			i++;
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;

			if (table->htstat == HIND) {	/* Head indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
		}
	} else {
		while ((i <= table->oimaxy - 2)
		       && (table->firstvisible != table->head)) {
			i++;
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			if (table->htstat == TIND) {	/* Tail indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
		}
	}
}

/*
 * Pop up sorting key window
 */

static void show_tcpsort_win(WINDOW ** win, PANEL ** panel)
{
	*win = newwin(9, 35, (LINES - 8) / 2, COLS - 40);
	*panel = new_panel(*win);

	wattrset(*win, DLGBOXATTR);
	tx_colorwin(*win);
	tx_box(*win, ACS_VLINE, ACS_HLINE);
	wattrset(*win, DLGTEXTATTR);
	mvwprintw(*win, 2, 2, "Select sort criterion");
	wmove(*win, 4, 2);
	tx_printkeyhelp("P", " - sort by packet count", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 5, 2);
	tx_printkeyhelp("B", " - sort by byte count", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 6, 2);
	tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	update_panels();
	doupdate();
}

/*
 * Routine to swap two TCP entries.  p1 and p2 are pointers to TCP entries,
 * but p1 must be ahead of p2.  It's a linked list thing.
 */
static void swap_tcp_entries(struct tcptable *table, struct tcptableent *p1,
		      struct tcptableent *p2)
{
	struct tcptableent *p2nextsaved;
	struct tcptableent *p1prevsaved;
	unsigned int tmp;

	if (p1 == p2)
		return;

	tmp = p1->index;
	p1->index = p2->index;
	p2->index = tmp;

	p1->next_entry->index = p1->index + 1;
	p2->next_entry->index = p2->index + 1;

	if (p1->prev_entry != NULL)
		p1->prev_entry->next_entry = p2;
	else
		table->head = p2;

	if (p2->next_entry->next_entry != NULL)
		p2->next_entry->next_entry->prev_entry = p1->next_entry;
	else
		table->tail = p1->next_entry;

	p2nextsaved = p2->next_entry->next_entry;
	p1prevsaved = p1->prev_entry;

	if (p1->next_entry->next_entry == p2) {	/* swapping adjacent entries */
		p2->next_entry->next_entry = p1;
		p1->prev_entry = p2->next_entry;
	} else {
		p2->next_entry->next_entry = p1->next_entry->next_entry;
		p1->prev_entry = p2->prev_entry;
		p2->prev_entry->next_entry = p1;
		p1->next_entry->next_entry->prev_entry = p2->next_entry;
	}

	p2->prev_entry = p1prevsaved;
	p1->next_entry->next_entry = p2nextsaved;
}

static unsigned long long qt_getkey(struct tcptableent *entry, int ch)
{
	if (ch == 'B')
		return (max(entry->bcount, entry->oth_connection->bcount));

	return (max(entry->pcount, entry->oth_connection->pcount));
}

static struct tcptableent *qt_partition(struct tcptable *table,
					struct tcptableent **low,
					struct tcptableent **high, int ch,
					int logging, FILE *logfile)
{
	struct tcptableent *pivot = *low;

	struct tcptableent *left = *low;
	struct tcptableent *right = *high;
	struct tcptableent *ptmp;

	unsigned long long pivot_value;

	time_t now;

	pivot_value = qt_getkey(pivot, ch);

	now = time(NULL);

	while (left->index < right->index) {
		while ((qt_getkey(left, ch) >= pivot_value)
		       && (left->next_entry->next_entry != NULL)) {

			/*
			 * Might as well check out timed out entries here too.
			 */
			if ((options.timeout > 0)
			    && ((now - left->lastupdate) / 60 > options.timeout)
			    && (!(left->inclosed))) {
				left->timedout =
				    left->oth_connection->timedout = 1;
				addtoclosedlist(table, left);

				if (logging)
					write_timeout_log(logging, logfile,
							  left);
			}

			left = left->next_entry->next_entry;
		}

		while (qt_getkey(right, ch) < pivot_value) {
			/*
			 * Might as well check out timed out entries here too.
			 */
			if ((options.timeout > 0)
			    && ((now - right->lastupdate) / 60 > options.timeout)
			    && (!(right->inclosed))) {
				right->timedout =
				    right->oth_connection->timedout = 1;
				addtoclosedlist(table, right);

				if (logging)
					write_timeout_log(logging, logfile,
							  right);
			}
			right = right->prev_entry->prev_entry;
		}

		if (left->index < right->index) {
			swap_tcp_entries(table, left, right);

			if (*low == left)
				*low = right;

			if (*high == right)
				*high = left;

			ptmp = left;
			left = right;
			right = ptmp;
		}
	}
	swap_tcp_entries(table, pivot, right);

	if (*low == pivot)
		*low = right;

	if (*high == right)
		*high = pivot;

	return pivot;
}

/*
 * Quicksort the TCP entries.
 */
static void quicksort_tcp_entries(struct tcptable *table,
				  struct tcptableent *low,
				  struct tcptableent *high, int ch,
				  int logging, FILE *logfile)
{
	struct tcptableent *pivot;

	if ((high == NULL) || (low == NULL))
		return;

	if (high->index > low->index) {
		pivot =
		    qt_partition(table, &low, &high, ch, logging, logfile);

		if (pivot->prev_entry != NULL)
			quicksort_tcp_entries(table, low,
					      pivot->prev_entry->prev_entry, ch,
					      logging, logfile);

		quicksort_tcp_entries(table, pivot->next_entry->next_entry,
				      high, ch, logging, logfile);
	}
}

/*
 * This function sorts the TCP window.  The old exchange sort has been
 * replaced with a Quicksort algorithm.
 */

static void sortipents(struct tcptable *table, unsigned long *idx, int ch,
		       int logging, FILE *logfile)
{
	struct tcptableent *tcptmp1;
	unsigned int idxtmp;

	if ((table->head == NULL)
	    || (table->head->next_entry->next_entry == NULL))
		return;

	ch = toupper(ch);

	if ((ch != 'P') && (ch != 'B'))
		return;

	quicksort_tcp_entries(table, table->head, table->tail->prev_entry, ch,
			      logging, logfile);

	update_panels();
	doupdate();
	tx_colorwin(table->tcpscreen);

	tcptmp1 = table->firstvisible = table->head;
	*idx = 1;
	idxtmp = 0;

	while ((tcptmp1 != NULL) && (idxtmp <= table->imaxy - 1)) {
		if (idxtmp++ <= table->imaxy - 1)
			table->lastvisible = tcptmp1;
		tcptmp1 = tcptmp1->next_entry;
	}

}

/*
 * Attempt to communicate with rvnamed, and if it doesn't respond, try
 * to start it.
 */

static int checkrvnamed(void)
{
	pid_t cpid = 0;
	int cstat;

	indicate("Trying to communicate with reverse lookup server");
	if (!rvnamedactive()) {
		indicate("Starting reverse lookup server");

		if ((cpid = fork()) == 0) {
			char *args[] = {
				"rvnamed-ng",
				NULL
			};
			execvp("rvnamed-ng", args);
			/*
			 * execvp() never returns, so if we reach this point, we have
			 * a problem.
			 */

			die("unable execvp() rvnamed-ng");
		} else if (cpid == -1) {
			write_error("Can't spawn new process; lookups will block");
			return 0;
		} else {
			while (waitpid(cpid, &cstat, 0) < 0)
				if (errno != EINTR)
					break;

			if (WEXITSTATUS(cstat) == 1) {
				write_error("Can't start rvnamed; lookups will block");
				return 0;
			} else {
				sleep(1);
				return 1;
			}
		}
	}
	return 1;
}

static void update_flowrate(struct tcptable *table, unsigned long msecs)
{
	struct tcptableent *entry;
	for (entry = table->head; entry != NULL; entry = entry->next_entry) {
		rate_add_rate(&entry->rate, entry->spanbr, msecs);
		entry->spanbr = 0;
	}
}

static void print_flowrate(struct tcptableent *entry, WINDOW *win)
{
	wattrset(win, IPSTATLABELATTR);
	mvwprintw(win, 0, COLS * 47 / 80, "TCP flow rate: ");
	wattrset(win, IPSTATATTR);

	char buf[32];
	rate_print(rate_get_average(&entry->rate), buf, sizeof(buf));
	mvwprintw(win, 0, COLS * 52 / 80 + 13, "%s", buf);
}

/*
 * The IP Traffic Monitor
 */

void ipmon(time_t facilitytime, char *ifptr)
{
	int logging = options.logging;

	unsigned int frag_off;
	struct tcphdr *transpacket;	/* IP-encapsulated packet */
	in_port_t sport = 0, dport = 0;	/* TCP/UDP port values */
	char sp_buf[10];

	unsigned long screen_idx = 1;

	struct timeval tv;
	struct timeval tv_rate;
	time_t starttime = 0;
	time_t now = 0;
	time_t timeint = 0;
	struct timeval updtime;
	time_t closedint = 0;

	WINDOW *statwin;
	PANEL *statpanel;

	WINDOW *sortwin;
	PANEL *sortpanel;

	FILE *logfile = NULL;

	int curwin = 0;

	char *ifname = ifptr;

	unsigned long long total_pkts = 0;

	unsigned int br;	/* bytes read.  Differs from readlen */

	struct tcptable table;
	struct tcptableent *tcpentry;
	struct tcptableent *tmptcp;
	int mode = 0;

	struct othptable othptbl;

	int p_sstat = 0, p_dstat = 0;	/* Reverse lookup statuses prior to */

	/* reattempt in updateentry() */
	int pkt_result = 0;	/* Non-IP filter ok */

	int fragment = 0;	/* Set to 1 if not first fragment */

	int fd;

	int ch;
	int keymode = 0;
	char msgstring[80];

	int rvnfd = 0;

	int revlook = options.revlook;
	int wasempty = 1;

	const int statx = COLS * 47 / 80;

	/*
	 * Mark this instance of the traffic monitor
	 */

	if (ifptr && !dev_up(ifptr)) {
		err_iface_down();
		return;
	}

	LIST_HEAD(promisc);
	if (options.promisc) {
		promisc_init(&promisc, ifptr);
		promisc_set_list(&promisc);
	}

	init_tcp_table(&table);
	init_othp_table(&othptbl);

	statwin = newwin(1, COLS, LINES - 2, 0);
	statpanel = new_panel(statwin);
	wattrset(statwin, IPSTATLABELATTR);
	wmove(statwin, 0, 0);
	sprintf(sp_buf, "%%%dc", COLS);
	wprintw(statwin, sp_buf, ' ');
	prepare_statwin(statwin);
	show_stats(statwin, 0);
	markactive(curwin, table.borderwin, othptbl.borderwin);
	update_panels();
	doupdate();

	if (revlook) {
		if (checkrvnamed())
			open_rvn_socket(&rvnfd);
	} else
		rvnfd = 0;

	ipmonhelp();
	uniq_help(0);

	update_panels();
	doupdate();

	if (options.servnames)
		setservent(1);

	/*
	 * Try to open log file if logging activated.  Turn off logging
	 * (for this session only) if an error was discovered in opening
	 * the log file.  Configuration setting is kept.  Who knows, the
	 * situation may be corrected later.
	 */

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			strncpy(current_logfile,
				gen_instance_logname(IPMONLOG, getpid()),
				80);

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
		signal(SIGUSR1, rotate_ipmon_log);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** IP traffic monitor started ********");
	}

	setprotoent(1);

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(fd == -1) {
		write_error("Unable to obtain monitoring socket");
		goto err;
	}
	if(ifptr && dev_bind_ifname(fd, ifptr) == -1) {
		write_error("Unable to bind interface on the socket");
		goto err_close;
	}

	exitloop = 0;
	gettimeofday(&tv, NULL);
	tv_rate = tv;
	updtime = tv;
	starttime = timeint = closedint = tv.tv_sec;

	PACKET_INIT(pkt);

	while (!exitloop) {
		char ifnamebuf[IFNAMSIZ];

		gettimeofday(&tv, NULL);
		now = tv.tv_sec;

		/* 
		 * Print timer at bottom of screen
		 */

		if (now - timeint >= 5) {
			printelapsedtime(starttime, now, othptbl.obmaxy - 1, 15,
					 othptbl.borderwin);
			timeint = now;
		}

		/*
		 * Automatically clear closed/timed out entries
		 */

		if ((options.closedint != 0)
		    && ((now - closedint) / 60 >= options.closedint)) {
			flushclosedentries(&table, &screen_idx, logging,
					   logfile);
			refreshtcpwin(&table, screen_idx, mode);
			closedint = now;
		}

		/*
		 * Update screen at configured intervals.
		 */

		if (screen_update_needed(&tv, &updtime)) {
			update_panels();
			doupdate();

			updtime = tv;
		}

		/*
		 * If highlight bar is on some entry, update the flow rate
		 * indicator after five seconds.
		 */
		unsigned long rate_msecs = timeval_diff_msec(&tv, &tv_rate);
		if (rate_msecs > 1000) {
			update_flowrate(&table, rate_msecs);
			if (table.barptr != NULL) {
				print_flowrate(table.barptr, statwin);
			} else {
				wattrset(statwin, IPSTATATTR);
				mvwprintw(statwin, 0, statx,
					  "No TCP entries              ");
			}
			tv_rate = tv;
		}

		/*
		 * Terminate facility should a lifetime be specified at the
		 * command line
		 */
		if ((facilitytime != 0)
		    && (((now - starttime) / 60) >= facilitytime))
			exitloop = 1;

		/*
		 * Close and rotate log file if signal was received
		 */
		if (logging && (rotate_flag == 1)) {
			announce_rotate_prepare(logfile);
			write_tcp_unclosed(logging, logfile, &table);
			rotate_logfile(&logfile, target_logname);
			announce_rotate_complete(logfile);
			rotate_flag = 0;
		}

		if (packet_get(fd, &pkt, &ch, table.tcpscreen) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch == ERR)
			goto no_key_ready;

		if (keymode == 0) {
			switch (ch) {
			case KEY_UP:
				if (curwin) {
					scrolllowerwin(&othptbl, SCROLLDOWN);
					break;
				}
				if (!table.barptr
				    || !table.barptr->prev_entry)
					break;

				tmptcp = table.barptr;
				table.barptr = table.barptr->prev_entry;

				printentry(&table, tmptcp, screen_idx, mode);

				if (table.baridx == 1)
					scrollupperwin(&table, SCROLLDOWN,
						       &screen_idx, mode);
				else
					(table.baridx)--;

				printentry(&table, table.barptr, screen_idx,
					   mode);
				break;
			case KEY_DOWN:
				if (curwin) {
					scrolllowerwin(&othptbl, SCROLLUP);
					break;
				}
				if (!table.barptr
				    || !table.barptr->next_entry)
					break;

				tmptcp = table.barptr;
				table.barptr = table.barptr->next_entry;
				printentry(&table, tmptcp, screen_idx,mode);

				if (table.baridx == table.imaxy)
					scrollupperwin(&table, SCROLLUP,
						       &screen_idx, mode);
				else
					(table.baridx)++;

				printentry(&table,table.barptr, screen_idx,
					   mode);
				break;
			case KEY_RIGHT:
				if (!curwin)
					break;

				if (othptbl.strindex != VSCRL_OFFSET)
					othptbl.strindex = VSCRL_OFFSET;

				refresh_othwindow(&othptbl);
				break;
			case KEY_LEFT:
				if (!curwin)
					break;

				if (othptbl.strindex != 0)
					othptbl.strindex = 0;

				refresh_othwindow(&othptbl);
				break;
			case KEY_PPAGE:
			case '-':
				if (curwin) {
					pagelowerwin(&othptbl, SCROLLDOWN);
					refresh_othwindow(&othptbl);
					break;
				}

				if (!table.barptr)
					break;

				pageupperwin(&table, SCROLLDOWN, &screen_idx);
				table.barptr = table.lastvisible;
				table.baridx = table.lastvisible->index
						- screen_idx + 1;
				refreshtcpwin(&table, screen_idx, mode);
				break;
			case KEY_NPAGE:
			case ' ':
				if (curwin) {
					pagelowerwin(&othptbl, SCROLLUP);
					refresh_othwindow(&othptbl);
					break;
				}

				if (!table.barptr)
					break;

				pageupperwin(&table, SCROLLUP, &screen_idx);
				table.barptr = table.firstvisible;
				table.baridx = 1;
				refreshtcpwin(&table, screen_idx, mode);
				break;
			case KEY_F(6):
			case 'w':
			case 'W':
			case 9:
				curwin = !curwin;
				markactive(curwin, table.borderwin,
					   othptbl.borderwin);
				uniq_help(curwin);
				break;
			case 'm':
			case 'M':
				if (curwin)
					break;
				mode = (mode + 1) % 3;
				if ((mode == 1) && !options.mac)
					mode = 2;
				refreshtcpwin(&table, screen_idx, mode);
				break;
			case 12:
			case 'l':
			case 'L':
				tx_refresh_screen();
				break;

			case 'F':
			case 'f':
			case 'c':
			case 'C':
				flushclosedentries(&table, &screen_idx, logging,
						   logfile);
				refreshtcpwin(&table, screen_idx, mode);
				break;
			case 's':
			case 'S':
				keymode = 1;
				show_tcpsort_win(&sortwin, &sortpanel);
				break;
			case 'Q':
			case 'q':
			case 'X':
			case 'x':
			case 24:
			case 27:
				exitloop = 1;
				break;
			}
		} else if (keymode == 1) {
			keymode = 0;
			del_panel(sortpanel);
			delwin(sortwin);
			show_sort_statwin(&sortwin, &sortpanel);
			update_panels();
			doupdate();
			sortipents(&table, &screen_idx, ch, logging,
				   logfile);

			if (table.barptr != NULL) {
				table.barptr = table.firstvisible;
				table.baridx = 1;
			}
			refreshtcpwin(&table, screen_idx, mode);
			del_panel(sortpanel);
			delwin(sortwin);
			update_panels();
			doupdate();
		}
	no_key_ready:

		if (pkt.pkt_len <= 0)
			continue;

		total_pkts++;
		show_stats(statwin, total_pkts);

		pkt_result =
		    packet_process(&pkt, &br, &sport, &dport,
				  MATCH_OPPOSITE_ALWAYS,
				  options.v6inv4asv6);

		if (pkt_result != PACKET_OK)
			continue;

		if (!ifptr) {
			/* we're capturing on "All interfaces", */
			/* so get the name of the interface */
			/* of this packet */
			int r = dev_get_ifname(pkt.pkt_ifindex, ifnamebuf);
			if (r != 0) {
				write_error("Unable to get interface name");
				break;          /* error getting interface name, get out! */
			}
			ifname = ifnamebuf;
		}

		struct sockaddr_storage saddr, daddr;
		switch(pkt.pkt_protocol) {
		case ETH_P_IP:
			frag_off = pkt.iphdr->frag_off;
			sockaddr_make_ipv4(&saddr, pkt.iphdr->saddr);
			sockaddr_make_ipv4(&daddr, pkt.iphdr->daddr);
			break;
		case ETH_P_IPV6:
			frag_off = 0;
			sockaddr_make_ipv6(&saddr, &pkt.ip6_hdr->ip6_src);
			sockaddr_make_ipv6(&daddr, &pkt.ip6_hdr->ip6_dst);
			break;
		default:
			add_othp_entry(&othptbl, &pkt, NULL, NULL,
				       NOT_IP,
				       pkt.pkt_protocol,
				       pkt.pkt_payload, ifname, 0,
				       0, logging, logfile, 0);
			continue;
		}

		/* only when packets fragmented */
		__u8 iphlen = pkt_iph_len(&pkt);
		transpacket = (struct tcphdr *) (pkt.pkt_payload + iphlen);

		__u8 ip_protocol = pkt_ip_protocol(&pkt);
		if (ip_protocol == IPPROTO_TCP) {
			sockaddr_set_port(&saddr, sport);
			sockaddr_set_port(&daddr, dport);
			tcpentry = in_table(&table, &saddr, &daddr, ifname,
					    logging, logfile, options.timeout);

			/*
			 * Add a new entry if it doesn't exist, and,
			 * to reduce the chances of stales, not a FIN.
			 */

			if (((ntohs(frag_off) & 0x3fff) == 0)	/* first frag only */
			    && (tcpentry == NULL)
			    && (!(transpacket->fin))) {

				/*
				 * Ok, so we have a packet.  Add it if this connection
				 * is not yet closed, or if it is a SYN packet.
				 */
				wasempty = (table.head == NULL);
				tcpentry = addentry(&table, &saddr, &daddr,
						    pkt_ip_protocol(&pkt),
						    ifname, &revlook, rvnfd);
				if (tcpentry != NULL) {
					printentry(&table, tcpentry->oth_connection, screen_idx,
						   mode);

					if (wasempty) {
						table.barptr = table.firstvisible;
						table.baridx = 1;
					}
				}
			}
			/*
			 * If we had an addentry() success, we should have no
			 * problem here.  Same thing if we had a table lookup
			 * success.
			 */

			if ((tcpentry != NULL)
			    && !(tcpentry->stat & FLAG_RST)) {
				/*
				 * Don't bother updating the entry if the connection
				 * has been previously reset.  (Does this really
				 * happen in practice?)
				 */

				if (revlook) {
					p_sstat = tcpentry->s_fstat;
					p_dstat = tcpentry->d_fstat;
				}

				if (pkt.iphdr)
					updateentry(&table, tcpentry, transpacket,
						    pkt.pkt_buf, pkt.pkt_hatype,
						    pkt.pkt_len, br, pkt.iphdr->frag_off,
						    logging, &revlook, rvnfd,
						    logfile);
				else
					updateentry(&table, tcpentry, transpacket,
						    pkt.pkt_buf, pkt.pkt_hatype,
						    pkt.pkt_len, pkt.pkt_len, 0, logging,
						    &revlook, rvnfd,
						    logfile);
				/*
				 * Log first packet of a TCP connection except if
				 * it's a RST, which was already logged earlier in
				 * updateentry()
				 */

				if (logging
				    && (tcpentry->pcount == 1)
				    && (!(tcpentry->stat & FLAG_RST))) {
					strcpy(msgstring, "first packet");
					if (transpacket->syn)
						strcat(msgstring, " (SYN)");

					writetcplog(logging, logfile, tcpentry,
						    pkt.pkt_len, msgstring);
				}

				if ((revlook)
				    && (((p_sstat != RESOLVED)
					 && (tcpentry->s_fstat == RESOLVED))
					|| ((p_dstat != RESOLVED)
					    && (tcpentry->d_fstat == RESOLVED)))) {
					clearaddr(&table, tcpentry, screen_idx);
					clearaddr(&table, tcpentry->oth_connection,
						  screen_idx);
				}
				printentry(&table, tcpentry, screen_idx, mode);

				/*
				 * Special cases: Update other direction if it's
				 * an ACK in response to a FIN.
				 *
				 *         -- or --
				 *
				 * Addresses were just resolved for the other
				 * direction, so we should also do so here.
				 */

				if (((tcpentry->oth_connection->finsent == 2)
				     &&	/* FINed and ACKed */
				     (ntohl(transpacket->seq) == tcpentry->oth_connection->finack))
				    || ((revlook)
					&& (((p_sstat != RESOLVED)
					     && (tcpentry->s_fstat == RESOLVED))
					    || ((p_dstat != RESOLVED)
						&& (tcpentry->d_fstat == RESOLVED)))))
					printentry(&table, tcpentry->oth_connection,
						   screen_idx, mode);
			}
		} else if (pkt.iphdr) {
			fragment =  ((ntohs(pkt.iphdr->frag_off) & 0x1fff) != 0);

			if (pkt_ip_protocol(&pkt) == IPPROTO_ICMP) {

				/*
				 * Cancel the corresponding TCP entry if an ICMP
				 * Destination Unreachable or TTL Exceeded message
				 * is received.
				 */

				if (((struct icmphdr *) transpacket)->type == ICMP_DEST_UNREACH)
					process_dest_unreach(&table, (char *) transpacket,
							     ifname);
			}
			add_othp_entry(&othptbl, &pkt, &saddr, &daddr,
				       IS_IP, pkt_ip_protocol(&pkt),
				       (char *) transpacket, ifname,
				       &revlook, rvnfd, logging, logfile,
				       fragment);

		} else {
			if (pkt_ip_protocol(&pkt) == IPPROTO_ICMPV6
			    && (((struct icmp6_hdr *) transpacket)->icmp6_type == ICMP6_DST_UNREACH))
				process_dest_unreach(&table, (char *) transpacket,
						     ifname);

			add_othp_entry(&othptbl, &pkt, &saddr, &daddr,
				       IS_IP, pkt_ip_protocol(&pkt),
				       (char *) transpacket, ifname,
				       &revlook, rvnfd, logging, logfile,
				       fragment);
		}
	}

err_close:
	close(fd);
err:
	killrvnamed();

	if (options.servnames)
		endservent();

	endprotoent();
	close_rvn_socket(rvnfd);

	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	attrset(STDATTR);
	mvprintw(0, COLS - 20, "                    ");
	del_panel(table.tcppanel);
	del_panel(table.borderpanel);
	del_panel(othptbl.othppanel);
	del_panel(othptbl.borderpanel);
	del_panel(statpanel);
	update_panels();
	doupdate();
	delwin(table.tcpscreen);
	delwin(table.borderwin);
	delwin(othptbl.othpwin);
	delwin(othptbl.borderwin);
	delwin(statwin);
	destroytcptable(&table);
	destroyothptable(&othptbl);
	pkt_cleanup();

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writelog(logging, logfile,
			 "******** IP traffic monitor stopped ********\n");
		fclose(logfile);
		strcpy(current_logfile, "");
	}
}
