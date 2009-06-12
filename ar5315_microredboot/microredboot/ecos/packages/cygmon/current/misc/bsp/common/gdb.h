#ifndef __BSP_COMMON_GDB_H__
#define __BSP_COMMON_GDB_H__
//==========================================================================
//
//      gdb.h
//
//      Definitions for GDB stub.
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
// Purpose:      Definitions for GDB stub.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include "gdb-cpu.h"
#include <bsp/gdb-data.h>

#ifndef DEBUG_STUB
#define DEBUG_STUB 0
#endif

/*
 *  These need to match the same in devo/gdb/target.h
 */
#define  TARGET_SIGNAL_INT  2
#define  TARGET_SIGNAL_ILL  4
#define  TARGET_SIGNAL_TRAP 5
#define  TARGET_SIGNAL_ABRT 6
#define  TARGET_SIGNAL_FPE  8
#define  TARGET_SIGNAL_BUS  10
#define  TARGET_SIGNAL_SEGV 11


/*
 *  Socket to use for tcp/ip debug channel.
 */
#define GDB_TCP_SOCKET  1000


#ifndef __ASSEMBLER__

/* generic gdb protocol handler */
extern void _bsp_gdb_handler(int exc_nr, void *saved_regs);
extern gdb_data_t _bsp_gdb_data;


/* start forming an outgoing gdb packet */
/* if ack is true, prepend an ack character */
extern void _gdb_pkt_start(int ack);

/* Append data to packet using formatted string. */
extern void _gdb_pkt_append(char *fmt, ...);

/* Calculate checksum and append to end of packet. */
extern void _gdb_pkt_end(void);

/* Send the packet. Blocks waiting for ACK */
extern void _gdb_pkt_send(void);
#endif

#endif //__BSP_COMMON_GDB_H__
