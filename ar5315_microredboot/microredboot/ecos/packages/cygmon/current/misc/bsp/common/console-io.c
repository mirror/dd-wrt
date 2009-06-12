//==========================================================================
//
//      console-io.c
//
//      BSP Console Channel Interfaces.
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
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      BSP Console Channel Interfaces.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"

static unsigned char __console_ungetc;

void
bsp_console_write(const char *p, int len)
{
    struct bsp_comm_procs *com;

    if ((com = bsp_shared_data->__console_procs) != NULL)
	com->__write(com->ch_data, p, len);
    else
	bsp_debug_write(p, len);
}


int
bsp_console_read(char *p, int len)
{
    struct bsp_comm_procs *com;

    if (len <= 0)
	return 0;

    if ((com = bsp_shared_data->__console_procs) != NULL) {
	if (__console_ungetc) {
	    *p = __console_ungetc;
	    __console_ungetc = 0;
	    return 1;
	}
	return com->__read(com->ch_data, p, len);
    } else
	return bsp_debug_read(p, len);
}

/*#define PRINTABLE_ONLY*/
#ifdef PRINTABLE_ONLY
#include <ctype.h>
#endif /* PRINTABLE_ONLY */

void
bsp_console_putc(char ch)
{
    struct bsp_comm_procs *com;

#ifdef PRINTABLE_ONLY
    if ((!isprint(ch)) && (!isspace(ch)))
        ch = '.';
#endif /* PRINTABLE_ONLY */

    if ((com = bsp_shared_data->__console_procs) != NULL)
	com->__putc(com->ch_data, ch);
    else
	bsp_debug_putc(ch);
}

int
bsp_console_getc(void)
{
    struct bsp_comm_procs *com;
    int    ch;

    if ((com = bsp_shared_data->__console_procs) != NULL) {
	if (__console_ungetc) {
	    ch = __console_ungetc;
	    __console_ungetc = 0;
	    return ch;
	}
	return com->__getc(com->ch_data);
    } else
	return bsp_debug_getc();
}


void
bsp_console_ungetc(char ch)
{
    if (bsp_shared_data->__console_procs != NULL)
	__console_ungetc = (unsigned char)ch;
    else
	bsp_debug_ungetc(ch);
}


