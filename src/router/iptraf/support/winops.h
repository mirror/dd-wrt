/***

stdwinset.h - prototype declaration for setting the standard window settings
for IPTraf

***/

#include <curses.h>
#define tx_coloreol() tx_wcoloreol(stdscr)

void tx_stdwinset(WINDOW * win);
void tx_refresh_screen(void);
void tx_colorwin(WINDOW *win);
void tx_wcoloreol(WINDOW *win);
void tx_box(WINDOW *win, int vline, int hline);
