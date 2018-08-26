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
#include "slinclud.h"
#include <ctype.h>

#include "slang.h"
#include "_slang.h"

#define DEFINE_PSLWC_WIDTH_TABLE
#include "slwcwidth.h"

static int Width_Flags = 0;
int SLwchar_wcwidth (SLwchar_Type ch)
{
   int w;

   SL_WIDTH_ALOOKUP(w,ch);

   if ((w == 1) || (w == 4))
     return w;

   if (Width_Flags & SLWCWIDTH_SINGLE_WIDTH)
     return 1;

   if (w == 3)
     {
	if (Width_Flags & SLWCWIDTH_CJK_LEGACY)
	  w = 2;
	else
	  w = 1;
     }
   return w;
}

int SLwchar_set_wcwidth_flags (int flags)
{
   int oflags = Width_Flags;
   Width_Flags = flags;
   return oflags;
}
