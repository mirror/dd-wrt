/**
 * @file IxHssAccNpeA_p.h
 * 
 * @author Intel Corporation
 * @date 02-Apr-2002
 *
 * @brief This file contains private dependencies on NPE-A
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/

/**
 * @defgroup IxHssAccNpeA_p IxHssAccNpeA_p
 *
 * @brief Private dependenices of HssAccess on NPE-A
 * 
 * @{
 */

#ifndef IXHSSACCNPEA_P_H
#define IXHSSACCNPEA_P_H


/**
 * Prototypes for interface functions.
 */

/**
 * #defines for function return types, etc.
 */

/* packetised mode defines 
 * Note: The values defined here are bit positional values of 
 *       HSSnP_PIPEm_MODE structure in the NPE
 */
#define IX_HSSACC_NPE_PKT_MODE_HDLC 	     0
#define IX_HSSACC_NPE_PKT_MODE_RAW  	     1
#define IX_HSSACC_NPE_PKT_MODE_56KMODE	     2
#define IX_HSSACC_NPE_PKT_MODE_56KENDIAN_LSB 0
#define IX_HSSACC_NPE_PKT_MODE_56KENDIAN_MSB 4

/* packetised invert mask defines */
#define IX_HSSACC_NPE_PKT_INVMASK 0x000000FF

/* packetised OR mask defines */
#define IX_HSSACC_NPE_PKT_ORMASK_MSB_POLARITY1 0x00000080
#define IX_HSSACC_NPE_PKT_ORMASK_LSB_POLARITY1 0x00000001

/* ChanRx QMQ entry defines */
#define IX_HSSACC_NPE_CHANRXQ_RXOFFSET_MASK   0x000000FF
#define IX_HSSACC_NPE_CHANRXQ_RXOFFSET_OFFSET 0
#define IX_HSSACC_NPE_CHANRXQ_TXOFFSET_MASK   0x0000FF00
#define IX_HSSACC_NPE_CHANRXQ_TXOFFSET_OFFSET 8
#define IX_HSSACC_NPE_CHANRXQ_NUMERRS_MASK    0x00FF0000
#define IX_HSSACC_NPE_CHANRXQ_NUMERRS_OFFSET  16

/* NPE last error register defines */
#define IX_HSSACC_NPE_HSSTX_NO_ERR1     0
#define IX_HSSACC_NPE_HSSTX_FRMSYNC_ERR 1
#define IX_HSSACC_NPE_HSSTX_NO_ERR2     2
#define IX_HSSACC_NPE_HSSTX_OVERRUN_ERR 3
#define IX_HSSACC_NPE_SWTX_NO_ERR       0
#define IX_HSSACC_NPE_SWTX_CHAN_ERR     1
#define IX_HSSACC_NPE_SWTX_PKT_ERR      2
#define IX_HSSACC_NPE_HSSRX_NO_ERR1     0
#define IX_HSSACC_NPE_HSSRX_FRMSYNC_ERR 1
#define IX_HSSACC_NPE_HSSRX_NO_ERR2     2
#define IX_HSSACC_NPE_HSSRX_OVERRUN_ERR 3
#define IX_HSSACC_NPE_SWRX_NO_ERR       0
#define IX_HSSACC_NPE_SWRX_CHAN_ERR     1
#define IX_HSSACC_NPE_SWRX_PKT_ERR      2

/* NPE command related defines */
#define IX_HSSACC_NPE_CMD_ID_OFFSET        24
#define IX_HSSACC_NPE_CMD_ID_MASK          0xFF000000
#define IX_HSSACC_NPE_CMD_HSSPORTID_OFFSET  8
#define IX_HSSACC_NPE_CMD_HSSPORTID_MASK   0x0000FF00
#define IX_HSSACC_NPE_LASTERR_OFFSET       16
#define IX_HSSACC_NPE_LASTERR_MASK         0xFFFF0000
#define IX_HSSACC_NPE_ERRCOUNT_OFFSET       8
#define IX_HSSACC_NPE_ERRCOUNT_MASK        0x0000FF00

/* NPE Packetised related command defines */
#define IX_HSSACC_NPE_PKT_RXCFG_OFFSET     24
#define IX_HSSACC_NPE_PKT_TXCFG_OFFSET     16
#define IX_HSSACC_NPE_PKT_RXSIZEB_OFFSET   16
#define IX_HSSACC_NPE_PKT_RXSIZEW_OFFSET    0
#define IX_HSSACC_NPE_PKT_NUMPIPES_OFFSET  24
#define IX_HSSACC_NPE_PKT_FIFOSIZEW_OFFSET 24
#define IX_HSSACC_NPE_PKT_MODE_OFFSET      24
#define IX_HSSACC_NPE_PKT_INVMASK_OFFSET   16
#define IX_HSSACC_NPE_PKT_ORMASK_OFFSET     8

/* NPE Channelised related command defines */
#define IX_HSSACC_NPE_CHAN_RXSIZEB_OFFSET   16
#define IX_HSSACC_NPE_CHAN_TRIG_OFFSET      24
#define IX_HSSACC_NPE_CHAN_TXBLK1B_OFFSET    0
#define IX_HSSACC_NPE_CHAN_TXBLK1W_OFFSET    8
#define IX_HSSACC_NPE_CHAN_TXBLK2B_OFFSET   16
#define IX_HSSACC_NPE_CHAN_TXBLK2W_OFFSET   24
#define IX_HSSACC_NPE_CHAN_NUMCHANS_OFFSET  24
#define IX_HSSACC_NPE_CHAN_TXBUFSIZE_OFFSET 24

/* NPE Channelised timeslot switching related command defines */
#define IX_HSSACC_NPE_CHAN_SRCTSLOT_OFFSET  24
#define IX_HSSACC_NPE_CHAN_DESTTSLOT_OFFSET  8
#define IX_HSSACC_NPE_CHAN_GCT_BYTE0_OFFSET 24
#define IX_HSSACC_NPE_CHAN_GCT_BYTE1_OFFSET 16
#define IX_HSSACC_NPE_CHAN_GCT_BYTE2_OFFSET  8
#define IX_HSSACC_NPE_CHAN_GCT_BYTE3_OFFSET  0

/* NPE hfifo values */
#define IX_HSSACC_NPE_HFIFO_ONE_BUFFER   0
#define IX_HSSACC_NPE_HFIFO_TWO_BUFFERS  1
#define IX_HSSACC_NPE_HFIFO_FOUR_BUFFERS 2  
#define IX_HSSACC_NPE_HFIFO_UNSUPPORTED  3  

/* NPE last error register related defines */
#define IX_HSSACC_NPE_ERR_RXBUF_OFFSET     13
#define IX_HSSACC_NPE_ERR_SWRXERR_OFFSET   10
#define IX_HSSACC_NPE_ERR_HSSRXERR_OFFSET  8
#define IX_HSSACC_NPE_ERR_TXBUF_OFFSET     5
#define IX_HSSACC_NPE_ERR_SWTXERR_OFFSET   2
#define IX_HSSACC_NPE_ERR_HSSTXERR_OFFSET  0
#define IX_HSSACC_NPE_ERR_RXBUF_MASK       0x6000
#define IX_HSSACC_NPE_ERR_SWRXERR_MASK     0x1C00
#define IX_HSSACC_NPE_ERR_HSSRXERR_MASK    0x0300
#define IX_HSSACC_NPE_ERR_TXBUF_MASK       0x0060
#define IX_HSSACC_NPE_ERR_SWTXERR_MASK     0x001C
#define IX_HSSACC_NPE_ERR_HSSTXERR_MASK    0x0003

#endif /* IXHSSACCNPEA_P_H */

/**
 * @} defgroup IxHssAccNpeA_p
 */
