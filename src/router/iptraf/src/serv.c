/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

serv.c  - TCP/UDP port statistics module

***/

#include "iptraf-ng-compat.h"

#include "tui/input.h"
#include "tui/labels.h"
#include "tui/listbox.h"
#include "tui/msgboxes.h"

#include "dirs.h"
#include "deskman.h"
#include "fltdefs.h"
#include "packet.h"
#include "ipfrag.h"
#include "ifaces.h"
#include "attrs.h"
#include "serv.h"
#include "servname.h"
#include "log.h"
#include "timer.h"
#include "options.h"
#include "packet.h"
#include "logvars.h"
#include "error.h"
#include "counters.h"
#include "rate.h"
#include "capt.h"
#include "timer.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

#define LEFT 0
#define RIGHT 1

struct portlistent {
	in_port_t port;
	unsigned int protocol;
	char servname[11];
	unsigned int idx;
	struct proto_counter serv_count;
	struct proto_counter span;

	struct timespec proto_starttime;

	struct rate rate;
	struct rate rate_in;
	struct rate rate_out;

	struct portlistent *prev_entry;
	struct portlistent *next_entry;
};

struct portlist {
	struct portlistent *head;
	struct portlistent *tail;
	struct portlistent *firstvisible;
	struct portlistent *lastvisible;
	struct portlistent *barptr;
	unsigned int count;

	WINDOW *win;
	PANEL *panel;
	WINDOW *borderwin;
	PANEL *borderpanel;
	WINDOW *statwin;
	PANEL *statpanel;
};

/*
 * SIGUSR1 logfile rotation signal handler
 */

static void rotate_serv_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_serv_log);
}

static void writeutslog(struct portlistent *list, unsigned long nsecs, FILE *fd)
{
	char atime[TIME_TARGET_MAX];
	struct portlistent *ptmp = list;
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	genatime(time(NULL), atime);

	fprintf(fd, "\n*** TCP/UDP traffic log, generated %s\n\n", atime);

	while (ptmp != NULL) {
		unsigned long secs = timespec_diff_msec(&now, &ptmp->proto_starttime) / 1000UL;
		char bps_string[64];

		if (ptmp->protocol == IPPROTO_TCP)
			fprintf(fd, "TCP/%s: ", ptmp->servname);
		else
			fprintf(fd, "UDP/%s: ", ptmp->servname);

		fprintf(fd, "%llu packets, %llu bytes total",
			ptmp->serv_count.proto_total.pc_packets,
			ptmp->serv_count.proto_total.pc_bytes);

		rate_print(ptmp->serv_count.proto_total.pc_bytes / secs,
			   bps_string, sizeof(bps_string));
		fprintf(fd, ", %s", bps_string);

		fprintf(fd, "; %llu packets, %llu bytes incoming",
			ptmp->serv_count.proto_in.pc_packets,
			ptmp->serv_count.proto_in.pc_bytes);

		rate_print(ptmp->serv_count.proto_in.pc_bytes / secs,
			   bps_string, sizeof(bps_string));
		fprintf(fd, ", %s", bps_string);

		fprintf(fd, "; %llu packets, %llu bytes outgoing",
			ptmp->serv_count.proto_out.pc_packets,
			ptmp->serv_count.proto_out.pc_bytes);

		rate_print(ptmp->serv_count.proto_out.pc_bytes / secs,
			   bps_string, sizeof(bps_string));
		fprintf(fd, ", %s", bps_string);

		fprintf(fd, "\n\n");
		ptmp = ptmp->next_entry;
	}

	fprintf(fd, "\nRunning time: %lu seconds\n", nsecs);
	fflush(fd);
}

static void initportlist(struct portlist *list)
{
	float screen_scale = ((float) COLS / 80 + 1) / 2;

	list->head = list->tail = list->barptr = NULL;
	list->firstvisible = list->lastvisible = NULL;
	list->count = 0;

	list->borderwin = newwin(LINES - 3, COLS, 1, 0);
	list->borderpanel = new_panel(list->borderwin);
	wattrset(list->borderwin, BOXATTR);
	tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);

	mvwprintw(list->borderwin, 0,  1 * screen_scale, " Proto/Port ");
	mvwprintw(list->borderwin, 0, 22 * screen_scale, " Pkts ");
	mvwprintw(list->borderwin, 0, 31 * screen_scale, " Bytes ");
	mvwprintw(list->borderwin, 0, 40 * screen_scale, " PktsTo ");
	mvwprintw(list->borderwin, 0, 49 * screen_scale, " BytesTo ");
	mvwprintw(list->borderwin, 0, 58 * screen_scale, " PktsFrom ");
	mvwprintw(list->borderwin, 0, 67 * screen_scale, " BytesFrom ");

	list->win = newwin(LINES - 5, COLS - 2, 2, 1);
	list->panel = new_panel(list->win);

	list->statwin = newwin(1, COLS, LINES - 2, 0);
	list->statpanel = new_panel(list->statwin);
	scrollok(list->statwin, 0);
	wattrset(list->statwin, IPSTATLABELATTR);
	mvwprintw(list->statwin, 0, 0, "%*c", COLS, ' ');

	tx_stdwinset(list->win);
	wtimeout(list->win, -1);
	wattrset(list->win, STDATTR);
	tx_colorwin(list->win);

	move(LINES - 1, 1);
	scrollkeyhelp();
	sortkeyhelp();
	stdexitkeyhelp();

	update_panels();
	doupdate();
}

static void print_serv_rates(struct portlist *table)
{
	if (table->barptr == NULL) {
		wattrset(table->statwin, IPSTATATTR);
		mvwprintw(table->statwin, 0, 1, "No entries");
	} else {
		char buf[64];

		wattrset(table->statwin, IPSTATLABELATTR);
		mvwprintw(table->statwin, 0, 1, "Protocol data rates:");
		mvwprintw(table->statwin, 0, 36, "total");
		mvwprintw(table->statwin, 0, 57, "in");
		mvwprintw(table->statwin, 0, 76, "out");

		wattrset(table->statwin, IPSTATATTR);
		rate_print(rate_get_average(&table->barptr->rate), buf, sizeof(buf));
		mvwprintw(table->statwin, 0, 21, "%s", buf);
		rate_print(rate_get_average(&table->barptr->rate_in), buf, sizeof(buf));
		mvwprintw(table->statwin, 0, 42, "%s", buf);
		rate_print(rate_get_average(&table->barptr->rate_out), buf, sizeof(buf));
		mvwprintw(table->statwin, 0, 61, "%s", buf);
	}
}

static struct portlistent *addtoportlist(struct portlist *list,
					 unsigned int protocol,
					 in_port_t port)
{
	struct portlistent *ptemp;

	ptemp = xmalloc(sizeof(struct portlistent));
	if (list->head == NULL) {
		ptemp->prev_entry = NULL;
		list->head = ptemp;
		list->firstvisible = ptemp;
	}

	if (list->tail != NULL) {
		list->tail->next_entry = ptemp;
		ptemp->prev_entry = list->tail;
	}
	list->tail = ptemp;
	ptemp->next_entry = NULL;

	ptemp->protocol = protocol;
	ptemp->port = port;	/* This is used in checks later. */
	rate_alloc(&ptemp->rate, 5);
	rate_alloc(&ptemp->rate_in, 5);
	rate_alloc(&ptemp->rate_out, 5);

	/*
	 * Obtain appropriate service name
	 */

	servlook(port, protocol, ptemp->servname, 10);

	proto_counter_reset(&ptemp->serv_count);
	proto_counter_reset(&ptemp->span);

	list->count++;
	ptemp->idx = list->count;

	clock_gettime(CLOCK_MONOTONIC, &ptemp->proto_starttime);

	if (list->count <= (unsigned) LINES - 5)
		list->lastvisible = ptemp;

	mvwprintw(list->borderwin, LINES - 4, 1, " %u entries ", list->count);

	if (list->barptr == NULL)
		list->barptr = ptemp;

	return ptemp;
}

static int portinlist(struct porttab *table, in_port_t port)
{
	struct porttab *ptmp = table;

	while (ptmp != NULL) {
		if (((ptmp->port_max == 0) && (ptmp->port_min == port))
		    || ((port >= ptmp->port_min) && (port <= ptmp->port_max)))
			return 1;

		ptmp = ptmp->next_entry;
	}

	return 0;
}

static int goodport(in_port_t port, struct porttab *table)
{
	return ((port < 1024) || (portinlist(table, port)));
}

static struct portlistent *inportlist(struct portlist *list,
				      unsigned int protocol, in_port_t port)
{
	struct portlistent *ptmp = list->head;

	while (ptmp != NULL) {
		if ((ptmp->port == port) && (ptmp->protocol == protocol))
			return ptmp;

		ptmp = ptmp->next_entry;
	}

	return NULL;
}

static void printportent(struct portlist *list, struct portlistent *entry)
{
	unsigned int target_row;
	float screen_scale = ((float) COLS / 80 + 1) / 2;
	int tcplabelattr;
	int udplabelattr;
	int highattr;

	if ((entry->idx < list->firstvisible->idx) ||
	    (entry->idx > list->lastvisible->idx))
		return;

	target_row = entry->idx - list->firstvisible->idx;

	if (entry == list->barptr) {
		tcplabelattr = BARSTDATTR;
		udplabelattr = BARPTRATTR;
		highattr = BARHIGHATTR;
	} else {
		tcplabelattr = STDATTR;
		udplabelattr = PTRATTR;
		highattr = HIGHATTR;
	}

	wattrset(list->win, tcplabelattr);
	scrollok(list->win, 0);
	mvwprintw(list->win, target_row, 0, "%*c", COLS - 2, ' ');
	scrollok(list->win, 1);

	wmove(list->win, target_row, 1);
	if (entry->protocol == IPPROTO_TCP) {
		wattrset(list->win, tcplabelattr);
		wprintw(list->win, "TCP");
	} else if (entry->protocol == IPPROTO_UDP) {
		wattrset(list->win, udplabelattr);
		wprintw(list->win, "UDP");
	}

	wprintw(list->win, "/%s          ", entry->servname);
	wattrset(list->win, highattr);
	wmove(list->win, target_row, 17 * screen_scale);
	printlargenum(entry->serv_count.proto_total.pc_packets, list->win);
	wmove(list->win, target_row, 27 * screen_scale);
	printlargenum(entry->serv_count.proto_total.pc_bytes, list->win);
	wmove(list->win, target_row, 37 * screen_scale);
	printlargenum(entry->serv_count.proto_in.pc_packets, list->win);
	wmove(list->win, target_row, 47 * screen_scale);
	printlargenum(entry->serv_count.proto_in.pc_bytes, list->win);
	wmove(list->win, target_row, 57 * screen_scale);
	printlargenum(entry->serv_count.proto_out.pc_packets, list->win);
	wmove(list->win, target_row, 67 * screen_scale);
	printlargenum(entry->serv_count.proto_out.pc_bytes, list->win);
}

static void destroyportlist(struct portlist *list)
{
	struct portlistent *ptmp = list->head;

	while (ptmp != NULL) {
		struct portlistent *ctmp = ptmp->next_entry;

		rate_destroy(&ptmp->rate_out);
		rate_destroy(&ptmp->rate_in);
		rate_destroy(&ptmp->rate);
		free(ptmp);

		ptmp = ctmp;
	}

	del_panel(list->panel);
	delwin(list->win);
	del_panel(list->borderpanel);
	delwin(list->borderwin);
	del_panel(list->statpanel);
	delwin(list->statwin);

	update_panels();
	doupdate();
}

static void updateportent(struct portlist *list, unsigned int protocol,
			  in_port_t sport, in_port_t dport, int br,
			  struct porttab *ports)
{
	struct portlistent *sport_listent = NULL;
	struct portlistent *dport_listent = NULL;
	enum {
		PORT_INCOMING = 0,
		PORT_OUTGOING
	};

	if (goodport(sport, ports)) {
		sport_listent = inportlist(list, protocol, sport);

		if (!sport_listent)
			sport_listent =
				addtoportlist(list, protocol, sport);

		if (sport_listent == NULL)
			return;

		proto_counter_update(&sport_listent->serv_count, PORT_OUTGOING, br);
		proto_counter_update(&sport_listent->span, PORT_OUTGOING, br);
	}

	if (goodport(dport, ports)) {
		dport_listent = inportlist(list, protocol, dport);

		if (!dport_listent)
			dport_listent =
				addtoportlist(list, protocol, dport);

		if (dport_listent == NULL)
			return;

		proto_counter_update(&dport_listent->serv_count, PORT_INCOMING, br);
		proto_counter_update(&dport_listent->span, PORT_INCOMING, br);
	}
}

/*
 * Swap two port list entries.  p1 must be previous to p2.
 */

static void swapportents(struct portlist *list, struct portlistent *p1,
			 struct portlistent *p2)
{
	register unsigned int tmp;
	struct portlistent *p1prevsaved;
	struct portlistent *p2nextsaved;

	if (p1 == p2)
		return;

	tmp = p1->idx;
	p1->idx = p2->idx;
	p2->idx = tmp;

	if (p1->prev_entry != NULL)
		p1->prev_entry->next_entry = p2;
	else
		list->head = p2;

	if (p2->next_entry != NULL)
		p2->next_entry->prev_entry = p1;
	else
		list->tail = p1;

	p2nextsaved = p2->next_entry;
	p1prevsaved = p1->prev_entry;

	if (p1->next_entry == p2) {
		p2->next_entry = p1;
		p1->prev_entry = p2;
	} else {
		p2->next_entry = p1->next_entry;
		p1->prev_entry = p2->prev_entry;
		p2->prev_entry->next_entry = p1;
		p1->next_entry->prev_entry = p2;
	}

	p2->prev_entry = p1prevsaved;
	p1->next_entry = p2nextsaved;
}

/*
 * Retrieve the appropriate sort criterion based on keystroke.
 */
static unsigned long long qp_getkey(struct portlistent *entry, int ch)
{
	unsigned long long result = 0;

	switch (ch) {
	case 'R':
		result = entry->port;
		break;
	case 'B':
		result = entry->serv_count.proto_total.pc_bytes;
		break;
	case 'O':
		result = entry->serv_count.proto_in.pc_bytes;
		break;
	case 'M':
		result = entry->serv_count.proto_out.pc_bytes;
		break;
	case 'P':
		result = entry->serv_count.proto_total.pc_packets;
		break;
	case 'T':
		result = entry->serv_count.proto_in.pc_packets;
		break;
	case 'F':
		result = entry->serv_count.proto_out.pc_packets;
		break;
	}

	return result;
}

/*
 * Refresh TCP/UDP service screen.
 */

static void refresh_serv_screen(struct portlist *table)
{
	struct portlistent *ptmp = table->firstvisible;

	wattrset(table->win, STDATTR);
	tx_colorwin(table->win);

	while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
		printportent(table, ptmp);
		ptmp = ptmp->next_entry;
	}
	update_panels();
	doupdate();
}


/*
 * Compare the sort criterion with the pivot value.  Receives a parameter
 * specifying whether the criterion is left or right of the pivot value.
 *
 * If criterion is the port number: return true if criterion is less than or
 *     equal to the pivot when the SIDE is left.  If SIDE is right, return
 *     true if the value is greater than the pivot.  This results in an
 *     ascending sort.
 *
 * If the criterion is a count: return true when the criterion is greater than
 *     or equal to the pivot when the SIDE is left, otherwise, when SIDE is
 *     right, return true if the value is less than the pivot.  This results
 *     in a descending sort. 
 */

static int qp_compare(struct portlistent *entry, unsigned long long pv, int ch,
	       int side)
{
	int result = 0;
	unsigned long long value;

	value = qp_getkey(entry, ch);

	if (ch == 'R') {
		if (side == LEFT)
			result = (value <= pv);
		else
			result = (value > pv);
	} else {
		if (side == LEFT)
			result = (value >= pv);
		else
			result = (value < pv);
	}

	return result;
}

/*
 * Partition port list such that a pivot is selected, and that all values
 * left of the pivot are less (or greater) than or equal to the pivot,
 * and that all values right of the pivot are greater (or less) than
 * the pivot.
 */
static struct portlistent *qp_partition(struct portlist *table,
					struct portlistent **low,
					struct portlistent **high, int ch)
{
	struct portlistent *pivot = *low;

	struct portlistent *left = *low;
	struct portlistent *right = *high;
	struct portlistent *ptmp;

	unsigned long long pivot_value;

	pivot_value = qp_getkey(pivot, ch);

	while (left->idx < right->idx) {
		while ((qp_compare(left, pivot_value, ch, LEFT))
		       && (left->next_entry != NULL))
			left = left->next_entry;

		while (qp_compare(right, pivot_value, ch, RIGHT))
			right = right->prev_entry;

		if (left->idx < right->idx) {
			swapportents(table, left, right);
			if (*low == left)
				*low = right;

			if (*high == right)
				*high = left;

			ptmp = left;
			left = right;
			right = ptmp;
		}
	}
	swapportents(table, pivot, right);

	if (*low == pivot)
		*low = right;

	if (*high == right)
		*high = pivot;

	return pivot;
}

/*
 * Quicksort for the port list.
 */
static void quicksort_port_entries(struct portlist *table,
				   struct portlistent *low,
				   struct portlistent *high, int ch)
{
	struct portlistent *pivot;

	if ((high == NULL) || (low == NULL))
		return;

	if (high->idx > low->idx) {
		pivot = qp_partition(table, &low, &high, ch);

		quicksort_port_entries(table, low, pivot->prev_entry, ch);
		quicksort_port_entries(table, pivot->next_entry, high, ch);
	}
}

static void sortportents(struct portlist *list, int command)
{
	if (!(list->head))
		return;

	command = toupper(command);

	if ((command != 'R') && (command != 'B') && (command != 'O')
	    && (command != 'M') && (command != 'P') && (command != 'T')
	    && (command != 'F'))
		return;

	quicksort_port_entries(list, list->head, list->tail, command);

	list->firstvisible = list->head;
	struct portlistent *ptmp = list->head;
	while (ptmp && ((int)ptmp->idx <= getmaxy(list->win))) {
		list->lastvisible = ptmp;
		ptmp = ptmp->next_entry;
	}
}

static void scrollservwin(struct portlist *table, int direction)
{
	wattrset(table->win, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;

			wscrl(table->win, 1);
			scrollok(table->win, 0);
			mvwprintw(table->win, LINES - 6, 0, "%*c", COLS - 2, ' ');
			scrollok(table->win, 1);

			printportent(table, table->lastvisible);
		}
	} else {
		if (table->firstvisible != table->head) {
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			wscrl(table->win, -1);
			mvwprintw(table->win, 0, 0, "%*c", COLS - 2, ' ');

			printportent(table, table->firstvisible);
		}
	}
}

static void move_bar_one(struct portlist *table, int direction)
{
	switch (direction) {
	case SCROLLDOWN:
		if (table->barptr->prev_entry == NULL)
			break;

		if (table->barptr == table->firstvisible)
			scrollservwin(table, SCROLLDOWN);

		table->barptr = table->barptr->prev_entry;
		printportent(table, table->barptr->next_entry);	/* hide bar */
		printportent(table, table->barptr);		/* show bar */

		break;
	case SCROLLUP:
		if (table->barptr->next_entry == NULL)
			break;

		if (table->barptr == table->lastvisible)
			scrollservwin(table, SCROLLUP);

		table->barptr = table->barptr->next_entry;
		printportent(table, table->barptr->prev_entry);	/* hide bar */
		printportent(table, table->barptr);		/* show bar */

		break;
	}
}

static void move_bar_many(struct portlist *table, int direction, int lines)
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
	refresh_serv_screen(table);
}

static void move_bar(struct portlist *table, int direction, int lines)
{
	if (table->barptr == NULL)
		return;
	if (lines < 1)
		return;
	if (lines < 16)
		while (lines--)
			move_bar_one(table, direction);
	else
		move_bar_many(table, direction, lines);

	print_serv_rates(table);
}

static void show_portsort_keywin(WINDOW ** win, PANEL ** panel)
{
	*win = newwin(14, 35, (LINES - 10) / 2, COLS - 40);
	*panel = new_panel(*win);

	wattrset(*win, DLGBOXATTR);
	tx_colorwin(*win);
	tx_box(*win, ACS_VLINE, ACS_HLINE);

	wattrset(*win, DLGTEXTATTR);
	mvwprintw(*win, 2, 2, "Select sort criterion");
	wmove(*win, 4, 2);
	tx_printkeyhelp("R", " - port number", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 5, 2);
	tx_printkeyhelp("P", " - total packets", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 6, 2);
	tx_printkeyhelp("B", " - total bytes", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 7, 2);
	tx_printkeyhelp("T", " - packets to", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 8, 2);
	tx_printkeyhelp("O", " - bytes to", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 9, 2);
	tx_printkeyhelp("F", " - packets from", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 10, 2);
	tx_printkeyhelp("M", " - bytes from", *win, DLGHIGHATTR, DLGTEXTATTR);
	wmove(*win, 11, 2);
	tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	update_panels();
	doupdate();
}

static void update_serv_rates(struct portlist *list, unsigned long msecs)
{
	/* update rates of all portlistents */
	for (struct portlistent *ple = list->head; ple != NULL; ple = ple->next_entry) {
		rate_add_rate(&ple->rate, ple->span.proto_total.pc_bytes, msecs);
		rate_add_rate(&ple->rate_in, ple->span.proto_in.pc_bytes, msecs);
		rate_add_rate(&ple->rate_out, ple->span.proto_out.pc_bytes, msecs);

		proto_counter_reset(&ple->span);
	}
}

static void serv_process_key(struct portlist *table, int ch)
{
	static WINDOW *sortwin;
	static PANEL *sortpanel;

	static int keymode = 0;

	if (keymode == 0) {
		switch (ch) {
		case KEY_UP:
			move_bar(table, SCROLLDOWN, 1);
			break;
		case KEY_DOWN:
			move_bar(table, SCROLLUP, 1);
			break;
		case KEY_PPAGE:
		case '-':
			move_bar(table, SCROLLDOWN, LINES - 5);
			break;
		case KEY_NPAGE:
		case ' ':
			move_bar(table, SCROLLUP, LINES - 5);
			break;
		case KEY_HOME:
			move_bar(table, SCROLLDOWN, INT_MAX);
			break;
		case KEY_END:
			move_bar(table, SCROLLUP, INT_MAX);
			break;
		case 12:
		case 'l':
		case 'L':
			tx_refresh_screen();
			break;
		case 's':
		case 'S':
			show_portsort_keywin(&sortwin, &sortpanel);
			keymode = 1;
			break;
		case 'q':
		case 'Q':
		case 'x':
		case 'X':
		case 27:
		case 24:
			exitloop = 1;
		}
	} else if (keymode == 1) {
		del_panel(sortpanel);
		delwin(sortwin);
		sortportents(table, ch);
		keymode = 0;
		refresh_serv_screen(table);
		table->barptr = table->firstvisible;
		print_serv_rates(table);
		update_panels();
		doupdate();
	}
}

static void serv_process_packet(struct portlist *table, struct pkt_hdr *pkt,
				struct porttab *ports)
{
	unsigned int tot_br;
	in_port_t sport = 0;
	in_port_t dport = 0;

	int pkt_result = packet_process(pkt, &tot_br, &sport, &dport,
					MATCH_OPPOSITE_USECONFIG,
					options.v6inv4asv6);

	if (pkt_result != PACKET_OK)
		return;

	unsigned short iplen;
	switch (pkt->pkt_protocol) {
	case ETH_P_IP:
		iplen =	ntohs(pkt->iphdr->tot_len);
		break;
	case ETH_P_IPV6:
		iplen = ntohs(pkt->ip6_hdr->ip6_plen) + 40;
		break;
	default:
		/* unknown link protocol */
		return;
	}

	__u8 ip_protocol = pkt_ip_protocol(pkt);
	switch (ip_protocol) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		updateportent(table, ip_protocol, sport, dport, iplen, ports);
		break;
	default:
		/* unknown L4 protocol */
		return;
	}
}

/*
 * The TCP/UDP service monitor
 */

void servmon(char *ifname, time_t facilitytime)
{
	int logging = options.logging;

	int ch;

	struct portlist list;

	FILE *logfile = NULL;

	struct capt capt;

	struct porttab *ports;

	struct pkt_hdr pkt;

	if (!dev_up(ifname)) {
		err_iface_down();
		return;
	}

	initportlist(&list);
	loadaddports(&ports);

	if (capt_init(&capt, ifname) == -1) {
		write_error("Unable to initialize packet capture interface");
		goto err;
	}

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			snprintf(current_logfile, 80, "%s-%s.log", TCPUDPLOG,
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
		signal(SIGUSR1, rotate_serv_log);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** TCP/UDP service monitor started ********");
	}

	if (options.servnames)
		setservent(1);

	packet_init(&pkt);

	exitloop = 0;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	struct timespec last_time = now;
	struct timespec next_screen_update = { 0 };
	time_t starttime = now.tv_sec;
	time_t endtime = INT_MAX;
	if (facilitytime != 0)
		endtime = now.tv_sec + facilitytime * 60;

	time_t log_next = INT_MAX;
	if (logging)
		log_next = now.tv_sec + options.logspan;

	while (!exitloop) {
		clock_gettime(CLOCK_MONOTONIC, &now);

		if (now.tv_sec > last_time.tv_sec) {
			unsigned long rate_msecs = timespec_diff_msec(&now, &last_time);
			/* update all portlistent rates ... */
			update_serv_rates(&list, rate_msecs);
			/* ... and print the current one */
			print_serv_rates(&list);

			printelapsedtime(now.tv_sec - starttime, 20, list.borderwin);

			print_packet_drops(capt_get_dropped(&capt), list.borderwin, 49);

			if (now.tv_sec > endtime)
				exitloop = 1;

			if (logging && (now.tv_sec > log_next)) {
				check_rotate_flag(&logfile);
				writeutslog(list.head, now.tv_sec - starttime,
					    logfile);
				log_next = now.tv_sec + options.logspan;
			}

			last_time = now;
		}

		if (time_after(&now, &next_screen_update)) {
			refresh_serv_screen(&list);

			update_panels();
			doupdate();

			set_next_screen_update(&next_screen_update, &now);
		}

		if (capt_get_packet(&capt, &pkt, &ch, list.win) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR)
			serv_process_key(&list, ch);

		if (pkt.pkt_len > 0) {
			serv_process_packet(&list, &pkt, ports);
			capt_put_packet(&capt, &pkt);
		}
	}
	packet_destroy(&pkt);

	if (options.servnames)
		endservent();

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writeutslog(list.head, time(NULL) - starttime, logfile);
		writelog(logging, logfile,
			 "******** TCP/UDP service monitor stopped ********");
		fclose(logfile);
	}
	strcpy(current_logfile, "");

	capt_destroy(&capt);
err:
	destroyporttab(ports);
	destroyportlist(&list);
}

static void portdlg(in_port_t *port_min, in_port_t *port_max,
		    int *aborted)
{
	WINDOW *bw;
	PANEL *bp;
	WINDOW *win;
	PANEL *panel;

	struct FIELDLIST list;

	bw = newwin(14, 50, (LINES - 14) / 2, (COLS - 50) / 2 - 10);
	bp = new_panel(bw);

	win = newwin(12, 48, (LINES - 14) / 2 + 1, (COLS - 50) / 2 - 9);
	panel = new_panel(win);

	wattrset(bw, DLGBOXATTR);
	tx_box(bw, ACS_VLINE, ACS_HLINE);

	wattrset(win, DLGTEXTATTR);
	tx_colorwin(win);
	tx_stdwinset(win);
	wtimeout(win, -1);

	mvwprintw(win, 1, 1, "Port numbers below 1024 are reserved for");
	mvwprintw(win, 2, 1, "TCP/IP services, and are normally the only");
	mvwprintw(win, 3, 1, "ones monitored by the TCP/UDP statistics");
	mvwprintw(win, 4, 1, "module.  If you wish to monitor a higher-");
	mvwprintw(win, 5, 1, "numbered port or range of ports, enter it");
	mvwprintw(win, 6, 1, "here.  Fill just the first field for a");
	mvwprintw(win, 7, 1, "single port, or both fields for a range.");

	wmove(win, 11, 1);
	tabkeyhelp(win);
	stdkeyhelp(win);

	tx_initfields(&list, 1, 20, (LINES - 14) / 2 + 10, (COLS - 50) / 2 - 8,
		      DLGTEXTATTR, FIELDATTR);
	mvwprintw(list.fieldwin, 0, 6, "to");

	tx_addfield(&list, 5, 0, 0, "");
	tx_addfield(&list, 5, 0, 9, "");

	int ok;
	do {
		unsigned int val;
		int ret;

		ok = 1;
		tx_fillfields(&list, aborted);

		if (*aborted)
			break;

		ret = strtoul_ui(list.list->buf, 10, &val);
		if (ret == -1 || val > 65535) {
			tui_error(ANYKEY_MSG, "Invalid port");
			ok = 0;
			continue;
		}
		*port_min = val;

		if (list.list->nextfield->buf[0] != '\0') {
			ret = strtoul_ui(list.list->nextfield->buf, 10, &val);
			if (ret == -1 || val > 65535 || *port_min > val) {
				tui_error(ANYKEY_MSG, "Invalid port");
				ok = 0;
				continue;
			}
			*port_max = val;
		} else
			*port_max = 0;
	} while (!ok);
	del_panel(bp);
	delwin(bw);
	del_panel(panel);
	delwin(win);
	tx_destroyfields(&list);
}

static void saveportlist(struct porttab *table)
{
	struct porttab *ptmp = table;
	int fd;
	int bw;

	fd = open(PORTFILE, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd < 0) {
		tui_error(ANYKEY_MSG, "Unable to open port list file");
		return;
	}
	while (ptmp != NULL) {
		bw = write(fd, &ptmp->port_min, sizeof(ptmp->port_min));
		bw = write(fd, &ptmp->port_max, sizeof(ptmp->port_max));

		if (bw < 0) {
			tui_error(ANYKEY_MSG,
				  "Unable to write port/range entry");
			destroyporttab(table);
			close(fd);
			return;
		}
		ptmp = ptmp->next_entry;
	}

	close(fd);
}

static int dup_portentry(struct porttab *table, unsigned int min,
			 unsigned int max)
{
	struct porttab *ptmp = table;

	while (ptmp != NULL) {
		if ((ptmp->port_min == min) && (ptmp->port_max == max))
			return 1;

		ptmp = ptmp->next_entry;
	}

	return 0;
}

void addmoreports(struct porttab **table)
{
	in_port_t port_min = 0, port_max = 0;
	int aborted;
	struct porttab *ptmp;

	portdlg(&port_min, &port_max, &aborted);

	if (!aborted) {
		if (dup_portentry(*table, port_min, port_max))
			tui_error(ANYKEY_MSG, "Duplicate port/range entry");
		else {
			ptmp = xmalloc(sizeof(struct porttab));

			ptmp->port_min = port_min;
			ptmp->port_max = port_max;
			ptmp->prev_entry = NULL;
			ptmp->next_entry = *table;

			if (*table != NULL)
				(*table)->prev_entry = ptmp;

			*table = ptmp;
			saveportlist(*table);
		}
	}
	update_panels();
	doupdate();
}

void loadaddports(struct porttab **table)
{
	int fd;
	struct porttab *ptemp;
	struct porttab *tail = NULL;
	int br;

	*table = NULL;

	fd = open(PORTFILE, O_RDONLY);
	if (fd < 0)
		return;

	do {
		ptemp = xmalloc(sizeof(struct porttab));

		br = read(fd, &ptemp->port_min, sizeof(ptemp->port_min));
		br = read(fd, &ptemp->port_max, sizeof(ptemp->port_max));

		if (br < 0) {
			tui_error(ANYKEY_MSG, "Error reading port list");
			close(fd);
			destroyporttab(*table);
			return;
		}
		if (br > 0) {
			if (*table == NULL) {
				*table = ptemp;
				ptemp->prev_entry = NULL;
			}
			if (tail != NULL) {
				tail->next_entry = ptemp;
				ptemp->prev_entry = tail;
			}
			tail = ptemp;
			ptemp->next_entry = NULL;
		} else
			free(ptemp);

	} while (br > 0);

	close(fd);
}

static void operate_portselect(struct porttab **table, struct porttab **node,
			       int *aborted)
{
	struct scroll_list list;
	char listtext[20];

	tx_init_listbox(&list, 25, 22, (COLS - 25) / 2, (LINES - 22) / 2,
			STDATTR, BOXATTR, BARSTDATTR, HIGHATTR);

	tx_set_listbox_title(&list, "Select Port/Range", 1);

	*node = *table;
	while (*node != NULL) {
		snprintf(listtext, 20, "%d to %d", (*node)->port_min,
			 (*node)->port_max);
		tx_add_list_entry(&list, (char *) *node, listtext);
		*node = (*node)->next_entry;
	}

	tx_show_listbox(&list);
	tx_operate_listbox(&list, aborted);

	if (!(*aborted))
		*node = (struct porttab *) list.textptr->nodeptr;

	tx_close_listbox(&list);
	tx_destroy_list(&list);
}

static void selectport(struct porttab **table, struct porttab **node,
		       int *aborted)
{
	if (*table == NULL) {
		tui_error(ANYKEY_MSG, "No custom ports");
		return;
	}

	operate_portselect(table, node, aborted);
}

static void delport(struct porttab **table, struct porttab *ptmp)
{
	if (ptmp != NULL) {
		if (ptmp == *table) {
			*table = (*table)->next_entry;
			if (*table != NULL)
				(*table)->prev_entry = NULL;
		} else {
			ptmp->prev_entry->next_entry = ptmp->next_entry;

			if (ptmp->next_entry != NULL)
				ptmp->next_entry->prev_entry = ptmp->prev_entry;
		}

		free(ptmp);
	}
}

void removeaport(struct porttab **table)
{
	int aborted;
	struct porttab *ptmp = NULL;

	selectport(table, &ptmp, &aborted);

	if (!aborted && ptmp) {
		delport(table, ptmp);
		saveportlist(*table);
	}
}

void destroyporttab(struct porttab *table)
{
	while (table != NULL) {
		struct porttab *ptemp = table->next_entry;

		free(table);
		table = ptemp;
	}
}
