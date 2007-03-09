//==========================================================================
//
//      net/pktbuf.c
//
//      Stand-alone network packet support for RedBoot
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <net/net.h>

#define MAX_PKTBUF CYGNUM_REDBOOT_NETWORKING_MAX_PKTBUF

#define BUFF_STATS 1

#if BUFF_STATS
int max_alloc = 0;
int num_alloc = 0;
int num_free  = 0;
#endif

static pktbuf_t  pktbuf_list[MAX_PKTBUF];
static word      bufdata[MAX_PKTBUF][ETH_MAX_PKTLEN/2 + 1];
static pktbuf_t *free_list;


/*
 * Initialize the free list.
 */
void
__pktbuf_init(void)
{
    int  i;
    word *p;
    static int init = 0;

    if (init) return;
    init = 1;

    for (i = 0; i < MAX_PKTBUF; i++) {
	p = bufdata[i];
	if ((((unsigned long)p) & 2) != 0)
	    ++p;
	pktbuf_list[i].buf = p;
	pktbuf_list[i].bufsize = ETH_MAX_PKTLEN;
	pktbuf_list[i].next = free_list;
	free_list = &pktbuf_list[i];
    }
}

void
__pktbuf_dump(void)
{
    int i;
    for (i = 0; i < MAX_PKTBUF; i++) {
        diag_printf("Buf[%d]/%p: buf: %p, len: %d/%d, next: %p\n", 
                    i,
                    (void*)&pktbuf_list[i],
                    (void*)pktbuf_list[i].buf,
                    pktbuf_list[i].bufsize,
                    pktbuf_list[i].pkt_bytes,
                    (void*)pktbuf_list[i].next);
    }
    diag_printf("Free list = %p\n", (void*)free_list);
}

/*
 * simple pktbuf allocation
 */
pktbuf_t *
__pktbuf_alloc(int nbytes)
{
    pktbuf_t *p = free_list;

    if (p) {
	free_list = p->next;
	p->ip_hdr  = (ip_header_t *)p->buf;
	p->tcp_hdr = (tcp_header_t *)(p->ip_hdr + 1);
	p->pkt_bytes = 0;
#if BUFF_STATS
	++num_alloc;
	if ((num_alloc - num_free) > max_alloc)
	    max_alloc = num_alloc - num_free;
#endif
    }
    return p;
}


/*
 * free a pktbuf.
 */
void
__pktbuf_free(pktbuf_t *pkt)
{
#if BUFF_STATS
    --num_alloc;
#endif
#ifdef BSP_LOG
    {
	int i;
	word *p;

	for (i = 0; i < MAX_PKTBUF; i++) {
	    p = bufdata[i];
	    if ((((unsigned long)p) & 2) == 0)
		++p;
	    if (p == (word *)pkt)
		break;
	}
	if (i < MAX_PKTBUF) {
	    BSPLOG(bsp_log("__pktbuf_free: bad pkt[%x].\n", pkt));
	    BSPLOG(while(1));
	}
    }
#endif
    pkt->next = free_list;
    free_list = pkt;
}


