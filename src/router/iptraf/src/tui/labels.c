/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/*
 * labels.c - some common keyhelp printing routines for the iptraf
 * user interface library
 */

#include "iptraf-ng-compat.h"

#include "winops.h"

void tx_printkeyhelp(char *keytext, char *desc, WINDOW * win, int highattr,
		     int textattr)
{
	wattrset(win, highattr);
	wprintw(win, "%s", keytext);
	wattrset(win, textattr);
	wprintw(win, "%s", desc);
}

void tx_menukeyhelp(int textattr, int highattr)
{
	move(LINES - 1, 1);
	tx_printkeyhelp("Up/Down", "-Move selector  ", stdscr, highattr,
			textattr);
	tx_printkeyhelp("Enter", "-execute", stdscr, highattr, textattr);
	tx_coloreol();
}

void tx_listkeyhelp(int textattr, int highattr)
{
	move(LINES - 1, 1);
	tx_printkeyhelp("Up/Down", "-move pointer  ", stdscr, highattr,
			textattr);
	tx_printkeyhelp("Enter", "-select  ", stdscr, highattr, textattr);
	tx_printkeyhelp("X/Ctrl+X", "-close list", stdscr, highattr, textattr);
	tx_coloreol();
}
