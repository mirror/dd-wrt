/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
	used to endorse or promote products derived from this software without
	specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#ifndef __INC_MV_TSU_H__
#define __INC_MV_TSU_H__

#include "ts/mvTsuRegs.h"
#include "mvCommon.h"

/********************************/
/* General enums and structures */
/********************************/

#define MV_TSU_NUM_PORTS			(2)

#define MV_TSU_STATUS_CONN_ERROR		(0x1)
#define MV_TSU_STATUS_OVFL_ERROR		(0x2)
#define MV_TSU_STATUS_TSIF_ERROR		(0x4)

#define TSU_DONE_STATUS_ENTRY_SIZE		(4)
#define TSU_INPUT_DESC_ENTRY_SIZE		(4)
#define TSU_OUTPUT_DESC_ENTRY_SIZE		(8)
#define TSU_MODE1_OUT_TMS_SIZE			(8)
#define MV_TSU_MAX_IN_QUEUE_LEN			(1024)
#define MV_TSU_MAX_OUT_QUEUE_LEN		(512)
#define MV_TSU_MAX_DONEQ_LEN			(1024)
#define TSU_DMA_ALIGN				(32)

#define MV_TSU_TX_CLOCK_EXTERNAL		(0xFFFFFFFF)

typedef enum
{
	TSU_CORE_CLK_83_MHZ = 0,
	TSU_CORE_CLK_71_MHZ,
	TSU_CORE_CLK_91_MHZ,
	TSU_CORE_CLK_100_MHZ
}MV_TSU_CORE_CLOCK;

typedef enum
{
	TSU_MODE_SERIAL,
	TSU_MODE_PARALLEL
}MV_TSU_PORTS_MODE;

typedef enum
{
	TSU_SIGNAL_KEEP_DEF,		/* Keep default.	*/
	TSU_SIGNAL_DIS,
	TSU_SIGNAL_EN_ACT_LOW,
	TSU_SIGNAL_EN_ACT_HIGH
}MV_TSU_SIGNAL_MODE;


typedef enum
{
	TSU_SIGNAL_EDGE_KEEP_DEF,	/* Keep default.	*/
	TSU_SIGNAL_EDGE_FALL,
	TSU_SIGNAL_EDGE_RISE
}MV_TSU_SIGNAL_EDGE;


typedef enum
{
	TSU_PORT_INPUT,
	TSU_PORT_OUTPUT
}MV_TSU_PORT_DIRECTION;


typedef struct
{
	MV_TSU_SIGNAL_MODE	tsSync;
	MV_TSU_SIGNAL_MODE	tsValid;
	MV_TSU_SIGNAL_MODE	tsError;
	MV_TSU_SIGNAL_EDGE	tsDataEdge;
}MV_TSU_SIGNAL_CONFIG;


/* Serial data bits order, only one option can be selected.	*/
#define MV_TSU_SER_DATA_ORDER_MSB	(0x0001)
#define MV_TSU_SER_DATA_ORDER_LSB	(0x0002)
#define MV_TSU_SER_DATA_ORDER_MASK				\
	(MV_TSU_SER_DATA_ORDER_MSB | MV_TSU_SER_DATA_ORDER_LSB)


/* Serial sync signal active length in bits, only one option	*/
/* can be selected.						*/
#define MV_TSU_SER_SYNC_ACT_1_BIT	(0x0004)
#define MV_TSU_SER_SYNC_ACT_8_BIT	(0x0008)
#define MV_TSU_SER_SYNC_ACT_LEN_MASK				\
		(MV_TSU_SER_SYNC_ACT_1_BIT | MV_TSU_SER_SYNC_ACT_8_BIT)

/* Serial Tx signal mode, continuous or gapped, only one option	*/
/* can be selected.						*/
#define MV_TSU_SER_TX_CLK_MODE_CONT 	(0x0010)
#define MV_TSU_SER_TX_CLK_MODE_GAPPED	(0x0020)
#define MV_TSU_SER_TX_CLK_MODE_MASK			\
	(MV_TSU_SER_TX_CLK_MODE_CONT | MV_TSU_SER_TX_CLK_MODE_GAPPED)


typedef struct
{
	MV_TSU_PORT_DIRECTION	portDir;
	MV_U16			pktSize;	/* 188 to 256 		   */
}MV_TSU_PORT_CONFIG;


#define MV_TSU_AGGR_MODE_DISABLED	(1)
#define MV_TSU_AGGR_MODE_1		(2)
#define MV_TSU_AGGR_MODE_2		(3)

/*
 * MV_TSU_BUFF_INFO:
 *
 * aggrMode:		Aggregation mode MV_TSU_AGGR_MODE_X
 * aggrMode2TmstmpOff:	Timestamp offset in case of aggr mode 2.
 * aggrNumPackets:	Number of packets in each aggregation.
 * numTsDesc:		Number of descriptors (Power of 2).
 * numDoneQEntry:	Number of done queue entries (Power of 2).
 * tsDataBuff:		Pointer to the data buffers list, the user must ensure 
 * 			DMA coherency for these buffers. The content of these
 *			buffers is never accessed by the HAL.
 * tsDoneBuff:		Pointer to the data status list, the user must ensure 
 * 			DMA coherency for these buffers. The content of these
 *			buffers is never accessed by the HAL.
 * dataBlockSize:	the size of a single data block, this can be larger than
 *			the amount of memory needed to hold TS packets in order
 *			to insure a certain alignment of the buffers.
 *
 * Guidelines for calculating the data & status buffers length:
 *
 *  	data-buff-len = dataBlockSize * numTsDesc
 *	In normal mode:
 *		dataBlockSize = <packet-size> + alignment_space
 *		done-buff-len= numDoneQEntry * TSU_DONE_STATUS_ENTRY_SIZE
 *
 *	In aggregation mode 1:
 *		dataBlockSize = <packet-size> * aggrNumPackets + alignment_space
 *		done-buff-len= numDoneQEntry * TSU_DONE_STATUS_ENTRY_SIZE
 *
 *	In aggregation mode 2: 
 *		dataBlockSize = ((<packet-size> + aggrMode2TmstmpOff)* 
 *			aggrNumPackets) + alignment_space
 *		done-buff-len= numDoneQEntry * TSU_DONE_STATUS_ENTRY_SIZE
 */
typedef struct
{
	MV_U8		aggrMode;
	MV_U8		aggrMode2TmstmpOff;
	MV_U8		aggrNumPackets;

	MV_U32		numTsDesc;
	MV_U32		numDoneQEntry;
	MV_U32		*tsDataBuff;
	MV_U32		tsDataBuffPhys;
	MV_U32		*tsDoneBuff;
	MV_U32		tsDoneBuffPhys;
	MV_U32		dataBlockSize;
}MV_TSU_BUFF_INFO;


/********************************/
/* Macros 			*/
/********************************/
#define MV_TSU_STATUS_ENTRY_TMS_GET(stat)	(MV_U32)(stat & 0xFFFFFFF)


/********************************/
/* Functions API 		*/
/********************************/

/*******************************************************************************
* mvTsuHalInit
*
* DESCRIPTION:
* 	Initialize the TSU unit, and get unit out of reset.
*
* INPUT:
*       coreClock	- The core clock at which the TSU should operate.
*       mode		- The mode on configure the unit into (serial/parallel).
* 	osHandle	- Memory handle used for memory allocations.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK	- on success,
*
*******************************************************************************/
MV_STATUS mvTsuHalInit(MV_TSU_CORE_CLOCK coreClock, MV_TSU_PORTS_MODE mode,
		       void *osHandle);


/*******************************************************************************
* mvTsuShutdown
*
* DESCRIPTION:
* 	Shutdown the TS unit, and put into reset state.
*
* INPUT:
*       None.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK	- on success,
*
*******************************************************************************/
MV_STATUS mvTsuShutdown(void);


/*******************************************************************************
* mvTsuPortReset
*
* DESCRIPTION:
* 	Perform a SW reset on a given port.
*
* INPUT:
*	port	- The port number to reset.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_BAD_VALUE	- Bad port configuration option.
*	MV_BAD_SIZE	- Illegal number of ports.
*
*******************************************************************************/
MV_STATUS mvTsuPortReset(MV_U8 port);


/*******************************************************************************
* mvTsuWinInit
*
* DESCRIPTION:
* 	Initialize the TSU unit access windows mapping.
*
* INPUT:
*       None.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK	- on success,
*
*******************************************************************************/
MV_STATUS mvTsuWinInit(void);


/*******************************************************************************
* mvTsuPortInit
*
* DESCRIPTION:
* 	Initialize the TSU ports.
*
* INPUT:
*	port	- The port number to configure.
*	portCfg	- Port configurations parameters.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_BAD_VALUE	- Bad port configuration option.
*	MV_BAD_SIZE	- Illegal number of ports.
*
*******************************************************************************/
MV_STATUS mvTsuPortInit(MV_U8 port, MV_TSU_PORT_CONFIG *portCfg);


/*******************************************************************************
* mvTsuPortSignalCfgSet
*
* DESCRIPTION:
* 	Configure port signals parameters.
*
* INPUT:
*       port 		- The port to configure.
*	signalCfg	- Signal configuration options.
*	serialflags	- Serial signal configuration options (valid only if the 
*			  port is working in serial mode).
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_BAD_VALUE	- Bad port configuration option.
*	MV_BAD_SIZE	- Illegal number of ports.
*
*******************************************************************************/
MV_STATUS mvTsuPortSignalCfgSet(MV_U8 port, MV_TSU_SIGNAL_CONFIG *signalCfg,
				MV_U32 serialFlags);


/*******************************************************************************
* mvTsuPortSignalCfgGet
*
* DESCRIPTION:
* 	Get port signals parameters.
*
* INPUT:
*       port 		- The port to configure.
* OUTPUT:
*	signalCfg	- Signal configuration options.
*	serialflags	- Serial signal configuration options (valid only if the 
*			  port is working in serial mode).
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Illegal port number.
*	MV_BAD_PARAM	- Bad pointers.
*
*******************************************************************************/
MV_STATUS mvTsuPortSignalCfgGet(MV_U8 port, MV_TSU_SIGNAL_CONFIG *signalCfg,
				MV_U32* serialFlags);


/*******************************************************************************
* mvTsuStatusGet
*
* DESCRIPTION:
* 	Get the TSU port status for a given port.
*
* INPUT:
*	port	- The port number to configure.
* OUTPUT:
*	status	- Bitmask representing the TSU port status (a bitwise or
*		between the MV_TSU_STATUS_* macros.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
MV_STATUS mvTsuStatusGet(MV_U8 port, MV_U32 *status);


/*******************************************************************************
* mvTsuBuffersInit
*
* DESCRIPTION:
* 	Initialize the TSU unit buffers.
*	This function is used to initialize both Rx or Tx buffers according to
*	the port mode configured in mvTsuPortsInit().
*
* INPUT:
*	port	- The port number to configure.
*	buffInfo- TSU buffer information.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad buffer configuration options.
*	MV_NOT_ALIGNED	- Bad data buffer alignemed.
*
*******************************************************************************/
MV_STATUS mvTsuBuffersInit(MV_U8 port, MV_TSU_BUFF_INFO *buffInfo);


/*******************************************************************************
* mvTsuPortShutdown
*
* DESCRIPTION:
* 	Shutdown the port, this will release any previously allocated port 
*	memory, and set the port to disable state.
*
* INPUT:
*	port	- The port number to shutdown.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad buffer configuration options.
*
*******************************************************************************/
MV_STATUS mvTsuPortShutdown(MV_U8 port);


/*******************************************************************************
* mvTsuDmaWatermarkSet
*
* DESCRIPTION:
*	Set the watermark for starting a DMA transfer from / to the TSU 
*	internal FIFO to the CPU memory.
*
* INPUT:
*	port		- The port number to configure.
*	watermark	- The watermark to configure (in DWORD units).
*			For Rx: Number of data DWORDS in FIFO to start DMA.
*			For Tx: Number of free DWORDS in FIFO to start DMA.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad watermark value..
*
*******************************************************************************/
MV_STATUS mvTsuDmaWatermarkSet(MV_U8 port, MV_U32 watermark);


/*******************************************************************************
* mvTsuAggrMode1TmsOnPcktEn
*
* DESCRIPTION:
*	Set the TSU unit to add the timestamp to the packet buffer when working
*	in aggregation mode-1.
*
* INPUT:
*	port	- The port number to configure.
*	enable	- When True, enables the placement of timestamp data in the 
*		  packet buffer.
*		  When False, the timestamp is placed in the Done-Queue.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
MV_STATUS mvTsuAggrMode1TmsOnPcktEn(MV_U8 port, MV_BOOL enable);


/********************************/
/* Rx related APIs		*/
/********************************/


/*******************************************************************************
* mvTsuRxSyncDetectionSet
*
* DESCRIPTION:
* 	Set TS synchronization parameters for Rx data.
*
* INPUT:
*	port		- The port number to configure.
*	syncDetect	- Number of TS sync matches to lock signal.
*	syncLoss	- Number of TS sync losses to unlock signal.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad sync values configuration.
*
*******************************************************************************/
MV_STATUS mvTsuRxSyncDetectionSet(MV_U8 port, MV_U8 syncDetect, MV_U8 syncLoss);


/*******************************************************************************
* mvTsuRxSyncDetectionGet
*
* DESCRIPTION:
* 	Get TS synchronization parameters for Rx data.
*
* INPUT:
*	port		- The port number.
* OUTPUT:
*	syncDetect	- Number of TS sync matches to lock signal.
*	syncLoss	- Number of TS sync losses to unlock signal.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad pointers.
*
*******************************************************************************/
MV_STATUS mvTsuRxSyncDetectionGet(MV_U8 port, MV_U8 *syncDetect, MV_U8 *syncLoss);


/*******************************************************************************
* mvTsuRxFullBuffCountGet
*
* DESCRIPTION:
* 	Get number of Rx packets ready for CPU processing.
*
* INPUT:
*	port		- TSU port number.
* OUTPUT:
*	numBlocks	- Number of data blocks ready for CPU processing 
*			 (already have data for processing).
*	numBuffers	- Number of buffers ready for CPU processing (already
*			have data for processing).
*			In non-aggreagtion mode numBlocks == numBuffers.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_NOT_SUPPORTED- If the port is not configured in input mode.
*
*******************************************************************************/
MV_STATUS mvTsuRxFullBuffCountGet(MV_U8 port, MV_U32 *numBlocks,
				  MV_U32 *numBuffers);


/*******************************************************************************
* mvTsuRxNextBuffGet
*
* DESCRIPTION:
* 	Get a pointer to the next available Rx data & status buffers from the Rx
*	queue.
*
* INPUT:
*	port		- TSU port number.
*	dataBuff	- Pointer to return the address of the next data buffer.
*	statBuff	- Pointer to return the address of the next status buffer.
* OUTPUT:
*	buffsHandle	- A handle returned to the user to be used when freeing 
*			  the buffers in mvTsuRxBuffFree().
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad parameters.
*	MV_NOT_SUPPORTED- Functionality not supported by the port configuration.
*	MV_NO_MORE	- No Rx data is available for copy.
* NOTE:
*	When working in Aggregation mode, the buffer will point to the 
*	beggining of the aggregation buffer and not to a single packet inside 
*	the buffer.
*
*******************************************************************************/
MV_STATUS mvTsuRxNextBuffGet(MV_U8 port,MV_U32 **dataBuff, MV_U32 **statBuff,
			     MV_U32 *buffsHandle);


/*******************************************************************************
* mvTsuRxTimestampCntEn
*
* DESCRIPTION:
* 	Enable / Disable the timestamp counter for Rx direction.
*
* INPUT:
*	port		- TSU port number.
*	enable		- MV_TRUE to enable the timestamp counter.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad parameters.
* NOTE:
*
*******************************************************************************/
MV_STATUS mvTsuRxTimestampCntEn(MV_U8 port,MV_BOOL enable);


/*******************************************************************************
* mvTsuRxBuffFree
*
* DESCRIPTION:
* 	Mark a given set of buffers to be free (ready for new data Rx).
*
* INPUT:
*	port		- TSU port number.
*	dataBuff	- Pointer to the start of data buffer to return.
*	statBuff	- Pointer to the start of status buffer to return.
*	buffsHandle	- The buffers handle as returned by mvTsuRxNextBuffGet()
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_NO_MORE	- No Rx data is available for copy.
*	MV_BAD_PARAM	- Bad parameters.
*	MV_BAD_STATE	- Bad buffer free order, attempting to free a buffer 
*			  before all previous buffers where freed.
* NOTE:
*	When working in Aggregation mode, the buffer will point to the 
*	beggining of the aggregation buffer and not to a single packet inside 
*	the buffer.
*	The numBuffs represents the number of aggregation buffers and not to the
*	number of packets.
*
*******************************************************************************/
MV_STATUS mvTsuRxBuffFree(MV_U8 port,MV_U32 *dataBuff, MV_U32 *statBuff,
			  MV_U32 buffsHandle);


/*******************************************************************************
* mvTsuRxFlushErrorPackets
*
* DESCRIPTION:
* 	Enable / Disable flushing of received erroneous packets.
*
* INPUT:
*	port		- TSU port number.
*	enableFlush	- MV_TRUE to flush recieved erroneous packets.
*			  MV_FALSE to copy erroneous packets to Rx buffers.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
MV_STATUS mvTsuRxFlushErrorPackets(MV_U8 port, MV_BOOL enableFlush);


/********************************/
/* Tx related APIs		*/
/********************************/


/*******************************************************************************
* mvTsuTxClockFreqSet
*
* DESCRIPTION:
* 	Configure the transmit clock frequency and parameters.
*
* INPUT:
*	port		- TSU port number.
*	freq		- The frequency to configure in Hz. 
*			range is 2500000 (2.5MHz) to 80000000 (80MHz).
*	autoAdjust	- Whether to adjust the Tx frequency according to the
*			next packet timestamp.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_VALUE	- Bad Tx frequency value.
*
*******************************************************************************/
MV_STATUS mvTsuTxClockFreqSet(MV_U8 port, MV_U32 freq, MV_BOOL autoAdjust);


/*******************************************************************************
* mvTsuTxFreeBuffCountGet
*
* DESCRIPTION:
* 	Get the number of free packets ready to be transmitted in the TX
*	descriptor list.
*
* INPUT:
*	port		- TSU port number.
* OUTPUT:
*	numBlocks	- Number of data blocks ready for CPU processing 
*			 (does not have data for transmit).
*	numBuffers	- Number of buffers ready for CPU processing (does not 
*			  have data for transmit).
*			In non-aggreagtion mode numBlocks == numBuffers.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_NOT_SUPPORTED- If the port is not configured in input mode.
*
*******************************************************************************/
MV_STATUS mvTsuTxFreeBuffCountGet(MV_U8 port, MV_U32 *numBlocks,
				  MV_U32 *numBuffers);


/*******************************************************************************
* mvTsuRxNextBuffGet
*
* DESCRIPTION:
* 	Get a pointer to the next available Tx data & status buffers from the Tx
*	queue.
*
* INPUT:
*	port		- TSU port number.
*	dataBuff	- Pointer to return the address of the next data buffer.
*	numBuffs	- Pointer to the maximum number of buffers to return.
* OUTPUT:
*	numBuffs	- Number of data / status buffers that where returned.
*	buffsHandle	- A handle returned to the user to be used when freeing 
*			  the buffers in mvTsuRxBuffFree().
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_NO_MORE	- No Rx data is available for copy.
* NOTE:
*	When working in Aggregation mode, the buffer will point to the 
*	beggining of the aggregation buffer and not to a single packet inside 
*	the buffer.
*	The numBuffs represents the number of aggregation buffers and not to the
*	number of packets.
*
*******************************************************************************/
MV_STATUS mvTsuTxNextBuffGet(MV_U8 port,MV_U32 **dataBuff,MV_U32 *buffsHandle);


/*******************************************************************************
* mvTsuTxBuffPut
*
* DESCRIPTION:
* 	Mark a given set of buffers to be ready for transmission.
*
* INPUT:
*	port		- TSU port number.
*	dataBuff	- Pointer to the start of data buffer to put.
*	tmsValue	- The timestamp to associate with the buffer.
*		 	  This parameter is applicable only when working in 
*			  non-aggregation mode.
*	tsErr		- Indicates if the TS packet should be sent as an 
*			  erroneous packet.
*	buffsHandle	- The buffer handle as returned by mvTsuTxNextBuffGet()
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad function parameters.
* NOTE:
*	When working in Aggregation mode, the buffer will point to the 
*	beggining of the aggregation buffer and not to a single packet inside 
*	the buffer.
*
*******************************************************************************/
MV_STATUS mvTsuTxBuffPut(MV_U8 port, MV_U32 *dataBuff, MV_U32 tmsValue,
			 MV_BOOL tsErr, MV_U32 buffsHandle);


/*******************************************************************************
* mvTsuTxStatusGet
*
* DESCRIPTION:
* 	Get the next TX done buffer from the TX done queue.
*
* INPUT:
*	port		- TSU port number.
*	doneBuff	- Pointer to return the address of the next done buffer.
*	buffsHandle	- The buffer handle as returned by mvTsuTxNextBuffGet()
* OUTPUT:
*	numBuffs	- Number of returned buffers.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_NO_MORE	- No free Tx buffers are avilable.
* NOTE:
*
*******************************************************************************/
MV_STATUS mvTsuTxStatusGet(MV_U8 port, MV_U32 **doneBuff, MV_U32 buffsHandle);


/*******************************************************************************
* mvTsuTxInitTimeStampSet
*
* DESCRIPTION:
* 	Set the initial timestamp value for TX operations.
*	This function must be called before each transmit session.
*
* INPUT:
*	port		- TSU port number.
*	enableTmstmp	- Enable the timestamp mechanism for packet transmit.
*			  When false, the TSU will transmit packets back-to-back
*	initTimestamp   - (Valid only if enableTs == MV_TRUE)
*			  The initial timestamp to set.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad timestamp value.
*
*******************************************************************************/
MV_STATUS mvTsuTxInitTimeStampSet(MV_U8 port, MV_BOOL enableTmstmp, 
				  MV_U32 initTimestamp);


/*******************************************************************************
* mvTsuTxDone
*
* DESCRIPTION:
* 	Inform the TS unit that the current transmission session is over.
*	This will stop the internal timestamp counters held by the unit.
*
* INPUT:
*	port		- TSU port number.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
MV_STATUS mvTsuTxDone(MV_U8 port);


#endif /* __INC_MV_TSU__H__ */

