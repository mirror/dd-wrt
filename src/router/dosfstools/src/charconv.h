/* charconv.h

   Copyright (C) 2010 Alexander Korolkov <alexander.korolkov@gmail.com>
   Copyright (C) 2018-2020 Pali Rohár <pali.rohar@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

   The complete text of the GNU General Public License
   can be found in /usr/share/common-licenses/GPL-3 file.
*/

#ifndef _CHARCONV_H
#define _CHARCONV_H

#include <stddef.h>

#define DEFAULT_DOS_CODEPAGE 850

int set_dos_codepage(int codepage);
int dos_char_to_printable(char **p, unsigned char c, unsigned int out_size);
int local_string_to_dos_string(char *out, char *in, unsigned int out_size);
int dos_string_to_wchar_string(wchar_t *out, char *in, unsigned int out_size);
int wchar_string_to_dos_string(char *out, wchar_t *in, unsigned int out_size);

#endif
