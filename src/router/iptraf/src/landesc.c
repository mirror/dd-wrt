/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

landesc.c	- LAN host description management module
		  Currently includes support for Ethernet, PLIP,
		  and FDDI

***/

#include "iptraf-ng-compat.h"

#include "tui/input.h"
#include "tui/listbox.h"
#include "tui/msgboxes.h"
#include "tui/menurt.h"

#include "landesc.h"
#include "deskman.h"
#include "attrs.h"
#include "dirs.h"

static int check_mac_addr(const char *mac)
{
	if (strlen(mac) != 17)
		return 0;

	char a[3], b[3], c[3], d[3], e[3], f[3];

	int success = sscanf(mac, "%02s:%02s:%02s:%02s:%02s:%02s",
			     a, b, c, d, e, f);

	if (success != 6)
		return 0;

	char mac_hex[13];

	sprintf(mac_hex, "%s%s%s%s%s%s", a, b, c, d, e, f);

	for (int ii = 0; ii < 12; ++ii)
		if (!isxdigit(mac_hex[ii]))
			return 0;

	return 1;
}

/* parse and insert unique eth description.
 * caller is responsible for freeing whole list
 */
static void parse_eth_desc(FILE * fp, struct eth_desc *hd)
{
	char *l = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&l, &len, fp)) != -1) {
		if (l[0] == '\n' || l[0] == '#')
			continue;

		char *line = l;

		if (strchr(line, '\n'))
			strchr(line, '\n')[0] = '\0';
		char mac[18] = { 0 };
		strncpy(mac, line, 17);

		if (!check_mac_addr(mac)) {
			tui_error(ANYKEY_MSG, "Not a mac '%s' address, skipped",
				  mac);
			continue;
		}

		/* skip mac address */
		line += 17;

		/* mandatory space between mac and ip */
		if (!isspace(*line)) {
			tui_error(ANYKEY_MSG,
				  "Missing mandatory space between"
				  "mac and host/ip address, skipped");
			continue;
		}

		line = skip_whitespace(line);

		if (!*line) {
			tui_error(ANYKEY_MSG, "Missing description, skipped");
			continue;
		}

		struct eth_desc *new = xmalloc(sizeof(struct eth_desc));

		memcpy(new->hd_mac, mac, sizeof(mac));
		new->hd_desc = xstrdup(line);

		struct eth_desc *desc = NULL;

		list_for_each_entry(desc, &hd->hd_list, hd_list)
		    if ((strcmp(desc->hd_mac, mac) == 0)
			|| (strcmp(desc->hd_desc, line) == 0))
			goto dupe;

		list_add_tail(&new->hd_list, &hd->hd_list);
	      dupe:;
	}

	free(l);
}

struct eth_desc *load_eth_desc(unsigned link_type)
{
/* why is usefull to have it two files with same content?
 * There is two options how to merge it.
 * 1) separate by comments
 * $ cat ETHFILE
 *   # ethernet host description
 *   MAC ip/hostname
 *
 *   # fddi host description
 *   MAC ip/hostname
 * 2) put it into groups
 * [ethernet]
 *     MAC ip/hostname
 *
 * [fddi]
 *     MAC ip/hostname
 */
	char *filename = NULL;
	FILE *fp = NULL;

	if (link_type == ARPHRD_ETHER)
		filename = ETHFILE;
	else if (link_type == ARPHRD_FDDI)
		filename = FDDIFILE;

	struct eth_desc *hd = xmallocz(sizeof(struct eth_desc));

	INIT_LIST_HEAD(&hd->hd_list);

	fp = fopen(filename, "r");
	if (fp) {
		parse_eth_desc(fp, hd);
		fclose(fp);
	}

	/* merge with /etc/ethers */
	fp = fopen("/etc/ethers", "r");
	if (fp) {
		parse_eth_desc(fp, hd);
		fclose(fp);
	}

	return hd;
}

static void save_eth_desc(struct eth_desc *hd, unsigned linktype)
{
	FILE *fd = NULL;

	if (linktype == ARPHRD_ETHER)
		fd = fopen(ETHFILE, "w");
	else if (linktype == ARPHRD_FDDI)
		fd = fopen(FDDIFILE, "w");

	if (!fd) {
		tui_error(ANYKEY_MSG, "Unable to save host description file");
		return;
	}

	fprintf(fd, "# see man ethers for syntax\n\n");
	struct eth_desc *desc = NULL;

	list_for_each_entry(desc, &hd->hd_list, hd_list)
	    fprintf(fd, "%s %s\n", desc->hd_mac, desc->hd_desc);

	fclose(fd);
}


void free_eth_desc(struct eth_desc *hd)
{
	struct eth_desc *entry = NULL;
	struct list_head *l, *n;

	list_for_each_safe(l, n, &hd->hd_list) {
		entry = list_entry(l, struct eth_desc, hd_list);

		free(entry->hd_desc);
		list_del(l);
		free(entry);
	}
}

static struct eth_desc *select_eth_desc(const struct eth_desc *hd)
{

	struct scroll_list slist;
	char descline[80];

	if (list_empty(&hd->hd_list)) {
		tui_error(ANYKEY_MSG, "No descriptions");
		return NULL;
	}

	tx_init_listbox(&slist, COLS, 20, 0, (LINES - 20) / 2, STDATTR, BOXATTR,
			BARSTDATTR, HIGHATTR);

	tx_set_listbox_title(&slist, "Address", 1);
	tx_set_listbox_title(&slist, "Description", 19);

	struct eth_desc *entry = NULL;

	list_for_each_entry(entry, &hd->hd_list, hd_list) {
		snprintf(descline, 80, "%-18s%s", entry->hd_mac,
			 entry->hd_desc);
		tx_add_list_entry(&slist, (char *) entry, descline);
	}

	tx_show_listbox(&slist);

	int aborted = 0;

	tx_operate_listbox(&slist, &aborted);

	if (!aborted)
		entry = (struct eth_desc *) slist.textptr->nodeptr;
	else
		entry = NULL;

	tx_close_listbox(&slist);
	tx_destroy_list(&slist);

	update_panels();
	doupdate();

	return entry;
}

static int dialog_eth_desc(struct FIELDLIST *fields, const char *initaddr,
			   const char *initdesc)
{
	/* TODO: move to tui */
	WINDOW *win = newwin(8, 70, 8, (COLS - 70) / 2);
	PANEL *panel = new_panel(win);

	wattrset(win, DLGBOXATTR);
	tx_colorwin(win);
	tx_box(win, ACS_VLINE, ACS_HLINE);
	wmove(win, 6, 2 * COLS / 80);
	tabkeyhelp(win);
	wmove(win, 6, 20 * COLS / 80);
	stdkeyhelp(win);

	wattrset(win, DLGTEXTATTR);
	mvwprintw(win, 2, 2 * COLS / 80, "MAC Address:");
	mvwprintw(win, 4, 2 * COLS / 80, "Description:");

	tx_initfields(fields, 3, 52, 10, (COLS - 52) / 2 + 6 * COLS / 80,
		      DLGTEXTATTR, FIELDATTR);
	tx_addfield(fields, 17, 0, 0, initaddr);
	tx_addfield(fields, 50, 2, 0, initdesc);

	int aborted = 0;

	tx_fillfields(fields, &aborted);

	del_panel(panel);
	delwin(win);

	return aborted;
}

static void add_eth_desc(struct eth_desc *list)
{
	struct FIELDLIST fields;

	int aborted = dialog_eth_desc(&fields, "", "");

	if (!aborted) {
		struct eth_desc *new = xmalloc(sizeof(struct eth_desc));

		memcpy(new->hd_mac, fields.list->buf, sizeof(new->hd_mac));
		new->hd_desc = xstrdup(fields.list->nextfield->buf);

		list_add_tail(&new->hd_list, &list->hd_list);
	}

	tx_destroyfields(&fields);
	update_panels();
	doupdate();
}

static void edit_eth_desc(struct eth_desc *list)
{
	struct eth_desc *hd = select_eth_desc(list);

	if (!hd)
		return;

	struct FIELDLIST fields;
	int aborted = dialog_eth_desc(&fields, hd->hd_mac, hd->hd_desc);

	if (!aborted) {
		free(hd->hd_desc);
		memcpy(hd->hd_mac, fields.list->buf, sizeof(hd->hd_mac));
		hd->hd_desc = xstrdup(fields.list->nextfield->buf);
	}

	tx_destroyfields(&fields);
}

static void del_eth_desc(struct eth_desc *list)
{
	struct eth_desc *hd = select_eth_desc(list);

	if (hd) {
		free(hd->hd_desc);
		list_del(&hd->hd_list);
		free(hd);
	}
}

void manage_eth_desc(unsigned linktype)
{
	struct MENU menu;
	int row = 1;
	int aborted = 0;

	tx_initmenu(&menu, 7, 31, (LINES - 6) / 2, (COLS - 31) / 2, BOXATTR,
		    STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR, DESCATTR);
	tx_additem(&menu, " ^A^dd description...",
		   "Adds a description for a MAC address");
	tx_additem(&menu, " ^E^dit description...",
		   "Modifies an existing MAC address description");
	tx_additem(&menu, " ^D^elete description...",
		   "Deletes an existing MAC address description");
	tx_additem(&menu, NULL, NULL);
	tx_additem(&menu, " E^x^it menu", "Returns to the main menu");

	struct eth_desc *list =
	    load_eth_desc(linktype /*, WITHOUTETCETHERS */ );

	do {
		tx_showmenu(&menu);
		tx_operatemenu(&menu, &row, &aborted);

		switch (row) {
		case 1:
			add_eth_desc(list);
			break;
		case 2:
			edit_eth_desc(list);
			break;
		case 3:
			del_eth_desc(list);
			break;
		}
	} while (row != 5);

	tx_destroymenu(&menu);
	update_panels();
	doupdate();
	save_eth_desc(list, linktype);
}
