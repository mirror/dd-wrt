//=============================================================================
//
//      split_words.c
//
//      Quick hack to handle splitting of binary images into .lo and .hi files
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-05-29
// Purpose:      Split binary ROM image into .hi and .lo parts for EPROMs
// Description:  Usage:
//                 split_words install/bin/redboot.bin
//
//               Will output file out.lo and out.hi which should be programmed
//               to the EPROMs.
//
//####DESCRIPTIONEND####
//
//=============================================================================



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int
main(int argc, char** argv)
{
  int fi, f1, f2;
  ssize_t s1, s2;
  fi = open(argv[1], O_RDONLY);
  f1 = open("out.lo", O_WRONLY|O_CREAT|O_TRUNC, 0666);
  f2 = open("out.hi", O_WRONLY|O_CREAT|O_TRUNC, 0666);

  if (-1 == f1 || -1 == f2) {
    fprintf(stderr, "failed to open output files\n");
    return -1;
  }

  if (-1 == fi) {
    fprintf(stderr, "failed to open input file\n");
    return -1;
  }

  do {
    unsigned char b[4];
    s1 = read(fi, b, 4);
    
    if (s1 == 4) {
      write(f1, b, 2);
      write(f2, b+2, 2);
    }
  } while (s1);

  return 0;
}
