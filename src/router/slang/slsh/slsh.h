/* -*- mode: C; mode: fold -*-
Copyright (C) 2010-2011 John E. Davis

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
#ifndef SLANG_SLSH_H
#define SLANG_SLSH_H

extern int slsh_interactive (void);
extern int slsh_use_readline (char *, int, int);
extern int slsh_init_readline_intrinsics (void);
#endif				       /* SLANG_SLSH_H */
