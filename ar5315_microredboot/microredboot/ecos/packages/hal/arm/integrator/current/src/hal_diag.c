/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Author(s):     David A Rusling
// Contributors:  Philippe Robin
// Date:          November 7, 2000
// Purpose:       HAL diagnostic output
// Description:   Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_arm_integrator.h> // board specifics

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_integrator.h>        // Hardware definitions


/*---------------------------------------------------------------------------*/
void abcd(void);

/* Codes for ASCI characters 32-127 */
unsigned int char_codes[] = {
    0x0000, 0x2400, 0x0044, 0x7E12, 0x25DA, 0x4848, 0x2580, 0x0400,
    0x0072, 0x001E, 0x7F80, 0x2580, 0x4000, 0x0180, 0x0000, 0x4800,
    0x007E, 0x080C, 0x01B6, 0x011E, 0x01CC, 0x01DA, 0x01FA, 0x2802,
    0x01FE, 0x01CE, 0x2400, 0x4400, 0x1800, 0x0190, 0x4200, 0x01A6,
    0x217E, 0x01EE, 0x251E, 0x0072, 0x241E, 0x00F2, 0x00E2, 0x017A,
    0x01EC, 0x2412, 0x001C, 0x18E0, 0x0070, 0x0A6C, 0x126C, 0x007E,
    0x01E6, 0x107E, 0x11E6, 0x01DA, 0x2402, 0x007C, 0x4860, 0x506C,
    0x5A00, 0x2A00, 0x4812, 0x0072, 0x1200, 0x001E, 0x0046, 0x0010,
    0x0200, 0x01BE, 0x01F8, 0x01B0, 0x01BC, 0x01F6, 0x2980, 0x01DE,
    0x01E8, 0x2000, 0x001C, 0x3500, 0x2400, 0x21A8, 0x01A8, 0x01B8,
    0x01E6, 0x01CE, 0x01A0, 0x01DA, 0x00F0, 0x0038, 0x4020, 0x5028,
    0x5A00, 0x030C, 0x4190, 0x2480, 0x2400, 0x2500, 0x0640, 0x0000
};

/* Codes for hexadecimal characters */
unsigned int hex_codes[] = {
    0x007E, 0x080C, 0x01B6, 0x011E, 0x01CC, 0x01DA, 0x01FA, 0x2802,
    0x01FE, 0x01CE, 0x01EE, 0x01F8, 0x0072, 0x01BC, 0x00F2, 0x00E2
};

void hal_diag_alpha_led_char(char c1, char c2);


// AMBA uart access macros
#define GET_STATUS(p)		(IO_READ((p) + AMBA_UARTFR))
#define GET_CHAR(p)		(IO_READ((p) + AMBA_UARTDR))
#define PUT_CHAR(p, c)		(IO_WRITE(((p) + AMBA_UARTDR), (c)))
#define IO_READ(p)              ((*(volatile unsigned int *)(p)) & 0xFF)
#define IO_WRITE(p, c)          (*(unsigned int *)(p) = (c))
#define RX_DATA(s)		(((s) & AMBA_UARTFR_RXFE) == 0)
#define TX_READY(s)		(((s) & AMBA_UARTFR_TXFF) == 0)
#define TX_EMPTY(p)		((GET_STATUS(p) & AMBA_UARTFR_TMSK) == 0)
#define RX_EMPTY(p)		((GET_STATUS(p) & AMBA_UARTFR_RXFE) == 0)
// Define the serial registers.

//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//-----------------------------------------------------------------------------

#if (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD == 9600)
#define ARM_INTEGRATOR_BAUD_DIVISOR	ARM_BAUD_9600
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD == 19200)
#define ARM_INTEGRATOR_BAUD_DIVISOR	ARM_BAUD_9600
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD == 38400)
#define ARM_INTEGRATOR_BAUD_DIVISOR	ARM_BAUD_38400
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD == 57600)
#define ARM_INTEGRATOR_BAUD_DIVISOR	ARM_BAUD_57600
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD == 115200)
#define ARM_INTEGRATOR_BAUD_DIVISOR	ARM_BAUD_115200
#endif

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;

    // first, disable everything
    IO_WRITE(base + AMBA_UARTCR, 0x0);
      
    // Set baud rate CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD
    IO_WRITE(base + AMBA_UARTLCR_M, ((ARM_INTEGRATOR_BAUD_DIVISOR & 0xf00) >> 8));
    IO_WRITE(base + AMBA_UARTLCR_L, (ARM_INTEGRATOR_BAUD_DIVISOR & 0xff));
      
    // ----------v----------v----------v----------v----------
    // NOTE: MUST BE WRITTEN LAST (AFTER UARTLCR_M & UARTLCR_L)
    // ----------^----------^----------^----------^----------
    // set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled
    IO_WRITE(base + AMBA_UARTLCR_H, (AMBA_UARTLCR_H_WLEN_8 | AMBA_UARTLCR_H_FEN));
  
    // finally, enable the uart
    IO_WRITE(base + AMBA_UARTCR, AMBA_UARTCR_UARTEN);

}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8* base = chan->base;
    cyg_uint32 status;
    CYGARC_HAL_SAVE_GP();

    do {
      status = GET_STATUS(base);
    } while (!TX_READY(status));	// wait until ready

    PUT_CHAR(base, c);

    if (c == '\n') {
      do {
	status = GET_STATUS(base);
      } while (!TX_READY(status));	// wait until ready
      
      PUT_CHAR(base, '\r');
    }

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint32 status ;
    long timeout = 100;  // A long time...

    do {
      status = GET_STATUS(base);
      if (--timeout == 0) return false ; 
    } while (!RX_DATA(status));	// wait until ready

    *ch = GET_CHAR(base);

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

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG) \
    || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)

static channel_data_t integrator_ser_channels[2] = {
    { (cyg_uint8*)0x16000000, 1000, CYGNUM_HAL_INTERRUPT_UARTINT0 },
    { (cyg_uint8*)0x17000000, 1000, CYGNUM_HAL_INTERRUPT_UARTINT1 }
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
    int ret = 0;
    cyg_uint8 status;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
	// Ensure that only Receive ints are generated.
	status = IO_READ(chan->base + AMBA_UARTCR);

	status |= (AMBA_UARTCR_RTIE | AMBA_UARTCR_RIE);
	HAL_WRITE_UINT32(chan->base + AMBA_UARTCR, status);

        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
	irq_state = 0;

	status = IO_READ(chan->base + AMBA_UARTCR);
	status &= ~(AMBA_UARTCR_RTIE | AMBA_UARTCR_TIE | AMBA_UARTCR_RIE | AMBA_UARTCR_MSIE);
	HAL_WRITE_UINT32(chan->base + AMBA_UARTCR, status);

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
    channel_data_t* chan = (channel_data_t*)__ch_data;
    char c;
    cyg_uint32 status;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    status = GET_STATUS(chan->base);

    if ( RX_DATA(status) ) {
        c = GET_CHAR(chan->base);

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
    HAL_INTERRUPT_MASK(integrator_ser_channels[0].isr_vector);
    HAL_INTERRUPT_MASK(integrator_ser_channels[1].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel(&integrator_ser_channels[0]);
    cyg_hal_plf_serial_init_channel(&integrator_ser_channels[1]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &integrator_ser_channels[0]);
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
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &integrator_ser_channels[1]);
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

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT2
    cyg_hal_gdb_isr_attach();	// FIXME, hack to get CTRLC working
#endif 
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG || CYGPRI_HAL_IMPLEMENTS_IF_SERVICES

/*---------------------------------------------------------------------------*/

#ifdef CYGHWR_HAL_ARM_INTEGRATOR_DIAG_LEDS

void
hal_diag_led(int n)
{
    volatile unsigned int *leds = (unsigned int *)INTEGRATOR_DBG_LEDS ;

    *leds |= (n & 0xF) ;
}
void
hal_diag_alpha_led(unsigned int val)
{
  volatile unsigned int *led = (unsigned int *)INTEGRATOR_DBG_BASE;

  while (*led & 0x01) ;

  *led = val;
}

void
hal_diag_alpha_led_char(char c1, char c2)
{
  volatile unsigned int *led = (unsigned int *)INTEGRATOR_DBG_BASE;
  unsigned int current;

  current = *led;
  if (c1) {
    if (c1 < 32 || c1 > 127) c1=32;
    if (c1 >= 'a' && c1 < 'z') c1 -= 32;

    current &= ~(0x00007FFE << 14);
    current |= (char_codes[c1-32] << 14);
    }
    
    if (c2) {
        if (c2 < 32 || c2 > 127) c2 = 32;
	if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
        
        current &= ~(0x00007FFE);
        current |= char_codes[c2-32];
    }
    
    hal_diag_alpha_led(current);

}

#endif // CYGHWR_HAL_ARM_INTEGRATOR_DIAG_LEDS

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
