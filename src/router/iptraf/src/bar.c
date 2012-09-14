/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

// TODO: full rewrite

/*
 * Set the highlight bar to point to the specified entry.
 * This routine also sets the cleared flag (indicates whether the
 * flow rate has been displayed).  The flow rate computation timer
 * and accumulator are also reset.
 */

#include "iptraf-ng-compat.h"

#include "tui/winops.h"

#include "attrs.h"

void set_barptr(void **barptr, void *entry, time_t * starttime, void *spanbr,
		size_t size, WINDOW * win, int *cleared, int x)
{
	*barptr = entry;
	*starttime = time(NULL);
	memset(spanbr, 0, size);

	if (!(*cleared)) {
		wattrset(win, IPSTATATTR);
		mvwprintw(win, 0, x, "Computing");
		tx_wcoloreol(win);
		*cleared = 1;
	}
}
