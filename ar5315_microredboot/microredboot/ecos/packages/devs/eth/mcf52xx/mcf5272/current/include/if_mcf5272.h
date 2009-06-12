#ifndef _IF_MCF5272_FEC
#define _IF_MCF5272_FEC
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

#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include <cyg/devs/eth/nbuf.h>

 /* Ethernet controller inetrrupt priority level. */
#define MCF5272_INT_LEVEL 4

/* Bit level definitions and macros */
#define MCF5272_FEC_ECR_RESET		(0x00000001)
#define MCF5272_FEC_ECR_ETHER_EN	(0x00000002)
#define MCF5272_FEC_EIR_GRA			(0x10000000)

#define MCF5272_FEC_EIR_HBE		    (0x80000000)
#define MCF5272_FEC_EIR_BR		    (0x40000000)
#define MCF5272_FEC_EIR_BT		    (0x20000000)
#define MCF5272_FEC_EIR_GRA		    (0x10000000)
#define MCF5272_FEC_EIR_TXF		    (0x08000000)
#define MCF5272_FEC_EIR_TXB		    (0x04000000)
#define MCF5272_FEC_EIR_RXF		    (0x02000000)
#define MCF5272_FEC_EIR_RXB		    (0x01000000)
#define MCF5272_FEC_EIR_MII		    (0x00800000)
#define MCF5272_FEC_EIR_EBERR		(0x00400000)
#define MCF5272_FEC_EIR_UMINT       (1<<21)

#define MCF5272_FEC_IMR_HBEEN		(0x80000000)
#define MCF5272_FEC_IMR_BREN		(0x40000000)
#define MCF5272_FEC_IMR_BTEN		(0x20000000)
#define MCF5272_FEC_IMR_GRAEN		(0x10000000)
#define MCF5272_FEC_IMR_TXFEN		(0x08000000)
#define MCF5272_FEC_IMR_TXBEN		(0x04000000)
#define MCF5272_FEC_IMR_RXFEN		(0x02000000)
#define MCF5272_FEC_IMR_RXBEN		(0x01000000)
#define MCF5272_FEC_IMR_MIIEN		(0x00800000)
#define MCF5272_FEC_IMR_EBERREN		(0x00400000)
#define MCF5272_FEC_RCR_PROM		(0x00000008)
#define MCF5272_FEC_RCR_MII_MODE	(0x00000004)
#define MCF5272_FEC_RCR_DRT			(0x00000002)
#define MCF5272_FEC_RCR_LOOP		(0x00000001)
#define MCF5272_FEC_TCR_GTS			(0x00000001)
#define MCF5272_FEC_TCR_HBC			(0x00000002)
#define MCF5272_FEC_TCR_FDEN		(0x00000004)
#define MCF5272_FEC_RDAR_DESTACT    (0x01000000)
#define MCF5272_FEC_MFLR_BRDCAST    (0x80000000)
#define MCF5272_FEC_MFLR_MLTCAST    (0x40000000)
#define MCF5272_FEC_XWMRK_64        (0x00000000)
#define MCF5272_FEC_XWMRK_128       (0x00000002)
#define MCF5272_FEC_XWMRK_192       (0x00000003)

/*      Define the PHY addresss.  This address specifies which 32  attached */
/* PHY devices.                                                             */

//#define MCF5272_PHY_ADD            (0)
#define MCF5272_PHY_ADD            (0 << 23)


/*      Start of frame delimter for valid MII management frame.             */

#define MCF5272_FEC_MII_ST          (0x40000000)

/*      Operation code.  This field bust be programmed to generate a  valid */
/* MII management frame.                                                    */

#define MCF5272_FEC_MII_OP_READ     (0x20000000)
#define MCF5272_FEC_MII_OP_WRITE    (0x10000000)

/*      Register address.  Specifies one of the 32 attached PHY devices.    */

#define MCF5272_FEC_MII_RA_POS      (18)
#define MCF5272_FEC_MII_RA_MASK     (0x1F)

/*      Turn around.  Must  be programmed  to 10  to generate  a valid  MII */
/* management frame.                                                        */

#define MCF5272_FEC_MII_TA          (0x00020000)


/*      The management frame data maske.  Field  for data to be written  to */
/* or read from PHY register.                                               */

#define MCF5272_FEC_MII_DATA_MASK   (0x0000FFFF)


/*      This define the MII unchanged MII frame management bits.            */

#define MCF5272_FEC_MII_FIX_HDR     (0 |                  \
                                     MCF5272_FEC_MII_ST | \
                                     MCF5272_PHY_ADD |    \
                                     MCF5272_FEC_MII_TA)


/*      The PHY address mask.                                               */

#define MCF5272_FEC_MII_PA          (0x1F << 23)

/*      Define the interrupt mask.                                          */

#define MCF5272_FEC_INTERRUPT_MASK (MCF5272_FEC_IMR_TXFEN    | \
                                    MCF5272_FEC_IMR_RXFEN    )


/**************************************************
 * FEC diagnostic counters
 **************************************************/
typedef struct MCF5272_FEC_DIAG
{

    /*   We put all the receive statistics first.                           */

    unsigned long     rx_pk_cnt;             /*   The   total   number   of */
                                             /* received packets            */

    unsigned long     rx_pk_cnt_sec;

    unsigned long     rx_bytes_cnt;          /*   The   total   number   of */
                                             /* received bytes              */

    unsigned long     rx_bytes_cnt_sec;

    unsigned long     rx_err_cnt;            /*   The total  number of  bad */
                                             /* packets received            */

    unsigned long     rx_long_frm_err_cnt;   /*   The total number of  long */
                                             /* frame errors.               */

    unsigned long     rx_short_frm_err_cnt;  /*   The total number of short */
                                             /* frame errors.               */

    unsigned long     rx_crc_err_cnt;        /*   The total  number of  CRC */
                                             /* errors.                     */

    unsigned long     rx_overrun_err_cnt;    /*   The   total   number   of */
                                             /* overrun errors.             */

    unsigned long     rx_trunc_error_cnt;    /*   The  total   numbers   of */
                                             /* receieve  truncated  packet */
                                             /* errors.                     */

    /*   We  put  all  the  transmit  statistics  next.   start_of_transmit */
    /* doesn't take any  space but  only separates  the receive  statistics */
    /* from the transmit statistics.                                        */

    char              start_of_transmit[0];

    unsigned long     tx_pk_cnt;          /*   The    total    number    of */
                                          /* transmitted packet             */

    unsigned long     tx_pk_cnt_sec;

    unsigned long     tx_bytes_cnt;       /*   The    total    number    of */
                                          /* transmitted bytes              */

    unsigned long     tx_bytes_cnt_sec;

    unsigned long     tx_err_cnt;         /*   The total  number of  failed */
                                          /* packet transmission            */

    unsigned long     tx_def_cnt;         /*   The total number of  tansmit */
                                          /* defers.                        */

    unsigned long     tx_hb_err_cnt;      /*   The total  number  of  heart */
                                          /* beat errors.                   */

    unsigned long     tx_late_col_cnt;    /*   The  total  number  of  late */
                                          /* collisions.                    */

    unsigned long     tx_exes_retry_cnt;  /*   Excessive rettry count.      */

    unsigned long     tx_retry_cnt;       /*   The total number of transmit */
                                          /* retries.                       */

    unsigned long     tx_underrun_cnt;    /*   The total number of transmit */
                                          /* underruns.                     */

    unsigned long     tx_carrrier_loss_cnt; /*   The   total   number    of */
                                            /* trasnmit carrier losses.     */

    unsigned long     tx_free_bd_cnt;     /*   The total  number  of  freed */
                                          /* buffer descriptor.             */

    unsigned long     tx_free_min_bd_cnt; /*   The minimum  number of  free */
                                          /* buffer descriptor count.       */

    unsigned long     tx_full_cnt;        /*   The  number  of  times  when */
                                          /* there is no transmit buffer.   */

    unsigned long     tx_not_complete_cnt;  /*   The number  of  times  the */
                                            /* device   driver   discovered */
                                            /* that the BD is still in  use */
                                            /* by the FEC.                  */

    unsigned long     internal_bus_error_cnt; /*   FEC bus error count.   A */
                                              /* bus  error  occurred  when */
                                              /* the FEC  was accessing  an */
                                              /* internal bus.              */

}MCF5272_FEC_DIAG __attribute__ ((aligned, packed));

/* Ethernet driver status. */
enum eth_drv_status_t
{
    ETH_DEV_UNKNOWN = 1,
    ETH_DEV_DOWN = 2,
    ETH_DEV_UP = 3
};

/* Ethernet duplex mode. */
enum eth_drv_mode_t
{
    ETH_MODE_UNKNOWN = 1,
    ETH_MODE_SIMPLEX = 2,
    ETH_MODE_DUPLEX = 3
};


/* Ethernet speed values. */
enum eth_speed_t
{
    ETH_SPEED_10MB  =   10*1000*1000,
    ETH_SPEED_100MB =  100*1000*1000
};

/* Ethernet driver statistics information structure. */

struct mcf5272_ether_drv_stats
{
    struct ifreq ifreq;                 // tell ioctl() which interface.

    char description[ DESC_LEN ];       // Textual description of hardware
    unsigned char snmp_chipset[ SNMP_CHIPSET_LEN ];
                                        // SNMP ID of chipset
    enum eth_drv_mode_t  duplex;               // 1 = UNKNOWN, 2 = SIMPLEX, 3 = DUPLEX
    enum eth_drv_status_t operational;          // 1 = UNKNOWN, 2 = DOWN, 3 = UP
    // These are general status information:
    unsigned int speed;                 // 10,000,000 or 100,000,000
                                        //     to infinity and beyond?

    MCF5272_FEC_DIAG stats;

}__attribute__ ((aligned, packed));

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type,field) (cyg_uint32)(&(((type*)0)->field)
#endif /* FIELD_OFFSET */

/* The value of 1 second in nanosecond. */
#define SEC_IN_NS 1000000000

/* 48-bit Ethernet Addresses */
typedef u8_t ETH_ADDR[6];

/* 16-bit Ethernet Frame Type, ie. Protocol */
typedef u16_t ETH_FTYPE;

/* Maximum and Minimum Ethernet Frame Size (Data Field) */
#define ETH_DATA_MAX_SIZE	(1500)
#define ETH_DATA_MIN_SIZE	(46)

/* Maximum and Minimum Ethernet Frame Size (Entire frame) */
#define ETH_MAX_SIZE	(ETH_DATA_MAX_SIZE+14)
#define ETH_MIN_SIZE	(ETH_DATA_MIN_SIZE+14)

/* Common Ethernet Frame definition */
typedef struct
{
	ETH_ADDR dest;
	ETH_ADDR src;
	ETH_FTYPE type;
	u8_t data[ETH_DATA_MAX_SIZE];
    u32_t fcs;
} eth_frame_hdr  __attribute__ ((aligned, packed));

/* 802.1Q Ethernet Frame definition */
typedef struct
{
	ETH_ADDR dest;
	ETH_ADDR src;
    u32_t header_802_1q;
	ETH_FTYPE type;
	u8_t data[ETH_DATA_MAX_SIZE];
    u32_t fcs;
} eth_802_1Q_frame_hdr  __attribute__ ((aligned, packed));

/* Definition of macros that access the FEC registers */
#define put_reg(_addr_,_value_) \
   *((volatile u32_t*)&(_addr_)) = (cyg_uint32)(_value_)

#define get_reg(_addr_) \
   *(((volatile u32_t*)&(_addr_)))


#endif /* _IF_MCF5272_FEC */


