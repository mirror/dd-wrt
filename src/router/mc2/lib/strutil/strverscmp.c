/*
   Compare strings while treating digits characters numerically.

   Copyright (C) 1997-2014
   Free Software Foundation, Inc.

   This file is part of the GNU C Library.
   Contributed by Jean-Fran�ois Bignolles <bignolle ecoledoc ibp fr>, 1997.

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <ctype.h>
#ifdef HAVE_STRVERSCMP
#include <string.h>
#endif /* HAVE_STRVERSCMP */

#include "lib/strutil.h"

/* states: S_N: normal, S_I: comparing integral part, S_F: comparing
   fractionnal parts, S_Z: idem but with leading Zeroes only */
#define  S_N    0x0
#define  S_I    0x4
#define  S_F    0x8
#define  S_Z    0xC

/* result_type: CMP: return diff; LEN: compare using len_diff/diff */
#define  CMP    2
#define  LEN    3


/* Compare S1 and S2 as strings holding indices/version numbers,
   returning less than, equal to or greater than zero if S1 is less than,
   equal to or greater than S2 (for more info, see the texinfo doc).
 */

int
str_verscmp (const char *s1, const char *s2)
{
#ifdef HAVE_STRVERSCMP
    return strverscmp (s1, s2);

#else /* HAVE_STRVERSCMP */
    unsigned char *p1 = (unsigned char *) s1;
    unsigned char *p2 = (unsigned char *) s2;
    unsigned char c1, c2;
    int state;
    int diff;

    /* Symbol(s)    0       [1-9]   others  (padding)
       Transition   (10) 0  (01) d  (00) x  (11) -   */
    static const unsigned int next_state[] = {
        /* state    x    d    0    - */
        /* S_N */ S_N, S_I, S_Z, S_N,
        /* S_I */ S_N, S_I, S_I, S_I,
        /* S_F */ S_N, S_F, S_F, S_F,
        /* S_Z */ S_N, S_F, S_Z, S_Z
    };

    static const int result_type[] = {
        /* state   x/x  x/d  x/0  x/-  d/x  d/d  d/0  d/-
           0/x  0/d  0/0  0/-  -/x  -/d  -/0  -/- */

        /* S_N */ CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
        CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
        /* S_I */ CMP, -1, -1, CMP, +1, LEN, LEN, CMP,
        +1, LEN, LEN, CMP, CMP, CMP, CMP, CMP,
        /* S_F */ CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
        CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
        /* S_Z */ CMP, +1, +1, CMP, -1, CMP, CMP, CMP,
        -1, CMP, CMP, CMP
    };

    if (p1 == p2)
        return 0;

    c1 = *p1++;
    c2 = *p2++;
    /* Hint: '0' is a digit too.  */
    state = S_N | ((c1 == '0') + (isdigit (c1) != 0));

    while ((diff = c1 - c2) == 0 && c1 != '\0')
    {
        state = next_state[state];
        c1 = *p1++;
        c2 = *p2++;
        state |= (c1 == '0') + (isdigit (c1) != 0);
    }

    state = result_type[state << 2 | (((c2 == '0') + (isdigit (c2) != 0)))];

    switch (state)
    {
    case CMP:
        return diff;

    case LEN:
        while (isdigit (*p1++))
            if (!isdigit (*p2++))
                return 1;

        return isdigit (*p2) ? -1 : diff;

    default:
        return state;
    }
#endif /* HAVE_STRVERSCMP */

}
