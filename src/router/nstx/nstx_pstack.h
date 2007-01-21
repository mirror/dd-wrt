/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Julien Oster and Florian Heinz

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

#ifndef _NSTXHDR_H
#error "Include nstx.h first"
#endif

#ifndef NSTX_PSTACK_H
#define NSTX_PSTACK_H

struct clist
{
   int seq;
   
   char *data;
   int len;
   struct clist *next;
};

struct nstx_item {
   struct nstx_item *next;
   struct nstx_item *prev;
   
   unsigned short id;
   unsigned int timestamp;
   int frc;
   
   struct clist *chunks;
};

struct nstx_senditem {
   struct nstx_senditem *next;
   
   unsigned char *data;
   int id;
   int len;
   int offset;
   int seq;
};

void nstx_handlepacket(const char *, size_t,
    void(*)(const char*, size_t));
void init_pstack(int len);

#endif
