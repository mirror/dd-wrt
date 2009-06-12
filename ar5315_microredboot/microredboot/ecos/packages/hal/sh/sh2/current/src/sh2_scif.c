//=============================================================================
//
//      sh2_scif.c
//
//      Simple driver for the SH2 Serial Communication Interface with FIFO
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-01-16
// Description: Simple driver for the SH Serial Communication Interface
//              The driver can be used for either the SCIF or the IRDA
//              modules (the latter can act as the former).
//              Clients of this file can configure the behavior with:
//              CYGNUM_SCIF_PORTS: number of SCI ports
//
// Note:        It should be possible to configure a channel to IRDA mode.
//              Worry about that when some board needs it.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGNUM_HAL_SH_SH2_SCIF_PORTS

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP
#include <cyg/hal/hal_if.h>             // Calling-if API
#include <cyg/hal/sh_regs.h>            // serial register definitions
#include <cyg/hal/sh_stub.h>            // target_register_t

#define CYGPRI_HAL_SH_SH2_SCIF_PRIVATE
#include <cyg/hal/sh2_scif.h>           // our header

//--------------------------------------------------------------------------

void
cyg_hal_plf_scif_init_channel(channel_data_t* chan)
{
    cyg_uint8* base = chan->base;
    cyg_uint8 tmp;
    cyg_uint16 sr;
    int baud_rate = CYGNUM_HAL_SH_SH2_SCIF_BAUD_RATE;

    // Disable everything.
    HAL_WRITE_UINT8(base+_REG_SCSCR, 0);

    // Reset FIFO.
    HAL_WRITE_UINT8(base+_REG_SCFCR, 
                    CYGARC_REG_SCIF_SCFCR_TFRST|CYGARC_REG_SCIF_SCFCR_RFRST);
    HAL_WRITE_UINT16(base+_REG_SCFER, 0);

    // 8-1-no parity. This is also fine for IrDA mode
    HAL_WRITE_UINT8(base+_REG_SCSMR, 0);
    if (chan->irda_mode)
        HAL_WRITE_UINT8(base+_REG_SCIMR, CYGARC_REG_SCIF_SCIMR_IRMOD);
    else {
        HAL_WRITE_UINT8(base+_REG_SCIMR, 0);
    }

    // Set speed to CYGNUM_HAL_SH_SH2_SCIF_DEFAULT_BAUD_RATE
    HAL_READ_UINT8(base+_REG_SCSMR, tmp);
    tmp &= ~CYGARC_REG_SCIF_SCSMR_CKSx_MASK;
    tmp |= CYGARC_SCBRR_CKSx(baud_rate);
    HAL_WRITE_UINT8(base+_REG_SCSMR, tmp);
    HAL_WRITE_UINT8(base+_REG_SCBRR, CYGARC_SCBRR_N(baud_rate));

    // Let things settle: Here we should should wait the equivalent of
    // one bit interval,
    // i.e. 1/CYGNUM_HAL_SH_SH2_SCIF_DEFAULT_BAUD_RATE second, but
    // until we have something like the Linux delay loop, it's hard to
    // do reliably. So just move on and hope for the best (this is
    // unlikely to cause problems since the CPU has just come out of
    // reset anyway).

    // Clear status register (read back first).
    HAL_READ_UINT16(base+_REG_SCSSR, sr);
    HAL_WRITE_UINT16(base+_REG_SCSSR, 0);

    HAL_WRITE_UINT8(base+_REG_SC2SSR, CYGARC_REG_SCIF_SC2SSR_BITRATE_16|CYGARC_REG_SCIF_SC2SSR_EI);

    // Bring FIFO out of reset and set to trigger on every char in
    // FIFO (or C-c input would not be processed).
    HAL_WRITE_UINT8(base+_REG_SCFCR, 
                    CYGARC_REG_SCIF_SCFCR_RTRG_1|CYGARC_REG_SCIF_SCFCR_TTRG_1);

    // Leave Tx/Rx interrupts disabled, but enable Rx/Tx (only Rx for IrDA)
    if (chan->irda_mode)
        HAL_WRITE_UINT8(base+_REG_SCSCR, CYGARC_REG_SCIF_SCSCR_RE);
#ifdef CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX
    else if (chan->async_rxtx_mode)
        HAL_WRITE_UINT8(base+_REG_SCSCR, CYGARC_REG_SCIF_SCSCR_RE);
#endif 
    else
        HAL_WRITE_UINT8(base+_REG_SCSCR, CYGARC_REG_SCIF_SCSCR_TE|CYGARC_REG_SCIF_SCSCR_RE);
}

//static 
cyg_bool
cyg_hal_plf_scif_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint16 fdr, sr;
    cyg_bool res = false;

    HAL_READ_UINT16(base+_REG_SCSSR, sr);
    if (sr & CYGARC_REG_SCIF_SCSSR_ER) {
        cyg_uint8 ssr2;
        HAL_WRITE_UINT16(base+_REG_SCFER, 0);
        HAL_READ_UINT8(base+_REG_SC2SSR, ssr2);
        ssr2 &= ~CYGARC_REG_SCIF_SC2SSR_ORER;
        HAL_WRITE_UINT8(base+_REG_SC2SSR, ssr2);
        HAL_WRITE_UINT16(base+_REG_SCSSR,
                         CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~(CYGARC_REG_SCIF_SCSSR_BRK | CYGARC_REG_SCIF_SCSSR_FER | CYGARC_REG_SCIF_SCSSR_PER));
    }


    HAL_READ_UINT16(base+_REG_SCFDR, fdr);
    if (0 != (fdr & CYGARC_REG_SCIF_SCFDR_RCOUNT_MASK)) {

        HAL_READ_UINT8(base+_REG_SCFRDR, *ch);

        // Clear DR/RDF flags
        HAL_READ_UINT16(base+_REG_SCSSR, sr);
        HAL_WRITE_UINT16(base+_REG_SCSSR,
                         CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~(CYGARC_REG_SCIF_SCSSR_RDF | CYGARC_REG_SCIF_SCSSR_DR));

        res = true;
    }

    return res;
}

cyg_uint8
cyg_hal_plf_scif_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_scif_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

void
cyg_hal_plf_scif_putc(void* __ch_data, cyg_uint8 c)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;
    cyg_uint16 fdr, sr;
    cyg_uint8 scscr = 0;
    CYGARC_HAL_SAVE_GP();

    HAL_READ_UINT8(base+_REG_SCSCR, scscr);
    if (chan->irda_mode) {
        HAL_WRITE_UINT8(base+_REG_SCSCR, scscr|CYGARC_REG_SCIF_SCSCR_TE);
    }
#ifdef CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX
    if (chan->async_rxtx_mode) {
        HAL_WRITE_UINT8(base+_REG_SCSCR, (scscr|CYGARC_REG_SCIF_SCSCR_TE)&~CYGARC_REG_SCIF_SCSCR_RE);
    }
#endif

    do {
        HAL_READ_UINT16(base+_REG_SCFDR, fdr);
    } while (((fdr & CYGARC_REG_SCIF_SCFDR_TCOUNT_MASK) >> CYGARC_REG_SCIF_SCFDR_TCOUNT_shift) == 16);

    HAL_WRITE_UINT8(base+_REG_SCFTDR, c);

    // Clear FIFO-empty/transmit end flags (read back SR first)
    HAL_READ_UINT16(base+_REG_SCSSR, sr);
    HAL_WRITE_UINT16(base+_REG_SCSSR, CYGARC_REG_SCIF_SCSSR_CLEARMASK   
                     & ~(CYGARC_REG_SCIF_SCSSR_TDFE | CYGARC_REG_SCIF_SCSSR_TEND ));

    // Hang around until all characters have been safely sent.
    do {
        HAL_READ_UINT16(base+_REG_SCSSR, sr);
    } while ((sr & CYGARC_REG_SCIF_SCSSR_TEND) == 0);


    if (chan->irda_mode) {
#ifdef CYGHWR_HAL_SH_SH2_SCIF_IRDA_TXRX_COMPENSATION
        // In IrDA mode there will be generated spurious RX events when
        // the TX unit is switched on. Eat that character.
        cyg_uint8 _junk;
        HAL_READ_UINT8(base+_REG_SCFRDR, _junk);

        // Clear buffer full flag (read back first)
        HAL_READ_UINT16(base+_REG_SCSSR, sr);
        HAL_WRITE_UINT16(base+_REG_SCSSR, 
                         CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~(CYGARC_REG_SCIF_SCSSR_RDF|CYGARC_REG_SCIF_SCSSR_DR));
#endif // CYGHWR_HAL_SH_SH2_SCIF_IRDA_TXRX_COMPENSATION
        // Disable transmitter again
        HAL_WRITE_UINT8(base+_REG_SCSCR, scscr);
    }
#ifdef CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX
    if (chan->async_rxtx_mode) {
        // Disable transmitter, enable receiver
        HAL_WRITE_UINT8(base+_REG_SCSCR, scscr);
    }
#endif // CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX

    CYGARC_HAL_RESTORE_GP();
}


static channel_data_t channels[CYGNUM_HAL_SH_SH2_SCIF_PORTS];

static void
cyg_hal_plf_scif_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_scif_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_scif_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_scif_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_scif_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int delay_count;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_scif_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_scif_control(void *__ch_data, __comm_control_cmd_t __func, ...)
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
        HAL_READ_UINT8(chan->base+_REG_SCSCR, scr);
        scr |= CYGARC_REG_SCIF_SCSCR_RIE;
        HAL_WRITE_UINT8(chan->base+_REG_SCSCR, scr);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_READ_UINT8(chan->base+_REG_SCSCR, scr);
        scr &= ~CYGARC_REG_SCIF_SCSCR_RIE;
        HAL_WRITE_UINT8(chan->base+_REG_SCSCR, scr);
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
cyg_hal_plf_scif_isr(void *__ch_data, int* __ctrlc, 
                     CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    cyg_uint8 c;
    cyg_uint16 fdr, sr;
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    HAL_READ_UINT16(base+_REG_SCFDR, fdr);
    if ((fdr & CYGARC_REG_SCIF_SCFDR_RCOUNT_MASK) != 0) {
        HAL_READ_UINT8(base+_REG_SCFRDR, c);

        // Clear buffer full flag (read back first).
        HAL_READ_UINT16(base+_REG_SCSSR, sr);
        HAL_WRITE_UINT16(base+_REG_SCSSR, 
                         CYGARC_REG_SCIF_SCSSR_CLEARMASK & ~CYGARC_REG_SCIF_SCSSR_RDF);

        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

void
cyg_hal_plf_scif_init(int scif_index, int comm_index, 
                      int rcv_vect, cyg_uint8* base, bool irda_mode)
{
    channel_data_t* chan = &channels[scif_index];
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Initialize channel table
    chan->base = base;
    chan->isr_vector = rcv_vect;
    chan->msec_timeout = 1000;
    chan->irda_mode = irda_mode;
#ifdef CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX
    chan->async_rxtx_mode = false;
#endif

    // Disable interrupts.
    HAL_INTERRUPT_MASK(chan->isr_vector);

    // Init channel
    cyg_hal_plf_scif_init_channel(chan);

    // Setup procs in the vector table

    // Initialize channel procs
    CYGACC_CALL_IF_SET_CONSOLE_COMM(comm_index);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, chan);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_scif_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_scif_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_scif_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_scif_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_scif_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_scif_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_scif_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

#ifdef CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX
void
cyg_hal_plf_scif_sync_rxtx(int scif_index, bool async_rxtx_mode)
{
    channel_data_t* chan = &channels[scif_index];
    chan->async_rxtx_mode = async_rxtx_mode;
    if (async_rxtx_mode)
        HAL_WRITE_UINT8(chan->base+_REG_SCSCR, CYGARC_REG_SCIF_SCSCR_RE);
    else
        HAL_WRITE_UINT8(chan->base+_REG_SCSCR, CYGARC_REG_SCIF_SCSCR_RE|CYGARC_REG_SCIF_SCSCR_TE);
}
#endif // CYGHWR_HAL_SH_SH2_SCIF_ASYNC_RXTX

#endif // CYGNUM_HAL_SH_SH2_SCIF_PORTS

//-----------------------------------------------------------------------------
// end of sh2_scif.c
