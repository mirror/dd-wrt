/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

timer.c		- module to display the elapsed time since a facility
		  was started

***/

#include "iptraf-ng-compat.h"

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
