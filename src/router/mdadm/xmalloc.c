/* mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include	"mdadm.h"
/*#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<sys/wait.h>
#include	<sys/un.h>
#include	<ctype.h>
#include	<dirent.h>
#include	<signal.h>
*/

void *xmalloc(size_t len)
{
	void *rv = malloc(len);
	char *msg;
	int n;
	if (rv)
		return rv;
	msg = ": memory allocation failure - aborting\n";
	n = write(2, Name, strlen(Name));
	n += write(2, msg, strlen(msg));
	exit(4+!!n);
}

void *xrealloc(void *ptr, size_t len)
{
	void *rv = realloc(ptr, len);
	char *msg;
	int n;
	if (rv)
		return rv;
	msg =  ": memory allocation failure - aborting\n";
	n = write(2, Name, strlen(Name));
	n += write(2, msg, strlen(msg));
	exit(4+!!n);
}

void *xcalloc(size_t num, size_t size)
{
	void *rv = calloc(num, size);
	char *msg;
	int n;
	if (rv)
		return rv;
	msg =  ": memory allocation failure - aborting\n";
	n = write(2, Name, strlen(Name));
	n += write(2, msg, strlen(msg));
	exit(4+!!n);
}

char *xstrdup(const char *str)
{
	char *rv = strdup(str);
	char *msg;
	int n;
	if (rv)
		return rv;
	msg =  ": memory allocation failure - aborting\n";
	n = write(2, Name, strlen(Name));
	n += write(2, msg, strlen(msg));
	exit(4+!!n);
}
