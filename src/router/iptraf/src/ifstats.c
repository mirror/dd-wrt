/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

ifstats.c	- the interface statistics module

 ***/

#include "iptraf-ng-compat.h"

#include "tui/labels.h"
#include "tui/listbox.h"
#include "tui/msgboxes.h"
#include "tui/winops.h"

#include "ifaces.h"
#include "fltdefs.h"
#include "packet.h"
#include "options.h"
#include "log.h"
#include "dirs.h"
#include "deskman.h"
#include "attrs.h"
#include "serv.h"
#include "timer.h"
#include "logvars.h"
#include "error.h"
#include "ifstats.h"
#include "rate.h"
#include "capt.h"
#include "counters.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

struct iflist {
	char ifname[IFNAMSIZ];
	int ifindex;
	unsigned long long iptotal;
	unsigned long long ip6total;
	unsigned long badtotal;
	unsigned long long noniptotal;
	unsigned long long total;
	unsigned int spanbr;
	unsigned long br;
	struct rate rate;
	unsigned long peakrate;
	unsigned int index;
	struct iflist *prev_entry;
	struct iflist *next_entry;
};

struct iftab {
	struct iflist *head;
	struct iflist *tail;
	struct iflist *firstvisible;
	struct iflist *lastvisible;
	struct pkt_counter totals;
	struct rate rate_total;
	struct rate rate_totalpps;
	WINDOW *borderwin;
	PANEL *borderpanel;
	WINDOW *statwin;
	PANEL *statpanel;
};

/*
 * USR1 log-rotation signal handlers
 */

static void rotate_gstat_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, GSTATLOG);
	signal(SIGUSR1, rotate_gstat_log);
}

static void writegstatlog(struct iftab *table, unsigned long nsecs, FILE *fd)
{
	struct iflist *ptmp = table->head;
	char atime[TIME_TARGET_MAX];

	genatime(time(NULL), atime);
	fprintf(fd, "\n*** General interface statistics log generated %s\n\n",
		atime);

	while (ptmp != NULL) {

		fprintf(fd,
			"%s: %llu total, %llu IP, %llu non-IP, %lu IP checksum errors",
			ptmp->ifname, ptmp->total, ptmp->iptotal,
			ptmp->noniptotal, ptmp->badtotal);

		if (nsecs > 5) {
			char buf[64];

			rate_print(ptmp->br / nsecs, buf, sizeof(buf));
			fprintf(fd, ", average activity %s", buf);
			rate_print(ptmp->peakrate, buf, sizeof(buf));
			fprintf(fd, ", peak activity %s", buf);
			rate_print(rate_get_average(&ptmp->rate), buf, sizeof(buf));
			fprintf(fd, ", last 5-second average activity %s", buf);
		}
		fprintf(fd, "\n");

		ptmp = ptmp->next_entry;
	}

	fprintf(fd, "\n%lu seconds running time\n", nsecs);
	fflush(fd);
}

/*
 * Function to check if an interface is already in the interface list.
 * This eliminates duplicate interface entries due to aliases
 */

static int ifinlist(struct iflist *list, char *ifname)
{
	struct iflist *ptmp = list;
	int result = 0;

	while ((ptmp != NULL) && (result == 0)) {
		result = (strcmp(ifname, ptmp->ifname) == 0);
		ptmp = ptmp->next_entry;
	}

	return result;
}

static struct iflist *alloc_iflist_entry(void)
{
	struct iflist *tmp = xmallocz(sizeof(struct iflist));

	rate_alloc(&tmp->rate, 5);

	return tmp;
}

static void free_iflist_entry(struct iflist *ptr)
{
	if (!ptr)
		return;

	rate_destroy(&ptr->rate);
	free(ptr);
}

/*
 * Initialize the list of interfaces.  This linked list is used in the
 * selection boxes as well as in the general interface statistics screen.
 *
 * This function parses the /proc/net/dev file and grabs the interface names
 * from there.  The SIOGIFFLAGS ioctl() call is used to determine whether the
 * interfaces are active.  Inactive interfaces are omitted from selection
 * lists.
 */

static void initiflist(struct iflist **list)
{
	char ifname[IFNAMSIZ];

	*list = NULL;

	FILE *fd = open_procnetdev();
	if (fd == NULL) {
		tui_error(ANYKEY_MSG, "Unable to obtain interface list");
		return;
	}

	while (get_next_iface(fd, ifname, sizeof(ifname))) {
		if (!*ifname)
			continue;

		if (ifinlist(*list, ifname))	/* ignore entry if already in */
			continue;	/* interface list */

		/*
		 * Check if the interface is actually up running.  This prevents
		 * inactive devices in /proc/net/dev from actually appearing in
		 * interface lists used by IPTraf.
		 */

		if (!dev_up(ifname))
			continue;

		int ifindex = dev_get_ifindex(ifname);
		if (ifindex < 0)
			continue;
		/*
		 * At this point, the interface is now sure to be up and running.
		 */

		struct iflist *itmp = alloc_iflist_entry();
		itmp->ifindex = ifindex;
		strcpy(itmp->ifname, ifname);

		/* make the linked list sorted by ifindex */
		struct iflist *cur = *list, *last = NULL;
		while (cur != NULL && strcmp(cur->ifname, ifname) < 0) {
			last = cur;
			cur = cur->next_entry;
		}
		itmp->prev_entry = last;
		itmp->next_entry = cur;
		if (cur)
			cur->prev_entry = itmp;
		if (last)
			last->next_entry = itmp;
		else
			*list = itmp;
	}
	fclose(fd);

	/* let the index follow the sorted linked list */
	unsigned int index = 1;
	struct iflist *cur;
	for (cur = *list; cur != NULL; cur = cur->next_entry)
		cur->index = index++;
}

static struct iflist *positionptr(struct iflist *iflist, const int ifindex)
{
	struct iflist *ptmp = iflist;
	struct iflist *last = ptmp;

	while ((ptmp != NULL) && (ptmp->ifindex != ifindex)) {
		last = ptmp;
		ptmp = ptmp->next_entry;
	}
	/* no interface was found, try to create new one */
	if (ptmp == NULL) {
		struct iflist *itmp = alloc_iflist_entry();
		itmp->ifindex = ifindex;
		itmp->index = last->index + 1;
		int r = dev_get_ifname(ifindex, itmp->ifname);
		if (r != 0) {
			write_error("Error getting interface name");
			free_iflist_entry(itmp);
			return(NULL);
		}

		/* last can't be NULL otherwise we will have empty iflist */
		last->next_entry = itmp;
		itmp->prev_entry = last;
		itmp->next_entry = NULL;
		ptmp = itmp;
	}
	return(ptmp);
}

static void destroyiflist(struct iflist *list)
{
	struct iflist *ptmp = list;

	while (ptmp != NULL) {
		struct iflist *ctmp = ptmp->next_entry;

		free_iflist_entry(ptmp);
		ptmp = ctmp;
	}
}

static void no_ifaces_error(void)
{
	write_error("No active interfaces. Check their status or the /proc filesystem");
}

static void updaterates(struct iftab *table, unsigned long msecs)
{
	struct iflist *ptmp = table->head;
	unsigned long rate;

	rate_add_rate(&table->rate_total, table->totals.pc_bytes, msecs);
	rate_add_rate(&table->rate_totalpps, table->totals.pc_packets, msecs);
	pkt_counter_reset(&table->totals);
	while (ptmp != NULL) {
		rate_add_rate(&ptmp->rate, ptmp->spanbr, msecs);
		rate = rate_get_average(&ptmp->rate);

		if (rate > ptmp->peakrate)
			ptmp->peakrate = rate;

		ptmp->spanbr = 0;
		ptmp = ptmp->next_entry;
	}
}

static void showrates(struct iftab *table)
{
	struct iflist *ptmp = table->firstvisible;
	unsigned int idx = table->firstvisible->index;
	unsigned long rate;
	char buf[64];

	wattrset(table->statwin, HIGHATTR);
	do {
		rate = rate_get_average(&ptmp->rate);
		rate_print(rate, buf, sizeof(buf));
		wmove(table->statwin, ptmp->index - idx, 63 * COLS / 80);
		wprintw(table->statwin, "%s", buf);

		ptmp = ptmp->next_entry;
	} while (ptmp != table->lastvisible->next_entry);
}

static void printifentry(struct iftab *table, struct iflist *ptmp)
{
	int target_row = ptmp->index - table->firstvisible->index;
	WINDOW *win = table->statwin;

	if ((target_row < 0) || (target_row > LINES - 5))
		return;

	wattrset(win, STDATTR);
	mvwprintw(win, target_row, 1, "%s", ptmp->ifname);
	wattrset(win, HIGHATTR);
	wmove(win, target_row, 14 * COLS / 80);
	printlargenum(ptmp->total, win);
	wmove(win, target_row, 24 * COLS / 80);
	printlargenum(ptmp->iptotal, win);
	wmove(win, target_row, 34 * COLS / 80);
	printlargenum(ptmp->ip6total, win);
	wmove(win, target_row, 44 * COLS / 80);
	printlargenum(ptmp->noniptotal, win);
	wmove(win, target_row, 53 * COLS / 80);
	wprintw(win, "%7lu", ptmp->badtotal);
}

static void print_if_entries(struct iftab *table)
{
	struct iflist *ptmp = table->firstvisible;
	unsigned int i = 1;

	unsigned int winht = LINES - 4;

	do {
		printifentry(table, ptmp);

		if (i <= winht)
			table->lastvisible = ptmp;

		ptmp = ptmp->next_entry;
		i++;
	} while ((ptmp != NULL) && (i <= winht));
}

static void labelstats(WINDOW *win)
{
	mvwprintw(win, 0, 1, " Iface ");
	/* 14, 24, 34, ... from printifentry() */
	/* 10 = strlen(printed number); from printlargenum() */
	/* 7 = strlen(" Total ") */
	/* 1 = align the string on 'l' from " Total " */
	mvwprintw(win, 0, (14 * COLS / 80) + 10 - 7 + 1, " Total ");
	mvwprintw(win, 0, (24 * COLS / 80) + 10 - 6 + 1, " IPv4 ");
	mvwprintw(win, 0, (34 * COLS / 80) + 10 - 6 + 1, " IPv6 ");
	mvwprintw(win, 0, (44 * COLS / 80) + 10 - 7 + 1, " NonIP ");
	mvwprintw(win, 0, (53 * COLS / 80) + 8 - 7 + 1, " BadIP ");
	mvwprintw(win, 0, (63 * COLS / 80) + 14 - 10, " Activity ");
}

static void initiftab(struct iftab *table)
{
	table->borderwin = newwin(LINES - 2, COLS, 1, 0);
	table->borderpanel = new_panel(table->borderwin);

	rate_alloc(&table->rate_total, 5);
	rate_alloc(&table->rate_totalpps, 5);
	pkt_counter_reset(&table->totals);

	move(LINES - 1, 1);
	scrollkeyhelp();
	stdexitkeyhelp();
	wattrset(table->borderwin, BOXATTR);
	tx_box(table->borderwin, ACS_VLINE, ACS_HLINE);
	labelstats(table->borderwin);
	table->statwin = newwin(LINES - 4, COLS - 2, 2, 1);
	table->statpanel = new_panel(table->statwin);
	tx_stdwinset(table->statwin);
	wtimeout(table->statwin, -1);
	wattrset(table->statwin, STDATTR);
	tx_colorwin(table->statwin);
}

/*
 * Scrolling routines for the general interface statistics window
 */

static void scrollgstatwin(struct iftab *table, int direction, int lines)
{
	if (lines < 1)
		return;

	wattrset(table->statwin, STDATTR);
	if (direction == SCROLLUP) {
		for (int i = 0; i < lines; i++) {
			if (table->lastvisible->next_entry == NULL)
				break;

			table->firstvisible = table->firstvisible->next_entry;
			table->lastvisible = table->lastvisible->next_entry;

			wscrl(table->statwin, 1);
			scrollok(table->statwin, 0);
			mvwprintw(table->statwin, LINES - 5, 0, "%*c", COLS - 2, ' ');
			scrollok(table->statwin, 1);

			printifentry(table, table->lastvisible);
		}
	} else {
		for (int i = 0; i < lines; i++) {
			if (table->firstvisible == table->head)
				break;

			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;

			wscrl(table->statwin, -1);
			mvwprintw(table->statwin, 0, 0, "%*c", COLS - 2, ' ');

			printifentry(table, table->firstvisible);
		}
	}
	showrates(table);
}

static void ifstats_process_key(struct iftab *table, int ch)
{
	switch (ch) {
	case KEY_UP:
		scrollgstatwin(table, SCROLLDOWN, 1);
		break;
	case KEY_DOWN:
		scrollgstatwin(table, SCROLLUP, 1);
		break;
	case KEY_PPAGE:
	case '-':
		scrollgstatwin(table, SCROLLDOWN, LINES - 5);
		break;
	case KEY_NPAGE:
	case ' ':
		scrollgstatwin(table, SCROLLUP, LINES - 5);
		break;
	case KEY_HOME:
		scrollgstatwin(table, SCROLLDOWN, INT_MAX);
		break;
	case KEY_END:
		scrollgstatwin(table, SCROLLUP, INT_MAX);
		break;
	case 12:
	case 'l':
	case 'L':
		tx_refresh_screen();
		break;
	case 'Q':
	case 'q':
	case 'X':
	case 'x':
	case 27:
	case 24:
		exitloop = 1;
		break;
	case ERR:
	default:
		/* no key ready, do nothing */
		break;
	}
}

static void ifstats_process_packet(struct iftab *table, struct pkt_hdr *pkt)
{
	int pkt_result = packet_process(pkt, NULL, NULL, NULL,
					MATCH_OPPOSITE_USECONFIG,
					options.v6inv4asv6);

	switch (pkt_result) {
	case PACKET_OK:			/* we only handle these */
	case MORE_FRAGMENTS:
	case CHECKSUM_ERROR:
		break;
	default:			/* drop others */
		return;
	}

	pkt_counter_update(&table->totals, pkt->pkt_len);

	struct iflist *ptmp = positionptr(table->head, pkt->from->sll_ifindex);
	if (!ptmp)
		return;

	ptmp->total++;

	ptmp->spanbr += pkt->pkt_len;
	ptmp->br += pkt->pkt_len;

	if (pkt->pkt_protocol == ETH_P_IP) {
		ptmp->iptotal++;

		if (pkt_result == CHECKSUM_ERROR) {
			ptmp->badtotal++;
			return;
		}
	} else if (pkt->pkt_protocol == ETH_P_IPV6) {
		ptmp->ip6total++;
	} else {
		ptmp->noniptotal++;
	}
}

/*
 * The general interface statistics function
 */

void ifstats(time_t facilitytime)
{
	int logging = options.logging;
	struct iftab table;

	FILE *logfile = NULL;

	int ch;

	struct capt capt;

	struct pkt_hdr pkt;

	initiflist(&(table.head));
	if (!table.head) {
		no_ifaces_error();
		return;
	}

	initiftab(&table);

	if (capt_init(&capt, NULL) == -1) {
		write_error("Unable to initialize packet capture interface");
		goto err;
	}

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			strcpy(current_logfile, GSTATLOG);

			if (!daemonized)
				input_logfile(current_logfile, &logging);
		}
	}

	if (logging) {
		opentlog(&logfile, GSTATLOG);

		if (logfile == NULL)
			logging = 0;
	}
	if (logging) {
		signal(SIGUSR1, rotate_gstat_log);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** General interface statistics started ********");
	}

	table.firstvisible = table.head;
	print_if_entries(&table);
	showrates(&table);

	update_panels();
	doupdate();

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
			unsigned long msecs = timespec_diff_msec(&now, &last_time);
			updaterates(&table, msecs);
			showrates(&table);

			printelapsedtime(now.tv_sec - starttime, 1, table.borderwin);

			print_packet_drops(capt_get_dropped(&capt), table.borderwin, 61);

			wattrset(table.borderwin, BOXATTR);
			char buf[64];
			rate_print(rate_get_average(&table.rate_total), buf, sizeof(buf));
			mvwprintw(table.borderwin,
				  getmaxy(table.borderwin) - 1, 19,
				  " Total: %s / %9lu pps ",
				  buf,
				  rate_get_average(&table.rate_totalpps));

			if (logging && (now.tv_sec > log_next)) {
				check_rotate_flag(&logfile);
				writegstatlog(&table, now.tv_sec - starttime, logfile);
				log_next = now.tv_sec + options.logspan;
			}

			if (now.tv_sec > endtime)
				exitloop = 1;

			last_time = now;
		}
		if (time_after(&now, &next_screen_update)) {
			print_if_entries(&table);
			update_panels();
			doupdate();

			set_next_screen_update(&next_screen_update, &now);
		}

		if (capt_get_packet(&capt, &pkt, &ch, table.statwin) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR)
			ifstats_process_key(&table, ch);

		if (pkt.pkt_len > 0) {
			ifstats_process_packet(&table, &pkt);
			capt_put_packet(&capt, &pkt);
		}

	}
	packet_destroy(&pkt);

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writegstatlog(&table, time(NULL) - starttime, logfile);
		writelog(logging, logfile,
			 "******** General interface statistics stopped ********");
		fclose(logfile);
	}
	strcpy(current_logfile, "");

	capt_destroy(&capt);
err:
	del_panel(table.statpanel);
	delwin(table.statwin);
	del_panel(table.borderpanel);
	delwin(table.borderwin);
	update_panels();
	doupdate();

	destroyiflist(table.head);
	rate_destroy(&table.rate_total);
	rate_destroy(&table.rate_totalpps);
}

void selectiface(char *ifname, int withall, int *aborted)
{
	struct iflist *list;
	struct iflist *ptmp;

	struct scroll_list scrolllist;

	initiflist(&list);

	if (list == NULL) {
		no_ifaces_error();
		*aborted = 1;
		return;
	}

	if ((withall) && (list != NULL)) {
		ptmp = alloc_iflist_entry();
		strncpy(ptmp->ifname, "All interfaces", sizeof(ptmp->ifname));
		ptmp->ifindex = 0;

		ptmp->prev_entry = NULL;
		list->prev_entry = ptmp;
		ptmp->next_entry = list;
		list = ptmp;
	}
	tx_listkeyhelp(STDATTR, HIGHATTR);

	ptmp = list;

	tx_init_listbox(&scrolllist, 24, 14, (COLS - 24) / 2 - 9,
			(LINES - 14) / 2, STDATTR, BOXATTR, BARSTDATTR,
			HIGHATTR);

	tx_set_listbox_title(&scrolllist, "Select Interface", 1);

	while (ptmp != NULL) {
		tx_add_list_entry(&scrolllist, (char *) ptmp, ptmp->ifname);
		ptmp = ptmp->next_entry;
	}

	tx_show_listbox(&scrolllist);
	tx_operate_listbox(&scrolllist, aborted);
	tx_close_listbox(&scrolllist);

	if (!(*aborted) && (list != NULL)) {
		ptmp = (struct iflist *) scrolllist.textptr->nodeptr;
		if ((withall) && (ptmp->prev_entry == NULL))	/* All Interfaces */
			strcpy(ifname, "");
		else
			strcpy(ifname, ptmp->ifname);
	}

	tx_destroy_list(&scrolllist);
	destroyiflist(list);
	update_panels();
	doupdate();
}
