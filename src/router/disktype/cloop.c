/*
 * cloop.c
 * Layered data source for Linux cloop images.
 *
 * Copyright (c) 2005 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

/*
 * image file detection
 */

void detect_cloop(SECTION *section, int level)
{
  unsigned char *buf;
  u4 blocksize, blockcount;
  char s[256];
  const char *sig_20 = "#!/bin/sh\n#V2.0 Format\nmodprobe cloop";

  /* check for signature */
  if (get_buffer(section, 0, 256, (void **)&buf) < 256)
    return;
  if (memcmp(buf, sig_20, strlen(sig_20)) != 0)
    return;

  print_line(level, "Linux cloop 2.0 image");

  blocksize = get_be_long(buf + 128);
  blockcount = get_be_long(buf + 132);
  format_blocky_size(s, blockcount, blocksize, "blocks", NULL);
  print_line(level + 1, "Volume size %s", s);
}

/* EOF */
