#ifndef CYGONCE_HAL_PC_SER_H
#define CYGONCE_HAL_PC_SER_H
//=============================================================================
//
//      pc87338.c
//
//      Simple driver for the serial controllers in the PC87338 SuperIO chip
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
// Date:        2000-06-20
// Description: Simple driver for the PC87338 serial controllers
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

//-----------------------------------------------------------------------------
// Controller definitions.

//-----------------------------------------------------------------------------
// There are two serial ports.
#define CYG_DEVICE_SERIAL_SCC1    0xb40003f8 // port 1
#define CYG_DEVICE_SERIAL_SCC2    0xb40002f8 // port 2

//-----------------------------------------------------------------------------
// Serial registers (shared by all banks)
#define CYG_DEVICE_BSR       (0x03)
#define CYG_DEVICE_LCR       (0x03)

#define CYG_DEVICE_BSR_BANK0 0x00
#define CYG_DEVICE_BSR_BANK2 0xe0

#define CYG_DEVICE_LCR_LEN_5BIT       0x00
#define CYG_DEVICE_LCR_LEN_6BIT       0x01
#define CYG_DEVICE_LCR_LEN_7BIT       0x02
#define CYG_DEVICE_LCR_LEN_8BIT       0x03
#define CYG_DEVICE_LCR_STOP_1         0x00
#define CYG_DEVICE_LCR_STOP_2         0x04
#define CYG_DEVICE_LCR_PARITY_NONE    0x00
#define CYG_DEVICE_LCR_PARITY_ODD     0x08
#define CYG_DEVICE_LCR_PARITY_EVEN    0x18
#define CYG_DEVICE_LCR_PARITY_LOGIC1  0x28
#define CYG_DEVICE_LCR_PARITY_LOGIC0  0x38
#define CYG_DEVICE_LCR_SBRK           0x40

// Bank 0 (control/status)
#define CYG_DEVICE_BK0_TXD   (0x00)
#define CYG_DEVICE_BK0_RXD   (0x00)
#define CYG_DEVICE_BK0_IER   (0x01)
#define CYG_DEVICE_BK0_EIR   (0x02)
#define CYG_DEVICE_BK0_FCR   (0x02)
#define CYG_DEVICE_BK0_MCR   (0x04)
#define CYG_DEVICE_BK0_LSR   (0x05)
#define CYG_DEVICE_BK0_MSR   (0x06)
#define CYG_DEVICE_BK0_SPR   (0x07)
#define CYG_DEVICE_BK0_ASCR  (0x07)

#define CYG_DEVICE_BK0_LSR_RXDA       0x01
#define CYG_DEVICE_BK0_LSR_OE         0x02
#define CYG_DEVICE_BK0_LSR_PE         0x04
#define CYG_DEVICE_BK0_LSR_FE         0x08
#define CYG_DEVICE_BK0_LSR_BRK        0x10
#define CYG_DEVICE_BK0_LSR_TXRDY      0x20
#define CYG_DEVICE_BK0_LSR_TXEMP      0x40
#define CYG_DEVICE_BK0_LSR_ER_INF     0x80

#define CYG_DEVICE_BK0_IER_TMR_IE     0x80
#define CYG_DEVICE_BK0_IER_SFIF_IE    0x40
#define CYG_DEVICE_BK0_IER_TXEMP_IE   0x20
#define CYG_DEVICE_BK0_IER_DMA_IE     0x10
#define CYG_DEVICE_BK0_IER_MS_IE      0x08
#define CYG_DEVICE_BK0_IER_LS_IE      0x04
#define CYG_DEVICE_BK0_IER_TXLDL_IE   0x02
#define CYG_DEVICE_BK0_IER_RXHDL_IE   0x01

#define CYG_DEVICE_BK0_EIR_FEN1       0x80
#define CYG_DEVICE_BK0_EIR_FEN0       0x40
#define CYG_DEVICE_BK0_EIR_RXFT       0x08
#define CYG_DEVICE_BK0_EIR_IPR1       0x04
#define CYG_DEVICE_BK0_EIR_IPR0       0x02
#define CYG_DEVICE_BK0_EIR_IPF        0x01

#define CYG_DEVICE_BK0_EIR_mask       0x07
#define CYG_DEVICE_BK0_EIR_IRQ_ERR    0x06
#define CYG_DEVICE_BK0_EIR_IRQ_RX     0x04
#define CYG_DEVICE_BK0_EIR_IRQ_TX     0x02

#define CYG_DEVICE_BK0_MCR_ISEN       0x08 // interrupt signal enable


// Bank 2 (baud generator)
#define CYG_DEVICE_BK2_BGDL  (0x00)
#define CYG_DEVICE_BK2_BGDH  (0x01)
#define CYG_DEVICE_BK2_EXCR1 (0x02)
#define CYG_DEVICE_BK2_EXCR2 (0x04)
#define CYG_DEVICE_BK2_TXFLV (0x06)
#define CYG_DEVICE_BK2_RXFLV (0x07)


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//-----------------------------------------------------------------------------
// The minimal init, get and put functions. All by polling.

void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lcr;

    HAL_WRITE_UINT8(base+CYG_DEVICE_BSR, CYG_DEVICE_BSR_BANK0);
    HAL_WRITE_UINT8(base+CYG_DEVICE_BK0_IER, 0);
    HAL_WRITE_UINT8(base+CYG_DEVICE_BK0_MCR, CYG_DEVICE_BK0_MCR_ISEN);

    // Disable FIFOs
    HAL_WRITE_UINT8(base+CYG_DEVICE_BK0_FCR, 0);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEVICE_LCR,
                    CYG_DEVICE_LCR_LEN_8BIT | CYG_DEVICE_LCR_STOP_1 | CYG_DEVICE_LCR_PARITY_NONE);

    // Set speed to 38400 (switch bank, remember old LCR setting)
    HAL_READ_UINT8(base+CYG_DEVICE_LCR, lcr);
    HAL_WRITE_UINT8(base+CYG_DEVICE_BSR, CYG_DEVICE_BSR_BANK2);
    HAL_WRITE_UINT8(base+CYG_DEVICE_BK2_BGDL, 3);
    HAL_WRITE_UINT8(base+CYG_DEVICE_BK2_BGDH, 0);
    HAL_WRITE_UINT8(base+CYG_DEVICE_LCR, lcr);
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 __ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+CYG_DEVICE_BK0_LSR, lsr);
    } while ((lsr & CYG_DEVICE_BK0_LSR_TXRDY) == 0);

    HAL_WRITE_UINT8(base+CYG_DEVICE_BK0_TXD, __ch);

    // Hang around until the character has been safely sent.
    do {
        HAL_READ_UINT8(base+CYG_DEVICE_BK0_LSR, lsr);
    } while ((lsr & CYG_DEVICE_BK0_LSR_TXRDY) == 0);

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;

    HAL_READ_UINT8(base+CYG_DEVICE_BK0_LSR, lsr);
    if ((lsr & CYG_DEVICE_BK0_LSR_RXDA) == 0)
        return false;

    HAL_READ_UINT8 (base+CYG_DEVICE_BK0_RXD, *ch);

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static channel_data_t channels[2] = {
    { (cyg_uint8*)CYG_DEVICE_SERIAL_SCC1, 1000, CYGNUM_HAL_INTERRUPT_DEBUG_UART},
    { (cyg_uint8*)CYG_DEVICE_SERIAL_SCC2, 1000, CYGNUM_HAL_INTERRUPT_USER_UART}
};

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}


cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

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
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 ier;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        HAL_READ_UINT8(chan->base+CYG_DEVICE_BK0_IER, ier);
        ier |= CYG_DEVICE_BK0_IER_RXHDL_IE;
        HAL_WRITE_UINT8(chan->base+CYG_DEVICE_BK0_IER, ier);

        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        HAL_READ_UINT8(chan->base+CYG_DEVICE_BK0_IER, ier);
        ier &= ~CYG_DEVICE_BK0_IER_RXHDL_IE;
        HAL_WRITE_UINT8(chan->base+CYG_DEVICE_BK0_IER, ier);

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
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    cyg_uint8 eir, c;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    CYGARC_HAL_SAVE_GP();

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    HAL_READ_UINT8(chan->base+CYG_DEVICE_BK0_EIR, eir);

    *__ctrlc = 0;
    if( (eir & CYG_DEVICE_BK0_EIR_mask) == CYG_DEVICE_BK0_EIR_IRQ_RX ) {

        HAL_READ_UINT8(chan->base+CYG_DEVICE_BK0_RXD, c);
    
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Disable interrupts.
    HAL_INTERRUPT_MASK(channels[0].isr_vector);
    HAL_INTERRUPT_MASK(channels[1].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel((void*)&channels[0]);
    cyg_hal_plf_serial_init_channel((void*)&channels[1]);
    
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

    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
     
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
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
// end of pc87338.c
#endif // CYGONCE_HAL_PC_SER_INL
