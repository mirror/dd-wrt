//==========================================================================
//
//        atmel_support.c
//
//        SA1110/iPAQ - Atmel micro-controller support routines
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2001-02-27
// Description:   Simple Atmel support
//####DESCRIPTIONEND####

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
#endif

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>       // Virtual vector functions
#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/hal_sa11x0.h>   // Board definitions
#include <cyg/hal/ipaq.h>
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_IO
// Need to export interrupt driven interfaces
#include <cyg/hal/drv_api.h>
#define ATMEL_INT CYGNUM_HAL_INTERRUPT_UART1
static cyg_interrupt atmel_interrupt;
static cyg_handle_t  atmel_interrupt_handle;
#endif

#include <cyg/hal/atmel_support.h>        // Interfaces, commands

externC void *memcpy(void *, const void *, size_t);

struct sa11x0_serial {
  volatile cyg_uint32 utcr0;
  volatile cyg_uint32 utcr1;
  volatile cyg_uint32 utcr2;
  volatile cyg_uint32 utcr3;
  volatile cyg_uint32 pad0010;
  volatile cyg_uint32 utdr;
  volatile cyg_uint32 pad0018;
  volatile cyg_uint32 utsr0;
  volatile cyg_uint32 utsr1;
};

#define ATMEL_PUTQ_SIZE 32
unsigned char atmel_putq[ATMEL_PUTQ_SIZE];
int atmel_putq_put, atmel_putq_get;

bool atmel_use_ints;

void
atmel_putc(unsigned char c)
{
    atmel_putq[atmel_putq_put++] = c;
    if (atmel_putq_put == ATMEL_PUTQ_SIZE) {
        atmel_putq_put = 0;
    }
}

static atmel_handler *handlers[NUM_ATMEL_CMDS];

static void
null_handler(atmel_pkt *pkt)
{
    diag_printf("Atmel - packet ignored: %x\n", pkt->data[0]);
    diag_dump_buf(pkt->data, 16);
}

//
// Register a handler for a particular packet type
//
void 
atmel_register(int cmd, atmel_handler *fun)
{
    handlers[cmd] = fun;
}

//
// This routine is called to process an input character from the
// Atmel micro-controller.  Since command packets are multiple
// characters, this routine has to keep state for the packet.  Once
// a complete packet is received, the appropriate handler function
// will be called (if the checksum matches)
//
static void
atmel_in(unsigned char ch)
{
    static atmel_pkt pkt;
    static unsigned char cksum;
    static bool sof = false;

    if (!sof) {
        // Wait for start of packet (SOF)
        if (ch != SOF) {
            return;  // Just ignore out of order characters
        }
        sof = true;
        pkt.size = 0;
        return;
    }
    if (pkt.size == 0) {
        // First byte of packet - command+length;
        pkt.len = (ch & 0x0F) + 1;
        cksum = 0;
    }
    pkt.data[pkt.size++] = ch;
    if (pkt.size > pkt.len) {
        // End of packet data
        if (cksum == ch) {
            (*handlers[pkt.data[0] >> 4])(&pkt);
        }
        sof = false;
    } else {
        cksum += ch;
    }
}

//
// This [internal] routine is used to handle data from the Atmel micro-controller
//
static void
atmel_poll(void)
{
    volatile struct sa11x0_serial *base = (volatile struct sa11x0_serial *)SA11X0_UART1_BASE;
    unsigned char ch;

    while ((base->utsr1 & SA11X0_UART_RX_FIFO_NOT_EMPTY) != 0) {
        ch = (char)base->utdr;
        atmel_in(ch);
    }
}

//
// This [internal] routine is used to send data to the Atmel micro-controller
//
static void
atmel_flush(void)
{
    volatile struct sa11x0_serial *base = (volatile struct sa11x0_serial *)SA11X0_UART1_BASE;

#ifdef CYGPKG_IO
    cyg_drv_isr_lock();
#endif
    while (atmel_putq_get != atmel_putq_put) {
        if ((base->utsr1 & SA11X0_UART_TX_FIFO_NOT_FULL) == 0) {
            // Wait forever if in non-interrupt mode
            if (atmel_use_ints) {
                break;
            }
        } else {
            base->utdr = atmel_putq[atmel_putq_get++];
            if (atmel_putq_get == ATMEL_PUTQ_SIZE) {
                atmel_putq_get = 0;
            }
        }
    }
#ifdef CYGPKG_IO
    cyg_drv_isr_unlock();
#endif
}

#ifdef CYGPKG_IO

// Low level interrupt handler (ISR)
static cyg_uint32 
atmel_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    cyg_drv_interrupt_acknowledge(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// High level interrupt handler (DSR)
static void       
atmel_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    volatile struct sa11x0_serial *base = (volatile struct sa11x0_serial *)SA11X0_UART1_BASE;
    unsigned int stat0 = base->utsr0;

    if (stat0 & SA11X0_UART_TX_SERVICE_REQUEST) {
        atmel_flush();
    }
    if (stat0 & (SA11X0_UART_RX_SERVICE_REQUEST|SA11X0_UART_RX_IDLE)) {
        atmel_poll();
        base->utsr0 = SA11X0_UART_RX_IDLE;  // Need to clear this manually
    }
    cyg_drv_interrupt_unmask(vector);
}

//
// Connect/disconnect interrupt processing
//
void
atmel_interrupt_mode(bool enable)
{
    volatile struct sa11x0_serial *base = (volatile struct sa11x0_serial *)SA11X0_UART1_BASE;

    if (enable) {
        // Enable the receiver (with interrupts) and the transmitter.
        base->utcr3 = SA11X0_UART_RX_ENABLED |
                      SA11X0_UART_TX_ENABLED |
                      SA11X0_UART_RX_FIFO_INT_ENABLED;
        cyg_drv_interrupt_unmask(ATMEL_INT);
    } else {
        cyg_drv_interrupt_mask(ATMEL_INT);
    }
    atmel_use_ints = enable;
}
#endif

//
// Set up the Atmel micro-controller environment
//
void
atmel_init(void)
{
    volatile struct sa11x0_serial *base = (volatile struct sa11x0_serial *)SA11X0_UART1_BASE;
    cyg_uint32 brd;
    int i;
    static bool _init = false;

    if (_init) return;

    // Disable Receiver and Transmitter (clears FIFOs)
    base->utcr3 = SA11X0_UART_RX_DISABLED | SA11X0_UART_TX_DISABLED;

    // Clear sticky (writable) status bits.
    base->utsr0 = SA11X0_UART_RX_IDLE | SA11X0_UART_RX_BEGIN_OF_BREAK |
                  SA11X0_UART_RX_END_OF_BREAK;

    // Set UART to 8N1 (8 data bits, no partity, 1 stop bit)
    base->utcr0 = SA11X0_UART_PARITY_DISABLED | SA11X0_UART_STOP_BITS_1 |
                  SA11X0_UART_DATA_BITS_8;

    // Set the desired baud rate.
    brd = SA11X0_UART_BAUD_RATE_DIVISOR(115200);
    base->utcr1 = (brd >> 8) & SA11X0_UART_H_BAUD_RATE_DIVISOR_MASK;
    base->utcr2 = brd & SA11X0_UART_L_BAUD_RATE_DIVISOR_MASK;

    // Enable the receiver and the transmitter.
    base->utcr3 = SA11X0_UART_RX_ENABLED | SA11X0_UART_TX_ENABLED;

    // Set up character queues
    atmel_putq_put = atmel_putq_get = 0;
    atmel_use_ints = false;

    // Set up handlers
    for (i = 0;  i < NUM_ATMEL_CMDS;  i++) {
        atmel_register(i, null_handler);
    }

#ifdef CYGPKG_IO
    cyg_drv_interrupt_create(ATMEL_INT,
                             99,                     // Priority - unused
                             (cyg_addrword_t)base,   // Data item passed to interrupt handler
                             atmel_ISR,
                             atmel_DSR,
                             &atmel_interrupt_handle,
                             &atmel_interrupt);
    cyg_drv_interrupt_attach(atmel_interrupt_handle);
#endif

    _init = true;
}

static void
atmel_pkt_send(atmel_pkt *pkt)
{
    int i;

    for (i = 0;  i < pkt->len;  i++) {
        atmel_putc(pkt->data[i]);
    }
    atmel_flush();
}

bool
atmel_send(int cmd, unsigned char *data, int len)
{
    atmel_pkt pkt;
    unsigned char cksum, *dp;
    int i;

    dp = pkt.data;
    *dp++ = SOF;
    *dp++ = cksum = (cmd << 4) | len;
    for (i = 0;  i < len;  i++) {
        *dp++ = data[i];
        cksum += data[i];
    }
    *dp = cksum;
    pkt.len = len + 3;
    atmel_pkt_send(&pkt);
    return true;
}

#define MAX_TS_EVENTS      32
static struct ts_event ts_event_list[MAX_TS_EVENTS];
static int num_ts_events = 0;
static int ts_event_get, ts_event_put;

static void
ts_event_handler(atmel_pkt *pkt)
{
    unsigned char *buf = pkt->data;
    static bool up = true;
    static int down_count = 0;
    struct ts_event *tse;

    if (num_ts_events == MAX_TS_EVENTS) {
        return;
    }
    if ((buf[0] & 0x0F) == 0) {
        // Stylus up
        up = true;
        down_count = 0;
        tse = &ts_event_list[ts_event_put++];
        if (ts_event_put == MAX_TS_EVENTS) {
            ts_event_put = 0;
        }
        num_ts_events++;
        tse->x = (buf[1] << 8) | buf[2];
        tse->y = (buf[3] << 8) | buf[4];
        tse->up = up;
    } else {
        up = false;
        if (down_count++ == 0) {
            // First 'down' event
        } else {
        }
        {
            tse = &ts_event_list[ts_event_put++];
            if (ts_event_put == MAX_TS_EVENTS) {
                ts_event_put = 0;
            }
            num_ts_events++;
            tse->x = (buf[1] << 8) | buf[2];
            tse->y = (buf[3] << 8) | buf[4];
            tse->up = up;
        }
    }
}

bool
ts_get_event(struct ts_event *tse)
{
    static bool _init = false;

    if (!_init) {
        atmel_register(ATMEL_CMD_TOUCH, ts_event_handler);
        atmel_register(ATMEL_CMD_UNKNOWN, ts_event_handler);
        _init = true;
    }

    if (num_ts_events == 0) {
        atmel_poll();
    }
    if (num_ts_events) {
        num_ts_events--;
        memcpy(tse, &ts_event_list[ts_event_get++], sizeof(*tse));
        if (ts_event_get == MAX_TS_EVENTS) {
            ts_event_get = 0;
        }
        return true;
    } else {
        return false;
    }
}

#define MAX_KEY_EVENTS     8
static struct key_event key_event_list[MAX_KEY_EVENTS];
static int num_key_events = 0;
static int key_event_get, key_event_put;

static void
kbd_event_handler(atmel_pkt *pkt)
{
    unsigned char *buf = pkt->data;
    struct key_event *ke;

    if (num_key_events == MAX_KEY_EVENTS) {
        return;
    }
//    printf("Keybd = %x\n", buf[1]);
    ke = &key_event_list[key_event_put++];
    if (key_event_put == MAX_KEY_EVENTS) {
        key_event_put = 0;
    }
    ke->button_info = buf[1];
    num_key_events++;
}

bool
key_get_event(struct key_event *ke)
{
    static bool _init = false;

    if (!_init) {
        atmel_register(ATMEL_CMD_KEYBD, kbd_event_handler);
        _init = true;
    }

    if (num_key_events == 0) {
        atmel_poll();
    }
    if (num_key_events) {
        num_key_events--;
        memcpy(ke, &key_event_list[key_event_get++], sizeof(*ke));
        if (key_event_get == MAX_KEY_EVENTS) {
            key_event_get = 0;
        }
        return true;
    } else {
        return false;
    }
}

#ifdef CYGPKG_REDBOOT

void
atmel_check(bool is_idle)
{
}
RedBoot_idle(atmel_check, RedBoot_BEFORE_NETIO);
#endif
