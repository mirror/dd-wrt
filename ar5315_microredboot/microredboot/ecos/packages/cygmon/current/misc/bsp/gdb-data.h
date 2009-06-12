#ifndef __BSP_GDB_DATA_H__
#define __BSP_GDB_DATA_H__
//==========================================================================
//
//      gdb-data.h
//
//      Shared data specific to gdb stub.
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
// Purpose:      Shared data specific to gdb stub.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#ifndef __ASSEMBLER__
typedef int (*gdb_memproc_t)(void *__addr,    /* start addr of memory to read/write */
			     int  __asid,     /* address space id */
			     int  __size,     /* size of individual read/write ops */
			     int  __n,        /* number of read/write operations */
			     void *__buf);    /* result(read)/src(write) buffer */


typedef void (*gdb_regproc_t)(int  __regno,   /* Register number */
			      void *__regs,   /* pointer to saved regs */
			      void *__val);   /* pointer to register value */


typedef struct {
    /*
     * An application may override the standard BSP memory
     * read and/or write routines with these hooks.
     */
    gdb_memproc_t	__mem_read_hook;
    gdb_memproc_t	__mem_write_hook;

    /*
     * An application may override the standard BSP register
     * access routines with these hooks.
     */
    gdb_regproc_t	__reg_get_hook;
    gdb_regproc_t	__reg_set_hook;

    /*
     * An application may extend the gdb remote protocol by
     * installing hooks to handle unknown general query and
     * set packets ("q" pkts and 'Q' pkts) with these two hooks.
     */
    void                (*__pkt_query_hook)(unsigned char *__pkt);
    void                (*__pkt_set_hook)(unsigned char *__pkt);

    /*
     * An application may also extend the gdb remote protocol
     * by installing a hook to handle all unknown packets.
     */
    void                (*__pkt_hook)(unsigned char *__pkt);

    /*
     * The above hooks for receiving packets will probably need
     * a mechanism to respond. This vector is provided to allow
     * an application to append data to the outgoing packet which
     * will be sent after the above hooks are called.
     *
     * This vector uses a printf-like format string followed by
     * some number of arguments.
     */
    void                (*__pkt_append)(char *fmt, ...);

    /*
     * An application can read/write from/to gdb console
     * through these vectors.
     *
     * NB: console read is not supported and will block forever.
     */
    int                 (*__console_read)(char *__buf, int len);
    int                 (*__console_write)(char *__buf, int len);

} gdb_data_t;


extern gdb_data_t *__get_gdb_data(void);

#endif /* __ASSEMBLER__ */

#endif // __BSP_GDB_DATA_H__
