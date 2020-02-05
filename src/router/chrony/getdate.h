/*  Copyright (C) 1995 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

/* Modified from the original to add stdlib.h and string.h */

#ifndef GOT_GETDATE_H
#define GOT_GETDATE_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

time_t get_date (const char *p, const time_t *now);

#endif /* GOT_GETDATE_H */
