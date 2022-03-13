/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

itrafmon.c - the IP traffic monitor module

***/

#include "iptraf-ng-compat.h"

#include "tui/labels.h"
#include "tui/winops.h"

#include "options.h"
#include "revname.h"
#include "tcptable.h"
#include "othptab.h"
#include "fltdefs.h"
#include "packet.h"
#include "ifaces.h"
#include "deskman.h"
#include "error.h"
#include "attrs.h"
#include "log.h"
#include "rvnamed.h"
#include "dirs.h"
#include "timer.h"
#include "ipfrag.h"
#include "logvars.h"
#include "itrafmon.h"
#include "sockaddr.h"
#include "capt.h"

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

static void update_flowrates(struct tcptable *table, unsigned long msecs)
{
	struct tcptableent *entry;
	for (entry = table->head; entry != NULL; entry = entry->next_entry) {
		rate_add_rate(&entry->rate, entry->spanbr, msecs);
		entry->spanbr = 0;
	}
}

static void print_flowrate(struct tcptable *table)
{
	if (table->barptr == NULL) {
		wattrset(table->statwin, IPSTATATTR);
		mvwprintw(table->statwin, 0, COLS * 47 / 80,
			  "No TCP entries              ");
	} else {
		wattrset(table->statwin, IPSTATLABELATTR);
		mvwprintw(table->statwin, 0, COLS * 47 / 80,
			  "TCP flow rate: ");

		char buf[32];
		rate_print(rate_get_average(&table->barptr->rate), buf, sizeof(buf));
		wattrset(table->statwin, IPSTATATTR);
		mvwprintw(table->statwin, 0, COLS * 52 / 80 + 13, "%s", buf);
	}
}

/*
 * Scrolling and paging routines for the upper (TCP) window
 */

static void scrollupperwin(struct tcptable *table, int direction)
{
	wattrset(table->tcpscreen, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;

			wscrl(table->tcpscreen, 1);
			scrollok(table->tcpscreen, 0);
			wmove(table->tcpscreen, table->imaxy - 1, 0);
			wprintw(table->tcpscreen, "%*c", COLS - 2, ' ');
			scrollok(table->tcpscreen, 1);

			printentry(table, table->lastvisible);
		}
	} else {
		if (table->firstvisible != table->head) {
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			wscrl(table->tcpscreen, -1);
			mvwprintw(table->tcpscreen, 0, 0, "%*c", COLS - 2, ' ');

			printentry(table, table->firstvisible);
		}
	}
}

static void move_tcp_bar_one(struct tcptable *table, int direction)
{
	switch (direction) {
	case SCROLLUP:
		if (table->barptr->next_entry == NULL)
			break;

		if (table->barptr == table->lastvisible)
			scrollupperwin(table, SCROLLUP);

		table->barptr = table->barptr->next_entry;

		break;
	case SCROLLDOWN:
		if (table->barptr->prev_entry == NULL)
			break;

		if (table->barptr == table->firstvisible)
			scrollupperwin(table, SCROLLDOWN);

		table->barptr = table->barptr->prev_entry;

		break;
	}
}

static void move_tcp_bar_many(struct tcptable *table, int direction, int lines)
{
	switch (direction) {
	case SCROLLUP:
		while (lines && (table->lastvisible != table->tail)) {
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;
			lines--;
		}
		if (lines == 0)
			table->barptr = table->firstvisible;
		else
			table->barptr = table->lastvisible;
		break;
	case SCROLLDOWN:
		while (lines && (table->firstvisible != table->head)) {
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			lines--;
		}
		table->barptr = table->firstvisible;
		break;
	}
}

static void move_tcp_bar(struct tcptable *table, int direction, int lines)
{
	if (table->barptr == NULL)
		return;
	if (lines < 1)
		return;
	if (lines < 10)
		while (lines--)
			move_tcp_bar_one(table, direction);
	else
		move_tcp_bar_many(table, direction, lines);

	print_flowrate(table);
	refreshtcpwin(table, false);
}

/*
 * Scrolling and paging routines for the lower (non-TCP) window.
 */

static void scroll_othp_one(struct othptable *table, int direction)
{
	switch (direction) {
	case SCROLLUP:
		if (table->lastvisible != table->tail) {
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;

			wscrl(table->othpwin, 1);
			printothpentry(table, table->lastvisible,
				       table->oimaxy - 1, 0, NULL);

			if (table->htstat == HIND) {	/* Head indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
		}
		break;
	case SCROLLDOWN:
		if (table->firstvisible != table->head) {
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			wscrl(table->othpwin, -1);
			printothpentry(table, table->firstvisible, 0, 0, NULL);

			if (table->htstat == TIND) {	/* Tail indicator on? */
				wmove(table->borderwin, table->obmaxy - 1, 1);
				whline(table->borderwin, ACS_HLINE, 8);
				table->htstat = NOHTIND;
			}
		}
		break;
	}
}

static void scroll_othp_many(struct othptable *table, int direction, int lines)
{
	switch (direction) {
	case SCROLLUP:
		while (lines-- && (table->lastvisible != table->tail)) {
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;
		}
		if (table->htstat == HIND) {	/* Head indicator on? */
			wmove(table->borderwin, table->obmaxy - 1, 1);
			whline(table->borderwin, ACS_HLINE, 8);
			table->htstat = NOHTIND;
		}
		break;
	case SCROLLDOWN:
		while (lines-- && (table->firstvisible != table->head)) {
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
		}
		if (table->htstat == TIND) {	/* Tail indicator on? */
			wmove(table->borderwin, table->obmaxy - 1, 1);
			whline(table->borderwin, ACS_HLINE, 8);
			table->htstat = NOHTIND;
		}
		break;
	}
	refresh_othwindow(table);
}

static void scroll_othp(struct othptable *table, int direction, int lines)
{
	if (table->head == NULL)
		return;
	if (lines < 1)
		return;
	if (lines < getmaxy(table->othpwin))
		while (lines--)
			scroll_othp_one(table, direction);
	else
		scroll_othp_many(table, direction, lines);
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
					struct tcptableent **high, int ch)
{
	struct tcptableent *pivot = *low;

	struct tcptableent *left = *low;
	struct tcptableent *right = *high;
	struct tcptableent *ptmp;

	unsigned long long pivot_value;

	pivot_value = qt_getkey(pivot, ch);

	while (left->index < right->index) {
		while ((qt_getkey(left, ch) >= pivot_value)
		       && (left->next_entry->next_entry != NULL))

			left = left->next_entry->next_entry;

		while (qt_getkey(right, ch) < pivot_value)
			right = right->prev_entry->prev_entry;

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
				  struct tcptableent *high, int ch)
{
	struct tcptableent *pivot;

	if ((high == NULL) || (low == NULL))
		return;

	if (high->index > low->index) {
		pivot =
		    qt_partition(table, &low, &high, ch);

		if (pivot->prev_entry != NULL)
			quicksort_tcp_entries(table, low,
					      pivot->prev_entry->prev_entry, ch);

		quicksort_tcp_entries(table, pivot->next_entry->next_entry,
				      high, ch);
	}
}

/*
 * This function sorts the TCP window.  The old exchange sort has been
 * replaced with a Quicksort algorithm.
 */

static void sortipents(struct tcptable *table, int ch)
{
	if ((table->head == NULL)
	    || (table->head->next_entry->next_entry == NULL))
		return;

	ch = toupper(ch);

	if ((ch != 'P') && (ch != 'B'))
		return;

	quicksort_tcp_entries(table, table->head, table->tail->prev_entry, ch);

	table->firstvisible = table->head;
	struct tcptableent *ptmp = table->head;

	while (ptmp && ((int)ptmp->index <= getmaxy(table->tcpscreen))) {
		table->lastvisible = ptmp;
		ptmp = ptmp->next_entry;
	}
}

static void ipmon_process_key(int ch, int *curwin, struct tcptable *table, struct othptable *othptbl)
{
	static int keymode = 0;
	static WINDOW *sortwin;
	static PANEL *sortpanel;

	if (keymode == 0) {
		switch (ch) {
		case KEY_UP:
			if (*curwin)
				scroll_othp(othptbl, SCROLLDOWN, 1);
			else
				move_tcp_bar(table, SCROLLDOWN, 1);
			break;
		case KEY_DOWN:
			if (*curwin)
				scroll_othp(othptbl, SCROLLUP, 1);
			else
				move_tcp_bar(table, SCROLLUP, 1);
			break;
		case KEY_RIGHT:
			if (!*curwin)
				break;

			if (othptbl->strindex != VSCRL_OFFSET)
				othptbl->strindex = VSCRL_OFFSET;

			refresh_othwindow(othptbl);
			break;
		case KEY_LEFT:
			if (!*curwin)
				break;

			if (othptbl->strindex != 0)
				othptbl->strindex = 0;

			refresh_othwindow(othptbl);
			break;
		case KEY_PPAGE:
		case '-':
			if (*curwin)
				scroll_othp(othptbl, SCROLLDOWN, othptbl->oimaxy);
			else
				move_tcp_bar(table, SCROLLDOWN, table->imaxy);
			break;
		case KEY_NPAGE:
		case ' ':
			if (*curwin)
				scroll_othp(othptbl, SCROLLUP, othptbl->oimaxy);
			else
				move_tcp_bar(table, SCROLLUP, table->imaxy);
			break;
		case KEY_HOME:
			if (*curwin)
				scroll_othp(othptbl, SCROLLDOWN, INT_MAX);
			else
				move_tcp_bar(table, SCROLLDOWN, INT_MAX);
			break;
		case KEY_END:
			if (*curwin)
				scroll_othp(othptbl, SCROLLUP, INT_MAX);
			else
				move_tcp_bar(table, SCROLLUP, INT_MAX);
			break;
		case KEY_F(6):
		case 'w':
		case 'W':
		case 9:
			*curwin = !*curwin;
			markactive(*curwin, table->borderwin,
				   othptbl->borderwin);
			uniq_help(*curwin);
			refreshtcpwin(table, false);
			break;
		case 'm':
		case 'M':
			if (*curwin)
				break;
			table->mode = (table->mode + 1) % 3;
			if ((table->mode == 1) && !options.mac)
				table->mode = 2;
			refreshtcpwin(table, true);
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
			flushclosedentries(table);
			refreshtcpwin(table, true);
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
		flushclosedentries(table);
		sortipents(table, ch);
		if (table->barptr != NULL) {
			table->barptr = table->firstvisible;
		}
		refreshtcpwin(table, true);
	}
}

static void ipmon_process_packet(struct pkt_hdr *pkt, char *ifname,
				 struct tcptable *table,
				 struct othptable *othptbl,
				 int logging, FILE *logfile,
				 struct resolver *res)
{
	in_port_t sport = 0, dport = 0;	/* TCP/UDP port values */
	unsigned int br;	/* bytes read.  Differs from readlen */
	char ifnamebuf[IFNAMSIZ];
	struct tcptableent *tcpentry;

	int pkt_result = packet_process(pkt, &br, &sport, &dport,
					MATCH_OPPOSITE_ALWAYS,
					options.v6inv4asv6);

	if (pkt_result != PACKET_OK)
		return;

	if (!ifname) {
		/* we're capturing on "All interfaces", */
		/* so get the name of the interface */
		/* of this packet */
		int r = dev_get_ifname(pkt->from->sll_ifindex, ifnamebuf);
		if (r != 0) {
			write_error("Unable to get interface name");
			return;          /* error getting interface name, get out! */
		}
		ifname = ifnamebuf;
	}

	struct sockaddr_storage saddr, daddr;
	switch(pkt->pkt_protocol) {
	case ETH_P_IP:
		sockaddr_make_ipv4(&saddr, pkt->iphdr->saddr);
		sockaddr_make_ipv4(&daddr, pkt->iphdr->daddr);
		break;
	case ETH_P_IPV6:
		sockaddr_make_ipv6(&saddr, &pkt->ip6_hdr->ip6_src);
		sockaddr_make_ipv6(&daddr, &pkt->ip6_hdr->ip6_dst);
		break;
	default:
		add_othp_entry(othptbl, pkt, NULL, NULL,
			       NOT_IP,
			       pkt->pkt_protocol,
			       pkt->pkt_payload, ifname, NULL,
			       logging, logfile);
		return;
	}

	/* only when packets fragmented */
	char *ip_payload = pkt->pkt_payload + pkt_iph_len(pkt);
	switch (pkt_ip_protocol(pkt)) {
	case IPPROTO_TCP: {
		struct tcphdr *tcp = (struct tcphdr *)ip_payload;
		sockaddr_set_port(&saddr, sport);
		sockaddr_set_port(&daddr, dport);
		tcpentry = in_table(table, &saddr, &daddr, ifname);

		/*
		 * Add a new entry if it doesn't exist, and,
		 * to reduce the chances of stales, not a FIN.
		 */

		if (packet_is_first_fragment(pkt)	/* first frag only */
		    && (tcpentry == NULL)
		    && !tcp->fin) {

			/*
			 * Ok, so we have a packet.  Add it if this connection
			 * is not yet closed, or if it is a SYN packet.
			 */
			tcpentry = addentry(table, &saddr, &daddr,
					    pkt_ip_protocol(pkt),
					    ifname, res);
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

			if (pkt->iphdr)
				updateentry(table, pkt, tcpentry, tcp,
					    br,
					    res,
					    logging, logfile);
			else
				updateentry(table, pkt, tcpentry, tcp,
					    pkt->pkt_len,
					    res,
					    logging, logfile);
			/*
			 * Log first packet of a TCP connection except if
			 * it's a RST, which was already logged earlier in
			 * updateentry()
			 */

			if (logging
			    && (tcpentry->pcount == 1)
			    && (!(tcpentry->stat & FLAG_RST))) {
				char msgstring[80];
				strcpy(msgstring, "first packet");
				if (tcp->syn)
					strcat(msgstring, " (SYN)");

				writetcplog(logging, logfile, tcpentry,
					    pkt->pkt_len, msgstring);
			}
		}
		break; }
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		check_icmp_dest_unreachable(table, pkt, ifname);
		/* print this ICMP(v6) and ... */
		/* fall through */
	default:
		add_othp_entry(othptbl, pkt, &saddr, &daddr,
			       IS_IP, pkt_ip_protocol(pkt),
			       ip_payload, ifname,
			       res, logging, logfile);
		break;
	}
}

/* the IP Traffic Monitor */
void ipmon(time_t facilitytime, char *ifptr)
{
	int logging = options.logging;

	FILE *logfile = NULL;

	int curwin = 0;

	unsigned long long total_pkts = 0;

	struct tcptable table;

	struct othptable othptbl;

	struct capt capt;

	struct pkt_hdr pkt;

	struct resolver res;

	int ch;

	if (ifptr && !dev_up(ifptr)) {
		err_iface_down();
		return;
	}

	if (capt_init(&capt, ifptr) == -1) {
		write_error("Unable to initialize packet capture interface");
		return;
	}

	resolver_init(&res, options.revlook);

	if (options.servnames)
		setservent(1);
	setprotoent(1);

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

	init_tcp_table(&table);
	init_othp_table(&othptbl);

	markactive(curwin, table.borderwin, othptbl.borderwin);
	ipmonhelp();
	uniq_help(0);
	update_panels();
	doupdate();

	packet_init(&pkt);

	exitloop = 0;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	struct timespec last_time = now;
	struct timespec next_screen_update = { 0 };
	time_t starttime = now.tv_sec;

	time_t check_closed;
	if (options.closedint != 0)
		check_closed = now.tv_sec + options.closedint * 60;
	else
		check_closed = INT_MAX;

	/* set the time after which we terminate the process */
	time_t endtime;
	if (facilitytime != 0)
		endtime = now.tv_sec + facilitytime * 60;
	else
		endtime = INT_MAX;

	while (!exitloop) {
		clock_gettime(CLOCK_MONOTONIC, &now);

		if (now.tv_sec > last_time.tv_sec) {
			unsigned long msecs = timespec_diff_msec(&now, &last_time);
			/* update all flowrates ... */
			update_flowrates(&table, msecs);
			/* ... and print the current one every second */
			print_flowrate(&table);

			resolve_visible_entries(&table, &res);

			/* print timer at bottom of screen */
			printelapsedtime(now.tv_sec - starttime, 15, othptbl.borderwin);

			print_packet_drops(capt_get_dropped(&capt), othptbl.borderwin, 40);

			mark_timeouted_entries(&table, logging, logfile);

			/* automatically clear closed/timed out entries */
			if (now.tv_sec > check_closed) {
				flushclosedentries(&table);
				refreshtcpwin(&table, true);
				check_closed = now.tv_sec + options.closedint * 60;
			}

			/* terminate after lifetime specified at the cmdline */
			if (now.tv_sec > endtime)
				exitloop = 1;

			/* close and rotate log file if signal was received */
			if (logging && (rotate_flag == 1)) {
				announce_rotate_prepare(logfile);
				write_tcp_unclosed(logging, logfile, &table);
				rotate_logfile(&logfile, target_logname);
				announce_rotate_complete(logfile);
				rotate_flag = 0;
			}

			last_time = now;
		}

		/* update screen at configured intervals. */
		if (time_after(&now, &next_screen_update)) {
			refreshtcpwin(&table, false);
			show_stats(table.statwin, total_pkts);

			update_panels();
			doupdate();

			set_next_screen_update(&next_screen_update, &now);
		}

		if (capt_get_packet(&capt, &pkt, &ch, table.tcpscreen) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR)
			ipmon_process_key(ch, &curwin, &table, &othptbl);

		if (pkt.pkt_len > 0) {
			total_pkts++;
			ipmon_process_packet(&pkt, ifptr, &table, &othptbl,
					     logging, logfile,
					     &res);
			capt_put_packet(&capt, &pkt);
		}
	}
	packet_destroy(&pkt);

	destroyothptable(&othptbl);
	destroytcptable(&table);
	update_panels();
	doupdate();

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writelog(logging, logfile,
			 "******** IP traffic monitor stopped ********\n");
		fclose(logfile);
		strcpy(current_logfile, "");
	}

	endprotoent();
	if (options.servnames)
		endservent();

	resolver_destroy(&res);

	capt_destroy(&capt);
}
