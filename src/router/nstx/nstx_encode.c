/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Florian Heinz and Julien Oster

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  -------------------------------------------------------------------------- */

/* $Id: nstx_encode.c,v 1.17 2003/10/07 22:09:20 sky Exp $ */

#include <string.h>
#include <stdlib.h>

unsigned char map[] = 
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_1234567890";
unsigned char *revmap = NULL;

void init_revmap (void)
{
   unsigned int i;
   
   revmap = malloc(256);
   
   for (i = 0; i < strlen((char*)map); i++)
     revmap[map[i]] = i;
}
   
const unsigned char *
nstx_encode(const unsigned char *data, int len) {
   int i = 0, off = 1, cut = 0;
   static unsigned char *buf = NULL;
   
   if (len % 3)
     cut = 3 - len%3;
   
   buf = realloc(buf, ((len+2)/3)*4+2);
   buf[0] = map[cut];
   while (i < len) {
      buf[off + 0] = map[(data[i] & 252) >> 2];
      buf[off + 1] = map[((data[i] & 3) << 4) | ((data[i+1] & 240) >> 4)];
      buf[off + 2] = map[((data[i+1] & 15) << 2 ) | ((data[i+2] & 192) >> 6)];
      buf[off + 3] = map[(data[i+2] & 63)];
      i += 3;
      off += 4;
   }
   buf[off] = '\0';
   
   return buf;
}

const unsigned char *
nstx_decode(const unsigned char *data, int *rlen) {
   int i = 0, off = 1;
   int len;
   static unsigned char *buf = NULL;
   
   if (!revmap)
     init_revmap();
   
   len = strlen((char*)data);

   buf = realloc(buf, ((len+3)/4)*3);
   
   while (off+3 < len) {
      buf[i+0] = (revmap[data[off]]<<2)|((revmap[data[off+1]]&48)>>4);
      buf[i+1] = ((revmap[data[off+1]]&15)<<4)|((revmap[data[off+2]]&60)>>2);
      buf[i+2] = ((revmap[data[off+2]]&3)<<6)|(revmap[data[off+3]]);
      i += 3;
      off += 4;
   }
   *rlen = i - revmap[data[0]];
   
   return buf;
}
