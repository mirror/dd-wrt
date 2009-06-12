//=============================================================================
//
//      plf_stub.c
//
//      Platform specific code for GDB stub support.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, jskov (based on the old tx39 hal_stub.c)
// Contributors:nickg, jskov
// Date:        1999-02-12
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_io.h>             // HAL IO macros
#include <cyg/hal/hal_diag.h>           // diag output. FIXME

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/plf_z8530.h>

/*---------------------------------------------------------------------------*/

static unsigned char _diag_init[] = {
    0x00, /* Register 0 */
    0x00, /* Register 1 - no interrupts */
    0x00, /* Register 2 */
    0xC1, /* Register 3 - Rx enable, 8 data */
    0x44, /* Register 4 - x16 clock, 1 stop, no parity */
    0x68, /* Register 5 - Tx enable, 8 data */
    0x00, /* Register 6 */
    0x00, /* Register 7 */
    0x00, /* Register 8 */
    0x00, /* Register 9 */
    0x00, /* Register 10 */
    0x56, /* Register 11 - Rx, Tx clocks from baud rate generator */
    0x00, /* Register 12 - baud rate LSB */
    0x00, /* Register 13 - baud rate MSB */
    0x03, /* Register 14 - enable baud rate generator */
    0x00  /* Register 15 */
};

#define BRTC(brate) (( ((unsigned) DUART_CLOCK) / (2*(brate)*SCC_CLKMODE_TC)) - 2)
#define DUART_CLOCK          4915200         /* Z8530 duart */
#define SCC_CLKMODE_TC       16              /* Always run x16 clock for async modes */

//-----------------------------------------------------------------------------

typedef struct {
    cyg_uint32 base;
    cyg_uint32 msec_timeout;
    int isr_vector;
} channel_data_t;

static channel_data_t channels[2] = {
    { DUART_A, 1000, CYGNUM_HAL_INTERRUPT_DUART},
    { DUART_B, 1000, CYGNUM_HAL_INTERRUPT_DUART}
};

//-----------------------------------------------------------------------------
// Set the baud rate

static void
cyg_hal_plf_serial_set_baud(cyg_uint32 duart, cyg_uint16 baud_rate)
{
    unsigned short brg = BRTC(baud_rate);
    HAL_DUART_WRITE_CR(duart, 12, brg&0xFF);    
    HAL_DUART_WRITE_CR(duart, 13, brg>>8);    
}

//-----------------------------------------------------------------------------
// The minimal init, get and put functions. All by polling.

void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint32 duart;
    unsigned short brg = BRTC(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
    int i;
    channel_data_t *chan;
    
    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
        __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

#ifdef CYGPRI_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL_CONFIGURABLE    
    if( (chan-&channels[0]) == CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL )
        brg = BRTC(CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL_BAUD);
#endif    
#ifdef CYGPRI_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_CONFIGURABLE
    if( (chan-&channels[0]) == CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL )
        brg = BRTC(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
#endif
    
    duart = chan->base;

    _diag_init[12] = brg & 0xFF;
    _diag_init[13] = brg >> 8;
    for (i = 1;  i < 16;  i++) {
        HAL_DUART_WRITE_CR(duart, i, _diag_init[i]);
    }
    
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 __ch)
{
    cyg_uint32 duart;
    cyg_uint8 rr0;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    duart = ((channel_data_t*)__ch_data)->base;

    CYGARC_HAL_SAVE_GP();

    do
    {
        HAL_DUART_READ_CR(duart, 0, rr0 );
    } while( (rr0 & 0x04) == 0 );

    HAL_DUART_WRITE_TR( duart, __ch );

    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_DUART );    

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint32 duart;
    cyg_uint8 rr0;

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    duart = ((channel_data_t*)__ch_data)->base;

    HAL_DUART_READ_CR(duart, 0, rr0 );    

    if( (rr0 & 0x01) == 0 )
        return false;

    HAL_DUART_READ_RR( duart, *ch );

    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_DUART );    
        
    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}


cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}


static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        HAL_DUART_WRITE_CR( chan->base, 1, 0x10 );
        HAL_DUART_WRITE_CR( chan->base, 9, 0x0a );

        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 0);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_DUART_WRITE_CR( chan->base, 1, 0x00 );
        HAL_DUART_WRITE_CR( chan->base, 9, 0x00 );

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    break;
    case __COMMCTL_SETBAUD:
    {
        cyg_uint32 baud_rate;
        cyg_uint32 duart = chan->base;
        va_list ap;

        va_start(ap, __func);
        baud_rate = va_arg(ap, cyg_uint32);
        va_end(ap);

        // Set baud rate.
        cyg_hal_plf_serial_set_baud(duart, baud_rate);

    }
    break;

    case __COMMCTL_GETBAUD:
        break;
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan;
    char c;
    
    CYGARC_HAL_SAVE_GP();

    // Some of the diagnostic print code calls through here with no idea what the ch_data is.
    // Go ahead and assume it is channels[0].
    if (__ch_data == 0)
      __ch_data = (void*)&channels[0];

    chan = (channel_data_t*)__ch_data;

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    *__ctrlc = 0;
    
    if ( cyg_hal_plf_serial_getc_nonblock(__ch_data, &c) )
    {
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }
    
    CYGARC_HAL_RESTORE_GP();
    
    return res;
}

#endif

static void
cyg_hal_plf_serial_init(void)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT    
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT); 
#endif
    // Disable interrupts.
    HAL_INTERRUPT_MASK(channels[0].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel((void*)&channels[0]);

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT    
    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
#endif    
}


void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//-----------------------------------------------------------------------------
// End of plf_serial.c
