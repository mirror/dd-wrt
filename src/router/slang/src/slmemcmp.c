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

/* This is an UNSIGNED comparison designed for systems that either do not have
* this function or performed a signed comparison (SunOS)
*/
int SLmemcmp(register char *s1, register char *s2, int n)
{
   register int cmp;
   register char *s1max;

   s1max = s1 + (n - 32);

   while (s1 <= s1max)
     {
	if (*s1 != *s2) return ((unsigned char) *s1 - (unsigned char) *s2);
	if (*(s1 + 1) != *(s2 + 1)) return ((unsigned char) *(s1 + 1) - (unsigned char) *(s2 + 1));
	if (*(s1 + 2) != *(s2 + 2)) return ((unsigned char) *(s1 + 2) - (unsigned char) *(s2 + 2));
	if (*(s1 + 3) != *(s2 + 3)) return ((unsigned char) *(s1 + 3) - (unsigned char) *(s2 + 3));
	if (*(s1 + 4) != *(s2 + 4)) return ((unsigned char) *(s1 + 4) - (unsigned char) *(s2 + 4));
	if (*(s1 + 5) != *(s2 + 5)) return ((unsigned char) *(s1 + 5) - (unsigned char) *(s2 + 5));
	if (*(s1 + 6) != *(s2 + 6)) return ((unsigned char) *(s1 + 6) - (unsigned char) *(s2 + 6));
	if (*(s1 + 7) != *(s2 + 7)) return ((unsigned char) *(s1 + 7) - (unsigned char) *(s2 + 7));
	if (*(s1 + 8) != *(s2 + 8)) return ((unsigned char) *(s1 + 8) - (unsigned char) *(s2 + 8));
	if (*(s1 + 9) != *(s2 + 9)) return ((unsigned char) *(s1 + 9) - (unsigned char) *(s2 + 9));
	if (*(s1 + 10) != *(s2 + 10)) return ((unsigned char) *(s1 + 10) - (unsigned char) *(s2 + 10));
	if (*(s1 + 11) != *(s2 + 11)) return ((unsigned char) *(s1 + 11) - (unsigned char) *(s2 + 11));
	if (*(s1 + 12) != *(s2 + 12)) return ((unsigned char) *(s1 + 12) - (unsigned char) *(s2 + 12));
	if (*(s1 + 13) != *(s2 + 13)) return ((unsigned char) *(s1 + 13) - (unsigned char) *(s2 + 13));
	if (*(s1 + 14) != *(s2 + 14)) return ((unsigned char) *(s1 + 14) - (unsigned char) *(s2 + 14));
	if (*(s1 + 15) != *(s2 + 15)) return ((unsigned char) *(s1 + 15) - (unsigned char) *(s2 + 15));
	if (*(s1 + 16) != *(s2 + 16)) return ((unsigned char) *(s1 + 16) - (unsigned char) *(s2 + 16));
	if (*(s1 + 17) != *(s2 + 17)) return ((unsigned char) *(s1 + 17) - (unsigned char) *(s2 + 17));
	if (*(s1 + 18) != *(s2 + 18)) return ((unsigned char) *(s1 + 18) - (unsigned char) *(s2 + 18));
	if (*(s1 + 19) != *(s2 + 19)) return ((unsigned char) *(s1 + 19) - (unsigned char) *(s2 + 19));
	if (*(s1 + 20) != *(s2 + 20)) return ((unsigned char) *(s1 + 20) - (unsigned char) *(s2 + 20));
	if (*(s1 + 21) != *(s2 + 21)) return ((unsigned char) *(s1 + 21) - (unsigned char) *(s2 + 21));
	if (*(s1 + 22) != *(s2 + 22)) return ((unsigned char) *(s1 + 22) - (unsigned char) *(s2 + 22));
	if (*(s1 + 23) != *(s2 + 23)) return ((unsigned char) *(s1 + 23) - (unsigned char) *(s2 + 23));
	if (*(s1 + 24) != *(s2 + 24)) return ((unsigned char) *(s1 + 24) - (unsigned char) *(s2 + 24));
	if (*(s1 + 25) != *(s2 + 25)) return ((unsigned char) *(s1 + 25) - (unsigned char) *(s2 + 25));
	if (*(s1 + 26) != *(s2 + 26)) return ((unsigned char) *(s1 + 26) - (unsigned char) *(s2 + 26));
	if (*(s1 + 27) != *(s2 + 27)) return ((unsigned char) *(s1 + 27) - (unsigned char) *(s2 + 27));
	if (*(s1 + 28) != *(s2 + 28)) return ((unsigned char) *(s1 + 28) - (unsigned char) *(s2 + 28));
	if (*(s1 + 29) != *(s2 + 29)) return ((unsigned char) *(s1 + 29) - (unsigned char) *(s2 + 29));
	if (*(s1 + 30) != *(s2 + 30)) return ((unsigned char) *(s1 + 30) - (unsigned char) *(s2 + 30));
	if (*(s1 + 31) != *(s2 + 31)) return ((unsigned char) *(s1 + 31) - (unsigned char) *(s2 + 31));
	s1 += 32; s2 += 32;
     }

   s1max = s1 + (n % 32);

   while (s1 < s1max)
     {
	cmp = (unsigned char) *s1 - (unsigned char) *s2;
	if (cmp) return(cmp);
	s1++;
	s2++;
     }

   return(0);
}
