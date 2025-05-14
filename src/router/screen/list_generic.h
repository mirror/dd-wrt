/* Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#ifndef SCREEN_LIST_GENERIC_H
#define SCREEN_LIST_GENERIC_H

#include <stdlib.h>

#include "window.h"

typedef struct ListData ListData;
typedef struct ListRow ListRow;
typedef struct GenericList GenericList;

struct ListRow {
	void *data;			/* Some data relevant to this row */
	ListRow *next, *prev;		/* doubly linked list */
	int y;				/* -1 if not on display */
};

struct GenericList {
	int (*gl_printheader) (ListData *);			/* Print the header */
	int (*gl_printfooter) (ListData *);			/* Print the footer */
	int (*gl_printrow) (ListData *, ListRow *);		/* Print one row */
	int (*gl_pinput) (ListData *, char **inp, size_t *len);	/* Process input */
	int (*gl_freerow) (ListData *, ListRow *);		/* Free data for a row */
	int (*gl_free) (ListData *);				/* Free data for the list */
	int (*gl_rebuild) (ListData *);				/* Rebuild display */
	int (*gl_matchrow) (ListData *, ListRow *, const char *);
};

struct ListData {
	const char *name;		/* An identifier for the list */
	ListRow *root;			/* The first item in the list */
	ListRow *selected;		/* The selected row */
	ListRow *top;			/* The topmost visible row */

	const GenericList *list_fn;		/* The functions that deal with the list */

	char *search;			/* The search term, if any */

	void *data;			/* List specific data */
};


ListRow * glist_add_row (ListData *ldata, void *data, ListRow *after);

void glist_remove_rows (ListData *ldata);

void glist_display_all (ListData *list);

ListData * glist_display (const GenericList *list, const char *name);

void glist_abort (void);

void display_displays (void);

void display_license (void);

void display_windows (int onblank, int order, Window *group);

/* global variables */

extern const struct LayFuncs ListLf;

#endif /* SCREEN_LIST_GENERIC_H */
