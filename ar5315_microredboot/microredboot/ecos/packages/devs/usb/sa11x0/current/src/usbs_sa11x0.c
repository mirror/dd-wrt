//==========================================================================
//
//      usbs_sa11x0.c
//
//      Device driver for the SA11x0 USB port.
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
// Author(s):    bartv
// Contributors: bartv
// Date:         2000-10-04
//
// This code implements support for the on-chip USB port on the SA11x0
// family of processors. The code has been developed on the SA1110 and
// may or may not work on other members of the SA11x0 family. There
// have problems with the USB support on certain revisions of the silicon,
// so the errata sheet appropriate to the specific processor being used
// should be consulted. There also appear to be problems which do not
// appear on any errata, which this code attempts to work around.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>

#include <pkgconf/hal_arm.h>
#include <pkgconf/devs_usb_sa11x0.h>

#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_sa11x0.h>
#include <cyg/error/codes.h>

#include <cyg/io/usb/usb.h>
#include <cyg/io/usb/usbs.h>

// Debugging support. By default this driver operates mostly at
// DSR level, with the ISR doing a minimal amount of processing.
// However is also possible to run most of the code at thread-level,
// This is subject to some restrictions because the USB standard
// imposes timing constraints, e.g. some control operations such
// as SET-ADDRESS have to complete within 50ms. However it is
// very useful for debugging, specifically it allows you to put
// printf()'s in various places.
//
// Right now these configuration options are not exported to the
// user because running at DSR level is likely to be good enough
// for everybody not actively debugging this code. The options
// could be exported if necessary.
//#define CYGPKG_DEVS_USB_SA11X0_THREAD
#undef CYGPKG_DEVS_USB_SA11X0_THREAD
#ifdef CYGPKG_DEVS_USB_SA11X0_THREAD
  // Default stack size should be CYGNUM_HAL_STACK_SIZE_TYPICAL
# define CYGNUM_DEVS_USB_SA11X0_THREAD_STACK_SIZE       4096
# define CYGNUM_DEVS_USB_SA11X0_THREAD_PRIORITY         7
# include <cyg/kernel/kapi.h>
#endif

#if 0
# define DBG(a) diag_printf a
#else
# define DBG(a)
#endif

#undef FAILURES
#ifdef FAILURES
static volatile int ep1_failure = 7;
#endif

#undef STATS
#ifdef STATS
int ep1_receives = 0;
int ep1_errors = 0;
int ep2_transmits = 0;
int ep2_errors = 0;
# define INCR_STAT(a) (a) += 1
# define SET_STAT(a, b) (a) = (b)
#else
# define INCR_STAT(a)
# define SET_STAT(a, b)
#endif

// ----------------------------------------------------------------------------
// Serial port 0 on the SA11x0 provides a USB slave connection (aka a
// USB device controller or UDC). The functionality is somewhat
// limited, there are just three endpoints.
//
// Endpoint 0 can only be used for control messages. It has an 8 byte
// fifo which cannot be connected to a DMA engine. Hence incoming
// control packets have to be limited to 8 bytes by the enumeration
// data. The endpoint has to be managed at a low-level, i.e. the
// incoming request has to be extracted from the fifo, processed, and
// any response put back into the fifo within the permitted USB
// response times.
//
// Endpoint 1 can only be used for host->slave bulk OUT transfers. It
// has a 20 byte receive fifo, and it can be hooked up to any of the
// six DMA engines. Since bulk transfers will typically involve 64
// byte packets, most applications will require the use of DMA.
//
// Endpoint 2 can only be used for slave-host bulk IN transfers. There
// is a 16 byte transmit fifo so small messages can be transferred in
// software. The fifo can also be hooked up to DMA, which is a more
// likely scenario.
//
// Start with definitions of the hardware. The use of a structure and
// a const base pointer should allow the compiler to do base/offset
// addressing and keep the hardware base address in a register. This
// is better than defining each hardware register via a separate
// address. Although the registers are only a byte wide, the peripheral
// bus only supports word accesses.
//
// The USBS_CONTROL etc. macros allow for an alternative way of
// accessing the hardware if a better approach is presented, without
// having to rewrite all the code. Macros that correspond to registers
// are actually addresses, making it easier in the code to distinguish
// them from bit values: the & and * operators will just cancel out.

typedef struct usbs_sa11x0_hardware {
    volatile int control;
    volatile int address;
    volatile int out_size;
    volatile int in_size;
    volatile int ep0_control;
    volatile int ep1_control;
    volatile int ep2_control;
    volatile int ep0_data;
    volatile int ep0_write_count;
             int dummy1;
    volatile int fifo;
             int dummy2;
    volatile int status;
} usbs_sa11x0_hardware;

static usbs_sa11x0_hardware* const usbs_sa11x0_base = (usbs_sa11x0_hardware* const) 0x80000000;
#define USBS_CONTROL    (&(usbs_sa11x0_base->control))
#define USBS_ADDRESS    (&(usbs_sa11x0_base->address))
#define USBS_OUT_SIZE   (&(usbs_sa11x0_base->out_size))
#define USBS_IN_SIZE    (&(usbs_sa11x0_base->in_size))
#define EP0_CONTROL     (&(usbs_sa11x0_base->ep0_control))
#define EP1_CONTROL     (&(usbs_sa11x0_base->ep1_control))
#define EP2_CONTROL     (&(usbs_sa11x0_base->ep2_control))
#define EP0_DATA        (&(usbs_sa11x0_base->ep0_data))
#define EP0_WRITE_COUNT (&(usbs_sa11x0_base->ep0_write_count))
#define EP1_DATA        (&(usbs_sa11x0_base->fifo))
#define EP2_DATA        (&(usbs_sa11x0_base->fifo))
#define USBS_STATUS     (&(usbs_sa11x0_base->status))

#define CONTROL_DISABLE                 (1 << 0)
#define CONTROL_ACTIVE                  (1 << 1)
// The meaning of bit 2 changed, see errata
#define CONTROL_RESUME_INTR             (1 << 2)
#define CONTROL_EP0_INTR                (1 << 3)
#define CONTROL_EP1_INTR                (1 << 4)
#define CONTROL_EP2_INTR                (1 << 5)
// The meaning of bit 6 also changed, see errata
#define CONTROL_SUSPEND_INTR            (1 << 6)
#define CONTROL_RESET_INTR              (1 << 7)

// Getting the control register settings right is a little bit tricky.
// Bit 0 is the disable bit so touching that is dangerous, and the
// other bits have inverted meanings i.e. 0 enables interrupts. The
// following macro encapsulates this.
#define CONTROL_ALL_INTR                0x00FC
#define CONTROL_INTR_ENABLE(bits)  ((~(bits)) & CONTROL_ALL_INTR)
#define CONTROL_INTR_CLEAR(bits)   ((bits) & CONTROL_ALL_INTR)

// All the endpoint interrupt numbers can be handled en masse,
// but some of the endpoints may be disabled.
#if defined(CYGPKG_DEVS_USB_SA11X0_EP1) && defined(CYGPKG_DEVS_USB_SA11X0_EP2)
# define CONTROL_EP_INTR_BITS      (CONTROL_EP0_INTR | CONTROL_EP1_INTR | CONTROL_EP2_INTR)
#elif defined(CYGPKG_DEVS_USB_SA11X0_EP1)
# define CONTROL_EP_INTR_BITS      (CONTROL_EP0_INTR | CONTROL_EP1_INTR)
#elif defined(CYGPKG_DEVS_USB_SA11X0_EP2)
# define CONTROL_EP_INTR_BITS      (CONTROL_EP0_INTR | CONTROL_EP2_INTR)
#else
# define CONTROL_EP_INTR_BITS      (CONTROL_EP0_INTR)
#endif

#define EP0_OUT_READY                   (1 << 0)
#define EP0_IN_READY                    (1 << 1)
#define EP0_SENT_STALL                  (1 << 2)
#define EP0_FORCE_STALL                 (1 << 3)
#define EP0_DATA_END                    (1 << 4)
#define EP0_SETUP_END                   (1 << 5)
#define EP0_SERVICED_OPR                (1 << 6)
#define EP0_SERVICED_SETUP_END          (1 << 7)

#define EP1_FIFO_SERVICE                (1 << 0)
#define EP1_PACKET_COMPLETE             (1 << 1)
#define EP1_PACKET_ERROR                (1 << 2)
#define EP1_SENT_STALL                  (1 << 3)
#define EP1_FORCE_STALL                 (1 << 4)
#define EP1_FIFO_NOT_EMPTY              (1 << 5)

#define EP2_FIFO_SERVICE                (1 << 0)
#define EP2_PACKET_COMPLETE             (1 << 1)
#define EP2_PACKET_ERROR                (1 << 2)
#define EP2_PACKET_UNDERRUN             (1 << 3)
#define EP2_SENT_STALL                  (1 << 4)
#define EP2_FORCE_STALL                 (1 << 5)

#define STATUS_EP0_INTR                 (1 << 0)
#define STATUS_EP1_INTR                 (1 << 1)
#define STATUS_EP2_INTR                 (1 << 2)
#define STATUS_SUSPEND_INTR             (1 << 3)
#define STATUS_RESUME_INTR              (1 << 4)
#define STATUS_RESET_INTR               (1 << 5)

#define EP0_FIFO_SIZE                     8
#define EP0_MTU                           8

#define EP1_FIFO_SIZE                   20
#ifdef CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL
# define EP1_MTU                        64
#else
# define EP1_MTU                        16
#endif

#define EP2_FIFO_SIZE                   16
#ifdef CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL
# define EP2_MTU                        64
#else
# define EP2_MTU                        16
#endif

#if defined(CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL) || defined(CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL)
typedef struct usbs_sa11x0_dma {
    volatile int                address;
    volatile int                control_set;
    volatile int                control_clear;
    volatile int                status;
    volatile int                buf_a_address;  // Absolute, not remapped
    volatile int                buf_a_size;
    volatile int                buf_b_address;  // Absolute, not remapped
    volatile int                buf_b_size;
} usbs_sa11x0_dma;

#define DMA_CONTROL_RUN                 (1 << 0)
#define DMA_CONTROL_INTR_ENABLE         (1 << 1)
#define DMA_STATUS_ERROR                (1 << 2)
#define DMA_STATUS_DONE_A               (1 << 3)
#define DMA_CONTROL_START_A             (1 << 4)
#define DMA_STATUS_DONE_B               (1 << 5)
#define DMA_CONTROL_START_B             (1 << 6)
#define DMA_STATUS_BUFFER_IN_USE        (1 << 7)
// All the bits that are useful to clear. BUFFER_IN_USE is read-only.
#define DMA_CONTROL_CLEAR_ALL           (DMA_CONTROL_RUN | DMA_CONTROL_INTR_ENABLE | DMA_STATUS_ERROR | \
                                         DMA_STATUS_DONE_A | DMA_CONTROL_START_A | DMA_STATUS_DONE_B | DMA_CONTROL_START_B)

// The DMA engines operate eight-bytes at a time. This affects issues
// such as alignment.
#define DMA_BURST_SIZE                  8

// The DMA engines bypass the cache and MMU, accessing physical
// memory directly. Newer HALS should provide appropriate macros.
#ifndef HAL_VIRT_TO_PHYS_ADDRESS
# error HAL macros for translating between virtual and physical memory are required.
#endif

// Make absolutely sure that the two endpoints use different
// DMA channels. Right now this check cannot be done easily
// at the CDL level.
# if defined(CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL) && defined(CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL)
#  if (CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL == CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL)
#   error Different DMA channels must be selected for the two endpoints.
#  endif
# endif

# ifdef CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL
static usbs_sa11x0_dma* const ep1_dma_base = (usbs_sa11x0_dma* const)(0xB0000000 | (0x20 * CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL));
#  define EP1_DMA_ADDRESS        (&(ep1_dma_base->address))
#  define EP1_DMA_CONTROL_SET    (&(ep1_dma_base->control_set))
#  define EP1_DMA_CONTROL_CLEAR  (&(ep1_dma_base->control_clear))
#  define EP1_DMA_STATUS         (&(ep1_dma_base->status))
#  define EP1_DMA_BUF_A_ADDRESS  (&(ep1_dma_base->buf_a_address))
#  define EP1_DMA_BUF_A_SIZE     (&(ep1_dma_base->buf_a_size))
#  define EP1_DMA_BUF_B_ADDRESS  (&(ep1_dma_base->buf_b_address))
#  define EP1_DMA_BUF_B_SIZE     (&(ep1_dma_base->buf_b_size))

// The correct value for the DMA address register is fixed for USB transfers
// See table 11.6 of the SA1110 Advanced Developer's Manual
//   Device datum width == 1 byte
//   Device burst size  == 8 bytes
//   Device transfer direction == read (device->memory)
//   Endianness is controlled by the ARM architectural HAL package
#  ifdef CYGHWR_HAL_ARM_BIGENDIAN
#   define EP1_DMA_ADDRESS_VALUE        (0x80000A00 | 0x10 | 0x0 | 0x4 | 0x2 | 0x1)
#  else
#   define EP1_DMA_ADDRESS_VALUE        (0x80000A00 | 0x10 | 0x0 | 0x4 | 0x0 | 0x1)
#  endif
# endif // EP1_DMA

# ifdef CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL

static usbs_sa11x0_dma* const ep2_dma_base = (usbs_sa11x0_dma* const)(0xB0000000 | (0x20 * CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL));
#  define EP2_DMA_ADDRESS        (&(ep2_dma_base->address))
#  define EP2_DMA_CONTROL_SET    (&(ep2_dma_base->control_set))
#  define EP2_DMA_CONTROL_CLEAR  (&(ep2_dma_base->control_clear))
#  define EP2_DMA_STATUS         (&(ep2_dma_base->status))
#  define EP2_DMA_BUF_A_ADDRESS  (&(ep2_dma_base->buf_a_address))
#  define EP2_DMA_BUF_A_SIZE     (&(ep2_dma_base->buf_a_size))
#  define EP2_DMA_BUF_B_ADDRESS  (&(ep2_dma_base->buf_b_address))
#  define EP2_DMA_BUF_B_SIZE     (&(ep2_dma_base->buf_b_size))

#  ifdef CYGHWR_HAL_ARM_BIGENDIAN
#   define EP2_DMA_ADDRESS_VALUE        (0x80000A00 | 0x00 | 0x0 | 0x4 | 0x2 | 0x0)
#  else
#   define EP2_DMA_ADDRESS_VALUE        (0x80000A00 | 0x00 | 0x0 | 0x4 | 0x0 | 0x0)
#  endif
# endif // EP2_DMA

#endif  // EP1_DMA || EP2_DMA

// ----------------------------------------------------------------------------
// Static data. There is a data structure for each endpoint. The
// implementation is essentially a private class that inherits from
// common classes for control and data endpoints, but device drivers
// are supposed to be written in C so some ugliness is required.
//
// Devtab entries are defined in usbs_sa11x0_data.cxx to make sure
// that the linker does not garbage-collect them.

// Support for the interrupt handling code.
static cyg_interrupt usbs_sa11x0_intr_data;
static cyg_handle_t  usbs_sa11x0_intr_handle;
static volatile int  isr_status_bits    = 0;

// Endpoint 0 is always present, this module would not get compiled
// otherwise.
static void usbs_sa11x0_ep0_start(usbs_control_endpoint*);
static void usbs_sa11x0_poll(usbs_control_endpoint*);

typedef enum ep0_state {
    EP0_STATE_IDLE      = 0,
    EP0_STATE_IN        = 1,
    EP0_STATE_OUT       = 2
} ep0_state;

typedef struct ep0_impl {
    usbs_control_endpoint   common;
    ep0_state               ep_state;
    int                     length;
    int                     transmitted;
} ep0_impl;

static ep0_impl ep0 = {
    common:
    {
        state:                  USBS_STATE_POWERED, // The hardware does not distinguish  between detached, attached and powered.
        enumeration_data:       (usbs_enumeration_data*) 0,
        start_fn:               &usbs_sa11x0_ep0_start,
        poll_fn:                &usbs_sa11x0_poll,
        interrupt_vector:       SA11X0_IRQ_USB_SERVICE_REQUEST,
        control_buffer:         { 0, 0, 0, 0, 0, 0, 0, 0 },
        state_change_fn:        (void (*)(usbs_control_endpoint*, void*, usbs_state_change, int)) 0,
        state_change_data:      (void*) 0,
        standard_control_fn:    (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        standard_control_data:  (void*) 0,
        class_control_fn:       (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        class_control_data:     (void*) 0,
        vendor_control_fn:      (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        vendor_control_data:    (void*) 0,
        reserved_control_fn:    (usbs_control_return (*)(usbs_control_endpoint*, void*)) 0,
        reserved_control_data:  (void*) 0,
        buffer:                 (unsigned char*) 0,
        buffer_size:            0,
        fill_buffer_fn:         (void (*)(usbs_control_endpoint*)) 0,
        fill_data:              (void*) 0,
        fill_index:             0,
        complete_fn:            (usbs_control_return (*)(usbs_control_endpoint*, int)) 0
    },
    ep_state:           EP0_STATE_IDLE,
    length:             0,
    transmitted:        0
};

extern usbs_control_endpoint usbs_sa11x0_ep0 __attribute__((alias ("ep0")));

// Endpoint 1 is optional. If the application only involves control
// messages or only slave->host transfers then the endpoint 1
// support can be disabled.
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1

typedef struct ep1_impl {
    usbs_rx_endpoint    common;
    int                 fetched;
    cyg_bool            using_buf_a;
} ep1_impl;

static void ep1_start_rx(usbs_rx_endpoint*);
static void ep1_set_halted(usbs_rx_endpoint*, cyg_bool);

static ep1_impl ep1 = {
    common: {
        start_rx_fn:        &ep1_start_rx,
        set_halted_fn:      &ep1_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    },
    fetched:            0,
    using_buf_a:        0
};

extern usbs_rx_endpoint usbs_sa11x0_ep1 __attribute__((alias ("ep1")));
#endif

// Endpoint 2 is optional. If the application only involves control
// messages or only host->slave transfers then the endpoint 2 support
// can be disabled.
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2

typedef struct ep2_impl {
    usbs_tx_endpoint        common;
    int                     transmitted;
    int                     pkt_size;
} ep2_impl;

static void ep2_start_tx(usbs_tx_endpoint*);
static void ep2_set_halted(usbs_tx_endpoint*, cyg_bool);

static ep2_impl ep2 = {
    common: {
        start_tx_fn:        &ep2_start_tx,
        set_halted_fn:      &ep2_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (const unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    }, 
    transmitted:        0,
    pkt_size:           0
};

extern usbs_tx_endpoint usbs_sa11x0_ep2 __attribute__ ((alias ("ep2")));

#endif

// ----------------------------------------------------------------------------
// Hardware problem: experiments indicate that manipulating the USB
// controller registers does not always work as expected. The control
// fifo is especially badly affected, with e.g. writes just being lost
// completely. It is necessary to work around these problems using
// retry loops. MAX_RETRIES controls the total number of attempts to
// access a register. MAX_CHECKS controls the number of times a
// register is checked to determine whether or not the attempt has
// been succesful. These constants are used to access the data fifo,
// so MAX_RETRIES has to be > 20 bytes.
#define MAX_RETRIES     32
#define MAX_CHECKS       8

// Write one or more bits to a register. This should result in some
// bits ending up set and other bits ending up clear. Some register
// bits are write-1-to-clear or may have side effects.
static cyg_bool
usbs_sa11x0_poke(volatile int* addr, int value, int should_be_set, int should_be_clear)
{
    cyg_bool    result  = false;
    int         retries, checks;
        
    for (retries = 0; !result && (retries < MAX_RETRIES); retries++) {
        *addr = value;
        (void) *addr;   // The first read is always invalid.
        for (checks = 0; !result && (checks < MAX_CHECKS); checks++) {
            int current_value = *addr;
            if (should_be_set != (should_be_set & current_value)) {
                continue;
            }
            if ((0 != should_be_clear) && (0 != (should_be_clear & current_value))) {
                continue;
            }
            result = true;
        }
    }
    if (!result) {
        DBG(("usbs_sa11x0_poke failed: addr %x, value %x, should_be_set %x, should_be_clear %x, actual %x\n", \
             (int) addr, value, should_be_set, should_be_clear, *addr));
    }
    return result;
}

// Write a whole value to a register, rather than just manipulating
// individual bits.
static cyg_bool
usbs_sa11x0_poke_value(volatile int* addr, int value)
{
    cyg_bool    result  = false;
    int         retries, checks;
        
    for (retries = 0; !result && (retries < MAX_RETRIES); retries++) {
        *addr = value;
        (void) *addr;   // The first read is always invalid.
        for (checks = 0; !result && (checks < MAX_CHECKS); checks++) {
            if (value == *addr) {
                result = true;
            }
        }
    }
    if (!result) {
        DBG(("usbs_sa11x0_poke_value failed: addr %x, value %x, actual %x\n", (int) addr, value, *addr));
    }
    return result;
}


// ----------------------------------------------------------------------------
// Control transfers
//
// Endpoint 0 is rather more complicated than the others. This is
// partly due to the nature of the control protocol, for example it is
// bidirectional and transfer sizes are unpredictable.
//
// The USB standard imposes some timing constraints on endpoint 0, see
// section 9.2.6 of the spec. For example the set-address operation is
// supposed to take at most 50ms. In general the timings are reasonably
// generous so no special action is taken here. There could be problems
// when debugging, but that is pretty much inevitable.
//
// It is necessary to maintain a state for the control endpoint, the
// default state being idle. Control operations involve roughly the
// following sequence of events:
//
// 1) the host transmits a special setup token, indicating the start
//    of a control operation and possibly cancelling any existing control
//    operation that may be in progress. USB peripherals cannot NAK this
//    even if they are busy.
//
// 2) the setup operation is followed by an eight-byte packet from the host
//    that describes the specific control operation. This fits into the
//    SA11X0's eight-byte control fifo. There will be an endpoint 0
//    interrupt with the out-packet-ready bit set. If the setup token
//    was sent while a previous control operation was also in progress
//    then the setup-end bit will be set as well.
//
// 3) the eight-byte packet is described in section 9.3 of the USB spec.
//    The first byte holds three fields, with the top bit indicating the
//    direction of subsequent data transfer. There are also two bytes
//    specifying the size of the subsequent transfer. Obviously the
//    packet also contains information such as the request type.
//
//    If the specified size is zero then the endpoint will remain in
//    its idle state. Otherwise the endpoint will switch to either
//    IN or OUT state, depending on the direction of subsequent
//    transfers.
// 
// 4) some standard control operations can be handled by the code
//    here. Set-address involves poking the address register and
//    a change of state. Set-feature and clear-feature on the
//    data endpoints can be used in conjunction with endpoint-halt.
//    Get-status on the data endpoints tests the halt condition.
//    It is also possible for the hardware-specific code to
//    implement set-feature, clear-feature and get-status
//    for the device as a whole since the SA11x0 always has to
//    be self-powered and is incapable of initiating a remote
//    wakeup.
//
//    Other standard control operations will be handled by the
//    application-specific installed handler, if any, or by the
//    default handler usbs_handle_standard_control(). Class-specific
//    and vendor-specific functions require appropriate handlers to be
//    installed as well, If a particular request is not recognized
//    then a stall condition should be raised. This will not prevent
//    subsequent control operations, just the current one.
//
//    Data transfers on endpoint 0 involve at most eight bytes at
//    a time. More data will only be accepted if the out-packet-ready
//    bit has been cleared via the serviced-opr bit, with the
//    hardware nak'ing OUT requests. To send data back to the host
//    the FIFO should be filled and then the in-packet-ready bit
//    should be set.
//
// It looks like processing all control packets at DSR level should be
// sufficient. During the data phase the hardware will NAK IN and
// OUT requests if the fifo is still empty/full, so timing is not
// an issue. Timing after receipt of the initial control message
// may be more important, e.g. the 50ms upper limit on processing
// the set-address control message, but this should still be ok.
// This decision may have to be re-examined in the light of
// experience.

// Init may get called during system startup or following a reset.
// During startup no work is needed since the hardware will
// have been reset and everything should be fine. After a reset
// the hardware will also be ok but there may be state information
// in ep0 that needs to be reset.
static void
usbs_sa11x0_ep0_init(void)
{
    if ((EP0_STATE_IDLE != ep0.ep_state) &&
        ((usbs_control_return (*)(usbs_control_endpoint*, int)) 0 != ep0.common.complete_fn)) {
        (*ep0.common.complete_fn)(&ep0.common, -EPIPE);
    }
    ep0.common.state            = USBS_STATE_POWERED;
    memset(ep0.common.control_buffer, 0, 8);
    ep0.common.buffer           = (unsigned char*) 0;
    ep0.common.buffer_size      = 0;
    ep0.common.fill_buffer_fn   = (void (*)(usbs_control_endpoint*)) 0;
    ep0.common.fill_data        = (void*) 0;
    ep0.common.fill_index       = 0;
    ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
    ep0.ep_state                = EP0_STATE_IDLE;
    ep0.length                  = 0;
    ep0.transmitted             = 0;
}

// The start function is called by higher-level code when things have
// been set up, i.e. the enumeration data is available, appropriate
// handlers have been installed for the different types of control
// messages, and communication with the host is allowed to start. The
// next event that should happen is a reset operation from the host,
// so all other interrupts should be blocked. However it is likely
// that the hardware will detect a suspend state before the reset
// arrives, and hence the reset will act as a resume as well as a
// reset.
static void
usbs_sa11x0_ep0_start(usbs_control_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep0.common, "USB startup involves the wrong endpoint");
    
    // Activate the hardware. Write a 0 to the enable/disable bit 0.
    // Bit 1 is read-only. The other bits are set to 1 to disable
    // the corresponding interrupt source.
    usbs_sa11x0_poke(USBS_CONTROL, CONTROL_ALL_INTR, CONTROL_ALL_INTR, 0);

    // If there is additional platform-specific initialization to
    // perform, do it now. This macro can come from the platform HAL.
#ifdef SA11X0_USB_PLATFORM_INIT
    SA11X0_USB_PLATFORM_INIT;
#endif
        
    // Clear any pending interrupts. There should not be any, but just
    // in case. Note: passing 0x00FF as the should_be_clear argument
    // is a race condition, an external event can happen at any time,
    // so we may loop unnecessarily and lose an interrupt. However
    // the initial reset should last for 10ms.
    usbs_sa11x0_poke(USBS_STATUS, 0x00FF, 0x00, 0x00FF);

    // The only interrupt really of interest right now is reset, but
    // it is likely to be preceded by a resume. 
    usbs_sa11x0_poke(USBS_CONTROL,
                     CONTROL_INTR_ENABLE(CONTROL_RESET_INTR | CONTROL_RESUME_INTR),
                     0,
                     CONTROL_INTR_CLEAR(CONTROL_RESET_INTR | CONTROL_RESUME_INTR));
}


// Filling the fifo with a reply to the host. This can be called
// immediately at the end of a control message, to prepare for
// the next IN token. It will also get called after each subsequent
// IN operation when the fifo has been emptied.
//
// Experiments have indicated serious problems with the control fifo:
// some writes to the fifo just get lost completely. The failure rate
// is sufficiently high that more often than not the host will be
// unable to read all the enumeration data. However, the write-count
// register appears to give a valid indication of the current fifo
// contents. This means the code can retry stuffing a particular byte
// into the fifo until the write-count goes up.

static void
usbs_sa11x0_ep0_fill_fifo(void)
{
    cyg_bool ok     = true;
    int filled      = 0;
    int max;
    int fifo_count  = *EP0_WRITE_COUNT;
    int bits_to_set = 0;
    
    // The host can interrupt the current control message at any time
    // with a new one. In practice this is unlikely, things could get
    // rather confused on the host side. However if a control message
    // has been received then the fifo should obviously not be filled.
    // A new control message is indicated by the SETUP_END bit.
    //
    // The hardware design means that there is a race condition: the
    // new control message can come in at any time, even in the middle
    // of filling the fifo. Checking the SETUP_END more often would
    // reduce the probability of things getting messed up, but not
    // eliminate it.
    //
    // There is a check for SETUP_END at the start of the DSR, so
    // the setting of this bit should have resulted in another ISR
    // and another DSR being scheduled. Hence there is no need for
    // special action here.
    if (0 != (*EP0_CONTROL & EP0_SETUP_END)) {
        DBG(("EP0_fill_fifo(), interrupted by SETUP_END\n"));
        return;
    }

    // There should never be any data in the fifo. Any such data could
    // be the remnant of a previous transfer to the host, but that
    // should all have gone out already. Alternatively it could be
    // incoming data, but that means a new control message.
    if (0 != fifo_count) {
        DBG(("EP0_fill_fifo(), fifo already contains %d bytes", fifo_count));
        return;
    }

    // The IN_READY bit should never be set on entry. It can only get
    // set by a previous call to fill_fifo(), and the data should
    // have gone out before we get back here.
    if (0 != (*EP0_CONTROL & EP0_IN_READY)) {
        DBG(("EP0 fill_fifo(), in-packet-ready bit already set, state %x\n", *EP0_CONTROL));
        return;
    }

    // Now put up to another eight bytes into the fifo.
    max = ((ep0.length - ep0.transmitted) > EP0_FIFO_SIZE) ? EP0_FIFO_SIZE : (ep0.length - ep0.transmitted);
    while (ok && (filled < max)) {
        if (0 != ep0.common.buffer_size) {
            int         datum;
            int         retries, checks;
            cyg_bool    written;

            datum       = *ep0.common.buffer++;
            ep0.common.buffer_size--;
            written     = false;
            
            for (retries = 0; ok && !written && (retries < MAX_RETRIES); retries++) {
                if (filled != *EP0_WRITE_COUNT) {
                    DBG(("EP0 fill_fifo, inconsistency, written %d but write count %d\n", filled, *EP0_WRITE_COUNT));
                    ok = false;
                }
                *EP0_DATA = datum;
                // The write-count may take a few cycles to settle down.
                for (checks = 0; !written && (checks < MAX_CHECKS); checks++) {
                    if (filled < *EP0_WRITE_COUNT) {
                        filled++;
                        written = true;
                        // DBG(("Transferred %d byte (%x) after %d checks, %d retries\n", filled - 1, datum, checks, retries));
                    }
                }
            }
        } else if ((void (*)(usbs_control_endpoint*))0 != ep0.common.fill_buffer_fn) {
            (*ep0.common.fill_buffer_fn)(&ep0.common);
        } else {
            break;
        }
    }

    // At this point either it has proved impossible to fill the fifo,
    // e.g. because of a new control message, or up to another eight
    // bytes have been sent. 
    if (!ok) {
        if (0 == (EP0_SETUP_END & *EP0_CONTROL)) {
            // There is something seriously wrong.
            DBG(("ep0_fill_fifo(), failed, only filled %d bytes, status %x\n", filled, *EP0_CONTROL));
            usbs_sa11x0_poke(EP0_CONTROL, EP0_FORCE_STALL, EP0_FORCE_STALL, 0);
        }
        return;
    }

    // The following conditions are possible:
    // 1) amount transferred == amount requested, transfer complete.
    // 2) amount transferred < amount requested, this fill involved
    //    <eight bytes, transfer complete by definition of the protocol.
    // 3) amount transferred < amount requested but exactly eight
    //    bytes were sent this time. It will be necessary to send
    //    another packet of zero bytes to complete the transfer.
    ep0.transmitted += filled;
    if ((ep0.transmitted == ep0.length) || (filled < EP0_FIFO_SIZE)) {

        ep0.ep_state    = EP0_STATE_IDLE;
        if ((usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn) {
            (void) (*ep0.common.complete_fn)(&ep0.common, 0);
        }
        ep0.common.buffer               = (unsigned char*) 0;
        ep0.common.buffer_size          = 0;
        ep0.common.fill_buffer_fn       = (void (*)(usbs_control_endpoint*)) 0;

        // This informs the hardware that the control message has been
        // handled.
        bits_to_set = EP0_DATA_END;
    }
    
    // This allows another IN operation to empty the fifo.
    bits_to_set |= EP0_IN_READY;
    usbs_sa11x0_poke(EP0_CONTROL, bits_to_set, bits_to_set, 0);
}

// Another utility function to empty the fifo. This involves similar
// hardware problems to writing, it is possible to read a byte without
// changing the fifo state so that next time the same byte would be
// read again. Again there is a possible race condition if another
// control message arrives while emptying the fifo.
static int
usbs_sa11x0_ep0_empty_fifo(unsigned char* buf)
{
    int         count   = *EP0_WRITE_COUNT & 0x00FF;
    int         emptied = 0;
    cyg_bool    ok      = true;

    CYG_ASSERT( (count >= 0) & (count <= 8), "EP0 write count must be in range");

    while (ok && (emptied < count)) {
        int      retries, checks;
        cyg_bool read   = false;

        for (retries = 0; !read && (retries < MAX_RETRIES); retries++) {
            if ((count - emptied) != *EP0_WRITE_COUNT) {
                DBG(("EP0_empty_fifo, inconsistency, read %d bytes of %d, but fifo count %d\n", emptied, count, *EP0_WRITE_COUNT));
                ok = false;
            } else {
                buf[emptied] = *EP0_DATA;
                for (checks = 0; !read && (checks < MAX_CHECKS); checks++) {
                    if ((count - emptied) > *EP0_WRITE_COUNT) {
                        //DBG(("Read %d byte (%x) after %d checks, %d retries\n", emptied, buf[emptied], checks, retries));
                        read = true;
                        emptied++;
                    }
                }
            }
        }
        if (ok && !read) {
            DBG(("EP0 empty fifo, failed to read byte from fifo\n"));
            ok = false;
        }
    }
    
    return emptied;
}

// This is where all the hard work happens. It is a very large routine
// for a DSR, but in practice nearly all of it is nested if's and very
// little code actually gets executed. Note that there may be
// invocations of callback functions and the driver has no control
// over how much time those will take, but those callbacks should be
// simple.
static void
usbs_sa11x0_ep0_dsr(void)
{
    int hw_state = *EP0_CONTROL;

    // Handle the stall bits.
    //
    // Force-stall should not be a problem. It is set by the code here
    // if the host needs to be told that the control message was
    // unacceptable and is cleared automatically by the hardware after
    // the stall is sent.
    // NOTE: it is not clear the hardware actually works in this
    // respect. The FORCE_STALL bit has been observed still set during
    // the next interrupt, and the host appears to receive spurious
    // data back in response to the next control packet.
    //
    // Sent-stall is set by the hardware following a protocol
    // violation, e.g. if there is an IN token when a new control
    // message is expected. There is nothing the software can do about
    // this. However if we are in the middle of an IN or OUT transfer
    // then those are not going to complete successfully.
    if (0 != (hw_state & EP0_SENT_STALL)) {
        if (EP0_STATE_IDLE != ep0.ep_state) {
            if ((usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn) {
                (*ep0.common.complete_fn)(&ep0.common, -EIO);
            }
            ep0.ep_state                = EP0_STATE_IDLE;
            ep0.common.buffer           = (unsigned char*) 0;
            ep0.common.buffer_size      = 0;
            ep0.common.fill_buffer_fn   = 0;
            ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
        }
        usbs_sa11x0_poke(EP0_CONTROL, EP0_SENT_STALL, 0, EP0_SENT_STALL);
    }   // STALL condition
    
    // Next, check whether we have received a new control message
    // while still busy processing an old one.
    if (0 != (hw_state & EP0_SETUP_END)) {
        if (EP0_STATE_IDLE != ep0.ep_state) {
            if ((usbs_control_return (*)(usbs_control_endpoint*, int)) 0 != ep0.common.complete_fn) {
                (*ep0.common.complete_fn)(&ep0.common, -EIO);
            }
            ep0.ep_state                = EP0_STATE_IDLE;
            ep0.common.buffer           = (unsigned char*) 0;
            ep0.common.buffer_size      = 0;
            ep0.common.fill_buffer_fn   = 0;
            ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
        }
        // We are now back in idle state so the control message will be
        // extracted and processed.
        usbs_sa11x0_poke(EP0_CONTROL, EP0_SERVICED_SETUP_END, 0, EP0_SETUP_END);
    }   // Interrupted control transaction

    // The endpoint can be in one of three states: IN, OUT, or IDLE.
    // For the first two it should mean that there is more data to be
    // transferred, which is pretty straightforward. IDLE means
    // that a new control message has arrived.
    if ((EP0_STATE_IN == ep0.ep_state) && (0 == (EP0_IN_READY & hw_state)))  {
        
        usbs_sa11x0_ep0_fill_fifo();
        
    } else if ((EP0_STATE_OUT == ep0.ep_state) && (0 != (EP0_OUT_READY & hw_state))) {

        // A host->device transfer. Higher level code must have
        // provided a suitable buffer.
        CYG_ASSERT( (unsigned char*)0 != ep0.common.buffer, "A receive buffer should have been provided" );

        ep0.transmitted += usbs_sa11x0_ep0_empty_fifo(ep0.common.buffer + ep0.transmitted);

        if (ep0.transmitted != ep0.length) {
            // The host is not allowed to send more data than it
            // indicated in the original control message, and all
            // messages until the last one should be full size.
            CYG_ASSERT( ep0.transmitted < ep0.length, "The host must not send more data than expected");
            CYG_ASSERT( 0 == (ep0.transmitted % EP0_FIFO_SIZE), "All OUT packets until the last one should be full-size");

            usbs_sa11x0_poke(EP0_CONTROL, EP0_SERVICED_OPR, 0, EP0_OUT_READY);
        } else {
            // The whole transfer is now complete. Invoke the
            // completion function, and based on its return value
            // either generate a stall or complete the message.
            usbs_control_return result;
            
            CYG_ASSERT( (usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn, \
                        "A completion function should be provided for OUT control messages");

            result = (*ep0.common.complete_fn)(&ep0.common, 0);
            ep0.common.buffer           = (unsigned char*) 0;
            ep0.common.buffer_size      = 0;
            ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
            
            if (USBS_CONTROL_RETURN_HANDLED == result) {
                usbs_sa11x0_poke(EP0_CONTROL,
                                 EP0_SERVICED_OPR | EP0_DATA_END,
                                 EP0_DATA_END,
                                 EP0_OUT_READY);
            } else {
                usbs_sa11x0_poke(EP0_CONTROL,
                                 EP0_SERVICED_OPR | EP0_DATA_END | EP0_FORCE_STALL,
                                 EP0_FORCE_STALL,
                                 EP0_OUT_READY);
            }
            // Also remember to switch back to IDLE state 
            ep0.ep_state = EP0_STATE_IDLE;
        }
        
    } else if (0 != (EP0_OUT_READY & hw_state)) {

        int emptied = usbs_sa11x0_ep0_empty_fifo(ep0.common.control_buffer);
        
        if (8 != emptied) {
            // This indicates a serious problem somewhere. Respond by
            // stalling. Hopefully the host will take some action that
            // sorts out the mess.
            usbs_sa11x0_poke(EP0_CONTROL,
                             EP0_SERVICED_OPR | EP0_DATA_END | EP0_FORCE_STALL,
                             EP0_FORCE_STALL,
                             EP0_OUT_READY);
            
        } else {
            usbs_control_return result  = USBS_CONTROL_RETURN_UNKNOWN;
            usb_devreq*         req     = (usb_devreq*) ep0.common.control_buffer;
            int length, direction, protocol, recipient;
            
            // Now we need to do some decoding of the data. A non-zero
            // length field indicates that there will be a subsequent
            // IN or OUT phase. The direction is controlled by the
            // top bit of the first byte. The protocol is determined
            // by other bits of the top byte.
            length      = (req->length_hi << 8) | req->length_lo;
            direction   = req->type & USB_DEVREQ_DIRECTION_MASK;
            protocol    = req->type & USB_DEVREQ_TYPE_MASK;
            recipient   = req->type & USB_DEVREQ_RECIPIENT_MASK;

#if 0
            DBG(("ep0, new control request: type %x, code %x\n", req->type, req->request));
            DBG(("     %s, length %d, value hi %x lo %x, index hi %x lo %x\n",
                 (USB_DEVREQ_DIRECTION_OUT == direction) ? "out" : "in",
                 length, req->value_hi, req->value_lo, req->index_hi, req->index_lo));
#endif            
            if (0 != length){
                // Clear the fifo straightaway. There is no harm in
                // doing this here. It may or may not do some good.
                usbs_sa11x0_poke(EP0_CONTROL, EP0_SERVICED_OPR, 0, EP0_OUT_READY);
            }
            
            if (USB_DEVREQ_TYPE_STANDARD == protocol) {
                
                // First see if the request can be handled entirely in
                // this module.
                if (USB_DEVREQ_SET_ADDRESS == req->request) {
                    // The USB device address should be in value_lo.
                    // No more data is expected. 
                    int address = req->value_lo;
                    if ((0 != length) || (address > 127)) {
                        result = USBS_CONTROL_RETURN_STALL;
                    } else {
                        // poke_value() cannot be used here because
                        // setting the address does not take effect
                        // until the status phase.
                        *USBS_ADDRESS = address;
                        result = USBS_CONTROL_RETURN_HANDLED;
                    }
                } else if (USB_DEVREQ_GET_STATUS == req->request) {
                    // GET_STATUS on the device as a whole is used to
                    // check the remote-wakeup and self-powered bits.
                    // GET_STATUS on an endpoint is used to determine
                    // the halted condition.
                    // GET_STATUS on anything else has to be left to
                    // other code.
                    if (USB_DEVREQ_RECIPIENT_DEVICE == recipient) {
                        // The host should expect two bytes back.
                        if ((2 == length) && (USB_DEVREQ_DIRECTION_IN == direction)) {
                            ep0.common.control_buffer[0] = 0;   // Not self-powered, no remote wakeup
                            ep0.common.control_buffer[1] = 0;
                            ep0.common.buffer            = ep0.common.control_buffer;
                            ep0.common.buffer_size       = 2;
                            result                       = USBS_CONTROL_RETURN_HANDLED;
                        } else {
                            result = USBS_CONTROL_RETURN_STALL;
                        }
                        
                    } else if (USB_DEVREQ_RECIPIENT_ENDPOINT == recipient) {
                        if ((2 == length) && (USB_DEVREQ_DIRECTION_IN == direction)) {
                            int endpoint = req->index_lo;
                            if (0 == endpoint) {
                                // get-status on endpoint 0 is either undefined or always valid.
                                // endpoint 0 is always up.
                                ep0.common.control_buffer[0] = 0;
                                result = USBS_CONTROL_RETURN_HANDLED;
                            }
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
                            else if (((USB_DEVREQ_INDEX_DIRECTION_OUT | 1) == endpoint) &&
                                       (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                                
                                ep0.common.control_buffer[0] = ep1.common.halted;
                                result = USBS_CONTROL_RETURN_HANDLED;

                            }
#endif                            
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
                            else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 2) == endpoint) &&
                                       (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {

                                ep0.common.control_buffer[0] = ep2.common.halted;
                                result = USBS_CONTROL_RETURN_HANDLED;
                                
                            }
#endif                            
                            else {
                                // An invalid endpoint has been specified or the
                                // endpoint can only be examined in configured state.
                                result = USBS_CONTROL_RETURN_STALL;
                            }
                            if (USBS_CONTROL_RETURN_HANDLED == result) {
                                ep0.common.control_buffer[1] = 0;
                                ep0.common.buffer            = ep0.common.control_buffer;
                                ep0.common.buffer_size       = 2;
                            }
                        } else {
                            result = USBS_CONTROL_RETURN_STALL;
                        }
                    } // Endpoint or device get-status
                    
                } else if (USB_DEVREQ_CLEAR_FEATURE == req->request) {

                    // CLEAR_FEATURE operates in much the same way as
                    // GET_STATUS
                    if (USB_DEVREQ_RECIPIENT_DEVICE == recipient) {
                        
                        // No data should be transferred, and only remote-wakeup can be cleared.
                        if ((0 != length) || (USB_DEVREQ_FEATURE_DEVICE_REMOTE_WAKEUP != req->value_lo)) {
                            result = USBS_CONTROL_RETURN_STALL;
                        } else {
                            // Clearing remote-wakeup is a no-op.
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }

                    } else if (USB_DEVREQ_RECIPIENT_ENDPOINT == recipient) {
                        // The only feature that can be cleared is endpoint-halt, no data should be transferred.
                        if ((0 != length) || (USB_DEVREQ_FEATURE_ENDPOINT_HALT != req->value_lo)) {
                            result = USBS_CONTROL_RETURN_STALL;
                        } else {
                            int endpoint = req->index_lo;
                            if (0 == endpoint) {
                                // Clearing halt on endpoint 0 is always a no-op since that endpoint cannot be halted
                            }
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
                            else if (((USB_DEVREQ_INDEX_DIRECTION_OUT | 1) == endpoint) &&
                                       (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                                ep1_set_halted(&ep1.common, false);
                                result = USBS_CONTROL_RETURN_HANDLED;
                                
                            }
#endif
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
                            else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 2) == endpoint) &&
                                       (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                                ep2_set_halted(&ep2.common, false);
                                result = USBS_CONTROL_RETURN_HANDLED;
                                    
                            }
#endif
                            else {
                                // Invalid endpoint or not in configured state.
                                result = USBS_CONTROL_RETURN_STALL;
                            }
                        }
                    }   // Endpoing or device clear-feature
                    
                } else if (USB_DEVREQ_SET_FEATURE == req->request) {

                    // SET_FEATURE also operates in much the same way as
                    // GET_STATUS
                    if (USB_DEVREQ_RECIPIENT_DEVICE == recipient) {
                        
                        // The only valid feature that can be set is remote-wakeup,
                        // which is not supported by the hardware.
                        result = USBS_CONTROL_RETURN_STALL;
                        
                    } else if (USB_DEVREQ_RECIPIENT_ENDPOINT == recipient) {

                        // Only the halt condition can be set, and no data should be transferred.
                        // Halting endpoint 0 should probably be disallowed although the
                        // standard does not explicitly say so.
                        if ((0 != length) ||
                            (USB_DEVREQ_FEATURE_ENDPOINT_HALT != req->value_lo) ||
                            (USBS_STATE_CONFIGURED != (ep0.common.state & USBS_STATE_MASK))) {
                            
                            result = USBS_CONTROL_RETURN_STALL;
                            
                        } else {
                            int endpoint = req->index_lo;
                            if (0) {
                            }
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
                            else if ((USB_DEVREQ_INDEX_DIRECTION_OUT | 1) == endpoint) {
                                ep1_set_halted(&ep1.common, true);
                                result = USBS_CONTROL_RETURN_HANDLED;
                            }
#endif                            
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
                            else if ((USB_DEVREQ_INDEX_DIRECTION_IN | 2) == endpoint) {
                                ep2_set_halted(&ep2.common, true);
                                result = USBS_CONTROL_RETURN_HANDLED;
                            }
#endif                            
                            else {
                                result = USBS_CONTROL_RETURN_STALL;
                            }
                        }
                    } // Endpoint or device set-feature
                }

                // If the result has not been handled yet, pass it to
                // the installed callback function (if any).
                if (USBS_CONTROL_RETURN_UNKNOWN == result) {
                    if ((usbs_control_return (*)(usbs_control_endpoint*, void*))0 != ep0.common.standard_control_fn) {
                        result = (*ep0.common.standard_control_fn)(&ep0.common, ep0.common.standard_control_data);
                    }
                }
                
#if 1                
                if ((USBS_CONTROL_RETURN_UNKNOWN == result) &&
                    (USB_DEVREQ_SET_INTERFACE == req->request)) {
                    
                    // This code should not be necessary. For
                    // non-trivial applications which involve
                    // alternate interfaces and the like, this request
                    // should be handled by the application itself.
                    // For other applications, the default handler
                    // will ignore this request so we end up falling
                    // through without actually handling the request
                    // and hence returning a stall condition. That
                    // is legitimate behaviour according to the standard.
                    //
                    // However, there are appear to be problems with
                    // the SA1110 USB hardware when it comes to stall
                    // conditions: they appear to affect some
                    // subsequent messages from target to host as
                    // well. Hence rather than returning a stall
                    // condition this code instead generates a dummy
                    // reply, which is also valid according to the
                    // standard. This avoids complications with certain
                    // USB compliance testers.
                    if ((0 != length) ||
                        (0 != req->value_hi) || (0 != req->index_hi) ||
                        (USBS_STATE_CONFIGURED != (ep0.common.state & USBS_STATE_MASK))) {
                        
                        result = USBS_CONTROL_RETURN_STALL;
                    } else {
                        int interface_id;
                        int alternate;
            
                        CYG_ASSERT( (1 == ep0.common.enumeration_data->device.number_configurations) && \
                                    (1 == ep0.common.enumeration_data->total_number_interfaces),       \
                                    "Higher level code should have handled this request");
            
                        interface_id = req->index_lo;
                        alternate    = req->value_lo;
                        if ((interface_id != ep0.common.enumeration_data->interfaces[0].interface_id) ||
                            (alternate  != ep0.common.enumeration_data->interfaces[0].alternate_setting)) {

                            result = USBS_CONTROL_RETURN_STALL;
                        } else {
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
                        
                    }
                }
#endif
                
                // If the result has still not been handled, leave it to
                // the default implementation in the USB slave common package
                if (USBS_CONTROL_RETURN_UNKNOWN == result) {
                    result = usbs_handle_standard_control(&ep0.common);
                }
                
            } else {
                // The other three types of control message can be
                // handled by similar code.
                usbs_control_return (*callback_fn)(usbs_control_endpoint*, void*);
                void* callback_arg;
                //DBG(("non-standard control request %x", req->request));
                
                if (USB_DEVREQ_TYPE_CLASS == protocol) {
                    callback_fn  = ep0.common.class_control_fn;
                    callback_arg = ep0.common.class_control_data;
                } else if (USB_DEVREQ_TYPE_VENDOR == protocol) {
                    callback_fn  = ep0.common.vendor_control_fn;
                    callback_arg = ep0.common.vendor_control_data;
                } else {
                    callback_fn  = ep0.common.reserved_control_fn;
                    callback_arg = ep0.common.reserved_control_data;
                }

                if ((usbs_control_return (*)(usbs_control_endpoint*, void*)) 0 == callback_fn) {
                    result = USBS_CONTROL_RETURN_STALL;
                } else {
                    result = (*callback_fn)(&ep0.common, callback_arg);
                }
            }
            //DBG(("Control request done, %d\n", result));

            if (USBS_CONTROL_RETURN_HANDLED != result) {
                // This control request cannot be handled. Generate a stall.
                usbs_sa11x0_poke(EP0_CONTROL,
                                 EP0_FORCE_STALL | EP0_SERVICED_OPR | EP0_DATA_END,
                                 EP0_FORCE_STALL,
                                 EP0_OUT_READY);
            } else {
                // The control request has been handled. Is there any more
                // data to be transferred?
                if (0 == length) {
                    usbs_sa11x0_poke(EP0_CONTROL,
                                     EP0_SERVICED_OPR | EP0_DATA_END,
                                     EP0_DATA_END,
                                     EP0_OUT_READY);
                } else {
                    // The endpoint should now go into IN or OUT mode while the
                    // remaining data is transferred.
                    ep0.transmitted     = 0;
                    ep0.length          = length;
                    if (USB_DEVREQ_DIRECTION_OUT == direction) {
                        // Wait for the next packet from the host.
                        ep0.ep_state = EP0_STATE_OUT;
                        CYG_ASSERT( (unsigned char*) 0 != ep0.common.buffer, "A receive buffer should have been provided");
                        CYG_ASSERT( (usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn, \
                                    "A completion function should be provided for OUT control messages");
                    } else {
                        ep0.ep_state = EP0_STATE_IN;
                        usbs_sa11x0_ep0_fill_fifo();
                    }
                }
            }   // Control message handled
        }       // Received 8-byte control message
    }           // Idle state, i.e. control message
}               // ep0_dsr

#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
// ----------------------------------------------------------------------------
// Endpoint 1 is used for OUT transfers, i.e. receive operations. Only
// the bulk protocol is supported by the hardware. The semantics allow
// for two different modes of operation: higher-level code can ask for
// exactly one or more bulk packets of 64 bytes each, allowing buffer
// requirements to be determined from a header; alternatively the
// rx request can just supply a large buffer. Processing the first
// packet of a larger transfer separately does not introduce any
// special problems at the protocol level.
//
// It is not legal to receive just part of a packet and expect the
// hardware or the driver to buffer the rest. Not all hardware will
// be capable of doing this buffering, and there should not be
// a driver design requirement to provide buffering space.
//
//
// The hardware design for endpoint 1 is flawed in a number of
// respects. The receive fifo is only 20 bytes, less than the packet
// size, so it is essential to use DMA (there is a configuration
// option to allow for communication protocols where packets will
// never exceed 16 bytes, but that is not the normal case). The DMA
// engine is triggered by a receive-fifo-service high-water mark
// bit. DMA transfers operate in bursts of eight bytes. Therefore
// it would make sense if the high-water mark was set when the
// receive fifo contained eight bytes or more.
//
// Instead the high-water mark is set when the fifo contains twelve
// bytes or more. Worse, there is no way of measuring how many bytes
// there are left in the fifo without actually extracting those bytes.
//
// For a full-size 64-byte packet, the first 56 bytes will be
// transferred by DMA and the remainder will remain in the fifo. For a
// partial packet of between 56 and 63 bytes, the first 56 bytes will
// be transferred by DMA and the remainder will remain in the fifo. There
// is no way to distinguish between these scenarios without emptying
// the fifo.
//
// The result is that there is never any point in attempting a DMA
// transfer of more than 56 bytes, and for every endpoint 1 interrupt
// it is necessary to read the remainder from the fifo. This adds
// a lot of software overhead, and it is not clear that DMA is
// particularly useful. It is still necessary because of the limited
// fifo size.
//
//
// Because DMA involves the use of physical rather than virtual
// memory, there are also cache interaction problems. Specifically it
// would be necessary to invalidate cache lines after a DMA transfer
// has completed, but that only works sensibly if the buffer is
// aligned to a cache-line boundary and is a multiple of the
// cache-line size. Imposing such restrictions on higher-level code
// is undesirable. Also the DMA engines have an apparently undocumented
// restriction that the buffer must be eight-byte aligned.
//
// To work around all these problems, the receive code works in terms
// of a small private buffer. After a packet has been received, data
// will be copied from this private buffer to the destination. Obviously
// this copy operation is overhead and, because the code is expected
// to run at DSR level, However the copy operation is limited to at
// most 64 bytes, which is not good but not disastrous either.
//
// For data transfers the entry points are:
//
// 1) ep1_start_rx_packet() - prepare to receive another packet from
//    the host.
// 2) ep1_clear_error() - an error condition has occurred (CRC,
//    bit-stuffing, fifo overrun). It appears that the only way
//    to clear this is to clear the receive-packet-complete bit,
//    which unfortunately allows in another packet from the host
//    before we are ready for it. Doing anything else while
//    the error bit is set does not work, for example it is not
//    possible to empty the fifo by hand.
// 3) ep1_process_packet() - a whole packet has been received
//    and now needs to be moved into application space.
//
// These three routines are called by the start_rx() routine and
// by the DSR. There are different implementations for DMA and
// non-DMA.
//
// There is another hardware problem: the receive-packet-complete bit
// comes up with the wrong default value, allowing the host to start
// transmitting before the target is ready to receive. Unfortunately
// there is not much that can be done about this: the
// receive-packet-complete bit cannot be set by software and the OUT
// max register has a minimum size of eight bytes. Fortunately for
// many protocols the target-side code has a chance to start a receive
// before the host is allowed to send, so this problem is mostly
// ignored for now.
//
// Another potential problem arises if the host sends more data than
// is expected for a given transfer. It would be possible to address
// this by manipulating the OUT max packet register and getting the
// hardware to generate protocol violation stalls. This would also
// eliminate the need to test for buffer overflows. For now it is
// left to higher-level code to sort it all out.

#ifdef CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL

// DMA needs an area of physical memory. To avoid conflicts with
// the cached shadow of this memory, this area needs to start at
// a cache line boundary and there must be padding at the end
// to the next cache line boundary, thus ensuring that the
// processor will not accidentally overwrite the physical
// memory because it is manipulating some other variable.
//
// NOTE: at the time of writing the toolchain has a problem with
// the aligned attribute, so instead the start alignment has
// to be handled in software.

# define EP1_DMA_MTU            56
# define EP1_DMA_BUFSIZE        ((EP1_DMA_MTU + HAL_DCACHE_LINE_SIZE - 1) - \
                                 ((EP1_DMA_MTU + HAL_DCACHE_LINE_SIZE - 1) % HAL_DCACHE_LINE_SIZE))
# define EP1_DMA_ALLOCSIZE      (EP1_DMA_BUFSIZE + HAL_DCACHE_LINE_SIZE - 1)

static unsigned char    ep1_dma_data[EP1_DMA_ALLOCSIZE];

// This variable cannot be initialized statically, instead it is
// set by ep1_init(). It corresponds to the physical address
// for the buffer.
static unsigned char*   ep1_dma_buf;

static void
ep1_start_rx_packet(void)
{
    int dma_size = EP1_DMA_MTU;

    // This assertion does not always hold: clearing an error condition
    // involves the packet-complete bit so another message may have
    // started to arrive.
    // CYG_ASSERT( 0 == (EP1_FIFO_NOT_EMPTY & *EP1_CONTROL), "The receive fifo should be empty");
    
    CYG_ASSERT( 0 == ((DMA_CONTROL_RUN | DMA_CONTROL_START_A) & *EP1_DMA_STATUS), "EP1 DMA should be inactive");

#ifdef FAILURES
    ep1_failure = (ep1_failure + 1) % 32;
    if (0 == ep1_failure) {
        dma_size = 8;
    }
#endif
    
    // The full flexibility of the DMA engines is not required here,
    // specifically the automatic chaining between buffers A and B.
    // Instead always using buffer A is sufficient. To avoid the
    // However the hardware still requires the software to alternate
    // between A and B. To avoid switching between buffers during a
    // transfer an excessive size field is used, EP1_MTU rather than
    // EP1_DMA_MTU, and hence the DMA transfer will never complete.
    //
    // With some silicon revisions writing to the DMA registers does
    // not always work either, so a retry is in order. Possibly
    // some short delays immediately after the clear and before the
    // set would be sufficient.
    *EP1_DMA_CONTROL_CLEAR  = DMA_CONTROL_CLEAR_ALL;
    if (0 == (DMA_STATUS_BUFFER_IN_USE & *EP1_DMA_STATUS)) {
        ep1.using_buf_a = true;
        usbs_sa11x0_poke_value(EP1_DMA_BUF_A_ADDRESS, (unsigned int) ep1_dma_buf);
        usbs_sa11x0_poke_value(EP1_DMA_BUF_A_SIZE, dma_size);
        *EP1_DMA_CONTROL_SET = DMA_CONTROL_RUN | DMA_CONTROL_START_A;
    } else {
        ep1.using_buf_a = false;
        usbs_sa11x0_poke_value(EP1_DMA_BUF_B_ADDRESS, (unsigned int) ep1_dma_buf);
        usbs_sa11x0_poke_value(EP1_DMA_BUF_B_SIZE, dma_size);
        *EP1_DMA_CONTROL_SET = DMA_CONTROL_RUN | DMA_CONTROL_START_B;
    }

    // This should not be necessary, but occasionally the equivalent
    // operation during ep1_init() fails. Strictly speaking it should
    // be calling poke_value(), but the added overheads for that are
    // not worthwhile.
    *USBS_OUT_SIZE      = EP1_MTU - 1;
    
    // Now allow the host to send the packet.
    usbs_sa11x0_poke(EP1_CONTROL, EP1_PACKET_COMPLETE | EP1_SENT_STALL, 0,
                     EP1_PACKET_COMPLETE | EP1_SENT_STALL | EP1_FORCE_STALL);
}

// Clear an error condition following a CRC, bit stuffing or overrun
// error. The only reliable way to do this is to halt DMA and clear
// the packet-complete bit. Unfortunately this allows the host to send
// another packet immediately, before start_rx_packet can be called,
// introducing another race condition. The hardware does not appear
// to offer any alternatives.
static void
ep1_clear_error(void)
{
    *EP1_DMA_CONTROL_CLEAR = DMA_CONTROL_CLEAR_ALL;
    usbs_sa11x0_poke(EP1_CONTROL, EP1_PACKET_COMPLETE | EP1_SENT_STALL, 0,
                     EP1_PACKET_COMPLETE | EP1_PACKET_ERROR | EP1_SENT_STALL | EP1_FORCE_STALL | EP1_FIFO_NOT_EMPTY);
    
    // Clearing the packet-complete bit may cause the host to send
    // another packet, immediately causing another error, so this
    // assertion does not hold.
    // CYG_ASSERT( 0 == (*EP1_CONTROL & (EP1_PACKET_ERROR | EP1_FIFO_NOT_EMPTY)), "Receive error should have been cleared");
}

// A packet has been received. Some of it may still be in the fifo
// and must be extracted by hand. The data then has to copied to
// a higher-level buffer.
static int
ep1_process_packet(void)
{
    int pkt_size;

    // First, work out how much data has been processed by the DMA
    // engine. This is the amount originally poked into the size
    // register minus its current value.
    *EP1_DMA_CONTROL_CLEAR = DMA_CONTROL_CLEAR_ALL;
    if (ep1.using_buf_a) {
        pkt_size = EP1_DMA_MTU - *EP1_DMA_BUF_A_SIZE;
    } else {
        pkt_size = EP1_DMA_MTU - *EP1_DMA_BUF_B_SIZE;
    }
    CYG_ASSERT( 0 == (pkt_size % DMA_BURST_SIZE), "DMA transfers must be in multiples of the burst size");
    
    // Move these bytes from physical memory to the target buffer.
    if ((pkt_size > 0) && ((ep1.fetched + pkt_size) < ep1.common.buffer_size)) {
        memcpy(ep1.common.buffer + ep1.fetched, ep1_dma_buf, pkt_size);
    } 

    // Copy remaining bytes into the target buffer directly.
    // The DMA buffer could be used instead, moving the memcpy()
    // down and avoiding the need for a buffer overflow check
    // inside the loop, but at the cost of accessing physical
    // memory every time. That cost is too high.
    while (1) {
        int status = *EP1_CONTROL;
        if ((EP1_PACKET_COMPLETE | EP1_PACKET_ERROR) == ((EP1_PACKET_COMPLETE | EP1_PACKET_ERROR) & status)) {
            break;
        } else if (0 == (EP1_FIFO_NOT_EMPTY & status)) {
            break;
        } else {
            int datum = *EP1_DATA;
            if (ep1.fetched < ep1.common.buffer_size) {
                ep1.common.buffer[ep1.fetched + pkt_size] = datum;
            }
            pkt_size++;
        }
    }
    ep1.fetched += pkt_size;
    return pkt_size;
}

#else

// Transfers not involving DMA. Obviously these are much simpler
// but restricted to packets of 16 bytes.
static void
ep1_start_rx_packet(void)
{
    // Nothing to be done, just let the host send a packet and it will
    // end up in the fifo.
    usbs_sa11x0_poke(EP1_CONTROL, EP1_PACKET_COMPLETE | EP1_SENT_STALL, 0,
                     EP1_PACKET_COMPLETE | EP1_SENT_STALL | EP1_FORCE_STALL);
}

static void
ep1_clear_error(void)
{
    usbs_sa11x0_poke(EP1_CONTROL, EP1_PACKET_COMPLETE | EP1_SENT_STALL, 0,
                     EP1_PACKET_COMPLETE | EP1_SENT_STALL | EP1_FORCE_STALL);
}

static int
ep1_process_packet(void)
{
    int pkt_size = 0;
    while (0 != (*EP1_CONTROL & EP1_FIFO_NOT_EMPTY)) {
        int datum = *EP1_DATA;
        pkt_size++;
        if (ep1.fetched < ep1.common.buffer_size) {
            ep1.common.buffer[ep1.fetched + pkt_size] = datum;
        }
    }
    return pkt_size;
}
#endif

// Complete a transfer. This takes care of invoking the completion
// callback and resetting the buffer.
static void
ep1_rx_complete(int result)
{
    void (*complete_fn)(void*, int)  = ep1.common.complete_fn;
    void* complete_data = ep1.common.complete_data;
    
    ep1.common.buffer           = (unsigned char*) 0;
    ep1.common.buffer_size      = 0;
    ep1.common.complete_fn      = (void (*)(void*, int)) 0;
    ep1.common.complete_data    = (void*) 0;

    if ((void (*)(void*, int))0 != complete_fn) {
        (*complete_fn)(complete_data, result);
    }
}

// Start a transmission. This functionality is overloaded to cope with
// waiting for stalls to complete.
static void
ep1_start_rx(usbs_rx_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep1.common, "USB data transfer involves the wrong endpoint");

    // Is this endpoint currently stalled? If so then a size of 0 can
    // be used to block until the stall condition is clear, anything
    // else should result in an immediate callback.
    if (ep1.common.halted) {
        if (0 != ep1.common.buffer_size) {
            ep1_rx_complete(-EAGAIN);
        }
    } else if (0 == ep1.common.buffer_size) {
        // A check to see if the endpoint is halted. It isn't.
        ep1_rx_complete(0);
    } else {
        int status      = *EP1_CONTROL;

        CYG_ASSERT((void*) 0 != ep1.common.buffer, "USB receives should not override the interrupt vectors");
        
        // This indicates the start of a transfer.
        ep1.fetched     = 0;

        // The sent-stall bit may get set by hardware because of
        // a protocol violation. If so it must be cleared here.
        if (0 != (status & EP1_SENT_STALL)) {
            usbs_sa11x0_poke(EP1_CONTROL, EP1_SENT_STALL, 0, EP1_SENT_STALL | EP1_FORCE_STALL);
            status = *EP1_CONTROL;
        }

        // The bogus initial value for the receive-packet-complete
        // bit means that we may start off with an error condition.
        if ((EP1_PACKET_COMPLETE | EP1_PACKET_ERROR) == (status & (EP1_PACKET_COMPLETE | EP1_PACKET_ERROR))) {
            ep1_clear_error();
            ep1_start_rx_packet();
        } else if (0 != (status & EP1_FIFO_NOT_EMPTY)) {
            // No error but data in the fifo. This implies a small
            // initial packet, all held in the fifo.
#ifdef CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL
            *EP1_DMA_BUF_A_SIZE = EP1_MTU;
            ep1.using_buf_a     = true;
#endif
            (void) ep1_process_packet();
            ep1_rx_complete(ep1.fetched);
        } else {
            // Start a new transfer.
            ep1_start_rx_packet();
        }
    }
}

static void
ep1_set_halted(usbs_rx_endpoint* endpoint, cyg_bool new_value)
{
    CYG_ASSERT( endpoint == &ep1.common, "USB set-stall operation involves the wrong endpoint");

    if (ep1.common.halted == new_value) {
        return;
    }
    if (new_value) {
        // The endpoint should be stalled. There is a potential race
        // condition here with a current transfer. Updating the
        // stalled flag means that the dsr will do nothing.
        ep1.common.halted = true;
        HAL_REORDER_BARRIER();

        // Now perform the actual stall. If we are in the middle of a
        // transfer then the stall bit may not get set for a while, so
        // poke() is inappropriate.
        *EP1_CONTROL = EP1_FORCE_STALL;
    } else {
        // The stall condition should be cleared. First take care of
        // things at the hardware level so that a new transfer is
        // allowed.
        usbs_sa11x0_poke(EP1_CONTROL, EP1_SENT_STALL, 0, EP1_SENT_STALL | EP1_FORCE_STALL);
        
        // Now allow new transfers to begin.
        ep1.common.halted = false;
    }
}

// The DSR is invoked following an interrupt. According to the docs an
// endpoint 1 interrupt can only happen if the receive-packet-complete
// bit is set.
static void
usbs_sa11x0_ep1_dsr(void)
{
    int status = *EP1_CONTROL;

    // This assertion does not always hold. During long runs
    // spurious interrupts have been observed.
    // CYG_ASSERT( 0 != (status & EP1_PACKET_COMPLETE), "ep1 dsr should only be invoked when there is data");
    if (0 == (status & EP1_PACKET_COMPLETE)) {
        return;
    }
    
    if (ep1.common.halted) {
        // Do nothing. What may have happened is that a transfer
        // was in progress when the stall bit was set. The
        // set_halted() call above will have taken care of things.
        return;
    }

    // The sent-stall bit should never get set, since we always
    // accept full-size 64-byte packets. Just in case...
    if (0 != (status & EP1_SENT_STALL)) {
        DBG(("ep1_dsr(), sent-stall bit\n"));
        usbs_sa11x0_poke(EP1_CONTROL, EP1_SENT_STALL, 0, EP1_SENT_STALL | EP1_FORCE_STALL);
    }

    // Was there a receive error (CRC, bit-stuffing, fifo-overrun?).
    // Whichever bits of the current packet have been received must be
    // discarded, and the current packet must be retried.
    if (0 != (status & EP1_PACKET_ERROR)) {
        INCR_STAT(ep1_errors);
        ep1_clear_error();
        ep1_start_rx_packet();
    } else {
        // Another packet has been received. Process it, which may
        // complete the transfer or it may leave more to be done.
        //
        // The hardware starts with the wrong default value for
        // the receive-packet-complete bit, so a packet may arrive
        // even though no rx operation has started yet. The
        // packets must be ignored for now. start_rx_packet()
        // will detect data in the fifo and do the right thing.
        int pkt_size;

        if ((unsigned char*)0 != ep1.common.buffer) {
            
            pkt_size = ep1_process_packet();
            INCR_STAT(ep1_receives);
            if (0 != (EP1_PACKET_ERROR & *EP1_CONTROL)) {
                CYG_ASSERT( 0, "an error has occurred inside ep1_process_packet()\n");

            } else if ((ep1.fetched != ep1.common.buffer_size) && (0 != pkt_size) && (0 == (ep1.fetched % EP1_MTU))) {
                ep1_start_rx_packet();
            } else if (ep1.fetched > ep1.common.buffer_size) {
                // The host has sent too much data.
                ep1_rx_complete(-EMSGSIZE);
            } else {
#if 0
                int i;
                diag_printf("------------------------------------------------------\n");
                diag_printf("rx: buf %x, total size %d\n", ep1.common.buffer, ep1.fetched);
                for (i = 0; (i < ep1.fetched) && (i < 128); i+= 8) {
                    diag_printf("rx %x %x %x %x %x %x %x %x\n",
                                ep1.common.buffer[i+0], ep1.common.buffer[i+1], ep1.common.buffer[i+2], ep1.common.buffer[i+3], 
                                ep1.common.buffer[i+4], ep1.common.buffer[i+5], ep1.common.buffer[i+6], ep1.common.buffer[i+7]);
                }
                diag_printf("------------------------------------------------------\n");
#endif
                ep1_rx_complete(ep1.fetched);
            }
        }
    }
}

// Initialization.
//
// This may get called during system start-up or following a reset
// from the host.
static void
usbs_sa11x0_ep1_init(void)
{
#ifdef CYGNUM_DEVS_USB_SA11X0_EP1_DMA_CHANNEL
    // What is the physical address that should be used for
    // transfers?
    unsigned int phys;
    HAL_VIRT_TO_PHYS_ADDRESS( ep1_dma_data, phys);
    phys += (HAL_DCACHE_LINE_SIZE - 1);
    phys -= (phys % HAL_DCACHE_LINE_SIZE);
    CYG_ASSERT( 0 == (phys % HAL_DCACHE_LINE_SIZE), "DMA buffer must be aligned to a cache-line boundary");
    ep1_dma_buf = (unsigned char*)phys;

    // Clear the DMA channel and fix the DMA address register. The
    // value is determined above. 
    *EP1_DMA_CONTROL_CLEAR      = DMA_CONTROL_CLEAR_ALL;
    *EP1_DMA_ADDRESS            = EP1_DMA_ADDRESS_VALUE;
#endif

    // Always allow the host to send full-size packets. If there is a
    // protocol problem and the host sends packets that are too large,
    // it will have to be handled at a level above the device driver.
    //
    // With some silicon revisions reading back the register does not
    // work, so poke_value() is not applicable. This may be an issue
    // with reset timing.
    *USBS_OUT_SIZE      = EP1_MTU - 1;

    // Endpoints should never be halted during a start-up.
    ep1.common.halted           = false;

    // If there has been a reset and there was a receive in progress,
    // abort it. This also takes care of sorting out the endpoint
    // fields ready for the next rx.
    ep1_rx_complete(-EPIPE);
}

#endif  // CYGPKG_DEVS_USB_SA11X0_EP1


#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
// ----------------------------------------------------------------------------
// Endpoint 2 is used for IN transfers, i.e. transmitting data to the
// host. The code is mostly similar to that for endpoint 1, although
// a little bit simpler (e.g. there is no need to worry about
// buffer overflow, that is the host's problem). 
//
// There is a flaw in the hardware design. If the transfer involves an
// exact multiple of 64 bytes then according to the USB spec there
// should be a terminating packet of 0 bytes. However the size of the
// current outgoing packet is determined by the IN_SIZE register and
// that only allows for packets between 1 and 256 bytes - even though
// USB bulk transfers can only go up to 64 bytes. This can be worked
// around at this level by transmitting an extra byte, at the risk of
// upsetting host-side device drivers. Both higher-level and host-side
// code need to be aware of this problem.
//
// Again there appear to be problems with the DMA engine. This time it
// appears that the transmit-fifo-service bit does not always work
// correctly. If you set up a DMA transfer for more than the packet
// size than once the packet has gone out the fifo-service bit just
// remains set, the DMA engine continues to fill the fifo, and the
// data gets lost. Instead DMA can only happen one packet at a time.
// The same issues regarding cache line alignment etc. arise, so
// using a small buffer here is convenient.
//
// 1) process_packet moves a packet from the main transmit buffer
//    into the dma buffer.
// 2) start_tx_packet() starts a transfer to the host
// 3) clear_error() copes with error conditions.

#ifdef CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL
// See the equivalent EP1 DMA definitions.
# define EP2_DMA_MTU   64
# define EP2_DMA_BUFSIZE        ((EP2_DMA_MTU + HAL_DCACHE_LINE_SIZE - 1) - \
                                 ((EP2_DMA_MTU + HAL_DCACHE_LINE_SIZE - 1) % HAL_DCACHE_LINE_SIZE))
# define EP2_DMA_ALLOCSIZE      (EP2_DMA_BUFSIZE + HAL_DCACHE_LINE_SIZE - 1)

static unsigned char  ep2_dma_data[EP2_DMA_ALLOCSIZE];
static unsigned char* ep2_dma_buf;

static void
ep2_process_packet(void)
{
    ep2.pkt_size = ep2.common.buffer_size - ep2.transmitted;
    if (ep2.pkt_size > EP2_MTU) {
        ep2.pkt_size = EP2_MTU;
    }
    // Work around the hardware's inability to send a zero-byte packet.
    if (0 == ep2.pkt_size) {
        ep2.pkt_size = 1;
        ep2_dma_buf[0] = 0;
    } else {
        memcpy(ep2_dma_buf, ep2.common.buffer + ep2.transmitted, ep2.pkt_size);
    }
}

static void
ep2_tx_packet(void)
{
    int dma_size, dma_control_settings;
    
    // CYG_ASSERT( 0 != (*EP2_CONTROL & EP2_FIFO_SERVICE), "Fifo should be empty");

    // Halt any DMA that may still be going on (there should not
    // be any). Then work out the desired DMA settings for the
    // current packet. The DMA engine needs to transfer a multiple
    // of the burst size. If the packet size is not a multiple of
    // the burst size, this presents a minor problem. The chances
    // of an interrupt handler running in time to put the
    // remaining bytes into the fifo by hand are not good, so
    // instead more data is DMA'd in then is absolutely necessary
    // and the surplus bytes will be cleared out during the next
    // tx_packet.
    //
    // A possible optimisation is to detect small packets of
    // less than the fifo size and byte-stuff those, bypassing
    // DMA. It is not clear that would give any performance benefits.
    *EP2_DMA_CONTROL_CLEAR      = DMA_CONTROL_CLEAR_ALL;
    
    dma_size    = ep2.pkt_size + DMA_BURST_SIZE - 1;
    dma_size   -= (dma_size % DMA_BURST_SIZE);

    CYG_ASSERT(dma_size > 0, "DMA calculations should result in a transfer of at least 8 bytes");
    
    // Now clear the fifo, after DMA has stopped.
    usbs_sa11x0_poke(EP2_CONTROL, EP2_SENT_STALL, 0, EP2_SENT_STALL);

    // Should we be using buf_a or buf_b for this transfer?
    // Getting this wrong means that the DMA engine just idles.
    if (0 == (*EP2_DMA_STATUS & DMA_STATUS_BUFFER_IN_USE)) {
        usbs_sa11x0_poke_value(EP2_DMA_BUF_A_ADDRESS, (int) ep2_dma_buf);
        usbs_sa11x0_poke_value(EP2_DMA_BUF_A_SIZE, dma_size);
        dma_control_settings    = DMA_CONTROL_RUN | DMA_CONTROL_START_A;
    } else {
        usbs_sa11x0_poke_value(EP2_DMA_BUF_B_ADDRESS, (int) ep2_dma_buf);
        usbs_sa11x0_poke_value(EP2_DMA_BUF_B_SIZE, dma_size);
        dma_control_settings    = DMA_CONTROL_RUN | DMA_CONTROL_START_B;
    }

    // Poke the tx size register while the fifo is clearing.
    // This operation must be reliable or the host will get
    // confused by funny-sized packets.
    usbs_sa11x0_poke_value(USBS_IN_SIZE, ep2.pkt_size - 1);

    // The USB hardware must be updated before the DMA engine
    // starts filling the fifo. Otherwise ~48% of outgoing
    // packets fail with a DMA underrun. When called from
    // start_tx() there is a race condition: if the host
    // request comes in before the DMA starts then an
    // error interrupt will be raised, to be processed by
    // the DSR, and then the DMA engine gets updated again.
    // Locking the scheduler eliminates this race.
    cyg_drv_dsr_lock();
    usbs_sa11x0_poke(EP2_CONTROL, EP2_PACKET_COMPLETE, 0, EP2_PACKET_COMPLETE | EP2_PACKET_ERROR | EP2_PACKET_UNDERRUN);
    *EP2_DMA_CONTROL_SET        = dma_control_settings;
    cyg_drv_dsr_unlock();
    
    // CYG_ASSERT(0 == (*EP2_CONTROL & EP2_FIFO_SERVICE), "DMA engine should have filled up the fifo by now");
}

// Clearing an error should be a no-op when DMA is involved.
// In practice clearing the packet-complete bit appears to
// have some desirable effects, at the risk of the host
// getting bogus data. This should only happen when there
// is a real transfer in progress: an error early on is
// likely because the PACKET_COMPLETE bit has a bogus initial
// value.
static void
ep2_clear_error(void)
{
    usbs_sa11x0_poke(EP2_CONTROL, EP2_PACKET_COMPLETE, 0, EP2_PACKET_COMPLETE | EP2_PACKET_ERROR | EP2_PACKET_UNDERRUN);
}

#else   // EP2_DMA

// When not using DMA, process_packet() is responsible for filling the
// fifo and keeping a shadow copy in a static buffer. clear_error()
// refills the fifo using the shadow copy. tx_packet() starts the
// actual transfer.
static unsigned char ep2_tx_buf[EP2_MTU];

static void
ep2_process_packet()
{
    int i;
    
    // Clear the fifo, just in case.
    usbs_sa11x0_poke(EP2_CONTROL, EP2_SENT_STALL, 0, EP2_SENT_STALL);

    ep2.pkt_size = ep2.common.buffer_size - ep2.transmitted;
    if (ep2.pkt_size > EP2_MTU) {
        ep2.pkt_size = EP2_MTU;
    }
    if (0 == ep2.pkt_size) {
        ep2.pkt_size    = 1;
        ep2_tx_buf[i]   = 0;
        *EP2_DATA       = 0;
    } else {
        for (i = 0; i < ep2.pkt_size; i++) {
            unsigned int datum = ep2.common.buffer[ep2.transmitted + i];
            ep2_tx_buf[i]   = datum;
            *EP2_DATA       = datum;
        }
    }
}

static void
ep2_tx_packet()
{
    usbs_sa11x0_poke_value(USBS_IN_SIZE, ep2.pkt_size - 1);
    usbs_sa11x0_poke(EP2_CONTROL, EP2_PACKET_COMPLETE, 0, EP2_PACKET_COMPLETE | EP2_PACKET_ERROR | EP2_PACKET_UNDERRUN);
}

static void
ep2_clear_error()
{
    int i;
    // Clear the fifo, just in case.
    usbs_sa11x0_poke(EP2_CONTROL, EP2_SENT_STALL, 0, EP2_SENT_STALL);
    for (i = 0; i < ep2.pkt_size; i++) {
        *EP2_DATA       = ep2_tx_buf[i];
    }
}

#endif  // !EP2_DMA

// A utility routine for completing a transfer. This takes care of the
// callback as well as resetting the buffer.
static void
ep2_tx_complete(int result)
{
    void (*complete_fn)(void*, int)  = ep2.common.complete_fn;
    void* complete_data = ep2.common.complete_data;
    
    ep2.common.buffer           = (unsigned char*) 0;
    ep2.common.buffer_size      = 0;
    ep2.common.complete_fn      = (void (*)(void*, int)) 0;
    ep2.common.complete_data    = (void*) 0;

    if ((void (*)(void*, int))0 != complete_fn) {
        (*complete_fn)(complete_data, result);
    }
}


// The exported interface to start a transmission.
static void
ep2_start_tx(usbs_tx_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep2.common, "USB data transfer involves the wrong endpoint");

    // Is this endpoint currently stalled? If so then a size of 0 can
    // be used to block until the stall condition is clear, anything
    // else should result in an immediate callback.
    if (ep2.common.halted) {
        if (0 != ep2.common.buffer_size) {
            ep2_tx_complete(-EAGAIN);
        }
    } else if (0 == ep2.common.buffer_size) {
        // A check to see if the endpoint is halted. It isn't.
        ep2_tx_complete(0);
    } else {
        // There should not be any errors at the start of a
        // transmission, but if there is one then there is no safe way
        // to recover. process_packet() and tx_packet() will hopefully
        // do the right thing.
        CYG_ASSERT((void*) 0 != ep2.common.buffer, "Transmitting the interrupt vectors is unlikely to be useful");
#if 0
        {
            int i;
            diag_printf("----------------------------------------------\n");
            diag_printf("ep2_start_tx: buf %x, %d bytes\n", ep2.common.buffer, ep2.common.buffer_size);
            for (i = 0; (i < ep2.common.buffer_size) && (i < 128); i+= 8) {
                diag_printf("tx: %x %x %x %x %x %x %x %x\n",
                            ep2.common.buffer[i+0], ep2.common.buffer[i+1], ep2.common.buffer[i+2], ep2.common.buffer[i+3],
                            ep2.common.buffer[i+4], ep2.common.buffer[i+5], ep2.common.buffer[i+6], ep2.common.buffer[i+7]);
            }
            diag_printf("----------------------------------------------\n");
        }
#endif

        // Prepare the first packet for transmission, then send it.
        ep2.transmitted = 0;
        ep2_process_packet();
        ep2_tx_packet();
    }
}

static void
ep2_set_halted(usbs_tx_endpoint* endpoint, cyg_bool new_value)
{
    CYG_ASSERT(endpoint == &ep2.common, "USB set-stall operation involves the wrong endpoint");

    if (ep2.common.halted == new_value) {
        return;
    }
    if (new_value) {
        // The endpoint should be stalled. There is a potential race
        // condition here with the current transfer and DSR invocation.
        // Updating the stalled flag means that the DSR will do nothing.
        ep2.common.halted = true;
        HAL_REORDER_BARRIER();

        // Now perform the actual stall. This may be delayed by the hardware
        // so poke() cannot be used.
        *EP2_CONTROL = EP2_FORCE_STALL;

        // If in the middle of a transfer then that cannot be aborted,
        // the DMA engines etc. would get very confused.
    } else {
        // Take care of the hardware so that a new transfer is allowed. 
        usbs_sa11x0_poke(EP2_CONTROL, EP2_SENT_STALL, 0, EP2_SENT_STALL | EP2_FORCE_STALL);
        ep2.common.halted = false;
    }
}

// The dsr will be invoked when the transmit-packet-complete bit is
// set. Typically this happens when a packet has been completed
// (surprise surprise) but it can also happen for error conditions.
static void
usbs_sa11x0_ep2_dsr(void)
{
    int status = *EP2_CONTROL;
    // This assertion does not always hold - spurious interrupts have
    // been observed if you run for a few hours.
    // CYG_ASSERT( 0 != (status & EP2_PACKET_COMPLETE), "ep2 dsr should only be invoked when the packet-complete bit is set");

    if (0 == (status & EP2_PACKET_COMPLETE)) {
        // Spurious interrupt, do nothing.
    } else if (ep2.common.halted) {
        // There is a possible race condition between a packet
        // completing and the stalled condition being set.
        // set_halted() above does everything that is needed.
    } else if (0 == ep2.pkt_size) {
        // This can happen because of the initial value for the
        // packet-complete bit, allowing the host to retrieve data
        // before the target is ready. The correct action is to do
        // nothing.
    } else if (0 != (status & (EP2_PACKET_ERROR | EP2_PACKET_UNDERRUN))) {
        // A transmit error occurred, the details are not important.
        INCR_STAT(ep2_errors);
        ep2_clear_error();
        ep2_tx_packet();
    } else {
        // Another packet has gone out.
        INCR_STAT(ep2_transmits);
        ep2.transmitted += ep2.pkt_size;
        if ((ep2.transmitted < ep2.common.buffer_size) ||
            ((ep2.transmitted == ep2.common.buffer_size) && (0 == (ep2.common.buffer_size % EP2_MTU)))) {
            ep2_process_packet();
            ep2_tx_packet();
        } else {
            ep2_tx_complete(ep2.transmitted);
        }
    }
}

// Endpoint 2 initialization.
//
// This may be called during system start-up or following a reset
// from the host.
static void
usbs_sa11x0_ep2_init(void)
{
#ifdef CYGNUM_DEVS_USB_SA11X0_EP2_DMA_CHANNEL
    // What is the physical address that should be used for
    // transfers?
    unsigned int phys;
    HAL_VIRT_TO_PHYS_ADDRESS(ep2_dma_data, phys);
    phys += (HAL_DCACHE_LINE_SIZE - 1);
    phys -= (phys % HAL_DCACHE_LINE_SIZE);
    CYG_ASSERT(0 == (phys % HAL_DCACHE_LINE_SIZE), "DMA buffer must be aligned to a cache-line boundary");
    ep2_dma_buf = (unsigned char*) phys;
    
    // Clear the DMA channel completely, otherwise it may not be
    // possible to write the ADDRESS register. Then set the DMA
    // address register. The value is determined above.
    *EP2_DMA_CONTROL_CLEAR      = DMA_CONTROL_CLEAR_ALL;
    *EP2_DMA_ADDRESS            = EP2_DMA_ADDRESS_VALUE;
#endif

    // Endpoints should never be halted after a reset
    ep2.common.halted   = false;

    // If there has been a reset and there was a receive in progress,
    // abort it. This also takes care of clearing the endpoint
    // structure fields.
    ep2_tx_complete(-EPIPE);
}

#endif // CYGPKG_DEVS_USB_SA11X0_EP2

// ----------------------------------------------------------------------------
// Interrupt handling
//
// As much work as possible is deferred to the DSR (or to the debug
// thread). Interrupts for the endpoints are never a problem: the
// variuos packet-complete etc. bits ensure that the endpoints
// remain quiescent until the relevant interrupt has been serviced.
// Suspend and resume are more complicated. A suspend means that
// there has been no activity for 3ms, which should be enough
// time for the whole thing to be handled. A resume means that there
// has been bus activity after a suspend, and again it is infrequent.
//
// Reset appears to be much more complicated. A reset means that the
// host is holding the USB lines to a specific state for 10ms. This is
// detected by the hardware, causing the USB controller to be reset
// (i.e. any pending transfers are discarded, etc.). The reset bit in
// the status register will be set, and an interrupt will be raised.
// Now, in theory the correct thing to do is to process this
// interrupt, block reset interrupts for the duration of these 10ms,
// and wait for further activity such as the control message to set
// the address.
//
// In practice this does not seem to work. Possibly the USB controller
// gets reset continuously while the external reset signal is applied,
// but I have not been able to confirm this. Messing about with the
// reset interrupt control bit causes the system to go off into
// never-never land. 10ms is too short a time to allow for manual
// debugging of what happens. So for now the interrupt source is
// blocked at the interrupt mask level and the dsr will do the
// right thing. This causes a significant number of spurious interrupts
// for the duration of the reset signal and not a lot else can happen.


// Perform reset operations on all endpoints that have been
// configured in. It is convenient to keep this in a separate
// routine to allow for polling, where manipulating the
// interrupt controller mask is a bad idea.
static void
usbs_sa11x0_handle_reset(void)
{
    int old_state = ep0.common.state;

    // Any state change must be reported to higher-level code
    ep0.common.state = USBS_STATE_DEFAULT;
    if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
        (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                      USBS_STATE_CHANGE_RESET, old_state);
    }

    // Reinitialize all the endpoints that have been configured in.
    usbs_sa11x0_ep0_init();
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
    usbs_sa11x0_ep1_init();
#endif    
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
    usbs_sa11x0_ep2_init();
#endif    

    // After a reset we need to handle endpoint interrupts, reset
    // interrupts, and suspend interrupts. There should not be a
    // resume since we have not suspended, but leaving resume
    // interrupts enabled appears to be desirable with some hardware.
    //
    // With some silicon revisions it appears that a longer delay
    // is needed after reset, so this poke() may not work.
    if (!usbs_sa11x0_poke(USBS_CONTROL,
                          CONTROL_INTR_ENABLE(CONTROL_EP_INTR_BITS|CONTROL_RESET_INTR|CONTROL_SUSPEND_INTR|CONTROL_RESUME_INTR),
                          0,
                          CONTROL_INTR_CLEAR(CONTROL_EP_INTR_BITS|CONTROL_RESET_INTR|CONTROL_SUSPEND_INTR|CONTROL_RESUME_INTR))) {
        // DBG(("usbs_sa11x0_handle_reset(), update of control register failed, status %x\n", *USBS_STATUS));
    }
}

// The DSR. This can be invoked directly by poll(), or via the usual
// interrupt subsystem. It acts as per the current value of
// isr_status_bits. If another interrupt goes off while this
// DSR is running, there will be another invocation of the DSR and
// the status bits will be updated.
static void
usbs_sa11x0_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    int status = 0;
    
    CYG_ASSERT(SA11X0_IRQ_USB_SERVICE_REQUEST == vector, "USB DSR should only be invoked for USB interrupts" );
    CYG_ASSERT(0 == data, "The SA11X0 USB DSR needs no global data pointer");

    // There is no atomic swap support, so interrupts have to be
    // blocked. It might be possible to do this via the USBS_CONTROL
    // register, but at the risk of messing up the status register
    // if another interrupt comes in. Blocking interrupts at the
    // processor level is less intrusive on the USB code.

    cyg_drv_isr_lock();
    status = isr_status_bits;
    isr_status_bits = 0;
    cyg_drv_isr_unlock();

    // Reset is special, since it invalidates everything else.
    // If the reset is still ongoing then do not attempt any
    // further processing, there will just be another interrupt.
    // Otherwise handle_reset() does the hard work. Unmasking
    // the interrupt means that another interrupt will occur
    // immediately if reset is still asserted, i.e. no threads
    // will run, but there is no easy way of triggering action
    // at the end of reset.
    if (0 != (status & STATUS_RESET_INTR)) {
        
        int new_status = *USBS_STATUS; 
        if (0 == (new_status & STATUS_RESET_INTR)) {
            usbs_sa11x0_handle_reset();
        } 
        // This unmask is likely to cause another interrupt immediately
        cyg_drv_interrupt_unmask(SA11X0_IRQ_USB_SERVICE_REQUEST);
        
    } else {
        // Process resume first. Ignore any resumes when we are not
        // actually suspended yet, this happens mainly during system
        // startup. If there has been a state change to suspended
        // then we need a matching state change to resumed.
        if (0 != (status & STATUS_RESUME_INTR)) {
            int old_state       = ep0.common.state;
            if (0 != (old_state & USBS_STATE_SUSPENDED)) {
                ep0.common.state   &= ~USBS_STATE_SUSPENDED;
                if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                    (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                              USBS_STATE_CHANGE_RESUMED, old_state);
                }
                // After a resume, all interrupts should be enabled.
                // In theory there is no need to worry about further
                // resume interrupts, but strange hardware behaviour
                // has been observed if resume interrupts are left
                // disabled.
                usbs_sa11x0_poke(USBS_CONTROL,
                                 CONTROL_INTR_ENABLE(CONTROL_EP_INTR_BITS|CONTROL_RESET_INTR|CONTROL_SUSPEND_INTR|CONTROL_RESUME_INTR),
                                 0,
                                 CONTROL_INTR_CLEAR(CONTROL_EP_INTR_BITS|CONTROL_RESET_INTR|CONTROL_SUSPEND_INTR|CONTROL_RESUME_INTR));
            }
        }

        // Now process endpoint interrupts. Control operations on
        // endpoint 0 may have side effects on the other endpoints
        // so it is better to leave them until last.
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1    
        if (0 != (status & STATUS_EP1_INTR)) {
            usbs_sa11x0_ep1_dsr();
        }
#endif    
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
        if (0 != (status & STATUS_EP2_INTR)) {
            usbs_sa11x0_ep2_dsr();
        }
#endif
        if (0 != (status & STATUS_EP0_INTR)) {
            usbs_sa11x0_ep0_dsr();
        }
        
        // Process suspend last, but only if there has not also been
        // a resume. A suspend immediately followed by a resume should
        // be ignored. A resume immediately followed by a suspend
        // would be unfortunate, but suspend means that there has been
        // at least 3ms of inactivity so the DSR latency would have
        // to be pretty bad.
        //
        // Total robustness is possible but requires more work in the ISR.
        if ((0 != (status & STATUS_SUSPEND_INTR)) && (0 == (status & STATUS_RESUME_INTR))) {
            int old_state       = ep0.common.state;
            ep0.common.state   |= USBS_STATE_SUSPENDED;
            if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                              USBS_STATE_CHANGE_SUSPENDED, old_state);
            }
            // We are no longer interested in further suspend interrupts,
            // which could happen every 3 ms, but resume has become
            // very interesting.
            usbs_sa11x0_poke(USBS_CONTROL,
                             CONTROL_INTR_ENABLE(CONTROL_EP_INTR_BITS | CONTROL_RESET_INTR | CONTROL_RESUME_INTR),
                             0,
                             CONTROL_INTR_CLEAR(CONTROL_EP_INTR_BITS | CONTROL_RESET_INTR | CONTROL_RESUME_INTR));
        }
    }
}

// ----------------------------------------------------------------------------
// Optionally the USB code can do most of its processing in a thread
// rather than in a DSR.
#ifdef CYGPKG_DEVS_USB_SA11X0_THREAD
static unsigned char usbs_sa11x0_thread_stack[CYGNUM_DEVS_USB_SA11X0_THREAD_STACK_SIZE];
static cyg_thread    usbs_sa11x0_thread;
static cyg_handle_t  usbs_sa11x0_thread_handle;
static cyg_sem_t     usbs_sa11x0_sem;


static void
usbs_sa11x0_thread_fn(cyg_addrword_t param)
{
    for (;;) {
        cyg_semaphore_wait(&usbs_sa11x0_sem);
        usbs_sa11x0_dsr(SA11X0_IRQ_USB_SERVICE_REQUEST, 0, 0);
    }
    CYG_UNUSED_PARAM(cyg_addrword_t, param);
}

static void
usbs_sa11x0_thread_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_ASSERT( 0 != isr_status_bits, "DSR's should only be scheduled when there is work to do");
    cyg_semaphore_post(&usbs_sa11x0_sem);
    
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
}

#endif

// ----------------------------------------------------------------------------
// The interrupt handler. This does as little as possible.
static cyg_uint32
usbs_sa11x0_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    int old_status_bits = isr_status_bits;
    int status_bits;

    CYG_ASSERT(SA11X0_IRQ_USB_SERVICE_REQUEST == vector, "USB ISR should only be invoked for USB interrupts" );
    CYG_ASSERT(0 == data, "The SA11X0 USB ISR needs no global data pointer" );

    // Read the current status. Reset is special, it means that the
    // whole chip has been reset apart from the one bit in the status
    // register. Nothing should be done about this until the DSR sets
    // the endpoints back to a consistent state and re-enables
    // interrupts in the control register.
    status_bits         = *USBS_STATUS;
    
    if (0 != (status_bits & STATUS_RESET_INTR)) {
        isr_status_bits = STATUS_RESET_INTR;
        *USBS_STATUS = status_bits;
        cyg_drv_interrupt_mask(SA11X0_IRQ_USB_SERVICE_REQUEST);
    } else {
        *USBS_STATUS    = status_bits;
        isr_status_bits |= status_bits;
    }

    // Now keep the rest of the system happy.
    cyg_drv_interrupt_acknowledge(vector);
    return (old_status_bits != isr_status_bits) ? CYG_ISR_CALL_DSR : CYG_ISR_HANDLED;
}

// ----------------------------------------------------------------------------
// Polling support. This acts mostly like the interrupt handler: it
// sets the isr status bits and causes the dsr to run. Reset has to be
// handled specially: polling does nothing as long as reset is asserted.

static void
usbs_sa11x0_poll(usbs_control_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep0.common, "USB poll involves the wrong endpoint");
    
    if (0 != (isr_status_bits & STATUS_RESET_INTR)) {
        // Reset was detected the last time poll() was invoked. If
        // reset is still active, do nothing. Once the reset has
        // completed things can continue.
        if (0 == (STATUS_RESET_INTR & *USBS_STATUS)) {
            isr_status_bits = 0;
            usbs_sa11x0_handle_reset();
        }
    } else {
        isr_status_bits = *USBS_STATUS;
        if (0 != (STATUS_RESET_INTR & isr_status_bits)) {
            // Reset has just been asserted. Do nothing, just continue
            // polling for the duration of the reset signal.
        } else if (0 != isr_status_bits) {
            usbs_sa11x0_dsr(SA11X0_IRQ_USB_SERVICE_REQUEST, 0, (cyg_addrword_t) 0);
        }
    }
}


// ----------------------------------------------------------------------------
// Initialization.

void
usbs_sa11x0_init(void)
{
    // Start by disabling/resetting the hardware. This is easy.
    *USBS_CONTROL       = CONTROL_DISABLE;
    *USBS_CONTROL       = CONTROL_DISABLE;
    *USBS_CONTROL       = CONTROL_DISABLE;

    // The USB bus is now tristated, preventing any communications.
    // This is a good thing, the situation should change only when
    // higher-level code has provided the enumeration data and done an
    // explicit start. 
    usbs_sa11x0_ep0_init();
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
    usbs_sa11x0_ep1_init();
#endif
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
    usbs_sa11x0_ep2_init();
#endif

    // If processing is supposed to happen in a thread rather
    // than in DSR, initialize the threads.
#ifdef CYGPKG_DEVS_USB_SA11X0_THREAD
    cyg_semaphore_init(&usbs_sa11x0_sem, 0);
    cyg_thread_create(CYGNUM_DEVS_USB_SA11X0_THREAD_PRIORITY,
                      &usbs_sa11x0_thread_fn,
                      0,
                      "SA11X0 USB support",
                      usbs_sa11x0_thread_stack,
                      CYGNUM_DEVS_USB_SA11X0_THREAD_STACK_SIZE,
                      &usbs_sa11x0_thread_handle,
                      &usbs_sa11x0_thread
        );
    cyg_thread_resume(usbs_sa11x0_thread_handle);
#endif
    
    // It is also possible and desirable to install the interrupt
    // handler here, even though there will be no interrupts for a
    // while yet.
    cyg_drv_interrupt_create(SA11X0_IRQ_USB_SERVICE_REQUEST,
                             99,        // priority
                             0,         // data
                             &usbs_sa11x0_isr,
#ifdef CYGPKG_DEVS_USB_SA11X0_THREAD
                             &usbs_sa11x0_thread_dsr,
#else                             
                             &usbs_sa11x0_dsr,
#endif                             
                             &usbs_sa11x0_intr_handle,
                             &usbs_sa11x0_intr_data);
    cyg_drv_interrupt_attach(usbs_sa11x0_intr_handle);
    cyg_drv_interrupt_unmask(SA11X0_IRQ_USB_SERVICE_REQUEST);
}

// ----------------------------------------------------------------------------
// Testing support.
usbs_testing_endpoint usbs_testing_endpoints[] = {
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL, 
        endpoint_number     : 0,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,
        endpoint            : (void*) &ep0.common,
#ifdef CYGVAR_DEVS_USB_SA11X0_EP0_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_SA11X0_DEVTAB_BASENAME "0c",
#else        
        devtab_entry        : (const char*) 0,
#endif        
        min_size            : 1,            // zero-byte control transfers are meaningless
        max_size            : 0x0FFFF,      // limit imposed by protocol
        max_in_padding      : 0,
        alignment           : 0
    },
#ifdef CYGPKG_DEVS_USB_SA11X0_EP1
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_BULK,
        endpoint_number     : 1,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT,
        endpoint            : (void*) &ep1.common,
#ifdef CYGVAR_DEVS_USB_SA11X0_EP1_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_SA11X0_DEVTAB_BASENAME "1r",
#else        
        devtab_entry        : (const char*) 0,
#endif        
        min_size            : 1,
        max_size            : -1,           // No hardware or driver limitation
        max_in_padding      : 0,
        alignment           : 0
    },
#endif
#ifdef CYGPKG_DEVS_USB_SA11X0_EP2
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_BULK,
        endpoint_number     : 2,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,
        endpoint            : (void*) &ep2.common,
#ifdef CYGVAR_DEVS_USB_SA11X0_EP2_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_SA11X0_DEVTAB_BASENAME "2w",
#else        
        devtab_entry        : (const char*) 0,
#endif        
        min_size            : 1,
        max_size            : -1,           // No hardware or driver limitation
        max_in_padding      : 1,            // hardware limitation
        alignment           : 0
    },
#endif
    USBS_TESTING_ENDPOINTS_TERMINATOR
};
