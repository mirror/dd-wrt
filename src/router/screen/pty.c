/* Copyright (c) 2008, 2009
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

#include "config.h"

#include "pty.h"

#include <sys/ioctl.h>

#include "screen.h"

#if defined(HAVE_PTY_H)
# include <pty.h>
#endif
#if defined(HAVE_UTIL_H)
# include <util.h>
#endif
#if defined(HAVE_LIBUTIL_H)
# include <libutil.h>
#endif

int pty_preopen = 0;

/***************************************************************/

int OpenPTY(char **ttyn)
{
	static char TtyName[32];
	int f, s;

	if (openpty(&f, &s, TtyName, NULL, NULL) != 0)
		return -1;
	close(s);

	tcflush(f, TCIOFLUSH);
#if defined(LOCKPTY) && defined(TIOCEXCL) && defined(TIOCNXCL)
	(void)ioctl(f, TIOCEXCL, NULL);
#endif

	pty_preopen = 1;
	*ttyn = TtyName;
	return f;
}

int ClosePTY(int fd)
{
#if defined(LOCKPTY) && defined(TIOCEXCL) && defined(TIOCNXCL)
	(void)ioctl(fd, TIOCNXCL, NULL);
#endif

	return close(fd);
}
