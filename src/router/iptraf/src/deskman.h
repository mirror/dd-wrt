#ifndef IPTRAF_NG_DESKMAN_H
#define IPTRAF_NG_DESKMAN_H

/*
   deskman.h - header file for deskman.c
 */

void draw_desktop(void);
void about(void);
void printipcerr(void);
void printkeyhelp(char *keytext, char *desc, WINDOW * win, int highattr,
		  int textattr);
void stdkeyhelp(WINDOW * win);
void sortkeyhelp(void);
void tabkeyhelp(WINDOW * win);
void scrollkeyhelp(void);
void stdexitkeyhelp(void);
void indicate(char *message);
void printlargenum(unsigned long long i, WINDOW * win);
int screen_update_needed(const struct timeval *now, const struct timeval *last);
void infobox(char *text, char *prompt);
void standardcolors(int color);
void show_sort_statwin(WINDOW **, PANEL **);

#endif	/* IPTRAF_NG_DESKMAN_H */
