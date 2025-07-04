/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2023
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************
 */

#include <array.c>
#include <util.h>
#include "test.h"

void
test_unit(void)
{
  unsigned int i, j, k, k2, l, es, s;
  unsigned char *el1, el2[20];
  ARR_Instance a;

  for (i = 0; i < 1000; i++) {
    es = random() % sizeof (el2) + 1;

    a = ARR_CreateInstance(es);

    TEST_CHECK(ARR_GetSize(a) == 0);

    for (j = 0; j < 100; j++) {
      s = ARR_GetSize(a);

      switch (random() % 6) {
        case 0:
          el1 = ARR_GetNewElement(a);
          TEST_CHECK(ARR_GetSize(a) == s + 1);
          memset(el1, s % 256, es);
          TEST_CHECK(ARR_GetElement(a, s) == el1);
          break;
        case 1:
          for (k = 0; k < s; k++) {
            el1 = ARR_GetElement(a, k);
            for (l = 0; l < es; l++)
              TEST_CHECK(el1[l] == k % 256);
          }
          break;
        case 2:
          if (s == 0)
            break;
          TEST_CHECK(ARR_GetElements(a) == ARR_GetElement(a, 0));
          break;
        case 3:
          memset(el2, s % 256, es);
          ARR_AppendElement(a, el2);
          TEST_CHECK(ARR_GetSize(a) == s + 1);
          break;
        case 4:
          if (s == 0)
            break;
          k2 = random() % s;
          ARR_RemoveElement(a, k2);
          TEST_CHECK(ARR_GetSize(a) == s - 1);
          for (k = k2; k < s - 1; k++) {
            el1 = ARR_GetElement(a, k);
            for (l = 0; l < es; l++) {
              TEST_CHECK(el1[l] == (k + 1) % 256);
              el1[l] = k % 256;
            }
          }
          break;
        case 5:
          k2 = random() % 1000;
          ARR_SetSize(a, k2);
          TEST_CHECK(ARR_GetSize(a) == k2);
          for (k = s; k < k2; k++) {
            el1 = ARR_GetElement(a, k);
            for (l = 0; l < es; l++)
              el1[l] = k % 256;
          }
          break;
        default:
          assert(0);
      }
    }

    ARR_DestroyInstance(a);
  }
}
