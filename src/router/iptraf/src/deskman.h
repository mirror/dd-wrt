/*
   deskman.h - header file for deskman.c
   Written by Gerard Paul Java
   Copyright (c) Gerard Paul Java 1997, 1998

 */

void draw_desktop(void);
void printnomem();
void printipcerr();
void printkeyhelp(char *keytext, char *desc, WINDOW * win,
                  int highattr, int textattr);
void stdkeyhelp(WINDOW * win);
void sortkeyhelp(void);
void tabkeyhelp(WINDOW * win);
void scrollkeyhelp();
void stdexitkeyhelp();
void indicate(char *message);
void printlargenum(unsigned long long i, WINDOW * win);
void infobox(char *text, char *prompt);
void standardcolors(int color);
void show_sort_statwin();
