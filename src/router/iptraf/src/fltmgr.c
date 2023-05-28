/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

fltmgr.c - filter list management routines

***/

#include "iptraf-ng-compat.h"

#include "tui/input.h"
#include "tui/labels.h"
#include "tui/listbox.h"
#include "tui/menurt.h"
#include "tui/msgboxes.h"
#include "tui/winops.h"

#include "attrs.h"
#include "deskman.h"
#include "dirs.h"
#include "fltdefs.h"
#include "fltmgr.h"
#include "error.h"

void makestdfiltermenu(struct MENU *menu)
{
	tx_initmenu(menu, 9, 31, (LINES - 8) / 2, (COLS - 31) / 2 + 15, BOXATTR,
		    STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR, DESCATTR);
	tx_additem(menu, " ^D^efine new filter...",
		   "Defines a new set of IP filter parameters");
	tx_additem(menu, " ^A^pply filter...", "Applies a defined filter");
	tx_additem(menu, " Detac^h^ filter",
		   "Removes the currently applied filter");
	tx_additem(menu, " ^E^dit filter...", "Modifies existing filter data");
	tx_additem(menu, " Dele^t^e filter...",
		   "Removes an IP filter from the filter list");
	tx_additem(menu, NULL, NULL);
	tx_additem(menu, " E^x^it menu", "Returns to the main menu");
}


/*
 * Generate a string representation of a number to be used as a name.
 */

void genname(unsigned long n, char *m)
{
	sprintf(m, "%lu", n);
}

void listfileerr(int code)
{
	if (code == 1)
		write_error("Error loading filter list file");
	else
		write_error("Error writing filter list file");
}

unsigned long int nametoaddr(char *ascname, int *err)
{
	unsigned long int result;
	struct hostent *he;
	char imsg[45];
	struct in_addr inp;
	int resolv_err = 0;

	resolv_err = inet_aton(ascname, &inp);
	if (resolv_err == 0) {
		snprintf(imsg, 44, "Resolving %s", ascname);
		indicate(imsg);

		he = gethostbyname(ascname);
		if (he != NULL)
			bcopy((he->h_addr_list)[0], &result, he->h_length);
		else {
			write_error("Unable to resolve %s", ascname);
			*err = 1;
			return (-1);
		}
	} else
		result = inp.s_addr;

	return (result);
	*err = 0;
}

int loadfilterlist(struct ffnode **fltfile)
{
	int pfd = 0;
	int result = 0;

	struct ffnode *ffiles = NULL;
	struct ffnode *ptemp;
	struct ffnode *tail = NULL;
	struct ffnode *insert_point = NULL;	/* new node is inserted *above* this */

	int br;

	pfd = open(OTHIPFLNAME, O_RDONLY);

	if (pfd < 0) {
		*fltfile = NULL;
		return 1;
	}

	do {
		ptemp = xmalloc(sizeof(struct ffnode));
		br = read(pfd, &(ptemp->ffe), sizeof(struct filterfileent));

		if (br > 0) {
			if (ffiles == NULL) {
				/*
				 * Create single-node list should initial list pointer be empty
				 */
				ffiles = ptemp;
				ffiles->prev_entry = ffiles->next_entry = NULL;
				tail = ffiles;
			} else {
				/*
				 * Find appropriate point for insertion into sorted list.
				 */

				insert_point = ffiles;
				while (insert_point != NULL) {
					if (strcasecmp
					    (insert_point->ffe.desc,
					     ptemp->ffe.desc)
					    < 0)
						insert_point =
						    insert_point->next_entry;
					else
						break;
				}

				/*
				 * Insert new node depending on whether insert_point = top of list;
				 * middle of list; end of list.
				 */

				if (insert_point == NULL) {
					/* Case 1: end of list; if insert_point is NULL, we get it
					   out of the way first */
					tail->next_entry = ptemp;
					ptemp->prev_entry = tail;
					tail = ptemp;
					ptemp->next_entry = NULL;
				} else if (insert_point->prev_entry == NULL) {
					/* Case 2: top of list */
					insert_point->prev_entry = ptemp;
					ffiles = ptemp;
					ffiles->prev_entry = NULL;
					ffiles->next_entry = insert_point;
					insert_point->prev_entry = ffiles;
				} else {
					/* Case 3: middle of list */
					ptemp->prev_entry =
					    insert_point->prev_entry;
					ptemp->next_entry = insert_point;
					insert_point->prev_entry->next_entry =
					    ptemp;
					insert_point->prev_entry = ptemp;
				}
			}
		} else {
			free(ptemp);

			if (br < 0)
				result = 1;
		}
	} while (br > 0);

	close(pfd);
	*fltfile = ffiles;

	if (ffiles == NULL)
		result = 1;

	return result;
}

void destroyfilterlist(struct ffnode *fltlist)
{
	while (fltlist != NULL) {
		struct ffnode *fftemp = fltlist->next_entry;

		free(fltlist);
		fltlist = fftemp;
	}
}

void save_filterlist(struct ffnode *fltlist)
{
	struct ffnode *fltfile;
	struct ffnode *ffntemp;
	int fd;
	int bw;

	fd = open(OTHIPFLNAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if (fd < 0) {
		listfileerr(2);
		return;
	}

	fltfile = fltlist;
	while (fltfile != NULL) {
		bw = write(fd, &(fltfile->ffe), sizeof(struct filterfileent));

		if (bw < 0) {
			listfileerr(2);
			return;
		}
		ffntemp = fltfile;
		fltfile = fltfile->next_entry;
		free(ffntemp);
	}

	close(fd);
}

void operate_select(struct ffnode *ffiles, struct ffnode **item, int *aborted)
{
	struct ffnode *pptr;
	struct scroll_list list;

	tx_listkeyhelp(STDATTR, HIGHATTR);
	update_panels();
	doupdate();

	pptr = ffiles;

	tx_init_listbox(&list, 60, 10, (COLS - 60) / 2 - 2,
			(LINES - 10) / 2 - 2, STDATTR, BOXATTR, BARSTDATTR,
			HIGHATTR);

	tx_set_listbox_title(&list, "Select Filter", 1);

	while (pptr != NULL) {
		tx_add_list_entry(&list, (char *) pptr, pptr->ffe.desc);
		pptr = pptr->next_entry;
	}

	tx_show_listbox(&list);
	tx_operate_listbox(&list, aborted);

	if (!(*aborted))
		*item = (struct ffnode *) list.textptr->nodeptr;

	tx_close_listbox(&list);
	tx_destroy_list(&list);
}

void pickafilter(struct ffnode *ffiles, struct ffnode **fltfile, int *aborted)
{
	operate_select(ffiles, fltfile, aborted);

	update_panels();
	doupdate();
}

void selectfilter(struct filterfileent *ffe, int *aborted)
{
	struct ffnode *fltfile;
	struct ffnode *ffiles;

	if (loadfilterlist(&ffiles)) {
		listfileerr(1);
		*aborted = 1;
		destroyfilterlist(ffiles);
		return;
	}
	pickafilter(ffiles, &fltfile, aborted);

	if (!(*aborted))
		*ffe = fltfile->ffe;

	destroyfilterlist(ffiles);
}


void get_filter_description(char *description, int *aborted, char *pre_edit)
{
	struct FIELDLIST descfield;
	int dlgwintop;
	WINDOW *dlgwin;
	PANEL *dlgpanel;

	dlgwintop = (LINES - 9) / 2;
	dlgwin = newwin(7, 42, dlgwintop, (COLS - 42) / 2 - 10);
	dlgpanel = new_panel(dlgwin);
	wattrset(dlgwin, DLGBOXATTR);
	tx_colorwin(dlgwin);
	tx_box(dlgwin, ACS_VLINE, ACS_HLINE);
	wattrset(dlgwin, DLGTEXTATTR);
	wmove(dlgwin, 2, 2);
	wprintw(dlgwin, "Enter a description for this filter");
	wmove(dlgwin, 5, 2);
	stdkeyhelp(dlgwin);
	update_panels();
	doupdate();

	tx_initfields(&descfield, 1, 35, dlgwintop + 3, (COLS - 42) / 2 - 8,
		      DLGTEXTATTR, FIELDATTR);
	tx_addfield(&descfield, 33, 0, 0, pre_edit);

	do {
		tx_fillfields(&descfield, aborted);

		if ((descfield.list->buf[0] == '\0') && (!(*aborted)))
			tui_error(ANYKEY_MSG,
				  "Enter an appropriate description for this filter");

	} while ((descfield.list->buf[0] == '\0') && (!(*aborted)));

	if (!(*aborted))
		strcpy(description, descfield.list->buf);

	tx_destroyfields(&descfield);
	del_panel(dlgpanel);
	delwin(dlgwin);
	update_panels();
	doupdate();
}
