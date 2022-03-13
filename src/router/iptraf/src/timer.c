/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

timer.c		- module to display the elapsed time since a facility
		  was started

***/

#include "iptraf-ng-compat.h"
#include "timer.h"

void printelapsedtime(time_t elapsed, int x, WINDOW *win)
{
	unsigned int hours = elapsed / 3600;
	unsigned int mins = (elapsed % 3600) / 60;

	int y = getmaxy(win) - 1;

	mvwprintw(win, y, x, " Time: %3u:%02u ", hours, mins);
}

inline bool time_after(struct timespec const *a, struct timespec const *b)
{
	if (a->tv_sec > b->tv_sec)
		return true;
	if (a->tv_sec < b->tv_sec)
		return false;
	if(a->tv_nsec > b->tv_nsec)
		return true;
	else
		return false;
}

void time_add_msecs(struct timespec *time, unsigned int msecs)
{
	if (time != NULL) {
		while (msecs >= 1000) {
			time->tv_sec++;
			msecs -= 1000;
		}
		time->tv_nsec += msecs * 1000000;
		while (time->tv_nsec >= 1000000000) {
			time->tv_sec++;
			time->tv_nsec -= 1000000000;
		}
	}
}
