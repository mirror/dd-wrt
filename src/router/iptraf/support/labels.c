/*
 * labels.c - some common keyhelp printing routines for the iptraf
 * user interface library
 *
 * Written by Gerard Paul Java
 * Copyright (c) Gerard Paul Java 2001
 */
 
#include <curses.h>
#include <panel.h>
#include "winops.h"

void tx_printkeyhelp(char *keytext, char *desc, WINDOW * win,
                  int highattr, int textattr)
{
    wattrset(win, highattr);
    wprintw(win, "%s", keytext);
    wattrset(win, textattr);
    wprintw(win, "%s", desc);
}

void tx_menukeyhelp(int textattr, int highattr)
{
    move(LINES - 1, 1);
    tx_printkeyhelp("Up/Down", "-Move selector  ", stdscr, highattr, textattr);
    tx_printkeyhelp("Enter", "-execute", stdscr, highattr, textattr);
    tx_coloreol();
}

void tx_listkeyhelp(int textattr, int highattr)
{
    move(LINES - 1, 1);
    tx_printkeyhelp("Up/Down", "-move pointer  ", stdscr, highattr, textattr);
    tx_printkeyhelp("Enter", "-select  ", stdscr, highattr, textattr);
    tx_printkeyhelp("X/Ctrl+X", "-close list", stdscr, highattr, textattr);
    tx_coloreol();
}
