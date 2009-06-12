//=============================================================================
//
//      sci.c
//
//      Simple driver for the H8/300 Serial Communication Interface (SCI)
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
// Author(s):   ysato
// Contributors:ysato
// Date:        2002-03-21
// Description: Simple driver for the H8/300 Serial Communication Interface
//              Clients of this file can configure the behavior with:
//              CYGNUM_SCI_PORTS:  number of SCI ports
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGNUM_HAL_H8300_SCI_PORTS

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP
#include <cyg/hal/hal_if.h>             // Calling-if API
#include <cyg/hal/mod_regs_sci.h>       // serial register definitions

#include <cyg/hal/h8_sci.h>             // our header

//--------------------------------------------------------------------------

void
cyg_hal_plf_sci_init_channel(void* chan)
{
    cyg_uint8 tmp;
    cyg_uint8* base = ((channel_data_t *)chan)->base;

    // Disable Tx/Rx interrupts, but enable Tx/Rx
    HAL_WRITE_UINT8(base+_REG_SCSCR,
                    CYGARC_REG_SCSCR_TE|CYGARC_REG_SCSCR_RE);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+_REG_SCSMR, 0);

    // Set speed to CYGNUM_HAL_H8300_SCI_DEFAULT_BAUD_RATE
    HAL_READ_UINT8(base+_REG_SCSMR, tmp);
    tmp &= ~CYGARC_REG_SCSMR_CKSx_MASK;
    tmp |= CYGARC_SCBRR_CKSx(CYGNUM_HAL_H8300_SCI_BAUD_RATE);
    HAL_WRITE_UINT8(base+_REG_SCSMR, tmp);
    HAL_WRITE_UINT8(base+_REG_SCBRR, CYGARC_SCBRR_N(CYGNUM_HAL_H8300_SCI_BAUD_RATE));
}

static cyg_bool
cyg_hal_plf_sci_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 sr;

    HAL_READ_UINT8(base+_REG_SCSSR, sr);
    if (sr & CYGARC_REG_SCSSR_ORER) {
        // Serial RX overrun. Clear error and let caller try again.
        HAL_WRITE_UINT8(base+_REG_SCSSR, 
                        CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_ORER);
        return false;
    }

    if ((sr & CYGARC_REG_SCSSR_RDRF) == 0)
        return false;

    HAL_READ_UINT8(base+_REG_SCRDR, *ch);

    // Clear buffer full flag.
    HAL_WRITE_UINT8(base+_REG_SCSSR, sr & ~CYGARC_REG_SCSSR_RDRF);
    return true;
}

cyg_uint8
cyg_hal_plf_sci_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_sci_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

void
cyg_hal_plf_sci_putc(void* __ch_data, cyg_uint8 c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 sr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+_REG_SCSSR, sr);
    } while ((sr & CYGARC_REG_SCSSR_TDRE) == 0);

    HAL_WRITE_UINT8(base+_REG_SCTDR, c);

    // Clear empty flag.
    HAL_WRITE_UINT8(base+_REG_SCSSR, sr & ~CYGARC_REG_SCSSR_TDRE);

    // Hang around until the character has been safely sent.
    do {
        HAL_READ_UINT8(base+_REG_SCSSR, sr);
    } while ((sr & CYGARC_REG_SCSSR_TDRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}


static channel_data_t channels[CYGNUM_HAL_H8300_SCI_PORTS];

static void
cyg_hal_plf_sci_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_sci_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_sci_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_sci_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_sci_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int delay_count;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 20; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_sci_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(50);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_sci_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 scr;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
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
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_sci_isr(void *__ch_data, int* __ctrlc, 
                    CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    cyg_uint8 c, sr;
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    HAL_READ_UINT8(base+_REG_SCSSR, sr);
    if (sr & CYGARC_REG_SCSSR_ORER) {
        // Serial RX overrun. Clear error and hope protocol recovers.
        HAL_WRITE_UINT8(base+_REG_SCSSR, 
                        CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_ORER);
        res = CYG_ISR_HANDLED;
    } else if (sr & CYGARC_REG_SCSSR_RDRF) {
        // Received character
        HAL_READ_UINT8(base+_REG_SCRDR, c);

        // Clear buffer full flag.
        HAL_WRITE_UINT8(base+_REG_SCSSR, 
                        CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_RDRF);

        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

void
cyg_hal_plf_sci_init(int sci_index, int comm_index, 
                     int rcv_vect, cyg_uint8* base)
{
    channel_data_t* chan = &channels[sci_index];
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Initialize channel table
    chan->base = base;
    chan->isr_vector = rcv_vect;
    chan->msec_timeout = 1000;

    // Disable interrupts.
    HAL_INTERRUPT_MASK(chan->isr_vector);

    // Init channel
    
    cyg_hal_plf_sci_init_channel(chan);

    // Setup procs in the vector table

    // Initialize channel procs
    CYGACC_CALL_IF_SET_CONSOLE_COMM(comm_index);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, chan);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_sci_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_sci_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_sci_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_sci_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_sci_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_sci_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_sci_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

#endif // CYGNUM_HAL_H8300_SCI_PORTS

//-----------------------------------------------------------------------------
// end of sci.c
