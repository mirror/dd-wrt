/***

menurt.c- ncurses-based menu definition module
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997, 1998

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <curses.h>
#include <panel.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "menurt.h"
#include "winops.h"
#include "labels.h"

/* initialize menu system */

void tx_initmenu(struct MENU *menu, int y1, int x1, int y2, int x2,
    int borderattr, int normalattr, int highattr,
    int barnormalattr, int barhighattr, int descattr)
{
    menu->itemlist = NULL;
    menu->itemcount = 0;
    strcpy(menu->shortcuts, "");
    menu->x1 = x1;
    menu->y1 = y1;
    menu->x2 = x2;
    menu->y2 = y2;
    menu->menuwin = newwin(y1, x1, y2, x2);
    menu->menupanel = new_panel(menu->menuwin);
    menu->menu_maxx = x1 - 2;

    keypad(menu->menuwin, 1);
    meta(menu->menuwin, 1);
    noecho();
    wtimeout(menu->menuwin, -1);	/* block until input */
    notimeout(menu->menuwin, 0);	/* disable Esc timer */
    nonl();
    cbreak();
    
    menu->borderattr = borderattr;
    menu->normalattr = normalattr;
    menu->highattr = highattr;
    menu->barnormalattr = barnormalattr;
    menu->barhighattr = barhighattr;
    menu->descriptionattr = descattr;
}

/* add menu item */

void tx_additem(struct MENU *menu, char *item, char *desc)
{
    struct ITEM *tnode;
    char cur_option[OPTIONSTRLEN_MAX];
    char thekey[2];

    if (menu->itemcount >= 25)
	return;

    tnode = malloc(sizeof(struct ITEM));

    if (item != NULL) {
	strcpy(tnode->option, item);
	strcpy(tnode->desc, desc);
	tnode->itemtype = REGULARITEM;

	strcpy(cur_option, item);
	strtok(cur_option, "^");
	strcpy(thekey, strtok(NULL, "^"));
	thekey[0] = toupper(thekey[0]);
	strcat(menu->shortcuts, thekey);
    } else {
	tnode->itemtype = SEPARATOR;
	strcat(menu->shortcuts, "^");	/* mark shortcut position for seps */
    }

    if (menu->itemlist == NULL) {
	menu->itemlist = tnode;
    } else {
	menu->lastitem->next = tnode;
	tnode->prev = menu->lastitem;
    }

    menu->itemlist->prev = tnode;
    menu->lastitem = tnode;
    tnode->next = menu->itemlist;
    menu->itemcount++;
}

/* show each individual item */

void tx_showitem(struct MENU *menu, struct ITEM *itemptr, int selected)
{
    int hiattr = 0;
    int loattr = 0;
    int ctr;
    char curoption[OPTIONSTRLEN_MAX];
    char padding[OPTIONSTRLEN_MAX];

    if (itemptr->itemtype == REGULARITEM) {
	switch (selected) {
	case NOTSELECTED:
	    hiattr = menu->highattr;
	    loattr = menu->normalattr;
	    break;
	case SELECTED:
	    hiattr = menu->barhighattr;
	    loattr = menu->barnormalattr;
	    break;
	}

	strcpy(curoption, itemptr->option);

	wattrset(menu->menuwin, loattr);
	wprintw(menu->menuwin, "%s", strtok(curoption, "^"));
	wattrset(menu->menuwin, hiattr);
	wprintw(menu->menuwin, "%s", strtok((char *) NULL, "^"));
	wattrset(menu->menuwin, loattr);
	wprintw(menu->menuwin, "%s", strtok((char *) NULL, "^"));

	strcpy(padding, "");

	for (ctr = strlen(itemptr->option); ctr <= menu->x1 - 1; ctr++)
	    strcat(padding, " ");

	wprintw(menu->menuwin, "%s", padding);
    } else {
	wattrset(menu->menuwin, menu->borderattr);
	whline(menu->menuwin, ACS_HLINE, menu->menu_maxx);
    }

    update_panels();
    doupdate();
}

/* repeatedly calls tx_showitem to display individual items */

void tx_showmenu(struct MENU *menu)
{
    struct ITEM *itemptr;	/* points to each item in turn */
    int ctr = 1;		/* counts each item */

    wattrset(menu->menuwin, menu->borderattr);	/* set to bg+/b */
    tx_colorwin(menu->menuwin);	/* color window */
    tx_box(menu->menuwin, ACS_VLINE, ACS_HLINE);	/* draw border */

    itemptr = menu->itemlist;	/* point to start */

    wattrset(menu->menuwin, menu->normalattr);

    do {			/* display items */
	wmove(menu->menuwin, ctr, 1);
	tx_showitem(menu, itemptr, NOTSELECTED);	/* show items, initially unselected */
	ctr++;
	itemptr = itemptr->next;
    } while (ctr <= menu->itemcount);

    update_panels();
    doupdate();
}

void menumoveto(struct MENU *menu, struct ITEM **itemptr, unsigned int row)
{
    struct ITEM *tnode;
    unsigned int i;

    tnode = menu->itemlist;
    for (i = 1; i < row; i++)
	tnode = tnode->next;

    *itemptr = tnode;
}

/* 
 * Actually do the menu operation after all the initialization
 */

void tx_operatemenu(struct MENU *menu, int *position, int *aborted)
{
    struct ITEM *itemptr;
    int row = *position;
    int exitloop = 0;
    int ch;
    char *keyptr;

    tx_menukeyhelp(menu->normalattr, menu->highattr);
    *aborted = 0;
    menumoveto(menu, &itemptr, row);

    menu->descwin = newwin(1, COLS, LINES - 2, 0);
    menu->descpanel = new_panel(menu->descwin);

    do {
	wmove(menu->menuwin, row, 1);
	tx_showitem(menu, itemptr, SELECTED);

	/*
	 * Print item description
	 */

	wattrset(menu->descwin, menu->descriptionattr);
	tx_colorwin(menu->descwin);
	wmove(menu->descwin, 0, 0);
	wprintw(menu->descwin, " %s", itemptr->desc);
	update_panels();
	doupdate();

	wmove(menu->menuwin, row, 2);
	ch = wgetch(menu->menuwin);
	wmove(menu->menuwin, row, 1);
	tx_showitem(menu, itemptr, NOTSELECTED);

	switch (ch) {
	case KEY_UP:
	    if (row == 1)
		row = menu->itemcount;
	    else
		row--;

	    itemptr = itemptr->prev;

	    if (itemptr->itemtype == SEPARATOR) {
		row--;
		itemptr = itemptr->prev;
	    }
	    break;
	case KEY_DOWN:
	    if (row == menu->itemcount)
		row = 1;
	    else
		row++;

	    itemptr = itemptr->next;
	    if (itemptr->itemtype == SEPARATOR) {
		row++;
		itemptr = itemptr->next;
	    }
	    break;
	case 12:
	    tx_refresh_screen();
	    break;
	case 13:
	    exitloop = 1;
	    break;
	    /* case 27: exitloop = 1;*aborted = 1;row=menu->itemcount;break; */
	case '^':
	    break;		/* ignore caret key */
	default:
	    keyptr = strchr(menu->shortcuts, toupper(ch));
	    if ((keyptr != NULL)
		&& keyptr - menu->shortcuts < menu->itemcount) {
		row = keyptr - menu->shortcuts + 1;
		exitloop = 1;
	    }
	}
    } while (!(exitloop));

    *position = row;		/* position of executed option is in *position */
    del_panel(menu->descpanel);
    delwin(menu->descwin);
    update_panels();
    doupdate();
}


void tx_destroymenu(struct MENU *menu)
{
    struct ITEM *tnode;
    struct ITEM *tnextnode;

    if (menu->itemlist != NULL) {
	tnode = menu->itemlist;
	tnextnode = menu->itemlist->next;

	tnode->prev->next = NULL;

	while (tnode != NULL) {
	    free(tnode);
	    tnode = tnextnode;

	    if (tnextnode != NULL)
		tnextnode = tnextnode->next;
	}
    }
    del_panel(menu->menupanel);
    delwin(menu->menuwin);
    update_panels();
    doupdate();
}
