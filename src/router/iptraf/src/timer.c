/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

timer.c		- module to display the elapsed time since a facility
		  was started

***/

#include "iptraf-ng-compat.h"

void printelapsedtime(time_t elapsed, int x, WINDOW *win)
{
	unsigned int hours = elapsed / 3600;
	unsigned int mins = (elapsed % 3600) / 60;

	int y = getmaxy(win) - 1;

	mvwprintw(win, y, x, " Elapsed time: %3u:%02u ", hours, mins);
}
