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

/* These routines are fast memcpy, memset routines.  When available, I
   use system rouines.  For msdos, I use inline assembly. */

/* The current versions only work in the forward direction only!! */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

char *SLmemchr(register char *p, register char c, register int n)
{
   int n2;
   register char *pmax;

   pmax = p + (n - 32);

   while (p <= pmax)
     {
	if ((*p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c))
	  return p;
	p++;
     }

   n2 = n % 32;

   while (n2--)
     {
	if (*p == c) return p;
	p++;
     }
   return(NULL);
}
