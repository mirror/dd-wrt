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

void tx_colorwin(WINDOW * win)
{
	int ctr;
	char *blankpad;
	blankpad = (char *) xmalloc(sizeof(char) * (getmaxx(win) + 1));

	strcpy(blankpad, "");

	for (ctr = 0; ctr < getmaxx(win); ctr++) {
		strcat(blankpad, " ");
	}

	scrollok(win, 0);
	for (ctr = 0; ctr < getmaxy(win); ctr++) {
		wmove(win, ctr, 0);
		wprintw(win, "%s", blankpad);
	}
	scrollok(win, 1);
	free(blankpad);
}

void tx_wcoloreol(WINDOW * win)
{
	int x, curx;
	int y __unused;
	int cury __unused;
	char sp_buf[10];

	getyx(win, cury, curx);
	getmaxyx(win, y, x);
	sprintf(sp_buf, "%%%dc", x - curx - 1);
	scrollok(win, 0);
	wprintw(win, sp_buf, ' ');
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
