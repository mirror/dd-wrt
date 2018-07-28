/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

fltedit.c	- the filter editing Facility

***/

#include "iptraf-ng-compat.h"

#include "tui/labels.h"
#include "tui/menurt.h"
#include "tui/msgboxes.h"
#include "tui/winops.h"

#include "fltdefs.h"
#include "fltmgr.h"
#include "ipfilter.h"
#include "dirs.h"
#include "getpath.h"
#include "attrs.h"
#include "deskman.h"
#include "error.h"
#include "cidr.h"

void init_filter_table(struct filterlist *fl)
{
	fl->head = fl->tail = NULL;
}

/*
 * Loads the filter from the filter file
 */

int loadfilter(char *filename, struct filterlist *fl, int resolve)
{
	struct filterent *fe;
	int pfd;
	unsigned int idx = 0;
	int br;
	int resolv_err = 0;

	init_filter_table(fl);

	pfd = open(filename, O_RDONLY);

	if (pfd < 0) {
		write_error("Error opening IP filter data file");
		fl->head = NULL;
		return 1;
	}
	do {
		fe = xmalloc(sizeof(struct filterent));
		br = read(pfd, &(fe->hp), sizeof(struct hostparams));

		if (br > 0) {
			fe->index = idx;
			if (resolve) {
				fe->saddr =
				    nametoaddr(fe->hp.s_fqdn, &resolv_err);
				fe->daddr =
				    nametoaddr(fe->hp.d_fqdn, &resolv_err);

				if (resolv_err) {
					free(fe);
					continue;
				}

				fe->smask = inet_addr(fe->hp.s_mask);
				fe->dmask = inet_addr(fe->hp.d_mask);
			}
			if (fl->head == NULL) {
				fl->head = fe;
				fe->prev_entry = NULL;
			} else {
				fl->tail->next_entry = fe;
				fe->prev_entry = fl->tail;
			}
			fe->next_entry = NULL;
			fl->tail = fe;
			idx++;
		} else {
			free(fe);
		}
	} while (br > 0);

	if (br == 0)
		close(pfd);

	return 0;
}

void savefilter(char *filename, struct filterlist *fl)
{
	struct filterent *fe = fl->head;
	int pfd;
	int bw;

	pfd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

	while (fe != NULL) {
		bw = write(pfd, &(fe->hp), sizeof(struct hostparams));

		if (bw < 0) {
			tui_error(ANYKEY_MSG, "Unable to save filter changes");
			return;
		}
		fe = fe->next_entry;
	}

	close(pfd);
}

void print_hostparam_line(struct filterent *fe, int idx, WINDOW * win, int attr)
{
	struct in_addr binmask;

	wattrset(win, attr);

	scrollok(win, 0);
	mvwprintw(win, idx, 0, "%78c", ' ');

	mvwaddnstr(win, idx, 1, fe->hp.s_fqdn, 20);
	if (inet_aton(fe->hp.s_mask, &binmask) == 0)
		inet_aton("255.255.255.255", &binmask);

	wprintw(win, "/%u", cidr_get_maskbits(binmask.s_addr));
	if (fe->hp.sport2 == 0)
		wprintw(win, ":%u", fe->hp.sport1);
	else
		wprintw(win, ":%u-%u", fe->hp.sport1, fe->hp.sport2);

	wmove(win, idx, 34);
	if (fe->hp.match_opposite != 'Y')
		wprintw(win, "-->");
	else
		wprintw(win, "<->");

	mvwaddnstr(win, idx, 38, fe->hp.d_fqdn, 15);

	if (inet_aton(fe->hp.d_mask, &binmask) == 0)
		inet_aton("255.255.255.255", &binmask);

	wprintw(win, "/%u", cidr_get_maskbits(binmask.s_addr));

	if (fe->hp.dport2 == 0)
		wprintw(win, ":%u", fe->hp.dport1);
	else
		wprintw(win, ":%u-%u", fe->hp.dport1, fe->hp.dport2);

	mvwprintw(win, idx, 76, "%c", toupper(fe->hp.reverse));
	wmove(win, idx, 0);
}

void update_hp_screen(struct filterent *firstvisible, WINDOW * win)
{
	struct filterent *ftmp = firstvisible;
	int i;

	wattrset(win, STDATTR);
	if (firstvisible == NULL) {
		mvwprintw(win, 0, 0, "%78c", ' ');
		wmove(win, 0, 0);
		return;
	}

	scrollok(win, 0);
	for (i = 0; i <= 12; i++) {
		if (ftmp != NULL) {
			print_hostparam_line(ftmp, i, win, STDATTR);
			ftmp = ftmp->next_entry;
		} else {
			mvwprintw(win, i, 0, "%78c", ' ');
			wmove(win, i, 0);
		}
	}
	scrollok(win, 1);
}

void modify_host_parameters(struct filterlist *fl)
{
	WINDOW *bwin;
	PANEL *bpanel;
	WINDOW *win;
	PANEL *panel;
	struct filterent *fe;
	struct filterent *ftemp;

	struct filterent *firstvisible = NULL;

	unsigned int idx = 0;
	int endloop_local = 0;
	int ch;
	int gh_aborted = 0;

	char s_portstr1[8];
	char d_portstr1[8];
	char s_portstr2[8];
	char d_portstr2[8];

	char inexstr[2];
	char matchop[2];

	bwin = newwin(15, 80, (LINES - 15) / 2, (COLS - 80) / 2);

	bpanel = new_panel(bwin);
	win = newwin(13, 78, (LINES - 13) / 2, (COLS - 78) / 2);
	panel = new_panel(win);

	wattrset(bwin, BOXATTR);
	tx_box(bwin, ACS_VLINE, ACS_HLINE);

	mvwprintw(bwin, 0, 2, " Source ");
	mvwprintw(bwin, 0, 38, " Destination ");
	mvwprintw(bwin, 0, 74, " I/E ");

	mvwprintw(bwin, 14, 1, " Filter Data ");
	tx_stdwinset(win);
	scrollok(win, 0);
	wattrset(win, STDATTR);
	tx_colorwin(win);

	move(LINES - 1, 1);
	tx_printkeyhelp("Up/Down", "-move ptr ", stdscr, HIGHATTR,
			STATUSBARATTR);
	tx_printkeyhelp("I", "-insert ", stdscr, HIGHATTR, STATUSBARATTR);
	tx_printkeyhelp("A", "-add to list ", stdscr, HIGHATTR, STATUSBARATTR);
	tx_printkeyhelp("D", "-delete ", stdscr, HIGHATTR, STATUSBARATTR);
	tx_printkeyhelp("Enter", "-edit ", stdscr, HIGHATTR, STATUSBARATTR);
	tx_printkeyhelp("X/Ctrl+X", "-exit", stdscr, HIGHATTR, STATUSBARATTR);

	update_panels();
	doupdate();

	firstvisible = fl->head;

	update_hp_screen(firstvisible, win);

	idx = 0;
	fe = firstvisible;

	update_panels();
	doupdate();

	do {
		if (fe != NULL) {
			print_hostparam_line(fe, idx, win, BARSTDATTR);
		}

		ch = wgetch(win);

		if (fe != NULL)
			print_hostparam_line(fe, idx, win, STDATTR);

		switch (ch) {
		case KEY_UP:
			if (fl->head != NULL) {
				if (fe->prev_entry != NULL) {
					if (idx > 0)
						idx--;
					else {
						scrollok(win, 1);
						wscrl(win, -1);
						firstvisible =
						    firstvisible->prev_entry;
					}
					fe = fe->prev_entry;
				}
			}
			break;
		case KEY_DOWN:
			if (fl->head != NULL) {
				if (fe->next_entry != NULL) {
					if (idx < 12)
						idx++;
					else {
						scrollok(win, 1);
						wscrl(win, 1);
						firstvisible =
						    firstvisible->next_entry;
					}
					fe = fe->next_entry;
				}
			}
			break;
		case 'i':
		case 'I':
		case KEY_IC:
			ftemp = xmallocz(sizeof(struct filterent));

			gethostparams(&(ftemp->hp), "", "", "", "", "", "", "",
				      "", "I", "N", &gh_aborted);

			if (gh_aborted) {
				free(ftemp);
				continue;
			}

			if (fl->head == NULL) {
				ftemp->next_entry = ftemp->prev_entry = NULL;
				fl->head = fl->tail = ftemp;
				firstvisible = fl->head;
				idx = 0;
			} else {
				ftemp->next_entry = fe;
				ftemp->prev_entry = fe->prev_entry;

				/*
				 * Point firstvisible at new entry if we inserted at the
				 * top of the list.
				 */

				if (ftemp->prev_entry == NULL) {
					fl->head = ftemp;
					firstvisible = ftemp;
				} else
					fe->prev_entry->next_entry = ftemp;

				fe->prev_entry = ftemp;
			}

			if (ftemp->next_entry == NULL)
				fl->tail = ftemp;

			fe = ftemp;
			update_hp_screen(firstvisible, win);
			break;
		case 'a':
		case 'A':
		case 1:
			ftemp = xmallocz(sizeof(struct filterent));

			gethostparams(&(ftemp->hp), "", "", "", "", "", "", "",
				      "", "I", "N", &gh_aborted);

			if (gh_aborted) {
				free(ftemp);
				continue;
			}

			/*
			 * Add new node to the end of the list (or to the head if the
			 * list is empty.
			 */
			if (fl->tail != NULL) {
				fl->tail->next_entry = ftemp;
				ftemp->prev_entry = fl->tail;
			} else {
				fl->head = ftemp;
				fl->tail = ftemp;
				ftemp->prev_entry = ftemp->next_entry = NULL;
				firstvisible = fl->head;
				fe = ftemp;
				idx = 0;
			}

			ftemp->next_entry = NULL;
			fl->tail = ftemp;
			update_hp_screen(firstvisible, win);
			break;
		case 'd':
		case 'D':
		case KEY_DC:
			if (fl->head != NULL) {
				/*
				 * Move firstvisible down if it's pointing to the target
				 * entry.
				 */

				if (firstvisible == fe)
					firstvisible = fe->next_entry;

				/*
				 * Detach target node from list.
				 */
				if (fe->next_entry != NULL)
					fe->next_entry->prev_entry =
					    fe->prev_entry;
				else
					fl->tail = fe->prev_entry;

				if (fe->prev_entry != NULL)
					fe->prev_entry->next_entry =
					    fe->next_entry;
				else
					fl->head = fe->next_entry;

				/*
				 * Move pointer up if we're deleting the last entry.
				 * The list tail pointer has since been moved to the
				 * previous entry.
				 */
				if (fe->prev_entry == fl->tail) {
					ftemp = fe->prev_entry;

					/*
					 * Move screen pointer up. Really adjust the index if
					 * the pointer is anywhere below the top of the screen.
					 */
					if (idx > 0)
						idx--;
					else {
						/*
						 * Otherwise scroll the list down, and adjust the
						 * firstvisible pointer to point to the entry
						 * previous to the target.
						 */
						if (ftemp != NULL) {
							firstvisible = ftemp;
						}
					}
				} else
					/*
					 * If we reach this point, we're deleting from before
					 * the tail of the list.  In that case, we point the
					 * screen pointer at the entry following the target.
					 */
					ftemp = fe->next_entry;

				free(fe);
				fe = ftemp;
				update_hp_screen(firstvisible, win);
			}
			break;
		case 13:
			if (fe != NULL) {
				sprintf(s_portstr1, "%u", fe->hp.sport1);
				sprintf(s_portstr2, "%u", fe->hp.sport2);
				sprintf(d_portstr1, "%u", fe->hp.dport1);
				sprintf(d_portstr2, "%u", fe->hp.dport2);
				inexstr[0] = toupper(fe->hp.reverse);
				inexstr[1] = '\0';
				matchop[0] = toupper(fe->hp.match_opposite);
				matchop[1] = '\0';

				gethostparams(&(fe->hp), fe->hp.s_fqdn,
					      fe->hp.s_mask, s_portstr1,
					      s_portstr2, fe->hp.d_fqdn,
					      fe->hp.d_mask, d_portstr1,
					      d_portstr2, inexstr, matchop,
					      &gh_aborted);

				update_hp_screen(firstvisible, win);
			}

			break;
		case 'x':
		case 'X':
		case 'q':
		case 'Q':
		case 27:
		case 24:
			endloop_local = 1;
			break;
		case 'l':
		case 'L':
			tx_refresh_screen();
			break;
		}
		update_panels();
		doupdate();
	} while (!endloop_local);

	del_panel(panel);
	delwin(win);
	del_panel(bpanel);
	delwin(bwin);
	update_panels();
	doupdate();
}

/* remove a currently applied filter from memory */
void destroyfilter(struct filterlist *fl)
{
	struct filterent *fe = fl->head;

	while (fe != NULL) {
		struct filterent *cfe = fe->next_entry;

		free(fe);
		fe = cfe;
	}

	fl->head = NULL;
	fl->tail = NULL;
}


void definefilter(int *aborted)
{
	struct filterfileent ffile;
	char fntemp[14];
	struct filterlist fl;

	int pfd;
	int bw;

	get_filter_description(ffile.desc, aborted, "");

	if (*aborted)
		return;

	genname(time(NULL), fntemp);

	pfd =
	    open(get_path(T_WORKDIR, fntemp), O_CREAT | O_WRONLY | O_TRUNC,
		 S_IRUSR | S_IWUSR);
	if (pfd < 0) {
		tui_error(ANYKEY_MSG, "Cannot create filter data file");
		*aborted = 1;
		return;
	}

	close(pfd);

	pfd =
	    open(OTHIPFLNAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);

	if (pfd < 0) {
		listfileerr(1);
		return;
	}
	strcpy(ffile.filename, fntemp);
	bw = write(pfd, &ffile, sizeof(struct filterfileent));
	if (bw < 0)
		listfileerr(2);

	close(pfd);

	init_filter_table(&fl);
	modify_host_parameters(&fl);
	savefilter(get_path(T_WORKDIR, fntemp), &fl);
	destroyfilter(&fl);
}

/*
 * Edit an existing filter
 */
void editfilter(int *aborted)
{
	char filename[FLT_FILENAME_MAX];
	struct filterlist fl;
	struct ffnode *flist;
	struct ffnode *ffile;
	struct filterfileent *ffe;

	if (loadfilterlist(&flist) == 1) {
		listfileerr(1);
		destroyfilterlist(flist);
		return;
	}
	pickafilter(flist, &ffile, aborted);

	if ((*aborted)) {
		destroyfilterlist(flist);
		return;
	}
	ffe = &(ffile->ffe);

	get_filter_description(ffe->desc, aborted, ffe->desc);

	if (*aborted) {
		destroyfilterlist(flist);
		return;
	}
	strncpy(filename, get_path(T_WORKDIR, ffe->filename),
		FLT_FILENAME_MAX - 1);

	if (loadfilter(filename, &fl, FLT_DONTRESOLVE))
		return;

	modify_host_parameters(&fl);

	save_filterlist(flist);	/* This also destroys it */
	savefilter(filename, &fl);
	destroyfilter(&fl);
}

/*
 * Delete a filter record from the disk
 */

void delfilter(int *aborted)
{
	struct ffnode *fltfile;
	struct ffnode *fltlist;

	if (loadfilterlist(&fltlist) == 1) {
		*aborted = 1;
		listfileerr(1);
		destroyfilterlist(fltlist);
		return;
	}
	pickafilter(fltlist, &fltfile, aborted);

	if (*aborted)
		return;

	unlink(get_path(T_WORKDIR, fltfile->ffe.filename));

	if (fltfile->prev_entry == NULL) {
		fltlist = fltlist->next_entry;
		if (fltlist != NULL)
			fltlist->prev_entry = NULL;
	} else {
		fltfile->prev_entry->next_entry = fltfile->next_entry;

		if (fltfile->next_entry != NULL)
			fltfile->next_entry->prev_entry = fltfile->prev_entry;
	}

	free(fltfile);

	save_filterlist(fltlist);
	*aborted = 0;
}
