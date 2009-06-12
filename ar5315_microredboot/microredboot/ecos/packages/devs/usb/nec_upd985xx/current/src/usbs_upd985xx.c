//==========================================================================
//
//      usbs_upd985xx.c
//
//      Driver for the NEC uPD985xx USB device
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Date:         2001-05-22
//
// This code implements support for the on-chip USB port on the NEC
// uPD985xx family of processors. The code has been developed on the
// uPD98503 and may or may not work on other members of the uPD985xx
// family.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>

#include <pkgconf/hal_mips_upd985xx.h>
#include <pkgconf/devs_usb_upd985xx.h>

#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/error/codes.h>

#include <cyg/io/usb/usb.h>
#include <cyg/io/usb/usbs.h>

// For memcpy()
#include <string.h>

// ----------------------------------------------------------------------------
// Toplevel FIXME's.
//
// The device supports remote wakeups, but this driver does not. Note that
// the device GET_STATUS, SET_FEATURE and CLEAR_FEATURE operations are 
// affected by remote wakeup support.

// ----------------------------------------------------------------------------
// Debugging-related odds and ends.
#if 0
# define DBG(a) diag_printf a
#else
# define DBG(a)
#endif

// ----------------------------------------------------------------------------
// Hardware definitions.
//
// The NEC uPD985Xx on-chip USB device provides the following:
//
// endpoint 0   - control messages only
// endpoint 1   - isochronous transmits
// endpoint 2   - isochronous receives
// endpoint 3   - bulk transmits
// endpoint 4   - bulk receives
// endpoint 5   - interrupt transmits
// endpoint 6   - interrupt receives

// All acess to the USB controller registers goes via the IBUS, which
// always runs little-endian. Hence when the CPU is running
// little-endian no extra work is needed, but when running big-endian
// all register updates involve swapping.
#ifdef CYGPKG_HAL_MIPS_LSBFIRST
# define IBUS_SWAP32(_a_)               (_a_)
# define IBUS_SWAPPTR(_type_, _a_)      (_a_)
#else
# error IBUS_SWAP32() needs to be defined and tested
#endif

// Move an address to kseg1. Or'ing in the relevant bits means
// that this macro will work even if the specified address is
// already in kseg1
#define MIPS_TO_UNCACHED(_a_)  ((void*)(((cyg_uint32)(_a_)) | MIPS_KSEG1_BASE))

// For now access the various registers directly. A structure might
// be marginally more inefficient in that if a function accesses
// several registers this could be handled by a single base plus
// offsets, rather than by separate addresses.
#define USBS_REGISTER(_a_)              ((volatile cyg_uint32*)(MIPS_IO_BASE + UPD985XX_SYSUSB_OFF + (_a_)))
#define USBS_ADDRREG(_type_, _a_)       ((_type_* volatile*)(MIPS_IO_BASE + UPD985XX_SYSUSB_OFF + (_a_)))

#define USBS_GMR                USBS_REGISTER(0x0000)
#define USBS_VER                USBS_REGISTER(0x0004)
#define USBS_GSR1               USBS_REGISTER(0x0010)
#define USBS_IMR1               USBS_REGISTER(0x0014)
#define USBS_GSR2               USBS_REGISTER(0x0018)
#define USBS_IMR2               USBS_REGISTER(0x001C)
#define EP0_CR                  USBS_REGISTER(0x0020)
#define EP1_CR                  USBS_REGISTER(0x0024)
#define EP2_CR                  USBS_REGISTER(0x0028)
#define EP3_CR                  USBS_REGISTER(0x002C)
#define EP4_CR                  USBS_REGISTER(0x0030)
#define EP5_CR                  USBS_REGISTER(0x0034)
#define EP6_CR                  USBS_REGISTER(0x0038)
#define USBS_CMR                USBS_REGISTER(0x0040)
#define USBS_CA                 USBS_ADDRREG(void, 0x0044)
#define USBS_TEPSR              USBS_REGISTER(0x0048)
#define USBS_RP0IR              USBS_REGISTER(0x0050)
#define USBS_RP0AR              USBS_ADDRREG(RxBufferDescriptor, 0x0054)
#define USBS_RP1IR              USBS_REGISTER(0x0058)
#define USBS_RP1AR              USBS_ADDRREG(RxBufferDescriptor, 0x005C)
#define USBS_RP2IR              USBS_REGISTER(0x0060)
#define USBS_RP2AR              USBS_ADDRREG(RxBufferDescriptor, 0x0064)
#define USBS_TMSA               USBS_ADDRREG(TxMailbox, 0x0070)
#define USBS_TMBA               USBS_ADDRREG(TxMailbox, 0x0074)
#define USBS_TMRA               USBS_ADDRREG(TxMailbox, 0x0078)
#define USBS_TMWA               USBS_ADDRREG(TxMailbox, 0x007C)
#define USBS_RMSA               USBS_ADDRREG(RxMailbox, 0x0080)
#define USBS_RMBA               USBS_ADDRREG(RxMailbox, 0x0084)
#define USBS_RMRA               USBS_ADDRREG(RxMailbox, 0x0088)
#define USBS_RMWA               USBS_ADDRREG(RxMailbox, 0x008C)

// There are additional counter registers from offset 0x100 onwards.
// These registers are not used by the driver, and anyway may not be
// available on all hardware.

// The General Mode register USBS_GMR
#define USBS_GMR_VT                     (0x01 << 23)
#define USBS_GMR_FA_MASK                (0x7F << 16)
#define USBS_GMR_FA_SHIFT               16
#define USBS_GMR_SOFINTVL_MASK          (0x00FF << 8)
#define USBS_GMR_SOFINTVL_SHIFT         8
#define USBS_GMR_SOFINTVL_DEFAULT_VALUE (0x18 << 8)
#define USBS_GMR_AU                     (0x01 << 2)
#define USBS_GMR_LE                     (0x01 << 1)
#define USBS_GMR_RR                     (0x01 << 0)

// The Frame Number/Version register
#define USBS_VER_UVER_MASK              (0x0FFFF << 16)
#define USBS_VER_UVER_SHIFT             16
#define USBS_VER_UFNR_MASK              (0x03FF << 0)
#define USBS_VER_UFNR_SHIFT             0

// General status register 1
#define USBS_GSR1_GSR2                  (0x01 << 31)
#define USBS_GSR1_TMF                   (0x01 << 23)
#define USBS_GSR1_RMF                   (0x01 << 22)
#define USBS_GSR1_RPE2                  (0x01 << 21)
#define USBS_GSR1_RPE1                  (0x01 << 20)
#define USBS_GSR1_RPE0                  (0x01 << 19)
#define USBS_GSR1_RPA2                  (0x01 << 18)
#define USBS_GSR1_RPA1                  (0x01 << 17)
#define USBS_GSR1_RPA0                  (0x01 << 16)
#define USBS_GSR1_DER                   (0x01 << 10)
#define USBS_GSR1_EP2FO                 (0x01 << 9)
#define USBS_GSR1_EP1FU                 (0x01 << 8)
#define USBS_GSR1_EP6RF                 (0x01 << 7)
#define USBS_GSR1_EP5TF                 (0x01 << 6)
#define USBS_GSR1_EP4RF                 (0x01 << 5)
#define USBS_GSR1_EP3TF                 (0x01 << 4)
#define USBS_GSR1_EP2RF                 (0x01 << 3)
#define USBS_GSR1_EP1TF                 (0x01 << 2)
#define USBS_GSR1_EP0RF                 (0x01 << 1)
#define USBS_GSR1_EP0TF                 (0x01 << 0)

// The Interrupt mask 1 bits correspond to the GSR1 bits above

// General status register 2
#define USBS_GSR2_FW                    (0x01 << 21)
#define USBS_GSR2_IFN                   (0x01 << 20)
#define USBS_GSR2_IEA                   (0x01 << 19)
#define USBS_GSR2_URSM                  (0x01 << 18)
#define USBS_GSR2_URST                  (0x01 << 17)
#define USBS_GSR2_USPD                  (0x01 << 16)
#define USBS_GSR2_EP2OS                 (0x01 << 7)
#define USBS_GSR2_EP2ED                 (0x01 << 6)
#define USBS_GSR2_EP2ND                 (0x01 << 5)
#define USBS_GSR2_EP1NT                 (0x01 << 4)
#define USBS_GSR2_EP1ET                 (0x01 << 3)
#define USBS_GSR2_EP1ND                 (0x01 << 2)
#define USBS_GSR2_ES                    (0x01 << 1)
#define USBS_GSR2_SL                    (0x01 << 0)

// Interrupt mask 2 bits correspond to GSR2

// Endpoint control registers.
// EP0 - control messages
#define EP0_CR_EP0EN                    (0x01 << 31)
#define EP0_CR_ISS                      (0x01 << 20)
#define EP0_CR_INAK                     (0x01 << 19)
#define EP0_CR_OSS                      (0x01 << 18)
#define EP0_CR_NHSK0                    (0x01 << 17)
#define EP0_CR_ONAK                     (0x01 << 16)
#define EP0_CR_MAXP0_MASK               (0x7F << 0)
#define EP0_CR_MAXP0_SHIFT              0

// EP1 - isochronous transmits
#define EP1_CR_EP1EN                    (0x01 << 31)
#define EP1_CR_TM1                      (0x01 << 19)
#define EP1_CR_TM1_MASK                 (0x01 << 19)
#define EP1_CR_TM1_SZLP                 (0x00 << 19)
#define EP1_CR_TM1_NZLP                 (0x01 << 19)
#define EP1_CR_MAXP1_MASK               (0x3FF << 0)
#define EP1_CR_MAXP1_SHIFT              0

// EP2 - isochronous receives
#define EP2_CR_EP2EN                    (0x01 << 31)
#define EP2_CR_RM2_MASK                 (0x03 << 19)
#define EP2_CR_RM2_NORMAL               (0x00 << 19)
#define EP2_CR_RM2_ASSEMBLE             (0x02 << 19)
#define EP2_CR_RM2_SEPARATE             (0x03 << 19)
#define EP2_CR_MAXP2_MASK               (0x3FF << 0)
#define EP2_CR_MAXP2_SHIFT              0

// EP3 - bulk transmits
#define EP3_CR_EP3EN                    (0x01 << 31)
#define EP3_CR_TM3                      (0x01 << 19)
#define EP3_CR_TM3_MASK                 (0x01 << 19)
#define EP3_CR_TM3_SZLP                 (0x00 << 19)
#define EP3_CR_TM3_NZLP                 (0x01 << 19)
#define EP3_CR_SS3                      (0x01 << 18)
#define EP3_CR_NAK3                     (0x01 << 16)
#define EP3_CR_MAXP3_MASK               (0x7F << 0)
#define EP3_CR_MAXP3_SHIFT              0

// EP4 - bulk receives
#define EP4_CR_EP4EN                    (0x01 << 31)
#define EP4_CR_RM4_MASK                 (0x03 << 19)
#define EP4_CR_RM4_NORMAL               (0x00 << 19)
#define EP4_CR_RM4_ASSEMBLE             (0x02 << 19)
#define EP4_CR_RM4_SEPARATE             (0x03 << 19)
#define EP4_CR_SS4                      (0x01 << 18)
#define EP4_CR_NHSK4                    (0x01 << 17)
#define EP4_CR_NAK4                     (0x01 << 16)
#define EP4_CR_MAXP4_MASK               (0x7F << 0)
#define EP4_CR_MAXP4_SHIFT              0

// EP5 - interrupt transmits
#define EP5_CR_EP5EN                    (0x01 << 31)
#define EP5_CR_FM                       (0x01 << 19)
#define EP5_CR_SS5                      (0x01 << 18)
#define EP5_CR_NAK5                     (0x01 << 16)
#define EP5_CR_MAXP5_MASK               (0x7F << 0)
#define EP5_CR_MAXP5_SHIFT              0

// EP6 - interrupt receives
#define EP6_CR_EP6EN                    (0x01 << 31)
#define EP6_CR_SS6                      (0x01 << 18)
#define EP6_CR_NHSK6                    (0x01 << 17)
#define EP6_CR_NAK6                     (0x01 << 16)
#define EP6_CR_MAXP6_MASK               (0x7F << 0)
#define EP6_CR_MAXP6_SHIFT              0

// Some bits which can be applied to multiple transmit or receive
// endpoint control registers, thus avoiding unnecessary code
// duplication. These will not work for the isochronous endpoints
// because those are just too special.
#define EPtx_CR_EpxEN                   (0x01 << 31)
#define EPtx_CR_SSx                     (0x01 << 18)
#define EPtx_CR_NAKx                    (0x01 << 16)
#define EPtx_CR_MAXPx_MASK              (0x7F << 0)
#define EPtx_CR_MAXPx_SHIFT             0

#define EPrx_CR_EPxEN                   (0x01 << 31)
#define EPrx_CR_SSx                     (0x01 << 18)
#define EPrx_CR_NHSKx                   (0x01 << 17)
#define EPrx_CR_NAKx                    (0x01 << 16)
#define EPrx_CR_MAXPx_MASK              (0x7F << 0)
#define EPrx_CR_MAXPx_SHIFT             0

// USB command register
#define USBS_CMR_BUSY                   (0x01 << 31)
#define USBS_CMR_COMMAND_MASK           (0x07 << 24)
#define USBS_CMR_COMMAND_SHIFT          24
#define USBS_CMR_COMMAND_TX_EP0         (0x00 << 24)
#define USBS_CMR_COMMAND_TX_EP1         (0x01 << 24)
#define USBS_CMR_COMMAND_TX_EP3         (0x02 << 24)
#define USBS_CMR_COMMAND_TX_EP5         (0x03 << 24)
#define USBS_CMR_COMMAND_ADD_POOL0      (0x04 << 24)
#define USBS_CMR_COMMAND_ADD_POOL1      (0x05 << 24)
#define USBS_CMR_COMMAND_ADD_POOL2      (0x06 << 24)
#define USBS_CMR_SIZE_MASK              (0x0FFFF << 0)
#define USBS_CMR_SIZE_SHIFT             0

// TX Endpoint status
#define USBS_TEPSR_EP5TS_MASK           (0x03 << 24)
#define USBS_TEPSR_EP5TS_SHIFT          24
#define USBS_TEPSR_EP5TS_IDLE           (0x00 << 24)
#define USBS_TEPSR_EP5TS_ONE            (0x01 << 24)
#define USBS_TEPSR_EP5TS_TWO            (0x02 << 24)
#define USBS_TEPSR_EP3TS_MASK           (0x03 << 16)
#define USBS_TEPSR_EP3TS_SHIFT          16
#define USBS_TEPSR_EP3TS_IDLE           (0x00 << 16)
#define USBS_TEPSR_EP3TS_ONE            (0x01 << 16)
#define USBS_TEPSR_EP3TS_TWO            (0x02 << 16)
#define USBS_TEPSR_EP1TS_MASK           (0x03 << 8)
#define USBS_TEPSR_EP1TS_SHIFT          8
#define USBS_TEPSR_EP1TS_IDLE           (0x00 << 8)
#define USBS_TEPSR_EP1TS_ONE            (0x01 << 8)
#define USBS_TEPSR_EP1TS_TWO            (0x02 << 8)
#define USBS_TEPSR_EP0TS_MASK           (0x03 << 0)
#define USBS_TEPSR_EP0TS_SHIFT          0
#define USBS_TEPSR_EP0TS_IDLE           (0x00 << 0)
#define USBS_TEPSR_EP0TS_ONE            (0x01 << 0)
#define USBS_TEPSR_EP0TS_TWO            (0x02 << 0)

// Receive pools. The RP0IR, RP1IR and RP2IR registers
// all use the same bits.
#define USBS_RPxIR_AL_MASK              (0x07 << 28)
#define USBS_RPxIR_AL_SHIFT             28
#define USBS_RPxIR_AL_NONE              (0 << 28)
#define USBS_RPxIR_RNOD_MASK            (0x0FFFF << 0)
#define USBS_RPxIR_RNOD_SHIFT           0

// The other registers do not have special bits.

// Data transfers involve buffer descriptors and mailboxes. The
// relevant data structures and fields need to be defined. For now
// assume 32-bit mode of operation, i.e. there will be no padding
// between two successive 32-bit entities

// A transmit packet directory consists of up to 255 buffer
// descriptors. Each buffer descriptor specifies a buffer and a size
// of up to 64K. 

typedef struct TxBufferDescriptor {
    cyg_uint32  control;
    void*       buffer;
} TxBufferDescriptor;

#define TXBUFDESC_CTRL_LAST                     (0x01 << 31)
#define TXBUFDESC_CTRL_BUFDESC_MASK             (0x01 << 30)
#define TXBUFDESC_CTRL_BUFDESC_SHIFT            30
#define TXBUFDESC_CTRL_BUFDESC_LINK             (0x00 << 30)
#define TXBUFDESC_CTRL_BUFDESC_BUFDESC          (0x01 << 30)
#define TXBUFDESC_CTRL_SIZE_MASK                (0x0FFFF << 0)
#define TXBUFDESC_CTRL_SIZE_SHIFT               0

// The result of a transmit operation gets written to a mailbox
// structure in memory.
typedef struct TxMailbox {
    cyg_uint32  status;
} TxMailbox;

#define TXMBOX_STATUS_IBUS_ERROR                (0x01 << 10)
#define TXMBOX_STATUS_UNDERRUN                  (0x01 << 9)
#define TXMBOX_STATUS_MODE_MASK                 (0x01 << 8)
#define TXMBOX_STATUS_MODE_SHIFT                8
#define TXMBOX_STATUS_MODE_SZLP                 (0x00 << 8)
#define TXMBOX_STATUS_MODE_NZLP                 (0x01 << 8)
#define TXMBOX_STATUS_EPN_MASK                  (0x07 << 0)
#define TXMBOX_STATUS_EPN_SHIFT                 0
#define TXMBOX_STATUS_EPN_EP0                   (0x00 << 0)
#define TXMBOX_STATUS_EPN_EP1                   (0x02 << 0)
#define TXMBOX_STATUS_EPN_EP3                   (0x04 << 0)
#define TXMBOX_STATUS_EPN_EP5                   (0x06 << 0)

// Now for receive operations. This involves adding buffer descriptors
// to one of three pools. The pools are managed by registers.
typedef struct RxBufferDescriptor {
    cyg_uint32          control;
    void*               buffer;
} RxBufferDescriptor;

#define RXBUFDESC_CTRL_LAST                     (0x01 << 31)
#define RXBUFDESC_CTRL_BUFDESC_MASK             (0x01 << 30)
#define RXBUFDESC_CTRL_BUFDESC_SHIFT            30
#define RXBUFDESC_CTRL_BUFDESC_LINK             (0x00 << 30)
#define RXBUFDESC_CTRL_BUFDESC_BUFDESC          (0x01 << 30)
#define RXBUFDESC_CTRL_SIZE_MASK                (0x0FFFF << 0)
#define RXBUFDESC_CTRL_SIZE_SHIFT               0

typedef struct RxMailbox {
    cyg_uint32          status;
    void*               address;
} RxMailbox;

#define RXMBOX_STATUS_EPN_MASK                  (0x07 << 29)
#define RXMBOX_STATUS_EPN_SHIFT                 29
#define RXMBOX_STATUS_EPN_EP0                   (0x01 << 29)
#define RXMBOX_STATUS_EPN_EP2                   (0x03 << 29)
#define RXMBOX_STATUS_EPN_EP4                   (0x05 << 29)
#define RXMBOX_STATUS_EPN_EP6                   (0x07 << 29)
#define RXMBOX_STATUS_CORRUPTION                (0x01 << 25)
#define RXMBOX_STATUS_IBUS_ERROR                (0x01 << 24)
#define RXMBOX_STATUS_SETUP_MASK                (0x01 << 23)
#define RXMBOX_STATUS_SETUP_SHIFT               23
#define RXMBOX_STATUS_SETUP_NORMAL              (0x00 << 23)
#define RXMBOX_STATUS_SETUP_SETUP               (0x01 << 23)
#define RXMBOX_STATUS_OVERRUN                   (0x01 << 22)
#define RXMBOX_STATUS_DATA_TOGGLE               (0x01 << 21)
#define RXMBOX_STATUS_CRC                       (0x01 << 20)
#define RXMBOX_STATUS_BIT_STUFFING              (0x01 << 19)
#define RXMBOX_STATUS_64K                       (0x01 << 18)
#define RXMBOX_STATUS_MODE_MASK                 (0x03 << 16)
#define RXMBOX_STATUS_MODE_SHIFT                16
#define RXMBOX_STATUS_MODE_NORMAL               (0x00 << 16)
#define RXMBOX_STATUS_MODE_NORMAL2              (0x01 << 16)
#define RXMBOX_STATUS_MODE_ASSEMBLE             (0x02 << 16)
#define RXMBOX_STATUS_MODE_SEPARATE             (0x03 << 16)
#define RXMBOX_STATUS_SIZE_MASK                 (0x0FFFF << 0)
#define RXMBOX_STATUS_SIZE_SHIFT                0


// ----------------------------------------------------------------------------
// Hardware work around - see NEC erratum S1, CPU to IBUS write restriction.
// Reading back from the USB device after every write prevents any problems.
// Strictly speaking it is only necessary to do this after every three
// writes, but if there is concurrent ethernet activity then doing it
// after eveyr write is safer. The frame number/version register seems
// like a good one to read back from.

#ifdef CYGIMP_DEVS_USB_UPD985XX_IBUS_WRITE_LIMIT
# define FLUSH_IBUS()   \
      CYG_MACRO_START   \
      (void)*USBS_VER;  \
      CYG_MACRO_END

#else
# define FLUSH_IBUS()       CYG_EMPTY_STATEMENT
#endif

// ----------------------------------------------------------------------------
// Static data. There is a data structure for each endpoint. The
// implementation is essentially a private class that inherits from
// common classes for control and data endpoints, but device drivers
// are supposed to be written in C so some ugliness is required.
//
// Devtab entries are defined in usbs_upd985xx_data.cxx to make sure
// that the linker does not garbage-collect them.

// Support for the interrupt handling code.
static cyg_interrupt usbs_upd985xx_intr_data;
static cyg_handle_t  usbs_upd985xx_intr_handle;

// The various bits in the two interrupt status registers are read-once,
// i.e. reading the register clears the bits. Since much of the processing
// is deferred to DSR level, it is necessary to keep track of pending
// interrupts in separate variables. If another interrupt happens during
// DSR processing, these variables will be updated. The main DSR loops
// until there are no interesting bits left. Interrupts have to be
// disabled briefly when clearing bits.
static volatile cyg_uint32 usbs_upd985xx_gsr1   = 0;
static volatile cyg_uint32 usbs_upd985xx_gsr2   = 0;

// Many of the interrupt bits are of no interest and it is convenient
// to mask them out in the ISR, thus avoiding unnecessary dsr
// invocations.
static cyg_uint32 usbs_upd985xx_gsr1_mask       = 0;
static cyg_uint32 usbs_upd985xx_gsr2_mask       = 0;

// Sizes for the receive and transmit mboxes.
// NOTE: it is not clear what the optimal size for these
// mailboxes is. For receives maybe one per rx endpoint,
// plus a spare. For transmits maybe just two, since only
// one transmit at a time is supported. Mailboxes are
// relatively small, so for now four each should be ok.
#define RXMBOX_COUNT    4
#define TXMBOX_COUNT    4

// There is one instance of this data structure. It is allocated
// in kseg0 cached memory, but during initialization a separate
// pointer value is set to the kseg1 uncached equivalent. This
// makes it easier to point the hardware at uncached memory without
// having to worry about cache line boundaries everywhere.

typedef struct uncached_data {
    // This partial cacheline does not actually store any data.
    // However it ensures that the data does not share a cacheline
    // with some other static, with updates to that other static
    // causing funny side effects on the uncached data. There is a
    // memory optimisation of subtracting sizeof(RxMailbox.status),
    // i.e. exploit knowledge of alignment.
    unsigned char       cacheline_start[HAL_DCACHE_LINE_SIZE - sizeof(cyg_uint32)];

    RxMailbox           rx_mboxes[RXMBOX_COUNT];
    TxMailbox           tx_mboxes[TXMBOX_COUNT];

    // For transmits a single buffer descriptor per endpoint suffices.
    // If transmit locking is enabled then actually a single buffer
    // descriptor for the whole system would suffice.
    TxBufferDescriptor  ep0_tx_bufdesc;
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
    TxBufferDescriptor  ep3_tx_bufdesc;
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
    TxBufferDescriptor  ep5_tx_bufdesc;
#endif
    
    
    // More buffer descriptors are needed than might be expected, see
    // the start_rx routines. 
    RxBufferDescriptor  ep0_rx_bufdescs[4];
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
    RxBufferDescriptor  ep4_rx_bufdescs[8];
#endif    

    
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
    // Space for the start and end of a transfer, avoiding problems
    // with invalidating partial cache lines.
    unsigned char       ep4_head[HAL_DCACHE_LINE_SIZE];
    unsigned char       ep4_tail[HAL_DCACHE_LINE_SIZE];
#endif
    
    // The "big" buffers come last, reducing the offsets for the previous
    // structures. It is not clear this really matters for MIPS.
    //
    // Endpoint 0 receive and transmit buffers. A transmit buffer is
    // convenient because the hardware pretty much expects all of the
    // data to be in contiguous memory, as opposed to the normal eCos
    // USB driver model with refill buffers etc. An alternative
    // implementation would keep the data in separate areas but would
    // require lots of TxBufferDescriptors, so in memory terms the
    // overheads of a single transmit buffer are not as big as might
    // seem. It might be possible to get things working eight bytes
    // at a time since the hardware appears to depend on zero-byte
    // terminating packets in places, but that has not been attempted.
    //
    // A separate receive buffer is useful because it can be placed in
    // uncached memory, avoiding the need for invalidation and
    // worrying about other data in the cache lines. Note that this
    // buffer may also get used for endpoint 6 interrupt receives
    // because the two endpoints share a single pool.
    unsigned char       ep0_rx_buffer[CYGNUM_DEVS_USB_UPD985XX_EP0_RXBUFSIZE];
    unsigned char       ep0_tx_buffer[CYGNUM_DEVS_USB_UPD985XX_EP0_TXBUFSIZE];

    // Another cacheline to prevent overlap with other statics.
    // This has to be full-sized since the previous field is only byte-aligned.
    unsigned char       cacheline_end[HAL_DCACHE_LINE_SIZE];
} uncached_data;

// This data structure is quite large so making it all uninitialized
// means a potentially big saving in ROM-booting systems. This
// requires additional effort by the endpoint initialization routines.
static uncached_data    cached_copy;

static uncached_data*   uncached        = (uncached_data*)0;

// Endpoint 0. See the description below.

static void usbs_upd985xx_ep0_start(usbs_control_endpoint*);
static void usbs_upd985xx_poll(usbs_control_endpoint*);

typedef struct ep0_impl {
    usbs_control_endpoint       common;
    cyg_bool                    rx_expecting_data;
    cyg_bool                    rx_indicator_valid;
    RxMailbox                   rx_indicator;
    cyg_bool                    tx_indicator_valid;
    TxMailbox                   tx_indicator;
    cyg_bool                    tx_needs_zero_transfer;
    cyg_uint32                  tx_size;
} ep0_impl;

static ep0_impl ep0 = {
    common:
    {
        state:                  USBS_STATE_POWERED, // The hardware does not distinguish  between detached, attached and powered.
        enumeration_data:       (usbs_enumeration_data*) 0,
        start_fn:               &usbs_upd985xx_ep0_start,
        poll_fn:                &usbs_upd985xx_poll,
        interrupt_vector:       CYGNUM_HAL_INTERRUPT_USB,
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
    rx_expecting_data:          false,
    rx_indicator_valid:         false,
    rx_indicator:               { 0, (void*) 0 },
    tx_indicator_valid:         false,
    tx_indicator:               { 0 },
    tx_needs_zero_transfer:     0
};

extern usbs_control_endpoint usbs_upd985xx_ep0 __attribute__((alias ("ep0")));

// Endpoint 1, isochronous transmits. This endpoint is not yet
// supported. Although the interface for bulk transmits should be
// mostly re-usable, there are some additional error conditions if
// either the host or the target fails to achieve the desired
// throughput.

// Endpoint 2, isochronous receives. Not yet supported for now, just
// like endpoint 1.

// Endpoints 3 and 5 can share some code.
#if defined(CYGPKG_DEVS_USB_UPD985XX_EP3) || defined(CYGPKG_DEVS_USB_UPD985XX_EP5)
// Endpoint 3, bulk transmits, and endpoint 5, either interrupt transmits
// or emulation of bulk transmits. The hardware does most
// of the work.
typedef struct ep35_impl {
    usbs_tx_endpoint        common;
    cyg_bool                tx_indicator_valid;
    TxMailbox               tx_indicator;
    int                     send_command;
    volatile cyg_uint32*    cr;
    TxBufferDescriptor*     tx_bufdesc;
} ep35_impl;

# ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
static void ep3_start_tx(usbs_tx_endpoint*);
static void ep3_set_halted(usbs_tx_endpoint*, cyg_bool);

static ep35_impl ep3 = {
    common: {
        start_tx_fn:        &ep3_start_tx,
        set_halted_fn:      &ep3_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (const unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    },
    tx_indicator_valid: false,
    tx_indicator:       { 0 },
    send_command:       USBS_CMR_COMMAND_TX_EP3,
    cr:                 EP3_CR,
    tx_bufdesc:         0       // Needs run-time initialization
};

extern usbs_tx_endpoint usbs_upd985xx_ep3 __attribute__ ((alias ("ep3")));
# endif

# ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
static void ep5_start_tx(usbs_tx_endpoint*);
static void ep5_set_halted(usbs_tx_endpoint*, cyg_bool);

static ep35_impl ep5 = {
    common: {
        start_tx_fn:        &ep5_start_tx,
        set_halted_fn:      &ep5_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (const unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    },
    tx_indicator_valid: false,
    tx_indicator:       { 0 },
    send_command:       USBS_CMR_COMMAND_TX_EP5,
    cr:                 EP5_CR,
    tx_bufdesc:         0       // Needs run-time initialization
};

extern usbs_tx_endpoint usbs_upd985xx_ep5 __attribute__ ((alias ("ep5")));
# endif
#endif


#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
// Endpoint 4, bulk receives. Again the hardware does the hard work.
// Receive pool 2 is reserved for this endpoint.

typedef struct ep4_impl {
    usbs_rx_endpoint    common;
    cyg_uint32          head_size;
    cyg_uint32          direct_size;
    cyg_uint32          tail_size;
    cyg_bool            rx_indicator_valid;
    RxMailbox           rx_indicator;
    cyg_int32           tail_index;
} ep4_impl;

static void ep4_start_rx(usbs_rx_endpoint*);
static void ep4_set_halted(usbs_rx_endpoint*, cyg_bool);

static ep4_impl ep4 = {
    common: {
        start_rx_fn:        &ep4_start_rx,
        set_halted_fn:      &ep4_set_halted,
        complete_fn:        (void (*)(void*, int)) 0,
        complete_data:      (void*) 0,
        buffer:             (unsigned char*) 0,
        buffer_size:        0,
        halted:             0,
    },
    rx_indicator_valid:     false,
    rx_indicator:           { 0, (void*) 0 },
    tail_index:             -1
};

extern usbs_rx_endpoint usbs_upd985xx_ep4 __attribute__((alias ("ep4")));
#endif

// Endpoint 6, interrupt receives. Not yet implemented. There may
// be conflicts because the hardware is shared with endpoint 0.

// ----------------------------------------------------------------------------
// Mailbox support.
//
// The transmit and receive mailboxes are shared between the
// appropriate endpoints. This causes some complications if e.g.
// transmits on several endpoints complete at the same time. For
// example the tx mailbox might contain send indicators for endpoints
// 3 and 0, but the DSR code will process endpoint 0 before endpoint
// 3.
//
// This device driver works on the basis that there can be only one
// transmit and/or receive in progress for any given endpoint, so the
// relevant information can be extracted from the mailbox and put into
// the per-endpoint structures. The routines below can be used to
// move data from the mailboxes. They will be called in DSR context
// so there is no need to worry about locking.

static void
drain_tx_mailbox(void)
{
    TxMailbox*  tmra    = IBUS_SWAPPTR(TxMailbox, *USBS_TMRA);
    TxMailbox*  tmwa    = IBUS_SWAPPTR(TxMailbox, *USBS_TMWA);
    if (tmra != tmwa) {
        do {
            TxMailbox mbox = *tmra;
            tmra++;
            if (tmra == &(uncached->tx_mboxes[TXMBOX_COUNT])) {
                tmra = &(uncached->tx_mboxes[0]);
            }

            switch(mbox.status & TXMBOX_STATUS_EPN_MASK) {
              case TXMBOX_STATUS_EPN_EP0:
                CYG_ASSERT(false == ep0.tx_indicator_valid, "Only one ep0 transmit should be in progress at a time");
                ep0.tx_indicator        = mbox;
                ep0.tx_indicator_valid  = true;
                break;
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3                
              case TXMBOX_STATUS_EPN_EP3:
                CYG_ASSERT(false == ep3.tx_indicator_valid, "Only one ep3 transmit should be in progress at a time");
                ep3.tx_indicator        = mbox;
                ep3.tx_indicator_valid  = true;
                break;
#endif                
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
              case TXMBOX_STATUS_EPN_EP5:
                CYG_ASSERT(false == ep5.tx_indicator_valid, "Only one ep5 transmit should be in progress at a time");
                ep5.tx_indicator        = mbox;
                ep5.tx_indicator_valid  = true;
                break;
#endif                
              default:
                break;
            }
        } while (tmra != tmwa);
        *USBS_TMRA = IBUS_SWAPPTR(TxMailbox, tmra); FLUSH_IBUS();
    }
}

static void
drain_rx_mailbox(void)
{
    RxMailbox*  rmra    = IBUS_SWAPPTR(RxMailbox, *USBS_RMRA);
    RxMailbox*  rmwa    = IBUS_SWAPPTR(RxMailbox, *USBS_RMWA);
    
    if (rmra != rmwa) {
        do {
            RxMailbox mbox = *rmra;
            rmra++;
            if (rmra == &(uncached->rx_mboxes[RXMBOX_COUNT])) {
                rmra = &(uncached->rx_mboxes[0]);
            }

            switch(mbox.status & RXMBOX_STATUS_EPN_MASK) {
              case RXMBOX_STATUS_EPN_EP0:
                // Ignore zero-byte transfers. It is not clear why
                // these happen, but they have been observed.
                if (0 != (mbox.status & RXMBOX_STATUS_SIZE_MASK)) {
                    CYG_ASSERT(false == ep0.rx_indicator_valid, "Only one ep0 receive should be in progress at a time");
                    ep0.rx_indicator        = mbox;
                    ep0.rx_indicator_valid  = true;
                }
                break;
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
              case RXMBOX_STATUS_EPN_EP4:
                // If an error occurs then the hardware may report
                // multiple rx completions, each with an IBUS error
                // indicator. For now only the last rx indicator is
                // taken into account, which means we could lose
                // a successful receive that happens to be followed
                // by an error.
                // NOTE: any possibility of improving on this?
#if 1                
                CYG_ASSERT(false == ep4.rx_indicator_valid, "Only one ep4 receive should be in progress at a time");
#endif                
                ep4.rx_indicator        = mbox;
                ep4.rx_indicator_valid  = true;
                break;
#endif                
              default:
                break;
            } 
        } while (rmra != rmwa);
        *USBS_RMRA = IBUS_SWAPPTR(RxMailbox, rmra); FLUSH_IBUS();
    }
}

// ----------------------------------------------------------------------------
// Transmit locking.
//
// According to NEC errata U3 and U4 the hardware may exhibit
// undesirable behaviour if there are concurrent transmissions. There
// are various ways of resolving this, but the simplest is to perform
// locking in software so that at most one transmit endpoint is in use
// at any one time. This approach works fine if transmissions only
// involve one tx endpoint plus the control endpoint because the
// control endpoint generally only gets used during initialization and
// the other endpoint only gets used after initialization. If multiple
// transmit endpoints are used then locking in software becomes less
// acceptable, especially if isochronous transfers are used because
// timing is important for those.
//
// There is a theoretical problem if e.g. there is a very large bulk
// transfer on a busy bus and it is necessary to respond to a control
// message. The control reply would be delayed, possibly causing a
// violation of the USB standard and a timeout on the host.

#ifdef CYGIMP_DEVS_USB_UPD985XX_SERIALIZE_TRANSMITS
static void ep0_start_tx(void);
# if defined(CYGPKG_DEVS_USB_UPD985XX_EP3) || defined(CYGPKG_DEVS_USB_UPD985XX_EP5)
static void ep35_start_tx(ep35_impl*);
# endif

static cyg_bool tx_in_progress  = false;
static cyg_bool ep0_tx_pending  = false;
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
static cyg_bool ep3_tx_pending  = false;
# endif
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
static cyg_bool ep5_tx_pending  = false;
# endif

// Invoked from ep?_start_tx(). Scheduling may or may not be locked.
static cyg_bool
tx_try_lock(cyg_bool* which)
{
    cyg_bool result;
    cyg_drv_dsr_lock();
    if (tx_in_progress) {
        result          = false;
        *which          = true;
    } else {
        result          = true;
        tx_in_progress  = true;
    }
    cyg_drv_dsr_unlock();
    return result;
}

// Invoked only from dsr context.
static void
tx_unlock(void)
{
    tx_in_progress  = false;
    if (ep0_tx_pending) {
        ep0_tx_pending  = false;
        ep0_start_tx();
        return;
    }
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
    if (ep3_tx_pending) {
        ep3_tx_pending  = false;
        ep35_start_tx(&ep3);
        return;
    }
# endif
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
    if (ep5_tx_pending) {
        ep5_tx_pending  = false;
        ep35_start_tx(&ep5);
        return;
    }
# endif
}

# define TX_TRY_LOCK(_x_)   tx_try_lock(_x_)
# define TX_UNLOCK()        tx_unlock()

#else

# define TX_TRY_LOCK(_x_)   1
# define TX_UNLOCK()        CYG_EMPTY_STATEMENT

#endif


// ----------------------------------------------------------------------------
// Endpoint 0
//
// As usual, control messages are more complicated than the rest.
//
// 1) during initialization a receive is initiated into the common
//    eight-byte buffer, used for the standard header of the control
//    packet. Until that header has been received and analysed,
//    there is no way of knowing whether or not the host will be
//    sending any more data.
//
// 2) the control packet may indicate that the host will be sending
//    more data. A higher-level handler for the control message should
//    have provided a suitable buffer, so a receive can be started
//    into that buffer. A flag indicates whether we are currently
//    receiving a new control packet or additional data.
//
// 3) the host may decide to cancel that extra data and send a new
//    control message instead. There is a flag to indicate that
//    the transfer included a SETUP token.
//
// 4) transmits only happen when the control packet involves returning
//    data. Unfortunately there is a problem in that, with eCos, the
//    return data will generally not be in a single contiguous buffer.
//    Discontinuous data could be handled by having a separate buffer
//    descriptor for each bit of data, but it is not known in advance
//    how many buffer descriptors might be needed so allocating
//    those statically presents a problem as well. Instead a single
//    static buffer is used, and data from higher-level code is copied
//    there. This introduces a new problem: how big should that buffer
//    be? A configuration option is used for that.
//
// If endpoint 6 is in use as well then things get more complicated
// because a single receive pool will be shared between endpoints 0
// and 6, and when adding a buffer to a pool there is no way of
// specifying the endpoint. Hence it will be necessary to receive
// into a static buffer and then copy into either an endpoint 0 or
// and endpoint 6 buffer.

// Fill the transmit buffer by repeatedly invoking the refill function
// and copying into the ep0 tx buffer. The relevant fields in the
// ep0 structure are cleared immediately and the completion function
// is called, even though the data has not actually gone out. That avoids
// a possible race condition where the host sends a new control packet
// immediately, before the transmit-complete has been processed
// (unlikely in practice, not least because ep0_tx_dsr() will get called
// before ep0_rx_dsr()).
static int
ep0_fill_txbuffer(void)
{
    int filled  = 0;
    while (filled < CYGNUM_DEVS_USB_UPD985XX_EP0_TXBUFSIZE) {
        if (0 != ep0.common.buffer_size) {
            if ((filled + ep0.common.buffer_size) < CYGNUM_DEVS_USB_UPD985XX_EP0_TXBUFSIZE) {
                memcpy(&(uncached->ep0_tx_buffer[filled]), ep0.common.buffer, ep0.common.buffer_size);
                filled += ep0.common.buffer_size;
                ep0.common.buffer_size = 0;
            } else {
                break;
            }
        } else if ((void (*)(usbs_control_endpoint*))0 != ep0.common.fill_buffer_fn) {
            (*ep0.common.fill_buffer_fn)(&ep0.common);
        } else {
            break;
        }
    }
    CYG_ASSERT((0 == ep0.common.buffer_size) && ((void (*)(usbs_control_endpoint*))0 == ep0.common.fill_buffer_fn), \
               "Endpoint 0 transmit buffer overflow");

    if ((usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn) {
        (*ep0.common.complete_fn)(&ep0.common, 0);
    }
    ep0.common.buffer           = (unsigned char*) 0;
    ep0.common.buffer_size      = 0;
    ep0.common.fill_buffer_fn   = 0;
    ep0.common.complete_fn      = 0;
    
    return filled;
}

// Start a new receive operation on endpoint 0. This needs to happen
// from a number of places, including from initialization.
//
// IMHO the hardware is somewhat overengineered here. All that is
// needed is to receive a single eight-byte control packet, or a
// small amount of additional control data. That could be achieved
// by using a single buffer descriptor in the uncached structure,
// plus a suitably-sized static uncached ep0_rx_buffer.
//
// But no, buffer descriptors must be linked and new buffers must
// be added to the end. When a control packet arrives, the
// receive pool continues to point at the old buffer descriptor.
// So we need two buffer descriptors plus two links, switching
// between them as appropriate.
//
// It is not at all clear what would happen if another packet
// started to happen while things were being updated. There is
// also potential confusion between endpoint 0 and endpoint 6
// receives.

static void
ep0_start_rx(cyg_uint32 size)
{
    // The buffer descriptor to be added. This will be either
    // ep0_rxbufdescs[0] or ep0_rxbufdescs[2];
    RxBufferDescriptor* desc = &(uncached->ep0_rx_bufdescs[0]);
    
    CYG_ASSERTC(size > 0);

    // Block interrupts for the duration. This does not prevent
    // problems if the hardware sees another packet and starts
    // doing things, but should prevent some software race
    // conditions.
    cyg_drv_isr_lock();

    // We are about to start a new rx operation, so the
    // current indicator may get invalidated.
    ep0.rx_indicator_valid = false;

    // Start by looking at the current pool0 status. There are
    // three possibilities: during init or after reset, the pool
    // will be empty; otherwise the pool should point at either
    // rx_bufdescs[0] or rx_bufdescs[2], corresponding to the
    // last received packet.
    if (0 == (*USBS_RP0IR & USBS_RPxIR_RNOD_MASK)) {
        // Nothing currently in the pool. Use ep0_rx_bufdescs[0],
        // and no need to update a link.
    } else if (desc == *USBS_RP0AR) {
        // The pool already points at bufdescs[0], switch to bufdescs[2],
        // and link from bufdescs[1].
        desc    = &(uncached->ep0_rx_bufdescs[2]);
        uncached->ep0_rx_bufdescs[1].buffer     = (void*) desc;
    } else {
        // The pool should point at bufdescs[2], stick with bufdescs[0]
        CYG_ASSERT(&(uncached->ep0_rx_bufdescs[2]) == *USBS_RP0AR, "Endpoint 0 rx buffer confusion");
        uncached->ep0_rx_bufdescs[3].buffer     = (void*) desc;
    }

    // Now fill in the buffer directory being added
    desc[0].control     = RXBUFDESC_CTRL_LAST | RXBUFDESC_CTRL_BUFDESC_BUFDESC | size;
    desc[0].buffer      = (void*) uncached->ep0_rx_buffer;
    desc[1].control     = RXBUFDESC_CTRL_BUFDESC_LINK;
    desc[1].buffer      = 0;
    
    while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
        // Do nothing: this situation should be short-lived.
    }
    *USBS_CA    = IBUS_SWAPPTR(void, desc);                     FLUSH_IBUS();
    *USBS_CMR   = IBUS_SWAP32(USBS_CMR_COMMAND_ADD_POOL0 | 1);  FLUSH_IBUS();
    cyg_drv_isr_unlock();
}

// Ditto for transmits. The data is assumed to be in
// uncached->ep0_tx_buffer already. A size of 0 indicates
// a need to send a terminating packet explicitly.
static void
ep0_start_tx(void)
{
    if (!TX_TRY_LOCK(&ep0_tx_pending)) {
        return;
    }
    
    uncached->ep0_tx_bufdesc.buffer     = uncached->ep0_tx_buffer;
    uncached->ep0_tx_bufdesc.control    = TXBUFDESC_CTRL_LAST | TXBUFDESC_CTRL_BUFDESC_BUFDESC | ep0.tx_size;

    cyg_drv_isr_lock();
    while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
        // Do nothing: this situation should be short-lived.
    }
    *USBS_CA    = IBUS_SWAPPTR(void, &(uncached->ep0_tx_bufdesc));      FLUSH_IBUS();
    *USBS_CMR   = IBUS_SWAP32(USBS_CMR_COMMAND_TX_EP0 | ep0.tx_size);   FLUSH_IBUS();
    cyg_drv_isr_unlock();
}

// An endpoint 0 transmission has completed. Usually the only action
// that is needed is to drain the tx mailbox entry, otherwise it is
// possible that we could end up with ep0 transmits using up all
// available slots. The endpoint 0 hardware requires no further
// attention, and as far as higher-level code is concerned the
// transmission completed a long time ago when ep0_fill_txbuffer()
// called the completion function.
//
// There is one special case. If the host asked for e.g. a string
// descriptor and asked for 255 bytes, but the string was only
// e.g. 32 bytes, then there is a problem. With a default value
// for CYGNUM_DEVS_USB_UPD985XX_EP0_PKTSIZE, the data will be
// transferred as four 8-byte packets, but it is necessary to
// terminate the transfer with a 0-byte packet. Endpoint 0 always
// operates in NZLP mode so the hardware will never generate
// this last packet. Instead it is necessary to set up an
// additional transfer of zero bytes. That could be done at the
// same time as the main data transfer, but then it would be
// necessary to poll the hardware and wait until it has finished
// processing that initial transfer.
static void
ep0_tx_dsr(void)
{
    if (!ep0.tx_indicator_valid) {
        drain_tx_mailbox();
        if (!ep0.tx_indicator_valid) {
            // A transmit interrupt when there does not appear to be
            // any data?
            CYG_FAIL("EP0 tx DSR invoked when there is no valid tx indicator");
            return;
        }
    }
    // There is not actually anything worth looking at in the status.
    ep0.tx_indicator_valid      = false;

    if (ep0.tx_needs_zero_transfer) {
        ep0.tx_needs_zero_transfer = false;
        uncached->ep0_tx_bufdesc.buffer     = uncached->ep0_tx_buffer;
        uncached->ep0_tx_bufdesc.control    = TXBUFDESC_CTRL_LAST | TXBUFDESC_CTRL_BUFDESC_BUFDESC | 0;

        cyg_drv_isr_lock();
        while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
            // Do nothing: this situation should be short-lived.
        }
        *USBS_CA    = IBUS_SWAPPTR(void, &(uncached->ep0_tx_bufdesc));          FLUSH_IBUS();
        *USBS_CMR   = IBUS_SWAP32(USBS_CMR_COMMAND_TX_EP0 | 0);   FLUSH_IBUS();
        cyg_drv_isr_unlock();
        
    } else {
        TX_UNLOCK();
    }
}

// An endpoint 0 receive has completed. This could be a new control
// message. Or it could be the data for a previous control message. Or
// it could be a new control message when expecting the data from
// a previous one. The ep0.rx_expecting_data field indicates
// whether or not a new control message is expected.
//
// At times an interrupt triggers and there is an rx indication for a
// zero-byte transfer. Such a transfer may be followed immediately by
// a real transfer. It is not understood why the zero-byte transfer
// occurs. They are ignored by the drain_rx_mailbox() code, to make
// sure that there is at most one valid rx indicator at a time.
static void
ep0_rx_dsr(void)
{
    // Start by checking the rx indicator to make sure that a packet
    // really has been received.
    if (!ep0.rx_indicator_valid) {
        drain_rx_mailbox();
        if (!ep0.rx_indicator_valid) {
            // Do not assert, in case of a spurious interrupt for a
            // zero-byte transfer.
            return;
        }
    }
    
    // We have a valid receive, with the data held in uncached->ep0_rx_buffer.
    // Are we expecting the remaining data of a control transfer?
    if (ep0.rx_expecting_data) {
        // Was this data interrupted by a new setup packet?
        if (0 != (ep0.rx_indicator.status & RXMBOX_STATUS_SETUP_SETUP)) {
            // NOTE: it is not clear from the documentation exactly what
            // happens here, e.g. is it guaranteed that the new control
            // packet appears at the start of the buffer rather than
            // after any data previously received? Given typical
            // USB host-side implementations this scenario is considered
            // sufficiently unlikely that no further investigation has
            // been carried out.
            
            // Inform higher-level code that the receive has been aborted.
            if ((usbs_control_return (*)(usbs_control_endpoint*, int)) 0 != ep0.common.complete_fn) {
                (*ep0.common.complete_fn)(&ep0.common, -EIO);
            }
            ep0.rx_expecting_data       = false;
            ep0.common.buffer           = (unsigned char*) 0;
            ep0.common.buffer_size      = 0;
            ep0.common.fill_buffer_fn   = 0;
            ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
            // Fall through the main control message handling code below.
        } else {
            // Data was expected and received. Transfer the data to the
            // user's buffer, and perform completion.
            usbs_control_return result;
            cyg_uint32          size     = ep0.rx_indicator.status & RXMBOX_STATUS_SIZE_MASK;
            
            CYG_ASSERT( (usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn, \
                        "A completion function should be provided for OUT control messages");
            CYG_ASSERT(size == ep0.common.buffer_size, "Inconsistency between buffer and transfer sizes");
            memcpy(ep0.common.buffer, uncached->ep0_rx_buffer, size);
            result = (*ep0.common.complete_fn)(&ep0.common, 0);
            ep0.common.buffer           = (unsigned char*) 0;
            ep0.common.buffer_size      = 0;
            ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
            ep0.rx_expecting_data       = false;

            // Start another receive for the next control message.
            // Note that there has been a window where there was no receive
            // in progress for endpoint 0, even though according to the
            // USB spec a device must always be able to accept new
            // control messages.
            ep0_start_rx(8);
            return;
        }
    }

    // When we get here we should have an eight-byte control message
    // in uncached->ep0_rx_buffer. This should get moved into
    // the ep0.common.control_buffer so that higher-level code sees
    // it in the appropriate location.
    CYG_ASSERT((ep0.rx_indicator.address == &(uncached->ep0_rx_bufdescs[0])) || \
               (ep0.rx_indicator.address == &(uncached->ep0_rx_bufdescs[2])),   \
               "Received ep0 data should involve the ep0 rx buffer descriptor");
    
    CYG_ASSERT(8 == (ep0.rx_indicator.status & RXMBOX_STATUS_SIZE_MASK), "Control messages should be 8 bytes");
    memcpy(ep0.common.control_buffer, uncached->ep0_rx_buffer, 8);

    // If we have received a control packet then any reset signals really
    // will have come from the host and must be processed normally.
    // Make sure that reset interrupts are no longer masked off.
    if (0 == (*USBS_IMR2 & IBUS_SWAP32(USBS_GSR2_URST))) {
        *USBS_IMR2              |= IBUS_SWAP32(USBS_GSR2_URST); FLUSH_IBUS();
        usbs_upd985xx_gsr2_mask |= USBS_GSR2_URST;
    }
    
    {
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

        DBG(("ep0, new control request: type %x, code %x\n", req->type, req->request));
        DBG(("     %s, length %d, value hi %x lo %x, index hi %x lo %x\n",
             (USB_DEVREQ_DIRECTION_OUT == direction) ? "out" : "in",
             length, req->value_hi, req->value_lo, req->index_hi, req->index_lo));

        if (USB_DEVREQ_TYPE_STANDARD == protocol) {
                
            // First see if the request can be handled entirely in
            // this module.
            if (USB_DEVREQ_SET_ADDRESS == req->request) {
                // The USB device address should be in value_lo.
                // No more data is expected.
                int old_state = ep0.common.state;
                int address = req->value_lo;
                if ((0 != length) || (address > 127)) {
                    result = USBS_CONTROL_RETURN_STALL;
                } else {
                    *USBS_GMR = (*USBS_GMR & ~(USBS_GMR_FA_MASK | USBS_GMR_VT)) | (address << USBS_GMR_FA_SHIFT); FLUSH_IBUS();
                    result = USBS_CONTROL_RETURN_HANDLED;
                }
                // Switch to addressed state, informing higher-level
                // code of this.
                if (USBS_STATE_ADDRESSED != (old_state & USBS_STATE_MASK)) {
                    ep0.common.state        = USBS_STATE_ADDRESSED;
                    if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                        (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                                      USBS_STATE_CHANGE_ADDRESSED, old_state);
                    }
                }
                // End of SET_ADDRESS handling
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
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
                        else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 3) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep0.common.control_buffer[0] = ep3.common.halted;
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
#endif                            
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
                        else if (((USB_DEVREQ_INDEX_DIRECTION_OUT | 4) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep0.common.control_buffer[0] = ep4.common.halted;
                            result = USBS_CONTROL_RETURN_HANDLED;
                                
                        }
#endif                            
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
                        else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 5) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep0.common.control_buffer[0] = ep5.common.halted;
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
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
                        else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 3) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep3_set_halted(&ep3.common, false);
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
                        else if (((USB_DEVREQ_INDEX_DIRECTION_OUT | 4) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep4_set_halted(&ep4.common, false);
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
                        else if (((USB_DEVREQ_INDEX_DIRECTION_IN | 5) == endpoint) &&
                                 (USBS_STATE_CONFIGURED == (ep0.common.state & USBS_STATE_MASK))) {
                            ep5_set_halted(&ep5.common, false);
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
                    // which is not supported by this driver.
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
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
                        else if ((USB_DEVREQ_INDEX_DIRECTION_IN | 3) == endpoint) {
                            ep3_set_halted(&ep3.common, true);
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
#endif                            
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
                        else if ((USB_DEVREQ_INDEX_DIRECTION_OUT | 4) == endpoint) {
                            ep4_set_halted(&ep4.common, true);
                            result = USBS_CONTROL_RETURN_HANDLED;
                        }
#endif                            
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
                        else if ((USB_DEVREQ_INDEX_DIRECTION_IN | 5) == endpoint) {
                            ep5_set_halted(&ep5.common, true);
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

            // If the result has still not been handled, leave it to
            // the default implementation in the USB slave common place.
            if (USBS_CONTROL_RETURN_UNKNOWN == result) {
                result = usbs_handle_standard_control(&ep0.common);
            }
        } else {
            // The other three types of control message can be
            // handled by similar code.
            usbs_control_return (*callback_fn)(usbs_control_endpoint*, void*);
            void* callback_arg;
                
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

        if (USBS_CONTROL_RETURN_HANDLED != result) {
            // This control request cannot be handled. Generate a stall.
            // These stalls will be cleared automaticaly by the next
            // setup packet.
            *EP0_CR     |= (EP0_CR_ISS | EP0_CR_OSS); FLUSH_IBUS();
            // Start a receive for the next control message
            ep0_start_rx(8);
        } else {
            // The control request has been handled. Is there any more
            // data to be transferred?
            if (0 == length) {
                // Definitely start a receive for another control message
                ep0_start_rx(8);
                
                // This operation is complete so we need to ack. It
                // appears that the way to achieve this is to send a
                // zero-byte packet.
                ep0.tx_size = 0;
                ep0_start_tx();
                
            } else {
                // Time to check the direction.

                if (USB_DEVREQ_DIRECTION_OUT == direction) {
                    // The host expects to send more data. Higher-level code
                    // should have provided an appropriate buffer.
                    CYG_ASSERT( (unsigned char*) 0 != ep0.common.buffer, "A receive buffer should have been provided");
                    CYG_ASSERT( (usbs_control_return (*)(usbs_control_endpoint*, int))0 != ep0.common.complete_fn, \
                                "A completion function should be provided for OUT control messages");
                    CYG_ASSERT(length <= CYGNUM_DEVS_USB_UPD985XX_EP0_RXBUFSIZE, "Insufficient buffer space configured");

                    ep0.rx_expecting_data = true;
                    ep0_start_rx(length);
                } else {
                    // The host expects to be able to read some data.
                    // This needs to go into a single contiguous
                    // buffer, and then the transfer can be started.
                    // Care has to be taken with various boundary conditions.
                    int actual_length = ep0_fill_txbuffer();
                    if (actual_length > length) {
                        actual_length = length;
                    } 
                    if ((length != actual_length) && (0 == (actual_length % CYGNUM_DEVS_USB_UPD985XX_EP0_PKTSIZE))) {
                        ep0.tx_needs_zero_transfer = true;
                    } else {
                        ep0.tx_needs_zero_transfer = false;
                    }
                    ep0.tx_size = actual_length;
                    ep0_start_tx();

                    // And make sure that there is another receive in progress
                    // for the next setup packet.
                    ep0_start_rx(8);
                }
            }
        }   // Control message handled
    }
}

// Endpoint 0 initialization also takes care of initializing generic bits
// of the USB controller, for example letting through resume and suspend
// interrupts and setting up the mailboxes. Also, it is necessary to
// start a receive operation so that the first control message can
// be processed. This code gets called during device driver initialization
// and after a reset from the host.
static void
ep0_init(void)
{
    // Reset the various fields in the ep0 structure.
    ep0.common.buffer           = (unsigned char*) 0;
    ep0.common.buffer_size      = 0;
    ep0.common.fill_buffer_fn   = (void (*)(usbs_control_endpoint*)) 0;
    ep0.common.fill_data        = (void*) 0;
    ep0.common.fill_index       = 0;
    ep0.common.complete_fn      = (usbs_control_return (*)(usbs_control_endpoint*, int)) 0;
    ep0.rx_expecting_data       = false;
    ep0.tx_indicator_valid      = false;
    ep0.rx_indicator_valid      = false;
    ep0.tx_needs_zero_transfer  = false;
    
#ifdef CYGIMP_DEVS_USB_UPD985XX_SERIALIZE_TRANSMITS
    tx_in_progress              = false;
    ep0_tx_pending              = false;
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
    ep3_tx_pending              = false;
# endif
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
    ep5_tx_pending              = false;
# endif
#endif    
    
    // The general mode register. We do not have an address yet. The
    // SOFINTVL field needs to be set to its default value. The other
    // bits should be zero for now.
    *USBS_GMR                    = USBS_GMR_SOFINTVL_DEFAULT_VALUE; FLUSH_IBUS();
    
    // The version register and the status registers are read-only.

    // Interrupt masks. Endpoint 0 transmits and receives both have
    // to be detected, as do the control operations. There should
    // be no need to worry about full mailboxes or empty receive
    // pools. DMA errors might be of interest, but it is not clear
    // what to do about them since there does not appear to be
    // a way of figuring out which transfer is affected. Frame number
    // and addressing problems are ignored, there is nothing obvious
    // that can be done about these. The other endpoints have their
    // own initialization routines.
    //
    // Care has to be taken with reset interrupts. With some hardware
    // the usb lines may be left floating during initialization, so
    // the chip believes it sees continuous reset interrupts. There
    // also appear to be problems if the host does generate a real
    // reset signal, with interrupt storms lasting 10 or more
    // milliseconds and preventing any other activity from taking
    // place. What is done here is that reset interrupts are enabled
    // if in the initial POWERED state. When a reset is detected,
    // either a spurious one or a real reset from the host,
    // handle_reset() will move the target to DEFAULT state, call
    // ep0_init() again, and reset interrupts will be masked out.
    // When a real control request is received from the host we
    // know we have a good connection and the reset interrupt will
    // be unmasked in ep0_rx_dsr(), so further resets from the
    // host will be processed correctly. If the target is disconnected
    // then we may again get a spurious reset interrupt, so we end
    // up back in DEFAULT state and the reset interrupt would be
    // masked again.
    *USBS_IMR1                  = IBUS_SWAP32(USBS_GSR1_GSR2 | USBS_GSR1_EP0TF | USBS_GSR1_EP0RF);  FLUSH_IBUS();
    usbs_upd985xx_gsr1_mask     = (USBS_GSR1_EP0TF | USBS_GSR1_EP0RF);
    if (USBS_STATE_DEFAULT == (ep0.common.state & USBS_STATE_MASK)) {
        *USBS_IMR2              = IBUS_SWAP32(USBS_GSR2_URSM | USBS_GSR2_USPD); FLUSH_IBUS();
        usbs_upd985xx_gsr2_mask = (USBS_GSR2_URSM | USBS_GSR2_USPD);
    } else {
        *USBS_IMR2              = IBUS_SWAP32(USBS_GSR2_URSM | USBS_GSR2_URST | USBS_GSR2_USPD); FLUSH_IBUS();
        usbs_upd985xx_gsr2_mask = (USBS_GSR2_URSM | USBS_GSR2_URST | USBS_GSR2_USPD);
    }
    
    // Writing to the command register is a bad idea, because even
    // writing 0 constitutes a command. Similarly there is no point
    // in writing to the command address register.

    // The endpoint status register is read-only.

    // Set the rx pool information registers to disable alerts.
    *USBS_RP0IR = IBUS_SWAP32(USBS_RPxIR_AL_NONE); FLUSH_IBUS();
    *USBS_RP1IR = IBUS_SWAP32(USBS_RPxIR_AL_NONE); FLUSH_IBUS();
    *USBS_RP2IR = IBUS_SWAP32(USBS_RPxIR_AL_NONE); FLUSH_IBUS();

    // The pool address registers are read-only. The documentation
    // that describes initialization says that these registers need to
    // be filled in, but that seems wrong: providing receive buffers
    // involves the command register. Presumably on early revisions it
    // was necessary to fill in the address register.

    // Sort out the mailboxes.
    *USBS_TMSA  = IBUS_SWAPPTR(TxMailbox, &(uncached->tx_mboxes[0]));               FLUSH_IBUS();
    *USBS_TMBA  = IBUS_SWAPPTR(TxMailbox, &(uncached->tx_mboxes[TXMBOX_COUNT]));    FLUSH_IBUS();
    *USBS_RMSA  = IBUS_SWAPPTR(RxMailbox, &(uncached->rx_mboxes[0]));               FLUSH_IBUS();
    *USBS_RMBA  = IBUS_SWAPPTR(RxMailbox, &(uncached->rx_mboxes[RXMBOX_COUNT]));    FLUSH_IBUS();
    // It is not clear whether these registers actually need to be initialized.
    // The documentation suggests that they do, unlike TMWA and RMWA which
    // are taken care of by the hardware.
#if 0    
    *USBS_TMRA  = IBUS_SWAPPTR(TxMailbox, &(uncached->tx_mboxes[0]));               FLUSH_IBUS();
    *USBS_RMRA  = IBUS_SWAPPTR(RxMailbox, &(uncached->rx_mboxes[0]));               FLUSH_IBUS();
#endif    

    // Start a receive operation for a control message.
    ep0_start_rx(8);
    
    // The endpoint 0 control register. The control packet size is
    // configurable, with a default value of 8. Setting the
    // enabled bit here affects the state as seen by the host.
    *EP0_CR                     = IBUS_SWAP32(EP0_CR_EP0EN | CYGNUM_DEVS_USB_UPD985XX_EP0_PKTSIZE); FLUSH_IBUS();
    
    // The other endpoint registers will be initialized by the appropriate
    // _init() functions. Note that those other _init() functions should
    // probably be called before the ep0-enabled bit is set.
}

// ----------------------------------------------------------------------------
// Endpoint 1 - isochronous transmits.
#if 0
// A real implementation
#else
static inline void
ep1_init(void)
{
    *EP1_CR     = IBUS_SWAP32(0);       // Clear EP1EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
#endif

// ----------------------------------------------------------------------------
// Endpoint 2 - isochronous receives
#if 0
// A real implementation
#else
static inline void
ep2_init(void)
{
    *EP2_CR     = IBUS_SWAP32(0);       // Clear EP2EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
#endif

// ----------------------------------------------------------------------------
// Generic transmit support. This is intended for use with both endpoints
// 3 and 5. For now the endpoint 0 code is too different.

#if defined(CYGPKG_DEVS_USB_UPD985XX_EP3) || defined(CYGPKG_DEVS_USB_UPD985XX_EP5)

// A utility routine for completing a transfer. This takes care of the
// callback as well as resetting the buffer.
static void
ep35_tx_complete(ep35_impl* ep, int result)
{
    void (*complete_fn)(void*, int)  = ep->common.complete_fn;
    void* complete_data = ep->common.complete_data;
    
    ep->common.buffer           = (unsigned char*) 0;
    ep->common.buffer_size      = 0;
    ep->common.complete_fn      = (void (*)(void*, int)) 0;
    ep->common.complete_data    = (void*) 0;

    if ((void (*)(void*, int))0 != complete_fn) {
        (*complete_fn)(complete_data, result);
    }
}

static void
ep35_start_tx(ep35_impl* ep)
{
    // Is this endpoint currently stalled? If so then a size of 0 can
    // be used to block until the stall condition is clear, anything
    // else should result in an immediate callback.
    if (ep->common.halted) {
        if (0 != ep->common.buffer_size) {
            ep35_tx_complete(ep, -EAGAIN);
        }
    } else if (0 == ep->common.buffer_size) {
        // A check to see if the endpoint is halted. It isn't.
        ep35_tx_complete(ep, 0);
    } else {
        cyg_uint32      send_command;
#if 0        
        diag_printf("Tx: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[0], ep->common.buffer[1], ep->common.buffer[2], ep->common.buffer[3],
                    ep->common.buffer[4], ep->common.buffer[5], ep->common.buffer[6], ep->common.buffer[7]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[8], ep->common.buffer[9], ep->common.buffer[10], ep->common.buffer[11],
                    ep->common.buffer[12], ep->common.buffer[13], ep->common.buffer[14], ep->common.buffer[15]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[16], ep->common.buffer[17], ep->common.buffer[18], ep->common.buffer[19],
                    ep->common.buffer[20], ep->common.buffer[21], ep->common.buffer[22], ep->common.buffer[23]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[24], ep->common.buffer[25], ep->common.buffer[26], ep->common.buffer[27],
                    ep->common.buffer[28], ep->common.buffer[29], ep->common.buffer[30], ep->common.buffer[31]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[32], ep->common.buffer[33], ep->common.buffer[34], ep->common.buffer[35],
                    ep->common.buffer[36], ep->common.buffer[37], ep->common.buffer[38], ep->common.buffer[39]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep->common.buffer[40], ep->common.buffer[41], ep->common.buffer[42], ep->common.buffer[43],
                    ep->common.buffer[44], ep->common.buffer[45], ep->common.buffer[46], ep->common.buffer[47]);
#endif

        // Update the static buffer descriptor. 
        ep->tx_bufdesc->buffer  = MIPS_TO_UNCACHED(ep->common.buffer);
        ep->tx_bufdesc->control = ep->common.buffer_size | TXBUFDESC_CTRL_LAST | TXBUFDESC_CTRL_BUFDESC_BUFDESC;

        // Make sure that the entire transmit buffer is flushed to
        // memory.
        HAL_DCACHE_STORE(ep->common.buffer, ep->common.buffer_size);
        
        // Issue the send command. It is known that no transmits are
        // in progress for this endpoint so the upper bound of 2
        // pending transmits can be ignored. The send command involves
        // writing to two registers in succession, so interrupts had
        // better be disabled while doing this.
        send_command = ep->send_command | ep->common.buffer_size;
        cyg_drv_isr_lock();
        while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
            // Do nothing: this situation should be short-lived.
        }
        *USBS_CA        = IBUS_SWAPPTR(void, (void*)ep->tx_bufdesc);  FLUSH_IBUS();
        *USBS_CMR       = IBUS_SWAP32(send_command); FLUSH_IBUS();
        cyg_drv_isr_unlock();
    }
}

// The stalled state is controlled by a single bit in the appropriate
// control register. However there is a problem in that it is not
// possible to abort a transmission that is already going out.
// Furthermore there is no way of detecting whether or not any
// packets have already gone out for this transfer: setting the halt
// bit before any data has gone out is reasonably ok, doing so
// in the middle of a transfer could be confusing.
//
// The approach taken here is to check whether or not there is a
// current tx buffer. If not then the stall bit can be set immediately.
// Otherwise the halted flag is set here, and it is left to the dsr
// to set the stall bit when the transfer completes. This may not
// be totally standards-compliant, but is probably the best solution
// for now.
static void
ep35_set_halted(ep35_impl* ep, cyg_bool new_value)
{
    if (ep->common.halted == new_value) {
        return;
    }

    // Avoid race conditions with the DSR updating the buffer fields.
    cyg_drv_dsr_lock();
    
    if (new_value ){
        // Set the halted flag to prevent further transmits, and if
        // there is no transmission currently in progress then set
        // the stalled bit immediately.
        ep->common.halted       = true;
        if ((void*)0 == ep->common.buffer) {
            *(ep->cr) |= IBUS_SWAP32(EPtx_CR_SSx); FLUSH_IBUS();
        }
    } else {
        // Update the hardware (that may be a no-op if the stalled bit
        // never got set by the DSR), and clear the halted flag.
        *(ep->cr) &= IBUS_SWAP32(~EPtx_CR_SSx); FLUSH_IBUS();
        ep->common.halted = false;

        // If there is a pending request to wait until the endpoint stall
        // condition is clear, inform higher-level code. This test may
        // give false positives but those would be harmless.
        if (0 == ep->common.buffer_size) {
            ep35_tx_complete(ep, 0);
        }
    }

    cyg_drv_dsr_unlock();
}

// An interrupt has occured related to endpoint 3 or 5 - i.e. the EP3TF
// or EP5TF interrupts, nothing else seems especially relevant. It is
// necessary to extract the appropriate send indicator from the tx
// mailbox to determine whether or not the transmission was successful
// and report status to higher-level code.
static void
ep35_dsr(ep35_impl* ep)
{
    TxMailbox   mbox;

    // Extract the transmit indicator if that has not happened
    // already courtesy of another DSR.
    if (!ep->tx_indicator_valid) {
        drain_tx_mailbox();
        if (!ep->tx_indicator_valid) {
            // A transmit interrupt when there does not appear to be
            // any data?
            CYG_FAIL("ep35_dsr invoked when there is no valid tx indicator");
            return;
        }
    }
    mbox                   = ep->tx_indicator;
    ep->tx_indicator_valid = false;
    
#ifdef CYGIMP_DEVS_USB_UPD985XX_EP5_BULK
    // If emulating bulk transfers over the interrupt endpoint, and
    // the transfer is an exact multiple of 64 bytes, then an extra
    // zero-byte terminating packet needs to be sent. Care has to be
    // taken to do this only once.
    if ( (ep == &ep5) && (0 == (ep5.common.buffer_size % 64)) ) {
        static cyg_bool sending_zero = false;
        if (!sending_zero) {
            sending_zero = true;
            uncached->ep5_tx_bufdesc.buffer     = uncached->ep0_tx_buffer;
            uncached->ep5_tx_bufdesc.control    = 0 | TXBUFDESC_CTRL_LAST | TXBUFDESC_CTRL_BUFDESC_BUFDESC;
            cyg_drv_isr_lock();
            while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
                // Do nothing: this situation should be short-lived.
            }
            *USBS_CA        = IBUS_SWAPPTR(void, (void*) &(uncached->ep5_tx_bufdesc));  FLUSH_IBUS();
            *USBS_CMR       = IBUS_SWAP32(USBS_CMR_COMMAND_TX_EP5 | 0); FLUSH_IBUS();
            cyg_drv_isr_unlock();
            
            // Do not complete the transfer. Instead completion has to
            // wait for another interrupt.
            return;
        } else {
            // This interrupt was for the zero-byte packet, so drop through
            sending_zero = false;
        }
    }
#endif    
    
    // If the endpoint should be halted but there was a transmit
    // in progress, update the hardware now.
    if (ep->common.halted) {
        *(ep->cr) |= IBUS_SWAP32(EPtx_CR_SSx); FLUSH_IBUS();
    }

    // Allow any blocked transmits to proceed.
    TX_UNLOCK();
    
    if (0 != (mbox.status & TXMBOX_STATUS_IBUS_ERROR)) {
        // This appears to be the only type of error that can be
        // detected. Possibly the transmit should be retried here
        // rather than reported.
        ep35_tx_complete(ep, -EPIPE);
    } else {
        ep35_tx_complete(ep, ep->common.buffer_size);
    }
}
#endif  // Endpoints 3 or 5

// ----------------------------------------------------------------------------
// Endpoint 3 - bulk transmits.

# ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
static void
ep3_start_tx(usbs_tx_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep3.common, "USB data transfer involves the wrong endpoint");
    CYG_ASSERT( ep3.common.buffer_size < (64 * 1024), "Specified transfer size too large for current implementation");
    if (TX_TRY_LOCK(&ep3_tx_pending)) {
        ep35_start_tx(&ep3);
    }
}

static void
ep3_set_halted(usbs_tx_endpoint* endpoint, cyg_bool new_value)
{
    CYG_ASSERT(endpoint = &ep3.common, "USB set-stall operation involves the wrong endpoint");
    ep35_set_halted(&ep3, new_value);
}

// Initialization. This gets called during the device driver
// initialization and after a reset. The main job is to initialize the
// EP3 control register, but the relevant bits of the interrupt mask
// are set here as well. The tx mailboxes are shared with other
// endpoints, so that is handled by ep0_init(). Any traffic that
// happened before the reset needs to be cleaned up.
static void
ep3_init(void)
{
    // Assume 64 byte packets, terminate transfers with a zero-byte packet
    // if necessary since this endpoint is used for bulk transfers.
    *EP3_CR                      = IBUS_SWAP32(EP3_CR_EP3EN | EP3_CR_TM3_SZLP | 64); FLUSH_IBUS();
    *USBS_IMR1                  |= IBUS_SWAP32(USBS_GSR1_EP3TF); FLUSH_IBUS();
    usbs_upd985xx_gsr1_mask     |= IBUS_SWAP32(USBS_GSR1_EP3TF);
    ep3.common.halted            = false;
    ep3.tx_indicator_valid       = false;
    ep3.tx_bufdesc               = &(uncached->ep3_tx_bufdesc);
    ep35_tx_complete(&ep3, -EPIPE);
}
#else
static inline void
ep3_init(void)
{
    *EP3_CR     = 0;    // Clear EP3EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
# endif // Endpoint 3 configured in

// ----------------------------------------------------------------------------
// Repeat for endpoint 5
# ifdef CYGPKG_DEVS_USB_UPD985XX_EP5

static void
ep5_start_tx(usbs_tx_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep5.common, "USB data transfer involves the wrong endpoint");
#ifdef CYGIMP_DEVS_USB_UPD985XX_EP5_BULK        
    CYG_ASSERT( ep5.common.buffer_size < (64 * 1024), "Specified transfer size too large for current implementation");
#else
    CYG_ASSERT( ep5.common.buffer_size <= 64, "Specified transfer size too large for current implementation");
#endif        
    if (TX_TRY_LOCK(&ep5_tx_pending)) {
        ep35_start_tx(&ep5);
    }
}

static void
ep5_set_halted(usbs_tx_endpoint* endpoint, cyg_bool new_value)
{
    CYG_ASSERT(endpoint = &ep5.common, "USB set-stall operation involves the wrong endpoint");
    ep35_set_halted(&ep5, new_value);
}

// Initialization. This gets called during the device driver
// initialization and after a reset. The main job is to initialize the
// EP5 control register, but the relevant bits of the interrupt mask
// are set here as well. The tx mailboxes are shared with other
// endpoints, so that is handled by ep0_init(). Any traffic that
// happened before the reset needs to be cleaned up.
static void
ep5_init(void)
{
    // Assume 64 byte packets, terminate transfers with a zero-byte packet
    // if necessary since this endpoint is used for bulk transfers.
    *EP5_CR                      = IBUS_SWAP32(EP5_CR_EP5EN | 64); FLUSH_IBUS();
    *USBS_IMR1                  |= IBUS_SWAP32(USBS_GSR1_EP5TF); FLUSH_IBUS();
    usbs_upd985xx_gsr1_mask     |= IBUS_SWAP32(USBS_GSR1_EP5TF);
    ep5.common.halted            = false;
    ep5.tx_indicator_valid       = false;
    ep5.tx_bufdesc               = &(uncached->ep5_tx_bufdesc);
    ep35_tx_complete(&ep5, -EPIPE);
}
#else
static inline void
ep5_init(void)
{
    *EP5_CR     = 0;    // Clear EP5EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
#endif // Endpoint 5 configured in


#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
// ----------------------------------------------------------------------------
// Endpoint 4 - bulk receives.
//
// Bulk receives are mostly straightforward, but the cache does involve
// a complication. The assumption is that the receive buffer will be in
// cached memory, and is unlikely to be cacheline-aligned. Clearly the bulk
// of the buffer has to be invalidated. However the receive buffer may share
// some cache lines with other data at the head and tail, and invalidating
// those would be wrong.
//
// The solution here is to split up a receive in to up to three areas,
// head, main, and tail, where the head and tail are statically
// allocated in uncached memory. Any one or two of these areas may be
// unused, depending on alignment and transfer size. The main area
// corresponds to the central section of the supplied receive buffer,
// will be cacheline-aligned, and invalidated at the start of a receive.
// Data will be copied from the head and tail areas into the receive
// buffer by the dsr on completion of the transfer.
//
// There are additional complications caused by the hardware's need for
// linked buffers.

static void
ep4_rx_complete(int result)
{
    void (*complete_fn)(void*, int)  = ep4.common.complete_fn;
    void* complete_data = ep4.common.complete_data;

#if 0
    *EP4_CR     = IBUS_SWAP32(EP4_CR_EP4EN | EP4_CR_RM4_ASSEMBLE | EP4_CR_NAK4 | 64);  FLUSH_IBUS();
#endif
    
#if 0    
    if (result > 0) {
        diag_printf("Rx: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[0], ep4.common.buffer[1], ep4.common.buffer[2], ep4.common.buffer[3],
                    ep4.common.buffer[4], ep4.common.buffer[5], ep4.common.buffer[6], ep4.common.buffer[7]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[8], ep4.common.buffer[9], ep4.common.buffer[10], ep4.common.buffer[11],
                    ep4.common.buffer[12], ep4.common.buffer[13], ep4.common.buffer[14], ep4.common.buffer[15]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[16], ep4.common.buffer[17], ep4.common.buffer[18], ep4.common.buffer[19],
                    ep4.common.buffer[20], ep4.common.buffer[21], ep4.common.buffer[22], ep4.common.buffer[23]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[24], ep4.common.buffer[25], ep4.common.buffer[26], ep4.common.buffer[27],
                    ep4.common.buffer[28], ep4.common.buffer[29], ep4.common.buffer[30], ep4.common.buffer[31]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[32], ep4.common.buffer[33], ep4.common.buffer[34], ep4.common.buffer[35],
                    ep4.common.buffer[36], ep4.common.buffer[37], ep4.common.buffer[38], ep4.common.buffer[39]);
        diag_printf("    %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    ep4.common.buffer[40], ep4.common.buffer[41], ep4.common.buffer[42], ep4.common.buffer[43],
                    ep4.common.buffer[44], ep4.common.buffer[45], ep4.common.buffer[46], ep4.common.buffer[47]);
    }
#endif    
    
    ep4.common.buffer           = (unsigned char*) 0;
    ep4.common.buffer_size      = 0;
    ep4.common.complete_fn      = (void (*)(void*, int)) 0;
    ep4.common.complete_data    = (void*) 0;

    if ((void (*)(void*, int))0 != complete_fn) {
        (*complete_fn)(complete_data, result);
    }
}

static void
ep4_start_rx(usbs_rx_endpoint* ep)
{
    CYG_ASSERT( ep == &ep4.common, "USB data transfer involves the wrong endpoint");

    // Is this endpoint currently stalled? If so then a size of 0 can
    // be used to block until the stall condition is clear, anything
    // else should result in an immediate callback.
    if (ep4.common.halted) {
        if (0 != ep4.common.buffer_size) {
            ep4_rx_complete(-EAGAIN);
        }
    } else if (0 == ep4.common.buffer_size) {
        // A check to see if the endpoint is halted. It isn't.
        ep4_rx_complete(0);
    } else {

        // Time to work out how much data should go into the uncached
        // head and tail buffers, how much can go directly into the
        // receive buffer, how much memory needs to be invalidated, and so on.
        cyg_uint32 buffer_arith;

        // Where to start filling in buffer descriptors.
        cyg_uint32 first_bufdesc;

        // And the current buffer descriptor.
        cyg_uint32 current_bufdesc;
        
        CYG_ASSERT( ep4.common.buffer_size < (64 * 1024), "Specified transfer size too large for current implementation");

        // If there has not been a receive operation, tail_index
        // will still be set to -1. Otherwise it will be somewhere
        // between 1 and 3, or between 5 and 7, depending on
        // whether the previous receive operation used the
        // first four buffer descriptors or the last four.
        if (ep4.tail_index < 4) {
            first_bufdesc = 4;
        } else {
            first_bufdesc = 0;
        }
        current_bufdesc = first_bufdesc;

        // Arithmetic, especially remainder operators, requires
        // integers rather than a pointer.
        buffer_arith = (cyg_uint32) ep4.common.buffer;

        // The size of the "head" area. This involves up to
        // (cacheline-1) bytes, so that the main receive buffer
        // is suitably aligned.
        ep4.head_size    = ((buffer_arith + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1)) - buffer_arith;
        if (ep4.head_size > ep4.common.buffer_size) {
            ep4.head_size = ep4.common.buffer_size;
        }
        if (0 < ep4.head_size) {
            // It is necessary to receive some data into the uncached head area.
            uncached->ep4_rx_bufdescs[current_bufdesc].buffer   = uncached->ep4_head;
            uncached->ep4_rx_bufdescs[current_bufdesc].control  = RXBUFDESC_CTRL_BUFDESC_BUFDESC | ep4.head_size;
            current_bufdesc++;
        }

        // Now for the size of the main area. This is the rest of the
        // transfer size, minus the tail area.
        ep4.direct_size      = ep4.common.buffer_size - ep4.head_size;
        ep4.direct_size     &= ~(HAL_DCACHE_LINE_SIZE - 1);
        if (ep4.direct_size > 0) {
            uncached->ep4_rx_bufdescs[current_bufdesc].buffer   = MIPS_TO_UNCACHED(ep4.common.buffer + ep4.head_size);
            uncached->ep4_rx_bufdescs[current_bufdesc].control  = RXBUFDESC_CTRL_BUFDESC_BUFDESC | ep4.direct_size;
            current_bufdesc++;
            HAL_DCACHE_INVALIDATE(ep4.common.buffer + ep4.head_size, ep4.direct_size);
        }

        // And the size of the tail. This is the transfer size minus what we have accumulated so far.
        ep4.tail_size = ep4.common.buffer_size - (ep4.head_size + ep4.direct_size);
        if (ep4.tail_size > 0) {
            uncached->ep4_rx_bufdescs[current_bufdesc].buffer   = uncached->ep4_tail;
            uncached->ep4_rx_bufdescs[current_bufdesc].control  = RXBUFDESC_CTRL_BUFDESC_BUFDESC | ep4.tail_size;
            current_bufdesc++;
        }

        // Or the LAST bit into the last of these buffer descriptors.
        uncached->ep4_rx_bufdescs[current_bufdesc - 1].control     |= RXBUFDESC_CTRL_LAST;

        // Turn the current one into a link descriptor.
        uncached->ep4_rx_bufdescs[current_bufdesc].control = RXBUFDESC_CTRL_BUFDESC_LINK;
        uncached->ep4_rx_bufdescs[current_bufdesc].buffer  = 0;

        // The buffer descriptors have now been sorted out. Time to
        // add this receive buffer to the pool. Atomicity becomes
        // important for some of these steps.
        cyg_drv_isr_lock();

        // Update the link pointer used for the last receive operation
        // to point at the new set of buffer descriptors.
        if (-1 != ep4.tail_index) {
            uncached->ep4_rx_bufdescs[ep4.tail_index].buffer = (void*) &(uncached->ep4_rx_bufdescs[first_bufdesc]);
        }

        // Keep track of the link pointer used for the last receive
        // operation.
        ep4.tail_index = current_bufdesc;

        while (0 != (*USBS_CMR & IBUS_SWAP32(USBS_CMR_BUSY))) {
            // Do nothing: this situation should be short-lived.
        }
        *USBS_CA    = IBUS_SWAPPTR(void, (void*)&(uncached->ep4_rx_bufdescs[first_bufdesc]));   FLUSH_IBUS();
        *USBS_CMR   = IBUS_SWAP32(USBS_CMR_COMMAND_ADD_POOL2 | 1);                              FLUSH_IBUS();
#if 0
        *EP4_CR     = IBUS_SWAP32(EP4_CR_EP4EN | EP4_CR_RM4_ASSEMBLE | 64);                     FLUSH_IBUS();
#endif
        cyg_drv_isr_unlock();
    }
}

// The stalled state is controlled by a single bit in the appropriate
// control register. When it comes to ongoing receives, the reasoning
// here is much the same as for ep3_set_halted(). Arguably this is not
// quite right. set_halted() is most likely to be called in response
// to a control request from the host, and the host is unlikely to
// transmit any data at the same time as making this request. Hence the
// host will see the wrong behaviour: it can still make one transfer
// after asking for the stalled bit to be set. Since there does not
// appear to be any way to cancel a supplied receive, this behaviour
// still seems the most sensible.
static void
ep4_set_halted(usbs_rx_endpoint* ep, cyg_bool new_state)
{
    CYG_ASSERT(ep == &ep4.common, "USB set-stall operation involves the wrong endpoint");
    
    if (ep4.common.halted == new_state) {
        return;
    }

    // Avoid race conditions with the DSR updating the buffer fields.
    cyg_drv_dsr_lock();
    
    if (new_state){
        // Set the halted flag to prevent further transmits, and if
        // there is no receive currently in progress then set
        // the stalled bit immediately.
        ep4.common.halted       = true;
        if ((void*)0 == ep4.common.buffer) {
            *EP4_CR |= IBUS_SWAP32(EP4_CR_SS4); FLUSH_IBUS();
        }
    } else {
        // Update the hardware (that may be a no-op if the stalled bit
        // never got set by the DSR), and clear the halted flag.
        *EP4_CR &= IBUS_SWAP32(~EP4_CR_SS4); FLUSH_IBUS();
        ep4.common.halted = false;

        // If there is a pending request to wait until the endpoint stall
        // condition is clear, inform higher-level code. This test may
        // give false positives but those would be harmless.
        if (0 == ep4.common.buffer_size) {
            ep4_rx_complete(0);
        }
    }

    cyg_drv_dsr_unlock();
}

// An interrupt has occured related to endpoint 4 - i.e. the EP4RF
// interrupt, nothing else seems especially relevant. The ISR will
// have set the NAK bit in the control register. It is necessary to
// extract the appropriate receive indicator from the rx mailbox to
// determine whether or not the transmission was successful and how
// much data was actually received, clear the interrupt bit, and
// report status to higher-level code.
static void
ep4_dsr(void)
{
    // Extract the transmit indicator if that has not happened
    // already courtesy of another DSR.
    if (!ep4.rx_indicator_valid) {
        drain_rx_mailbox();
        if (!ep4.rx_indicator_valid) {
            // A receive interrupt when there does not appear to be
            // any data? It appears that this can happen when there
            // are error conditions.
#if 1            
            CYG_FAIL("EP4_DSR invoked when there is no valid rx indicator");
#endif            
            return;
        }
    }
    ep4.rx_indicator_valid = false;

    // If the endpoint should be halted but there was a transmit
    // in progress, update the hardware now.
    if (ep4.common.halted) {
        *EP4_CR |= IBUS_SWAP32(EP4_CR_SS4); FLUSH_IBUS();
    }
    if (0 != (ep4.rx_indicator.status & (RXMBOX_STATUS_IBUS_ERROR | 0))) {
        // This appears to be the only type of error that can be
        // detected. Everything else gets retried by the hardware,
        // except when using the isochronous endpoint.
        ep4_rx_complete(-EPIPE);
    } else {
        cyg_uint32 actual_size = ep4.rx_indicator.status & RXMBOX_STATUS_SIZE_MASK;
        
        // Either the transfer size must match the requested size, or the
        // supplied buffer should have been aligned to cacheline boundaries.
        // Anything else risks leaving the receive pool in a confused
        // state and there is no way of cleaning things up.
        CYG_ASSERT((actual_size == ep4.common.buffer_size) || ((0 == ep4.head_size) && (0 == ep4.tail_size)), \
                   "Buffers should be cacheline aligned if the protocol involves partial transfers");
        
        // If there was some data in the head, move it from uncached
        // to cached. Ditto for tail. Note that these copies may be
        // for data that has not actually been transferred if the
        // actual transfer is less than expected, but overwriting bits
        // of the receive buffer in such circumstances should be
        // harmless.
        if (ep4.head_size > 0) {
            memcpy(ep4.common.buffer, uncached->ep4_head, ep4.head_size);
        }
        if (ep4.tail_size > 0) {
            memcpy(ep4.common.buffer + ep4.head_size + ep4.direct_size, uncached->ep4_tail, ep4.tail_size);
        }
        ep4_rx_complete(actual_size);
    }
}

// Initialization. This gets called during the device driver
// initialization and after a reset. The main job is to initialize the
// EP4 control register, but the relevant bits of the interrupt mask
// are set here as well. The rx mailboxes are shared with other
// endpoints, so that is handled by ep0_init(). Any traffic that
// happened before the reset needs to be cleaned up.
static void
ep4_init(void)
{
    // Assume 64 byte packets, and use assemble mode so that we get a
    // single rx indication per transfer. In practice the buffer
    // directory will only ever contain one entry so there should be
    // no discernible difference between normal, assemble, or separate
    // mode.
#if 0
    *EP4_CR                      = IBUS_SWAP32(EP4_CR_EP4EN | EP4_CR_RM4_ASSEMBLE | EP4_CR_NAK4 | 64); FLUSH_IBUS();
#else
    *EP4_CR                      = IBUS_SWAP32(EP4_CR_EP4EN | EP4_CR_RM4_ASSEMBLE | 64); FLUSH_IBUS();
#endif
    
    *USBS_IMR1                  |= IBUS_SWAP32(USBS_GSR1_EP4RF); FLUSH_IBUS();
    usbs_upd985xx_gsr1_mask     |= USBS_GSR1_EP4RF;
    ep4.common.halted            = false;
    ep4.rx_indicator_valid       = false;
    ep4.tail_index               = -1;
    ep4_rx_complete(-EPIPE);
}
#else
static inline void
ep4_init(void)
{
    *EP4_CR     = 0;    // Clear EP4EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
#endif  // Endpoint 4 configured in

// ----------------------------------------------------------------------------
// Endpoint 6 - interrupt receives
#if 0
// A real implementation
#else
static inline void
ep6_init(void)
{
    *EP6_CR     = 0;    // Clear EP6EN bit, thus disabling the endpoint
    FLUSH_IBUS();
}
#endif

// ----------------------------------------------------------------------------
// Make sure the hardware is in a known state by completely resetting
// the USB controller. This gets called during device driver
// initialization, and again whenever the host issues a reset signal.
// The previous state is unknown. Even during eCos initialization
// RedBoot may have involved USB I/O, or some POST code may have
// performed loopback tests. The various endpoint init routines will
// also perform software resets as appropriate.

static void
usbs_upd985xx_handle_reset(void)
{
    // Reset the USB hardware. This involves poking the warm reset
    // register and then polling the matching status register. It is
    // assumed that this poll will take a short time, and in practice
    // the loop appears to terminate immediately.
    *S_WRCR = S_WRCR_USBWR; FLUSH_IBUS();
    while (0 == (*S_WRSR & S_WRCR_USBWR)) {
        // Do nothing.
    }
    
    // Get all the endpoints into a known state - for disabled
    // endpoints these init calls are inlined and just disable the
    // relevant hardware.
    ep0_init();
    ep1_init();
    ep2_init();
    ep3_init();
    ep4_init();
    ep5_init();
    ep6_init();
}

// ----------------------------------------------------------------------------
// Start(). This is typically called by the application itself once
// everything else has been initialized, i.e. when the host should be
// able to start talking to this device. There is no actual bit in the
// chip itself to switch the USB pins from tri-state, so instead the
// platform HAL has to supply appropriate functionality. In the absence
// of such functionality things will only work if you start up eCos
// with the USB cable disconnected, then plug in the cable once
// start() has been called.
//
// The device driver initialization will have already set up the
// hardware, and the first action from the host should be a reset
// signal which will cause a re-initialization. There is no need
// to do anything else here.

static void
usbs_upd985xx_ep0_start(usbs_control_endpoint* endpoint)
{
    CYG_ASSERT( endpoint == &ep0.common, "USB startup involves the wrong endpoint");

    // If there is additional platform-specific initialization to
    // perform, do it now. This macro can come from the platform HAL,
    // but may not be available on all platforms.
#ifdef UPD985XX_USB_PLATFORM_INIT
    UPD985XX_USB_PLATFORM_INIT();
#endif
}
 
// ----------------------------------------------------------------------------
// The main DSR
//
// This gets called by the interrupt system or by the polling code
// when one or more USB-related events have occurred. The ISR code
// will have updated globals usbs_upd985xx_gsr1 and usbs_upd98x0x_gsr2
// to indicate which events are pending. When running in interrupt
// mode it is possible that further interrupts will occur while the
// DSR is running, and hence that these globals will be updated
// while the DSR is running. This is handled by a loop. A side effect
// is that the DSR may get called when all the work has already
// been done by a previous DSR, but that is harmless.

static void
usbs_upd985xx_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_ASSERT(CYGNUM_HAL_INTERRUPT_USB == vector, "USB DSR should only be invoked for USB interrupts" );
    CYG_ASSERT(0 == data, "The USB DSR needs no global data pointer");

    while ((0 != usbs_upd985xx_gsr1) || (0 != usbs_upd985xx_gsr2)) {
        // Only update the globals in one place since it involves
        // the overhead of two function calls.
        cyg_uint32 gsr1, gsr2;
        cyg_drv_isr_lock();
        gsr1 = usbs_upd985xx_gsr1;
        gsr2 = usbs_upd985xx_gsr2;
        usbs_upd985xx_gsr1 = 0;
        usbs_upd985xx_gsr2 = 0;
        cyg_drv_isr_unlock();

        if (0 != gsr2) {
            // Treat reset specially. If there has been a reset then none of
            // the other bits are of any interest.
            if (0 != (USBS_GSR2_URST & gsr2)) {
                int old_state       = ep0.common.state;
                // Update the state. ep0_init() detects this state change and
                // updates imr2 appropriate, preventing a continuous storm
                // of reset interrupts.
                ep0.common.state = USBS_STATE_DEFAULT;
                usbs_upd985xx_handle_reset();
                // This state change must be reported to higher-level code
                if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                    (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                                  USBS_STATE_CHANGE_RESET, old_state);
                }
                break;
            }
            // There is possible confusion if both suspend and resume
            // bits are set. Was there a suspend, quickly followed by
            // a resume? Were we already suspended, then resumed, now
            // suspended again? For now this complication is ignored and
            // resume is given priority over suspend.
            if (0 != (USBS_GSR2_URSM & gsr2)) {
                int old_state       = ep0.common.state;
                if (0 != (old_state & USBS_STATE_SUSPENDED)) {
                    ep0.common.state   &= ~USBS_STATE_SUSPENDED;
                    if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                        (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                                      USBS_STATE_CHANGE_RESUMED, old_state);
                    }
                }
            } else if (0 != (USBS_GSR2_USPD & gsr2)) {
                int old_state       = ep0.common.state;
                if (0 == (old_state & USBS_STATE_SUSPENDED)) {
                    ep0.common.state   |= USBS_STATE_SUSPENDED;
                    if ((void (*)(usbs_control_endpoint*, void*, usbs_state_change, int))0 != ep0.common.state_change_fn) {
                        (*ep0.common.state_change_fn)(&ep0.common, ep0.common.state_change_data,
                                                      USBS_STATE_CHANGE_SUSPENDED, old_state);
                    }
                }
            } else {
                // Handle error conditions on the isochronous endpoints?
            }
        }
        if (0 != gsr1) {
            
            if (0 != (USBS_GSR1_EP0TF & gsr1)) {
                ep0_tx_dsr();
            }
            if (0 != (USBS_GSR1_EP0RF & gsr1)) {
                ep0_rx_dsr();
            }
#if 0
            // EP1FU?
            if (0 != (USBS_GSR1_EP1TF & gsr1)) {
                ep1_dsr();
            }
#endif
#if 0
            // EP2FO?
            if (0 != (USBS_GSR1_EP2RF & gsr1)) {
                ep1_dsr();
            }
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
            if (0 != (USBS_GSR1_EP3TF & gsr1)) {
                ep35_dsr(&ep3);
            }
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
            if (0 != (USBS_GSR1_EP4RF & gsr1)) {
                ep4_dsr();
            }
#endif
#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
            if (0 != (USBS_GSR1_EP5TF & gsr1)) {
                ep35_dsr(&ep5);
            }
#endif
#if 0            
            if (0 != (USBS_GSR1_EP6RF & gsr1)) {
                ep6_dsr();
            }
#endif
        }
    }   // while there are unprocessed interrupts
}
    
// ----------------------------------------------------------------------------
// Interrupt handling.
//
// There are two status registers to look at. These are read-once
// registers, i.e. reading the register causes all bits to be cleared,
// so the relevant state has to be preserved in volatile globals which
// can then be examined by the DSR. In theory the top bit of the first
// status register can be used to check whether or not there is
// anything of interest in the second one, but it seems quicker to
// just read both registers. After masking out interrupts that are of
// no interest, some global flags are updated. If this leaves a
// non-zero value then the DSR must be invoked.

static cyg_uint32
usbs_upd985xx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    CYG_ASSERT(CYGNUM_HAL_INTERRUPT_USB == vector, "USB ISR should only be invoked for USB interrupts");
    CYG_ASSERT(0 == data, "The UPD985xx ISR needs no global data pointer");

    usbs_upd985xx_gsr1 |= IBUS_SWAP32(*USBS_GSR1) & usbs_upd985xx_gsr1_mask;
    usbs_upd985xx_gsr2 |= IBUS_SWAP32(*USBS_GSR2) & usbs_upd985xx_gsr2_mask;

    cyg_drv_interrupt_acknowledge(vector);
    return ((0 == usbs_upd985xx_gsr1) && (0 == usbs_upd985xx_gsr2)) ?
        CYG_ISR_HANDLED : CYG_ISR_CALL_DSR;
}

// ----------------------------------------------------------------------------
// Polling support. It is not clear that this is going to work particularly
// well since according to the documentation the hardware does not generate
// NAKs automatically - instead the ISR has to set the appropriate bits
// sufficiently quickly to avoid confusing the host.
//
// Calling the isr directly avoids duplicating code, but means that
// cyg_drv_interrupt_acknowledge() will get called when not inside a
// real interrupt handler. This should be harmless.

static void
usbs_upd985xx_poll(usbs_control_endpoint* endpoint)
{
    CYG_ASSERT(endpoint == &ep0.common, "USB poll involves the wrong endpoint");
    if (CYG_ISR_CALL_DSR == usbs_upd985xx_isr(CYGNUM_HAL_INTERRUPT_USB, 0)) {
        usbs_upd985xx_dsr(CYGNUM_HAL_INTERRUPT_USB, 0, 0);
    }
}

// ----------------------------------------------------------------------------
// Initialization
//
// This routine gets called from a prioritized static constructor during
// eCos startup.
void
usbs_upd985xx_init(void)
{
    // Make sure the uncached data structure is accessed through
    // kseg1.
    uncached = (uncached_data*) MIPS_TO_UNCACHED(&cached_copy);

    // Perform a full hardware reset.
    usbs_upd985xx_handle_reset();

    // It is possible and desirable to install the interrupt handler
    // here, even though there will be no interrupts for a while yet.
    // FIXME: is 99 a sensible interrupt priority :-?
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_USB,
                             99,        // priority
                             0,         // data
                             &usbs_upd985xx_isr,
                             &usbs_upd985xx_dsr,
                             &usbs_upd985xx_intr_handle,
                             &usbs_upd985xx_intr_data);
    
    cyg_drv_interrupt_attach(usbs_upd985xx_intr_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_USB);
}

// ----------------------------------------------------------------------------
// Testing support.

usbs_testing_endpoint usbs_testing_endpoints[] = {
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_CONTROL, 
        endpoint_number     : 0,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,
        endpoint            : (void*) &ep0.common,
#ifdef CYGVAR_DEVS_USB_UPD985XX_EP0_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "0c",
#else        
        devtab_entry        : (const char*) 0,
#endif        
        min_size            : 1,            // zero-byte control transfers are meaningless
#if (CYGNUM_DEVS_USB_UPD985XX_EP0_RXBUFSIZE < CYGNUM_DEVS_USB_UPD985XX_EP0_TXBUFSIZE)
        max_size            : CYGNUM_DEVS_USB_UPD985XX_EP0_RXBUFSIZE,
#else
        max_size            : CYGNUM_DEVS_USB_UPD985XX_EP0_TXBUFSIZE,
#endif        
        max_in_padding      : 0,
        alignment           : 0
    },

#ifdef CYGPKG_DEVS_USB_UPD985XX_EP3
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_BULK,
        endpoint_number     : 3,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,
        endpoint            : (void*) &ep3.common,
# ifdef CYGVAR_DEVS_USB_UPD985XX_EP3_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "3w",
# else        
        devtab_entry        : (const char*) 0,
# endif        
        min_size            : 0,
        max_size            : 0x0FFFF,      // Driver limitation, only a single buffer descriptor is used
        max_in_padding      : 0,
        alignment           : HAL_DCACHE_LINE_SIZE
    },
#endif

#ifdef CYGPKG_DEVS_USB_UPD985XX_EP4
    {
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_BULK,
        endpoint_number     : 4,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_OUT,
        endpoint            : (void*) &ep4.common,
# ifdef CYGVAR_DEVS_USB_UPD985XX_EP4_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "4r",
# else        
        devtab_entry        : (const char*) 0,
# endif        
        min_size            : 1,
        max_size            : 0x0FFFF,      // Driver limitation
        max_in_padding      : 0,
        alignment           : HAL_DCACHE_LINE_SIZE
    },
#endif

#ifdef CYGPKG_DEVS_USB_UPD985XX_EP5
    {
# ifdef CYGIMP_DEVS_USB_UPD985XX_EP5_BULK        
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_BULK,
# else
        endpoint_type       : USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT,
# endif        
        endpoint_number     : 5,
        endpoint_direction  : USB_ENDPOINT_DESCRIPTOR_ENDPOINT_IN,
        endpoint            : (void*) &ep5.common,
# ifdef CYGVAR_DEVS_USB_UPD985XX_EP5_DEVTAB_ENTRY
        devtab_entry        : CYGDAT_DEVS_USB_UPD985XX_DEVTAB_BASENAME "5w",
# else        
        devtab_entry        : (const char*) 0,
# endif        
        min_size            : 1,
        max_size            : 0x0FFFF,      // Driver limitation, only a single buffer descriptor is used
        max_in_padding      : 0,
        alignment           : HAL_DCACHE_LINE_SIZE
    },
#endif
    
    USBS_TESTING_ENDPOINTS_TERMINATOR
};
