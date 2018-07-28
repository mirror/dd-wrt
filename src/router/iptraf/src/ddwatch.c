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
#include "ddwatch.h"
#include "servname.h"
#include "log.h"
#include "timer.h"
#include "promisc.h"
#include "options.h"
#include "packet.h"
#include "logvars.h"
#include "error.h"
#include "counters.h"
#include "rate.h"
#include "sockaddr.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

#define LEFT 0
#define RIGHT 1

struct addresslistent {
  struct sockaddr_storage addr;


  in_port_t port; 

  unsigned int protocol;
  char servname[11];
  unsigned int idx;
  struct proto_counter serv_count;
  struct proto_counter span;
  
  struct timeval proto_starttime;
  
  struct rate rate;
  struct rate rate_in;
  struct rate rate_out;
  
  struct addresslistent *prev_entry;
  struct addresslistent *next_entry;
};

struct addresslist {
  struct addresslistent *head;
  struct addresslistent *tail;
  struct addresslistent *firstvisible;
  struct addresslistent *lastvisible;
  struct addresslistent *barptr;
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

static void writeutslog(struct addresslistent *list, unsigned long nsecs, FILE *fd)
{
  char atime[TIME_TARGET_MAX];
  struct addresslistent *ptmp = list;
  struct timeval now;

  gettimeofday(&now, NULL);

  genatime(time(NULL), atime);

  fprintf(fd, "\n*** TCP/UDP traffic log, generated %s\n\n", atime);

  while (ptmp != NULL) {
    unsigned long secs = timeval_diff_msec(&now, &ptmp->proto_starttime) / 1000UL;
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

static void initaddresslist(struct addresslist *list)
{
  float screen_scale = ((float) COLS / 80 + 1) / 2;

  list->head = list->tail = list->barptr = NULL;
  list->firstvisible = list->lastvisible = NULL;
  list->count = 0;

  list->borderwin = newwin(LINES - 3, COLS, 1, 0);
  list->borderpanel = new_panel(list->borderwin);
  wattrset(list->borderwin, BOXATTR);
  tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);

  mvwprintw(list->borderwin, 0,  1 * screen_scale, " Destination IP ");
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

static void print_serv_rates(struct addresslist *table)
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

static struct addresslistent *addtoaddresslist(struct addresslist *list,
					       struct sockaddr_storage addr)
{
  struct addresslistent *ptemp;

  ptemp = xmalloc(sizeof(struct addresslistent));
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

  ptemp->addr = addr;	/* This is used in checks later. */
  rate_alloc(&ptemp->rate, 5);
  rate_alloc(&ptemp->rate_in, 5);
  rate_alloc(&ptemp->rate_out, 5);

  /*
   * Obtain appropriate service name
   */


  proto_counter_reset(&ptemp->serv_count);
  proto_counter_reset(&ptemp->span);

  list->count++;
  ptemp->idx = list->count;

  gettimeofday(&ptemp->proto_starttime, NULL);

  if (list->count <= (unsigned) LINES - 5)
    list->lastvisible = ptemp;

  mvwprintw(list->borderwin, LINES - 4, 1, " %u entries ", list->count);

  if (list->barptr == NULL)
    list->barptr = ptemp;

  return ptemp;
}

static int portinlist(struct addtab *table, in_port_t port)
{
  struct addtab *ptmp = table;

  while (ptmp != NULL) {
    if (((ptmp->port_max == 0) && (ptmp->port_min == port))
	|| ((port >= ptmp->port_min) && (port <= ptmp->port_max)))
      return 1;

    ptmp = ptmp->next_entry;
  }

  return 0;
}

static int goodport(in_port_t port, struct addtab *table)
{
  return ((port < 1024) || (portinlist(table, port)));
}

static struct addresslistent *inaddresslist(struct addresslist *list,
					    struct sockaddr_storage *addr)
{
  struct addresslistent *ptmp = list->head;

  while (ptmp != NULL) {
    if (sockaddr_is_equal(addr, &(ptmp->addr)))
      return ptmp;

    ptmp = ptmp->next_entry;
  }
  return NULL;
}

    static void printaddresslistent(struct addresslist *list, struct addresslistent *entry)
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

      struct sockaddr_in *sin = (struct sockaddr_in *)&entry->addr;
      
      switch (entry->addr.ss_family) {
      case AF_INET: {
	struct sockaddr_in *sa = (struct sockaddr_in *)&(entry->addr);
	unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;
	
	
	wprintw(list->win, "%d.%d.%d.%d\          ", ip[0], ip[1], ip[2], ip[3]);
      }
      case AF_INET6: {
	struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&(entry->addr);
	wprintw(list->win, "%s          ", (unsigned char *)&sa->sin6_addr.s6_addr);
	
      }
      }
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

    static void destroyaddresslist(struct addresslist *list)
    {
      struct addresslistent *ptmp = list->head;

      while (ptmp != NULL) {
	struct addresslistent *ctmp = ptmp->next_entry;

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

    static void updateaddresslistent(struct addresslist *list, 
				     struct sockaddr_storage saddr,
				     struct sockaddr_storage daddr, int br)
    {
      struct addresslistent *sadd_listent = NULL;
      struct addresslistent *dadd_listent = NULL;
      enum {
	PORT_INCOMING = 0,
	PORT_OUTGOING
      };


	sadd_listent = inaddresslist(list, &saddr);

	if (!sadd_listent)
	  sadd_listent =
	    addtoaddresslist(list, saddr);

	if (sadd_listent == NULL)
	  return;

	proto_counter_update(&sadd_listent->serv_count, PORT_OUTGOING, br);
	proto_counter_update(&sadd_listent->span, PORT_OUTGOING, br);



	dadd_listent = inaddresslist(list, &daddr);

	if (!dadd_listent)
	  dadd_listent =
	    addtoaddresslist(list, daddr);

	if (dadd_listent == NULL)
	  return;

	proto_counter_update(&dadd_listent->serv_count, PORT_INCOMING, br);
	proto_counter_update(&dadd_listent->span, PORT_INCOMING, br);
    }

    /*
     * Swap two port list entries.  p1 must be previous to p2.
     */

    static void swapportents(struct addresslist *list, struct addresslistent *p1,
			     struct addresslistent *p2)
    {
      register unsigned int tmp;
      struct addresslistent *p1prevsaved;
      struct addresslistent *p2nextsaved;

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
    static unsigned long long qp_getkey(struct addresslistent *entry, int ch)
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

    static void refresh_serv_screen(struct addresslist *table)
    {
      struct addresslistent *ptmp = table->firstvisible;

      wattrset(table->win, STDATTR);
      tx_colorwin(table->win);

      while ((ptmp != NULL) && (ptmp->prev_entry != table->lastvisible)) {
	printaddresslistent(table, ptmp);
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

    static int qp_compare(struct addresslistent *entry, unsigned long long pv, int ch,
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
    static struct addresslistent *qp_partition(struct addresslist *table,
					       struct addresslistent **low,
					       struct addresslistent **high, int ch)
    {
      struct addresslistent *pivot = *low;

      struct addresslistent *left = *low;
      struct addresslistent *right = *high;
      struct addresslistent *ptmp;

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
    static void quicksort_port_entries(struct addresslist *table,
				       struct addresslistent *low,
				       struct addresslistent *high, int ch)
    {
      struct addresslistent *pivot;

      if ((high == NULL) || (low == NULL))
	return;

      if (high->idx > low->idx) {
	pivot = qp_partition(table, &low, &high, ch);

	quicksort_port_entries(table, low, pivot->prev_entry, ch);
	quicksort_port_entries(table, pivot->next_entry, high, ch);
      }
    }

    static void sortportents(struct addresslist *list, int command)
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
      struct addresslistent *ptmp = list->head;
      while (ptmp && ((int)ptmp->idx <= getmaxy(list->win))) {
	list->lastvisible = ptmp;
	ptmp = ptmp->next_entry;
      }
    }

    static void scrollservwin(struct addresslist *table, int direction)
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

	  printaddresslistent(table, table->lastvisible);
	}
      } else {
	if (table->firstvisible != table->head) {
	  table->firstvisible = table->firstvisible->prev_entry;
	  table->lastvisible = table->lastvisible->prev_entry;

	  wscrl(table->win, -1);
	  mvwprintw(table->win, 0, 0, "%*c", COLS - 2, ' ');

	  printaddresslistent(table, table->firstvisible);
	}
      }
    }

    static void move_bar_one(struct addresslist *table, int direction)
    {
      switch (direction) {
      case SCROLLDOWN:
	if (table->barptr->prev_entry == NULL)
	  break;

	if (table->barptr == table->firstvisible)
	  scrollservwin(table, SCROLLDOWN);

	table->barptr = table->barptr->prev_entry;
	printaddresslistent(table, table->barptr->next_entry);	/* hide bar */
	printaddresslistent(table, table->barptr);		/* show bar */

	break;
      case SCROLLUP:
	if (table->barptr->next_entry == NULL)
	  break;

	if (table->barptr == table->lastvisible)
	  scrollservwin(table, SCROLLUP);

	table->barptr = table->barptr->next_entry;
	printaddresslistent(table, table->barptr->prev_entry);	/* hide bar */
	printaddresslistent(table, table->barptr);		/* show bar */

	break;
      }
    }

    static void move_bar_many(struct addresslist *table, int direction, int lines)
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

    static void move_bar(struct addresslist *table, int direction, int lines)
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

    static void update_serv_rates(struct addresslist *list, unsigned long msecs)
    {
      /* update rates of all addresslistents */
      for (struct addresslistent *ple = list->head; ple != NULL; ple = ple->next_entry) {
	rate_add_rate(&ple->rate, ple->span.proto_total.pc_bytes, msecs);
	rate_add_rate(&ple->rate_in, ple->span.proto_in.pc_bytes, msecs);
	rate_add_rate(&ple->rate_out, ple->span.proto_out.pc_bytes, msecs);

	proto_counter_reset(&ple->span);
      }
    }

    static void serv_process_key(struct addresslist *table, int ch)
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

    static void serv_process_packet(struct addresslist *table, struct pkt_hdr *pkt,
				    struct addtab *addresses)
    {
      unsigned int tot_br;
      in_port_t sport = 0;
      in_port_t dport = 0;
      struct sockaddr_storage saddr, daddr;
	
      int pkt_result = packet_process(pkt, &tot_br, &sport, &dport,
				      MATCH_OPPOSITE_USECONFIG,
				      options.v6inv4asv6);

      if (pkt_result != PACKET_OK)
	return;

      unsigned short iplen;
      switch (pkt->pkt_protocol) {
      case ETH_P_IP:
	iplen =	ntohs(pkt->iphdr->tot_len);
	sockaddr_make_ipv4(&saddr, pkt->iphdr->saddr);
	sockaddr_make_ipv4(&daddr, pkt->iphdr->daddr);
	break;
      case ETH_P_IPV6:
	iplen = ntohs(pkt->ip6_hdr->ip6_plen) + 40;
	sockaddr_make_ipv6(&saddr, &pkt->ip6_hdr->ip6_src);
	sockaddr_make_ipv6(&daddr, &pkt->ip6_hdr->ip6_dst);
	break;
      default:
	/* unknown link protocol */
	return;
      }

	
      __u8 ip_protocol = pkt_ip_protocol(pkt);
      switch (ip_protocol) {
      case IPPROTO_TCP:
      case IPPROTO_UDP:
	updateaddresslistent (table, saddr, daddr, iplen);
	break;
      default:
	/* unknown L4 protocol */
	return;
      }
    }

    /*
     * The TCP/UDP service monitor
     */

    void ddmon(char *ifname, time_t facilitytime)
    {
      int logging = options.logging;

      int ch;

      struct addresslist list;

      FILE *logfile = NULL;

      int fd;

      unsigned long dropped = 0UL;

      struct addtab *addresses = NULL;

      struct pkt_hdr pkt;

      if (!dev_up(ifname)) {
	err_iface_down();
	return;
      }

      initaddresslist(&list);

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

      if (logging) {
	if (strcmp(current_logfile, "") == 0) {
	  snprintf(current_logfile, 80, "%s-%s.log", DDDESTLOG,
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
		 "******** DD WATCH service monitor started ********");
      }

      if (options.servnames)
	setservent(1);

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
	  unsigned long rate_msecs = timeval_diff_msec(&now, &last_time);
	  /* update all addresslistent rates ... */
	  update_serv_rates(&list, rate_msecs);
	  /* ... and print the current one */
	  print_serv_rates(&list); 

	  printelapsedtime(now.tv_sec - starttime, 20, list.borderwin);

	  dropped += packet_get_dropped(fd);
	  print_packet_drops(dropped, list.borderwin, 49);

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

	if (screen_update_needed(&now, &last_update)) {
	  refresh_serv_screen(&list);

	  update_panels();
	  doupdate();

	  last_update = now;
	}

	if (packet_get(fd, &pkt, &ch, list.win) == -1) {
	  write_error("Packet receive failed");
	  exitloop = 1;
	  break;
	}

	if (ch != ERR)
	  serv_process_key(&list, ch);

	if (pkt.pkt_len > 0)
	  serv_process_packet(&list, &pkt, addresses);
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

    err_close:
      close(fd);
    err:
      if (options.promisc) {
	promisc_restore_list(&promisc);
	promisc_destroy(&promisc);
      }

      destroyaddtab(addresses);
      destroyaddresslist(&list);
    }

    void ddloadaddports(struct addtab **table)
    {
      int fd;
      struct addtab *ptemp;
      struct addtab *tail = NULL;
      int br;

      *table = NULL;

      fd = open(PORTFILE, O_RDONLY);
      if (fd < 0)
	return;

      do {
	ptemp = xmalloc(sizeof(struct addtab));

	br = read(fd, &(ptemp->port_min), sizeof(unsigned int));
	br = read(fd, &(ptemp->port_max), sizeof(unsigned int));

	if (br < 0) {
	  tui_error(ANYKEY_MSG, "Error reading port list");
	  close(fd);
	  destroyaddtab(*table);
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




    void destroyaddtab(struct addtab *table)
    {
      while (table != NULL) {
	struct addtab *ptemp = table->next_entry;

	free(table);
	table = ptemp;
      }
    }
