//==========================================================================
//
//      aim711_misc.c
//
//      HAL misc board support code for ARM Industrial Module AIM 711
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
// Contributors: gthomas, jskov, r.cassebohm
//               Michael Checky <Michael_Checky@ThermoKing.com>
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf()

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_if.h>             // calling interface
#include <cyg/hal/hal_misc.h>           // helper functions
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#include <cyg/hal/drv_api.h>            // HAL ISR support
#endif

#include <pkgconf/system.h>

#ifndef MIN
#define MIN(_x_,_y_) ((_x_) < (_y_) ? (_x_) : (_y_))
#endif
#ifndef MAX
#define MAX(_x_,_y_) ((_x_) > (_y_) ? (_x_) : (_y_))
#endif

//======================================================================
// Use Timer0 for kernel clock

static cyg_uint32 _period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#if 0 // Not supported yet
static cyg_interrupt abort_interrupt;
static cyg_handle_t  abort_interrupt_handle;

// This ISR is called only for the Abort button interrupt
static int
ks32c_abort_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_hal_user_break((CYG_ADDRWORD*)regs);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT2);
    return 0;  // No need to run DSR
}
#endif
#endif

void hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 tmod, clkcon;

    // Disable timer 0
    HAL_READ_UINT32(KS32C_TMOD, tmod);
    tmod &= ~(KS32C_TMOD_TE0);
    HAL_WRITE_UINT32(KS32C_TMOD, 0);

    tmod &= ~(KS32C_TMOD_TMD0 | KS32C_TMOD_TCLR0);
    tmod |= KS32C_TMOD_TE0;

    // Set counter
    HAL_READ_UINT32(KS32C_CLKCON, clkcon);
    period = period/((clkcon & 0xffff) + 1);
    HAL_WRITE_UINT32(KS32C_TDATA0, period);

    // And enable timer
    HAL_WRITE_UINT32(KS32C_TMOD, tmod);

    _period = period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#if 0 // Not supported yet
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT2,
                             99,           // Priority
                             0,            // Data item passed to interrupt handler
                             ks32c_abort_isr,
                             0,
                             &abort_interrupt_handle,
                             &abort_interrupt);
    cyg_drv_interrupt_attach(abort_interrupt_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT2);
#endif
#endif
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;

    HAL_READ_UINT32(KS32C_TCNT0, value);
    *pvalue = _period - value;
}

static void
hal_ks32c_i2c_init(void);

//======================================================================
// Interrupt controller stuff

void hal_hardware_init(void)
{
    cyg_uint32 intmask, syscfg, val;

    // Setup external IO timing
    // Timing for Ext0 is for external 16550 UART, all other are default values
    val = ( KS32C_EXTACON_INIT(1,1,3,0) << KS32C_EXTACON0_EXT0_shift ) \
            | ( KS32C_EXTACON_INIT(1,1,3,1) << KS32C_EXTACON0_EXT1_shift );
    HAL_WRITE_UINT32(KS32C_EXTACON0, val);
    val = ( KS32C_EXTACON_INIT(1,1,3,1) << KS32C_EXTACON1_EXT2_shift ) \
            | ( KS32C_EXTACON_INIT(1,1,3,1) << KS32C_EXTACON1_EXT3_shift );
    HAL_WRITE_UINT32(KS32C_EXTACON1, val);

    // Setup GPIO ports
    HAL_READ_UINT32(KS32C_IOPMOD, val);
    val |= (AIM711_GPIO_DOUT0_DAK0|AIM711_GPIO_DOUT1_DAK1 \
            |AIM711_GPIO_DOUT2_TO0|AIM711_GPIO_DOUT3_TO1 \
            |AIM711_GPIO_POWERLED);
    HAL_WRITE_UINT32(KS32C_IOPMOD, val);

    // Make Power LED on
    AIM711_GPIO_SET(AIM711_GPIO_POWERLED);

    // Enable XIRQ0 for external 16550 UART
    HAL_READ_UINT32(KS32C_IOPCON, val);
    val &= ~( KS32C_IOPCON_XIRQ_MASK << KS32C_IOPCON_XIRQ0_shift );
    val |= ( KS32C_IOPCON_XIRQ_LEVEL|KS32C_IOPCON_XIRQ_AKTIV_HI| \
            KS32C_IOPCON_XIRQ_ENABLE ) << KS32C_IOPCON_XIRQ0_shift ;
    HAL_WRITE_UINT32(KS32C_IOPCON, val);

    // Set up eCos/ROM interfaces
    hal_if_init();

    // Enable cache
    HAL_READ_UINT32(KS32C_SYSCFG, syscfg);
    syscfg &= ~KS32C_SYSCFG_CM_MASK;
    syscfg |= KS32C_SYSCFG_CM_0R_8C|KS32C_SYSCFG_WE;
    HAL_WRITE_UINT32(KS32C_SYSCFG, syscfg);
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();

    // Setup I2C bus
    hal_ks32c_i2c_init();

    // Clear global interrupt mask bit
    HAL_READ_UINT32(KS32C_INTMSK, intmask);
    intmask &= ~KS32C_INTMSK_GLOBAL;
    HAL_WRITE_UINT32(KS32C_INTMSK, intmask);
}

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    // Do hardware-level IRQ handling
    cyg_uint32 irq_status;
    HAL_READ_UINT32(KS32C_INTOFFSET_IRQ, irq_status);
    irq_status = irq_status / 4;
    if (CYGNUM_HAL_ISR_MAX >= irq_status)
        return irq_status;
    // It's a bit bogus to test for FIQs after IRQs, but we use the
    // latter more, so don't impose the overhead of checking for FIQs
    HAL_READ_UINT32(KS32C_INTOFFSET_FIQ, irq_status);
    irq_status = irq_status / 4;
    if (CYGNUM_HAL_ISR_MAX >= irq_status)
        return irq_status;
    return CYGNUM_HAL_INTERRUPT_NONE;
}

// -------------------------------------------------------------------------
//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(KS32C_INTMSK, mask);
    old_mask = mask;
    mask |= (1<<vector);
    HAL_WRITE_UINT32(KS32C_INTMSK, mask);
}

void hal_interrupt_unmask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(KS32C_INTMSK, mask);
    old_mask = mask;
    mask &= ~(1<<vector);
    HAL_WRITE_UINT32(KS32C_INTMSK, mask);
}

void hal_interrupt_acknowledge(int vector)
{
    HAL_WRITE_UINT32(KS32C_INTPND, (1<<vector));
}

void hal_interrupt_configure(int vector, int level, int up)
{
}

void hal_interrupt_set_level(int vector, int level)
{
}

void hal_show_IRQ(int vector, int data, int handler)
{
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 count;
    cyg_uint32 ticks = ((CYGNUM_HAL_RTC_PERIOD*CYGNUM_HAL_RTC_DENOMINATOR)/1000000) * usecs;
    cyg_uint32 tmod;

    // Disable timer 1
    HAL_READ_UINT32(KS32C_TMOD, tmod);
    tmod &= ~(KS32C_TMOD_TE1);
    HAL_WRITE_UINT32(KS32C_TMOD, tmod);

    tmod &= ~(KS32C_TMOD_TMD1 | KS32C_TMOD_TCLR1);
    tmod |= KS32C_TMOD_TE1;

    // Clear pending flag
    HAL_WRITE_UINT32(KS32C_INTPND, (1 << CYGNUM_HAL_INTERRUPT_TIMER1));

    // Set counter
    HAL_WRITE_UINT32(KS32C_TDATA1, ticks);

    // And enable timer
    HAL_WRITE_UINT32(KS32C_TMOD, tmod);

    // Wait for timer to underflow. Can't test the timer completion
    // bit without actually enabling the interrupt. So instead watch
    // the counter.
    ticks /= 2;                         // wait for this threshold

    // Wait till timer counts below threshold
    do {
        HAL_READ_UINT32(KS32C_TCNT1, count);
    } while (count >= ticks);
    // then wait for it to be reloaded
    do {
        HAL_READ_UINT32(KS32C_TCNT1, count);
    } while (count < ticks);

    // Then disable timer 1 again
    tmod &= ~KS32C_TMOD_TE1;
    HAL_WRITE_UINT32(KS32C_TMOD, tmod);
}

// -------------------------------------------------------------------------
//
// To reset the AIM 711, set P3 to low, which is connected to the reset
// logic 
//
void hal_reset(void)
{
    cyg_uint32 value;
    CYG_INTERRUPT_STATE old;

    CYGACC_CALL_IF_DELAY_US(100000);

    // Set P3 to output
    HAL_READ_UINT32(KS32C_IOPMOD, value);
    value |= AIM711_GPIO_RESET;
    HAL_WRITE_UINT32(KS32C_IOPMOD, value);

    // Set P3 to low
    AIM711_GPIO_CLR(AIM711_GPIO_RESET);

    HAL_DISABLE_INTERRUPTS(old);
    while (1)
    ;
}

//----------------------------------------------------------------------
//
// I2C Support
//

#include <string.h>
#include <cyg/hal/drv_api.h>

#ifdef CYGPKG_ERROR
#include <errno.h>
#define I2C_STATUS_SUCCESS      (ENOERR)
#define I2C_STATUS_MUTEX        (-EINTR)
#define I2C_STATUS_BUSY         (-EBUSY)
#define I2C_STATUS_ADDR_NAK     (-1000)
#define I2C_STATUS_DATA_NAK     (-1001)
#else
#define I2C_STATUS_SUCCESS      (0)
#define I2C_STATUS_MUTEX        (-1)
#define I2C_STATUS_BUSY         (-1)
#define I2C_STATUS_ADDR_NAK     (-1)
#define I2C_STATUS_DATA_NAK     (-1)
#endif

static cyg_drv_mutex_t i2c_mutex;

//  Initialize the I2C bus controller.
static void
hal_ks32c_i2c_init(void)
{
    cyg_uint32 prescale = KS32C_I2C_FREQ(100000);

    // reset the bus controller
    HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_RESET);

    // set the bus frequency
    HAL_WRITE_UINT32(KS32C_I2CPS, prescale);

    cyg_drv_mutex_init(&i2c_mutex);
}

#define RETURN(_x_) \
    CYG_MACRO_START                             \
    diag_printf("%s: line %d error=%d\n",__FUNCTION__,__LINE__,(_x_)); \
    return (_x_); \
    CYG_MACRO_END

//  Transfer the I2C messages.
int
hal_ks32c_i2c_transfer(cyg_uint32 nmsg, hal_ks32c_i2c_msg_t* pmsgs)
{
    cyg_uint32 i2ccon;

    // serialize access to the I2C bus
    if (!cyg_drv_mutex_lock(&i2c_mutex))
    {
        RETURN(I2C_STATUS_MUTEX);
    }

    // is the bus free ?
    do
    {
        HAL_READ_UINT32(KS32C_I2CCON, i2ccon);
    } while (i2ccon & KS32C_I2C_CON_BUSY);

    // transfer the messages
    for (; nmsg > 0; --nmsg, ++pmsgs)
    {
        // generate the start condition
        HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_START);

        // send the device address
        HAL_WRITE_UINT32(KS32C_I2CBUF, pmsgs->devaddr);
        do
        {
            HAL_READ_UINT32(KS32C_I2CCON, i2ccon);
        } while ((i2ccon & KS32C_I2C_CON_BF) == 0);

        // check if the slave ACK'ed the device address
        if (i2ccon & KS32C_I2C_CON_LRB)
        {
            // generate the stop condition
            HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_STOP);
            cyg_drv_mutex_unlock(&i2c_mutex);
            RETURN(I2C_STATUS_ADDR_NAK);
        }

        // read the message ?
        if (pmsgs->devaddr & KS32C_I2C_RD)
        {
            cyg_uint8* pbuf = pmsgs->pbuf;
            cyg_uint32 bufsize = pmsgs->bufsize;
            cyg_uint32 i2cbuf;

            // read more than one byte ?
            if (--bufsize > 0)
            {
                // enable ACK
                HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_ACK);

                while (bufsize-- > 0)
                {
                    do
                    {
                        HAL_READ_UINT32(KS32C_I2CCON, i2ccon);
                    } while ((i2ccon & KS32C_I2C_CON_BF) == 0);

                    // read the data byte
                    HAL_READ_UINT32(KS32C_I2CBUF, i2cbuf);
                    *pbuf++ = i2cbuf;
                }
            }

            // disable ACK
            HAL_WRITE_UINT32(KS32C_I2CCON, 0);
            do
            {
                HAL_READ_UINT32(KS32C_I2CCON, i2ccon);
            } while ((i2ccon & KS32C_I2C_CON_BF) == 0);

            // read the data byte
            HAL_READ_UINT32(KS32C_I2CBUF, i2cbuf);
            *pbuf++ = i2cbuf;
        }

        // write the message
        else
        {
            cyg_uint32 i;

            for (i = 0; i < pmsgs->bufsize; ++i)
            {
                HAL_WRITE_UINT32(KS32C_I2CBUF, pmsgs->pbuf[i]);
                do
                {
                    HAL_READ_UINT32(KS32C_I2CCON, i2ccon);
                } while ((i2ccon & KS32C_I2C_CON_BF) == 0);

                // check if the slave ACK'ed the data byte
                if (i2ccon & KS32C_I2C_CON_LRB)
                {
                    // generate the stop condition
                    HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_STOP);
                    cyg_drv_mutex_unlock(&i2c_mutex);
                    RETURN(I2C_STATUS_DATA_NAK);
                }
            }
        }

        // generate a restart condition ?
        if (nmsg > 1)
        {
            HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_RESTART);
        }
    }

    // generate the stop condition
    HAL_WRITE_UINT32(KS32C_I2CCON, KS32C_I2C_CON_STOP);

    cyg_drv_mutex_unlock(&i2c_mutex);
    return I2C_STATUS_SUCCESS;
}

//----------------------------------------------------------------------
//
// EEPROM Support
//

int hal_aim711_eeprom_write(cyg_uint8 *buf, int offset, int len)
{
    cyg_uint8 addr_page[1 + AIM711_EEPROM_PAGESIZE];
    cyg_uint8 const* pbufbyte = (cyg_uint8 const*)buf;
    hal_ks32c_i2c_msg_t msg;
    cyg_uint8 addr;
    cyg_uint32 bufsize;
 
    if (offset > AIM711_EEPROM_SIZE)
        RETURN(-1);

    if (len > (AIM711_EEPROM_SIZE - offset))
        len = (AIM711_EEPROM_SIZE - offset);

    addr = offset;
    bufsize = len;

    msg.devaddr = AIM711_EEPROM_ADDR | KS32C_I2C_WR;
    msg.pbuf = addr_page;

    while (bufsize > 0)
    {
        cyg_uint32 nbytes;
        int status;

        // write at most a page at a time
        nbytes = MIN(bufsize, AIM711_EEPROM_PAGESIZE);

        // don't cross a page boundary
        if (addr%AIM711_EEPROM_PAGESIZE)
        {
            nbytes = MIN(nbytes, AIM711_EEPROM_PAGESIZE - addr%AIM711_EEPROM_PAGESIZE);
        }

        // build the write message
        addr_page[0] = addr;
        memcpy(&addr_page[1], pbufbyte, nbytes);
        msg.bufsize = nbytes + 1;
        addr += nbytes;
        pbufbyte += nbytes;
        bufsize -= nbytes;

        // transfer the message
        status = hal_ks32c_i2c_transfer(1, &msg);
        if (status != I2C_STATUS_SUCCESS)
        {
            RETURN(status);
        }

        // delay 10 msec
        CYGACC_CALL_IF_DELAY_US(10000);
    }

    return len;
}

int hal_aim711_eeprom_read(cyg_uint8 *buf, int offset, int len)
{
    hal_ks32c_i2c_msg_t msgs[2];
    int status;
    cyg_uint8 toffset;
 
    if (offset > AIM711_EEPROM_SIZE)
        RETURN(-1);

    if (len > (AIM711_EEPROM_SIZE - offset))
        len = (AIM711_EEPROM_SIZE - offset);

    toffset = offset;

    // write message to set the address
    msgs[0].devaddr = AIM711_EEPROM_ADDR | KS32C_I2C_WR;
    msgs[0].pbuf = &toffset;
    msgs[0].bufsize = sizeof(toffset);

    // read message
    msgs[1].devaddr = AIM711_EEPROM_ADDR | KS32C_I2C_RD;
    msgs[1].pbuf = buf;
    msgs[1].bufsize = len;

    // transfer the messages
    status = hal_ks32c_i2c_transfer(2, msgs);

    if (status < 0)
        RETURN(status);

    return len;
}

//-----------------------------------------------------------------------------
//

