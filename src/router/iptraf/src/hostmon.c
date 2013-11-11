/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

hostmon.c - Host traffic monitor
Discovers LAN hosts and displays packet statistics for them

***/

#include "iptraf-ng-compat.h"

#include "tui/labels.h"
#include "tui/winops.h"

#include "dirs.h"
#include "deskman.h"
#include "fltdefs.h"
#include "packet.h"
#include "ifaces.h"
#include "hostmon.h"
#include "attrs.h"
#include "log.h"
#include "timer.h"
#include "landesc.h"
#include "options.h"
#include "logvars.h"
#include "promisc.h"
#include "error.h"
#include "rate.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

struct ethtabent {
	int type;
	union {
		struct {
			unsigned long long inpcount;
			unsigned long long inbcount;
			unsigned long long inippcount;
			unsigned long inspanbr;
			unsigned long long outpcount;
			unsigned long long outbcount;
			unsigned long long outippcount;
			unsigned long outspanbr;
			struct rate inrate;
			struct rate outrate;
		} figs;

		struct {
			char eth_addr[ETH_ALEN];
			char ascaddr[18];
			char desc[65];
			char ifname[IFNAMSIZ];
			int withdesc;
			int printed;
			unsigned int linktype;
		} desc;
	} un;

	unsigned int index;
	struct ethtabent *prev_entry;
	struct ethtabent *next_entry;
};

struct ethtab {
	struct ethtabent *head;
	struct ethtabent *tail;
	struct ethtabent *firstvisible;
	struct ethtabent *lastvisible;
	unsigned long count;
	unsigned long entcount;
	int units;
	WINDOW *borderwin;
	PANEL *borderpanel;
	WINDOW *tabwin;
	PANEL *tabpanel;
};

/*
 * SIGUSR1 logfile rotation handler
 */

static void rotate_lanlog(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_lanlog);
}

static void writeethlog(struct ethtabent *list, unsigned long nsecs, FILE *fd)
{
	char atime[TIME_TARGET_MAX];
	struct ethtabent *ptmp = list;

	genatime(time(NULL), atime);

	fprintf(fd, "\n*** LAN traffic log, generated %s\n\n", atime);

	while (ptmp != NULL) {
		if (ptmp->type == 0) {
			if (ptmp->un.desc.linktype == ARPHRD_ETHER)
				fprintf(fd, "\nEthernet address: %s",
					ptmp->un.desc.ascaddr);
			else if (ptmp->un.desc.linktype == ARPHRD_FDDI)
				fprintf(fd, "\nFDDI address: %s",
					ptmp->un.desc.ascaddr);

			if (ptmp->un.desc.withdesc)
				fprintf(fd, " (%s)", ptmp->un.desc.desc);

			fprintf(fd, "\n");
		} else {
			fprintf(fd,
				"\tIncoming total %llu packets, %llu bytes; %llu IP packets\n",
				ptmp->un.figs.inpcount, ptmp->un.figs.inbcount,
				ptmp->un.figs.inippcount);
			fprintf(fd,
				"\tOutgoing total %llu packets, %llu bytes; %llu IP packets\n",
				ptmp->un.figs.outpcount,
				ptmp->un.figs.outbcount,
				ptmp->un.figs.outippcount);

			fprintf(fd, "\tAverage rates: ");
			char buf_in[32];
			char buf_out[32];
			rate_print(ptmp->un.figs.inbcount / nsecs, buf_in, sizeof(buf_in));
			rate_print(ptmp->un.figs.outbcount / nsecs, buf_out, sizeof(buf_out));
			fprintf(fd, "%s incoming, %s outgoing\n",
				buf_in, buf_out);

			if (nsecs > 5) {
				rate_print(rate_get_average(&ptmp->un.figs.inrate),
					   buf_in, sizeof(buf_in));
				rate_print(rate_get_average(&ptmp->un.figs.outrate),
					   buf_out, sizeof(buf_out));
				fprintf(fd,
					"\tLast 5-second rates: %s incoming, %s outgoing\n",
					buf_in, buf_out);
			}
		}

		ptmp = ptmp->next_entry;
	}

	fprintf(fd, "\nRunning time: %lu seconds\n", nsecs);
	fflush(fd);
}

static void initethtab(struct ethtab *table)
{
	table->head = table->tail = NULL;
	table->firstvisible = table->lastvisible = NULL;
	table->count = table->entcount = 0;

	table->borderwin = newwin(LINES - 2, COLS, 1, 0);
	table->borderpanel = new_panel(table->borderwin);

	table->tabwin = newwin(LINES - 4, COLS - 2, 2, 1);
	table->tabpanel = new_panel(table->tabwin);

	wattrset(table->borderwin, BOXATTR);
	tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
	wmove(table->borderwin, 0, 5 * COLS / 80);
	wprintw(table->borderwin, " PktsIn ");
	wmove(table->borderwin, 0, 16 * COLS / 80);
	wprintw(table->borderwin, " IP In ");
	wmove(table->borderwin, 0, 24 * COLS / 80);
	wprintw(table->borderwin, " BytesIn ");
	wmove(table->borderwin, 0, 34 * COLS / 80);
	wprintw(table->borderwin, " InRate ");

	wmove(table->borderwin, 0, 42 * COLS / 80);
	wprintw(table->borderwin, " PktsOut ");
	wmove(table->borderwin, 0, 53 * COLS / 80);
	wprintw(table->borderwin, " IP Out ");
	wmove(table->borderwin, 0, 61 * COLS / 80);
	wprintw(table->borderwin, " BytesOut ");
	wmove(table->borderwin, 0, 70 * COLS / 80);
	wprintw(table->borderwin, " OutRate ");

	wmove(table->borderwin, LINES - 3, 40);

	wprintw(table->borderwin, " InRate and OutRate are in %s ",
		dispmode(options.actmode));

	wattrset(table->tabwin, STDATTR);
	tx_colorwin(table->tabwin);
	tx_stdwinset(table->tabwin);
	wtimeout(table->tabwin, -1);

	update_panels();
	doupdate();
}

static struct ethtabent *addethnode(struct ethtab *table)
{
	struct ethtabent *ptemp;

	ptemp = xmalloc(sizeof(struct ethtabent));

	if (table->head == NULL) {
		ptemp->prev_entry = NULL;
		table->head = ptemp;
		table->firstvisible = ptemp;
	} else {
		ptemp->prev_entry = table->tail;
		table->tail->next_entry = ptemp;
	}

	table->tail = ptemp;
	ptemp->next_entry = NULL;

	table->count++;
	ptemp->index = table->count;

	if (table->count <= (unsigned) LINES - 4)
		table->lastvisible = ptemp;

	return ptemp;
}

void convmacaddr(char *addr, char *result)
{
	u_int8_t *ptmp = (u_int8_t *) addr;

	sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x",
			*ptmp,
			*(ptmp + 1),
			*(ptmp + 2),
			*(ptmp + 3),
			*(ptmp + 4),
			*(ptmp + 5));
}

static struct ethtabent *addethentry(struct ethtab *table,
				     unsigned int linktype, char *ifname,
				     char *addr, struct eth_desc *list)
{
	struct ethtabent *ptemp;

	ptemp = addethnode(table);

	if (ptemp == NULL)
		return NULL;

	ptemp->type = 0;
	memcpy(&(ptemp->un.desc.eth_addr), addr, ETH_ALEN);
	strcpy(ptemp->un.desc.desc, "");

	convmacaddr(addr, ptemp->un.desc.ascaddr);

	ptemp->un.desc.linktype = linktype;
	struct eth_desc *desc = NULL;

	list_for_each_entry(desc, &list->hd_list, hd_list)
		if (!strcasecmp(desc->hd_mac, ptemp->un.desc.ascaddr))
			strcpy(ptemp->un.desc.desc, desc->hd_desc);

	strcpy(ptemp->un.desc.ifname, ifname);

	if (strcmp(ptemp->un.desc.desc, "") == 0)
		ptemp->un.desc.withdesc = 0;
	else
		ptemp->un.desc.withdesc = 1;

	ptemp->un.desc.printed = 0;

	ptemp = addethnode(table);

	if (ptemp == NULL)
		return NULL;

	ptemp->type = 1;
	ptemp->un.figs.inpcount = 0;
	ptemp->un.figs.outpcount = 0;
	ptemp->un.figs.inspanbr = ptemp->un.figs.outspanbr = 0;
	ptemp->un.figs.inippcount = ptemp->un.figs.outippcount = 0;
	ptemp->un.figs.inbcount = ptemp->un.figs.outbcount = 0;
	rate_alloc(&ptemp->un.figs.inrate, 5);
	rate_alloc(&ptemp->un.figs.outrate, 5);

	table->entcount++;

	wmove(table->borderwin, LINES - 3, 1);
	wprintw(table->borderwin, " %u entries ", table->entcount);

	return ptemp;
}

static struct ethtabent *in_ethtable(struct ethtab *table,
				     unsigned int linktype, char *addr)
{
	struct ethtabent *ptemp = table->head;

	while (ptemp != NULL) {
		if ((ptemp->type == 0)
		    && (memcmp(addr, ptemp->un.desc.eth_addr, ETH_ALEN) == 0)
		    && (ptemp->un.desc.linktype == linktype))
			return ptemp->next_entry;

		ptemp = ptemp->next_entry;
	}

	return NULL;
}

static void updateethent(struct ethtabent *entry, int pktsize, int is_ip,
			 int inout)
{
	if (inout == 0) {
		entry->un.figs.inpcount++;
		entry->un.figs.inbcount += pktsize;
		entry->un.figs.inspanbr += pktsize;
		if (is_ip)
			entry->un.figs.inippcount++;
	} else {
		entry->un.figs.outpcount++;
		entry->un.figs.outbcount += pktsize;
		entry->un.figs.outspanbr += pktsize;
		if (is_ip)
			entry->un.figs.outippcount++;
	}
}

static void printethent(struct ethtab *table, struct ethtabent *entry,
			unsigned int idx)
{
	unsigned int target_row;

	if ((entry->index < idx) || (entry->index > idx + LINES - 5))
		return;

	target_row = entry->index - idx;

	if (entry->type == 0) {
		wmove(table->tabwin, target_row, 1);
		wattrset(table->tabwin, STDATTR);

		if (entry->un.desc.linktype == ARPHRD_ETHER)
			wprintw(table->tabwin, "Ethernet");
		else if (entry->un.desc.linktype == ARPHRD_FDDI)
			wprintw(table->tabwin, "FDDI");

		wprintw(table->tabwin, " HW addr: %s", entry->un.desc.ascaddr);

		if (entry->un.desc.withdesc)
			wprintw(table->tabwin, " (%s)", entry->un.desc.desc);

		wprintw(table->tabwin, " on %s       ", entry->un.desc.ifname);

		entry->un.desc.printed = 1;
	} else {
		wattrset(table->tabwin, PTRATTR);
		wmove(table->tabwin, target_row, 1);
		waddch(table->tabwin, ACS_LLCORNER);

		wattrset(table->tabwin, HIGHATTR);

		/* Inbound traffic counts */

		wmove(table->tabwin, target_row, 2 * COLS / 80);
		printlargenum(entry->un.figs.inpcount, table->tabwin);
		wmove(table->tabwin, target_row, 12 * COLS / 80);
		printlargenum(entry->un.figs.inippcount, table->tabwin);
		wmove(table->tabwin, target_row, 22 * COLS / 80);
		printlargenum(entry->un.figs.inbcount, table->tabwin);

		/* Outbound traffic counts */

		wmove(table->tabwin, target_row, 40 * COLS / 80);
		printlargenum(entry->un.figs.outpcount, table->tabwin);
		wmove(table->tabwin, target_row, 50 * COLS / 80);
		printlargenum(entry->un.figs.outippcount, table->tabwin);
		wmove(table->tabwin, target_row, 60 * COLS / 80);
		printlargenum(entry->un.figs.outbcount, table->tabwin);
	}
}

static void destroyethtab(struct ethtab *table)
{
	struct ethtabent *ptemp = table->head;
	struct ethtabent *cnext = NULL;

	if (table->head != NULL)
		cnext = table->head->next_entry;

	while (ptemp != NULL) {
		if (ptemp->type == 1) {
			rate_destroy(&ptemp->un.figs.outrate);
			rate_destroy(&ptemp->un.figs.inrate);
		}
		free(ptemp);
		ptemp = cnext;

		if (cnext != NULL)
			cnext = cnext->next_entry;
	}
}

static void hostmonhelp(void)
{
	move(LINES - 1, 1);
	scrollkeyhelp();
	sortkeyhelp();
	stdexitkeyhelp();
}

static void printrates(struct ethtab *table, unsigned int target_row,
		       struct ethtabent *ptmp)
{
	char buf[32];

	rate_print_no_units(rate_get_average(&ptmp->un.figs.inrate),
		   buf, sizeof(buf));
	wmove(table->tabwin, target_row, 32 * COLS / 80);
	wprintw(table->tabwin, "%s", buf);

	rate_print_no_units(rate_get_average(&ptmp->un.figs.outrate),
		   buf, sizeof(buf));
	wmove(table->tabwin, target_row, 69 * COLS / 80);
	wprintw(table->tabwin, "%s", buf);
}

static void updateethrates(struct ethtab *table, unsigned long msecs,
			   unsigned int idx)
{
	struct ethtabent *ptmp = table->head;
	unsigned int target_row = 0;

	if (table->lastvisible == NULL)
		return;

	while (ptmp != NULL) {
		if (ptmp->type == 1) {
			rate_add_rate(&ptmp->un.figs.inrate, ptmp->un.figs.inspanbr, msecs);
			ptmp->un.figs.inspanbr = 0;

			rate_add_rate(&ptmp->un.figs.outrate, ptmp->un.figs.outspanbr, msecs);
			ptmp->un.figs.outspanbr = 0;

			if ((ptmp->index >= idx)
			    && (ptmp->index <= idx + LINES - 5)) {
				wattrset(table->tabwin, HIGHATTR);
				target_row = ptmp->index - idx;
				printrates(table, target_row, ptmp);
			}
		}
		ptmp = ptmp->next_entry;
	}
}

static void refresh_hostmon_screen(struct ethtab *table, unsigned int idx)
{
	struct ethtabent *ptmp = table->firstvisible;

	wattrset(table->tabwin, STDATTR);
	tx_colorwin(table->tabwin);

	while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
		printethent(table, ptmp, idx);
		ptmp = ptmp->next_entry;
	}

	update_panels();
	doupdate();
}

static void scrollethwin(struct ethtab *table, int direction, unsigned int *idx)
{
	char sp_buf[10];

	sprintf(sp_buf, "%%%dc", COLS - 2);
	wattrset(table->tabwin, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible != table->tail) {
			wscrl(table->tabwin, 1);
			table->lastvisible = table->lastvisible->next_entry;
			table->firstvisible = table->firstvisible->next_entry;
			(*idx)++;
			wmove(table->tabwin, LINES - 5, 0);
			scrollok(table->tabwin, 0);
			wprintw(table->tabwin, sp_buf, ' ');
			scrollok(table->tabwin, 1);
			printethent(table, table->lastvisible, *idx);
			if (table->lastvisible->type == 1)
				printrates(table, LINES - 5,
					   table->lastvisible);
		}
	} else {
		if (table->firstvisible != table->head) {
			wscrl(table->tabwin, -1);
			table->lastvisible = table->lastvisible->prev_entry;
			table->firstvisible = table->firstvisible->prev_entry;
			(*idx)--;
			wmove(table->tabwin, 0, 0);
			wprintw(table->tabwin, sp_buf, ' ');
			printethent(table, table->firstvisible, *idx);
			if (table->firstvisible->type == 1)
				printrates(table, 0, table->firstvisible);
		}
	}
}

static void pageethwin(struct ethtab *table, int direction, unsigned int *idx)
{
	int i = 1;

	if (direction == SCROLLUP) {
		while ((i <= LINES - 7) && (table->lastvisible != table->tail)) {
			i++;
			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;
			(*idx)++;
		}
	} else {
		while ((i <= LINES - 7) && (table->firstvisible != table->head)) {
			i++;
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			(*idx)--;
		}
	}
	refresh_hostmon_screen(table, *idx);
}

static void show_hostsort_keywin(WINDOW ** win, PANEL ** panel)
{
	*win = newwin(13, 35, (LINES - 10) / 2, COLS - 40);
	*panel = new_panel(*win);

	wattrset(*win, DLGBOXATTR);
	tx_colorwin(*win);
	tx_box(*win, ACS_VLINE, ACS_HLINE);

	wattrset(*win, DLGTEXTATTR);
	mvwprintw(*win, 2, 2, "Select sort criterion");
	wmove(*win, 4, 2);
	tx_printkeyhelp("P", " - total packets in", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 5, 2);
	tx_printkeyhelp("I", " - IP packets in", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 6, 2);
	tx_printkeyhelp("B", " - total bytes in", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 7, 2);
	tx_printkeyhelp("K", " - total packets out", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 8, 2);
	tx_printkeyhelp("O", " - IP packets out", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 9, 2);
	tx_printkeyhelp("Y", " - total bytes out", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	wmove(*win, 10, 2);
	tx_printkeyhelp("Any other key", " - cancel sort", *win, DLGHIGHATTR,
			DLGTEXTATTR);
	update_panels();
	doupdate();
}

/*
 * Swap two host table entries.
 */

static void swaphostents(struct ethtab *list, struct ethtabent *p1,
			 struct ethtabent *p2)
{
	register unsigned int tmp;
	struct ethtabent *p1prevsaved;
	struct ethtabent *p2nextsaved;

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
		list->head = p2;

	if (p2->next_entry->next_entry != NULL)
		p2->next_entry->next_entry->prev_entry = p1->next_entry;
	else
		list->tail = p1->next_entry;

	p2nextsaved = p2->next_entry->next_entry;
	p1prevsaved = p1->prev_entry;

	if (p1->next_entry->next_entry == p2) {
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

static unsigned long long ql_getkey(struct ethtabent *entry, int ch)
{
	unsigned long long result = 0;

	switch (ch) {
	case 'P':
		result = entry->next_entry->un.figs.inpcount;
		break;
	case 'I':
		result = entry->next_entry->un.figs.inippcount;
		break;
	case 'B':
		result = entry->next_entry->un.figs.inbcount;
		break;
	case 'K':
		result = entry->next_entry->un.figs.outpcount;
		break;
	case 'O':
		result = entry->next_entry->un.figs.outippcount;
		break;
	case 'Y':
		result = entry->next_entry->un.figs.outbcount;
		break;
	}
	return result;
}

static struct ethtabent *ql_partition(struct ethtab *table,
				      struct ethtabent **low,
				      struct ethtabent **high, int ch)
{
	struct ethtabent *pivot = *low;

	struct ethtabent *left = *low;
	struct ethtabent *right = *high;
	struct ethtabent *ptmp;

	unsigned long long pivot_value;

	pivot_value = ql_getkey(pivot, ch);

	while (left->index < right->index) {
		while ((ql_getkey(left, ch) >= pivot_value)
		       && (left->next_entry->next_entry != NULL))
			left = left->next_entry->next_entry;

		while (ql_getkey(right, ch) < pivot_value)
			right = right->prev_entry->prev_entry;

		if (left->index < right->index) {
			swaphostents(table, left, right);

			if (*low == left)
				*low = right;

			if (*high == right)
				*high = left;

			ptmp = left;
			left = right;
			right = ptmp;
		}
	}
	swaphostents(table, pivot, right);

	if (*low == pivot)
		*low = right;

	if (*high == right)
		*high = pivot;

	return pivot;
}

/*
 * Quicksort routine for the LAN station monitor
 */

static void quicksort_lan_entries(struct ethtab *table, struct ethtabent *low,
				  struct ethtabent *high, int ch)
{
	struct ethtabent *pivot;

	if ((high == NULL) || (low == NULL))
		return;

	if (high->index > low->index) {
		pivot = ql_partition(table, &low, &high, ch);

		if (pivot->prev_entry != NULL)
			quicksort_lan_entries(table, low,
					      pivot->prev_entry->prev_entry,
					      ch);

		quicksort_lan_entries(table, pivot->next_entry->next_entry,
				      high, ch);
	}
}

static void sort_hosttab(struct ethtab *list, unsigned int *idx, int command)
{
	struct ethtabent *ptemp1;
	int idxtmp;

	if (!list->head)
		return;

	command = toupper(command);

	if ((command != 'P') && (command != 'I') && (command != 'B')
	    && (command != 'K') && (command != 'O') && (command != 'Y'))
		return;

	quicksort_lan_entries(list, list->head, list->tail->prev_entry,
			      command);

	ptemp1 = list->firstvisible = list->head;
	*idx = 1;
	idxtmp = 0;
	tx_colorwin(list->tabwin);
	while ((ptemp1) && (idxtmp <= LINES - 4)) {
		printethent(list, ptemp1, *idx);
		idxtmp++;
		if (idxtmp <= LINES - 4)
			list->lastvisible = ptemp1;
		ptemp1 = ptemp1->next_entry;
	}

}

/*
 * The LAN station monitor
 */

void hostmon(time_t facilitytime, char *ifptr)
{
	int logging = options.logging;
	struct ethtab table;
	struct ethtabent *entry;

	char scratch_saddr[ETH_ALEN];
	char scratch_daddr[ETH_ALEN];
	unsigned int idx = 1;
	int is_ip;
	int ch;

	char *ifname = ifptr;

	struct timeval tv;
	struct timeval tv_rate;
	time_t now = 0;
	time_t statbegin = 0;
	time_t startlog = 0;
	struct timeval updtime;

	struct eth_desc *list = NULL;

	FILE *logfile = NULL;

	int pkt_result;

	WINDOW *sortwin;
	PANEL *sortpanel;
	int keymode = 0;

	int fd;

	if (ifptr && !dev_up(ifptr)) {
		err_iface_down();
		return;
	}

	LIST_HEAD(promisc);
	if (options.promisc) {
		promisc_init(&promisc, ifptr);
		promisc_set_list(&promisc);
	}

	hostmonhelp();

	initethtab(&table);

	/* Ethernet description list */
	struct eth_desc *elist = load_eth_desc(ARPHRD_ETHER);

	/* FDDI description list */
	struct eth_desc *flist = load_eth_desc(ARPHRD_FDDI);

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			strncpy(current_logfile,
				gen_instance_logname(LANLOG, getpid()), 80);

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
		signal(SIGUSR1, rotate_lanlog);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** LAN traffic monitor started ********");
	}

	leaveok(table.tabwin, TRUE);

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
	statbegin = startlog = tv.tv_sec;

	PACKET_INIT(pkt);

	do {
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;

		unsigned long msecs = timeval_diff_msec(&tv, &tv_rate);
		if (msecs >= 1000) {
			printelapsedtime(statbegin, now, LINES - 3, 15,
					 table.borderwin);
			updateethrates(&table, msecs, idx);
			tv_rate = tv;
		}
		if (logging) {
			check_rotate_flag(&logfile);
			if ((now - startlog) >= options.logspan) {
				writeethlog(table.head, now - statbegin,
					    logfile);
				startlog = now;
			}
		}
		if (screen_update_needed(&tv, &updtime)) {
			update_panels();
			doupdate();

			updtime = tv;
		}

		if ((facilitytime != 0)
		    && (((now - statbegin) / 60) >= facilitytime))
			exitloop = 1;

		if (packet_get(fd, &pkt, &ch, table.tabwin) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR) {
			if (keymode == 0) {
				switch (ch) {
				case KEY_UP:
					scrollethwin(&table, SCROLLDOWN, &idx);
					break;
				case KEY_DOWN:
					scrollethwin(&table, SCROLLUP, &idx);
					break;
				case KEY_PPAGE:
				case '-':
					pageethwin(&table, SCROLLDOWN, &idx);
					break;
				case KEY_NPAGE:
				case ' ':
					pageethwin(&table, SCROLLUP, &idx);
					break;
				case 12:
				case 'l':
				case 'L':
					tx_refresh_screen();
					break;
				case 's':
				case 'S':
					show_hostsort_keywin(&sortwin,
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
				sort_hosttab(&table, &idx, ch);
				keymode = 0;
			}
		}

		if (pkt.pkt_len <= 0)
			continue;

		char ifnamebuf[IFNAMSIZ];

		pkt_result =
			packet_process(&pkt, NULL, NULL, NULL,
				       MATCH_OPPOSITE_USECONFIG,
				       0);

		if (pkt_result != PACKET_OK)
			continue;

		if (!ifptr) {
			/* we're capturing on "All interfaces", */
			/* so get the name of the interface */
			/* of this packet */
			int r = dev_get_ifname(pkt.pkt_ifindex, ifnamebuf);
			if (r != 0) {
				write_error("Unable to get interface name");
				break;	/* can't get interface name, get out! */
			}
			ifname = ifnamebuf;
		}

		/* get HW addresses */
		switch (pkt.pkt_hatype) {
		case ARPHRD_ETHER: {
			memcpy(scratch_saddr, pkt.ethhdr->h_source, ETH_ALEN);
			memcpy(scratch_daddr, pkt.ethhdr->h_dest, ETH_ALEN);
			list = elist;
			break; }
		case ARPHRD_FDDI: {
			memcpy(scratch_saddr, pkt.fddihdr->saddr, FDDI_K_ALEN);
			memcpy(scratch_daddr, pkt.fddihdr->daddr, FDDI_K_ALEN);
			list = flist;
			break; }
		default:
			/* unknown link protocol */
			continue;
		}

		switch(pkt.pkt_protocol) {
		case ETH_P_IP:
		case ETH_P_IPV6:
			is_ip = 1;
			break;
		default:
			is_ip = 0;
			break;
		}

		/* Check source address entry */
		entry = in_ethtable(&table, pkt.pkt_hatype,
				    scratch_saddr);

		if (!entry)
			entry = addethentry(&table, pkt.pkt_hatype,
					    ifname, scratch_saddr, list);

		if (entry != NULL) {
			updateethent(entry, pkt.pkt_len, is_ip, 1);
			if (!entry->prev_entry->un.desc.printed)
				printethent(&table, entry->prev_entry,
					    idx);

			printethent(&table, entry, idx);
		}

		/* Check destination address entry */
		entry = in_ethtable(&table, pkt.pkt_hatype,
				    scratch_daddr);
		if (!entry)
			entry = addethentry(&table, pkt.pkt_hatype,
					    ifname, scratch_daddr, list);

		if (entry != NULL) {
			updateethent(entry, pkt.pkt_len, is_ip, 0);
			if (!entry->prev_entry->un.desc.printed)
				printethent(&table, entry->prev_entry,
					    idx);

			printethent(&table, entry, idx);
		}
	} while (!exitloop);

err_close:
	close(fd);

err:
	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writeethlog(table.head, time(NULL) - statbegin, logfile);
		writelog(logging, logfile,
			 "******** LAN traffic monitor stopped ********");
		fclose(logfile);
	}


	del_panel(table.tabpanel);
	delwin(table.tabwin);
	del_panel(table.borderpanel);
	delwin(table.borderwin);
	update_panels();
	doupdate();
	destroyethtab(&table);

	free_eth_desc(elist);
	free_eth_desc(flist);

	strcpy(current_logfile, "");
}
