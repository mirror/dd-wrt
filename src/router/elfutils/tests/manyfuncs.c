/* Test file for a binary with 64K+ sections and symbols.
   Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


/* We need a quoted string starting with a dot (the section name).  */
#define DQ(x) #x
#define DOTQUOTE(x) DQ(.x)

/* We want 2 * (8 ^ 5) = 2 * ((2 ^ 3) ^ 5) = 2 ^ 16 = 64K functions.  */
#define f(x) __attribute__ ((section (DOTQUOTE(x)))) void x(void) {};
#define A(x) f(x##0) f(x##1) f(x##2) f(x##3) f(x##4) f(x##5) f(x##6) f(x##7)
#define B(x) A(x##0) A(x##1) A(x##2) A(x##3) A(x##4) A(x##5) A(x##6) A(x##7)
#define C(x) B(x##0) B(x##1) B(x##2) B(x##3) B(x##4) B(x##5) B(x##6) B(x##7)
#define D(x) C(x##0) C(x##1) C(x##2) C(x##3) C(x##4) C(x##5) C(x##6) C(x##7)
#define E(x) D(x##0) D(x##1) D(x##2) D(x##3) D(x##4) D(x##5) D(x##6) D(x##7)
E(y)
E(z)

#undef f
#undef A
#undef B
#undef C
#undef D
#undef E

/* Call all functions from main.  */
#define f(x) x();
#define A(x) f(x##0) f(x##1) f(x##2) f(x##3) f(x##4) f(x##5) f(x##6) f(x##7)
#define B(x) A(x##0) A(x##1) A(x##2) A(x##3) A(x##4) A(x##5) A(x##6) A(x##7)
#define C(x) B(x##0) B(x##1) B(x##2) B(x##3) B(x##4) B(x##5) B(x##6) B(x##7)
#define D(x) C(x##0) C(x##1) C(x##2) C(x##3) C(x##4) C(x##5) C(x##6) C(x##7)
#define E(x) D(x##0) D(x##1) D(x##2) D(x##3) D(x##4) D(x##5) D(x##6) D(x##7)

int
main ()
{
E(y)
E(z)
}
