/*
 * bar.c - sets the highlight and rate indicator bar for the IP traffic
 *         monitor and TCP/UDP statistics
 *
 * Copyright (c) Gerard Paul Java 2001
 *
 * 
 */

/*
 * Set the highlight bar to point to the specified entry.
 * This routine also sets the cleared flag (indicates whether the
 * flow rate has been displayed).  The flow rate computation timer
 * and accumulator are also reset.
 */

#include <curses.h>
#include <time.h>
#include <string.h>
#include <winops.h>
#include "attrs.h"

void set_barptr(char **barptr, char *entry,
                time_t * starttime, char *spanbr, size_t size,
                WINDOW * win, int *cleared, int x)
{
    *barptr = entry;
    *starttime = time(NULL);
    bzero(spanbr, size);

    if (!(*cleared)) {
        wattrset(win, IPSTATATTR);
        mvwprintw(win, 0, x, "Computing");
        tx_wcoloreol(win);
        *cleared = 1;
    }
}
