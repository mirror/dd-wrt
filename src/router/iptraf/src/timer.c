
/***

timer.c		- module to display the elapsed time since a facility
		  was started

Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <curses.h>
#include <time.h>

void printelapsedtime(time_t start, time_t now, int y, int x, WINDOW * win)
{
    time_t elapsed;
    unsigned int hours;
    unsigned int mins;

    elapsed = now - start;

    hours = elapsed / 3600;
    mins = (elapsed % 3600) / 60;

    wmove(win, y, x);
    wprintw(win, " Elapsed time: %3u:%02u ", hours, mins);
}
