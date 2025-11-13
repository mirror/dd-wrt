/*  GNU SED, a batch stream editor.
    Copyright (C) 2003-2022 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <https://www.gnu.org/licenses/>. */

#include "sed.h"
#include <stdlib.h>
#include <string.h>

#include "localcharset.h"

int mb_cur_max;
bool is_utf8;

/* Return non-zero if CH is part of a valid multibyte sequence:
   Either incomplete yet valid sequence (in case of a leading byte),
   or the last byte of a valid multibyte sequence.

   Return zero in all other cases:
    CH is a valid single-byte character (e.g. 0x01-0x7F in UTF-8 locales);
    CH is an invalid byte in a multibyte sequence for the currentl locale,
    CH is the NUL byte.

   Reset CUR_STAT in the case of an invalid byte.
*/
int
is_mb_char (int ch, mbstate_t *cur_stat)
{
  const char c = ch ;
  const int mb_pending = !mbsinit (cur_stat);
  const int result = mbrtowc (NULL, &c, 1, cur_stat);

  switch (result)
    {
    case -2: /* Beginning or middle of valid multibyte sequence */
      return 1;

    case -1: /* Invalid sequence, byte treated like a single-byte character */
      memset (cur_stat, 0, sizeof (mbstate_t));
      return 0;

    case 1: /* A valid byte, check if part of on-going multibyte sequence */
      return mb_pending;

    case 0: /* Special case of mbrtowc(3): the NUL character */
      /* TODO: test this */
      return 1;

    default: /* Should never happen, as per mbrtowc(3) documentation */
      panic ("is_mb_char: mbrtowc (0x%x) returned %d",
             (unsigned int) ch, result);
    }
}

void
initialize_mbcs (void)
{
  /* For UTF-8, we know that the encoding is stateless.  */
  const char *codeset_name;

  codeset_name = locale_charset ();
  is_utf8 = (strcmp (codeset_name, "UTF-8") == 0);

  mb_cur_max = MB_CUR_MAX;
}
