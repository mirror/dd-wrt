#ifndef MCF5272_SIM_H
#define MCF5272_SIM_H
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

/*

    Defines for the mcf5272 System Integration Module (SIM)

*/

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

/*      General configuration registers.                                    */
typedef struct mcf5272_sim_cfg_t
{

    u32_t mbar;                         /*   Module base  address  register */
                                        /* (MBAR), after initialization     */

    u16_t scr;                          /*   System configuration register  */

    u16_t spr;                          /*   System protection register     */

    u32_t pmr;                          /*   Power management register      */

    u16_t res1;

    u16_t alpr;                         /*   Active low power register      */

    u32_t dir;                          /*   Device identification register */

    u32_t res2[3];

} __attribute__ ((aligned (4), packed)) mcf5272_sim_cfg_t;

/*      Interrupt controller registers.                                     */
typedef struct mcf5272_sim_int_t
{

    u32_t icr[4];                       /*   Interrupt control register 1-4 */

    u32_t isr;                          /*   Interrupt source register      */

    u32_t pitr;                         /*   Programmable         interrupt */
                                        /* transition register              */

    u32_t piwr;                         /*   Programmable interrupt  wakeup */
                                        /* register                         */

    u8_t  res1[3];

    u8_t  ipvr;                         /*   Programmable interrupt  vector */
                                        /* register                         */

} __attribute__ ((aligned (4), packed)) mcf5272_sim_int_t;

/*      Chip Select Module                                                  */
typedef struct mcf5272_sim_cs_t
{

    u32_t csbr;                         /*   CS base register.              */

    u32_t csor;                         /*   CS option register.            */

} __attribute__ ((aligned (4), packed)) mcf5272_sim_cs_t;

/*      General Purpose I/O Module                                          */
typedef struct mcf5272_sim_gpio_t
{

    /*   Use the following labels  to  initialize  the  bits  in  the  data */
    /* direction registers.  Setting  the bit to  zero indicates that  this */
    /* pin is an input, one indicates an input.                             */

#define MCF5272_GPIO_DDR_IN (0)
#define MCF5272_GPIO_DDR_OUT (1)

    u32_t pacnt;                        /*   Port A control register.       */

    u16_t paddr;                        /*   Port    A    data    direction */
                                        /* register.                        */

    u16_t padat;                        /*   Port A data register.          */

    u32_t pbcnt;                        /*   Port B control register.       */

    /*   Set these bits in the  port  B  control  register  to  enable  the */
    /* Ethernet, UART0, and data transfer acknowledge pins.                 */

#define MCF5272_GPIO_PBCNT_ETH_EN  (0x55550000)
#define MCF5272_GPIO_PBCNT_ETH_DE  (0x00000000)
#define MCF5272_GPIO_PBCNT_ETH_MSK (0xFFFF0000)

#define MCF5272_GPIO_PBCNT_TA_EN    (0x00000400)
#define MCF5272_GPIO_PBCNT_TA_DE    (0x00000000)
#define MCF5272_GPIO_PBCNT_TA_MSK   (0x00000C00)

#define MCF5272_GPIO_PBCNT_URT0_EN  (0x00000155)
#define MCF5272_GPIO_PBCNT_URT0_DE  (0x00000000)
#define MCF5272_GPIO_PBCNT_URT0_MSK (0x000003FF)

    u16_t pbddr;                        /*   Port    B    data    direction */
                                        /* register.                        */

    u16_t pbdat;                        /*   Port B data register.          */

    u32_t res1;

    u16_t pcddr;                        /*   Port    C    data    direction */
                                        /* register.                        */

    u16_t pcdat;                        /*   Port C data register.          */

    u32_t pdcnt;                        /*   Port D control register.       */

    /*   Set these bits in the port D control register to enable the  UART1 */
    /* and interrupt 4 pins.                                                */

#define MCF5272_GPIO_PDCNT_INT4_EN  (0x00000C00)
#define MCF5272_GPIO_PDCNT_INT4_DE  (0x00000000)
#define MCF5272_GPIO_PDCNT_INT4_MSK (0x00000C00)

#define MCF5272_GPIO_PDCNT_URT1_EN  (0x000002AA)
#define MCF5272_GPIO_PDCNT_URT1_DE  (0x00000000)
#define MCF5272_GPIO_PDCNT_URT1_MSK (0x000003FF)

    u16_t res2;
    u16_t res3;

} __attribute__ ((aligned (4), packed)) mcf5272_sim_gpio_t;

/*      UART Module                                                         */
typedef struct mcf5272_sim_uart_t
{

    u8_t umr;                           /*   UART mode register.            */
    u8_t res1[3];

#define MCF5272_UART_UMR_8BNP (0x13)    /*   Write this value  to  umr1  to */
                                        /* program the device  for  8  bits */
                                        /* and no parity.                   */

#define MCF5272_UART_UMR_1S (0x07)      /*   Write this value  to  umr2  to */
                                        /* program the device  for  1  stop */
                                        /* bit.                             */

    u8_t usr_ucsr;                      /*   UART status  register (R)  and */
    u8_t res2[3];                       /* UART clock-select register (W).  */

#define MCF5272_UART_USR_RRDY (1<<0)    /*   Bit 0  of  the  device  status */
                                        /* register is set when the receive */
                                        /* data register contains data.  If */
                                        /* the data is not removed from the */
                                        /* holding   register,   additional */
                                        /* data will be placed in the  FIFO */
                                        /* until the FIFO is overrun.       */

#define MCF5272_UART_USR_FFUL (1<<1)    /*   Bit 1  of  the  device  status */
                                        /* register is set when the receive */
                                        /* data  FIFO  is   full.    If   a */
                                        /* character is not removed  before */
                                        /* the next character is  received, */
                                        /* overrun will occur.              */

#define MCF5272_UART_USR_TXRDY (1<<2)   /*   Bit 2  of  the  device  status */
                                        /* register   is   set   when   the */
                                        /* transmit data  holding  register */
                                        /* is empty.   Note  that  this  is */
                                        /* different than  the  FIFO  being */
                                        /* empty  as  the  FIFO  may  still */
                                        /* contain characters  even if  the */
                                        /* holding register is empty.       */

#define MCF5272_UART_USR_TXEMP (1<<3)   /*   Bit 3  of  the  device  status */
                                        /* register   is   set   when   the */
                                        /* transmit data FIFO is empty.     */

#define MCF5272_UART_USR_OE (1<<4)      /*   Bit 4  of  the  device  status */
                                        /* register is set when an  overrun */
                                        /* error has occurred.              */

#define MCF5272_UART_USR_PE (1<<5)      /*   Bit 5  of  the  device  status */
                                        /* register is set  when  a  parity */
                                        /* error has occurred.              */

#define MCF5272_UART_USR_FE (1<<6)      /*   Bit 6  of  the  device  status */
                                        /* register is set  when a  framing */
                                        /* error has occurred.              */

#define MCF5272_UART_USR_RB (1<<7)      /*   Bit 7  of  the  device  status */
                                        /* register is set when a change in */
                                        /* break status has occurred on the */
                                        /* port.                            */

#define MCF5272_UART_UCSR_CLKIN (0xDD)  /*   Writing this value to the ucsr */
                                        /* selects CLKIN/16  as the  UART's */
                                        /* clock source.                    */

    u8_t ucr;                           /*   UART command register (W).     */
    u8_t res3[3];

#define MCF5272_UART_UCR_RMR (0x01<<4)  /*   Write this  value  to  ucr  to */
                                        /* reset the mode register to umr1. */

#define MCF5272_UART_UCR_RRX (0x02<<4)  /*   Write this  value  to  ucr  to */
                                        /* reset the receiver.              */

#define MCF5272_UART_UCR_RTX (0x03<<4)  /*   Write this  value  to  ucr  to */
                                        /* reset the transmitter.           */

#define MCF5272_UART_UCR_RES (0x04<<4)  /*   Write this  value  to  ucr  to */
                                        /* reset the error status.          */

#define MCF5272_UART_UCR_RBC (0x05<<4)  /*   Write this  value  to  ucr  to */
                                        /* reset    the    break     change */
                                        /* interrupt.                       */

#define MCF5272_UART_UCR_TXEN (1<<2)    /*   Write this  value  to  ucr  to */
                                        /* enable the transmitter.          */

#define MCF5272_UART_UCR_TXDE (1<<3)    /*   Write this  value  to  ucr  to */
                                        /* disable the transmitter.         */

#define MCF5272_UART_UCR_RXEN (1<<0)    /*   Write this  value  to  ucr  to */
                                        /* enable the receiver.             */

#define MCF5272_UART_UCR_RXDE (1<<1)    /*   Write this  value  to  ucr  to */
                                        /* disable the receiver.            */

                                        /*   Write this value to the ucr to */
                                        /* enablt   the   transmitter   and */
                                        /* receiver.                        */

#define MCF5272_UART_UCR_TXRXEN \
        (MCF5272_UART_UCR_TXEN | \
         MCF5272_UART_UCR_RXEN)

    u8_t urb_utb;                       /*   UART receiver buffers (R)  and */
    u8_t res4[3];                       /* UART transmitter buffers (W).    */

    u8_t uipcr_uacr;                    /*   UART   input    port    change */
    u8_t res5[3];                       /* register (R) and UART  auxiliary */
                                        /* control register (W).            */

    u8_t uisr_uimr;                     /*   UART interrupt status register */
    u8_t res6[3];                       /* (R)  and  UART  interrupt   mask */
                                        /* register (W).                    */

    u8_t udu;                           /*   UART  divider  upper  register */
    u8_t res7[3];                       /* (W).                             */

    u8_t udl;                           /*   UART  divider  lower  register */
    u8_t res8[3];                       /* (W).                             */

    u8_t uabu;                          /*   UART  autobaud  register   MSB */
    u8_t res9[3];                       /* (R).                             */

    u8_t uabl;                          /*   UART  autobaud  register   LSB */
    u8_t res10[3];                      /* (R).                             */

    u8_t utf;                           /*   UART     transmitter      FIFO */
    u8_t res11[3];                      /* register.                        */

#define MCF5272_UART_UTF_TXB (0x1F)     /*   Transmitter buffer data level. */
                                        /* Indicates the  number  of  bytes */
                                        /* (0-24) currently  stored in  the */
                                        /* transmitter FIFO.                */

    u8_t urf;                           /*   UART receiver FIFO register.   */
    u8_t res12[3];

    u8_t ufpd;                          /*   UART   Fractional    Precision */
    u8_t res13[3];                      /* Divider Control register.        */

    u8_t uip;                           /*   UART input port register (CTS) */
    u8_t res14[3];                      /* (R).                             */

    u8_t uop1;                          /*   UART  output   port  bit   set */
    u8_t res15[3];                      /* command register (RTS) (W).      */

    u8_t uop0;                          /*   UART  output  port  bit  reset */
    u8_t res16[3];                      /* command register (RTS) (W).      */

} __attribute__ ((aligned (4), packed)) mcf5272_sim_uart_t;

/*      Timer Module                                                        */
typedef struct mcf5272_sim_timer_t
{

    u16_t tmr;                          /*   Timer Mode Register            */
    u16_t res1;

#define MCF5272_TIMER_TMR_PS 0xFF00     /*   Prescaler.    Programmed    to */
#define MCF5272_TIMER_TMR_PS_BIT 8      /* divide the clock input by values */
                                        /* from  1  to   256.   The   value */
                                        /* 0000_0000 divides  the clock  by */
                                        /* 1; the  value 1111_1111  divides */
                                        /* the clock by 256.                */

#define MCF5272_TIMER_TMR_CE 0x00C0     /*   Capture   edge   and    enable */
#define MCF5272_TIMER_TMR_CE_BIT 6      /* interrupt.  00  Disable  capture */
                                        /* and interrupt  on capture  event */
                                        /* 01 Capture on  rising edge  only */
                                        /* and   generate   interrupt    on */
                                        /* capture  event  10  Capture   on */
                                        /* falling edge  only and  generate */
                                        /* interrupt on  capture  event  11 */
                                        /* Capture on any edge and generate */
                                        /* interrupt on capture event.      */

#define MCF5272_TIMER_TMR_OM 0x0020     /*   Output  mode  (TMR0  and  TMR1 */
#define MCF5272_TIMER_TMR_OM_BIT 5      /* only.   Reserved  in  TMR2   and */
                                        /* TMR3).  0  Active-low pulse  for */
                                        /* one system clock cycle (15 nS at */
                                        /* 66 MHz).  1 Toggle output; TOUTn */
                                        /* is  high   at   reset   but   is */
                                        /* unavailable externally until the */
                                        /* appropriate     port     control */
                                        /* register is configured for  this */
                                        /* function.                        */

#define MCF5272_TIMER_TMR_ORI 0x0010    /*   Output   reference   interrupt */
#define MCF5272_TIMER_TMR_ORI_BIT 4     /* enable.  0 Disable interrupt for */
                                        /* reference  reached   (does   not */
                                        /* affect  interrupt   on   capture */
                                        /* function).  1  Enable  interrupt */
                                        /* upon  reaching   the   reference */
                                        /* value.  If ORI  is  1  when  the */
                                        /* TER[REF] is  set,  an  immediate */
                                        /* interrupt occurs.                */

#define MCF5272_TIMER_TMR_FRR 0x0008    /*   Free run/restart.  0 Free run. */
#define MCF5272_TIMER_TMR_FRR_BIT 3     /* Timer   count    continues    to */
                                        /* increment  after  the  reference */
                                        /* value is  reached.   1  Restart. */
                                        /* Timer count is reset immediately */
                                        /* after  the  reference  value  is */
                                        /* reached.                         */

#define MCF5272_TIMER_TMR_CLK 0x0006    /*   Input  clock  source  for  the */
#define MCF5272_TIMER_TMR_CLK_BIT 1     /* timer.   00   Stop  count.    01 */
                                        /* Master system clock.  10  Master */
                                        /* system  clock  divided  by   16. */
                                        /* TIN0 and TIN1  are  external  to */
                                        /* the   MCF5272   and   are    not */
                                        /* synchronized   to   the   system */
                                        /* clock,  so  successive   timeout */
                                        /* lengths may  vary slightly.   11 */
                                        /* Corresponding TIN  pin, TIN0  or */
                                        /* TIN1 (falling  edge), unused  in */
                                        /* TMR2 and TMR3.  The minimum high */
                                        /* and low periods  for TIN as  the */
                                        /* clock source is 1 system  clock, */
                                        /* which  gives   a   maximum   TIN */
                                        /* frequency of clock/2.            */

#define MCF5272_TIMER_TMR_RST 0x0001    /*   Reset timer.   0 A  transition */
#define MCF5272_TIMER_TMR_RST_BIT 0     /* from 1  to 0  resets the  timer. */
                                        /* Other  register  values  can  be */
                                        /* written.                     The */
                                        /* counter/timer/prescaler are  not */
                                        /* clocked  unless  the  timer   is */
                                        /* enabled.  1 Enable timer.        */

    u16_t trr;                          /*   Timer Reference Register       */
    u16_t res2;

    u16_t tcap;                         /*   Timer Capture Register         */
    u16_t res3;

    u16_t tcn;                          /*   Timer Counter                  */
    u16_t res4;

    u16_t ter;                          /*   Timer Event Register           */
    u16_t res5;

#define MCF5272_TIMER_TER_REF (0x0002)  /*   Output  reference  event.   If */
#define MCF5272_TIMER_TER_REF_BIT (1)   /* the bit  is 0,  the counter  has */
                                        /* not  reached   the  TRR   value; */
                                        /* otherwise,   the   counter   has */
                                        /* reached the TRR value.   Writing */
                                        /* a 1 clears this bit.             */

#define MCF5272_TIMER_TER_CAP (0x0001)  /*   Captuer event.  If this bit is */
#define MCF5272_TIMER_TER_CAP_BIT (0)   /* 0, the  counter  value  has  not */
                                        /* been  latched   into  the   TCR; */
                                        /* otherwise, the counter value  is */
                                        /* latched into the TCR.  Writing a */
                                        /* 1 clears this bit.               */

    u32_t res6[3];

} __attribute__ ((aligned (4), packed)) mcf5272_sim_timer_t;

/*      Watchdog timer                                                      */
typedef struct mcf5272_sim_wdtmr_t
{

    u16_t wrrr;                         /*   Watchdog    reset    reference */
    u16_t res1;                         /* register.                        */

    u16_t wirr;                         /*   Watchdog  interrupt  reference */
    u16_t res2;                         /* register.                        */

    u16_t wcr;                          /*   Watchdog counter register.     */
    u16_t res3;

    u16_t wer;                          /*   Watchdog event register.       */
    u16_t res4;

    u32_t res5[28];

} __attribute__ ((aligned (4), packed)) mcf5272_sim_wdtmr_t;

/*      Ethernet Module                                                     */
typedef struct mcf5272_sim_enet_t
{

    u8_t res1[0x40];

    u32_t ecr;                          /*   Ethernet control register      */

    u32_t eir;                          /*   Interrupt event register       */

    u32_t eimr;                         /*   Interrupt mask register        */

    u32_t ivsr;                         /*   Interrupt    vector     status */
                                        /* register                         */

    u32_t rdar;                         /*   Receive   descriptor    active */
                                        /* register                         */

    u32_t tdar;                         /*   Transmit   descriptor   active */
                                        /* register                         */

    u8_t res2[0x0880-0x0858];

    u32_t mmfr;                         /*   MII management frame register  */

    u32_t mscr;                         /*   MII speed control register     */

    u8_t res3[0x08cc-0x0888];

    u32_t frbr;                         /*   FIFO receive bound register    */

    u32_t frsr;                         /*   FIFO receive start register    */

    u8_t res4[0x08e4-0x08d4];

    u32_t tfwr;                         /*   Transmit FIFO watermark        */

    u8_t res5[0x08ec-0x08e8];

    u32_t tfsr;                         /*   Transmit FIFO start register   */

    u8_t res6[0x0944-0x08f0];

    u32_t rcr;                          /*   Receive control register       */

    u32_t mflr;                         /*   Maximum frame length register  */

    u8_t res7[0x0984-0x094c];

    u32_t tcr;                          /*   Transmit control register      */

    u8_t res8[0x0c00-0x0988];

    u32_t malr;                         /*   Lower 32-bits of MAC address   */

    u32_t maur;                         /*   Upper 16-bits of MAC address   */

    u32_t htur;                         /*   Upper 32-bits of hash table    */

    u32_t htlr;                         /*   Lower 32-bits of hash table    */

    u32_t erdsr;                        /*   Pointer to receive  descriptor */
                                        /* ring                             */

    u32_t etdsr;                        /*   Pointer to transmit descriptor */
                                        /* ring                             */

    u32_t emrbr;                        /*   Maximum receive buffer size    */

    u8_t res9[0x0c40-0x0c1c];

    u8_t efifo[448];                    /*   FIFO RAM space                 */

    u8_t res10[0x1000-0x0e00];

} __attribute__ ((aligned (4), packed)) mcf5272_sim_enet_t;

/*      System  Integration  Module  (SIM)  This  structure  defines   each */
/* register's offset from the current value of the mbar register.           */
typedef struct mcf5272_sim_t
{

    mcf5272_sim_cfg_t cfg;              /*   0x0000: General  configuration */
                                        /* registers.                       */

    mcf5272_sim_int_t intc;             /*   0x0020:  Interrupt  controller */
                                        /* registers.                       */

    mcf5272_sim_cs_t cs[8];             /*   0x0040: Chip Select Module     */

    mcf5272_sim_gpio_t gpio;            /*   0x0080:  General  purpose  I/O */
                                        /* control registers                */

    u32_t qspi[8];                      /*   0x00a0:     Queued      serial */
                                        /* peripheral interface module.     */

    u32_t pwm[8];                       /*   0x00c0: Pulse Width Modulation */
                                        /* (PWM) Module                     */

    u32_t dmac[8];                      /*   0x00e0: DMA Controller         */

    mcf5272_sim_uart_t uart[2];         /*   0x0100: UART Modules           */

    u32_t sdramc[32];                   /*   0x0180: SDRAM Controller       */

    mcf5272_sim_timer_t timer[4];       /*   0x0200: Timer Module           */

    mcf5272_sim_wdtmr_t wdtimer;        /*   0x0280: Watchdog Timer Module  */

    u32_t plic[320];                    /*   0x0300:     Physical     Layer */
                                        /* Interface Controller             */

    mcf5272_sim_enet_t enet;            /*   0x0800: Ethernet Module        */

    u32_t usb[512];                     /*   0x1000: Universal Serial Bus   */

} __attribute__ ((aligned (4), packed)) mcf5272_sim_t;

#endif /* MCF5272_SIM_H */

