/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __SYN_DMA_DESC__
#define __SYN_DMA_DESC__

/**********************************************************
 * DMA Engine descriptors
 **********************************************************/
/*
******Enhanced Descritpor structure to support 8K buffer per buffer *******

dma_rx_base_addr = 0x000C,	CSR3 - Receive Descriptor list base address
dma_rx_base_addr is the pointer to the first Rx Descriptors.
The Descriptor format in Little endian with a 32 bit Data bus is as shown below.

Similarly
dma_tx_base_addr     = 0x0010,  CSR4 - Transmit Descriptor list base address
dma_tx_base_addr is the pointer to the first Tx Descriptors.
The Descriptor format in Little endian with a 32 bit Data bus is as shown below.
	-------------------------------------------------------------------------
 RDES0	|OWN (31)|								|
	-------------------------------------------------------------------------
 RDES1	| Ctrl | Res | Byte Count Buffer 2 | Ctrl | Res | Byte Count Buffer 1	|
	-------------------------------------------------------------------------
 RDES2	|	Buffer 1 Address						|
	-------------------------------------------------------------------------
 RDES3	|	Buffer 2 Address / Next Descriptor Address			|
	-------------------------------------------------------------------------
 RDES4	|	Extended Status							|
	-------------------------------------------------------------------------
 RDES5	|	Reserved							|
	-------------------------------------------------------------------------
 RDES6	|	Receive Timestamp Low						|
	-------------------------------------------------------------------------
 RDES7	|	Receive Timestamp High						|
	-------------------------------------------------------------------------

	------------------------------------------------------------------------
 TDES0	|OWN (31)| Ctrl | Res | Ctrl | Res | Status				|
	------------------------------------------------------------------------
 TDES1	| Res | Byte Count Buffer 2 | Res |	Byte Count Buffer 1		|
	------------------------------------------------------------------------
 TDES2	|	Buffer 1 Address						|
	------------------------------------------------------------------------
 TDES3	|	Buffer 2 Address / Next Descriptor Address			|
	------------------------------------------------------------------------
 TDES4	|	Reserved							|
	------------------------------------------------------------------------
 TDES5	|	Reserved							|
	------------------------------------------------------------------------
 TDES6	|	Transmit Timestamp Low						|
	------------------------------------------------------------------------
 TDES7	|	Transmit Timestamp Higher					|
	------------------------------------------------------------------------
*/

/*
 * syn_dma_desc_mode
 *	GMAC DMA descriptors mode
 */
enum syn_dma_desc_mode {
	RINGMODE = 0x00000001,
	CHAINMODE = 0x00000002,
};

/*
 * syn_dma_desc_status
 *	status word of DMA descriptor
 */
enum syn_dma_desc_status {
	DESC_OWN_BY_DMA = 0x80000000,		/* (OWN)Descriptor is
						   owned by DMA engine */
	DESC_RX_DA_FILTER_FAIL = 0x40000000,	/* (AFM)Rx - DA Filter
						   Fail for the rx frame */
	DESC_RX_FRAME_LENGTH_MASK = 0x3FFF0000,	/* (FL)Receive descriptor
						   frame length */
	DESC_RX_FRAME_LENGTH_SHIFT = 16,
	DESC_RX_ERROR = 0x00008000,		/* (ES)Error summary bit
						   - OR of the  following bits:
						   DE || OE || IPC || GF || LC  || RWT
						   || RE || CE */
	DESC_RX_TRUNCATED = 0x00004000,		/* (DE)Rx - no more descriptors
						   for receive frame */
	DESC_SA_FILTER_FAIL = 0x00002000,	/* (SAF)Rx - SA Filter Fail for
						   the received frame */
	DESC_RX_LENGTH_ERROR = 0x00001000,	/* (LE)Rx - frm size not
						   matching with len field */
	DESC_RX_OVERFLOW = 0x00000800,		/* (OE)Rx - frm was damaged due
						   to buffer overflow */
	DESC_RX_VLAN_TAG = 0x00000400,		/* (VLAN)Rx - received frame
						   is a VLAN frame */
	DESC_RX_FIRST = 0x00000200,		/* (FS)Rx - first
						   descriptor of the frame */
	DESC_RX_LAST = 0x00000100,		/* (LS)Rx - last
						   descriptor of the frame */
	DESC_RX_LONG_FRAME = 0x00000080,	/* (Giant Frame)Rx - frame is
						   longer than 1518/1522 */
	DESC_RX_COLLISION = 0x00000040,		/* (LC)Rx - late collision
						   occurred during reception */
	DESC_RX_FRAME_ETHER = 0x00000020,	/* (FT)Rx - Frame type - Ether,
						   otherwise 802.3 */
	DESC_RX_WATCHDOG = 0x00000010,		/* (RWT)Rx - watchdog timer
						   expired during reception */
	DESC_RX_MII_ERROR = 0x00000008,		/* (RE)Rx - error reported
						   by MII interface */
	DESC_RX_DRIBBLING = 0x00000004,		/* (DE)Rx - frame contains non
						   int multiple of 8 bits */
	DESC_RX_CRC = 0x00000002,		/* (CE)Rx - CRC error */
	DESC_RX_EXT_STS = 0x00000001,		/* Extended Status Available
						   in RDES4 */
	DESC_TX_ERROR = 0x00008000,		/* (ES)Error summary Bits */
	DESC_TX_INT_ENABLE = 0x40000000,	/* (IC)Tx - interrupt on
						   completion */
	DESC_TX_LAST = 0x20000000,		/* (LS)Tx - Last segment of the
						   frame */
	DESC_TX_FIRST = 0x10000000,		/* (FS)Tx - First segment of the
						   frame */
	DESC_TX_DISABLE_CRC = 0x08000000,	/* (DC)Tx - Add CRC disabled
						   (first segment only) */
	DESC_TX_DISABLE_PADD = 0x04000000,	/* (DP)disable padding,
						   added by - reyaz */
	DESC_TX_CIS_MASK = 0x00c00000,		/* Tx checksum offloading
						   control mask */
	DESC_TX_CIS_BYPASS = 0x00000000,	/* Checksum bypass */
	DESC_TX_CIS_IPV4_HDR_CS = 0x00400000,	/* IPv4 header checksum */
	DESC_TX_CIS_TCP_ONLY_CS = 0x00800000,	/* TCP/UDP/ICMP checksum.
						   Pseudo header  checksum
						   is assumed to be present */
	DESC_TX_CIS_TCP_PSEUDO_CS = 0x00c00000,	/* TCP/UDP/ICMP checksum fully
						   in hardware  including
						   pseudo header */
	DESC_TX_DESC_END_OF_RING = 0x00200000,	/* (TER)End of descriptor ring */
	DESC_TX_DESC_CHAIN = 0x00100000,	/* (TCH)Second buffer address
						   is chain address */
	DESC_RX_CHK_BIT0 = 0x00000001,		/* Rx Payload Checksum Error */
	DESC_RX_CHK_BIT7 = 0x00000080,		/* (IPC CS ERROR)Rx - Ipv4
						   header checksum error */
	DESC_RX_CHK_BIT5 = 0x00000020,		/* (FT)Rx - Frame type - Ether,
						   otherwise 802.3 */
	DESC_RX_TS_AVAIL = 0x00000080,		/* Time stamp available */
	DESC_RX_FRAME_TYPE = 0x00000020,	/* (FT)Rx - Frame type - Ether,
						   otherwise 802.3 */
	DESC_TX_IPV4_CHK_ERROR = 0x00010000,	/* (IHE) Tx Ip header error */
	DESC_TX_TIMEOUT = 0x00004000,		/* (JT)Tx - Transmit
						   jabber timeout */
	DESC_TX_FRAME_FLUSHED = 0x00002000,	/* (FF)Tx - DMA/MTL flushed
						   the frame  due to SW flush */
	DESC_TX_PAY_CHK_ERROR = 0x00001000,	/* (PCE) Tx Payload checksum
						   Error */
	DESC_TX_LOST_CARRIER = 0x00000800,	/* (LC)Tx - carrier lost
						   during tramsmission */
	DESC_TX_NO_CARRIER = 0x00000400,	/* (NC)Tx - no carrier signal
						   from the tranceiver */
	DESC_TX_LATE_COLLISION = 0x00000200,	/* (LC)Tx - transmission aborted
						   due to collision */
	DESC_TX_EXC_COLLISIONS = 0x00000100,	/* (EC)Tx - transmission aborted
						   after 16 collisions */
	DESC_TX_VLAN_FRAME = 0x00000080,	/* (VF)Tx - VLAN-type frame */
	DESC_TX_COLL_MASK = 0x00000078,		/* (CC)Tx - Collision count */
	DESC_TX_COLL_SHIFT = 3,
	DESC_TX_EXC_DEFERRAL = 0x00000004,	/* (ED)Tx - excessive deferral */
	DESC_TX_UNDERFLOW = 0x00000002,		/* (UF)Tx - late data arrival
						   from the memory */
	DESC_TX_DEFERRED = 0x00000001,		/* (DB)Tx - frame
						   transmision deferred */

	/*
	 * This explains the RDES1/TDES1 bits layout
	 *             ------------------------------------------------------
	 * RDES1/TDES1 | Control Bits | Byte Count Buf 2 | Byte Count Buf 1 |
	 *             ------------------------------------------------------
	 */

	/* dma_descriptor_length */	/* length word of DMA descriptor */
	DESC_RX_DIS_INT_COMPL = 0x80000000,	/* (Disable Rx int on completion) */
	DESC_RX_DESC_END_OF_RING = 0x00008000,	/* (RER)End of descriptor ring */
	DESC_RX_DESC_CHAIN = 0x00004000,	/* (RCH)Second buffer address
						    is chain address */
	DESC_SIZE2_MASK = 0x1FFF0000,		/* (RBS2/TBS2) Buffer 2 size */
	DESC_SIZE2_SHIFT = 16,
	DESC_SIZE1_MASK = 0x00001FFF,		/* (RBS1/TBS1) Buffer 1 size */
	DESC_SIZE1_SHIFT = 0,

	/*
	 * This explains the RDES4 Extended Status bits layout
	 *              --------------------------------------------------------
	 *   RDES4      |                 Extended Status                      |
	 *              --------------------------------------------------------
	 */
	DESC_RX_TS_DROPPED = 0x00004000,	/* PTP snapshot available */
	DESC_RX_PTP_VER = 0x00002000,		/* When set indicates IEEE1584
						   Version 2 (else Ver1) */
	DESC_RX_PTP_FRAME_TYPE = 0x00001000,	/* PTP frame type Indicates PTP
						   sent over ethernet */
	DESC_RX_PTP_MESSAGE_TYPE = 0x00000F00,	/* Message Type */
	DESC_RX_PTP_NO = 0x00000000,		/* 0000 => No PTP message rcvd */
	DESC_RX_PTP_SYNC = 0x00000100,		/* 0001 => Sync (all clock
						   types) received */
	DESC_RX_PTP_FOLLOW_UP = 0x00000200,	/* 0010 => Follow_Up (all clock
						   types) received */
	DESC_RX_PTP_DELAY_REQ = 0x00000300,	/* 0011 => Delay_Req (all clock
						   types) received */
	DESC_RX_PTP_DELAY_RESP = 0x00000400,	/* 0100 => Delay_Resp (all clock
						   types) received */
	DESC_RX_PTP_PDELAY_REQ = 0x00000500,	/* 0101 => Pdelay_Req (in P
						   to P tras clk)  or Announce
						   in Ord and Bound clk */
	DESC_RX_PTP_PDELAY_RESP = 0x00000600,	/* 0110 => Pdealy_Resp(in P to
						   P trans clk) or Management in
						   Ord and Bound clk */
	DESC_RX_PTP_PDELAY_RESP_FP = 0x00000700,/* 0111 => Pdelay_Resp_Follow_Up
						   (in P to P trans clk) or
						   Signaling in Ord and Bound
						   clk */
	DESC_RX_PTP_IPV6 = 0x00000080,		/* Received Packet is in IPV6 */
	DESC_RX_PTP_IPV4 = 0x00000040,		/* Received Packet is in IPV4 */
	DESC_RX_CHK_SUM_BYPASS = 0x00000020,	/* When set indicates checksum
						   offload engine is bypassed */
	DESC_RX_IP_PAYLOAD_ERROR = 0x00000010,	/* When set indicates 16bit IP
						   payload CS is in error */
	DESC_RX_IP_HEADER_ERROR = 0x00000008,	/* When set indicates 16bit IPV4
						   hdr CS is err or IP datagram
						   version is not consistent
						   with Ethernet type value */
	DESC_RX_IP_PAYLOAD_TYPE = 0x00000007,	/* Indicate the type of payload
						   encapsulated in IPdatagram
						   processed by COE (Rx) */
	DESC_RX_IP_PAYLOAD_UNKNOWN = 0x00000000,/* Unknown or didnot process
						   IP payload */
	DESC_RX_IP_PAYLOAD_UDP = 0x00000001,	/* UDP */
	DESC_RX_IP_PAYLOAD_TCP = 0x00000002,	/* TCP */
	DESC_RX_IP_PAYLOAD_ICMP = 0x00000003,	/* ICMP */
};

/*
 * dma_desc_rx
 *	Rx DMA Descriptor Structure
 *
 * Enhanced descriptor format for receive.
 */
struct dma_desc_rx {
	uint32_t status;	/* Status */
	uint32_t length;	/* Buffer 1  and Buffer 2 length */
	uint32_t buffer1;	/* Network Buffer 1 pointer (DMA-able) */
	uint32_t buffer2;	/* Network Buffer 2 pointer (DMA-able) */
	/* This data below is used only by driver */
	uint32_t extstatus;	/* Extended status of a Rx Descriptor */
	uint32_t reserved1;	/* Reserved word */
	uint32_t timestamplow;	/* Lower 32 bits of the 64
				   bit timestamp value */
	uint32_t timestamphigh;	/* Higher 32 bits of the 64
					   bit timestamp value */
	uint32_t padding[8];	/* Pad 32 byte to align to 64B cacheline size */
};

/*
 * dma_desc_tx
 *	Tx DMA Descriptor Structure
 *
 * Enhanced descriptor format for transmit.
 */
struct dma_desc_tx {
	uint32_t status;	/* Status */
	uint32_t length;	/* Buffer 1  and Buffer 2 length */
	uint32_t buffer1;	/* Network Buffer 1 pointer (DMA-able) */
	uint32_t buffer2;	/* Network Buffer 2 pointer (DMA-able) */
	uint32_t reserved1;	/* Reserved word */
	uint32_t reserved2;	/* Reserved word */
	uint32_t timestamplow;	/* Lower 32 bits of the 64
				   bit timestamp value */
	uint32_t timestamphigh;	/* Higher 32 bits of the 64
					   bit timestamp value */
	uint32_t padding[8];	/* Pad 32 byte to align to 64B cacheline size */
};

/*
 * syn_dp_gmac_tx_checksum_offload_tcp_pseudo()
 *	The checksum offload engine is enabled to do complete checksum computation.
 */
static inline void syn_dp_gmac_tx_checksum_offload_tcp_pseudo(struct dma_desc_tx *desc)
{
	desc->status = ((desc->status & (~DESC_TX_CIS_MASK)) | DESC_TX_CIS_TCP_PSEUDO_CS);
}

/*
 * syn_dp_gmac_tx_desc_init_ring()
 *	Initialize the tx descriptors for ring mode operation.
 */
static inline void syn_dp_gmac_tx_desc_init_ring(struct dma_desc_tx *desc, uint32_t no_of_desc)
{
	struct dma_desc_tx *last_desc = desc + no_of_desc - 1;
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc_tx));
	last_desc->status = DESC_TX_DESC_END_OF_RING;
}

/*
 * syn_dp_gmac_rx_desc_init_ring()
 *	Initialize the rx descriptors for ring mode operation.
 */
static inline void syn_dp_gmac_rx_desc_init_ring(struct dma_desc_rx *desc, uint32_t no_of_desc)
{
	struct dma_desc_rx *last_desc = desc + no_of_desc - 1;
	memset(desc, 0, no_of_desc * sizeof(struct dma_desc_rx));
	last_desc->length = DESC_RX_DESC_END_OF_RING;
}

/*
 * syn_dp_gmac_is_rx_desc_linear_and_valid()
 *	Checks whether the rx descriptor is linear and valid.
 */
static inline bool syn_dp_gmac_is_rx_desc_linear_and_valid(uint32_t status, uint32_t extstatus)
{
	return (((status & (DESC_RX_ERROR | DESC_RX_FIRST | DESC_RX_LAST)) == (DESC_RX_FIRST | DESC_RX_LAST)) &&
		!(extstatus & (DESC_RX_IP_HEADER_ERROR | DESC_RX_IP_PAYLOAD_ERROR)));
}

/*
 * syn_dp_gmac_is_rx_desc_valid()
 * 	Check if Rx descriptor is valid with no error.
 */
static inline bool syn_dp_gmac_is_rx_desc_valid(uint32_t status, uint32_t extstatus)
{
	return (!(status & DESC_RX_ERROR) && !(extstatus & (DESC_RX_IP_HEADER_ERROR | DESC_RX_IP_PAYLOAD_ERROR)));
}

/*
 * syn_dp_gmac_get_rx_desc_frame_length()
 *	Returns the byte length of received frame including CRC.
 */
static inline uint32_t syn_dp_gmac_get_rx_desc_frame_length(uint32_t status)
{
	return (status & DESC_RX_FRAME_LENGTH_MASK) >> DESC_RX_FRAME_LENGTH_SHIFT;
}

/*
 * syn_dp_gmac_is_rx_desc_owned_by_dma()
 *	Checks whether the Rx descriptor is owned by DMA.
 */
static inline bool syn_dp_gmac_is_rx_desc_owned_by_dma(uint32_t status)
{
	return (status & DESC_OWN_BY_DMA) == DESC_OWN_BY_DMA;
}

/*
 * syn_dp_gmac_is_tx_desc_owned_by_dma()
 *	Checks whether the Tx descriptor is owned by DMA.
 */
static inline bool syn_dp_gmac_is_tx_desc_owned_by_dma(uint32_t status)
{
	return (status & DESC_OWN_BY_DMA) == DESC_OWN_BY_DMA;
}

/*
 * syn_dp_gmac_is_rx_desc_empty()
 *	Checks whether the descriptor is empty.
 */
static inline bool syn_dp_gmac_is_rx_desc_empty(struct dma_desc_rx *desc)
{
	/*
	 * If length of both buffer1 & buffer2 are zero then desc is empty
	 */
	return (desc->length & DESC_SIZE1_MASK) == 0;
}

/*
 * syn_dp_gmac_is_tx_desc_empty()
 *	Checks whether the descriptor is empty.
 */
static inline bool syn_dp_gmac_is_tx_desc_empty(struct dma_desc_tx *desc)
{
	/*
	 * If length of both buffer1 & buffer2 are zero then desc is empty
	 */
	return (desc->length & DESC_SIZE1_MASK) == 0;
}

/*
 * syn_dp_gmac_get_tx_collision_count()
 *	Gives the transmission collision count.
 */
static inline uint32_t syn_dp_gmac_get_tx_collision_count(uint32_t status)
{
	return (status & DESC_TX_COLL_MASK) >> DESC_TX_COLL_SHIFT;
}

#endif /*  __SYN_DMA_DESC__ */
