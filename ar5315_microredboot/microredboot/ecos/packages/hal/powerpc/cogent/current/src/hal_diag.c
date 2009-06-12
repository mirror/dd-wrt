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
// Author(s):    nickg, jskov
// Contributors: nickg, jskov
// Date:         1999-03-23
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
static void cyg_hal_plf_lcd_init(void);

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
    cyg_hal_plf_lcd_init();
}

//=============================================================================
// Serial driver
//=============================================================================

//-----------------------------------------------------------------------------
// There are two serial ports.
#define CYG_DEV_SERIAL_BASE_A    0xe900047 // port A
#define CYG_DEV_SERIAL_BASE_B    0xe900007 // port B

//-----------------------------------------------------------------------------
// Default baud rate is 38400
#define CYG_DEV_SERIAL_RS232_T1_VALUE_B38400         0x00
#define CYG_DEV_SERIAL_RS232_T2_VALUE_B38400         0x06

//-----------------------------------------------------------------------------
// Define the serial registers. The Cogent board is equipped with a 16552
// serial chip.
#define CYG_DEV_SERIAL_RBR   0x00  // receiver buffer register, read, dlab = 0
#define CYG_DEV_SERIAL_THR   0x00 // transmitter holding register, write, dlab = 0
#define CYG_DEV_SERIAL_DLL   0x00 // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IER   0x08 // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_SERIAL_DLM   0x08 // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IIR   0x10 // interrupt identification register, read, dlab = 0
#define CYG_DEV_SERIAL_FCR   0x10 // fifo control register, write, dlab = 0
#define CYG_DEV_SERIAL_AFR   0x10 // alternate function register, read/write, dlab = 1
#define CYG_DEV_SERIAL_LCR   0x18 // line control register, read/write
#define CYG_DEV_SERIAL_MCR   0x20
#define CYG_DEV_SERIAL_MCR_A 0x20
#define CYG_DEV_SERIAL_MCR_B 0x20
#define CYG_DEV_SERIAL_LSR   0x28 // line status register, read
#define CYG_DEV_SERIAL_MSR   0x30 // modem status register, read
#define CYG_DEV_SERIAL_SCR   0x38 // scratch pad register

// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits
#define ISR_Tx  0x02
#define ISR_Rx  0x04

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The FIFO control register
#define SIO_FCR_FCR0   0x01             // enable xmit and rcvr fifos
#define SIO_FCR_FCR1   0x02             // clear RCVR FIFO
#define SIO_FCR_FCR2   0x04             // clear XMIT FIFO


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//-----------------------------------------------------------------------------
static void
init_serial_channel(channel_data_t* __ch_data)
{
    cyg_uint8* base = __ch_data->base;
    cyg_uint8 lcr;

    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_IER, 0);

    // Disable and clear FIFOs (need to enable to clear).
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR,
                    (SIO_FCR_FCR0 | SIO_FCR_FCR1 | SIO_FCR_FCR2));
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR, 0);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);

    // Set speed to 38400.
    HAL_READ_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);
    lcr |= SIO_LCR_DLAB;
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);


    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLL,
                    CYG_DEV_SERIAL_RS232_T2_VALUE_B38400);
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLM,
                    CYG_DEV_SERIAL_RS232_T1_VALUE_B38400);
    lcr &= ~SIO_LCR_DLAB;
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);

    {
        // Special initialization for ST16C552 on CMA102
        cyg_uint8 mcr;
        
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_MCR_A, mcr);
        mcr |= 8;
        HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_MCR_A, mcr);

        HAL_READ_UINT8(base+CYG_DEV_SERIAL_MCR_B, mcr);
        mcr |= 8;
        HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_MCR_B, mcr);
    }

    // Enable FIFOs (and clear them).
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR,
                    (SIO_FCR_FCR0 | SIO_FCR_FCR1 | SIO_FCR_FCR2));
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;

    HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    if ((lsr & SIO_LSR_DR) == 0)
        return false;

    HAL_READ_UINT8(base+CYG_DEV_SERIAL_RBR, *ch);

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
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_THR, c);

    // Hang around until the character has been safely sent.
    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}

static channel_data_t channels[2] = {
    { (cyg_uint8*)CYG_DEV_SERIAL_BASE_A, 1000, CYGNUM_HAL_INTERRUPT_SIU_IRQ1},
    { (cyg_uint8*)CYG_DEV_SERIAL_BASE_B, 1000, CYGNUM_HAL_INTERRUPT_SIU_IRQ1}
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
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier |= SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
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
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 _iir;
    int res = 0;
    CYGARC_HAL_SAVE_GP();

    HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;
    if ( ISR_Rx == _iir ) {
        cyg_uint8 c, lsr;
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_LSR, lsr);
        if (lsr & SIO_LSR_DR) {

            HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_RBR, c);

            if( cyg_hal_is_break( &c , 1 ) )
                *__ctrlc = 1;
        }

        // Acknowledge the interrupt
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
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
// LCD driver
//=============================================================================

// The LCD driver is only used by the new vector code. Cannot be used
// by the old compatibility cruft.

// FEMA 162B 16 character x 2 line LCD
// base addresses and register offsets *

#define MBD_BASE 0

#define LCD_BASE                (MBD_BASE + 0xEB00007)

#define LCD_DATA                0x00 // read/write lcd data
#define LCD_STAT                0x08 // read lcd busy status
#define LCD_CMD                 0x08 // write lcd command

// status register bit definitions
#define LCD_STAT_BUSY   0x80    // 1 = display busy
#define LCD_STAT_ADD    0x7F    // bits 0-6 return current display address

// command register definitions
#define LCD_CMD_RST             0x01    // clear entire display and reset display address
#define LCD_CMD_HOME            0x02    // reset display address and reset any shifting
#define LCD_CMD_ECL             0x04    // move cursor left one position on next data write
#define LCD_CMD_ESL             0x05    // shift display left one position on next data write
#define LCD_CMD_ECR             0x06    // move cursor right one position on next data write
#define LCD_CMD_ESR             0x07    // shift display right one position on next data write
#define LCD_CMD_DOFF            0x08    // display off, cursor off, blinking off
#define LCD_CMD_BL              0x09    // blink character at current cursor position
#define LCD_CMD_CUR             0x0A    // enable cursor on
#define LCD_CMD_DON             0x0C    // turn display on
#define LCD_CMD_CL              0x10    // move cursor left one position
#define LCD_CMD_SL              0x14    // shift display left one position
#define LCD_CMD_CR              0x18    // move cursor right one position
#define LCD_CMD_SR              0x1C    // shift display right one position
#define LCD_CMD_MODE            0x38    // sets 8 bits, 2 lines, 5x7 characters
#define LCD_CMD_ACG             0x40    // bits 0-5 sets the character generator address
#define LCD_CMD_ADD             0x80    // bits 0-6 sets the display data address to line 1 +

// LCD status values
#define LCD_OK                  0x00
#define LCD_ERR                 0x01

#define LCD_LINE0       0x00
#define LCD_LINE1       0x40

#define LCD_LINE_LENGTH 16

static char lcd_line0[LCD_LINE_LENGTH+1];
static char lcd_line1[LCD_LINE_LENGTH+1];
static char *lcd_line[2] = { lcd_line0, lcd_line1 };
static int lcd_curline = 0;
static int lcd_linepos = 0;


static void lcd_dis(int add, char *s, cyg_uint8* base);

static void
init_lcd_channel(cyg_uint8* base)
{
    cyg_uint8 stat;
    int i;
    
    // wait for not busy
    // Note: It seems that the LCD isn't quite ready to process commands
    // when it clears the BUSY flag. Reading the status address an extra
    // time seems to give it enough breathing room.
    do { HAL_READ_UINT8(base+LCD_STAT, stat); } while (stat & LCD_STAT_BUSY);
    HAL_READ_UINT8(base+LCD_STAT, stat);

    // configure the lcd for 8 bits/char, 2 lines
    // and 5x7 dot matrix
    HAL_WRITE_UINT8(base+LCD_CMD, LCD_CMD_MODE);

    // wait for not busy
    do { HAL_READ_UINT8(base+LCD_STAT, stat); } while (stat & LCD_STAT_BUSY);
    HAL_READ_UINT8(base+LCD_STAT, stat);

    // turn the LCD display on
    HAL_WRITE_UINT8(base+LCD_CMD, LCD_CMD_DON);

    lcd_curline = 0;
    lcd_linepos = 0;

    for( i = 0; i < LCD_LINE_LENGTH; i++ )
        lcd_line[0][i] = lcd_line[1][i] = ' ';

    lcd_line[0][LCD_LINE_LENGTH] = lcd_line[1][LCD_LINE_LENGTH] = 0;

    lcd_dis(LCD_LINE0, lcd_line[0], base);
    lcd_dis(LCD_LINE1, lcd_line[1], base);
}

// this routine writes the string to the LCD
// display after setting the address to add
static void
lcd_dis(int add, char *s, cyg_uint8* base)
{
    cyg_uint8 stat;
    int i;

    // wait for not busy (see Note in hal_diag_init above)
    do { HAL_READ_UINT8(base+LCD_STAT, stat); } while (stat & LCD_STAT_BUSY);
    HAL_READ_UINT8(base+LCD_STAT, stat);

    // write the address
    HAL_WRITE_UINT8(base+LCD_CMD, (LCD_CMD_ADD + add));

    // write the string out to the display stopping when we reach 0
    for (i = 0; *s != '\0'; i++)
    {
        // wait for not busy
        do { HAL_READ_UINT8(base+LCD_STAT, stat); } while (stat & LCD_STAT_BUSY);
        HAL_READ_UINT8(base+LCD_STAT, stat);

        // write the data
        HAL_WRITE_UINT8(base+LCD_DATA, *s++);
    }
}

void
cyg_hal_plf_lcd_putc(void* __ch_data, cyg_uint8 c)
{
    cyg_uint8* base = (cyg_uint8*)__ch_data;
    unsigned long __state;
    int i;
    CYGARC_HAL_SAVE_GP();

    // ignore CR
    if( c == '\r' ) return;
    
    HAL_DISABLE_INTERRUPTS(__state);
    if( c == '\n' )
    {
        lcd_dis(LCD_LINE0, &lcd_line[lcd_curline^1][0], base);
        lcd_dis(LCD_LINE1, &lcd_line[lcd_curline][0], base);

        // Do a line feed
        lcd_curline ^= 1;
        lcd_linepos = 0;
        
        for( i = 0; i < LCD_LINE_LENGTH; i++ )
            lcd_line[lcd_curline][i] = ' ';

        HAL_RESTORE_INTERRUPTS(__state);
        return;
    }

    // Only allow to be output if there is room on the LCD line
    if( lcd_linepos < LCD_LINE_LENGTH )
        lcd_line[lcd_curline][lcd_linepos++] = c;

    HAL_RESTORE_INTERRUPTS(__state);
}

cyg_uint8
cyg_hal_plf_lcd_getc(void* __ch_data)
{
    return 0;
}

static void
cyg_hal_plf_lcd_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_lcd_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_lcd_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_lcd_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

static int
cyg_hal_plf_lcd_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    // Do nothing (yet).
    return 0;
}

static void
cyg_hal_plf_lcd_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channel
    init_lcd_channel((cyg_uint8*)LCD_BASE);

    // Setup procs in the vector table

    // Set channel 2
    CYGACC_CALL_IF_SET_CONSOLE_COMM(2);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, LCD_BASE);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_lcd_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_lcd_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_lcd_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_lcd_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_lcd_control);

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
# define __BASE ((cyg_uint8*)CYG_DEV_SERIAL_BASE_A)
#elif (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 1)
# define __BASE ((cyg_uint8*)CYG_DEV_SERIAL_BASE_B)
#else
# error "Cannot use LCD"
#endif

static channel_data_t channel = { __BASE, 0, 0};

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
