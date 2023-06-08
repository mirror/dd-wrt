// Copyright (C) 2004, 2008, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <output.h>

int main()
{
  unsigned int const	NUMS[] = { 0, 2, 42, 529, 1073741824,
				   2147483648u, 3141592653u, 4294967295u };
  size_t		i;

  for (i=0; i<sizeof(NUMS)/sizeof(NUMS[0]); ++i) {
    writeUInt(1, NUMS[i]); write(1, "\n", 1);
  }

  return EXIT_SUCCESS;
}
