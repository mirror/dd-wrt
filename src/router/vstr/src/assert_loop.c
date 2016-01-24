#define ASSERT_LOOP_C
/*
 *  Copyright (C) 1999, 2000, 2002 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@and.org
 */
#include "main.h"

#if defined(USE_ASSERT_LOOP) && !defined(NDEBUG)

/* don't raise a signal as it tends to blow bits of the stack */
void vstr__assert_loop(const char *expr,
                       const char *file_name, int line_num,
                       const char *pretty_func)
{
  volatile int keep_going = 1;

  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n",
          expr, pretty_func, line_num, file_name);

  while (keep_going) { /* just wait... can be set from gdb */ }
}



#endif
