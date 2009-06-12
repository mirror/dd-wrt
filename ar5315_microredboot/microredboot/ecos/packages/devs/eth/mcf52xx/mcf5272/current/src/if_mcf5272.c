//==========================================================================
//
//      dev/if_MCF5272_fec.c
//
//      Ethernet device driver for MCF5272's Fast Ethernet Controller (FEC)
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


// Ethernet device driver for Fast Ethernet MCF5272_fec
#include <pkgconf/io_eth_drivers.h>

#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <cyg/devs/eth/nbuf.h>
#include <cyg/devs/eth/if_mcf5272.h>
#include <cyg/devs/eth/if_mcf5272_private_data.h>

#include <cyg/infra/cyg_ass.h>
#include <sys/param.h>
#include <net/if.h>


/* Function to retrieve the Ethernet address of the device from the device's
   database. We declare it weak so that other routines can overide it.
   */

externC const void*
db_get_eth_address(void) __attribute__ ((weak));


/*****************************************************************************

     The following  functions  provide  an  interface  directly  to  the
ethernet driver for applications that wish to circumvent the IP stack.

     Applications that wish  to take advantage  of this should  override
these routine with their own.  Leaving these routines as default  routes
all data through the IP stack.

*****************************************************************************/
externC int_t
eth_rx_pkt_filter(u8_t* pkt, uint_t pkt_len) __attribute__ ((weak));
externC void
eth_tx_check(struct eth_drv_sg * sg_list, unsigned int sg_len)
             __attribute__ ((weak));
externC void
eth_send_done(unsigned long tag) __attribute__ ((weak));
externC int_t
eth_send(struct eth_drv_sg * sg_list, unsigned int sg_len, int total_len,
         unsigned long tag);

static MCF5272_fec_priv_data_t MCF5272_fec_priv_data;

/* Interrupt strcture and handles. */
static cyg_interrupt MCF5272_fec_rx_interrupt;
static cyg_interrupt MCF5272_fec_tx_interrupt;

static cyg_handle_t MCF5272_fec_rx_interrupt_handle;
static cyg_handle_t MCF5272_fec_tx_interrupt_handle;


// Interrupt handler
static void MCF5272_fec_int(struct eth_drv_sc *sc);
static int MCF5272_fec_int_vector(struct eth_drv_sc *sc);

// This DSR handles the ethernet [logical] processing
static void MCF5272_fec_deliver(struct eth_drv_sc * sc);
static void MCF5272_fec_stop(struct eth_drv_sc *sc);

static void
MCF5272_fec_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len,
            int total_len, unsigned long key);

static void
MCF5272_fec_common_send(struct eth_drv_sc *sc,
                        struct eth_drv_sg *sg_list, int sg_len,
                        int total_len,
                        unsigned long key,
                        tx_key_type_t key_type);

static int
MCF5272_fec_isr(cyg_vector_t vector, cyg_addrword_t data,
                HAL_SavedRegisters *regs);


// One-second call back alarm
static void one_second_alarm_func(cyg_handle_t alarm, cyg_addrword_t data);

// Retrieve statistics
static void MCF5272_get_stats(struct eth_drv_sc *sc, MCF5272_FEC_DIAG* stats);


ETH_DRV_SC(MCF5272_fec_sc,
           &MCF5272_fec_priv_data, // Driver specific data
           "eth0",             // Name for this interface
           MCF5272_fec_start,
           MCF5272_fec_stop,
           MCF5272_fec_control,
           MCF5272_fec_can_send,
           MCF5272_fec_send,
           MCF5272_fec_recv,
           MCF5272_fec_deliver,
           MCF5272_fec_int,
           MCF5272_fec_int_vector);

/* Device name */
static const char ether_device_name[] =  "MCF5272-eth";

NETDEVTAB_ENTRY(MCF5272_fec_netdev,
                ether_device_name,
                MCF5272_fec_init,
                &MCF5272_fec_sc);


/*******************************************************************************
    db_get_eth_address() - Returns the default Ethernet address.
*/
const void* db_get_eth_address(void)
{

    /*   Just use an obviously invalid address until someone overrides this */
    /* routine to provide their own address.                                */

    static const unsigned char enaddr[] =
    {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55
    };
    return  (const void*)enaddr;
}
/*******************************************************************************
 MCF5272_fec_init() - Routine that initializes the FEC.

 INPUT:
    tab - Pointer to the network device table.

 */
static bool MCF5272_fec_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    const u8_t *enaddr;

    /*   Indicate that the ethernet driver is down.                         */

    PMCF5272_FEC_DATA(sc)->operational = ETH_DEV_DOWN;

    /*   Initialize the entire driver private area to zero.                 */

    memset((char*)sc->driver_private, sizeof(MCF5272_fec_priv_data_t), 0);

    /*   Initialize the buffers structure.  This strucre contains  transmit */
    /* and receive buffer descriptor managment information.                 */

    nbuf_init(PBUF_INFO(sc));

    /*   Start a alarm that  will trigger  every second.   This alarm  will */
    /* periodically update the recevie and transmit statistics.             */

    cyg_clock_to_counter(cyg_real_time_clock(),
                         &(((MCF5272_fec_priv_data_t*)sc->driver_private)->counter_h));
    cyg_alarm_create(((MCF5272_fec_priv_data_t*)sc->driver_private)->counter_h,
                      one_second_alarm_func,
                     (cyg_addrword_t)(MCF5272_fec_priv_data_t*)sc->driver_private,
                      &(((MCF5272_fec_priv_data_t*)sc->driver_private)->alarm_h),
                      &(((MCF5272_fec_priv_data_t*)sc->driver_private)->alarm));
    cyg_alarm_initialize(((MCF5272_fec_priv_data_t*)sc->driver_private)->alarm_h,
                         cyg_current_time()+
                         (1*SEC_IN_NS)/CYGNUM_KERNEL_COUNTERS_RTC_PERIOD,
                         (1*SEC_IN_NS)/CYGNUM_KERNEL_COUNTERS_RTC_PERIOD);

    /*   Initialize environment, setup  receive, transmit and  non-critical */
    /* interrupt handlers.                                                  */

    cyg_drv_interrupt_create(CYGNUM_HAL_VECTOR_ERX,
                             MCF5272_INT_LEVEL, // Priority
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)MCF5272_fec_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &MCF5272_fec_rx_interrupt_handle,
                             &MCF5272_fec_rx_interrupt);
    cyg_drv_interrupt_create(CYGNUM_HAL_VECTOR_ETX,
                             MCF5272_INT_LEVEL, // Priority
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)MCF5272_fec_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &MCF5272_fec_tx_interrupt_handle,
                             &MCF5272_fec_tx_interrupt);

    /*   Attach interrupt here in order  to start receiving interrupt  from */
    /* the FEC.                                                             */

    cyg_drv_interrupt_attach(MCF5272_fec_rx_interrupt_handle);
    cyg_drv_interrupt_attach(MCF5272_fec_tx_interrupt_handle);



    put_reg(MCF5272_SIM->gpio.pbcnt, 0x55550000 |
            (get_reg(MCF5272_SIM->gpio.pbcnt) & 0x0000FFFF));


    /*   Reset the FEC - equivalent to a hard reset.                        */

    put_reg(MCF5272_SIM->enet.ecr, MCF5272_FEC_ECR_RESET);

    /*   Wait for the reset sequence to complete.                           */

    while(get_reg(MCF5272_SIM->enet.ecr) & MCF5272_FEC_ECR_RESET);

    /*   Set the Ethernet control register to zero to disable the FEC.      */

    put_reg(MCF5272_SIM->enet.ecr, 0);

    /*   Set the source address for the controller.                         */

    /*   Initialize  physical  address  register  by  copying  our  adapter */
    /* address from the device's permanent storage.                         */

    enaddr = (cyg_uint8*)db_get_eth_address();

    put_reg(MCF5272_SIM->enet.malr,0
                    		       | (enaddr[0] <<24)
                    		       | (enaddr[1] <<16)	
                    		       | (enaddr[2] <<8)
                    		       | (enaddr[3] <<0));
    put_reg(MCF5272_SIM->enet.maur,0
                    		       | (enaddr[4] <<24)
                    		       | (enaddr[5] <<16));

    /*   Initialize the hash  table registers  to ignore  hash checking  to */
    /* detect multicast Etherhet addresses.                                 */

    put_reg(MCF5272_SIM->enet.htur, 0);
    put_reg(MCF5272_SIM->enet.htlr, 0);

    /*   Set Receive Buffer Size.   This  is  the  size  for  each  receive */
    /* buffer.                                                              */

    put_reg(MCF5272_SIM->enet.emrbr, (uint16)RX_BUFFER_SIZE);

    /*   Point to the start of the circular Rx buffer descriptor queue.     */

    put_reg(MCF5272_SIM->enet.erdsr, nbuf_get_start(PBUF_INFO(sc), Rx));

    /*   Point to the start of the circular Tx buffer descriptor queue.     */

    put_reg(MCF5272_SIM->enet.etdsr, nbuf_get_start(PBUF_INFO(sc), Tx));

    /*   Set  the  FIFO  transmit  highwater  mark  to  128  bytes.   Frame */
    /* transmission begins when the number of bytes selected by this  field */
    /* are written into the  transmit FIFO,  if an  end of  frame has  been */
    /* written to the FIFIO, or if the FIFO is full before selected  number */
    /* of bytes are written.                                                */

    put_reg(MCF5272_SIM->enet.tfwr, MCF5272_FEC_XWMRK_128);

    /*   Clear any interrupts by setting all bits in the EIR register.      */

    put_reg(MCF5272_SIM->enet.eir, 0xFFFFFFFF);

    /*   Set the tranceiver interface to MII mode.                          */

    put_reg(MCF5272_SIM->enet.rcr, 0 | MCF5272_FEC_RCR_MII_MODE);
                                   // | MCF5272_FEC_RCR_DRT);

    /*   Set the mode is ETH_MODE_SIMPLEX.   We are assuming the device  is */
    /* half-duplex mode.                                                    */

    PMCF5272_FEC_DATA(sc)->duplex = ETH_MODE_SIMPLEX;

    /* The default speed is 10 Mbs. */

    PMCF5272_FEC_DATA(sc)->speed = ETH_SPEED_10MB;

    /*   Write the maximum  frame  length  and  setup  so  we  can  receive */
    /* broadcast packets.                                                   */

    put_reg(MCF5272_SIM->enet.mflr, MCF5272_FEC_MFLR_BRDCAST |
            sizeof(eth_frame_hdr));

    /*   Check for heartbeat count  and enable  full-duplex transmit.   The */
    /* hearbeat check is performed following end of transmission and the HB */
    /* bit in the status  reguster is set if  the collision input does  not */
    /* assert within the heartbeat window.                                  */

    /*   NOTE: We disable full  duplex mode  because we  notice that  we're */
    /* getting Receive CRC erors.                                           */


    put_reg(MCF5272_SIM->enet.tcr, 0 |MCF5272_FEC_TCR_HBC);
                                   //| MCF5272_FEC_TCR_FDEN);

    /*   Set  the  MII  frequency  divider.   The  MII_SPEED  controls  the */
    /* frequency of the MII management  interface  clock  relative  to  the */
    /* system clock.  We set MII  speed  to  7  because  the  system  clock */
    /* frequency is 66 Mhz.                                                 */

    put_reg(MCF5272_SIM->enet.mscr, 7<<1);

    /*   Initialize upper level driver.                                     */

    (sc->funs->eth_drv->init)(sc, (unsigned char *)enaddr);

    /*   Return  true  to  indicate  that  the  driver  initialization  has */
    /* completed successfully.                                              */

    return true;

}

/*      This function is called to  "start up"  the interface.   It may  be */
/* called multiple times, even  when the hardware  is already running.   It */
/* will be called whenever something "hardware oriented" changes and should */
/* leave the hardware ready to send/receive packets.                        */

static void
MCF5272_fec_start(struct eth_drv_sc *sc, cyg_uint8 *enaddr, int flags)
{
    	
    /*   Initialize the buffers structure.  This strucre contains  transmit */
    /* and receive buffer descriptor managment information.  We  initialize */
    /* again here becuase we don't  know the internal  state of the  buffer */
    /* descriptor pointer in the FEC if the FEC was disabled after  calling */
    /* MCF5272_fec_stop.                                                    */

    if (PMCF5272_FEC_DATA(sc)->operational != ETH_DEV_UP)
    {
        nbuf_init(PBUF_INFO(sc));
    }

    /*   Unmask the Transmit and Receive  frame  interrupt  to  handle  the */
    /* interrupts.                                                          */

    /*   Unmask the Internal Bus  Errorso we  can detect  any internal  bus */
    /* error when the FEC tries to acess the internal bus.                  */

    put_reg(MCF5272_SIM->enet.eimr,
            get_reg(MCF5272_SIM->enet.eimr) | MCF5272_FEC_INTERRUPT_MASK);

    /*   Enable FEC.                                                        */

    put_reg(MCF5272_SIM->enet.ecr, MCF5272_FEC_ECR_ETHER_EN);

    /*   Indicate that there have been empty receive buffers produced.      */

    put_reg(MCF5272_SIM->enet.rdar, MCF5272_FEC_RDAR_DESTACT);


    /*   Set the flag to indicate that the device is up and running.        */

    PMCF5272_FEC_DATA(sc)->operational = ETH_DEV_UP;

}

/*      A routine to halt the FEC.                                          */
static void
MCF5272_fec_stop(struct eth_drv_sc *sc)
{

    /*   Stop the packet transmission gracefully.                           */

    /*   Set the Graceful Transmit Stop bit.                                */

    put_reg(MCF5272_SIM->enet.tcr, get_reg(MCF5272_SIM->enet.tcr)
                                   | MCF5272_FEC_TCR_GTS);

    /*   Wait for the current transmission to complete.                     */

    while( !(get_reg(MCF5272_SIM->enet.eir) & MCF5272_FEC_EIR_GRA));

    /*   Clear the GRA event.                                               */

    put_reg(MCF5272_SIM->enet.eir, MCF5272_FEC_EIR_GRA);

    /*   Disable all FEC interrupts by clearing the IMR register.           */

    put_reg(MCF5272_SIM->enet.eimr,  0);

    /*   Clear the GTS bit so frames can be tranmitted when restarted       */

    put_reg(MCF5272_SIM->enet.tcr, get_reg(MCF5272_SIM->enet.tcr) &
                                   ~MCF5272_FEC_TCR_GTS);

    /*   Set the Ethernet control register to zero to disable the FEC.      */

    put_reg(MCF5272_SIM->enet.ecr, 0);

    /*   Deliver any pending frames and acknowledge any transmitted frames.   */

    MCF5272_fec_deliver(sc);

    /*   Set the flag to indicate that the device is down.                  */

    PMCF5272_FEC_DATA(sc)->operational = ETH_DEV_DOWN;

}

/*      This routine is called to perform special "control" opertions.       */

static int
MCF5272_fec_control(struct eth_drv_sc *sc, unsigned long key,
                    void *data, int data_length)
{
    switch (key)
    {

    case ETH_DRV_SET_MAC_ADDRESS:

        {
            /*   Set the hardware address of the Ethernet controller.       */

            struct ifreq* p_ifreq = data;

            /*   If the length of the strcutre is not equal to the size  of */
            /* ifreq, then exit with an error.                              */

            if (data_length != sizeof(*p_ifreq))
            {
                return 0;
            }

            /*   Set the lower 4-byte address.                              */

            put_reg(MCF5272_SIM->enet.malr,0
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[0] <<24)
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[1] <<16)	
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[2] <<8)
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[3] <<0));

            /*   Set the upper 2-byte address.                              */

            put_reg(MCF5272_SIM->enet.maur,
                                           0
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[4] <<24)
                            		       | (p_ifreq->ifr_ifru.ifru_hwaddr.sa_data[5] <<16));


            /*   Return 1 to indicate  that  programming  of  the  new  MAC */
            /* address is successful.                                       */

            return 1;
        }


        break;
    #ifdef CYGPKG_NET

    case ETH_DRV_GET_IF_STATS:
    case ETH_DRV_GET_IF_STATS_UD:

        #if 0
        {

            struct ether_drv_stats* pstats = (struct ether_drv_stats*)
                                                data;
            MCF5272_FEC_DIAG diag;

            /* Retrieve the driver defined diagnostic structure. */
            MCF5272_get_stats(sc, &diag);

            strcpy(pstats->description, ether_device_name);
            pstats->duplex = ETH_MODE_UNKNWON;
            pstats->operational = (unsigned char )PMCF5272_FEC_DATA(sc)->operational;
            pstats->speed = 0;

            /*   Translate the device specific  diagnostic  values  to  the */
            /* generic ether_drv_stats values.                              */

            /* Get the receive bytes count. */
            pstats->rx_count = diag.rx_bytes_cnt;

            /* Get the number of successful packet received. */
            pstats->rx_good = diag.rx_pk_cnt;

            /* Get the receive CRC error count. */
            pstats->rx_crc_errors = diag.rx_crc_err_cnt;

            /* Get the receive overrun error count. */
            pstats->rx_overrun_errors = diag.rx_overrun_err_cnt;

            /* Get the received short frame error count. */
            pstats->rx_short_frames = diag.rx_short_frm_err_cnt;

            /* Get the received long frame error count. */
            pstats->rx_too_long_frames = diag.rx_long_frm_err_cnt;

            /* Get the number of transmitted bytes. */
            pstats->tx_count = diag.tx_bytes_cnt;

            /* Get the number of defered packets. */
            pstats->tx_deferred = diag.tx_def_cnt;

            /* The number of successfully transmitted packets. */
            pstats->tx_good = diag.tx_pk_cnt;

            /* Get the transmit late collision count. */
            pstats->tx_late_collisions = diag.tx_late_col_cnt;

            /* Get the transmit underrun count. */
            pstats->tx_underrun = diag.tx_underrun_cnt;

            /* Get the transmit late collision count. */
            pstats->tx_total_collisions = diag.tx_late_col_cnt;
            return 1;
        }
        #else
        {

            /*   Copy the ethernet name device over.                        */

            strcpy(((struct mcf5272_ether_drv_stats*)data)->description,
                   ether_device_name);

            /* Get the stats. */
            MCF5272_get_stats(sc,
                              &((struct mcf5272_ether_drv_stats*)data)->stats);

            /* Copy the mode over. */

            ((struct mcf5272_ether_drv_stats*)data)->duplex =
                PMCF5272_FEC_DATA(sc)->duplex;

            /* The ethernet driver is operational. */

            ((struct mcf5272_ether_drv_stats*)data)->operational =
                PMCF5272_FEC_DATA(sc)->operational;

            /*   Copy the speed over.                                       */

            ((struct mcf5272_ether_drv_stats*)data)->speed =
                PMCF5272_FEC_DATA(sc)->speed;

            return 1;
        }

        #endif
        break;

    #endif /* CYGPKG_NET */
    default:
        return 1;
        break;
    }

}


/*      This routine is  called to see  if it is  possible to send  another */
/* packet.  It will  return  non-zero  if  a  transmit  is  possible,  zero */
/* otherwise.                                                               */

static int
MCF5272_fec_can_send(struct eth_drv_sc *sc)
{
    const int buffer_window = 5; /* Specifies the minimum empty buffer descrpitors */

    if ((NUM_TXBDS - PBUF_INFO(sc)->num_busy_bd) > buffer_window)
    {
        return 1;
    }
    else
    {
        PMCF5272_FEC_DATA(sc)->diag_counters.tx_full_cnt++;
        return 0;
    }
}


/*      This routine is called  by eCos  to send  a frame  to the  ethernet */
/* controller.                                                              */

static void
MCF5272_fec_send(struct eth_drv_sc *sc,
                 struct eth_drv_sg *sg_list,
                 int sg_len,
                 int total_len,
                 unsigned long key)
{

    /*   Call eth_tx_check() routine for any packet transmitted by eCos.    */

    eth_tx_check(sg_list, sg_len);


    /*   If   we   do   have   enough    buffer    to    send    we    call */
    /* MCF5272_fec_common_send routine to send  the packet.  Otherwise,  we */
    /* throw away the packet and call eCos's tx_done rutine to notify  that */
    /* the packet has been sent.                                            */

    if (NUM_TXBDS - PBUF_INFO(sc)->num_busy_bd > sg_len)
    {
        MCF5272_fec_common_send(sc, sg_list, sg_len, total_len, key, TX_KEY_ECOS);


    }
    else
    {

        CYG_ASSERT(false, "ETH: Send buffer full");

        /*   Inform the upper layer of a completion of the packet.          */

        (sc->funs->eth_drv->tx_done)(sc,
                                     key,
                                     0);
    }

}

/*      This routine is called to send a frame to the ethernet  controller. */
/* This is a generic send routine.                                          */
/*

  INPUT:
    sc - Ethernet driver sc.
    sg_glist - scatter gather list.
    sg_len - The number of scattter gather entries in the list.
    total_len - The total length of the frame.

*/


static void
MCF5272_fec_common_send(struct eth_drv_sc *sc,
                        struct eth_drv_sg *sg_list,
                        int sg_len,
                        int total_len,
                        unsigned long key,
                        tx_key_type_t key_type)
{
    int i = 0;
    NBUF *pBD = NULL;
    NBUF *first_bd;
    buf_info_t* p_buf = PBUF_INFO(sc);


    CYG_ASSERT(sg_len > 0,  "ETH: sg_len cannot be zero");


    /* Update the number of used transmitted buffer desciptors. */

    p_buf->num_busy_bd += sg_len;

    /*   Keep  track  of  the  maximum  number  of  busy  transmit   buffer */
    /* descriptors.                                                         */

    if (p_buf->num_busy_bd > p_buf->max_num_busy_bd)
    {
        p_buf->max_num_busy_bd = p_buf->num_busy_bd;
    }

    /*   Enqueue the key, the index to first and last buffer desriptor, the */
    /* number of  buffer descriptors,  the packet  length and  the type  of */
    /* packet to the transmit queue.                                        */

    nbuf_enq_tx_key(p_buf, (p_buf->iTxbd + sg_len - 1) % NUM_TXBDS,
                    key,
                    p_buf->iTxbd,
                    sg_len,
                    total_len,
                    key_type);

    /*   Get the pointer to the first buffer descriptor of the packet.   We */
    /* don't set the R bit for the first packet until we have allocated and */
    /* initialized all the buffer descriptors.                              */

    first_bd = pBD = nbuf_tx_allocate(p_buf);

    do
    {

        CYG_ASSERT(pBD != NULL, "ETH: nbuf_tx_allocate() returned NULL");


        /*   Copy the address of the buffer and the length to the allocated */
        /* buffer descriptor.  Note that  buf_index indexes  to the  buffer */
        /* descriptor it returns.                                           */

        pBD->data = (uint8*)sg_list[i].buf;
        pBD->length = sg_list[i].len;

        if (i == sg_len - 1)
        {

            /*   Set the the L,  TC and R bit  to indicate that the  buffer */
            /* descriptor  is  the  last  buffer  descriptor,  the  CRC  is */
            /* appended by the FEC and  the buffer descriptor is ready  for */
            /* transmission.                                                */

            pBD->status |= TX_BD_L | TX_BD_TC | TX_BD_R;

            /*   When we have  reached the  last buffer  scatther list,  we */
            /* break from the loop.                                         */

            break;

        }
        else
        {

            if (i)
            {

                /*   If the buffer  descriptor  is  not  the  first  buffer */
                /* descriptor, then set the  R bit to  notify the FEC  that */
                /* the buffer descriptor is  read  for  transmission.   FEC */
                /* must reset R bit after transmitted for buffer.           */

    	        pBD->status |= TX_BD_R;

            }

            /*   Allocate the next buffer descriptor.                       */

            pBD = nbuf_tx_allocate(p_buf);

        }

        /*   Advance to the next index.                                     */

        i++;

    }while(1);

    /*   Set the R bit in the first buffer descriptor to indicate that  the */
    /* buffer is ready for transmission if  there are more than one  buffer */
    /* descriptors.                                                         */

    if (sg_len > 1)
        first_bd->status  |= TX_BD_R;

    /*   Indicate that there is a transmit buffer ready.                    */

    put_reg(MCF5272_SIM->enet.tdar, 1);

}


/*      This function is called  as a result  of the "eth_drv_recv()"  call */
/* above.  It's job is to  actually  fetch  data  for  a  packet  from  the */
/* hardware once memory buffers have  been allocated for the packet.   Note */
/* that the buffers may come in pieces, using a scatter-gather list.   This */
/* allows for more efficient processing in the upper layers of the stack.   */

/*      Note that the  total buffer allocated  for the scatter-gather  list */
/* can be smaller than the packet or the buffer that in the scatter  gather */
/* list is invalid.  This happens when  the upper layer driver runs out  of */
/* buffers for the scatter-gather list.                                     */

static void
MCF5272_fec_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{	
    uint_t fill_count = 0, buf_count;
    uint_t frame_length = 0, buf_len;
    NBUF *pNbuf = NULL;
    cyg_uint8* buf = NULL;
    cyg_bool_t done = false;
    uint_t sg_index = 0;

    /*   If the scatter-gather list is zero,  set buf to NULL so that  this */
    /* routine would not copy the buffer to the scatter-gatther buffer.     */

    if (sg_len > 0)
    {
        buf = (cyg_uint8*)sg_list[0].buf;
    }

    do
    {

        /*   Get the next buffer descriptor (bd).                           */

        pNbuf = nbuf_rx_get_next(PBUF_INFO(sc));

        CYG_ASSERT(pNbuf != NULL, "Cannot get the next bd");

        if (pNbuf->status & TX_BD_L)
        {

            /*   Calculate the remaining numer of bytes in the packet  that */
            /* needed to be copied out since  the the length of the in  the */
            /* last BD contains the total length of the packet and not  the */
            /* length of the buffer.                                        */

            buf_len = (uint_t)pNbuf->length - frame_length;

            /*   Since the last bd, set done  to true in order to exit  the */
            /* loop.                                                        */

            done = true;
        }
        else
        {
            buf_len = RX_BUFFER_SIZE;

            /*   Update the frame length.                                   */

            frame_length += RX_BUFFER_SIZE;
        }


       /*   Copy the packet to the scatter gather list if there is a buffer */
       /* to copy the packet to.                                            */

       for (buf_count = 0; buf_count < buf_len && buf != NULL;)
       {

           uint_t copy_len;

           /*   Retrieve the minimum copy length.  We basically copy  based */
           /* on the smaller size: the buffer from the scatther list or the */
           /* from the buffer descriptor.                                   */

           copy_len =  (((uint_t)sg_list[sg_index].len - fill_count) <
                        (buf_len - buf_count) ?
                        (uint_t)sg_list[sg_index].len - fill_count :
                        (buf_len - buf_count));

           /*   Copy the buffer to the upper layer driver buffer.           */

           memcpy(&buf[fill_count],
                  &pNbuf->data[buf_count],
                  copy_len);

           /*   Update the counts to reflect the number of bytes copied.    */

           fill_count += copy_len;
           buf_count += copy_len;

           /*   If the buffer  in  the  scatter-gather  list  is  full,  we */
           /* attempt to retrieve the next buffer in the list.              */

           if (fill_count >= sg_list[sg_index].len)
           {

               /*   If there is no more  buffer, set  the buf  to NULL  and */
               /* exit the loop.                                            */

               if (++sg_index >= sg_len)
               {
                   buf = NULL;
                   break;
               }
               else
               {
                   buf = (cyg_uint8*)sg_list[sg_index].buf;
                   fill_count = 0;
               }
           }
       }


       /*   Release the buffer scriotor so it cab be used to receive  other */
       /* packets.                                                          */

        nbuf_rx_release(pNbuf);

    }while(done == false);

    /*   Notify the FEC that there are receive buffer descriptors available */
    /* for the FEC.                                                         */

    put_reg(MCF5272_SIM->enet.rdar, MCF5272_FEC_RDAR_DESTACT);
}

/*******************************************************************************
MCF5252_fec_recv_handler() - Receive handler to read the received
buffer descriptors and inform the upper layer driver of the arrival
of the packet.
*/
inline static
bool MCF5252_fec_recv_handler(struct eth_drv_sc * sc)
{

    /*   Receive interrupt has occurred informing  driver that a frame  has */
    /* been written to the buffer.                                          */

    NBUF* pNBuf;
    buf_info_t* p_buf_info = PBUF_INFO(sc);
    MCF5272_fec_priv_data_t* eth_data = PMCF5272_FEC_DATA(sc);
    uint_t len;

    /*   Pointer to the first buffer descriptor.                            */

    NBUF* p_first_bd;

    /*   Check to see if the buffer descrpitor is not busy.                 */

    if  (nbuf_rx_next_ready(p_buf_info))
    {

        /*   Flag that indicates whether the buffer is wrapped.             */

        cyg_bool_t wrap = false;

        /*   Get the index of the buffer desrciptor of the next frame.       */

        uint_t index = nbuf_rx_get_index(p_buf_info);
        cyg_bool_t error = false;
        len = 0;

        /* Get the pointer to the first buffer descriptor. */

        p_first_bd = nbuf_rx_get(p_buf_info, index);

        do
        {
            pNBuf = nbuf_rx_get(p_buf_info, index);

            if (pNBuf->status & RX_BD_E)
            {

                /*   The buffer is empty or the FEC is stll writing to  the */
                /* buffer.  then exit.                                      */

                return false;

            }

            /*   Advance the index  to the  next buffer  descriptor in  the */
            /* ring buffer.                                                 */

            index = (index + 1) % NUM_RXBDS;

            if ((pNBuf->status & (RX_BD_L | RX_BD_W)) == RX_BD_W)
            {

                /*   If the buffer descriptor wraps, set the wrap flag  and */
                /* initalize  the  index  pointer   to  the  first   buffer */
                /* descriptor in the ring.                                  */

                wrap = true;

            }

            if (pNBuf->status & RX_BD_TR)
            {

                /*   Packet truncate count.                                 */

                eth_data->diag_counters.rx_trunc_error_cnt++;

                /*   Release the bds.                                       */

                nbuf_rx_release_pkt(p_buf_info);

                /*   Update the receive error count.                        */

                eth_data->diag_counters.rx_err_cnt++;

                /*   Notify  the  FEC   that  there   are  receive   buffer */
                /* descriptors available from the FEC.                      */

                put_reg(MCF5272_SIM->enet.rdar, MCF5272_FEC_RDAR_DESTACT);

                error = true;
                break;
            }

            if (pNBuf->status & RX_BD_L)
            {

                /*   Get the length of frame contain in the buffers.        */

                len = pNBuf->length;

                /*   If there is an  error  in  receiving  the  packet,  we */
                /* proceed to update the counters.  Otherwise, we just fall */
                /* through.                                                 */

                if (pNBuf->status & (RX_BD_LG | RX_BD_SH | RX_BD_CR |
                                     RX_BD_OV))
                {

                    /*   Update the diagnostic counters.                    */

                    if (pNBuf->status & RX_BD_LG)
                    {
                        /* Larget frame error count. */
                        eth_data->diag_counters.rx_long_frm_err_cnt++;
                    }

                    if (pNBuf->status & RX_BD_SH)
                    {
                        /* Short  frame error count. */
                        eth_data->diag_counters.rx_short_frm_err_cnt++;
                    }

                    if (pNBuf->status & RX_BD_CR)
                    {
                        /* CRC error count. */
                        eth_data->diag_counters.rx_crc_err_cnt++;
                    }

                    if (pNBuf->status & RX_BD_OV)
                    {
                        /* Overrun error count. */
                        eth_data->diag_counters.rx_overrun_err_cnt++;
                    }

                    /*   Release the packet.                                */

                    nbuf_rx_release_pkt(p_buf_info);

                    /*   Update the receive error count.                    */

                    eth_data->diag_counters.rx_err_cnt++;

                    /*   Notify the  FEC  that  there  are  receive  buffer */
                    /* descriptors available for the FEC                    */

                    put_reg(MCF5272_SIM->enet.rdar, MCF5272_FEC_RDAR_DESTACT);

                    /*   Set the error flag to true to indicate that  there */
                    /* is an error.                                         */

                    error = true;

                }
            }
        }while(!(pNBuf->status & RX_BD_L));

        if (error == false)
        {
            cyg_uint8* buf_ptr =  p_first_bd->data;

            /*   Since there is no error,  we  update  the  good  statistic */
            /* counters.                                                    */

            /*   Update the number of frames received.                      */

            eth_data->diag_counters.rx_pk_cnt++;

            /*   Update the number of bytes in the frame received.          */

            /*   Subract  4  bytes  from  the  length  because  the  packet */
            /* includes the 4-byte FCS.                                     */

            eth_data->diag_counters.rx_bytes_cnt += (len - 4);


            /*   If the  packet  wraps  then  copy  the  packet  to  the  a */
            /* temporary buffer in  order  to  make  the  packet  contigous */
            /* packet in memory.                                            */

            if (wrap)
            {

                uint_t count_len = 0;
                uint_t pk_len;

                /*   Set p_buf the pointer to temporary packet biffer which */
                /* we will use to copy the packet.                          */

                u8_t* p_buf =  (u8_t*)&eth_data->pkt_buf;

                /* Get the index of the buffer desrcitor of the next frame. */

                uint_t index = nbuf_rx_get_index(p_buf_info);

                do
                {

                    /*   Get the buffer descriptor.                         */

                    pNBuf = nbuf_rx_get(p_buf_info, index);

                    /*   Calculate the  length of  the data  in the  buffer */
                    /* descriptor.  If we reach the last buffer descriptor, */
                    /* the  the  actual  data  size  in  the  last   buffer */
                    /* descriptor is  the number  of bytes  we have  copied */
                    /* less from the value of the length field of the  last */
                    /* buffer descriptor.                                   */

                    if (pNBuf->status & RX_BD_L)
                    {
                        pk_len = len - count_len;
                    }
                    else
                    {
                        pk_len = RX_BUFFER_SIZE;
                    }

                    /*   Copy the content of  the buffer  to the  temporary */
                    /* packet buffer.                                       */

                    memcpy(&p_buf[count_len], pNBuf->data, pk_len);


                    /*   Keep count of  the number of  bytes read from  the */
                    /* buffer descriptor.                                   */

                    count_len += pk_len;

                    /*   Advance to the to next buffer descriptor.          */

                    index = (index + 1) % NUM_RXBDS;

                }while(!(pNBuf->status & RX_BD_L));

                buf_ptr = (u8_t*)&eth_data->pkt_buf;


            }

            /*   If there is  a copy  wrap error  or the  Rx packet  filter */
            /* rejected the packet, pass it to the upper layer.  Otherwise, */
            /* release the buffer descriptors.                              */

            if (!eth_rx_pkt_filter(buf_ptr, len))
            {

                /*   Inform the upper layer of a complete packet.           */

                 (sc->funs->eth_drv->recv)(sc, len);
            }
            else
            {

                /*   Release the buffer descriptors.                        */

                nbuf_rx_release_good_pkt(p_buf_info);


                /*   Notify the FEC that there is at least a receive buffer */
                /* descriptos available for the FEC.                        */

                put_reg(MCF5272_SIM->enet.rdar, MCF5272_FEC_RDAR_DESTACT);

            }

        }

    }
    else
    {

        /*   Indicates that there are no more receive packets.              */

        return false;
    }
    return true;

}
/*******************************************************************************
MCF5272_fec_transmit_handler() - Transmit handler informs the upper
layer packet of a completion of a transmit packet.
*/

static
bool MCF5272_fec_transmit_handler(struct eth_drv_sc * sc)

/*      If the FEC has successfull transmitted a packet, then realease  the */
/* buffer used for the packet so that the buffer can be reused.             */

{

    /*   Check to see which frame has completed sending so we can tell  the */
    /* upper layer to free up ts buffer.                                    */

    buf_info_t* p_buf_info = PBUF_INFO(sc);
    MCF5272_fec_priv_data_t* eth_data = PMCF5272_FEC_DATA(sc);
    NBUF* pNbd = NULL;
    int_t index, i;
    tx_keys_t key_entry;
    bool result = true;
    NBUF* next_bd;


    /*   Check wether there is any pending transmit buffer descriptors that */
    /* are to deallocated.                                                  */

    if ((index = nbuf_peek_tx_key(p_buf_info)) != -1)
    {

        /*   Get the pointer to the buffer descriptor so that the flags  in */
        /* the status word in the buffer descriptor can be examined.        */

        NBUF* pNbuf = nbuf_tx_get(p_buf_info, index);

        CYG_ASSERT(pNbuf->status & TX_BD_L, "Index to BD is not the last BD");

        if (pNbuf->status & TX_BD_R)
        {


            /*   Increment the number of times the device driver  discovers */
            /* that the buffer descriptor is still in use by the FEC and it */
            /* has been skipped.                                            */

            if ((next_bd = nbuf_peek_bd_ahead(p_buf_info)))
            {
                if (!(next_bd->status & TX_BD_R) &&
                    pNbuf->status & TX_BD_R)
                {

                    eth_data->diag_counters.tx_not_complete_cnt++;
                    goto RELEASE_BUF;

                }
            }

            /*   If the buffer not ready we return immediatly.               */

            return false;

         }

RELEASE_BUF:

        /*   Dequeue the packet from the trasnmt packet queue to  indicate */
        /* that the packet is not being transmitted by the FEC.            */

        nbuf_deq_tx_key(p_buf_info, &key_entry);

        /*   Release the used buffers.                                     */

        for ( i = 0; i < key_entry.num_dbufs; i++)

        {

            /*   Get the BD based on the index.                            */

            pNbd = nbuf_tx_get(p_buf_info,
                              (key_entry.start_index + i) % NUM_TXBDS);

            /*   The last buffer descriptor of the packet.                 */

            if (pNbd->status & TX_BD_L)
            {

                if (pNbd->status & TX_BD_RL)
                {

                    /*   Update the number of retries.                     */

                    eth_data->diag_counters.tx_retry_cnt += 16;
                    eth_data->diag_counters.tx_err_cnt++;
                    eth_data->diag_counters.tx_exes_retry_cnt++;
                }
                else
                {

                    /*   Check for  error status.   If there  is an  error */
                    /* proceed the  increment  the  appropriate  statistic */
                    /* counter.                                            */

                    if (pNbd->status  & (TX_BD_UN |
                                         TX_BD_LC |
                                         TX_BD_CSL |
                                         TX_BD_HB))
                    {

                        if (pNbd->status & TX_BD_HB)
                        {

                            /*   Heartbeat error count.                     */

                            eth_data->diag_counters.tx_hb_err_cnt++;
                        }

                        if (pNbd->status & TX_BD_UN)
                        {

                            /*   Transmit underrun error count.             */

                            eth_data->diag_counters.tx_underrun_cnt++;
                        }

                        if (pNbd->status & TX_BD_CSL)
                        {

                            /*   Transmit carrier loss count.               */

                            eth_data->diag_counters.tx_carrrier_loss_cnt++;
                        }

                        if (pNbd->status & TX_BD_LC)
                        {

                            /*   Update the late collision counter.         */

                            eth_data->diag_counters.tx_late_col_cnt++;
                        }

                        /*   Update the number transmit error count.       */

                        eth_data->diag_counters.tx_err_cnt++;
                    }
                    else
                    {
                        /* Update the diagnostic counters */

                        if (pNbd->status & TX_BD_DEF)
                        {
                            /* Defer indication count. */
                            eth_data->diag_counters.tx_def_cnt++;
                        }

                        /*   Update the number of transmitted packets.     */

                        eth_data->diag_counters.tx_pk_cnt++;

                        /*   Update the  transmitted packet  size.  If  the */
                        /* size is less than the minimum size then take the */
                        /* minimum size as the  actual frame length.   This */
                        /* is because we take account of the pad bytes that */
                        /* the FEC appends when  it sends  out frames  that */
                        /* has less than the minimum length.                */

                        eth_data->diag_counters.tx_bytes_cnt +=
                            key_entry.pk_len < ETH_MIN_SIZE ? ETH_MIN_SIZE :
                            key_entry.pk_len;
                    }

                    /*   Get the number of retries.                        */

                    eth_data->diag_counters.tx_retry_cnt +=
                        (pNbd->status >> 2) & 0xF;
                }

            }

            /*   Release the buffer descriptor so that it can be reused to */
            /* transmit the next packet.                                   */

            nbuf_tx_release(pNbd);

            /*   If the start_index is the same  as the index to the  last */
            /* buffer descriptor, then quit the loop.                      */

        }

        /*   Decrement the number of busy descriptors.                     */

        p_buf_info->num_busy_bd -=  key_entry.num_dbufs;

         switch(key_entry.key_type)
         {
             case TX_KEY_ECOS:

             /*   Inform the upper layer of a completion of the packet.     */

             (sc->funs->eth_drv->tx_done)(sc,
                                         key_entry.tx_key,
                                         0);
             break;
         case TX_KEY_USER:

             /*   Inform the application of the completion of the packet.   */

             eth_send_done(key_entry.tx_key);
             break;
         }

    }
    else
    {

        /*   Retrun false to indicate that the transmit buffer is empty.    */

        result =  false;
    }

    /*   Indicate  that  the  packet  at  the  transmit  buffer  has   been */
    /* successfully handled.                                                */

    return result;
}


/*      This routine informs  the upper  layer of  a completion  of a  sent */
/* frame and a reception of a frame.                                        */

static void
MCF5272_fec_deliver(struct eth_drv_sc * sc)
{
    u32_t event;


    /*   Clear the event register.                                          */

    put_reg(MCF5272_SIM->enet.eir,
            (event = (get_reg(MCF5272_SIM->enet.eir) &
                      MCF5272_FEC_INTERRUPT_MASK)));

    while(event & MCF5272_FEC_INTERRUPT_MASK)
    {

        /*   This flag will  specifies  whether  we  need  to  service  the */
        /* transmit or receive sides of the Ethernet controller.            */

        bool packet_status;


        /*   Keep count of  any bus  error that  might occur  when the  FEC */
        /* attempts while the FEC accessing the internal bus.               */

        if (event & MCF5272_FEC_EIR_EBERR)
        {
            PMCF5272_FEC_DATA(sc)->diag_counters.internal_bus_error_cnt++;
        }

        do
        {
            packet_status = false;

            /*   Call receive the handler to receive packets.               */

            packet_status |= MCF5252_fec_recv_handler(sc);

            /*   Call transit the handed to release the transmit buffers.   */

            packet_status |= MCF5272_fec_transmit_handler(sc);

            /*   Loop back up  until all the  receive and transmit  buffers */
            /* are empty.                                                   */

        }while(packet_status);

        /*   Retrieve the next interrupt event.                             */

        /*   Clear the event register.                                      */

        put_reg(MCF5272_SIM->enet.eir,
                event = (get_reg(MCF5272_SIM->enet.eir) &
                         MCF5272_FEC_INTERRUPT_MASK));

    }

    /*   NOTE: If the a bit in the eir is set after clearing the bit in the */
    /* eir, unmasking the bit in the imr will generate an interrupt.   This */
    /* assumption is true only if the interrupt line to the  microprocessor */
    /* core is level sensitive.                                             */

    /*   Allow interrupts by setting IMR register.                          */

	put_reg(MCF5272_SIM->enet.eimr, get_reg(MCF5272_SIM->enet.eimr) |
                                    MCF5272_FEC_INTERRUPT_MASK
                                    );

}

/*      Generic Interrupt Service Routine.  This  routine wakes up the  DSR */
/* for further processing.                                                  */

static int
MCF5272_fec_isr(cyg_vector_t vector, cyg_addrword_t data,
                HAL_SavedRegisters *regs)
{

    /*   Mask  the  FEC's  interrupts  so  that  it  won't  generate  these */
    /* interrupts anymore as the driver  reads the packets or  acknowledges */
    /* packets.                                                             */

    put_reg(MCF5272_SIM->enet.eimr, get_reg(MCF5272_SIM->enet.eimr) &
            ~(MCF5272_FEC_INTERRUPT_MASK));

    return CYG_ISR_CALL_DSR;
}


/*      Call MCF5272_fec_deliver() to poll the FEC.                         */

static void
MCF5272_fec_int(struct eth_drv_sc *sc)
{
    MCF5272_fec_deliver(sc);
}

static int
MCF5272_fec_int_vector(struct eth_drv_sc *sc)
{

    /*   How do you return multiple interrupt vector?                       */

    return CYGNUM_HAL_VECTOR_ERX;
}

/*      This routine updates the Ethernet statistics counters.  This method */
/* is called by eCos every second.                                          */

static void one_second_alarm_func(cyg_handle_t alarm, cyg_addrword_t data)
{
    #define WRAP_SUBTRACT(_VAL1_,_VAL2_) \
        ({unsigned long val;if (_VAL1_ >= _VAL2_) val = _VAL1_ - _VAL2_; \
         else val=(0-_VAL2_)+_VAL1_; val;})

    ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_bytes_cnt_sec =
        WRAP_SUBTRACT(
            ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_bytes_cnt,
            ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_rx_bytes_cnt);

    ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_bytes_cnt_sec =
        WRAP_SUBTRACT(
            ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_bytes_cnt,
            ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_tx_bytes_cnt);

    ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_pk_cnt_sec =
        WRAP_SUBTRACT(
            ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_pk_cnt,
            ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_rx_pk_cnt);

    ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_pk_cnt_sec =
        WRAP_SUBTRACT(
            ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_pk_cnt,
            ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_tx_pk_cnt);


    ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_rx_bytes_cnt =
        ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_bytes_cnt;
    ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_tx_bytes_cnt =
        ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_bytes_cnt;
    ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_rx_pk_cnt =
        ((MCF5272_fec_priv_data_t*)data)->diag_counters.rx_pk_cnt;
    ((MCF5272_fec_priv_data_t*)data)->diag_info_sup.old_tx_pk_cnt =
        ((MCF5272_fec_priv_data_t*)data)->diag_counters.tx_pk_cnt;
}

/*      Retrieve the stats information.                                     */

void MCF5272_get_stats(struct eth_drv_sc *sc, MCF5272_FEC_DIAG* stats)
{
    memcpy(stats, &PMCF5272_FEC_DATA(sc)->diag_counters,
           sizeof (MCF5272_FEC_DIAG));

    /*   Retrieve the  number  of  available  buffer  descriptors  and  the */
    /* minimum value.                                                       */

    PMCF5272_FEC_DATA(sc)->diag_counters.tx_free_bd_cnt =
        NUM_TXBDS - PBUF_INFO(sc)->num_busy_bd;
    PMCF5272_FEC_DATA(sc)->diag_counters.tx_free_min_bd_cnt =
        NUM_TXBDS - PBUF_INFO(sc)->max_num_busy_bd;
}


/*****************************************************************************

     The following  functions  provide  an  interface  directly  to  the
ethernet driver for applications that wish to circumvent the IP stack.

     Applications that wish  to take advantage  of this should  override
these routine with their own.  Leaving these routines as default  routes
all data through the IP stack.

*****************************************************************************/

/*****************************************************************************
eth_rx_pkt_filter -- Ethernet receive packet filter

     This is an ethernet packet filter function that allows the application
to receive raw ethernet frames from  the driver.  The return value of  this
function determines whether or not to pass the packet to the IP stack.

     We declare it weak so that other routine can override it.

INPUT:

     pkt: Pointer to the packet.

     pkt_len: The length of  the packet including  all headers, the  32-bit
Ethernet CRC, and any Ethernet frame padding.

OUTPUT:

RETURN VALUE:

     true: Do not send the packet to the IP stack.

     false: Send the packet to the IP stack.

*****************************************************************************/
int_t eth_rx_pkt_filter(u8_t * pkt, uint_t pkt_len)
{

    /*   Always return false by default. */

    return false;
}

/*****************************************************************************
eth_tx_check -- Watch transmitting Ethernet packets

     The driver calls this routine before transmitting packets from  the
IP stack.  It provides a hook for applications to watch all packets that
the IP stack transmits.

     We declare it weak so that other routine can override it.

INPUT:

     sg_list: Pointer to the scatter-gather list.

     sg_len: The number of scatter-gather entries in the list.

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void eth_tx_check(struct eth_drv_sg * sg_list, unsigned int sg_len)
{

    /*   Do nothing by default.                                             */

}

/*****************************************************************************
eth_send -- Transmit a raw Ethernet packet

     This function sends  a  packet  to  the  Ethernet  controller  thus
passing the eCos IP stack.

     Note that the application must reuse the buffer parameters to  this
function until the driver releases  the buffer with the  eth_send_done()
function.

INPUT:

     sg_list: Pointer to the scatter-gather list to send.

     sg_len: The size of the scatter-gather list.

     tag: A value  that  we  pass  as  a  parameter  to  eth_send_done()
function when the FEC has completed sending the packet.

OUTPUT:

RETURN VALUE:

     true: if the packet is  successfully queued to the device  driver's
queue.

     false : If the routine fails to send the packet.

*****************************************************************************/
int_t eth_send(struct eth_drv_sg* sg_list, unsigned int sg_len,
              int total_len,
              unsigned long tag)
{
    int_t success;
    cyg_uint32 s;

    s = cyg_splsoftnet();

    /*   If there is enough  buffer  descriptors,  then  send  the  packet. */
    /* Otherwise, throw the packet away and return a false value.           */

    if (NUM_TXBDS - PBUF_INFO(&MCF5272_fec_sc)->num_busy_bd > sg_len)
    {

        /*   Call the common send routine to send the packet.               */

        MCF5272_fec_common_send(&MCF5272_fec_sc,
                                sg_list,
                                sg_len,
                                total_len,
                                tag,
                                TX_KEY_USER);

        success = true;
    }
    else
    {

        /*   Fail to send the packet.                                       */

        success = false;
    }

    /*   Release the Ethernet driver lock.                                  */

    cyg_splx(s);

    return success;
}

/*****************************************************************************
eth_send_done -- eth_send callback

     The driver calls this  function when  it has  sent out  the packet.   The
parameter tag is the same  value as the  tag value when  the caller calls  the
eth_send to send the packet.

     We declare it weak so that other routine can override it.

INPUT:

     tag: A value  that  we  pass  as  a  parameter  to  eth_send_done()
function when the FEC has completed sending the packet.

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void eth_send_done(unsigned long tag)
{

}


#if 0


/* Defined Ethernet Frame Types */
#define FRAME_IP	(0x0800)
#define FRAME_ARP	(0x0806)
#define FRAME_RARP	(0x8035)

/* Offset and size of protocol headers */
#define ETH_HDR_OFFSET	0	/* Ethernet header at the top of the frame */
#define ETH_HDR_SIZE	14

/* Assign a protocol number for the loop test */
static uint16 eth_type = 0x0300;

/* Global variable containing length of data to transmit */
static uint16 data_length = 512;

/* Transmit data size of each transmti buffer descriptor. */

#define TX_BUFFER_SIZE (576)	/* must be divisible by 16 */


void
fec_start_loopback_test(void)
{
    /* Initalize the buffer descriptors. */

    nbuf_init(&MCF5272_fec_priv_data.nbuffer);

    /* Initalize the Ethernet controllder device. */

    MCF5272_fec_init(&MCF5272_fec_netdev);


}

/********************************************************************/
static void
loop_fill_buffers(void)
{
	/* Fill all the buffers in the TX buffer ring with a unique data pattern */
	uint16 index, pattern, i;
    static unsigned char output_buffer[NUM_TXBDS][TX_BUFFER_SIZE];


    uint_t data_length = TX_BUFFER_SIZE;

	for (i = 0; i < NUM_TXBDS; i++)
	{

        MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data = output_buffer[i];
		switch (i % 8)
		{
			/* Load buffers 0 through 3 with a single data patterns */
			case (0):
				memset(&MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[ETH_HDR_SIZE],0x55,data_length);
				break;	
			case (1):
				memset(&MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[ETH_HDR_SIZE],0xAA,data_length);
				break;
			case (2):
				memset(&MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[ETH_HDR_SIZE],0x00,data_length);
				break;
			case (3):
				memset(&MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[ETH_HDR_SIZE],0xFF,data_length);
				break;

			/* Buffer[4]: Load increasing walking ones */
			case (4):
				pattern = 1;
				for (index = 0; index < data_length; index++)
				{
					if (pattern == 0x0100)
						pattern = 0x01;	
					MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[index] = (uint8)pattern;
					pattern <<= 1;
				}
				break;
			
			/* Buffer[5]: Load decreasing walking ones */
			case(5):
				pattern = 0x80;
				for (index = 0; index < data_length; index++)
				{
					if (pattern == 0x00)
						pattern = 0x80;
					MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[index] = (uint8)pattern;
					pattern >>= 1;
				}		
				break;

			/* Buffer[6]: Load "Increment from 0" pattern */
			case (6):
				for (index = 0; index < data_length; index++)
					MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[index] = (uint8) ((index-14)%256);
				break;
			
			/* Buffer[7]: Load "Decrement from 255" pattern */
			case (7):
				for (index = 0; index < data_length; index++)
					MCF5272_fec_priv_data.nbuffer.TxNBUF[i].data[index] = (uint8)(255- ((index-14)%256));
				break;
		}
	}
}

/********************************************************************/
void
loop_handler(void)
{
	
    /*   This is the loop  specific RX  handler called  from the  interrupt */
    /* receive routine.  This routine  gets  bound  to  the  loop  protocol */
    /* (0x0300) by  a call  to nif_bind_protocol()  and is  then called  in */
    /* fec_receive().  This function  simply checks to  make sure that  the */
    /* receive buffer matches the transmit buffer.                          */
	
	int i;

	/* Compare what I received to what I transmitted */
	for (i = 0; i < (data_length + ETH_HDR_SIZE); i++)
	{
		if (TxBuffer[fec_nif->f_rx % NUM_TXBDS].data[i] != pNbuf->data[i])
		{
			/* Increment reception error count */
			fec_nif->f_rx_err++;
		}
	}

	/* Increment reception count */
	fec_nif->f_rx++;

	/* Update progress indicator */
	if (!(fec_nif->f_rx % 200))
	{
		ihash = (ihash + 1) % 4;
		diag_printf("\b%c",hash[ihash]);
	}

	return;
}

#endif


