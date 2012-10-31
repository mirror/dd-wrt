/*
  LzmaDecode.h
  LZMA Decoder interface

  LZMA SDK 4.05 Copyright (c) 1999-2004 Igor Pavlov (2004-08-25)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to
  statically or dynamically link your code (or bind by name) to the
  interfaces of this file without subjecting your linked code to the
  terms of the CPL or GNU LGPL. Any modifications or additions
  to this file, however, are subject to the LGPL or CPL terms.
*/

#ifndef __LZMADECODE_H
#define __LZMADECODE_H

#include <Types.h>

int LzmaDecode2(int lc, int lp, int pb,
    unsigned char *inStream, unsigned int inSize,
    unsigned char *outStream, unsigned int outSize,
    unsigned int *outSizeProcessed);

#endif
