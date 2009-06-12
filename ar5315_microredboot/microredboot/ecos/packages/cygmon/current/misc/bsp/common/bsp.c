//==========================================================================
//
//      bsp.c
//
//      General BSP support.
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
// Purpose:      General BSP support. 
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <stdlib.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "gdb.h"

#ifndef DEBUG_BSP_INIT
#define DEBUG_BSP_INIT 0
#endif

struct bsp_comm_channel *_bsp_net_channel = NULL;

/*
 *  This is the array of pointers to interrupt controller descriptors.
 */
static struct bsp_irq_controller *_bsp_ictrl_table[BSP_MAX_IRQ_CONTROLLERS];

/*
 *  This is the array of second-level exception vectors.
 */
static bsp_vec_t *_bsp_exc_table[BSP_MAX_EXCEPTIONS];


/*
 * Debug (default exception) handler vector.
 */
bsp_handler_t _bsp_dbg_vector;


static bsp_shared_t __bsp_shared = {
    BSP_SHARED_DATA_VERSION,                                  /* version         */
    (const struct bsp_irq_controller **)&_bsp_ictrl_table[0], /* __ictrl_table   */
    &_bsp_exc_table[0],                                       /* __exc_table     */
    &_bsp_dbg_vector,                                         /* __dbg_vector    */
    (bsp_handler_t)0,                                         /* __kill_vector   */
    (struct bsp_comm_procs *)NULL,                            /* __console_procs */
    (struct bsp_comm_procs *)NULL,                            /* __debug_procs   */
    (void (*)(void *, int      ))NULL,                        /* __flush_dcache  */
    (void (*)(void *, int      ))NULL,                        /* __flush_icache  */
    (void (*)(void             ))NULL,                        /* __reset         */
    (void*)NULL,                                              /* __cpu_data      */
    (void*)NULL                                               /* __board_data    */
};


static void
default_cache_proc(void *p, int nbytes)
{
    /* do nothing */
}


static void
default_reset_proc(void)
{
    /* do nothing */
}


static int
find_comm_id(struct bsp_comm_procs *procs)
{
    int i;

    for (i = 0; i < _bsp_num_comms; i++)
	if (&_bsp_comm_list[i].procs == procs)
	    return i;

    if (procs == &_bsp_net_channel->procs)
	return _bsp_num_comms;

    return -1;
}


/*
 * Set or get active debug channel.
 * Returns -1 if unsucessful.
 * If the passed in comm id is -1, then the id of the current channel
 * is returned.
 */
static int
set_debug_comm(int id)
{
    struct bsp_comm_channel *chan;
    struct bsp_comm_procs *procs;
    int  current_chan = find_comm_id(bsp_shared_data->__debug_procs);

    if (id < 0)
	return current_chan;

    if (id > _bsp_num_comms)
	return -1;

    if (id == _bsp_num_comms && _bsp_net_channel == NULL)
	return -1;

    if (id == current_chan)
	return 0;

    /* Remove existing channel */
    if ((procs = bsp_shared_data->__debug_procs) != NULL)
	(*procs->__control)(procs->ch_data, COMMCTL_REMOVE_DBG_ISR);
    
    /* Install new channel */
    if (id == _bsp_num_comms)
	chan = _bsp_net_channel;
    else
	chan = &_bsp_comm_list[id];
    bsp_shared_data->__debug_procs = &chan->procs;
    (*chan->procs.__control)(chan->procs.ch_data, COMMCTL_INSTALL_DBG_ISR);

    return 0;
}


/*
 * Set or get active console channel.
 * Returns -1 if unsucessful.
 * If the passed in comm id is -1, then the id of the current channel
 * is returned.
 */
static int
set_console_comm(int id)
{
    int  current_chan = find_comm_id(bsp_shared_data->__console_procs);

    if (id < 0)
	return current_chan;

    if (id > _bsp_num_comms)
	return -1;

    if (id == _bsp_num_comms && _bsp_net_channel == NULL)
	return -1;

    if (id == current_chan)
	return 0;

    /*
     * Install new channel. If its the same as the debug channel,
     * just clear the __console_procs and the bsp_console_*
     * interface functions will take care of the rest.
     */
    if (id == _bsp_num_comms)
	bsp_shared_data->__console_procs = &_bsp_net_channel->procs;
    else
	bsp_shared_data->__console_procs = &_bsp_comm_list[id].procs;

    if (bsp_shared_data->__console_procs == bsp_shared_data->__debug_procs)
	bsp_shared_data->__console_procs = NULL;

    return 0;
}


/*
 * Set or get the current baud rate of a serial comm channel.
 * Returns -1 on if unsuccessful.
 * If the given baud is -1, then the current baudrate is returned.
 */
int
set_serial_baud(int id, int baud)
{
    struct bsp_comm_channel *chan;

    if (id < 0 || id >= _bsp_num_comms)
	return -1;

    chan = &_bsp_comm_list[id];

    if (chan->info.kind != BSP_COMM_SERIAL)
	return -1;

    if (baud == -1)
	return (*chan->procs.__control)(chan->procs.ch_data,
					COMMCTL_GETBAUD);

    return (*chan->procs.__control)(chan->procs.ch_data,
				    COMMCTL_SETBAUD, baud);
}

/*
 *  Final initialization before calling main.
 */
void
_bsp_init(void)
{
    struct bsp_comm_procs *com;
    extern void __init_irq_controllers(void);

    bsp_shared_data = &__bsp_shared;
    _bsp_dbg_vector = (bsp_handler_t)_bsp_gdb_handler;
    bsp_shared_data->__dbg_data = &_bsp_gdb_data;

    bsp_shared_data->__flush_dcache = default_cache_proc;
    bsp_shared_data->__flush_icache = default_cache_proc;

    bsp_shared_data->__reset = default_reset_proc;

    /*
     * General BSP information access.
     */
    bsp_shared_data->__sysinfo = _bsp_sysinfo;

    /*
     * Setup comm port handlers.
     */
    bsp_shared_data->__set_debug_comm = set_debug_comm;
    bsp_shared_data->__set_console_comm = set_console_comm;
    bsp_shared_data->__set_serial_baud = set_serial_baud;

    /*
     *  Very first thing is to initialize comm channels so
     *  we can have debug printfs working. None of this
     *  must rely on interrupts until interrupt controllers
     *  are initialized below.
     */
    _bsp_init_board_comm();

    /*
     * Assume first comm channel is the default.
     */
    bsp_shared_data->__debug_procs = &_bsp_comm_list[0].procs;

    /*
     * By default, console i/o goes through the debug channel.
     * We indicate this by making the console i/o procs
     * pointer NULL.
     */
    bsp_shared_data->__console_procs = NULL;

    /*
     * Install interrupt controllers.
     */
#if DEBUG_BSP_INIT
    bsp_printf("Installing interrupt controllers...\n");
#endif

    _bsp_install_cpu_irq_controllers();
    _bsp_install_board_irq_controllers();

#if DEBUG_BSP_INIT
    bsp_printf("Done installing interrupt controllers.\n");
    bsp_printf("Initializing interrupt controllers...\n");
#endif
    /*
     *  Actually run the init routines for all installed
     *  interrupt controllers.
     */
    __init_irq_controllers();

#if DEBUG_BSP_INIT
    bsp_printf("Done initializing interrupt controllers.\n");
    bsp_printf("CPU-specific initialization...\n");
#endif

    /*
     *  Final architecture specific initialization.
     */
    _bsp_cpu_init();

#if DEBUG_BSP_INIT
    bsp_printf("Done w/ CPU-specific initialization.\n");
    bsp_printf("Board specific initialization...\n");
#endif
    /*
     *  Final board specific initialization.
     */
    _bsp_board_init();

#if DEBUG_BSP_INIT
    bsp_printf("Done w/ board specific initialization.\n");
#endif

    /*
     * Now we can install the debug interrupt handler on the debug channel.
     */
    com = bsp_shared_data->__debug_procs;
    (*com->__control)(com->ch_data, COMMCTL_INSTALL_DBG_ISR);


    if (_bsp_net_channel != NULL) {
	set_debug_comm(_bsp_num_comms);
	set_console_comm(0);
    }
}
