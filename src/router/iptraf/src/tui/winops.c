/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

winops.c - screen configuration and setup functions

***/

#include "iptraf-ng-compat.h"

void tx_stdwinset(WINDOW * win)
{
	meta(win, TRUE);
	keypad(win, TRUE);
	notimeout(win, 0);
	scrollok(win, 1);
}

void tx_refresh_screen(void)
{
	endwin();
	doupdate();
	curs_set(0);
}

void tx_colorwin(WINDOW *win)
{
	scrollok(win, 0);
	for (int row = 0; row < getmaxy(win); row++)
		mvwprintw(win, row, 0, "%*c", getmaxx(win), ' ');
	scrollok(win, 1);
}

void tx_wcoloreol(WINDOW *win)
{
	scrollok(win, 0);
	wprintw(win, "%*c", getmaxx(win) - getcurx(win) - 1, ' ');
}

/*
 * This function is written to address a strange symptom in ncurses 5.2, at
 * least on RedHat 7.3.  The border drawn by the box() macro (actually an alias
 * for a call to wborder()) no longer uses the color attributes set by
 * wattrset(). However, the addch() and wvline() functions still do.
 *
 * The tx_box function is a drop-in replacement for box().
 */
void tx_box(WINDOW *win, int vline, int hline)
{
	int winwidth;
	int winheight;
	int i;

	scrollok(win, 0);
	getmaxyx(win, winheight, winwidth);
	winheight--;
	winwidth--;

	mvwaddch(win, 0, 0, ACS_ULCORNER);
	mvwhline(win, 0, 1, hline, winwidth - 1);
	mvwaddch(win, 0, winwidth, ACS_URCORNER);

	for (i = 1; i < winheight; i++) {
		mvwaddch(win, i, 0, vline);
		mvwaddch(win, i, winwidth, vline);
	}

	mvwaddch(win, winheight, 0, ACS_LLCORNER);
	mvwhline(win, winheight, 1, hline, winwidth - 1);
	mvwaddch(win, winheight, winwidth, ACS_LRCORNER);
}
