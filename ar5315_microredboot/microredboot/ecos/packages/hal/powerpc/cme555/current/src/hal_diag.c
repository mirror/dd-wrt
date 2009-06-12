//=============================================================================
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
// Author(s):    Bob Koninckx
// Contributors: Bob Koninckx
// Date:         2001-12-11
// Purpose:      HAL diagnostic output
// Description:  Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_diag.h>           // our header.

#include <cyg/infra/cyg_type.h>         // base types, externC
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // Interrupt macros

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP
#include <cyg/hal/hal_if.h>             // Calling-if API


static void cyg_hal_plf_serial_init(void);

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

//=============================================================================
// Serial driver
//=============================================================================

//-----------------------------------------------------------------------------
// There are two serial ports.
#define CYG_DEV_SERIAL_BASE_A    0x305008 // SCI0
#define CYG_DEV_SERIAL_BASE_B    0x305020 // SCI1

//-----------------------------------------------------------------------------
// Define CYG_DEVICE_SERIAL_RS232_SCIBR
// Default baudrate is 38400
// These values are calculated for a 40Mhz clock frequency
// This should be enough, we did not provide clock frequency as a configuration 
// option anyway
#define CYG_DEV_SERIAL_RS232_SCxBR_300    4167
#define CYG_DEV_SERIAL_RS232_SCxBR_600    2083
#define CYG_DEV_SERIAL_RS232_SCxBR_1200   1042
#define CYG_DEV_SERIAL_RS232_SCxBR_2400    521
#define CYG_DEV_SERIAL_RS232_SCxBR_4800    260
#define CYG_DEV_SERIAL_RS232_SCxBR_9600    130 
#define CYG_DEV_SERIAL_RS232_SCxBR_14400    87
#define CYG_DEV_SERIAL_RS232_SCxBR_19200    65
#define CYG_DEV_SERIAL_RS232_SCxBR_28800    43
#define CYG_DEV_SERIAL_RS232_SCxBR_38400    33
#define CYG_DEV_SERIAL_RS232_SCxBR_57600    22
#define CYG_DEV_SERIAL_RS232_SCxBR_115200   11

//-----------------------------------------------------------------------------
// Define the serial registers.
#define CYG_DEV_SERIAL_RS232_SCCR0 0x00
#define CYG_DEV_SERIAL_RS232_SCCR1 0x01
#define CYG_DEV_SERIAL_RS232_SCSR  0x02
#define CYG_DEV_SERIAL_RS232_SCDR  0x03

#define SCCR0_OTHR   0x8000 // Select baud rate other than system clock
#define SCCR0_LINKBD 0x4000 // Link baud
#define SCCR0_SCxBR  0x1fff // SCI baud rate

#define SCCR1_LOOPS  0x4000 // Loop mode
#define SCCR1_WOMS   0x2000 // Wired or for SCI pins
#define SCCR1_ILT    0x1000 // Idle line detect type
#define SCCR1_PT     0x0800 // Parity type (0 = Odd, 1 = Even)
#define SCCR1_PE     0x0400 // Parity enable
#define SCCR1_M      0x0200 // Mode select (0 = 10 bit frame, 1 = 11 bit frame)
#define SCCR1_WAKE   0x0100 // Wakeup by address mark
#define SCCR1_TIE    0x0080 // Transmit interrupt enable
#define SCCR1_TCIE   0x0040 // Transmit complete interrupt enable
#define SCCR1_RIE    0x0020 // Receiver interrupt enable
#define SCCR1_ILIE   0x0010 // Idle line interrupt enable
#define SCCR1_TE     0x0008 // Transmiter enable
#define SCCR1_RE     0x0004 // Receiver enable
#define SCCR1_RWU    0x0002 // Receiver wakeup
#define SCCR1_SBK    0x0001 // Send break

#define SCSR_TDRE    0x0100 // Transmit data register empty
#define SCSR_TC      0x0080 // Transmit complete
#define SCSR_RDRF    0x0040 // Receive data register full
#define SCSR_RAF     0x0020 // Receive active flag
#define SCSR_IDLE    0x0010 // Idle line detected
#define SCSR_OR      0x0008 // Overrun error
#define SCSR_NF      0x0004 // Noise error flag
#define SCSR_FE      0x0002 // Framing error
#define SCSR_PF      0x0001 // Parity error

//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint16*  base;
    cyg_int32    msec_timeout;
    int          siu_vector;
    int          imb3_vector;
    unsigned int level;
    int          baud_rate;
} channel_data_t;

//-----------------------------------------------------------------------------
static void
init_serial_channel(const channel_data_t* __ch_data)
{
    cyg_uint16 * base = __ch_data->base;
    cyg_uint16 br;
    
    switch(__ch_data->baud_rate)
    {
      case 300:
        br = CYG_DEV_SERIAL_RS232_SCxBR_300;
	break;
      case 600:
        br = CYG_DEV_SERIAL_RS232_SCxBR_600;
	break;
      case 1200:
        br = CYG_DEV_SERIAL_RS232_SCxBR_1200;
	break;
      case 2400:
        br = CYG_DEV_SERIAL_RS232_SCxBR_2400;
	break;
      case 4800:
        br = CYG_DEV_SERIAL_RS232_SCxBR_4800;
	break;
      case 9600:
        br = CYG_DEV_SERIAL_RS232_SCxBR_9600;
	break;
      case 14400:
        br = CYG_DEV_SERIAL_RS232_SCxBR_14400;
	break;
      case 19200:
        br = CYG_DEV_SERIAL_RS232_SCxBR_19200;
	break;
      case 28800:
        br = CYG_DEV_SERIAL_RS232_SCxBR_28800;
	break;
      case 38400:
        br = CYG_DEV_SERIAL_RS232_SCxBR_38400;
	break;
      case 57600:
        br = CYG_DEV_SERIAL_RS232_SCxBR_57600;
	break;
      case 115200:
        br = CYG_DEV_SERIAL_RS232_SCxBR_115200;
	break;
      default:
	// Use the default if something unknown is requested
        br = CYG_DEV_SERIAL_RS232_SCxBR_38400;
	break;
    }
    
    // 8-1-No parity
    HAL_WRITE_UINT16(base+CYG_DEV_SERIAL_RS232_SCCR1, (SCCR1_TE | SCCR1_RE));
       
    // Set baud rate
    HAL_WRITE_UINT16(base+CYG_DEV_SERIAL_RS232_SCCR0, br);
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint16 status;
    cyg_uint16 result;
    cyg_uint16 * base = ((channel_data_t *)__ch_data)->base;

    HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCSR, status);
    if((status & SCSR_RDRF) == 0)
        return false;

    HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCDR, result);
    *ch = (cyg_uint8)(result & 0x00ff);

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

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c)
{
    cyg_uint16 status;
    cyg_uint16 * base = ((channel_data_t *)__ch_data)->base;

    CYGARC_HAL_SAVE_GP();
    
    do {
       HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCSR, status);
    } while((status & SCSR_TDRE) == 0);

    HAL_WRITE_UINT16(base+CYG_DEV_SERIAL_RS232_SCDR, (short)c);

    // Hang around until the character is safely sent
    do {
       HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCSR, status);
    } while((status & SCSR_TDRE) == 0);
         
    CYGARC_HAL_RESTORE_GP();
}

// Channel data
// Do NOT make them const, will cause problems when trying
// to change the timeout parameter ... (Is this a bug in other)
// PowerPC platform hals ?? You only see it when you really start from
// flash ....
static channel_data_t channels[2] = {
    { (cyg_uint16*)CYG_DEV_SERIAL_BASE_A, 
      1000, 
      CYGNUM_HAL_INTERRUPT_SIU_LVL0, 
      CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX,
      0,
      CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD },
    { (cyg_uint16*)CYG_DEV_SERIAL_BASE_B, 
      1000, 
      CYGNUM_HAL_INTERRUPT_SIU_LVL0, 
      CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX,
      0,
      CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD }
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
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
	{
	va_list ap;
	va_start(ap, __func);
	
	ret = chan->baud_rate;
	chan->baud_rate = va_arg(ap, cyg_int32);
	init_serial_channel(chan);
	
	va_end(ap);
	}
	break;
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_SET_LEVEL(chan->imb3_vector, chan->level);
	HAL_INTERRUPT_UNMASK(chan->imb3_vector);
        HAL_INTERRUPT_UNMASK(chan->siu_vector);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->imb3_vector);
	HAL_INTERRUPT_MASK(chan->siu_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->siu_vector;
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
    
    cyg_uint16 status;
    cyg_uint16 control;

    cyg_uint16 * base = ((channel_data_t *)__ch_data)->base;
    
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;

    HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCSR, status);
    HAL_READ_UINT16(base+CYG_DEV_SERIAL_RS232_SCCR1, control);

    if((status & SCSR_RDRF) && (control & SCCR1_RIE))
    {   // Only if the interrupt was caused by the channel
        cyg_uint8 c;
        c = cyg_hal_plf_serial_getc(__ch_data);
	
        if(cyg_hal_is_break(&c, 1))
	    *__ctrlc = 1;
	
	HAL_INTERRUPT_ACKNOWLEDGE(((channel_data_t *)__ch_data)->imb3_vector);
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
    HAL_INTERRUPT_MASK(channels[0].imb3_vector);
    HAL_INTERRUPT_MASK(channels[1].imb3_vector);

    // Init channels
    init_serial_channel(&channels[0]);
    init_serial_channel(&channels[1]);

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

//=============================================================================
// Compatibility with older stubs
//=============================================================================
#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION
#endif

//-----------------------------------------------------------------------------
// Assumption: all diagnostic output must be GDB packetized unless
// this is a configuration for a stand-alone ROM system.
#if defined(CYG_HAL_STARTUP_ROM) && !defined(CYGSEM_HAL_ROM_MONITOR)
# define HAL_DIAG_USES_HARDWARE
#endif

#if (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 0)
# define __BASE ((cyg_uint16*)CYG_DEV_SERIAL_BASE_A)
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 1)
# define __BASE ((cyg_uint16*)CYG_DEV_SERIAL_BASE_B)
#else
# error "LCD driver not (yet) implemented for cme555"
#endif

static channel_data_t channel = { __BASE, 0, 0 };

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_init(void)
{
    init_serial_channel(&channel);
}

void hal_diag_write_char(char __c)
{
    cyg_hal_plf_serial_putc(&channel, __c);
}

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&channel);
}

#else  // ifdef HAL_DIAG_USES_HARDWARE

// Initialize diag port
void
hal_diag_init(void)
{
    // Init devices
    init_serial_channel(&channel);
}

void 
hal_diag_write_char_serial( char c )
{
    unsigned long __state;
    HAL_DISABLE_INTERRUPTS(__state);
    cyg_hal_plf_serial_putc(&channel, c);
    HAL_RESTORE_INTERRUPTS(__state);
}

void
hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&channel);
}

void 
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif
        
        while(1)
        {
            char c1;
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.
            hal_diag_read_char(&c1);

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            if( 3 == c1 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt(
                    (target_register_t)__builtin_return_address(0));
                break;
            }
#endif
            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
        
    }
}

#endif  // ifdef HAL_DIAG_USES_HARDWARE

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

//-----------------------------------------------------------------------------
// End of hal_diag.c
