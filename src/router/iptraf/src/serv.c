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
#include "isdntab.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "packet.h"
#include "ipfrag.h"
#include "ifaces.h"
#include "attrs.h"
#include "serv.h"
#include "servname.h"
#include "log.h"
#include "timer.h"
#include "promisc.h"
#include "options.h"
#include "instances.h"
#include "packet.h"
#include "logvars.h"
#include "error.h"
#include "bar.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

#define LEFT 0
#define RIGHT 1

extern int exitloop;
extern int daemonized;

struct serv_spans {
	int spanbr_in;
	int spanbr_out;
	int spanbr;
};

struct portlistent {
	unsigned int port;
	unsigned int protocol;
	char servname[11];
	unsigned int idx;
	unsigned long long count;
	unsigned long long bcount;
	unsigned long long icount;
	unsigned long long ibcount;
	unsigned long long ocount;
	unsigned long long obcount;
	time_t starttime;
	time_t proto_starttime;
	struct serv_spans spans;
	struct portlistent *prev_entry;
	struct portlistent *next_entry;
};

struct portlist {
	struct portlistent *head;
	struct portlistent *tail;
	struct portlistent *firstvisible;
	struct portlistent *lastvisible;
	struct portlistent *barptr;
	int imaxy;
	unsigned int baridx;
	unsigned int count;
	unsigned long bcount;
	WINDOW *win;
	PANEL *panel;
	WINDOW *borderwin;
	PANEL *borderpanel;
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

static void writeutslog(struct portlistent *list, unsigned long nsecs,
			int units, FILE *fd)
{
	char atime[TIME_TARGET_MAX];
	struct portlistent *ptmp = list;
	float inrate, outrate, totalrate;
	time_t now = time(NULL);

	genatime(time(NULL), atime);

	fprintf(fd, "\n*** TCP/UDP traffic log, generated %s\n\n", atime);

	while (ptmp != NULL) {
		if (now - ptmp->proto_starttime < 5)
			inrate = outrate = totalrate = -1.0;
		else {
			if (units == KBITS) {
				inrate =
				    (float) (ptmp->ibcount * 8 / 1000) /
				    (float) (now - ptmp->proto_starttime);
				outrate =
				    (float) (ptmp->obcount * 8 / 1000) /
				    (float) (now - ptmp->proto_starttime);
				totalrate =
				    (float) (ptmp->bcount * 8 / 1000) /
				    (float) (now - ptmp->proto_starttime);
			} else {
				inrate =
				    (float) (ptmp->obcount / 1024) /
				    (float) (now - ptmp->proto_starttime);
				outrate =
				    (float) (ptmp->obcount / 1024) /
				    (float) (now - ptmp->proto_starttime);
				totalrate =
				    (float) (ptmp->obcount / 1024) /
				    (float) (now - ptmp->proto_starttime);
			}
		}

		if (ptmp->protocol == IPPROTO_TCP)
			fprintf(fd, "TCP/%s: ", ptmp->servname);
		else
			fprintf(fd, "UDP/%s: ", ptmp->servname);

		fprintf(fd, "%llu packets, %llu bytes total", ptmp->count,
			ptmp->bcount);

		if (totalrate >= 0.0)
			fprintf(fd, ", %.2f %s", totalrate, dispmode(units));

		fprintf(fd, "; %llu packets, %llu bytes incoming", ptmp->icount,
			ptmp->ibcount);

		if (inrate >= 0.0)
			fprintf(fd, ", %.2f %s", inrate, dispmode(units));

		fprintf(fd, "; %llu packets, %llu bytes outgoing", ptmp->ocount,
			ptmp->obcount);

		if (outrate >= 0.0)
			fprintf(fd, ", %.2f %s", outrate, dispmode(units));

		fprintf(fd, "\n\n");
		ptmp = ptmp->next_entry;
	}

	fprintf(fd, "\nRunning time: %lu seconds\n", nsecs);
	fflush(fd);
}

static void initportlist(struct portlist *list)
{
	float screen_scale = ((float) COLS / 80 + 1) / 2;
	int scratchx __unused;

	list->head = list->tail = list->barptr = NULL;
	list->firstvisible = list->lastvisible = NULL;
	list->count = 0;
	list->baridx = 0;

	list->borderwin = newwin(LINES - 3, COLS, 1, 0);
	list->borderpanel = new_panel(list->borderwin);
	wattrset(list->borderwin, BOXATTR);
	tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);

	wmove(list->borderwin, 0, 1 * screen_scale);
	wprintw(list->borderwin, " Proto/Port ");
	wmove(list->borderwin, 0, 22 * screen_scale);
	wprintw(list->borderwin, " Pkts ");
	wmove(list->borderwin, 0, 31 * screen_scale);
	wprintw(list->borderwin, " Bytes ");
	wmove(list->borderwin, 0, 40 * screen_scale);
	wprintw(list->borderwin, " PktsTo ");
	wmove(list->borderwin, 0, 49 * screen_scale);
	wprintw(list->borderwin, " BytesTo ");
	wmove(list->borderwin, 0, 58 * screen_scale);
	wprintw(list->borderwin, " PktsFrom ");
	wmove(list->borderwin, 0, 67 * screen_scale);
	wprintw(list->borderwin, " BytesFrom ");

	list->win = newwin(LINES - 5, COLS - 2, 2, 1);
	list->panel = new_panel(list->win);
	getmaxyx(list->win, list->imaxy, scratchx);

	tx_stdwinset(list->win);
	wtimeout(list->win, -1);
	wattrset(list->win, STDATTR);
	tx_colorwin(list->win);
	update_panels();
	doupdate();
}

static struct portlistent *addtoportlist(struct portlist *list,
					 unsigned int protocol,
					 unsigned int port, int servnames)
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

	/*
	 * Obtain appropriate service name
	 */

	servlook(servnames, htons(port), protocol, ptemp->servname, 10);

	ptemp->count = ptemp->bcount = 0;
	ptemp->icount = ptemp->ibcount = 0;
	ptemp->ocount = ptemp->obcount = 0;

	list->count++;
	ptemp->idx = list->count;

	ptemp->proto_starttime = time(NULL);

	if (list->count <= LINES - 5)
		list->lastvisible = ptemp;

	wmove(list->borderwin, LINES - 4, 1);
	wprintw(list->borderwin, " %u entries ", list->count);

	return ptemp;
}

static int portinlist(struct porttab *table, unsigned int port)
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

static int goodport(unsigned int port, struct porttab *table)
{
	return ((port < 1024) || (portinlist(table, port)));
}

static struct portlistent *inportlist(struct portlist *list,
				      unsigned int protocol, unsigned int port)
{
	struct portlistent *ptmp = list->head;

	while (ptmp != NULL) {
		if ((ptmp->port == port) && (ptmp->protocol == protocol))
			return ptmp;

		ptmp = ptmp->next_entry;
	}

	return NULL;
}

static void printportent(struct portlist *list, struct portlistent *entry,
		  unsigned int idx)
{
	unsigned int target_row;
	float screen_scale = ((float) COLS / 80 + 1) / 2;
	int tcplabelattr;
	int udplabelattr;
	int highattr;
	char sp_buf[10];

	if ((entry->idx < idx) || (entry->idx > idx + (LINES - 6)))
		return;

	target_row = entry->idx - idx;

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
	sprintf(sp_buf, "%%%dc", COLS - 2);
	scrollok(list->win, 0);
	mvwprintw(list->win, target_row, 0, sp_buf, ' ');
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
	printlargenum(entry->count, list->win);
	wmove(list->win, target_row, 27 * screen_scale);
	printlargenum(entry->bcount, list->win);
	wmove(list->win, target_row, 37 * screen_scale);
	printlargenum(entry->icount, list->win);
	wmove(list->win, target_row, 47 * screen_scale);
	printlargenum(entry->ibcount, list->win);
	wmove(list->win, target_row, 57 * screen_scale);
	printlargenum(entry->ocount, list->win);
	wmove(list->win, target_row, 67 * screen_scale);
	printlargenum(entry->obcount, list->win);
}

static void destroyportlist(struct portlist *list)
{
	struct portlistent *ptmp = list->head;
	struct portlistent *ctmp = NULL;

	if (list->head != NULL)
		ctmp = list->head->next_entry;

	while (ptmp != NULL) {
		free(ptmp);
		ptmp = ctmp;

		if (ctmp != NULL)
			ctmp = ctmp->next_entry;
	}
}

static void updateportent(struct portlist *list, unsigned int protocol,
			  unsigned int sport, unsigned int dport, int br,
			  unsigned int idx, struct porttab *ports,
			  int servnames)
{
	struct portlistent *sport_listent = NULL;
	struct portlistent *dport_listent = NULL;

	if (goodport(sport, ports)) {
		sport_listent = inportlist(list, protocol, sport);

		if (!sport_listent)
			sport_listent =
				addtoportlist(list, protocol, sport, servnames);

		if (sport_listent == NULL)
			return;

		sport_listent->count++;
		sport_listent->bcount += br;
		sport_listent->spans.spanbr += br;

		sport_listent->obcount += br;
		sport_listent->spans.spanbr_out += br;
		sport_listent->ocount++;
	}

	if (goodport(dport, ports)) {
		dport_listent = inportlist(list, protocol, dport);

		if (!dport_listent)
			dport_listent =
				addtoportlist(list, protocol, dport, servnames);

		if (dport_listent == NULL)
			return;

		if (dport_listent != sport_listent) {
			dport_listent->count++;
			dport_listent->bcount += br;
			dport_listent->spans.spanbr += br;
		}

		dport_listent->ibcount += br;
		dport_listent->spans.spanbr_in += br;
		dport_listent->icount++;
	}
	if (sport_listent != NULL || dport_listent != NULL) {
		if (sport_listent != NULL)
			printportent(list, sport_listent, idx);
		if (dport_listent != NULL && dport_listent != sport_listent)
			printportent(list, dport_listent, idx);
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
		result = entry->bcount;
		break;
	case 'O':
		result = entry->ibcount;
		break;
	case 'M':
		result = entry->obcount;
		break;
	case 'P':
		result = entry->count;
		break;
	case 'T':
		result = entry->icount;
		break;
	case 'F':
		result = entry->ocount;
		break;
	}

	return result;
}

/*
 * Refresh TCP/UDP service screen.
 */

static void refresh_serv_screen(struct portlist *table, int idx)
{
	struct portlistent *ptmp = table->firstvisible;

	wattrset(table->win, STDATTR);
	tx_colorwin(table->win);

	while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
		printportent(table, ptmp, idx);
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

static void sortportents(struct portlist *list, unsigned int *idx, int command)
{
	struct portlistent *ptemp1;
	int idxtmp;

	if (!(list->head))
		return;

	command = toupper(command);

	if ((command != 'R') && (command != 'B') && (command != 'O')
	    && (command != 'M') && (command != 'P') && (command != 'T')
	    && (command != 'F'))
		return;

	quicksort_port_entries(list, list->head, list->tail, command);

	ptemp1 = list->firstvisible = list->head;
	*idx = 1;
	idxtmp = 1;

	while ((ptemp1) && (idxtmp <= LINES - 5)) {	/* printout */
		printportent(list, ptemp1, *idx);
		if (idxtmp <= LINES - 5)
			list->lastvisible = ptemp1;
		ptemp1 = ptemp1->next_entry;
		idxtmp++;
	}
}

static void scrollservwin(struct portlist *table, int direction,
			  unsigned int *idx)
{
	char sp_buf[10];

	sprintf(sp_buf, "%%%dc", COLS - 2);
	wattrset(table->win, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			wscrl(table->win, 1);
			table->lastvisible = table->lastvisible->next_entry;
			table->firstvisible = table->firstvisible->next_entry;
			(*idx)++;
			wmove(table->win, LINES - 6, 0);
			scrollok(table->win, 0);
			wprintw(table->win, sp_buf, ' ');
			scrollok(table->win, 1);
			printportent(table, table->lastvisible, *idx);
		}
	} else {
		if (table->firstvisible != table->head) {
			wscrl(table->win, -1);
			table->lastvisible = table->lastvisible->prev_entry;
			table->firstvisible = table->firstvisible->prev_entry;
			(*idx)--;
			wmove(table->win, 0, 0);
			wprintw(table->win, sp_buf, ' ');
			printportent(table, table->firstvisible, *idx);
		}
	}
}

static void pageservwin(struct portlist *table, int direction,
			unsigned int *idx)
{
	int i = 1;

	if (direction == SCROLLUP) {
		while ((i <= LINES - 9) && (table->lastvisible != table->tail)) {
			i++;
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;
			(*idx)++;
		}
	} else {
		while ((i <= LINES - 9) && (table->firstvisible != table->head)) {
			i++;
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			(*idx)--;
		}
	}
	refresh_serv_screen(table, *idx);
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

static void update_serv_rates(struct portlist *list, WINDOW * win, int actmode,
		       int *cleared)
{
	float inrate, outrate, totalrate;
	time_t now = time(NULL);

	if (actmode == KBITS) {
		inrate =
		    (float) (list->barptr->spans.spanbr_in * 8 / 1000) /
		    (float) (now - list->barptr->starttime);
		outrate =
		    (float) (list->barptr->spans.spanbr_out * 8 / 1000) /
		    (float) (now - list->barptr->starttime);
		totalrate =
		    (float) (list->barptr->spans.spanbr * 8 / 1000) /
		    (float) (now - list->barptr->starttime);
	} else {
		inrate =
		    (float) (list->barptr->spans.spanbr_in / 1024) /
		    (float) (now - list->barptr->starttime);
		outrate =
		    (float) (list->barptr->spans.spanbr_out / 1024) /
		    (float) (now - list->barptr->starttime);
		totalrate =
		    (float) (list->barptr->spans.spanbr / 1024) / (float) (now -
									   list->
									   barptr->
									   starttime);
	}

	wattrset(win, IPSTATLABELATTR);
	mvwprintw(win, 0, 1,
		  "Protocol data rates (%s):              in             out            total",
		  dispmode(actmode));
	wattrset(win, IPSTATATTR);
	mvwprintw(win, 0, 31, "%10.2f", inrate);
	mvwprintw(win, 0, 46, "%10.2f", outrate);
	mvwprintw(win, 0, 61, "%10.2f", totalrate);

	memset(&(list->barptr->spans), 0, sizeof(struct serv_spans));
	list->barptr->starttime = time(NULL);
	*cleared = 0;
}

/*
 * The TCP/UDP service monitor
 */

void servmon(char *ifname, struct porttab *ports, const struct OPTIONS *options,
	     time_t facilitytime, struct filterstate *ofilter)
{
	int logging = options->logging;
	int pkt_result;

	int keymode = 0;

	unsigned int idx = 1;

	unsigned int sport = 0;
	unsigned int dport = 0;

	struct timeval tv;
	time_t starttime, startlog, timeint;
	time_t now;
	unsigned long long unow;
	time_t updtime = 0;
	unsigned long long updtime_usec = 0;

	unsigned int tot_br;

	int ch;

	struct portlist list;
	struct portlistent *serv_tmp;
	int statcleared = 0;

	FILE *logfile = NULL;

	struct promisc_states *promisc_list;

	WINDOW *sortwin;
	PANEL *sortpanel;

	WINDOW *statwin;
	PANEL *statpanel;

	char sp_buf[10];

	const int statx = 1;

	int fd;

	/*
	 * Mark this facility
	 */

	if (!facility_active(TCPUDPIDFILE, ifname))
		mark_facility(TCPUDPIDFILE, "TCP/UDP monitor", ifname);
	else {
		write_error("TCP/UDP monitor already running on %s", ifname);
		return;
	}

	if (!dev_up(ifname)) {
		err_iface_down();
		unmark_facility(TCPUDPIDFILE, ifname);
		return;
	}

	if ((first_active_facility()) && (options->promisc)) {
		init_promisc_list(&promisc_list);
		save_promisc_list(promisc_list);
		srpromisc(1, promisc_list);
		destroy_promisc_list(&promisc_list);
	}

	adjust_instance_count(PROCCOUNTFILE, 1);

	initportlist(&list);
	statwin = newwin(1, COLS, LINES - 2, 0);
	statpanel = new_panel(statwin);
	scrollok(statwin, 0);
	wattrset(statwin, IPSTATLABELATTR);
	sprintf(sp_buf, "%%%dc", COLS);
	mvwprintw(statwin, 0, 0, sp_buf, ' ');

	move(LINES - 1, 1);
	scrollkeyhelp();
	sortkeyhelp();
	stdexitkeyhelp();

	if (options->servnames)
		setservent(1);

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

	//isdnfd = -1;
	exitloop = 0;
	gettimeofday(&tv, NULL);
	starttime = startlog = timeint = tv.tv_sec;

	wattrset(statwin, IPSTATATTR);
	mvwprintw(statwin, 0, 1, "No entries");
	update_panels();
	doupdate();

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

	while (!exitloop) {
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;
		unow = tv.tv_sec * 1000000ULL + tv.tv_usec;

		if (now - timeint >= 5) {
			printelapsedtime(starttime, now, LINES - 4, 20,
					 list.borderwin);
			timeint = now;
		}
		if (logging) {
			check_rotate_flag(&logfile);
			if ((now - startlog) >= options->logspan) {
				writeutslog(list.head, now - starttime,
					    options->actmode, logfile);
				startlog = now;
			}
		}

		if (list.barptr != NULL) {
			if ((now - list.barptr->starttime) >= 5) {
				update_serv_rates(&list, statwin,
						  options->actmode,
						  &statcleared);
			}
		}

		if (((options->updrate != 0)
		     && (now - updtime >= options->updrate))
		    || ((options->updrate == 0)
			&& (unow - updtime_usec >= DEFAULT_UPDATE_DELAY))) {
			update_panels();
			doupdate();
			updtime = now;
			updtime_usec = unow;
		}

		if ((facilitytime != 0)
		    && (((now - starttime) / 60) >= facilitytime))
			exitloop = 1;

		if (packet_get(fd, &pkt, &ch, list.win) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch == ERR)
			goto no_key_ready;

		if (keymode == 0) {
			switch (ch) {
			case KEY_UP:
				if (!list.barptr
				    || !list.barptr->prev_entry)
					break;

				serv_tmp = list.barptr;
				set_barptr((void *) &(list.barptr),
					   list.barptr->prev_entry,
					   &(list.barptr->prev_entry->starttime),
					   &(list.barptr->prev_entry->spans),
					   sizeof(struct serv_spans),
					   statwin, &statcleared, statx);
				printportent(&list, serv_tmp, idx);

				if (list.baridx == 1)
					scrollservwin(&list, SCROLLDOWN, &idx);
				else
					list.baridx--;

				printportent(&list, list.barptr, idx);
				break;
			case KEY_DOWN:
				if (!list.barptr
				    || !list.barptr->next_entry)
					break;

				serv_tmp = list.barptr;
				set_barptr((void *) &(list.barptr),
					   list.barptr->next_entry,
					   &(list.barptr->next_entry->starttime),
					   &(list.barptr->next_entry->spans),
					   sizeof(struct serv_spans),
					   statwin, &statcleared, statx);
				printportent(&list,serv_tmp, idx);

				if (list.baridx == list.imaxy)
					scrollservwin(&list, SCROLLUP, &idx);
				else
					list.baridx++;

				printportent(&list, list.barptr, idx);
				break;
			case KEY_PPAGE:
			case '-':
				if (!list.barptr)
					break;

				pageservwin(&list, SCROLLDOWN, &idx);

				set_barptr((void *) &(list.barptr),
					   list.lastvisible,
					   &(list.lastvisible->starttime),
					   &(list.lastvisible->spans),
					   sizeof(struct serv_spans),
					   statwin, &statcleared, statx);
				list.baridx = list.lastvisible->idx - idx + 1;

				refresh_serv_screen(&list, idx);
				break;
			case KEY_NPAGE:
			case ' ':
				if (!list.barptr)
					break;

				pageservwin(&list, SCROLLUP, &idx);

				set_barptr((void *) &(list.barptr),
					   list.firstvisible,
					   &(list.firstvisible->starttime),
					   &(list.firstvisible->spans),
					   sizeof(struct serv_spans),
					   statwin, &statcleared, statx);
				list.baridx = 1;

				refresh_serv_screen(&list, idx);

				break;
			case 12:
			case 'l':
			case 'L':
				tx_refresh_screen();
				break;
			case 's':
			case 'S':
				show_portsort_keywin(&sortwin,
						     &sortpanel);
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
			sortportents(&list, &idx, ch);
			keymode = 0;
			if (list.barptr != NULL) {
				set_barptr((void *) &(list.barptr),
					   list.firstvisible,
					   &(list.firstvisible->
					     starttime),
					   &(list.firstvisible->spans),
					   sizeof(struct serv_spans),
					   statwin, &statcleared,
					   statx);
				list.baridx = 1;
			}
			refresh_serv_screen(&list, idx);
			update_panels();
			doupdate();
		}
	no_key_ready:

		if (pkt.pkt_len <= 0)
			continue;

		unsigned short ipproto;
		unsigned short iplen;

		pkt_result =
			packet_process(&pkt, &tot_br, &sport, &dport,
				      ofilter,
				      MATCH_OPPOSITE_USECONFIG,
				      options->v6inv4asv6);

		if (pkt_result != PACKET_OK)
			continue;

		switch (pkt.pkt_protocol) {
		case ETH_P_IP:
			ipproto = ((struct iphdr *) pkt.pkt_payload)->protocol;
			iplen =	ntohs(((struct iphdr *) pkt.pkt_payload)->tot_len);
			break;
		case ETH_P_IPV6:
			ipproto = ((struct ip6_hdr *) pkt.pkt_payload)->ip6_nxt;	/* FIXME: extension headers ??? */
			iplen = ntohs(((struct ip6_hdr *) pkt.pkt_payload)->ip6_plen) + 40;
			break;
		default:
			/* unknown link protocol */
			continue;
		}
		switch (ipproto) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			updateportent(&list, ipproto, ntohs(sport),
				      ntohs(dport), iplen, idx, ports,
				      options->servnames);
			break;
		default:
			/* unknown L4 protocol */
			continue;
		}
		if ((list.barptr == NULL) && (list.head != NULL)) {
			set_barptr((void *) &(list.barptr), list.head,
				   &(list.head->starttime),
				   &(list.head->spans),
				   sizeof(struct serv_spans), statwin,
				   &statcleared, statx);
			list.baridx = 1;
		}
	}

err_close:
	close(fd);
err:
	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writeutslog(list.head, time(NULL) - starttime,
			    options->actmode, logfile);
		writelog(logging, logfile,
			 "******** TCP/UDP service monitor stopped ********");
		fclose(logfile);
	}
	if (options->servnames)
		endservent();

	if ((options->promisc) && (is_last_instance())) {
		load_promisc_list(&promisc_list);
		srpromisc(0, promisc_list);
		destroy_promisc_list(&promisc_list);
	}

	adjust_instance_count(PROCCOUNTFILE, -1);

	del_panel(list.panel);
	delwin(list.win);
	del_panel(list.borderpanel);
	delwin(list.borderwin);
	del_panel(statpanel);
	delwin(statwin);
	unmark_facility(TCPUDPIDFILE, ifname);
	update_panels();
	doupdate();
	destroyportlist(&list);
	pkt_cleanup();
	strcpy(current_logfile, "");
}

static void portdlg(unsigned int *port_min, unsigned int *port_max,
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
		bw = write(fd, &(ptmp->port_min), sizeof(unsigned int));
		bw = write(fd, &(ptmp->port_max), sizeof(unsigned int));

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
	unsigned int port_min, port_max;
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

		br = read(fd, &(ptemp->port_min), sizeof(unsigned int));
		br = read(fd, &(ptemp->port_max), sizeof(unsigned int));

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
	int ch = 0;
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
	tx_operate_listbox(&list, &ch, aborted);

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
	struct porttab *ptmp;

	selectport(table, &ptmp, &aborted);

	if (!aborted) {
		delport(table, ptmp);
		saveportlist(*table);
	}
}

void destroyporttab(struct porttab *table)
{
	struct porttab *ptemp = table;
	struct porttab *ctemp = NULL;

	if (ptemp != NULL)
		ctemp = ptemp->next_entry;

	while (ptemp != NULL) {
		free(ptemp);
		ptemp = ctemp;

		if (ctemp != NULL)
			ctemp = ctemp->next_entry;
	}
}
