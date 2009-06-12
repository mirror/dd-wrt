//==========================================================================
//
//      devs/serial/MCF52xx/MCF5282
//
//      MCF5272 UART Serial I/O Interface Module (interrupt driven)
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

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_memmap.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <pkgconf/io_serial_mcf5272_uart.h>
#include <cyg/io/ser_mcf5272_uart.h>


/* The UART priority level */
#define MCF5272_UART_PRIORITY_LEVEL 2


/* Autobaud states */
typedef enum autobaud_states_t
{
    AB_IDLE = 0,  /* Normal state. Autobaud process hasn't been initiated yet. */
    AB_BEGIN_BREAK, /* Detected a start of the break */
    AB_BEGIN,       /* Detected the end of the break and has set up the autobaud.*/

}autobaud_states_t;

#define FIELD_OFFSET(type,field) (cyg_uint32)(&(((type*)0)->field))

typedef struct MCF5272_uart_info_t
{

    volatile mcf5272_sim_uart_t*    base;                       // Base address of the UART registers
    uint32                          uart_vector;                // UART interrupt vector number

    cyg_interrupt                   serial_interrupt;           // Interrupt context
    cyg_handle_t                    serial_interrupt_handle;    // Interrupt handle

    volatile uint8                  imr_mirror;                 // Interrupt mask register mirror

    cyg_serial_info_t               config;                     // The channel configuration

    autobaud_states_t               autobaud_state;             // The autobaud state


} MCF5272_uart_info_t;

/* Function prtoftyps for the MCF5272 UART ISR and DSR. */
static cyg_uint32 MCF5272_uart_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       MCF5272_uart_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);


/* Function prototypes for the serial functions. */
static bool MCF5272_uart_init(struct cyg_devtab_entry * tab);
static Cyg_ErrNo MCF5272_uart_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry *sub_tab,
                  const char *name);
static bool MCF5272_uart_putc(serial_channel *chan, unsigned char c);
static unsigned char MCF5272_uart_getc(serial_channel *chan);
Cyg_ErrNo MCF5272_uart_set_config(serial_channel *chan, cyg_uint32 key,
                            const void *xbuf, cyg_uint32 *len);
static void MCF5272_uart_start_xmit(serial_channel *chan);
static void MCF5272_uart_stop_xmit(serial_channel * chan);


/* Declare the serial functions that are called by the common serial driver layer. */
static SERIAL_FUNS
(
    MCF5272_uart_funs,
    MCF5272_uart_putc,
    MCF5272_uart_getc,
    MCF5272_uart_set_config,
    MCF5272_uart_start_xmit,
    MCF5272_uart_stop_xmit
);


/* Definition for channel 0 UART configuration. */
/************************************************/
#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL0

/* Data structure contains
   channel informtion.
 */
static MCF5272_uart_info_t MCF5272_uart_channel_info_0;

/* If the channel buffer size is zero, do not include interrupt UART processing */
#if CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BUFSIZE > 0

/*      Allocated receive and transmit buffer.   The size of the buffer  is */
/* configured by the configtool.                                            */

static unsigned char MCF5272_uart_out_buf0[CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BUFSIZE];
static unsigned char MCF5272_uart_in_buf0[CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BUFSIZE];

/*      Channel function table.   We register  the UART  functions here  so */
/* that uppper serial drivers can call the serial driver's routines.        */

static SERIAL_CHANNEL_USING_INTERRUPTS(
    MCF5272_uart_channel_0,
    MCF5272_uart_funs,
    MCF5272_uart_channel_info_0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT,
    MCF5272_uart_out_buf0, sizeof(MCF5272_uart_out_buf0),
    MCF5272_uart_in_buf0, sizeof(MCF5272_uart_in_buf0)
);
#else
/* Don't use interrupt processing for the UART. */
static SERIAL_CHANNEL(
    MCF5272_uart_channel_0,
    MCF5272_uart_funs,
    MCF5272_uart_channel_info_0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT
);
#endif

DEVTAB_ENTRY(
    MCF5272_uart_io0,
    CYGDAT_IO_SERIAL_MCF5272_UART_CHANNEL0_NAME,
    0,                       // Does not depend on a lower level interface
    &cyg_io_serial_devio,    // The table of I/O functions.
    MCF5272_uart_init,       // UART initialization function.
    MCF5272_uart_lookup,     // The UART lookup function. This function typically sets
                             // up the device for actual use, turing on interrupts, configuring the port, etc.
    &MCF5272_uart_channel_0
);
#endif //  CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0

/* Definition for channel 1 UART configuration. */
/************************************************/

#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL1


/* Data structure contains
   channel informtion.
 */
static MCF5272_uart_info_t MCF5272_uart_channel_info_1;

/* If the channel buffer size is zero, do not include interrupt UART processing */
#if CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL1_BUFSIZE > 0

/*      Allocated receive and transmit buffer.   The size of the buffer  is */
/* configured by the configtool.                                            */

static unsigned char MCF5272_uart_out_buf1[CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL1_BUFSIZE];
static unsigned char MCF5272_uart_in_buf1[CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL1_BUFSIZE];

/*      Channel function table.   We register  the UART  functions here  so */
/* that uppper serial drivers can call the serial driver's routines.        */

static SERIAL_CHANNEL_USING_INTERRUPTS(
    MCF5272_uart_channel_1,
    MCF5272_uart_funs,
    MCF5272_uart_channel_info_1,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL1_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT,
    MCF5272_uart_out_buf1, sizeof(MCF5272_uart_out_buf1),
    MCF5272_uart_in_buf1, sizeof(MCF5272_uart_in_buf1)
);

#else
static SERIAL_CHANNEL(MCF5272_uart_channel_1,
                      MCF5272_uart_funs,
                      MCF5272_uart_channel_info_1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
DEVTAB_ENTRY(
    MCF5272_uart_io1,
    CYGDAT_IO_SERIAL_MCF5272_UART_CHANNEL1_NAME,
    0,                     // Does not depend on a lower level interface
    &cyg_io_serial_devio,  // The table of I/O functions.
    MCF5272_uart_init,     // UART initialization function.
    MCF5272_uart_lookup,   // The UART lookup function. This function typically sets
                           // up the device for actual use, turing on interrupts, configuring the port, etc.
    &MCF5272_uart_channel_1
);
#endif //  CYGNUM_IO_SERIAL_MCF5272_UART_CHANNEL1



/* Definition of macros that access the UART's SIM registers */
/* Read from a register */

#define MCF5272_UART_WRITE(_addr_,_value_) \
   *((volatile CYG_BYTE*)&(_addr_)) = (CYG_BYTE)(_value_)
/* Write to a register */
#define MCF5272_UART_READ(_addr_) \
   *(volatile CYG_BYTE*)&(_addr_)


/* Function Prototypes */
/* =================== */
/* Internal function to actually configure the hardware to desired baud rate, etc. */
static bool MCF5272_uart_config_port(serial_channel*, cyg_serial_info_t*);
static void MCF5272_uart_start_xmit(serial_channel*);


/* Baudrate conversion table. */
static unsigned long baud_rates_table[]=
{
    0,
    50,     // CYGNUM_SERIAL_BAUD_50 = 1
    75,     // CYGNUM_SERIAL_BAUD_75
    110,    // CYGNUM_SERIAL_BAUD_110
    134,    // CYGNUM_SERIAL_BAUD_134_5
    150,    // CYGNUM_SERIAL_BAUD_150
    200,    // CYGNUM_SERIAL_BAUD_200
    300,    // CYGNUM_SERIAL_BAUD_300
    600,    // CYGNUM_SERIAL_BAUD_600
    1200,   // CYGNUM_SERIAL_BAUD_1200
    1800,   // CYGNUM_SERIAL_BAUD_1800
    2400,   // CYGNUM_SERIAL_BAUD_2400
    3600,   // CYGNUM_SERIAL_BAUD_3600
    4800,   // CYGNUM_SERIAL_BAUD_4800
    7200,   // CYGNUM_SERIAL_BAUD_7200
    9600,   // CYGNUM_SERIAL_BAUD_9600
    14400,  // CYGNUM_SERIAL_BAUD_14400
    19200,  // CYGNUM_SERIAL_BAUD_19200
    38400,  // CYGNUM_SERIAL_BAUD_38400
    57600,  // CYGNUM_SERIAL_BAUD_57600
    115200, // CYGNUM_SERIAL_BAUD_115200
    230400  // CYGNUM_SERIAL_BAUD_230400
};

/*      The table contains  divers  to  divide  the  clock  to  configre  a */
/* approppriate for the UART.                                               */

static unsigned long dividers_table[]=
{
    0,
    46080,   // 50
    30720,   // 75
    20945,   // 110
    17130,   // 134_5
    15360,   // 150
    11520,   // 200
    7680,    // 300
    3840,    // 600
    1920,    // 1200
    1280,    // 1800
    960,     // 2400
    640,     // 3600
    480,     // 4800
    320,     // 7200
    240,     // 9600
    160,     // 14400
    120,     // 19200
    60,      // 38400
    40,      // 57600
    20,      // 115200
    10       // 230400
};

/*******************************************************************************
 MCF5272_uart_init() - This routine is called during bootstrap to set up the
                       UART driver.

 INPUT:
    Pointer to the the device table.

 RETURN:
    Returns true if the initialization is successful. Otherwise, it retuns false
*/
static bool MCF5272_uart_init(struct cyg_devtab_entry * tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    MCF5272_uart_info_t *MCF5272_uart_chan = (MCF5272_uart_info_t *)chan->dev_priv;

    #ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL0

    /*   Instantiation of the  UART channel  0 data  strucutre.  This  data */
    /* structure contains channel information.                              */

    if (strcmp(tab->name, CYGDAT_IO_SERIAL_MCF5272_UART_CHANNEL0_NAME) == 0)
    {

        /*   Initiliaze the UART information data to all zeros.             */

        memset(MCF5272_uart_chan, sizeof(MCF5272_uart_info_t), 0);

        /*   Set the base  address of the  UART registers to  differentiate */
        /* itself from the different regusters for the other UART port.     */

        MCF5272_uart_chan->base = (mcf5272_sim_uart_t*)&MCF5272_SIM->uart[0];

        /*   Set the UART interrupt vector number.                          */

        MCF5272_uart_chan->uart_vector = CYGNUM_HAL_VECTOR_UART1;

        /*   Set the autobaud state to idle.                                */

        MCF5272_uart_chan->autobaud_state = AB_IDLE;

        /* Initilize the UART 0 output pins */

        MCF5272_SIM->gpio.pbcnt =  MCF5272_GPIO_PBCNT_URT0_EN |
            ((MCF5272_SIM->gpio.pbcnt) & ~MCF5272_GPIO_PBCNT_URT0_MSK);
    }
    #endif

    #ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL1

    /* Instantiation of the UART channel 1 data strucutre. This data structure contains
       channel information.
     */
    if (strcmp(tab->name, CYGDAT_IO_SERIAL_MCF5272_UART_CHANNEL1_NAME) == 0)
    {

        /*   Initiliaze the UART information data to all zeros.             */

        memset(MCF5272_uart_chan, sizeof(MCF5272_uart_info_t), 0);

        /*   Set the base  address of the  UART registers to  differentiate */
        /* itself from the different regusters for the other UART port.     */

        MCF5272_uart_chan->base = (mcf5272_sim_uart_t*)&MCF5272_SIM->uart[1];

         /*   Set the UART interrupt vector number.                          */

        MCF5272_uart_chan->uart_vector = CYGNUM_HAL_VECTOR_UART2;

        /*   Set the autobaud state to idle.                                */

        MCF5272_uart_chan->autobaud_state = AB_IDLE;

        /* Initilize the UART 1 output pins */

        MCF5272_SIM->gpio.pdcnt =  MCF5272_GPIO_PDCNT_URT1_EN |
            ((MCF5272_SIM->gpio.pdcnt) & ~MCF5272_GPIO_PDCNT_URT1_MSK);

    }
    #endif


    if (chan->out_cbuf.len > 0) {

        /*   If the the buffer is greater  than zero, then the driver  will */
        /* use  interrupt  driven  I/O.   Hence,  the  driver  creates   an */
        /* interrupt context for the UART device.                           */

        cyg_drv_interrupt_create(MCF5272_uart_chan->uart_vector,
                                 MCF5272_UART_PRIORITY_LEVEL,           // Priority - Level 2
                                 (cyg_addrword_t)chan,                  //  Data item passed to interrupt handler
                                 MCF5272_uart_ISR,
                                 MCF5272_uart_DSR,
                                 &MCF5272_uart_chan->serial_interrupt_handle,
                                 &MCF5272_uart_chan->serial_interrupt);

        cyg_drv_interrupt_attach(MCF5272_uart_chan->serial_interrupt_handle);

        (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    }

    // Configure Serial device.
    return(MCF5272_uart_config_port(chan, &chan->config));
}

/******************************************************************************************************
 MCF5272_uart_config_port() - Configure the UART port.

 Internal function to actually configure the hardware to desired baud rate, etc.

 INPUT:
    chan        - The channel information
    new_confg   - The port configuration which include the desired baud rate, etc.

 RETURN:
    Returns true if the port configuration is successful. Otherwise, it retuns false

 */
static bool MCF5272_uart_config_port(serial_channel *chan,
                                     cyg_serial_info_t *new_config)
{
    MCF5272_uart_info_t * port = (MCF5272_uart_info_t *) chan->dev_priv;
    uint8 mode_reg = 0;
    uint32 ubgs;
    unsigned int baud_divisor;


    /*   Get the  divisor  from  the  baudrate  table  which  will  use  to */
    /* configure the port's baud rate.                                      */

    baud_divisor = baud_rates_table[new_config->baud];

    /*   If the divisor is zeor, we dont' configure the port.               */

    if (baud_divisor == 0) return false;

    /*   Save the configuration value for later use.                        */

    port->config = *new_config;

    /*   We first write the reset values into the device and then configure */
    /* the device the we way we want to use it.                             */
	
	/* Reset Transmitter */

    MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_RESET_TX);

	/* Reset Receiver */
	
    MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_RESET_RX);

	/* Reset Mode Register */
	
    MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_RESET_MR);

    /* Translate the parity configuration to UART mode bits. */

    switch(port->config.parity)
    {
    default:
    case CYGNUM_SERIAL_PARITY_NONE:
        mode_reg = 0 | MCF5272_UART_UMR1_PM_NONE;
        break;
    case CYGNUM_SERIAL_PARITY_EVEN:
        mode_reg = 0 | MCF5272_UART_UMR1_PM_EVEN;
        break;
    case CYGNUM_SERIAL_PARITY_ODD:
        mode_reg = 0 | MCF5272_UART_UMR1_PM_ODD;
        break;
    case CYGNUM_SERIAL_PARITY_MARK:
        mode_reg = 0 | MCF5272_UART_UMR1_PM_FORCE_HI;
        break;
    case CYGNUM_SERIAL_PARITY_SPACE:
        mode_reg = 0 | MCF5272_UART_UMR1_PM_FORCE_LO;
        break;
    }

    /* Translate the number of bits per character configuration to UART mode bits. */

    switch(port->config.word_length)
    {

    case CYGNUM_SERIAL_WORD_LENGTH_5:
        mode_reg |= MCF5272_UART_UMR1_BC_5;
        break;
    case CYGNUM_SERIAL_WORD_LENGTH_6:
        mode_reg |= MCF5272_UART_UMR1_BC_6;
        break;
    case CYGNUM_SERIAL_WORD_LENGTH_7:
        mode_reg |= MCF5272_UART_UMR1_BC_7;
        break;
    default:
    case CYGNUM_SERIAL_WORD_LENGTH_8:
        mode_reg |= MCF5272_UART_UMR1_BC_8;
        break;
    }

   	/* Configure the parity and the bits per character */
	
    MCF5272_UART_WRITE(port->base->umr, mode_reg);

    /* Translate the stop bit length to UART mode bits. */

    switch(port->config.stop)
    {
    default:
    case CYGNUM_SERIAL_STOP_1:
        mode_reg = MCF5272_UART_UMR2_STOP_BITS_1;
        break;
    case CYGNUM_SERIAL_STOP_1_5:
        mode_reg = MCF5272_UART_UMR2_STOP_BITS_15;
        break;
    case CYGNUM_SERIAL_STOP_2:
        mode_reg = MCF5272_UART_UMR2_STOP_BITS_2;
        break;
    }

	/* No echo or loopback */
	
    MCF5272_UART_WRITE(port->base->umr, 0 | MCF5272_UART_UMR2_CM_NORMAL | mode_reg);

	/* Set Rx and Tx baud by timer */
	
    MCF5272_UART_WRITE(port->base->ucr, 0 | MCF5272_UART_UCSR_RCS(0xD) |
                       MCF5272_UART_UCSR_TCS(0xD));

	/* Mask all USART interrupts */
	
    MCF5272_UART_WRITE(port->base->uisr_uimr, 0);

	/* Calculate baud settings */
	
    ubgs = (uint16)((CYGHWR_HAL_SYSTEM_CLOCK_MHZ*1000000)/
                    (baud_divisor * 32));

    /*   Program the baud settings to the device.                           */

	MCF5272_UART_WRITE(port->base->udu, (uint8)((ubgs & 0xFF00) >> 8));
	MCF5272_UART_WRITE(port->base->udl, (uint8)(ubgs & 0x00FF));

	/* Enable receiver and transmitter */

    MCF5272_UART_WRITE(port->base->ucr, 0 | MCF5272_UART_UCR_TXRXEN);

    /* Enable both transmit and receive interrupt. */

    port->imr_mirror = MCF5272_UART_UIMR_TXRDY | MCF5272_UART_UIMR_FFULL |
                       MCF5272_UART_UIMR_DB;
    MCF5272_UART_WRITE(port->base->uisr_uimr, port->imr_mirror);

    return true; /* Returns true to indicate a successful configuration */

}

/*******************************************************************************
 MCF5272_uart_lookup() - This routine is called when the device is "looked" up
                        (i.e. attached)

 INPUT:
    tab - pointer to a pointer of the device table
    sub_tab - Pointer to the sub device table.
    name - name of the device

 RETURN:
    always return ENOERR

*/
static Cyg_ErrNo MCF5272_uart_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

/*******************************************************************************
 MCF5272_uart_putc() - Send a character to the device output buffer.

 INPUT:
    chan - pointer to the serial private data.
    c    - the character to output

 RETURN:
    'true' if character is sent to device, return 'false' when we've
    ran out of buffer space in the device itself.

*/

static bool MCF5272_uart_putc(serial_channel *chan, unsigned char c)
{
    CYG_INTERRUPT_STATE int_state;
    MCF5272_uart_info_t *port = (MCF5272_uart_info_t *)chan->dev_priv;

    /* Make sure the transmitter is not full. If it is full, return false. */
    if  (!(MCF5272_UART_READ(port->base->usr_ucsr) & MCF5272_UART_USR_TXRDY))
        return false;

    /* Enable transmit interrupt. */
    HAL_DISABLE_INTERRUPTS(int_state);
    port->imr_mirror |= MCF5272_UART_UIMR_TXRDY;
    MCF5272_UART_WRITE(port->base->uisr_uimr, port->imr_mirror);
    HAL_RESTORE_INTERRUPTS(int_state);

    /* Enable the UART transmit. */
    MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_TXRXEN);

	/* Send the character */
    MCF5272_UART_WRITE(port->base->urb_utb, c);

	return true ;
}


/******************************************************************************************************
 MCF5272_uart_getc() - Fetch a character from the device input bufferand return it to the alling
                       routine. Wait until there is a character ready.

 INPUT:
    chan - pointer to the serial private data.

 RETURN:
    the character read from the UART.

 */
static unsigned char MCF5272_uart_getc(serial_channel *chan)
{
    MCF5272_uart_info_t * port = (MCF5272_uart_info_t *)chan->dev_priv;

    /* Wait until character has been received */

	while (!(MCF5272_UART_READ(port->base->usr_ucsr) & MCF5272_UART_USR_RXRDY))
    {
        diag_printf("ready poll");
    }

    /* Read the character from the FIFO queue. */
	
    return MCF5272_UART_READ(port->base->urf);

}


/*******************************************************************************
 MCF5272_uart_set_config() - Set up the device characteristics; baud rate, etc.

 INPUT:
    chan - pointer to the serial private data.
    key  - configuration key (command).
    xbuf - pointer to the configuration buffer
    len  - the length of the configuration buffer

 RETURN:
    NOERR - If the configuration is successful
    EINVAL -  If the argument is invalid

*/

Cyg_ErrNo MCF5272_uart_set_config(serial_channel *chan, cyg_uint32 key,
                            const void *xbuf, cyg_uint32 *len)
{
    cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
    MCF5272_uart_info_t * port = (MCF5272_uart_info_t *) chan->dev_priv;


    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
          /* Set serial configuration. */
              if ( *len < sizeof(cyg_serial_info_t) ) {
              return EINVAL;
          }
          *len = sizeof(cyg_serial_info_t);

          if (!MCF5272_uart_config_port(chan, config))
              return EINVAL;
      }
      break;

    case CYG_IO_GET_CONFIG_SERIAL_INFO:
        // Retrieve UART configuration
        *config = port->config;
        break;

    default:
        return EINVAL;
    }
    return ENOERR;
}


/*******************************************************************************
  MCF5272_uart_start_xmit() - Enable the transmitter on the device.

  INPUT:
    chan - pointer to the serial private data.

*/
static void MCF5272_uart_start_xmit(serial_channel *chan)
{
    CYG_INTERRUPT_STATE int_state;
    MCF5272_uart_info_t * port = (MCF5272_uart_info_t *) chan->dev_priv;

    /* Enable the UART transmit. */
    MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_TXEN);

    /* Enable transmit interrupt */
    HAL_DISABLE_INTERRUPTS(int_state);
    port->imr_mirror |= MCF5272_UART_UIMR_TXRDY;
    MCF5272_UART_WRITE(port->base->uisr_uimr, port->imr_mirror);
    HAL_RESTORE_INTERRUPTS(int_state);


}


/******************************************************************************************************
 MCF5272_uart_stop_xmit() - Disable the transmitter on the device

 INPUT:
    chan - pointer to the serial private data.

*/
static void MCF5272_uart_stop_xmit(serial_channel * chan)
{	
   CYG_INTERRUPT_STATE int_state;
   MCF5272_uart_info_t * port = (MCF5272_uart_info_t *) chan->dev_priv;

   /* Disable transmit interrupt */
   HAL_DISABLE_INTERRUPTS(int_state);
   port->imr_mirror &= ~MCF5272_UART_UIMR_TXRDY;
   MCF5272_UART_WRITE(port->base->uisr_uimr, port->imr_mirror);
   HAL_RESTORE_INTERRUPTS(int_state);

   /* Disable the UART transmit.
      !!!!!!!!!!!!!
      !!!WARNING!!!
      !!!!!!!!!!!!!
      If the transmit the disabe
      the diag_printf routines will poll forever to transmit the
      a character. Hence, don't ever disable the transmit if
      we want it to work with diag_printf.
   */
   //MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_TXDE);


}


/******************************************************************************************************
 MCF5272_uart_ISR() - UART I/O interrupt interrupt service routine (ISR).

 INPUT:
    vector - the interrupt vector number
    data   - user parameter.


 RETURN:
     returns CYG_ISR_CALL_DSR to call the DSR.

 */
static cyg_uint32 MCF5272_uart_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *) data;
    MCF5272_uart_info_t * port = (MCF5272_uart_info_t *) chan->dev_priv;


    /* Write the value in the interrupt status register back
       to the mask register to disable the interrupt temporarily.
     */

    MCF5272_UART_WRITE(port->base->uisr_uimr, 0);

    return CYG_ISR_CALL_DSR;  // Cause DSR to run
}



/******************************************************************************************************
 MCF5272_uart_DSR() - Defered Service Routine (DSR) - This routine processes the interrupt
                      from the device.

 INPUT:
    vector - The interrupt vector number
    count  - The nunber of DSR requests.
    data   - Device specific information

*/

static void MCF5272_uart_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    MCF5272_uart_info_t *port = (MCF5272_uart_info_t *)chan->dev_priv;
    volatile u8_t isr;


    /* Retrieve the interrupt status bits. We use these status bits to figure out
       what process shouled we perform: read from the UART or inform of a completion
       of a data transmission.
    */

    /* Retrieve the interrupt status register so
     * the DSR can look it up.
     */


    while((isr = (MCF5272_UART_READ(port->base->uisr_uimr) & port->imr_mirror)))
    {
       switch (port->autobaud_state)
       {
       default:
       case AB_IDLE:
           if (isr & MCF5272_UART_UISR_DB)
           {
               /* Detected the begin of a break, set the state to AB_BEGIN_BREAK
               */
               port->autobaud_state = AB_BEGIN_BREAK;

               /* Reset the Delta Break bit in the UISR */
               MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_RESET_BKCHGINT);

           }
           break;
       case AB_BEGIN_BREAK:
           if (isr & MCF5272_UART_UISR_DB)
           {
               /* Detected the end of a break, set the state to AB_BEGIN, and
                  setup autobaud detection.
               */
               port->autobaud_state = AB_BEGIN;

               /* Reset the Delta Break bit in the UISR and Enable autobaud */
               MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_RESET_BKCHGINT |
                                  MCF5272_UART_UCR_ENAB);

               /* Enable autobaud completion interrupt */
               port->imr_mirror |= MCF5272_UART_UIMR_ABC;
               /* Disable the delta break interrupt so we can't receive
                  anymore break interrupt.
               */
               port->imr_mirror &= ~MCF5272_UART_UIMR_DB;

           }
           break;

       case AB_BEGIN:
           if (isr & MCF5272_UART_UISR_ABC)
           {
               int count;
               // Retrieve the baudrate that we're using now.
               u16_t divider = (port->base->uabu << 8) + port->base->uabl;
               // Search in the list to find a match.
               for (count = sizeof(dividers_table)/sizeof(unsigned long) - 1;
                    count >= 0;
                    count--)
               {
                   if (divider < dividers_table[count]) break;
               }

               // Set the baud.
               port->config.baud = count;

               /* Autobaud completion */
               port->autobaud_state = AB_IDLE;

               /* Disable autobaud */
               MCF5272_UART_WRITE(port->base->ucr, MCF5272_UART_UCR_NONE);

               /* Ignore autobaud completion interrupt. */
               port->imr_mirror &= ~MCF5272_UART_UIMR_ABC;

               /* Reenable begin break change and receive interrupt. */
               port->imr_mirror |= MCF5272_UART_UIMR_DB;

           }
           break;

       }


        /* Receive character interrupt */
        if ((isr & MCF5272_UART_UISR_RXRDY))
            /* Ignore all receive interrupt when we're autobauding. */
        {
            // Read all the characters in the fifo.
            while ((MCF5272_UART_READ(port->base->uisr_uimr) & MCF5272_UART_UISR_RXRDY))
            {
                char c;
                /* Read the character from the UART. */
                c = MCF5272_UART_READ(port->base->urb_utb);
                /* Pass the read character to the upper layer. */
                (chan->callbacks->rcv_char)(chan, c);
            }
        }

        /* Transmit  complete interrupt */

        if ((isr & MCF5272_UART_UISR_TXRDY))
        {

            /*   Transmit holding register is empty                         */
           (chan->callbacks->xmt_char)(chan);

        }

    }

    /*   Unmask all the DUART interrupts  that were masked  in the ISR,  so */
    /* that we can receive the next interrupt.                              */

    MCF5272_UART_WRITE(port->base->uisr_uimr, port->imr_mirror);

}

#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL0
unsigned long MCF5272_uart_get_channel_0_baud_rate()
{
    /* return the baud rate for the first serial port */

    return baud_rates_table[MCF5272_uart_channel_info_0.config.baud];
}
#endif

#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL1
unsigned long MCF5272_uart_get_channel_1_baud_rate()
{
    /* return the baud rate for the second serial port */

    return baud_rates_table[MCF5272_uart_channel_info_1.config.baud];
}
#endif


