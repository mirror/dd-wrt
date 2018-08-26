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

#include "slang.h"
#include "_slang.h"

int SLang_init_all (void)
{
   if ((-1 == SLang_init_slang ())
#if SLANG_HAS_FLOAT
       || (-1 == SLang_init_slmath ())
#endif
       || (-1 == SLang_init_posix_dir ())
       || (-1 == SLang_init_posix_process ())
       || (-1 == SLang_init_stdio ())
       || (-1 == SLang_init_array ())
       || (-1 == SLang_init_posix_io ())
       || (-1 == SLang_init_signal ())
       || (-1 == SLang_init_ospath ())
       )
     return -1;

   return 0;
}

