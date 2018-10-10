/* 
   Unix SMB/CIFS implementation.
   replacement routines for broken systems
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Jelmer Vernooij 2005-2008

     ** NOTE! The following LGPL license applies to the replace
     ** library. This does NOT imply that all of Samba is released
     ** under the LGPL
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include "util.h"

/* glibc doesn't have strlcpy, strlcat. Ensure we do. JRA. We
 * don't link to libreplace so need them here. */

/* like strncpy but does not 0 fill the buffer and always null
 *    terminates. bufsize is the size of the destination buffer */

#ifndef HAVE_STRLCPY
size_t strlcpy(char *d, const char *s, size_t bufsize)
{
	size_t len = strlen(s);
	size_t ret = len;
	if (bufsize <= 0) return 0;
	if (len >= bufsize) len = bufsize-1;
	memcpy(d, s, len);
	d[len] = 0;
	return ret;
}
#endif

/* like strncat but does not 0 fill the buffer and always null
 *    terminates. bufsize is the length of the buffer, which should
 *       be one more than the maximum resulting string length */

#ifndef HAVE_STRLCAT
size_t strlcat(char *d, const char *s, size_t bufsize)
{
	size_t len1 = strlen(d);
	size_t len2 = strlen(s);
	size_t ret = len1 + len2;

	if (len1+len2 >= bufsize) {
		if (bufsize < (len1+1)) {
			return ret;
		}
		len2 = bufsize - (len1+1);
	}
	if (len2 > 0) {
		memcpy(d+len1, s, len2);
		d[len1+len2] = 0;
	}
	return ret;
}
#endif

/* caller frees username if necessary */
char *
getusername(uid_t uid)
{
	char *username = NULL;
	struct passwd *password = getpwuid(uid);

	if (password)
		username = password->pw_name;
	return username;
}

