/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 2001-2006 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Utility module linked to utilities to provide functions normally
 * present in full src tree.
 * $Id: misc.c,v 1.6 2006/11/01 03:11:04 castaglia Exp $
 */

#include "conf.h"

/* "safe" strcat, saves room for \0 at end of dest, and refuses to copy
 * more than "n" bytes.
 */

char *sstrcat(char *dest, const char *src, size_t n) {
  register char *d;

  if (n == 0)
    return NULL;

  for (d = dest; *d && n > 1; d++, n--)
    ;

  while (n-- > 1 && *src)
    *d++ = *src++;

  *d = 0;
  return dest;
}

/* "safe" strncpy, saves room for \0 at end of dest, and refuses to copy
 * more than "n" bytes.
 */
char *sstrncpy(char *dest, const char *src, size_t n) {
  register char *d = dest;

  if (!dest)
    return NULL;

  if (n == 0)
    return NULL;

  if (src && *src) {
    for (; *src && n > 1; n--)
      *d++ = *src++;
  }

  *d = '\0';

  return dest;
}
