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

#include "mvTsu.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

/********************************/
/* Local enums and structures	*/
/********************************/
#define TSU_MIN_PKT_SIZE		188
#define TSU_MAX_PKT_SIZE		256
#define TSU_NUM_CLOCK_DIVIDERS		6

typedef struct
{
	MV_U32			aggrMode;
	MV_U16			pktSize;
	MV_TSU_PORT_DIRECTION	portDir;
	MV_ULONG		descPhyAddr;
	MV_U32			*descVirtAddr;
	MV_U32			descMemHandle;
	MV_U32			numTsDesc;
	MV_U32			numDoneQEntry;
	MV_U32			aggrNumPckts;
	MV_U32			*tsDataBuff;
	MV_U32			*tsDoneBuff;
	MV_U32			dataBlockSize;
	MV_U32			dataReadIdx;
	MV_U32			statReadIdx;
	MV_U32			cpuRollBit;
	MV_U32			descSize;
	MV_U32			queueMask;
	MV_BOOL			enableTimer;
}MV_TSU_PORT_CTRL;

typedef struct
{
	MV_U32 			numActPorts;
	MV_TSU_PORTS_MODE 	mode;
	MV_TSU_CORE_CLOCK	coreClock;
	void 			*osHandle;
}MV_TSU_CTRL;

/********************************/
/* Local Macros			*/
/********************************/
#define TS_SIGNAL_PARAMS_CONFIG(cfg,usedMask,polMask,reg)		\
	{								\
		if(cfg != TSU_SIGNAL_KEEP_DEF)				\
		{							\
			reg &= ~(usedMask | polMask);			\
			if(cfg != TSU_SIGNAL_DIS)			\
			{						\
				reg |= usedMask;			\
				if(cfg == TSU_SIGNAL_EN_ACT_HIGH)	\
					reg |= polMask;			\
			}						\
		}							\
	}

#define TS_SIGNAL_PARAMS_GET(cfg,usedMask,polMask,reg)			\
	{								\
		if(reg & usedMask)					\
		{							\
			if(reg & polMask)				\
				cfg = TSU_SIGNAL_EN_ACT_HIGH;		\
			else						\
				cfg = TSU_SIGNAL_EN_ACT_LOW;		\
		}							\
		else							\
		{							\
			cfg = TSU_SIGNAL_DIS;				\
		}							\
	}
	


#define TSU_SET_DESC_BUFF_PTR(desc,addr)	(*(MV_U32*)desc) = addr
#define TSU_SET_OUT_DESC_TMSTMP(desc,tms,err)		\
	(*((MV_U32*)desc + 1)) = (tms | (err << 28))

#define TSU_BUFF_HNDL(d, s) (MV_U32) ((d & 0xFFFF) | ((s & 0xFFFF) << 16))
#define TSU_BUFF_HNDL_2_DATA_IDX(h) (MV_U32)(h & 0xFFFF)
#define TSU_BUFF_HNDL_2_STAT_IDX(h) (MV_U32)((h >> 16) & 0xFFFF)

/********************************/
/* Control variables.           */
/********************************/
MV_U32  mvTsuCoreClock2Val[] = {
	83 * _1M,	/* TSU_CORE_CLK_83_MHZ 	*/
	71 * _1M,	/* TSU_CORE_CLK_71_MHZ	*/
	91 * _1M,	/* TSU_CORE_CLK_91_MHZ	*/
	100 * _1M	/* TSU_CORE_CLK_100_MHZ	*/
};
MV_TSU_CTRL		mvTsuCtrl;
MV_TSU_PORT_CTRL 	mvTsuPortCtrl[MV_TSU_NUM_PORTS];


/********************************/
/* Forward functions declaration*/
/********************************/
inline static MV_STATUS mvTsuOperationModeSet(MV_U8 port, MV_BOOL enable);
static MV_STATUS mvTsuReadyBuffCountGet(MV_U8 port, MV_U32 *numBlocks,
					MV_U32 *numBuffers);
static MV_STATUS mvTsuPortEnable(MV_U8 port,MV_BOOL enable);

/********************************/
/* Functions Implementation	*/
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
		       void *osHandle)
{
	MV_U32	reg;
	MV_U32 port;

	/* Setup the core clock.			*/
	reg = MV_REG_READ(MV_TSU_MODES_REG);
	reg &= TSU_MODES_TSCK_MASK;
	reg |= (coreClock << TSU_MODES_TSCK_OFF);

	/* Configure the mode				*/
	reg &= ~TSU_MODES_PAR_MODE_MASK;
	if(mode == TSU_MODE_SERIAL) {
		reg |= TSU_MODES_PAR_MODE_SER;
		mvTsuCtrl.numActPorts = 2;
	} else {
		reg |= TSU_MODES_PAR_MODE_PAR;
		mvTsuCtrl.numActPorts = 1;
	}
	MV_REG_WRITE(MV_TSU_MODES_REG,reg);

	/* Get ports out of reset.				*/
	for(port = 0; port < MV_TSU_NUM_PORTS; port++)
		mvTsuPortReset(port);

	/* Setup control veraibles.			*/
	mvTsuCtrl.coreClock = coreClock;
	mvTsuCtrl.osHandle = osHandle;
	mvTsuCtrl.mode = mode;

	return MV_OK;
}


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
MV_STATUS mvTsuShutdown(void)
{
	//...
	// Check if it's possible to put the module back into reset mode.
	return MV_NOT_IMPLEMENTED;
}


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
MV_STATUS mvTsuPortReset(MV_U8 port)
{
	MV_U32 reg;

	/* Check the correctness of parameters.		*/
	if(port >= MV_TSU_NUM_PORTS)
		return MV_BAD_SIZE;

	/* First, set in reset mode, then get out of reset.	*/
	reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));
	reg &= ~TSU_CFG_RESET_MASK;
	reg |= TSU_CFG_RESET_SET;
	MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);
	reg &= ~TSU_CFG_RESET_MASK;
	reg |= TSU_CFG_RESET_CLEAR;
	MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);

	return MV_OK;
}


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
MV_STATUS mvTsuPortInit(MV_U8 port, MV_TSU_PORT_CONFIG *portCfg)
{
	MV_U32	reg;

	/* Check the correctness of parameters.		*/
	if(((mvTsuCtrl.mode == TSU_MODE_SERIAL) && (port >= MV_TSU_NUM_PORTS))||
	   ((mvTsuCtrl.mode == TSU_MODE_PARALLEL) && (port >= 1)))
		return MV_BAD_SIZE;

	if((portCfg->pktSize < TSU_MIN_PKT_SIZE) || 
	   (portCfg->pktSize > TSU_MAX_PKT_SIZE)) 
		return MV_BAD_VALUE;

	/* configure the port parameters		*/
	/* Disable operation mode.			*/
	mvTsuOperationModeSet(port,MV_FALSE);

        reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));

	/* Setup packet size.			*/
	reg &= ~TSU_CFG_PKT_SIZE_MASK;
	reg |= (((portCfg->pktSize - 1) & 0xFF) << TSU_CFG_PKT_SIZE_OFFS);

	/* Setup data direction.		*/
	reg &= ~TSU_CFG_DATA_DIR_MASK;
	if(portCfg->portDir == TSU_PORT_INPUT)
		reg |= TSU_CFG_DATA_DIR_IN;
	else
		reg |= TSU_CFG_DATA_DIR_OUT;

	/* Setup serial / parallel mode.	*/
	reg &= ~TSU_CFG_DATA_MODE_MASK;
	if(mvTsuCtrl.mode == TSU_MODE_SERIAL)
		reg |= TSU_CFG_DATA_MODE_SER;
	else
		reg |= TSU_CFG_DATA_MODE_PAR;

	MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);

	/* Setup DMA packet size.		*/
	reg = MV_REG_READ(MV_TSU_DMA_PARAMS_REG(port));
	reg &= ~TSU_DMAP_DMA_LEN_MASK;
	reg |= portCfg->pktSize;
	MV_REG_WRITE(MV_TSU_DMA_PARAMS_REG(port),reg);

	/* Setup timestamp auto adjust.		*/
	if(portCfg->portDir == TSU_PORT_OUTPUT)
	{
		reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));
		reg &= ~TSU_TMS_CTRL_AUTO_ADJ_MASK;
		reg |= TSU_TMS_CTRL_AUTO_ADJ_ON;
		MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);
	}

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	/* Update global control vars.			*/
	mvTsuPortCtrl[port].pktSize = portCfg->pktSize;
	mvTsuPortCtrl[port].portDir = portCfg->portDir;

	return MV_OK;
}


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
				MV_U32 serialFlags)
{
	MV_U32	reg;

	if(port >= MV_TSU_NUM_PORTS)
		return MV_OUT_OF_RANGE;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));

	/* Setup signal related options.	*/
	TS_SIGNAL_PARAMS_CONFIG(signalCfg->tsSync,
				TSU_CFG_SYNC_USED_MASK,
				TSU_CFG_SYNC_POL_MASK,reg);
	TS_SIGNAL_PARAMS_CONFIG(signalCfg->tsValid,
				TSU_CFG_VAL_USED_MASK,
				TSU_CFG_VAL_POL_MASK,reg);
	TS_SIGNAL_PARAMS_CONFIG(signalCfg->tsError,
				TSU_CFG_ERR_USED_MASK,
				TSU_CFG_ERR_POL_MASK,reg);

	if(signalCfg->tsDataEdge != TSU_SIGNAL_EDGE_KEEP_DEF)
	{
		if(signalCfg->tsDataEdge == TSU_SIGNAL_EDGE_FALL)
			reg |= TSU_CFG_TX_EDGE_MASK;
		else
			reg &= ~TSU_CFG_TX_EDGE_MASK;
	}

	/* Setup serial mode related configurations.	*/
	if(mvTsuCtrl.mode == TSU_MODE_SERIAL)
	{
		if(serialFlags & MV_TSU_SER_DATA_ORDER_MASK)
		{
			reg &= ~TSU_CFG_DATA_ORD_MASK;
			if(serialFlags & MV_TSU_SER_DATA_ORDER_MSB)
				reg |= TSU_CFG_DATA_ORD_MSB;
			else if(serialFlags & MV_TSU_SER_DATA_ORDER_LSB)
				reg |= TSU_CFG_DATA_ORD_LSB;
		}

		if(serialFlags & MV_TSU_SER_SYNC_ACT_LEN_MASK)
		{
			reg &= ~TSU_CFG_TS_SYNC_MASK;
			if(serialFlags & MV_TSU_SER_SYNC_ACT_1_BIT)
				reg |= TSU_CFG_TS_SYNC_1BIT;
			else if(serialFlags & MV_TSU_SER_SYNC_ACT_8_BIT)
				reg |= TSU_CFG_TS_SYNC_8BIT;
		}

		if(serialFlags & MV_TSU_SER_TX_CLK_MODE_MASK)
		{
			reg &= ~TSU_CFG_CLK_MODE_MASK;
			if(serialFlags & MV_TSU_SER_TX_CLK_MODE_GAPPED)
				reg |= TSU_CFG_CLK_MODE_GAPPED;
			else if(serialFlags & MV_TSU_SER_TX_CLK_MODE_CONT)
				reg |= TSU_CFG_CLK_MODE_CONT;
		}
	}

	MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
				MV_U32* serialFlags)
{
	MV_U32	reg;

	if(port >= MV_TSU_NUM_PORTS)
		return MV_OUT_OF_RANGE;

	if((signalCfg == NULL) ||
	   ( (mvTsuCtrl.mode == TSU_MODE_SERIAL) && (serialFlags == NULL)))
		return MV_BAD_PARAM;

	reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));

	/* Setup signal related options.	*/
	TS_SIGNAL_PARAMS_GET(signalCfg->tsSync,TSU_CFG_SYNC_USED_MASK,
			     TSU_CFG_SYNC_POL_MASK,reg);
	TS_SIGNAL_PARAMS_GET(signalCfg->tsValid,TSU_CFG_VAL_USED_MASK,
			     TSU_CFG_VAL_POL_MASK,reg);
	TS_SIGNAL_PARAMS_GET(signalCfg->tsError,TSU_CFG_ERR_USED_MASK,
			     TSU_CFG_ERR_POL_MASK,reg);

	if(reg & TSU_CFG_TX_EDGE_MASK)
		signalCfg->tsDataEdge = TSU_SIGNAL_EDGE_FALL;
	else
		signalCfg->tsDataEdge = TSU_SIGNAL_EDGE_RISE;

	/* Setup serial mode related configurations.	*/
	if(mvTsuCtrl.mode == TSU_MODE_SERIAL)
	{
		*serialFlags = 0;

		if((reg & TSU_CFG_DATA_ORD_MASK) == TSU_CFG_DATA_ORD_LSB)
			*serialFlags |= MV_TSU_SER_DATA_ORDER_LSB;
		else
			*serialFlags |= MV_TSU_SER_DATA_ORDER_MSB;

		if((reg & TSU_CFG_TS_SYNC_MASK) == TSU_CFG_TS_SYNC_1BIT)
			*serialFlags |= MV_TSU_SER_SYNC_ACT_1_BIT;
		else
			*serialFlags |= MV_TSU_SER_SYNC_ACT_8_BIT;

		if((reg & TSU_CFG_CLK_MODE_MASK) == MV_TSU_SER_TX_CLK_MODE_GAPPED)
			*serialFlags |= TSU_CFG_CLK_MODE_GAPPED;
		else
			*serialFlags |= TSU_CFG_CLK_MODE_CONT;
	}

	return MV_OK;
}


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
MV_STATUS mvTsuStatusGet(MV_U8 port, MV_U32 *status)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
	{
		return MV_OUT_OF_RANGE;
	}

	reg = MV_REG_READ(MV_TSU_STATUS_REG(port));
	reg &= TSU_STATUS_MASK;

	*status = 0;
	if(reg & TSU_STATUS_IF_ERR)
		*status |= MV_TSU_STATUS_TSIF_ERROR;
	if(reg & TSU_STATUS_FIFO_OVFL_ERR)
		*status |= MV_TSU_STATUS_OVFL_ERROR;
	if(reg & TSU_STATUS_CONN_ERR)
		*status |= MV_TSU_STATUS_CONN_ERROR;
	return MV_OK;
}


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
MV_STATUS mvTsuBuffersInit(MV_U8 port, MV_TSU_BUFF_INFO *buffInfo)
{
	MV_U32	reg;
	MV_U32	descSize;
	MV_U32	i;
	MV_U8	*tsDataBuff;
	MV_U32	*descEntry;
	MV_U32	phyAddr;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;

	if( ( (buffInfo->aggrMode != MV_TSU_AGGR_MODE_DISABLED) &&
	      ( (buffInfo->aggrMode2TmstmpOff > 0xF ) ||
		(buffInfo->aggrMode2TmstmpOff < 4 ) ||
		(buffInfo->aggrNumPackets <= 1) ||
		(buffInfo->numDoneQEntry < buffInfo->aggrNumPackets) ) ) ||
	    (!MV_IS_POWER_OF_2(buffInfo->numTsDesc) ) ||
	    (!MV_IS_POWER_OF_2(buffInfo->numDoneQEntry) ) ||
	    (buffInfo->numTsDesc == 0) ||
	    (buffInfo->numDoneQEntry == 0) ||
	    (buffInfo->numDoneQEntry > MV_TSU_MAX_DONEQ_LEN) ||
	    ( (mvTsuPortCtrl[port].portDir == TSU_PORT_INPUT) &&
	      (buffInfo->numTsDesc > MV_TSU_MAX_IN_QUEUE_LEN) ) ||
	    ( (mvTsuPortCtrl[port].portDir == TSU_PORT_OUTPUT) &&
	      (buffInfo->numTsDesc > MV_TSU_MAX_OUT_QUEUE_LEN) ) )
	{
		return MV_BAD_PARAM;
	}

	/* Check buffer alignment.			*/
	if( ( (buffInfo->aggrMode == MV_TSU_AGGR_MODE_2) &&
	      (((buffInfo->tsDataBuffPhys + buffInfo->aggrMode2TmstmpOff) &
		(TSU_DMA_ALIGN - 1)) != 0) ) ||
	    ( (buffInfo->aggrMode == MV_TSU_AGGR_MODE_DISABLED) &&
	      (buffInfo->tsDataBuffPhys & (TSU_DMA_ALIGN - 1)) != 0 ) )
	{
		return MV_NOT_ALIGNED;
	}

	/* Disable operation mode.			*/
	mvTsuOperationModeSet(port,MV_FALSE);

	/* Setup aggregation mode.			*/
	reg = MV_REG_READ(MV_TSU_AGGREGATION_CTRL_REG(port));
	reg &= ~TSU_AGGR_ENABLE_MASK;
	if(buffInfo->aggrMode == MV_TSU_AGGR_MODE_DISABLED)
	{
		reg |= TSU_AGGR_DISABLE;
		mvTsuPortCtrl[port].aggrNumPckts = 1;
	}
	else
	{
                reg |= TSU_AGGR_ENABLE;

		reg &= ~TSU_AGGR_TMSTMP_OFF_MASK;
		if(buffInfo->aggrMode == MV_TSU_AGGR_MODE_2)
		{
			reg |= (buffInfo->aggrMode2TmstmpOff << 
				TSU_AGGR_TMSTMP_OFF_OFFS);
			reg &= ~TSU_AGGR_TMSTMP_MODE_MASK;
			reg |= TSU_AGGR_TMSTMP_TO_PCKT;
		}

		reg &= ~TSU_AGGR_PCKT_NUM_MASK;
		reg |= (buffInfo->aggrNumPackets << TSU_AGGR_PCKT_NUM_OFFS);

		MV_REG_WRITE(MV_TSU_AGGREGATION_CTRL_REG(port),reg);
		mvTsuPortCtrl[port].aggrNumPckts = buffInfo->aggrNumPackets;
	}

	mvTsuPortCtrl[port].aggrMode = buffInfo->aggrMode;
	if(mvTsuPortCtrl[port].portDir == TSU_PORT_INPUT)
		descSize = TSU_INPUT_DESC_ENTRY_SIZE;
	else
		descSize = TSU_OUTPUT_DESC_ENTRY_SIZE;

	/* Initialize descriptor data.			*/
	mvTsuPortCtrl[port].descVirtAddr = mvOsIoUncachedMalloc(
		mvTsuCtrl.osHandle,descSize * buffInfo->numTsDesc,
		&(mvTsuPortCtrl[port].descPhyAddr),
		&(mvTsuPortCtrl[port].descMemHandle));

	/* Initialize the descriptors list with buffer pointers.	*/
	descEntry = (MV_U32*)mvTsuPortCtrl[port].descVirtAddr;
	tsDataBuff = (MV_U8*)buffInfo->tsDataBuff;
	phyAddr = buffInfo->tsDataBuffPhys;
	for(i = 0; i < buffInfo->numTsDesc; i++)
	{
                TSU_SET_DESC_BUFF_PTR(descEntry,phyAddr);
		descEntry += (descSize >> 2);
		tsDataBuff += buffInfo->dataBlockSize;
		phyAddr += buffInfo->dataBlockSize;
	}

	mvTsuPortCtrl[port].queueMask =
		~(0xFFFFFFFF << mvLog2(buffInfo->numTsDesc * descSize));

	if(mvTsuPortCtrl[port].portDir == TSU_PORT_INPUT)
		mvTsuPortCtrl[port].cpuRollBit =
			mvTsuPortCtrl[port].queueMask + 1;
	else
		mvTsuPortCtrl[port].cpuRollBit = 0;

	/* Write the read / write pointers for data & status buffers.	*/

	/* Desc start pointer.	*/
	phyAddr = mvTsuPortCtrl[port].descPhyAddr;
	MV_REG_WRITE(MV_TSU_DESC_QUEUE_BASE_REG(port),phyAddr);

        /* Desc read pointer.	*/
	MV_REG_WRITE(MV_TSU_DESC_QUEUE_READ_PTR_REG(port),
		     0 << TSU_DESC_READ_PTR_OFFS);

	/* Desc write pointer.	*/
        MV_REG_WRITE(MV_TSU_DESC_QUEUE_WRITE_PTR_REG(port),
		     mvTsuPortCtrl[port].cpuRollBit);

	/* Done start pointer.	*/
	phyAddr = buffInfo->tsDoneBuffPhys;
	MV_REG_WRITE(MV_TSU_DONE_QUEUE_BASE_REG(port),phyAddr);

	/* Done read pointer.	*/
	MV_REG_WRITE(MV_TSU_DONE_QUEUE_READ_PTR_REG(port),
		     0 << TSU_DONE_READ_PTR_OFFS);

	/* Done write pointer.	*/
	MV_REG_WRITE(MV_TSU_DONE_QUEUE_WRITE_PTR_REG(port),
		     mvTsuPortCtrl[port].cpuRollBit);//0 << TSU_DONE_WRITE_PTR_OFFS);

	/* Done & Data queues size.	*/
	reg = MV_REG_READ(MV_TSU_DMA_PARAMS_REG(port));
	reg &= ~(TSU_DMAP_DESC_Q_SIZE_MASK | TSU_DMAP_DONE_Q_SIZE_MASK);

	i = mvLog2((buffInfo->numTsDesc * descSize) >> 2);
	reg |= (i << TSU_DMAP_DESC_Q_SIZE_OFFS);

	i = mvLog2((buffInfo->numDoneQEntry * TSU_DONE_STATUS_ENTRY_SIZE) >> 2);
	reg |= (i << TSU_DMAP_DONE_Q_SIZE_OFFS);

	MV_REG_WRITE(MV_TSU_DMA_PARAMS_REG(port),reg);

	mvTsuPortCtrl[port].descSize = descSize;
	mvTsuPortCtrl[port].numTsDesc = buffInfo->numTsDesc;
	mvTsuPortCtrl[port].numDoneQEntry = buffInfo->numDoneQEntry;
	mvTsuPortCtrl[port].tsDataBuff = buffInfo->tsDataBuff;
	mvTsuPortCtrl[port].tsDoneBuff = buffInfo->tsDoneBuff;
	mvTsuPortCtrl[port].dataBlockSize = buffInfo->dataBlockSize;
	mvTsuPortCtrl[port].dataReadIdx = 0;
	mvTsuPortCtrl[port].statReadIdx = 0;
	mvTsuPortCtrl[port].enableTimer = MV_FALSE;

	mvTsuPortEnable(port,MV_TRUE);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
MV_STATUS mvTsuPortShutdown(MV_U8 port)
{
	MV_TSU_PORT_CTRL *portCtrl;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;

	/* Disable operation mode.	*/
	mvTsuOperationModeSet(port,MV_FALSE);

	/* Disable the port.		*/
	mvTsuPortEnable(port,MV_FALSE);

	/* Free descriptors buffer.	*/
	portCtrl = &mvTsuPortCtrl[port];
	mvOsIoUncachedFree(mvTsuCtrl.osHandle,
			   portCtrl->descSize * portCtrl->numTsDesc,
			   portCtrl->descPhyAddr,portCtrl->descVirtAddr,
			   portCtrl->descMemHandle);
	return MV_OK;
}


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
MV_STATUS mvTsuDmaWatermarkSet(MV_U8 port, MV_U32 watermark)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(watermark > TSU_DMAP_DATA_WTRMK_MAX)
		return MV_BAD_PARAM;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_DMA_PARAMS_REG(port));
	reg &= ~TSU_DMAP_DATA_WTRMK_MASK;
	reg |= (watermark << TSU_DMAP_DATA_WTRMK_OFFS);
	MV_REG_WRITE(MV_TSU_DMA_PARAMS_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
MV_STATUS mvTsuAggrMode1TmsOnPcktEn(MV_U8 port, MV_BOOL enable)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_AGGREGATION_CTRL_REG(port));
	reg &= ~TSU_AGGR_TMSTMP_MODE_MASK;
	if(enable == MV_TRUE)
		reg |= TSU_AGGR_TMSTMP_TO_PCKT;
	else
		reg |= TSU_AGGR_TMSTMP_TO_DONE_Q;
	MV_REG_WRITE(MV_TSU_AGGREGATION_CTRL_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
MV_STATUS mvTsuRxSyncDetectionSet(MV_U8 port, MV_U8 syncDetect, MV_U8 syncLoss)
{
	MV_U32	reg;
	MV_U8	maxVal;

	if(port >= MV_TSU_NUM_PORTS)
		return MV_OUT_OF_RANGE;

	maxVal = TSU_SYNC_LOSS_CNT_MASK >> TSU_SYNC_LOSS_CNT_OFFS;
	if((syncDetect > maxVal) || (syncLoss > maxVal))
		return MV_BAD_PARAM;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_SYNCBYTE_DETECT_REG(port));
	reg &= ~(TSU_SYNC_LOSS_CNT_MASK | TSU_SYNC_DETECT_CNT_MASK);
	reg |= ((syncDetect << TSU_SYNC_DETECT_CNT_OFFS) |
		(syncLoss << TSU_SYNC_LOSS_CNT_OFFS));
	MV_REG_WRITE(MV_TSU_SYNCBYTE_DETECT_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
MV_STATUS mvTsuRxSyncDetectionGet(MV_U8 port, MV_U8 *syncDetect, MV_U8 *syncLoss)
{
	MV_U32	reg;

	if(port >= MV_TSU_NUM_PORTS)
		return MV_OUT_OF_RANGE;

	if((syncDetect == NULL) || (syncLoss == NULL))
		return MV_BAD_PARAM;

	reg = MV_REG_READ(MV_TSU_SYNCBYTE_DETECT_REG(port));

	*syncDetect = (reg & TSU_SYNC_DETECT_CNT_MASK) >> TSU_SYNC_DETECT_CNT_OFFS;
	*syncLoss = (reg & TSU_SYNC_LOSS_CNT_MASK) >> TSU_SYNC_LOSS_CNT_OFFS;

	return MV_OK;
}


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
				  MV_U32 *numBuffers)
{
	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_INPUT)
		return MV_NOT_SUPPORTED;
	return mvTsuReadyBuffCountGet(port,numBlocks,numBuffers);
}

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
*	statBuff	- Pointer to return the address of the next status
*			  buffer.
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
MV_U32 g_type;
MV_STATUS mvTsuRxNextBuffGet(MV_U8 port,MV_U32 **dataBuff, MV_U32 **statBuff,
			     MV_U32 *buffsHandle)
{
	MV_TSU_PORT_CTRL *portCtrl;
	MV_U32	numBlocks;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_INPUT)
		return MV_NOT_SUPPORTED;
	if((dataBuff == NULL) || (statBuff == NULL) || (buffsHandle == NULL))
		return MV_BAD_PARAM;

	mvTsuReadyBuffCountGet(port,&numBlocks,NULL);
	if(numBlocks <= 2)
		return MV_NO_MORE;

	portCtrl = &mvTsuPortCtrl[port];

	/* Get read pointer.		*/
	*dataBuff = (MV_U32*)(((MV_U32)portCtrl->tsDataBuff) + 
			      (portCtrl->dataBlockSize * portCtrl->dataReadIdx));
        *statBuff = 
		(MV_U32*)(((MV_U32)portCtrl->tsDoneBuff) + 
			  (TSU_DONE_STATUS_ENTRY_SIZE * portCtrl->statReadIdx));
	*buffsHandle = TSU_BUFF_HNDL(portCtrl->dataReadIdx,
				     portCtrl->statReadIdx);
	return MV_OK;
}


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
MV_STATUS mvTsuRxTimestampCntEn(MV_U8 port,MV_BOOL enable)
{
	MV_U32 reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_INPUT)
		return MV_NOT_SUPPORTED;

	reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));
	reg &= ~TSU_TMS_CTRL_TIMER_MASK;

	if(enable == MV_TRUE)
		reg |= TSU_TMS_CTRL_TIMER_EN;
	else
		reg |= TSU_TMS_CTRL_TIMER_DIS;

	MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);

	return MV_OK;
}


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
*
*******************************************************************************/
MV_STATUS mvTsuRxBuffFree(MV_U8 port,MV_U32 *dataBuff, MV_U32 *statBuff,
			  MV_U32 buffsHandle)
{
	MV_TSU_PORT_CTRL *portCtrl;
	MV_U32 ptr;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_INPUT)
		return MV_NOT_SUPPORTED;
	if((dataBuff == NULL) || (statBuff == NULL))
		return MV_BAD_PARAM;

	portCtrl = &mvTsuPortCtrl[port];

	if(buffsHandle != 
	   TSU_BUFF_HNDL(portCtrl->dataReadIdx,portCtrl->statReadIdx)){
		mvOsPrintf("Bad state..........................\n");
		return MV_BAD_STATE;
	}

	portCtrl->dataReadIdx = 
		(portCtrl->dataReadIdx + 1) & (portCtrl->numTsDesc - 1);
	if(portCtrl->dataReadIdx == 0)
		portCtrl->cpuRollBit ^= (portCtrl->queueMask + 1);

	portCtrl->statReadIdx =
		((portCtrl->statReadIdx + portCtrl->aggrNumPckts) & 
		 (portCtrl->numDoneQEntry - 1));

	/* Update the desc queue write pointer.	*/
	ptr = (portCtrl->dataReadIdx << 2) | portCtrl->cpuRollBit;

//#warning "mvTsuRxBuffFree() is hacked."
	MV_REG_WRITE(MV_TSU_DESC_QUEUE_WRITE_PTR_REG(port),ptr);

	mvOsBridgeReorderWA();

	return MV_OK;
}


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
MV_STATUS mvTsuRxFlushErrorPackets(MV_U8 port, MV_BOOL enableFlush)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_AGGREGATION_CTRL_REG(port));
	reg &= ~TSU_AGGR_FLUSH_ERR_MASK;
	if(enableFlush == MV_TRUE)
		reg |= TSU_AGGR_FLUSH_ERR_ENABLE;
	else
		reg |= TSU_AGGR_FLUSH_ERR_DISABLE;
	MV_REG_WRITE(MV_TSU_AGGREGATION_CTRL_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);
	return MV_OK;
}


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
MV_STATUS mvTsuTxClockFreqSet(MV_U8 port, MV_U32 freq, MV_BOOL autoAdjust)
{
	MV_U32	reg;
	MV_U32	clockVal;
	MV_U32	divider;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	clockVal = mvTsuCoreClock2Val[mvTsuCtrl.coreClock];
	if(freq > clockVal)
		return MV_BAD_VALUE;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	/* Find the correct divider that satisfies: freq <= (core_clock / div)*/
	divider = TSU_NUM_CLOCK_DIVIDERS - 1;
	while(divider != 0)
	{
		if(freq <= (clockVal >> divider))
			break;
		divider--;
	}

	/* Now, bestDev holds the best divider value.			*/
	reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));
	reg &= ~(TSU_CFG_OUT_CLOCK_MASK | TSU_CFG_FREQ_MODE_MASK);
	switch(divider)
	{
	case(0):	/* 1 */
		reg |= (1 << TSU_CFG_FREQ_MODE_OFFS);
		/* Fall through */
	case(3):	/* 8 */
        	reg |= TSU_CFG_OUT_CLOCK_1_8;
		break;
	case(1):	/* 2 */
		reg |= (1 << TSU_CFG_FREQ_MODE_OFFS);
		/* Fall through */
	case(4):	/* 16 */
        	reg |= TSU_CFG_OUT_CLOCK_2_16;
		break;
	case(2):	/* 4 */
		reg |= (1 << TSU_CFG_FREQ_MODE_OFFS);
		/* Fall through */
	case(5):	/* 32 */
        	reg |= TSU_CFG_OUT_CLOCK_4_32;
		break;
	default:
		break;
	}
        MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);

	/* Setup auto adjust.				*/
	reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));
	reg &= ~TSU_TMS_CTRL_AUTO_ADJ_MASK;
	if(autoAdjust == MV_TRUE)
		reg |= TSU_TMS_CTRL_AUTO_ADJ_ON;
	else
		reg |= TSU_TMS_CTRL_AUTO_ADJ_OFF;
	MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
*	MV_NOT_SUPPORTED- If the port is not configured in output mode.
*
*******************************************************************************/
MV_STATUS mvTsuTxFreeBuffCountGet(MV_U8 port, MV_U32 *numBlocks,
				  MV_U32 *numBuffers)
{
	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;
	return mvTsuReadyBuffCountGet(port,numBlocks,numBuffers);
}


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
*	statBuff	- Pointer to return the address of the next status 
*			  buffer.
* OUTPUT:
*	buffsHandle	- A handle returned to the user to be used when freeing 
*			  the buffers in mvTsuTxBuffPut().
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*	MV_BAD_PARAM	- Bad parameters.
*	MV_NOT_SUPPORTED- Functionality not supported by the port configuration.
*	MV_NO_MORE	- No free Tx data is available.
* NOTE:
*	When working in Aggregation mode, the buffer will point to the 
*	beggining of the aggregation buffer and not to a single packet inside 
*	the buffer.
*
*******************************************************************************/
MV_STATUS mvTsuTxNextBuffGet(MV_U8 port,MV_U32 **dataBuff,MV_U32 *buffsHandle)
{
	MV_TSU_PORT_CTRL *portCtrl;
	MV_U32	numBlocks;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;
	if((dataBuff == NULL) || (buffsHandle == NULL))
		return MV_BAD_PARAM;

	mvTsuTxFreeBuffCountGet(port,&numBlocks,NULL);
	//printk("g_type = 0x%x.\n",g_type);
	if(numBlocks == 0)
		return MV_NO_MORE;

	portCtrl = &mvTsuPortCtrl[port];

	if(dataBuff != NULL) {
	*dataBuff = (MV_U32*)(((MV_U32)portCtrl->tsDataBuff) + 
			      (portCtrl->dataBlockSize * portCtrl->dataReadIdx));
	*buffsHandle = TSU_BUFF_HNDL(portCtrl->dataReadIdx,portCtrl->statReadIdx);
	}

	return MV_OK;
}


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
*			  This parameter is applicable only when working in
*			  non-aggregation mode.
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
			 MV_BOOL tsErr, MV_U32 buffsHandle)
{
	MV_TSU_PORT_CTRL *portCtrl;
	MV_U32 *descEntry;
	MV_U32 ptr;
	MV_U32 reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;
	if(dataBuff == NULL)
		return MV_BAD_PARAM;

	portCtrl = &mvTsuPortCtrl[port];
	if(TSU_BUFF_HNDL_2_DATA_IDX(buffsHandle) != portCtrl->dataReadIdx)
		return MV_BAD_STATE;

	if(portCtrl->aggrMode == MV_TSU_AGGR_MODE_DISABLED) {
		/* Write the timestamp value.	*/
		descEntry = (MV_U32*)(((MV_U32)portCtrl->descVirtAddr) + 
				      (portCtrl->dataReadIdx << 3));
		TSU_SET_OUT_DESC_TMSTMP(descEntry,tmsValue,tsErr);
	}

	portCtrl->dataReadIdx = 
		(portCtrl->dataReadIdx + 1) & (portCtrl->numTsDesc - 1);
	if(portCtrl->dataReadIdx == 0)
		portCtrl->cpuRollBit ^= (portCtrl->queueMask + 1);

	//mvTsuOperationModeSet(port,MV_FALSE);
	/* Update the desc queue write pointer.	*/
	ptr = (portCtrl->dataReadIdx << 3) | portCtrl->cpuRollBit;
	MV_REG_WRITE(MV_TSU_DESC_QUEUE_WRITE_PTR_REG(port),ptr);

	mvOsBridgeReorderWA();

	if(portCtrl->enableTimer) {
		/* Enable the timestamp counter.	*/
		reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));
		reg &= ~TSU_TMS_CTRL_TIMER_MASK;
		reg |= TSU_TMS_CTRL_TIMER_EN;
		MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);
		portCtrl->enableTimer = MV_FALSE;
	}

	//mvTsuOperationModeSet(port,MV_TRUE);

	return MV_OK;
}


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
MV_STATUS mvTsuTxStatusGet(MV_U8 port, MV_U32 **doneBuff, MV_U32 buffsHandle)
{
	MV_TSU_PORT_CTRL *portCtrl;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;
	if(doneBuff == NULL)
		return MV_BAD_PARAM;

	portCtrl = &mvTsuPortCtrl[port];
	if(TSU_BUFF_HNDL_2_STAT_IDX(buffsHandle) != portCtrl->statReadIdx)
		return MV_BAD_STATE;

	*doneBuff = (MV_U32*)(portCtrl->tsDoneBuff + portCtrl->statReadIdx);

	portCtrl->statReadIdx = 
		((portCtrl->statReadIdx + portCtrl->aggrNumPckts)&
		 (portCtrl->numDoneQEntry - 1));

	return MV_OK;
}


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
				  MV_U32 initTimestamp)
{
	MV_U32	reg;
	MV_U32	tsReg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));
	if(enableTmstmp == MV_TRUE)
	{
		/* Setup timestamp initial value.		*/
		tsReg = MV_REG_READ(MV_TSU_TIMESTAMP_REG(port));
		tsReg &= ~TSU_TMSTMP_TIMESTAMP_MASK;
		tsReg |= (initTimestamp & 
			  (TSU_TMSTMP_TIMESTAMP_MASK >> TSU_TMSTMP_TIMESTAMP_OFFS));
		MV_REG_WRITE(MV_TSU_TIMESTAMP_REG(port),tsReg);

		/* Trigger the TSU to read the timestamp.	*/
		reg |= (1 << TSU_TMS_CTRL_READ_TIMER_OFFS);
	}

	/* Disable the timestamp counter (Will be enabled on 	*/
	/* the first packet Tx */
	reg &= ~TSU_TMS_CTRL_TIMER_MASK;
	reg |= TSU_TMS_CTRL_TIMER_DIS;
	MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);
	mvTsuPortCtrl[port].enableTimer = enableTmstmp;

	/* Enable operation mode.				*/
	mvTsuOperationModeSet(port,MV_TRUE);
	return MV_OK;
}


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
MV_STATUS mvTsuTxDone(MV_U8 port)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;
	if(mvTsuPortCtrl[port].portDir != TSU_PORT_OUTPUT)
		return MV_NOT_SUPPORTED;

	/* Disable operation mode.		*/
	mvTsuOperationModeSet(port,MV_FALSE);

	reg = MV_REG_READ(MV_TSU_TIMESTAMP_CTRL_REG(port));

	/* Disable the timestamp counter.	*/
	reg &= ~TSU_TMS_CTRL_TIMER_MASK;
	reg |= TSU_TMS_CTRL_TIMER_DIS;
	MV_REG_WRITE(MV_TSU_TIMESTAMP_CTRL_REG(port),reg);

	/* Enable operation mode.		*/
	mvTsuOperationModeSet(port,MV_TRUE);
	return MV_OK;
}


/*******************************************************************************
* mvTsuOperationModeSet
*
* DESCRIPTION:
* 	Set the given TS port into operaion mode.
*
* INPUT:
*	port	- TSU port number.
*	enable	- MV_TRUE: enable operation mode.
*		  MV_FALSE: disable operation mode.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
inline static MV_STATUS mvTsuOperationModeSet(MV_U8 port, MV_BOOL enable)
{
	MV_U32	reg;

	reg = MV_REG_READ(MV_TSU_CONFIG_REG(port));
	reg &= ~TSU_CFG_OPER_MASK;
	if(enable)
		reg |= TSU_CFG_OPER_ENABLE;
	else
		reg |= TSU_CFG_OPER_DISABLE;
	MV_REG_WRITE(MV_TSU_CONFIG_REG(port),reg);
	return MV_OK;
}


/*******************************************************************************
* mvTsuReadyBuffCountGet
*
* DESCRIPTION:
* 	Get number of packets ready for CPU processing.
*	In Rx direction, this is the number of packets ready to be rpocessed by 
*	CPU. For Tx direction, this is the number of free buffers ready for data
*	transmission.
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
*
*******************************************************************************/
static MV_STATUS mvTsuReadyBuffCountGet(MV_U8 port, MV_U32 *numBlocks,
					MV_U32 *numBuffers)
{
	MV_U32 cpuPtr;
	MV_U32 tsuPtr;
	MV_U32 result = 0;
	MV_U32 tsuRollBit;
	MV_TSU_PORT_CTRL *portCtrl = &mvTsuPortCtrl[port];

	/* Get CPU pointer.		*/
	cpuPtr = portCtrl->dataReadIdx;

	/* Get TSU pointer.		*/
	tsuPtr = MV_REG_READ(MV_TSU_DESC_QUEUE_READ_PTR_REG(port));

	tsuRollBit = tsuPtr & (portCtrl->queueMask + 1);
	tsuPtr &= portCtrl->queueMask;
	tsuPtr /= portCtrl->descSize;

	g_type = (tsuPtr << 16) | (cpuPtr << 8) | (tsuRollBit << 24);
	if(tsuPtr > cpuPtr) {
//		printk("tsuPtr(%d) > cpuPtr(%d), ",tsuPtr,cpuPtr);
		result = tsuPtr - cpuPtr;
		g_type |= 1;
	} else if(tsuPtr < cpuPtr) {
//		printk("tsuPtr(%d) < cpuPtr(%d), ",tsuPtr,cpuPtr);
		result = tsuPtr + portCtrl->numTsDesc - cpuPtr;
		g_type |= 2;
	} else if( (tsuPtr == cpuPtr) && (portCtrl->cpuRollBit == tsuRollBit) ) {
//		printk("tsuPtr(%d) == cpuPtr(%d) - %d, ",tsuPtr,cpuPtr,tsuRollBit);
		result = portCtrl->numTsDesc;
		g_type |= 3;
	}

	if(numBuffers)
		*numBuffers = result * portCtrl->aggrNumPckts;
	if(numBlocks)
		*numBlocks = result;

	return MV_OK;
}


/*******************************************************************************
* mvTsuPortEnable
*
* DESCRIPTION:
* 	Enable port for receiving & transmitting data after it has been 
*	configured.
*
* INPUT:
*	port	- The port number to configure.
*	enable	- MV_TRUE to enable port.
*		  MV_FALSE to disable it.
* OUTPUT:
*	None.
* RETURN:
*       MV_OK		- On success,
*	MV_OUT_OF_RANGE	- Unsupported port number.
*
*******************************************************************************/
static MV_STATUS mvTsuPortEnable(MV_U8 port,MV_BOOL enable)
{
	MV_U32	reg;

	if(port >= mvTsuCtrl.numActPorts)
		return MV_OUT_OF_RANGE;

	if(enable == MV_TRUE)
		reg = 0xFFFFFFFF;
	else
		reg = 0x04040404;

	reg = MV_REG_WRITE(MV_TSU_ENABLE_ACCESS_REG(port),reg);
	return MV_OK;
}

