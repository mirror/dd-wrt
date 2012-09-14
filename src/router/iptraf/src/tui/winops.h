#ifndef IPTRAF_NG_TUI_WINOPS_H
#define IPTRAF_NG_TUI_WINOPS_H

/***

stdwinset.h - prototype declaration for setting the standard window settings
for IPTraf

***/

#define tx_coloreol() tx_wcoloreol(stdscr)

void tx_stdwinset(WINDOW * win);
void tx_refresh_screen(void);
void tx_colorwin(WINDOW * win);
void tx_wcoloreol(WINDOW * win);
void tx_box(WINDOW * win, int vline, int hline);

#endif	/* IPTRAF_NG_TUI_WINOPS_H */
