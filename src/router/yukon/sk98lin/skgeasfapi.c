/******************************************************************************
 *
 * Name:        skgeasfapi.c
 * Project:     GEnesis, PCI Gigabit Ethernet Adapter
 * Purpose:     ASF API module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *	Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 *****************************************************************************/

/*******************************************************************************
 *
 * Defines
 *
 ******************************************************************************/
#include "h/sktypes.h"
#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

extern int SkFwApiGet (SK_AC *pAC , SK_IOC IoC );
extern SK_U32 HandleDrvCommand (SK_AC *pAC, SK_GE_CMDIOCTL *pFwCommand, SK_U32 FrameLength);
extern SK_U32 SendFwCommand (SK_AC *pAC, SK_U32 DatagramType, char *pFwDataBuff, SK_U32 FrameLength);
static SK_U8 SkWriteRamBuf(SK_AC *, SK_U32 *, SK_U32, SK_U32);
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
static void PrintDataBuffer(char *pInfo, char *pData, SK_U32 Framelength);
#endif
static SK_U32 SkWriteData(SK_AC *pAC, SK_U32 *pMsgBuff, SK_U32 WPointer, SK_U32 MsgSize);
static SK_U32 SkReadDta(SK_AC *pAC, SK_U32 *pMsgBuff, SK_U32 RamAddrOffset, SK_U32 RamAddrMaxValue,
	SK_U32 RPointer, SK_U32 MsgSize);

/******************************************************************************
 *
 *	SetRamAddr() - set the RAM address
 *
 * Description:
 *	Set the RAM address used in ReadRamQWord and WriteRamQWord.
 *
 * Returns:
 *	nothing
 */
void SetRamAddr(
SK_AC	*pAC,		/* Adapter context */
int		BufNumber,	/* Number of RAM buffer: 3 (ASF Fifo) */
int		AddrLoWord,	/* Address low word:     Address/8 (quadword) */
int		AddrHiWord)	/* Address high word:    0 */
{
	/* Support for Yukon Extreme and Yukon Supreme */
	if ((pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
		(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)) {
		if (BufNumber >= 0 && BufNumber <= 3) {
			pAC->RamSelect = BufNumber;
		}
	}
	else {
		return;
	}

	pAC->RamAddr = (SK_U32)((SK_U32)AddrLoWord & 0xffff) +
			(((SK_U32)AddrHiWord << 16) & 0xffff0000L);

} /* SetRamAddr */

/******************************************************************************
 *
 *      RamWriteAddr() - Writes one quadword to board RAM
 *
 * Description:
 *      Write one quadword to board RAM.
 *      For Yukon Extreme, if Addr == 2^29 then the qword
 *      at the current position of the hardware pointer is read.
 *
 * Returns:
 *      0 on success, 1 on error
 */
static int RamWriteAddr(
SK_AC   *pAC,                    /* Adapter context */
SK_U32  Addr,                   /* Address to be write at */
SK_U32  LowDword,               /* Lower Dword to be write */
SK_U32  HighDword,              /* Upper Dword to be write */
int     SelectedRam)            /* Selected RAM buffer 0, 1 or 3 */
{
        /* Support for Yukon Extreme and Supreme */
        if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EX) ||
                (pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)) {
                if (Addr == BIT_29) {
                        Addr = SelectedRam << 30;
                }
                else {
                        /* Use given address (lower 8 bits for TX, lower 9 bits for RX). */
                        Addr = (Addr & 0x7fffL) | BIT_29 | (SelectedRam << 30);
                }
                SelectedRam = 0;
        }
        else {
                return(1);
        }

        SK_OUT32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_ADDR), Addr);
        SK_OUT32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_DATA_LO), LowDword);
        SK_OUT32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_DATA_HI), HighDword);

        return(0);
}

/******************************************************************************
 *
 *      RamReadAddr() - Reads one quadword from board RAM
 *
 * Description:
 *      Read one quadword from board RAM.
 *      For Yukon Extreme, if Addr == 2^29 then the qword
 *      at the current position of the hardware pointer is read.
 *
 * Returns:
 *      0 on success, 1 on error
 */
static int RamReadAddr(
SK_AC   *pAC,                   /* Adapter context */
SK_U32  Addr,                   /* Address to be read at */
SK_U32  *pLowDword,             /* Lower Dword to be read */
SK_U32  *pHighDword,    /* Upper Dword to be read */
int     SelectedRam)    /* Selected RAM buffer 0, 1 or 3 */
{

        /* Support for Yukon Extreme and Supreme */
        if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EX) ||
		(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)) {
                if (Addr == BIT_29) {
                        Addr = SelectedRam << 30;
                }
                else {
                        /* Use given address (lower 8 bits for TX, lower 9 bits for RX). */
                        Addr = (Addr & 0x7fffL) | BIT_29 | (SelectedRam << 30);
                }
                SelectedRam = 0;
        }
        else {
                return(1);
        }

        SK_OUT32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_ADDR), Addr);
        /* Read Access is initiated by reading the lower dword. */
        SK_IN32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_DATA_LO), pLowDword);
        SK_IN32(pAC->IoBase, SELECT_RAM_BUFFER(SelectedRam, B3_RAM_DATA_HI), pHighDword);

        return(0);
}


/******************************************************************************
 *
 *      SkReadDta() - Read data from board RAM
 *
 * Description:
 *      Read a data block from board RAM.
 *
 * Returns:
 *      0 on success, 1 on error
 */

static SK_U32 SkReadDta(
SK_AC   *pAC,            /* Adapter context */
SK_U32  *pMsgBuff,       /* Message buffer */
SK_U32  RamAddrOffset,   /* First RAM address (qword) */
SK_U32  RamAddrMaxValue, /* Last RAM address (qword) */
SK_U32  RPointer,        /* Read pointer (qword) */
SK_U32  MsgSize)         /* Message size */
{
	SK_U32  LowDword = 0x0;
	SK_U32  HighDword = 0x0;
	SK_U32  *pWriteDword = NULL;
	SK_U32  i = 0;

	pWriteDword = pMsgBuff;

	/* Get the payload */
	for (i=0; i<MsgSize; i+=8) {
		/* Overflow check */
		if (RPointer >= RamAddrMaxValue) {
			RPointer = RamAddrOffset+2;
		}

		(void) RamReadAddr(pAC, RPointer,
					&LowDword, &HighDword, pAC->RamSelect);
		*pWriteDword = LowDword;
		pWriteDword++;
		*pWriteDword = HighDword;
		pWriteDword++;
		RPointer++;
	}
	return RPointer;
}


/******************************************************************************

 *
 *      SkHandleStatusMsgFromFW() - Print a status message from the FW
 *
 * Description:
 *      Print a status message buffer from the firmware
 *
 * Returns:
 *      0 on success, 1 on error
 */
void SkHandleStatusMsgFromFW(
SK_AC   *pAC,				/* Adapter context */
SK_U32  *pMsgBuff,			/* Message buffer */
SK_U32  FrameLength)		/* Message size */
{

	printk("%s: FW Status -> Time:0x%x  Status:0x%x  Base:0x%x\n",
		SK_DRV_NAME,
		(SK_U32) pMsgBuff[0],
		(SK_U32) pMsgBuff[1],
		(SK_U32) pMsgBuff[2]);
}

/******************************************************************************
 *
 *      SkHandleDataPacket() - Send a data packet to upper layer
 *
 * Description:
 *      Send a data packet to the commonication protocol stack
 *
 * Returns:
 *      0 on success, 1 on error
 */
void SkHandleDataPacket(
SK_AC   *pAC,				/* Adapter context */
SK_U32  *pMsgBuff,			/* Message buffer */
SK_U32  FrameLength)		/* Message size */
{
	struct  sk_buff *pMsg = NULL;

	pMsg = alloc_skb(FrameLength+2, GFP_ATOMIC);
	if (pMsg == NULL) {
		printk("SkHandleDataPacket: No RX buffer!!\n");
		return;
	}
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("SkHandleDataPacket: Send packet!!\n");
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	skb_copy_to_linear_data(pMsg, pMsgBuff,
		FrameLength);
#else
	eth_copy_and_sum(pMsg, (char *) pMsgBuff,
		FrameLength, 0);
#endif
	skb_put(pMsg, FrameLength);
	pMsg->ip_summed = CHECKSUM_NONE;
	pMsg->dev = pAC->dev[0];
	pMsg->protocol = eth_type_trans(pMsg, pAC->dev[0]);

#ifdef CONFIG_SK98LIN_NAPI
	netif_receive_skb(pMsg);
#else
	netif_rx(pMsg);
#endif
	pAC->dev[0]->last_rx = jiffies;
	return;
}


/******************************************************************************
 *
 *      SkHandleResultPacket() - handte the FW resulta packet
 *
 * Description:
 *      Retrieve the command result and handle the data
 *
 * Returns:
 *      0 on success, 1 on error
 */
void SkHandleResultPacket(
SK_AC   *pAC,				/* Adapter context */
SK_U32  *pMsgBuff,			/* Message buffer */
SK_U32  FrameLength)		/* Message size */
{
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	FwCmdResult	*FwCommandResult;
	SK_U16      CommandType;
	SK_U16      CommandLength;
	SK_U16      CommandResult;
#endif

	/*
	 * THIS ROUTINE MUST COPY THE CONTENT OF pMsgBuff TO ANOTHER LOCATION!
	 * pMsgBuff WILL BE FREED AFTER THIS ROUTINE HAS FINISHED!
	 */

	if (pAC->FwBufferLen > 0) {
		/* Clear the old buffer */
		kfree(pAC->pFwBuffer);
	}

	if ((pAC->pFwBuffer = (SK_U32 *) kmalloc(FrameLength, GFP_KERNEL)) != NULL ) {
		/* Copy the data into a new buffer */
		pAC->FwBufferLen = FrameLength;
		memcpy(pAC->pFwBuffer, pMsgBuff, FrameLength);
	} else {
		printk("Alloc data ERROR\n");
		return;
	}

	/* DEBUG CODE! */

#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	FwCommandResult = (FwCmdResult *) pAC->pFwBuffer;
	CommandType = FwCommandResult->Hdr.Cmd;
	CommandLength = FwCommandResult->Hdr.Len;
	CommandResult = FwCommandResult->Result;

	printk("%s: SkHandleResultPacket => CommandType:   0x%x\n", DRV_NAME, CommandType);
	printk("%s: SkHandleResultPacket => CommandLength: 0x%x\n", DRV_NAME, CommandLength);
	printk("%s: SkHandleResultPacket => CommandResult: 0x%x\n", DRV_NAME, CommandResult);

	/* Print output to see complete FW command result buffer */
	PrintDataBuffer("SkHandleResultPacket => FIFO DATA", pMsgBuff, FrameLength);
#endif
}

/******************************************************************************
 *
 *      SkHandleMsgCmdResponsePacket() - handte the FW resulta packet
 *
 * Description:
 *      Store the received msg cmd response for later retrieval
 *
 * Returns:
 *      - (void)
 */
void SkHandleMsgCmdResponsePacket(
SK_AC   *pAC,				/* Adapter context */
SK_U32  *pMsgBuff,			/* Message buffer */
SK_U32  FrameLength)		/* Message size */
{
	if (pMsgBuff && FrameLength) {
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
		printk("%s-%s() FrameLength: 0x%x\n", DRV_NAME, __func__, FrameLength);
		PrintDataBuffer("FIFO DATA", pMsgBuff, FrameLength);
#endif

		if (pAC->FwBufferLen > 0) {
			kfree(pAC->pFwBuffer); /* Clear the old buffer */
		}

		if ((pAC->pFwBuffer = (SK_U32 *) kmalloc(FrameLength, GFP_KERNEL)) != NULL ) {
			/* Copy the data into a new buffer */
			pAC->FwBufferLen = FrameLength;
			memcpy(pAC->pFwBuffer, pMsgBuff, FrameLength);
		} else {
			printk("%s-%s() Alloc data ERROR\n", DRV_NAME, __func__);
		}
	}
}

/******************************************************************************
 *
 *      SkGenDta() - Generate a test data packet
 *
 * Description:
 *      Generate a test data packet
 *
 * Returns:
 *      0 on success, 1 on error
 */
SK_U32 SkGenDta(
SK_AC   *pAC,			/* Adapter context */
SK_U32	*pMsg,			/* Datagram data */
SK_U32  DgramId,		/* Datagram ID */
SK_U16  DgramLen,		/* Datagram length */
SK_U8   *ddata)			/* Payload data */
{
	FifoDgram     dgram;
//        SK_U8         pad[8] = {0,0,0,0,0,0,0,0};
        SK_U8         padlen = 0;

	/* Generate the datagram */
        /* fill header */
        dgram.Id = DgramId;
        dgram.DgramLen = DgramLen+sizeof(pMsg);
        dgram.PayloadLen = DgramLen;

        /* update DgramLen with padding */
        if (dgram.DgramLen % 8)
                padlen = 8 - ( dgram.DgramLen % 8 );
        dgram.DgramLen += padlen;



	printk("Data: 0x%x 0x%x 0x%x\n", dgram.DgramLen, dgram.PayloadLen, dgram.Id);
	return 0;
}


/******************************************************************************
 *
 *      SkGenDta() - Initialize the download fifo
 *
 * Description:
 *      Init the download fifo on board RAM
 *
 * Returns:
 *      0 on success, 1 on error
 */

void SkInitDloadFifo (
SK_AC   *pAC)                   /* Adapter context */
{
//	SK_U32	Addr;

	/* Calculate the address and write the read pointer */



	printk("QW: 0x%x Addr->0x%x\n", HOST_WRITE_QWORD, HOST_WRITE_ADDR);
}

/******************************************************************************
 *
 *      SkAccessRamBuf() - Handle a read request from the firmware
 *
 * Description:
 *      Handle a read request from the firmware
 *
 * Returns:
 *      0 on success, 1 on error
 */

SK_U8 SkAccessRamBuf(
SK_AC  *pAC,            /* Adapter context */
SK_U32 RamAddrOffset,   /* First RAM address (qword) */
SK_U32 RamAddrMaxValue) /* Last RAM address (qword) */
{
	SK_U32  LowDword = 0x0;
	SK_U32  HighDword = 0x0;
	SK_U32  RPointer = 0x0;
	SK_U32  WPointer = 0x0;
	SK_U32  StartAddress = 0x0;
	SK_U32  DPointer = 0x0;
	SK_U32  CurrRamAddr = pAC->RamAddr;
	SK_U32  RPointerRamAddr = pAC->RamAddr;
	SK_U16  *pDataLow = (SK_U16 *) &LowDword;
	SK_U32  *pMsgBuff = NULL;

#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("RamOffset: 0x%x RamMaxVal: 0x%x\n", RamAddrOffset, RamAddrMaxValue);
#endif

	/* Read the fifo status */
	(void) RamReadAddr(pAC, CurrRamAddr,
				&LowDword, &HighDword, pAC->RamSelect);

	/* Fifo not available... Return... */
	if ((SK_U32) LowDword == 0) {
		printk("sk98lin: Fifo not available\n");
		return 0;
	}

	/* Set CurrRamAddr to beginning of FIFO write array! */
	CurrRamAddr = RamAddrOffset;

	/* Datagram handling */
	(void) RamReadAddr(pAC, CurrRamAddr,
				&StartAddress, &RPointer, pAC->RamSelect);
	RPointerRamAddr = CurrRamAddr;
	CurrRamAddr++;
	(void) RamReadAddr(pAC, CurrRamAddr,
				&DPointer, &WPointer, pAC->RamSelect);
	RPointer = (RPointer - HOST_ROM_ADDR)/8;
	WPointer = (WPointer - HOST_ROM_ADDR)/8;
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("Read pointer STA: RP: 0x%x WP: 0x%x\n", RPointer, WPointer);
#endif

	/*
	**  exit if the read pointer == write pointer
	*/
	while (RPointer != WPointer) {
		if (RPointer >= RamAddrMaxValue) {
			RPointer = RamAddrOffset+2;
		}
		if (RPointer == WPointer)
			goto pointer_save;

		(void) RamReadAddr(pAC, RPointer,
						&LowDword, &HighDword, pAC->RamSelect);

		if ((SK_U32) LowDword == 0) {
			printk("sk98lin: Fifo data not available\n");
			return 0;
		}

		pMsgBuff = (SK_U32 *) kmalloc(pDataLow[0], GFP_ATOMIC);
		if (pMsgBuff == NULL) {
			printk("sk98lin: Memory allocation failed!\n");

			/* Set read pointer to beginning of next FIFO datagram */
			RPointer = RPointer + (pDataLow[0]/8);
			goto pointer_save;
		}

		RPointer++;
		RPointer = SkReadDta(pAC, pMsgBuff, RamAddrOffset,
						RamAddrMaxValue, RPointer, pDataLow[1]);

		switch(HighDword) {
			case SKGE_DGRAM_CMD:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle Command\n");
#endif
				break;
			case SKGE_DGRAM_RESULT:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle Command result\n");
#endif
				SkHandleResultPacket(pAC, pMsgBuff, pDataLow[1]);
				break;
			case SKGE_DGRAM_PACKET:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle Packet\n");
#endif
				SkHandleDataPacket(pAC, pMsgBuff, pDataLow[1]);
				break;
			case SKGE_DGRAM_MSG2FW:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle Message to FW app\n");
#endif
				break;
			case SKGE_DGRAM_MSGFW:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle Message from FW app\n");
#endif
				SkHandleMsgCmdResponsePacket(pAC, pMsgBuff, pDataLow[1]);
				break;
			case SKGE_DGRAM_STATUS:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle status message from FW\n");
#endif
				SkHandleStatusMsgFromFW(pAC, pMsgBuff, pDataLow[1]);
				break;
			case SKGE_DGRAM_APP_VERSION:
				((SK_U8*)pMsgBuff)[pDataLow[1]-1]	= '\0';
				printk("Firmware Application Code: %s\n", (SK_U8 *)(pMsgBuff+1));
				break;
			default:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("  +++ Handle default\n");
#endif
				break;
		}

		/* Done */
		if (pMsgBuff != NULL) {
			kfree(pMsgBuff);
		}
	}

	pointer_save:
	/* Set the read pointer address */
	if (RPointer >= RamAddrMaxValue)
		RPointer = RamAddrOffset+2;

#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("Read pointer END: RP: 0x%x WP: 0x%x\n", RPointer, WPointer);
#endif

	RPointer = (RPointer*8)+HOST_ROM_ADDR;
	RamWriteAddr(pAC, RPointerRamAddr, StartAddress, RPointer, pAC->RamSelect);

	return 0;
}


/*****************************************************************************
*
* SkFwApiGet - Get the communication data
*
* Description:
*
* Returns:
*
*/

int SkFwApiGet (
    SK_AC *pAC,     /* Pointer to adapter context */
    SK_IOC IoC)     /* IO context handle */
{
//	SK_U32  *pMsg = NULL;
	unsigned long          Flags;

	/* Datagram structure */
	DgamMsg     ddata;
        ddata.cmd = 1;
        ddata.len = 5;
        ddata.data = 1;
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("Handle SkFwApiGet\n");
#endif
	spin_lock_irqsave(&pAC->FwFifoLock, Flags);
	SetRamAddr(pAC, SK_ST_FIFOTYPE, HOST_READ_QWORD, SK_ST_BUFADDR_HIGH);
	SkAccessRamBuf(pAC, HOST_READ_DATA_QWORD, HOST_READ_PACKET_QWORD);
	spin_unlock_irqrestore(&pAC->FwFifoLock, Flags);
	return 0;
}


/******************************************************************************
 *
 *      SkWriteRamBuf() - Handle a write request from the upper layer
 *
 * Description:
 *      Handle a write request to the firmware
 *
 * BE AWARE: SK_IN/SK_OUT macros will swap the following 32bit
 *           variables in RamReadAddr() and RamWriteAddr():
 *
 *           - LowDword
 *           - HighDword
 *           - RPointer
 *           - WPointer
 *           - DPointer
 *           - CurrRamAddr
 *
 * Returns:
 *      0 on success, 1 on error
 */
SK_U8 SkWriteRamBuf(
SK_AC   *pAC,			/* Adapter context */
SK_U32  *pMsgBuff,
SK_U32  FrameLength,
SK_U32  DatagramType)
{
	SK_U32 LowDword = 0x0;
	SK_U32 HighDword = 0x0;
	SK_U32 RPointer = 0x0;
	SK_U32 WPointer = 0x0;
	SK_U32 Size = 0x0;
	SK_U32 DPointer = 0x0;
	SK_U32 CurrRamAddr = pAC->RamAddr;
	SK_U32 WPointerRamAddr;
	SK_U16 *pDataLow = (SK_U16 *) &LowDword;
	SK_U16 DatagramLength;

	/* Read the FIFO status */
	(void) RamReadAddr(pAC, CurrRamAddr,
					   &LowDword, &HighDword, pAC->RamSelect);

	/* FIFO not available => Return! */
	if ((SK_U32) LowDword == 0) {
		printk("%s: SkWriteRamBuf => Fifo not available!\n", DRV_NAME);
		return 1;
	}
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("SkWriteRamBuf => Fifo available: 0x%x\n", LowDword);
#endif

	/* Set CurrRamAddr to beginning of FIFO write array! */
	CurrRamAddr = HOST_WRITE_QWORD;

	/* Retrieve read pointer */
	(void) RamReadAddr(pAC, CurrRamAddr,
					   &DPointer, &RPointer, pAC->RamSelect);

	/* Retrieve write pointer */
	CurrRamAddr++;
	(void) RamReadAddr(pAC, CurrRamAddr,
					   &Size, &WPointer, pAC->RamSelect);
	WPointerRamAddr = CurrRamAddr;

	/* Convert pointer addresses to quadword addresses */
	RPointer = (RPointer - HOST_ROM_ADDR)/8;
	WPointer = (WPointer - HOST_ROM_ADDR)/8;
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("SkWriteRamBuf START => RPointer: 0x%x\n", RPointer);
	printk("SkWriteRamBuf START => WPointer: 0x%x\n", WPointer);
#endif
	/*
	 * Fill LowDword and HighDword with FIFO datagram information:
	 * LowDword: Datagram and payload length, HighDword: Transaction ID
	 */
	DatagramLength = 8;
	DatagramLength += (FrameLength/8) * 8;
	if (FrameLength % 8) {
		DatagramLength += 8;
	}

	/* ENDIANESS */
#ifdef SK_LITTLE_ENDIAN
	pDataLow[0] = DatagramLength;
	pDataLow[1] = FrameLength;
#else
	pDataLow[1] = DatagramLength;
	pDataLow[0] = FrameLength;
#endif
	HighDword = DatagramType;

	/* Write FIFO datagram information into RAM */
	(void) RamWriteAddr(pAC, WPointer,
						LowDword, HighDword, pAC->RamSelect);
	WPointer++;

	/* TEST */
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	PrintDataBuffer("SkWriteRamBuf => FIFO DATA", (char *) pMsgBuff, FrameLength);
#endif

	/* Write payload data into FIFO RAM */
	WPointer = SkWriteData(pAC, pMsgBuff,
							WPointer, FrameLength);

	/* Set the write pointer address */
	if (WPointer > HOST_WRITE_LAST_QWORD) {
		WPointer = HOST_WRITE_QWORD+2;
	}
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("SkWriteRamBuf END => RPointer:   0x%x\n", RPointer);
	printk("SkWriteRamBuf END => WPointer:   0x%x\n", WPointer);
#endif
	WPointer = (WPointer*8)+HOST_ROM_ADDR;
	(void) RamWriteAddr(pAC, WPointerRamAddr,
						Size, WPointer, pAC->RamSelect);
	return 0;

} /* SkWriteRamBuf */

/******************************************************************************
 *
 *      SkWriteData() - Write data to board RAM
 *
 * Description:
 *      Write a data block to board RAM.
 *
 * Returns:
 *      0 on success, 1 on error
 */
static SK_U32 SkWriteData(
SK_AC   *pAC,			/* Adapter context */
SK_U32  *pMsgBuff,		/* Message buffer */
SK_U32  WPointer,		/* Write pointer */
SK_U32  MsgSize)		/* Message size */
{
	SK_U32 LowDword = 0x0;
	SK_U32 HighDword = 0x0;
	SK_U32 *pWriteDword = NULL;
	SK_U32 i = 0;

	pWriteDword = pMsgBuff;

	/* Write the payload */
	for (i=0; i<MsgSize; i+=8) {

		/* Overflow check */
		if (WPointer > HOST_WRITE_LAST_QWORD) {
			WPointer = HOST_WRITE_QWORD+2;
		}

		LowDword = *pWriteDword;
		pWriteDword++;
		HighDword = *pWriteDword;
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
		printk("SkWriteData   => LowDword:    0x%08x\n", LowDword);
		printk("SkWriteData   => HighDword:   0x%08x\n", HighDword);
#endif
		(void) RamWriteAddr(pAC, WPointer,
						   LowDword, HighDword, pAC->RamSelect);
		pWriteDword++;
		WPointer++;
	}

	return WPointer;

} /* SkWriteData */


/*****************************************************************************
*
* SkFwIsr - Handle an ISR from the firmware
*
* Description:
*
* Returns:
*
*/

void SkFwIsr (
    SK_AC *pAC,     /* Pointer to adapter context */
    SK_IOC IoC)     /* IO context handle */
{
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
	printk("Handle SkFwIsr\n");
#endif
	SetRamAddr(pAC, SK_ST_FIFOTYPE, HOST_READ_QWORD, SK_ST_BUFADDR_HIGH);
	SkAccessRamBuf(pAC, HOST_READ_DATA_QWORD, HOST_READ_PACKET_QWORD);
	if (!pAC->SdkQH) {
		if ((netif_running(pAC->dev[0])) &&
			(pAC->dev[0]->flags & IFF_RUNNING)) {
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
			printk("Handle SkFwIsr packet\n");
#endif
			SkAccessRamBuf(pAC, HOST_READ_PACKET_QWORD, HOST_WRITE_DATA_QWORD);
		}
	}
	FwHciStateMachine(pAC, IoC, 0);

	return;
}


/*****************************************************************************
 *
 * HandleDrvCommand - Handle a FW command
 *
 * Description:
 *
 * Returns:
 *
 */
SK_U32 HandleDrvCommand (
	SK_AC          *pAC,		/* Pointer to adapter context */
	SK_GE_CMDIOCTL *pFwCommand,	/* FW data buffer from OS application layer/driver */
	SK_U32         FrameLength)	/* Length of FW data buffer in bytes */
{

	switch(pFwCommand->Command) {
		case SK_COMMAND_SLEEP:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
			printk("  +++ Handle sleep command\n");
#endif
			FwHciSendCommand(pAC, pAC->IoBase, YASF_HOSTCMD_SYS_WILL_SLEEP, 0, 0, 0, ASF_HCI_WAIT, 0);
			if (pAC->SdkQH)
				NETIF_STOP_ALLQ(pAC->dev[0]);
			break;
		case SK_COMMAND_SA_UPDATE:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
			printk("  +++ Handle sa update command\n");
#endif
			FwHciSendCommand(pAC, pAC->IoBase, YASF_HOSTCMD_SYS_SA_UPDATE_COMPLETE, 0, 0, 0, ASF_HCI_WAIT, 0);
			if (pAC->SdkQH) {
				NETIF_WAKE_ALLQ(pAC->dev[0]);
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
				printk("Handle HandleDrvCommand packet\n");
#endif
				SetRamAddr(pAC, SK_ST_FIFOTYPE, HOST_READ_QWORD, SK_ST_BUFADDR_HIGH);
				SkAccessRamBuf(pAC, HOST_READ_PACKET_QWORD, HOST_WRITE_DATA_QWORD);
			}
			break;
		default:
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
			printk("  +++ Handle default\n");
#endif
			break;
	}

	printk("Handle DrvCommand -> 0x%x\n", pFwCommand->Command);

	return 0;
}


/*****************************************************************************
 *
 * SendFwCommand - Handle a FW command
 *
 * Description:
 *
 * Returns:
 *
 */
SK_U32 SendFwCommand (
	SK_AC 		*pAC,			/* Pointer to adapter context */
	SK_U32		DatagramType,	/* Datagram type */
	char		*pFwDataBuff,	/* FW data buffer from OS application layer/driver */
	SK_U32		FrameLength)	/* Length of FW data buffer in bytes */
{
	unsigned long          Flags;

	spin_lock_irqsave(&pAC->FwFifoLock, Flags);
	SetRamAddr(pAC, SK_ST_FIFOTYPE, HOST_READ_QWORD, SK_ST_BUFADDR_HIGH);

	if (SkWriteRamBuf(pAC, (SK_U32 *) pFwDataBuff, FrameLength, DatagramType)) {
		spin_unlock_irqrestore(&pAC->FwFifoLock, Flags);
		return -1;
	}
	spin_unlock_irqrestore(&pAC->FwFifoLock, Flags);
	return 0;

} /* SendFwCommand */

#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
/*****************************************************************************
 *
 * PrintDataBuffer - Do exactly what the name states  => ONLY FOR TESTING!
 *
 * Description:
 *
 * Returns:
 *
 */
void PrintDataBuffer(char *pInfo, char *pData, SK_U32 Framelength)
{
	int currByte = 0;
	int numBytes = 0;
	int offset = 0;

	printk("%s: Framelength: %d\n\n", pInfo, Framelength);

	for (currByte = 0; currByte < Framelength; currByte=currByte+8) {
		if ((currByte + 8) < Framelength) {
#ifndef SK_PRINT_CHAR
			printk("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
#else
			printk("%c %c %c %c %c %c %c %c\n",
#endif
				(pData[currByte + 0] & 0xff),
				(pData[currByte + 1] & 0xff),
				(pData[currByte + 2] & 0xff),
				(pData[currByte + 3] & 0xff),
				(pData[currByte + 4] & 0xff),
				(pData[currByte + 5] & 0xff),
				(pData[currByte + 6] & 0xff),
				(pData[currByte + 7] & 0xff));
		} else {
			numBytes = Framelength - currByte;
			offset = currByte;
			for (currByte = 0; currByte < numBytes; currByte++ ) {
#ifndef SK_PRINT_CHAR
				printk("0x%02x ", (pData[offset + currByte] & 0xff));
#else
				printk("%c ", (pData[offset + currByte] & 0xff));
#endif
			}
			printk("\n\n");
			break;
		}
	}
	return;

} /* PrintDataBuffer */
#endif
