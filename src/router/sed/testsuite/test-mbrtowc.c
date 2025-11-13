/* Auxiliary program to test mbrtowc(3) behaviour.
   Copyright 2016-2022 Free Software Foundation, Inc.

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

/* Test the operating-system's native mbrtowc(3) function,
   by feeding it multibyte seqeunces one byte at a time,
   and reporting the result.

   The program prints the following values after each mbrtowc invocation,
   separated by commas:

   -2  the octet is contributes to a valid yet incomplete multibyte sequence
       in the current locale.

   -1  the octet causes an encoding error.

    0  the octet represents a NUL byte

    1  the octet is a valid single-byte character, OR
       completes a valid multibyte sequence.

  Because the program invokes mbrtowc(3) byte-by-byte, the reported
  result should never be larger than 1.

  Example of typical output with UTF-8 encoding
  ---------------------------------------------

  The unicode character 'N-ARY SUMMATION' (U+2211), encoded in UTF-8 as:
    hex: 0xE2 0x88 0x91
    oct:  342  210  211

  Decoding the valid sequence byte-by-byte gives:
    $ printf '\342\210\221' | LC_ALL=en_US.UTF-8 test-mbrtowc
    -2,-2,1

  '\210' is not a valid leading byte in UTF-8,
  thus the first byte gives -1, and the 'X' is treated
  as a valid single-byte character:

    $ printf '\210X' | LC_ALL=en_US.UTF-8 test-mbrtowc
    -1,1

  '\342' is a valid yet incomplete multibyte sequence.
  Passing it to mbrtowc results in value '-2'.
  The following value 'X' gives an encoding error '-1'
  (as 'X' is not a valid trailing byte in a multibyte UTF-8 sequence):

    $ printf '\342X' | LC_ALL=en_US.UTF-8 test-mbrtowc
    -2,-1


  Detecting implementation bugs in mbrtowc
  ----------------------------------------

  UTF-8 implementation is correct on most operating systems.
  Other multibyte locales might present more difficulties.
  An example is the Japanese SHIFT-JIS locale under Mac OS X.
  NOTE: The locale is 'ja_JP.SJIS' under Mac OS X, 'ja_JP.shiftjis'
  under Ubuntu. 'ja_JP.sjis' was also found on some systems.

  Using unicode character 'KATAKANA LETTER ZE' (U+30BC)
   UTF-8:    hex: 0xE3  0x82  0xBC
   Shift-jis hex: 0x83  0x5B
             oct:  203   133

  The following is a valid multibyte sequence in SHIFT-JIS,
  the first byte should result in '-2' (valid yet incomplete),
  and the second byte should result in '1' (a valid multibyte sequence
  completed):

    $ printf '\203\133' | LC_ALL=ja_JP.SJIS test-mbrtowc
    -2,1

  The follwing is an INVALID multibyte sequence in SHIFT-JIS
  (The byte ':' is not valid as a second octet).
  Buggy implementations will accept this as a valid multibyte sequence:

    # NOTE: this result indicates a buggy mbrtowc
    $ printf '\203:' | LC_ALL=ja_JP.SJIS test-mbrtowc
    -2,1

  A correct implementations should report '-1' for the second byte (i.e.
  an encoding error):

    $ printf '\203:' | LC_ALL=ja_JP.SJIS test-mbrtowc
    -2,-1


  Expected results with correct implementations
  ---------------------------------------------

  In GNU Sed some tests purposely use invalid multibyte sequences
  to test sed's behaviour. A buggy implemetation of mbrtowc
  would result in false-alarm failures.

  The following are expected results in correct implementations:
  (locale names are from Mac OS X):

    $ printf '\203\133' | LC_ALL=ja_JP.SJIS test-mbrtowc
    -2,1
    $ printf '\203:' | LC_ALL=ja_JP.SJIS test-mbrtowc
    -2,-1
    $ printf '\262C' | LC_ALL=ja_JP.eucJP test-mbrtowc
    -2,-1
*/

#include <config.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "closeout.h"
#include "error.h"
#include "progname.h"

/* stub replacement for non-standard err(3) */
static int
die (const char *msg)
{
  error (0, 0, "%s: error: %s\n", program_name, msg);
  exit (EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
  int c;
  int first = 1;

  set_program_name (argv[0]);
  if (!setlocale (LC_ALL, ""))
    die ("failed to set locale");

  while ((c = getchar ()) != EOF)
    {
      wchar_t wc;
      char ch = (unsigned char) c;
      int i = (int) mbrtowc (&wc, &ch, 1, NULL);

      if (!first)
        putchar (',');
      first = 0;

      printf ("%d", i);
    }

  if (first)
    die ("empty input");

  putchar ('\n');

  if (ferror (stdin))
    die ("read error");
  close_stdout ();

  exit (EXIT_SUCCESS);
}
