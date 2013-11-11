/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

input.c - a custom keyboard input module

***/

#include "iptraf-ng-compat.h"

#include "input.h"

void tx_initfields(struct FIELDLIST *list, int leny, int lenx, int begy,
		   int begx, int dlgtextattr, int fieldattr)
{
	list->list = NULL;
	list->fieldwin = newwin(leny, lenx, begy, begx);
	list->fieldpanel = new_panel(list->fieldwin);
	tx_stdwinset(list->fieldwin);
	wtimeout(list->fieldwin, -1);
	wattrset(list->fieldwin, dlgtextattr);
	tx_colorwin(list->fieldwin);
	update_panels();
	doupdate();

	list->dlgtextattr = dlgtextattr;
	list->fieldattr = fieldattr;
}

void tx_addfield(struct FIELDLIST *list, unsigned int len, unsigned int y,
		 unsigned int x, const char *initstr)
{
	struct FIELD *newfield;
	unsigned int i;

	newfield = xmalloc(sizeof(struct FIELD));

	if (list->list == NULL) {
		list->list = newfield;
		newfield->prevfield = newfield;
		newfield->nextfield = newfield;
	} else {
		newfield->prevfield = list->list->prevfield;
		list->list->prevfield->nextfield = newfield;
		list->list->prevfield = newfield;
		newfield->nextfield = list->list;
	}

	newfield->xpos = x;
	newfield->ypos = y;
	newfield->len = len;
	newfield->tlen = strlen(initstr);
	newfield->buf = xmallocz(len + 1);
	strncpy(newfield->buf, initstr, len);

	if (newfield->tlen > (len))
		newfield->tlen = len;

	wattrset(list->fieldwin, list->fieldattr);
	wmove(list->fieldwin, y, x);
	for (i = 1; i <= len; i++)
		wprintw(list->fieldwin, " ");

	wmove(list->fieldwin, y, x);
	wprintw(list->fieldwin, "%s", newfield->buf);

	update_panels();
	doupdate();
}

void tx_getinput(struct FIELDLIST *list, struct FIELD *field, int *exitkey)
{
	int ch;
	int y, x;
	int endloop = 0;

	wmove(list->fieldwin, field->ypos, field->xpos);
	wattrset(list->fieldwin, list->fieldattr);
	wprintw(list->fieldwin, "%s", field->buf);
	update_panels();
	doupdate();

	do {
		ch = wgetch(list->fieldwin);
		switch (ch) {
		case KEY_BACKSPACE:
		case 7:
		case 8:
		case KEY_DC:
		case KEY_LEFT:
		case 127:
			if (field->tlen > 0) {
				getyx(list->fieldwin, y, x);
				x--;
				wmove(list->fieldwin, y, x);
				wprintw(list->fieldwin, " ");
				wmove(list->fieldwin, y, x);
				field->tlen--;
				field->buf[field->tlen] = '\0';
			}
			break;
		case 9:
		case 27:
		case 24:
		case 13:
		case 10:
		case KEY_UP:
		case KEY_DOWN:
			endloop = 1;
			*exitkey = ch;

			break;
		case 12:
			tx_refresh_screen();
			break;
		default:
			if ((field->tlen < field->len)
			    && ((ch >= 32) && (ch <= 127))) {
				wprintw(list->fieldwin, "%c", ch);
				if (ch == ' ') {
					getyx(list->fieldwin, y, x);
					wmove(list->fieldwin, y, x);
				}
				field->buf[field->tlen + 1] = '\0';
				field->buf[field->tlen] = ch;
				field->tlen++;
			}
			break;
		}

		doupdate();
	} while (!endloop);
}

void tx_fillfields(struct FIELDLIST *list, int *aborted)
{
	struct FIELD *field;
	int exitkey;
	int endloop = 0;

	field = list->list;

	curs_set(1);
	do {
		tx_getinput(list, field, &exitkey);

		switch (exitkey) {
		case 9:
		case KEY_DOWN:
			field = field->nextfield;
			break;
		case KEY_UP:
			field = field->prevfield;
			break;
		case 13:
		case 10:
			*aborted = 0;
			endloop = 1;
			break;
		case 27:
		case 24:
			*aborted = 1;
			endloop = 1;
			break;
		}
	} while (!endloop);

	curs_set(0);
}

void tx_destroyfields(struct FIELDLIST *list)
{
	struct FIELD *ptmp;
	struct FIELD *pnext;

	list->list->prevfield->nextfield = NULL;
	ptmp = list->list;
	pnext = list->list->nextfield;

	do {
		free(ptmp);

		ptmp = pnext;
		if (pnext != NULL) {
			pnext = pnext->nextfield;
		}
	} while (ptmp != NULL);

	del_panel(list->fieldpanel);
	delwin(list->fieldwin);
}
