
/***

winops.c - screen configuration and setup functions

***/

#include <curses.h>
#include <stdlib.h>
#include <string.h>

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
    blankpad = (char *) malloc(sizeof(char) * (COLS + 1));

    strcpy(blankpad, "");

    for (ctr = 0; ctr <= win->_maxx; ctr++) {
	strcat(blankpad, " ");
    }

    scrollok(win, 0);
    for (ctr = 0; ctr <= win->_maxy; ctr++) {
	wmove(win, ctr, 0);
	wprintw(win, "%s", blankpad);
    }
    scrollok(win, 1);
    free(blankpad);
}

void tx_wcoloreol(WINDOW *win)
{
    int y, x;
    int cury, curx;
    char sp_buf[10];
    
    getyx(win, cury, curx);
    getmaxyx(win, y, x);
    sprintf(sp_buf, "%%%dc", x - curx - 1);
    scrollok(win, 0);
    wprintw(win, sp_buf, ' ');
}

