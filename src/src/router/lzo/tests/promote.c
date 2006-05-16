/* promote.c -- test intergral promotion

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */

#include <stdio.h>

int main()
{
  unsigned char c;
  int s;

  c = (unsigned char) (1 << (8 * sizeof(char) - 1));
  s = 8 * (int) (sizeof(int) - sizeof(char));

  printf("Integral promotion: ");
  if ((c << s) > 0)
  {
    printf("Classic C (unsigned-preserving)\n");
    printf("%d %d %u\n", c, s, c << s);
    return 1;
  }
  else
  {
    printf("ANSI C (value-preserving)\n");
    printf("%d %d %d\n", c, s, c << s);
    return 0;
  }
}
