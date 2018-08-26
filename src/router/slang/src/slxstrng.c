/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

/* These routines are simple and inefficient.  They were designed to work on
 * SunOS when using Electric Fence.
 */

#include "slinclud.h"
#include "slang.h"
#include "_slang.h"
char *SLstrcpy(register char *aa, register char *b)
{
   char *a = aa;
   while ((*a++ = *b++) != 0);
   return aa;
}

int SLstrcmp(register char *a, register char *b)
{
   while (*a && (*a == *b))
     {
	a++;
	b++;
     }
   if (*a) return((unsigned char) *a - (unsigned char) *b);
   else if (*b) return ((unsigned char) *a - (unsigned char) *b);
   else return 0;
}

char *SLstrncpy(char *a, register char *b,register  int n)
{
   register char *aa = a;
   while ((n > 0) && *b)
     {
	*aa++ = *b++;
	n--;
     }
   while (n-- > 0) *aa++ = 0;
   return (a);
}
