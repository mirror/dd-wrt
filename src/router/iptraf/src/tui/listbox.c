/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/*
 * listbox.c - scrollable listbox management module
 */

#include "iptraf-ng-compat.h"

#include "winops.h"
#include "labels.h"
#include "listbox.h"
#include "msgboxes.h"

#define SCROLLUP 0
#define SCROLLDOWN 1

void tx_init_listbox(struct scroll_list *list, int width, int height,
		     int startx, int starty, int mainattr, int borderattr,
		     int selectattr, int keyattr)
{
	memset(list, 0, sizeof(struct scroll_list));
	list->borderwin = newwin(height, width, starty, startx);
	list->borderpanel = new_panel(list->borderwin);
	wattrset(list->borderwin, borderattr);
	tx_box(list->borderwin, ACS_VLINE, ACS_HLINE);

	list->win = newwin(height - 2, width - 2, starty + 1, startx + 1);
	list->panel = new_panel(list->win);
	wattrset(list->win, mainattr);
	tx_colorwin(list->win);

	list->mainattr = mainattr;
	list->selectattr = selectattr;
	list->height = height;
	list->width = width;
	list->keyattr = keyattr;
	list->row = 0;

	tx_stdwinset(list->win);
	scrollok(list->win, 0);
}

void tx_set_listbox_title(struct scroll_list *list, char *text, int x)
{
	mvwprintw(list->borderwin, 0, x, " %s ", text);
}

void tx_add_list_entry(struct scroll_list *list, char *node, char *text)
{
	struct textlisttype *ptmp;

	ptmp = xmallocz(sizeof(struct textlisttype));

	strncpy(ptmp->text, text, MAX_TEXT_LENGTH - 1);
	ptmp->nodeptr = node;

	if (list->textlist == NULL) {
		list->textlist = ptmp;
		ptmp->prev_entry = NULL;
	} else {
		list->texttail->next_entry = ptmp;
		ptmp->prev_entry = list->texttail;
	}

	list->texttail = ptmp;
	ptmp->next_entry = NULL;
}

void tx_show_listbox(struct scroll_list *list)
{
	int i = 0;
	struct textlisttype *tptr = list->textlist;

	while ((i <= list->height - 3) && (tptr != NULL)) {
		mvwprintw(list->win, i, 1, "%s", tptr->text);
		tptr = tptr->next_entry;
		i++;
	}

	update_panels();
	doupdate();
}

static void tx_print_row(struct scroll_list *list, int attr)
{
	wattrset(list->win, attr);
	mvwprintw(list->win, list->row, 0, " %-*s", list->width - 3,
		  list->textptr->text);

}

static void tx_scroll_listbox(struct scroll_list *list, int direction,
			      int lines)
{
	if (lines < 1)
		return;

	if (direction == SCROLLUP) {
		for (int i = 0; i < lines; i++) {
			if (list->textptr->next_entry == NULL)
				break;

			tx_print_row(list, list->mainattr);
			if (list->row == list->height - 3) {
				scrollok(list->win, 1);
				wscrl(list->win, 1);
				scrollok(list->win, 0);
			} else
				list->row++;
			list->textptr = list->textptr->next_entry;
			tx_print_row(list, list->selectattr);
		}
	} else {
		for (int i = 0; i < lines; i++) {
			if (list->textptr->prev_entry == NULL)
				break;

			tx_print_row(list, list->mainattr);
			if (list->row == 0) {
				scrollok(list->win, 1);
				wscrl(list->win, -1);
				scrollok(list->win, 0);
			} else
				list->row--;
			list->textptr = list->textptr->prev_entry;
			tx_print_row(list, list->selectattr);
		}
	}
}

void tx_operate_listbox(struct scroll_list *list, int *aborted)
{
	int endloop = 0;

	if (list->textlist == NULL) {
		tui_error(ANYKEY_MSG, "No list entries");
		*aborted = 1;
		return;
	}

	list->textptr = list->textlist;

	tx_listkeyhelp(list->mainattr, list->keyattr);
	update_panels();
	doupdate();

	tx_print_row(list, list->selectattr);
	while (!endloop) {
		switch (wgetch(list->win)) {
		case KEY_UP:
			tx_scroll_listbox(list, SCROLLDOWN, 1);
			break;
		case KEY_DOWN:
			tx_scroll_listbox(list, SCROLLUP, 1);
			break;
		case KEY_PPAGE:
			tx_scroll_listbox(list, SCROLLDOWN, list->height - 2);
			break;
		case KEY_NPAGE:
			tx_scroll_listbox(list, SCROLLUP, list->height - 2);
			break;
		case KEY_HOME:
			tx_scroll_listbox(list, SCROLLDOWN, INT_MAX);
			break;
		case KEY_END:
			tx_scroll_listbox(list, SCROLLUP, INT_MAX);
			break;
		case 13:
			*aborted = 0;
			endloop = 1;
			break;
		case 27:
		case 'x':
		case 'X':
		case 24:
			*aborted = 1;
			endloop = 1;
			/* fall through */
		case 12:
		case 'l':
		case 'L':
			tx_refresh_screen();
			break;
		}
	}
}

void tx_hide_listbox(struct scroll_list *list)
{
	hide_panel(list->panel);
	hide_panel(list->borderpanel);
	update_panels();
	doupdate();
}

void tx_unhide_listbox(struct scroll_list *list)
{
	show_panel(list->panel);
	show_panel(list->panel);
	update_panels();
	doupdate();
}

void tx_close_listbox(struct scroll_list *list)
{
	del_panel(list->panel);
	del_panel(list->borderpanel);
	delwin(list->win);
	delwin(list->borderwin);

	update_panels();
	doupdate();
}

void tx_destroy_list(struct scroll_list *list)
{
	struct textlisttype *ttmp = list->textlist;
	struct textlisttype *ctmp;

	while (ttmp != NULL) {
		ctmp = ttmp->next_entry;
		free(ttmp);
		ttmp = ctmp;
	}
}
