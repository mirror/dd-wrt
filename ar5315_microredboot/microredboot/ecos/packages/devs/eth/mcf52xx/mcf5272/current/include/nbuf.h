#ifndef _NBUF_H
#define _NBUF_H
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
 * File:		nbuf.h
 * Purpose:		Definitions for Network Buffer Allocation.
 *
 * Notes:		These routines implement a static buffer scheme.
 *				The buffer descriptors are as specified by the
 *				MPC860T/MCF5272 FEC.
 *
 * Modifications:
 *
 */

#include <cyg/hal/drv_api.h>
#include <pkgconf/net_mcf5272_eth_drivers.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/io/eth/eth_drv.h>



/********************************************************************/


typedef unsigned char		uint8;  /*  8 bits */
typedef unsigned short int	uint16; /* 16 bits */
typedef unsigned long int	uint32; /* 32 bits */

typedef signed char			int8;   /*  8 bits */
typedef signed short int	int16;  /* 16 bits */
typedef signed long int		int32;  /* 32 bits */


#define Rx	1
#define Tx	0

/*
 * Buffer sizes in bytes -- The following values were chosen based
 * on TFTP maximum packet sizes.  These sizes may need to be
 * increased to implement other protocols.
 */
#define RX_BUFFER_SIZE (576)	/* must be divisible by 16 */
/* #define TX_BUFFER_SIZE 576 */


/* The this label is not defined, we define it and default it to 256. */
#ifndef CYGPKG_NET_MCF5272_ETH_DRIVERS_RX_NUM_BDS
#define CYGPKG_NET_MCF5272_ETH_DRIVERS_RX_NUM_BDS 256
#endif

/* The this label is not defined, we define it and default it to 256. */
#ifndef CYGPKG_NET_MCF5272_ETH_DRIVERS_TX_NUM_BDS
#define CYGPKG_NET_MCF5272_ETH_DRIVERS_TX_NUM_BDS 256
#endif


/* Number of Receive and Transmit Buffers and Buffer Descriptors */
#define NUM_RXBDS (CYGPKG_NET_MCF5272_ETH_DRIVERS_RX_NUM_BDS)
#define NUM_TXBDS (CYGPKG_NET_MCF5272_ETH_DRIVERS_TX_NUM_BDS)



/*
 * Buffer Descriptor Format -- must be aligned on 4-byte boundary
 * but a 16-byte boundary is recommended.  However, we cannot pack
 * them on 16-byte boundaries as this will add 8-bytes of padding
 * between structures and the FEC of the MPC860T/MCF5272 will choke.
 */
typedef struct NBUF
{
	uint16 status;	/* control and status */
	uint16 length;	/* transfer length */
	uint8  *data;	/* buffer address */
} __attribute__ ((packed, aligned))NBUF;


/* Defines the tx key type. */
typedef enum tx_key_type_t
{
    TX_KEY_ECOS, /* eCos key */
    TX_KEY_USER  /* user key */
}tx_key_type_t;

typedef struct tx_keys_t
{
    unsigned long tx_key;       /* The transmit key that eCos gives us. */
    uint_t  tx_buf_index; /* Index to the TxNBUF where the last Buffer Descriptor of the frame. */
    int   num_dbufs;    /* The number transmit buffer allocated for the frame. */
    uint_t  start_index;  /* Index of the bd */
    int_t   pk_len;
    tx_key_type_t key_type;     /* The type of the key. */

}tx_keys_t;

typedef struct buf_info_t
{

    /*   Buffer descriptor indexes                                          */

    uint_t iTxbd;
    uint_t iRxbd;

    /*   Queue for the transmit keys                                        */

    #define TX_KEY_QUEUE_SIZE (NUM_TXBDS+1)
    tx_keys_t tx_keys_queue[TX_KEY_QUEUE_SIZE];

    /*   Rear and front pointer for the keys queue.                         */

    uint_t tq_rear;
    uint_t tq_front;

    /*   Number transmit descriptor buffers busy.                           */

    volatile int  num_busy_bd;

    /*   The maximum number of buffer decriptor used.                       */

    volatile int  max_num_busy_bd;

    /* Buffer Descriptors */
    NBUF  ntxbuf[NUM_TXBDS+2];
    NBUF  nrxbuf[NUM_RXBDS+2];

    NBUF* RxNBUF;
    NBUF* TxNBUF;

    /* Buffers */
    //TXB TxBuffer[NUM_TXBDS];
    u8_t RxBuffer[(NUM_RXBDS+2)*RX_BUFFER_SIZE];


}buf_info_t;


/********************************************************************/

/*
 * Bit level Buffer Descriptor definitions
 */

#define TX_BD_R			0x8000
#define TX_BD_TO1		0x4000
#define TX_BD_INUSE     TX_BD_TO1
#define TX_BD_W			0x2000
#define TX_BD_TO2		0x1000
#define TX_BD_L			0x0800
#define TX_BD_TC		0x0400
#define TX_BD_DEF		0x0200
#define TX_BD_HB		0x0100
#define TX_BD_LC		0x0080
#define TX_BD_RL		0x0040
#define TX_BD_UN		0x0002
#define TX_BD_CSL		0x0001

#define RX_BD_E			0x8000
#define RX_BD_R01		0x4000
#define RX_BD_W			0x2000
#define RX_BD_R02		0x1000
#define RX_BD_L			0x0800
#define RX_BD_M			0x0100
#define RX_BD_BC		0x0080
#define RX_BD_MC		0x0040
#define RX_BD_LG		0x0020
#define RX_BD_NO		0x0010
#define RX_BD_SH		0x0008
#define RX_BD_CR		0x0004
#define RX_BD_OV		0x0002
#define RX_BD_TR		0x0001

/*******************************************************************/

/*
 * Functions to manipulate the network buffers.
 */

void nbuf_init (buf_info_t* pBuf);


/********************************************************************/
inline static
uint32
nbuf_get_start(buf_info_t* pBuf, uint8 direction)
{
	/*
	 * Return the address of the first buffer descriptor in the ring.
	 * This routine is needed by the FEC of the MPC860T and MCF5272
	 * in order to write the Rx/Tx descriptor ring start registers
	 */
	switch (direction){
	case Rx:
		return (uint32)pBuf->RxNBUF;
    default:
	case Tx:
		return (uint32)pBuf->TxNBUF;
	}
}






/******************************************************************************
 nbuf_rx_get_next() - Retrieve the next  receive buffer descriptor.

 INPUT:
    pBuf - Pointer to the buffer info.

RETURN:
    Returns the pointer to the next receive buffer descriptor.
*/

inline static
NBUF *
nbuf_rx_get_next(buf_info_t* pBuf)
{
    NBUF* pBd = &pBuf->RxNBUF[pBuf->iRxbd];

	/* Check to see if the ring of BDs is full */
	if (pBd->status & RX_BD_E)
    {
   		return NULL;
    }

	/* increment the circular index */
	pBuf->iRxbd = (pBuf->iRxbd + 1) % NUM_RXBDS;

	return pBd;
}


/* Return the pointer to the buffer descriptor based on the index */
inline static
NBUF*
nbuf_rx_get(buf_info_t* pBuf, uint_t index)
{
    return(&pBuf->RxNBUF[index]);
}

/********************************************************************
  Release the buffer descrip so that it can be use
  to receive the enxt packet.

  INPUT:
    pNbuf - pointer to the buffer descriptor.
*/

inline static
void
nbuf_rx_release (NBUF* pNbuf)
{
	
        /* Mark the buffer as empty and not in use */
	pNbuf->status |= RX_BD_E;

}


/* Return the current rx buffer descriptor index */
inline static uint_t
nbuf_rx_get_index(buf_info_t* pBuf)
{
    return  pBuf->iRxbd;
}

/****************************************************************
 This function checks the EMPTY bit of the next Rx buffer to be
 allocated. If the EMPTY bit is cleared, then the next buffer in
 the ring has been filled by the FEC and has not already been
 allocated and passed up the stack. In this case, the next buffer
 in the ring is ready to be allocated. Otherwise, the  buffer is
 either empty or not empty but still in use by a higher level
 protocol. The FEC receive routine uses this function to determine
 if multiple buffers where filled by the FEC during a single
 interrupt event.
 ****************************************************************/
inline static
uint_t
nbuf_rx_next_ready(buf_info_t* pBuf)
{

	return ( !(pBuf->RxNBUF[pBuf->iRxbd].status & RX_BD_E));
}




/******************************************************************************
   nbuf_rx_release_pkt() - Release the buffer descriptors of the next packet.

   INPUT:
        pointer to the buffer info.
*/
inline static
void nbuf_rx_release_pkt(buf_info_t* pBuf)
{

     NBUF* pNbuf;

     /* Indicates whether the last buffer descriptor is encountered. */
     cyg_bool_t last = false;

     do
     {
         /* Get the buffer descriptor. */
         pNbuf = nbuf_rx_get_next(pBuf);

         /*   Stop when there is a packet truncated bit is set or it is the */
         /* last buffer descriptor of the packet.                           */

         if ((!(pNbuf->status & RX_BD_E) && pNbuf->status & RX_BD_TR) ||
             pNbuf->status & RX_BD_L)
         {
             last = true;
         }

         /* Release the buffer descriptor. */
         nbuf_rx_release(pNbuf);

     }while(last == false);

}



/******************************************************************************
   nbuf_rx_release_good_pkt() - Release the buffer descriptors for
                                good received packets.
                                Note call this function
                                only when there is valid received
                                buffer descriptors.

   INPUT:
        pBuf - pointer to the buffer info.
*/
inline static
void nbuf_rx_release_good_pkt(buf_info_t* pBuf)
{

     u16_t status;

     do
     {

         /*   Read the status bits.  If the RX_BD_L is set we terminate the */
         /* loop.                                                           */

         status = pBuf->RxNBUF[pBuf->iRxbd].status;

         /*   Release the buffer descriptor so that it could reused.        */

         nbuf_rx_release(&pBuf->RxNBUF[pBuf->iRxbd]);

         /*   Advance the index to the next buffer descriptor.              */

         pBuf->iRxbd = (pBuf->iRxbd + 1) % NUM_RXBDS;


     }while(!(status & RX_BD_L));

}


/******************************************************************************
  nbuf_tx_release() - Set the buffer descriptor ready for use to transmit
                      the next packet.

   INPUT:
        pNbuf - Pointer to the transmit buffer descriptor.
*/
inline static
void
nbuf_tx_release (NBUF* pNbuf)
{

    /*   Clear  the  TX_BD_INUSE  to  indicate   that  we  have  read   the */
    /* transmitted buffer.                                                  */

    pNbuf->status &= ~(TX_BD_INUSE | TX_BD_R | TX_BD_L | TX_BD_TC);

}

/* Return a nozero value  if the buffer is full. Otherwise, it returns a
   zero value.
*/
inline static
int_t nbuf_tx_full(buf_info_t* pBuf)
{
    return (pBuf->TxNBUF[pBuf->iTxbd].status & TX_BD_R);
}


/* Return the pointer to the transmit buffer descriptor. */
inline static
NBUF*
nbuf_tx_get(buf_info_t* pBuf, int_t index)
{
    return(&pBuf->TxNBUF[index]);
}

/******************************************************************************
 nbuf_peek_tx_key() - Peek whether there is any
                      pending packets that are still in transmisison.

 INPUT:
    pBuf - Pointer to the buffer structure.

 OUTPUT:
    index - The index to the oldest pending packet in the queue.

 RETURN:
    Returns the index to the oldest pending packet in the queue. Otherwise,
    it returns -1.

*/
inline static
int_t nbuf_peek_tx_key(buf_info_t* pBuf)
{
    if (pBuf->tq_rear == pBuf->tq_front)
    {
        return -1; /* No pending transmit buffers. */
    }
    return pBuf->tx_keys_queue[pBuf->tq_front].tx_buf_index;
}

/******************************************************************************
 nbuf_peek_bd_ahead() - Returns the pointer of the to the last buffer
                        descriptor of the 2nd. pending transmit frame.

 INPUT:
    pBuf - Pointer to the buffer structure.


 RETURN:
    Returns the pointer of the to the last buffer
    descriptor of the 2nd. pending transmit frame. If there is not 2nd.
    pending frame, it returns NULL.


*/
inline static
NBUF* nbuf_peek_bd_ahead(buf_info_t* pBuf)
{
    uint_t ahead_front = (pBuf->tq_front + 1) % TX_KEY_QUEUE_SIZE;

    if (pBuf->tq_rear == pBuf->tq_front ||
        ahead_front == pBuf->tq_rear)
    {
        return NULL; /* No pending transmit buffers. */
    }

    return &pBuf->TxNBUF[pBuf->tx_keys_queue[ahead_front].tx_buf_index];
}



/******************************************************************************
 nbuf_enq_tx_key() - Enqueue and deuque transmit key queue.
   Enqueue the transmit key. The  buf_index is the index to the transmit buffer deiscriptor
   table of the last buffer descriptor for the frame and the transmit packet
   type.
*/
inline static
void nbuf_enq_tx_key(buf_info_t* pBuf,
                     uint_t buf_index,
                     unsigned long txkey,
                     uint_t start_index,
                     int num_bd,
                     int_t total_len,
                     tx_key_type_t key_type)
{
    tx_keys_t* p_keys = &pBuf->tx_keys_queue[pBuf->tq_rear];
    /*   Assign the transmit packet information to the transmit key queue.  */

    /*   NOTE: We don't check for  the full condition because the  transmit */
    /* key queue size is as big as the number of buffer descriptors.        */

    p_keys->tx_key = txkey;
    p_keys->tx_buf_index  = buf_index;
    p_keys->num_dbufs = num_bd;
    p_keys->pk_len = total_len;
    p_keys->start_index = start_index;
    p_keys->key_type = key_type;

    /*   Advance the transmit queue pointer to the next entry.              */

    pBuf->tq_rear = (pBuf->tq_rear + 1) % TX_KEY_QUEUE_SIZE;

 }


/******************************************************************************
 nbuf_deq_tx_key() - Dequeue the transmit packet from the transmit key queue.
                     Assume that the queue is not empty.


 INPUT:
    pBuf - Pointer to the buffer structure.

 OUTPUT:
    key - The  key the transmit information.
*/
inline static
void nbuf_deq_tx_key(buf_info_t* pBuf, tx_keys_t* key)
{

    CYG_ASSERT(pBuf->tq_rear != pBuf->tq_front, "nbuf_deq_tx_key: empty");
    *key = pBuf->tx_keys_queue[pBuf->tq_front];
    pBuf->tq_front = (pBuf->tq_front + 1) %  TX_KEY_QUEUE_SIZE;

}

/*******************************************************************************
 nbuf_tx_dump() - Dump the transmit buffer information.

 INPUT:
    pBuf - pointer to the buffer info.
*/
inline static
void nbuf_tx_dump(buf_info_t* pBuf)
{

    tx_keys_t key;

    diag_printf("Current index to ring buffer: %d\n", pBuf->iTxbd);
    diag_printf("Address to the BD:            %08X\n",
                &pBuf->TxNBUF[pBuf->iTxbd]);

    diag_printf("BD status:            %04X\n", pBuf->TxNBUF[pBuf->iTxbd].status);
    diag_printf("TX Queue rear index:  %d\n", pBuf->tq_rear);
    diag_printf("TX Queue front index: %d\n", pBuf->tq_front);
    diag_printf("Number of busy BDs:   %d\n", pBuf->num_busy_bd);

    diag_printf("Dump Transmit Queue\n");
    diag_printf("===================\n");

    while(nbuf_peek_tx_key(pBuf) != -1)
    {
        nbuf_deq_tx_key(pBuf, &key);
        diag_printf("Number of BDs: %d\n", key.num_dbufs);
        diag_printf("Frame length:  %d\n", key.pk_len);
        diag_printf("Begin index to ring bufer: %d\n", key.start_index);
        diag_printf("BD address: %08X\n", &pBuf->TxNBUF[key.tx_buf_index]);
        diag_printf("status: %04X\n", pBuf->TxNBUF[key.tx_buf_index].status);

        diag_printf("End index to ring buffer:  %d\n", key.tx_buf_index);
        diag_printf("Key Info: %08X\n", key.tx_key);
        diag_printf("Key type: %d\n", key.key_type);
        diag_printf("\n");

    }
    diag_printf("===================\n");


}


/********************************************************************
 nbuf_tx_allocate() - Alocate transmit buffer descriptor.

 INPUT:
    pBuf - pointer to the buffer info.

 OUTPUT:
    index - index to the buffer descriptor ring buffer that it returns. This
            value is invalid if this funtion returns a NULL.

 RETURN:
    Returns the pointer to the buffer descriptor. If the ring buffer
    descriptor is full, it returns NULL.

 */
inline static
NBUF *
nbuf_tx_allocate (buf_info_t* pBuf)
{
    NBUF* pBd = &pBuf->TxNBUF[pBuf->iTxbd];

    /*   If the ring buffer is full, then return a NULL.                    */

	if (pBd->status & TX_BD_INUSE)
    {
        nbuf_tx_dump(pBuf);
        return NULL;
    }
	
    /*   Make sure that the  buffer descriptor is still  not in use by  the */
    /* FEC .                                                                */

    CYG_ASSERT(!(pBd->status & TX_BD_R),
               "Buffer descriptor allocated still in use");

    /*   Set the buffer  to be  in used  so that  we can  check the  status */
    /* before we resuse it to send another packet.                          */

    pBd->status |= TX_BD_INUSE;


	/* increment the circular index */
	pBuf->iTxbd = ((pBuf->iTxbd + 1) % NUM_TXBDS);

	return pBd;
}




/*****************************************************************************/

#endif 	/* _NBUF_H */
