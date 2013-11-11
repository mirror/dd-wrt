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
#include "promisc.h"
#include "error.h"
#include "ifstats.h"
#include "rate.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

struct iflist {
	char ifname[IFNAMSIZ];
	int ifindex;
	unsigned int encap;
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

		struct iflist *itmp = xmallocz(sizeof(struct iflist));
		strcpy(itmp->ifname, ifname);
		itmp->ifindex = ifindex;
		rate_alloc(&itmp->rate, 5);

		/* make the linked list sorted by ifindex */
		struct iflist *cur = *list, *last = NULL;
		while (cur != NULL && cur->ifindex < ifindex) {
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
		struct iflist *itmp = xmallocz(sizeof(struct iflist));
		itmp->ifindex = ifindex;
		itmp->index = last->index + 1;
		int r = dev_get_ifname(ifindex, itmp->ifname);
		if (r != 0) {
			write_error("Error getting interface name");
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
	struct iflist *ctmp;
	struct iflist *ptmp;

	if (list != NULL) {
		ptmp = list;
		ctmp = ptmp->next_entry;

		do {
			rate_destroy(&ptmp->rate);
			free(ptmp);
			ptmp = ctmp;
			if (ctmp != NULL)
				ctmp = ctmp->next_entry;
		} while (ptmp != NULL);
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

static void printifentry(struct iflist *ptmp, WINDOW * win, unsigned int idx)
{
	unsigned int target_row;

	if ((ptmp->index < idx) || (ptmp->index > idx + (LINES - 5)))
		return;

	target_row = ptmp->index - idx;

	wattrset(win, STDATTR);
	wmove(win, target_row, 1);
	wprintw(win, "%s", ptmp->ifname);
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
		printifentry(ptmp, table->statwin, table->firstvisible->index);

		if (i <= winht)
			table->lastvisible = ptmp;

		ptmp = ptmp->next_entry;
		i++;
	} while ((ptmp != NULL) && (i <= winht));
}

static void labelstats(WINDOW *win)
{
	wmove(win, 0, 1);
	wprintw(win, " Iface ");
	/* 14, 24, 34, ... from printifentry() */
	/* 10 = strlen(printed number); from printlargenum() */
	/* 7 = strlen(" Total ") */
	/* 1 = align the string on 'l' from " Total " */
	wmove(win, 0, (14 * COLS / 80) + 10 - 7 + 1);
	wprintw(win, " Total ");
	wmove(win, 0, (24 * COLS / 80) + 10 - 6 + 1);
	wprintw(win, " IPv4 ");
	wmove(win, 0, (34 * COLS / 80) + 10 - 6 + 1);
	wprintw(win, " IPv6 ");
	wmove(win, 0, (44 * COLS / 80) + 10 - 7 + 1);
	wprintw(win, " NonIP ");
	wmove(win, 0, (53 * COLS / 80) + 8 - 7 + 1);
	wprintw(win, " BadIP ");
	wmove(win, 0, (63 * COLS / 80) + 14 - 10);
	wprintw(win, " Activity ");
}

static void initiftab(struct iftab *table)
{
	table->borderwin = newwin(LINES - 2, COLS, 1, 0);
	table->borderpanel = new_panel(table->borderwin);

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
	wattrset(table->statwin, BOXATTR);
	wmove(table->borderwin, LINES - 3, 32 * COLS / 80);
	wprintw(table->borderwin,
		" Total, IP, NonIP, and BadIP are packet counts ");
}

/*
 * Scrolling routines for the general interface statistics window
 */

static void scrollgstatwin(struct iftab *table, int direction)
{
	char buf[255];

	sprintf(buf, "%%%dc", COLS - 2);
	wattrset(table->statwin, STDATTR);
	if (direction == SCROLLUP) {
		if (table->lastvisible->next_entry != NULL) {
			wscrl(table->statwin, 1);
			table->lastvisible = table->lastvisible->next_entry;
			table->firstvisible = table->firstvisible->next_entry;
			wmove(table->statwin, LINES - 5, 0);
			scrollok(table->statwin, 0);
			wprintw(table->statwin, buf, ' ');
			scrollok(table->statwin, 1);
			printifentry(table->lastvisible, table->statwin,
				     table->firstvisible->index);
		}
	} else {
		if (table->firstvisible != table->head) {
			wscrl(table->statwin, -1);
			table->firstvisible = table->firstvisible->prev_entry;
			table->lastvisible = table->lastvisible->prev_entry;
			wmove(table->statwin, 0, 0);
			wprintw(table->statwin, buf, ' ');
			printifentry(table->firstvisible, table->statwin,
				     table->firstvisible->index);
		}
	}
}

static void pagegstatwin(struct iftab *table, int direction)
{
	int i = 1;

	if (direction == SCROLLUP) {
		while ((i <= LINES - 5)
		       && (table->lastvisible->next_entry != NULL)) {
			i++;
			scrollgstatwin(table, direction);
		}
	} else {
		while ((i <= LINES - 5) && (table->firstvisible != table->head)) {
			i++;
			scrollgstatwin(table, direction);
		}
	}
}


/*
 * The general interface statistics function
 */

void ifstats(time_t facilitytime)
{
	int logging = options.logging;
	struct iftab table;

	int pkt_result = 0;

	struct iflist *ptmp = NULL;

	FILE *logfile = NULL;

	int ch;

	int fd;

	struct timeval tv;
	time_t starttime = 0;
	time_t statbegin = 0;
	time_t now = 0;
	struct timeval start_tv;
	time_t startlog = 0;
	struct timeval updtime;

	initiflist(&(table.head));
	if (!table.head) {
		no_ifaces_error();
		return;
	}

	initiftab(&table);

	LIST_HEAD(promisc);
	if (options.promisc) {
		promisc_init(&promisc, NULL);
		promisc_set_list(&promisc);
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

	update_panels();
	doupdate();

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(fd == -1) {
		write_error("Unable to obtain monitoring socket");
		goto err;
	}

	exitloop = 0;
	gettimeofday(&tv, NULL);
	start_tv = tv;
	updtime = tv;
	starttime = startlog = statbegin = tv.tv_sec;

	PACKET_INIT(pkt);

	while (!exitloop) {
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;

		if ((now - starttime) >= 1) {
			unsigned long msecs;

			msecs = timeval_diff_msec(&tv, &start_tv);
			updaterates(&table, msecs);
			showrates(&table);
			printelapsedtime(statbegin, now, LINES - 3, 1,
					 table.borderwin);
			starttime = now;
			start_tv = tv;
		}
		if (logging) {
			check_rotate_flag(&logfile);
			if ((now - startlog) >= options.logspan) {
				writegstatlog(&table,
					      time(NULL) - statbegin,
					      logfile);
				startlog = now;
			}
		}
		if (screen_update_needed(&tv, &updtime)) {
			print_if_entries(&table);
			update_panels();
			doupdate();

			updtime = tv;
		}

		if ((facilitytime != 0)
		    && (((now - statbegin) / 60) >= facilitytime))
			exitloop = 1;

		if (packet_get(fd, &pkt, &ch, table.statwin) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		switch (ch) {
		case ERR:
			/* no key ready, do nothing */
			break;
		case KEY_UP:
			scrollgstatwin(&table, SCROLLDOWN);
			break;
		case KEY_DOWN:
			scrollgstatwin(&table, SCROLLUP);
			break;
		case KEY_PPAGE:
		case '-':
			pagegstatwin(&table, SCROLLDOWN);
			break;
		case KEY_NPAGE:
		case ' ':
			pagegstatwin(&table, SCROLLUP);
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
		}
		if (pkt.pkt_len <= 0)
			continue;

		pkt_result = packet_process(&pkt, NULL, NULL, NULL,
					   MATCH_OPPOSITE_USECONFIG,
					   options.v6inv4asv6);

		if (pkt_result != PACKET_OK
		    && pkt_result != MORE_FRAGMENTS)
			continue;

		ptmp = positionptr(table.head, pkt.pkt_ifindex);
		if (!ptmp)
			continue;

		ptmp->total++;

		ptmp->spanbr += pkt.pkt_len;
		ptmp->br += pkt.pkt_len;

		if (pkt.pkt_protocol == ETH_P_IP) {
			ptmp->iptotal++;

			if (pkt_result == CHECKSUM_ERROR) {
				(ptmp->badtotal)++;
				continue;
			}
		} else if (pkt.pkt_protocol == ETH_P_IPV6) {
			ptmp->ip6total++;
		} else {
			(ptmp->noniptotal)++;
		}
	}
	close(fd);

err:
	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	del_panel(table.statpanel);
	delwin(table.statwin);
	del_panel(table.borderpanel);
	delwin(table.borderwin);
	update_panels();
	doupdate();

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writegstatlog(&table, time(NULL) - statbegin, logfile);
		writelog(logging, logfile,
			 "******** General interface statistics stopped ********");
		fclose(logfile);
	}
	destroyiflist(table.head);
	pkt_cleanup();
	strcpy(current_logfile, "");
}

void selectiface(char *ifname, int withall, int *aborted)
{
	int ch;

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
		ptmp = xmalloc(sizeof(struct iflist));
		strncpy(ptmp->ifname, "All interfaces", sizeof(ptmp->ifname));
		ptmp->ifindex = 0;
		rate_alloc(&ptmp->rate, 5);	/* FIXME: need iflist_entry_init() */

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
	tx_operate_listbox(&scrolllist, &ch, aborted);
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
