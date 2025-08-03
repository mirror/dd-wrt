/* Copyright (c) 2010
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
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

/* Deals with showing license */

#include "config.h"

#include "list_generic.h"

#include <stdbool.h>
#include <stdint.h>

#include "screen.h"

#include "help.h"
#include "misc.h"

static char ListID[] = "license";

static char license[] = {
"Copyright (c) 2025 Alexander Naumov\n"
"Copyright (c) 2018-2024 Alexander Naumov, Amadeusz Slawinski\n"
"Copyright (c) 2015-2017 Juergen Weigert, Alexander Naumov, Amadeusz Slawinski\n"
"Copyright (c) 2010-2014 Juergen Weigert, Sadrul Habib Chowdhury\n"
"Copyright (c) 2008-2009 Juergen Weigert, Michael Schroeder, Micah Cowan, Sadrul Habib Chowdhury\n"
"Copyright (c) 1993-2007 Juergen Weigert, Michael Schroeder\n"
"Copyright (c) 1987 Oliver Laumann\n"
"\n"
"This program is free software; you can redistribute it and/or modify it under \
the terms of the GNU General Public License as published by the Free \
Software Foundation; either version 3, or (at your option) any later \
version.\n"
"\n"
"This program is distributed in the hope that it will be useful, but WITHOUT \
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or \
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for \
more details.\n"
"\n"
"You should have received a copy of the GNU General Public License along with \
this program (see the file COPYING); if not, see \
https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc., 51 \
Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA.\n"
"\n"
"Send bugreports, fixes, enhancements, t-shirts, money, beer & pizza to"
" screen-devel@gnu.org\n"
};

static int gl_License_header(ListData *ldata)
{
	(void)ldata; /* unused */
	char str_version[75]; /* 75 is strlen("Screen version ") + sizeof(version[60]); */

	snprintf(str_version, 75, "Screen version %s", version);

	centerline(str_version, 0);

	return 1;
}

static int gl_License_footer(ListData *ldata)
{
	(void)ldata; /* unused */

	centerline("[Press Space for next page or Return to end.]", flayer->l_height - 1);
	return 1;
}

static int gl_License_row(ListData *ldata, ListRow *lrow)
{
	(void)ldata; /* unused */

	char *line = calloc(flayer->l_width + 1, sizeof(char));
	char *start = (char *)lrow->data;
	char *lastspace = start;
	size_t linelen = 0;

	for (char *c = start; (*c != 0) && (*c != '\n'); c++) {
		linelen++;
		if (*c == ' ')
			lastspace = c;
		if (linelen >= (size_t)flayer->l_width) {
			linelen = lastspace - start;
			break;
		}
	}
	strncpy(line, start, linelen);

	leftline(line, lrow->y, NULL);
	free(line);

	return 1;
}

static int gl_License_rebuild(ListData *ldata)
{
	/* recreate the rows */
	ListRow *row = NULL;
	size_t linelen = 0;
	char *lastspace = NULL;

	if (flayer->l_width < 40 || flayer->l_height < 5)
		return -1; 

	for (char *c = license; *c != 0; c++) {
		if (linelen == 0)
			row = glist_add_row(ldata, c, row);
		if (*c == '\n') {
			linelen = 0;
			lastspace = NULL;
			continue;
		}
		if (*c == ' ') {
			lastspace = c;
		}
		linelen++;
		if (linelen >= (size_t)flayer->l_width) {
			if (lastspace) {
				c = lastspace;
				lastspace = NULL;
				linelen = 0;
			}
		}
	}

	glist_display_all(ldata);
	return 0;
}

static int gl_License_input(ListData *ldata, char **inp, size_t *len)
{
	unsigned char ch;

	if (!ldata->selected)
		return 0;

	ch = (unsigned char)**inp;
	++*inp;
	--*len;

	switch (ch) {
	case '\f':		/* ^L to refresh */
		glist_remove_rows(ldata);
		gl_License_rebuild(ldata);
		break;

	case '\r':
	case '\n':
		glist_abort();
		*len = 0;
		break;

	default:
		/* We didn't actually process the input. */
		--*inp;
		++*len;
		return 0;
	}
	return 1;
}

static int gl_License_freerow(ListData *ldata, ListRow *row)
{
	(void)ldata; /* unused */
	(void)row; /* unused */
	/* There was no allocation when row->data was set. So nothing to do here. */
	return 0;
}

static int gl_License_free(ListData *ldata)
{
	(void)ldata; /* unused */
	/* There was no allocation in ldata->data. So nothing to do here. */
	return 0;
}

static const GenericList gl_License = {
	gl_License_header,
	gl_License_footer,
	gl_License_row,
	gl_License_input,
	gl_License_freerow,
	gl_License_free,
	gl_License_rebuild,
	NULL			/* We do not allow searching in the license page, at the moment */
};

void display_license(void)
{
	ListData *ldata;
	if (flayer->l_width < 40 || flayer->l_height < 5) {
		LMsg(0, "Window size too small for license page");
		return;
	}

	ldata = glist_display(&gl_License, ListID);
	if (!ldata)
		return;

	gl_License_rebuild(ldata);
}
