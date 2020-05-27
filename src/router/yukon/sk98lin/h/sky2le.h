/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/common/V6/h/sky2le.h#8 $
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: #8 $, $Change: 9172 $
 * Date:	$DateTime: 2012/03/01 18:14:31 $
 * Purpose:	Common List Element definitions and access macros
 *
 ******************************************************************************/

/******************************************************************************
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
 ******************************************************************************/

#ifndef __INC_SKY2LE_H
#define __INC_SKY2LE_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ********************************************************************/

#define MIN_LEN_OF_LE_TAB	128
#define MAX_LEN_OF_LE_TAB	4096
#ifdef USE_POLLING_UNIT
#define NUM_LE_POLLING_UNIT	2
#endif
#define MAX_FRAG_OVERHEAD	10

/* Macro for aligning a given value */
#define SK_ALIGN_SIZE(Value, Alignment, AlignedVal) {					\
	(AlignedVal) = (((Value) + (Alignment) - 1) & (~((Alignment) - 1)));\
}

/******************************************************************************
 *
 * LE2DWord() - Converts the given Little Endian value to machine order value
 *
 * Description:
 *	This function converts the Little Endian value received as an argument to
 *	the machine order value.
 *
 * Returns:
 *	The converted value
 *
 */

#ifdef SK_LITTLE_ENDIAN

#ifndef	SK_USE_REV_DESC
#define LE2DWord(value)	(value)
#else	/* SK_USE_REV_DESC */
#define LE2DWord(value)					\
	((((value)<<24L) & 0xff000000L) +	\
	 (((value)<< 8L) & 0x00ff0000L) +	\
	 (((value)>> 8L) & 0x0000ff00L) +	\
	 (((value)>>24L) & 0x000000ffL))
#endif	/* SK_USE_REV_DESC */

#else	/* !SK_LITTLE_ENDIAN */

#ifndef	SK_USE_REV_DESC
#define LE2DWord(value)					\
	((((value)<<24L) & 0xff000000L) +	\
	 (((value)<< 8L) & 0x00ff0000L) +	\
	 (((value)>> 8L) & 0x0000ff00L) +	\
	 (((value)>>24L) & 0x000000ffL))
#else	/* SK_USE_REV_DESC */
#define LE2DWord(value)	(value)
#endif	/* SK_USE_REV_DESC */

#endif	/* !SK_LITTLE_ENDIAN */

/******************************************************************************
 *
 * DWord2LE() - Converts the given value to a Little Endian value
 *
 * Description:
 *	This function converts the value received as an argument to a Little Endian
 *	value on Big Endian machines. If the machine running the code is Little
 *	Endian, then no conversion is done.
 *
 * Returns:
 *	The converted value
 *
 */

#ifdef SK_LITTLE_ENDIAN

#ifndef	SK_USE_REV_DESC
#define DWord2LE(value) (value)
#else	/* SK_USE_REV_DESC */
#define DWord2LE(value)					\
	((((value)<<24L) & 0xff000000L) +	\
	 (((value)<< 8L) & 0x00ff0000L) +	\
	 (((value)>> 8L) & 0x0000ff00L) +	\
	 (((value)>>24L) & 0x000000ffL))
#endif	/* SK_USE_REV_DESC */

#else	/* !SK_LITTLE_ENDIAN */

#ifndef	SK_USE_REV_DESC
#define DWord2LE(value)					\
	((((value)<<24L) & 0xff000000L) +	\
	 (((value)<< 8L) & 0x00ff0000L) +	\
	 (((value)>> 8L) & 0x0000ff00L) +	\
	 (((value)>>24L) & 0x000000ffL))
#else	/* SK_USE_REV_DESC */
#define DWord2LE(value) (value)
#endif	/* SK_USE_REV_DESC */
#endif	/* !SK_LITTLE_ENDIAN */

/******************************************************************************
 *
 * LE2Word() - Converts the given Little Endian value to machine order value
 *
 * Description:
 *	This function converts the Little Endian value received as an argument to
 *	the machine order value.
 *
 * Returns:
 *	The converted value
 *
 */

#ifdef SK_LITTLE_ENDIAN
#ifndef	SK_USE_REV_DESC
#define LE2Word(value) (value)
#else	/* SK_USE_REV_DESC */
#define LE2Word(value)				\
	((((value)<< 8L) & 0xff00) +	\
	 (((value)>> 8L) & 0x00ff))
#endif	/* SK_USE_REV_DESC */

#else	/* !SK_LITTLE_ENDIAN */
#ifndef	SK_USE_REV_DESC
#define LE2Word(value)				\
	((((value)<< 8L) & 0xff00) +	\
	 (((value)>> 8L) & 0x00ff))
#else	/* SK_USE_REV_DESC */
#define LE2Word(value) (value)
#endif	/* SK_USE_REV_DESC */
#endif	/* !SK_LITTLE_ENDIAN */

/******************************************************************************
 *
 * Word2LE() - Converts the given value to a Little Endian value
 *
 * Description:
 *	This function converts the value received as an argument to a Little Endian
 *	value on Big Endian machines. If the machine running the code is Little
 *	Endian, then no conversion is done.
 *
 * Returns:
 *	The converted value
 *
 */

#ifdef SK_LITTLE_ENDIAN
#ifndef	SK_USE_REV_DESC
#define Word2LE(value) (value)
#else	/* SK_USE_REV_DESC */
#define Word2LE(value)				\
	((((value)<< 8L) & 0xff00) +	\
	 (((value)>> 8L) & 0x00ff))
#endif	/* SK_USE_REV_DESC */

#else	/* !SK_LITTLE_ENDIAN */
#ifndef	SK_USE_REV_DESC
#define Word2LE(value)				\
	((((value)<< 8L) & 0xff00) +	\
	 (((value)>> 8L) & 0x00ff))
#else	/* SK_USE_REV_DESC */
#define Word2LE(value) (value)
#endif	/* SK_USE_REV_DESC */
#endif	/* !SK_LITTLE_ENDIAN */

/******************************************************************************
 *
 * Transmit list element macros
 *
 */

#define TXLE_SET_ADDR(pLE, Addr)	\
	((pLE)->Tx.TxUn.BufAddr = DWord2LE(Addr))
#define TXLE_SET_LSLEN(pLE, Len)	\
	((pLE)->Tx.TxUn.LargeSend.Length = Word2LE(Len))
#define TXLE_SET_STACS(pLE, Start)	\
	((pLE)->Tx.TxUn.ChkSum.TxTcpSp = Word2LE(Start))
#define TXLE_SET_WRICS(pLE, Write)	\
	((pLE)->Tx.TxUn.ChkSum.TxTcpWp = Word2LE(Write))
#define TXLE_SET_INICS(pLE, Ini)	((pLE)->Tx.Send.InitCsum = Word2LE(Ini))
#define TXLE_SET_LEN(pLE, Len)		((pLE)->Tx.Send.BufLen = Word2LE(Len))
#define TXLE_SET_VLAN(pLE, Vlan)	((pLE)->Tx.Send.VlanTag = Word2LE(Vlan))
#define TXLE_SET_LCKCS(pLE, Lock)	((pLE)->Tx.ControlFlags = (Lock))
#define TXLE_SET_CTRL(pLE, Ctrl)	((pLE)->Tx.ControlFlags = (Ctrl))
#define TXLE_SET_OPC(pLE, Opc)		((pLE)->Tx.Opcode = (Opc))

#define TXLE_GET_ADDR(pLE)		LE2DWord((pLE)->Tx.TxUn.BufAddr)
#define TXLE_GET_LSLEN(pLE)		LE2Word((pLE)->Tx.TxUn.LargeSend.Length)
#define TXLE_GET_STACS(pLE)		LE2Word((pLE)->Tx.TxUn.ChkSum.TxTcpSp)
#define TXLE_GET_WRICS(pLE)		LE2Word((pLE)->Tx.TxUn.ChkSum.TxTcpWp)
#define TXLE_GET_INICS(pLE)		LE2Word((pLE)->Tx.Send.InitCsum)
#define TXLE_GET_LEN(pLE)		LE2Word((pLE)->Tx.Send.BufLen)
#define TXLE_GET_VLAN(pLE)		LE2Word((pLE)->Tx.Send.VlanTag)
#define TXLE_GET_LCKCS(pLE)		((pLE)->Tx.ControlFlags)
#define TXLE_GET_CTRL(pLE)		((pLE)->Tx.ControlFlags)
#define TXLE_GET_OPC(pLE)		((pLE)->Tx.Opcode)

	/* Yukon-Extreme only */
#define TXLE_SET_LSOV2(pLE, Len)	\
	((pLE)->Tx.TxUn.LsoV2Len = DWord2LE(Len))
#define TXLE_SET_MSSVAL(pLE, Val)	\
((pLE)->Tx.TxUn.Mss.TxMssVal = Word2LE(Val))

#define TXLE_GET_LSOV2(pLE)		LE2DWord((pLE)->Tx.TxUn.LsoV2Len)
#define TXLE_GET_MSSVAL(pLE)	LE2Word((pLE)->Tx.TxUn.Mss.TxMssVal)

/******************************************************************************
 *
 * Receive list element macros
 *
 */

#define RXLE_SET_ADDR(pLE, Addr)	\
	((pLE)->Rx.RxUn.BufAddr = (SK_U32)DWord2LE(Addr))
#define RXLE_SET_STACS2(pLE, Offs)	\
	((pLE)->Rx.RxUn.ChkSum.RxTcpSp2 = Word2LE(Offs))
#define RXLE_SET_STACS1(pLE, Offs)	\
	((pLE)->Rx.RxUn.ChkSum.RxTcpSp1 = Word2LE(Offs))
#define RXLE_SET_LEN(pLE, Len)		((pLE)->Rx.BufferLength = Word2LE(Len))
#define RXLE_SET_CTRL(pLE, Ctrl)	((pLE)->Rx.ControlFlags = (Ctrl))
#define RXLE_SET_OPC(pLE, Opc)		((pLE)->Rx.Opcode = (Opc))

#define RXLE_GET_ADDR(pLE)		LE2DWord((pLE)->Rx.RxUn.BufAddr)
#define RXLE_GET_STACS2(pLE)	LE2Word((pLE)->Rx.RxUn.ChkSum.RxTcpSp2)
#define RXLE_GET_STACS1(pLE)	LE2Word((pLE)->Rx.RxUn.ChkSum.RxTcpSp1)
#define RXLE_GET_LEN(pLE)		LE2Word((pLE)->Rx.BufferLength)
#define RXLE_GET_CTRL(pLE)		((pLE)->Rx.ControlFlags)
#define RXLE_GET_OPC(pLE)		((pLE)->Rx.Opcode)

/******************************************************************************
 *
 * Status list element macros
 *
 */

#define STLE_SET_OPC(pLE, Opc)		((pLE)->St.Opcode = (Opc))

#define STLE_GET_FRSTATUS(pLE)	LE2DWord((pLE)->St.StUn.StRxStatWord)
#define STLE_GET_TIST(pLE)		LE2DWord((pLE)->St.StUn.StRxTimeStamp)
#define STLE_GET_TCP1(pLE)		LE2Word((pLE)->St.StUn.StRxTCPCSum.RxTCPSum1)
#define STLE_GET_TCP2(pLE)		LE2Word((pLE)->St.StUn.StRxTCPCSum.RxTCPSum2)
#define STLE_GET_LEN(pLE)		LE2Word((pLE)->St.Stat.BufLen)
#define STLE_GET_VLAN(pLE)		LE2Word((pLE)->St.Stat.VlanTag)

/*
 * SKLE_GET_LINK() returns the link field including TCP checksum information.
 * To select the port from the link field use (STLE_GET_LINK(pLe) & 1)
 */
#define STLE_GET_LINK(pLE)		((pLE)->St.Link)
#define STLE_GET_OPC(pLE)		((pLE)->St.Opcode)
#define STLE_GET_DONE_IDX(pLE,LowVal,HighVal) {			\
	(LowVal) = LE2DWord((pLE)->St.StUn.StTxStatLow);	\
	(HighVal) = LE2Word((pLE)->St.Stat.StTxStatHi);		\
}
#define STLE_GET_TXDONE1_IDX(pLE) ((pLE)->St.StUn.TxDone.TxDone1&STLE_TXA1_MSKL)
#define STLE_GET_TXDONE2_IDX(pLE) ((pLE)->St.StUn.TxDone.TxDone2&STLE_TXA1_MSKL)
#define STLE_GET_TXDONE3_IDX(pLE) ((pLE)->St.Stat.TxDone3&STLE_TXA1_MSKL)
#define STLE_GET_TXDONE_VALID_BITS(pLE) (STLE_GET_LINK(pLE)&7)
#define STLE_GET_TXDONE_BASE(pLE) (((pLE)->St.Opcode-OP_TXINDEXLE)*3)

#define STLE_GET_RSS(pLE)		LE2DWord((pLE)->St.StUn.StRxRssValue)
#define STLE_GET_IPV6BIT(pLE)	((pLE)->St.Stat.Rss.FlagField & RSS_IPV6_FLAG)
#define STLE_GET_IPBIT(pLE)		((pLE)->St.Stat.Rss.FlagField & RSS_IP_FLAG)
#define STLE_GET_TCPBIT(pLE)	((pLE)->St.Stat.Rss.FlagField & RSS_TCP_FLAG)

/* Yukon-Extreme only */
#define STLE_GET_MX_STAT(pLE)	((pLE)->St.StUn.StMacSecWord)

/* Yukon-Ext CSum Status defines (Rx Status, Link field) */
#define CSS_GET_PORT(Link)			(Link & CSS_LINK_BIT)
#define CSS_IS_IPV4(Link)			(Link & CSS_ISIPV4)
#define CSS_IPV4_CSUM_OK(Link)		(Link & CSS_IPV4CSUMOK)
#define CSS_IS_IPV6(Link)			(Link & CSS_ISIPV6)
#define CSS_IS_IPFRAG(Link)			(Link & CSS_ISIPFRAG)
#define CSS_IS_TCP(Link)			(Link & CSS_ISTCP)
#define CSS_IS_UDP(Link)			(Link & CSS_ISUDP)
#define CSS_TCPUDP_CSUM_OK(Link)	(Link & CSS_TCPUDPCSOK)

/* always take both values as a parameter to avoid typos */
#define STLE_GET_DONE_IDX_TXA1(LowVal,HighVal)			\
	(((LowVal) & STLE_TXA1_MSKL) >> STLE_TXA1_SHIFTL)
#define STLE_GET_DONE_IDX_TXS1(LowVal,HighVal)			\
	((LowVal & STLE_TXS1_MSKL) >> STLE_TXS1_SHIFTL)
#define STLE_GET_DONE_IDX_TXA2(LowVal,HighVal)			\
	(((LowVal & STLE_TXA2_MSKL) >> STLE_TXA2_SHIFTL) +	\
	((HighVal & STLE_TXA2_MSKH) << STLE_TXA2_SHIFTH))
#define STLE_GET_DONE_IDX_TXS2(LowVal,HighVal)			\
	((HighVal & STLE_TXS2_MSKH) >> STLE_TXS2_SHIFTH)

/* check also length in MAC status with transferred length in LE */
#define SK_Y2_RXSTAT_CHECK_PKT(Len, RxStat, IsOk)					\
	(IsOk) = (((RxStat) & GMR_FS_RX_OK) != 0) &&					\
			 (((RxStat) & GMR_FS_ANY_ERR) == 0) &&					\
			((((RxStat) & GMR_FS_LEN_MSK) >> GMR_FS_LEN_SHIFT) == (Len))

/******************************************************************************
 *
 * Polling unit list element macros
 *
 * NOTE: the Idx must be <= 0xfff and PU_PUTIDX_VALID makes them valid
 *
 */

#ifdef USE_POLLING_UNIT

#define POLE_SET_OPC(pLE, Opc)		((pLE)->Sa.Opcode = (Opc))
#define POLE_SET_LINK(pLE, Port)	((pLE)->Sa.Link = (Port))
#define POLE_SET_RXIDX(pLE, Idx)	((pLE)->Sa.RxIdxVld = Word2LE(Idx))
#define POLE_SET_TXAIDX(pLE, Idx)	((pLE)->Sa.TxAIdxVld = Word2LE(Idx))
#define POLE_SET_TXSIDX(pLE, Idx)	((pLE)->Sa.TxSIdxVld = Word2LE(Idx))

#define POLE_GET_OPC(pLE)		((pLE)->Sa.Opcode)
#define POLE_GET_LINK(pLE)		((pLE)->Sa.Link)
#define POLE_GET_RXIDX(pLE)		LE2Word((pLE)->Sa.RxIdxVld)
#define POLE_GET_TXAIDX(pLE)	LE2Word((pLE)->Sa.TxAIdxVld)
#define POLE_GET_TXSIDX(pLE)	LE2Word((pLE)->Sa.TxSIdxVld)

#endif /* USE_POLLING_UNIT */

/******************************************************************************
 *
 * Debug macros for list elements
 *
 */

#ifdef DEBUG

#define SK_DBG_DUMP_RX_LE(pLE)	{										\
	SK_U8	Opcode;														\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("=== RX_LIST_ELEMENT @addr: %p cont: %02x %02x %02x %02x %02x %02x %02x %02x\n",	\
		pLE, ((SK_U8 *) pLE)[0], ((SK_U8 *) pLE)[1], ((SK_U8 *) pLE)[2],\
		((SK_U8 *) pLE)[3], ((SK_U8 *) pLE)[4], ((SK_U8 *) pLE)[5],		\
		((SK_U8 *) pLE)[6], ((SK_U8 *) pLE)[7]));						\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("\t (16bit) %04x %04x %04x %04x\n",							\
		((SK_U16 *) pLE)[0], ((SK_U16 *) pLE)[1], ((SK_U16 *) pLE)[2],	\
		((SK_U16 *) pLE)[3]));											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("\t (32bit) %08x %08x\n",										\
		((SK_U32 *) pLE)[0], ((SK_U32 *) pLE)[1]));						\
	Opcode = RXLE_GET_OPC(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("\tOwn belongs to %s\n", ((Opcode & HW_OWNER) == HW_OWNER) ?	\
		 "Hardware" : "Software"));										\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("\tOpc: 0x%x ",Opcode));										\
	switch (Opcode & (~HW_OWNER)) {										\
	case OP_BUFFER:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tOP_BUFFER\n"));											\
		break;															\
	case OP_PACKET:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tOP_PACKET\n"));											\
		break;															\
	case OP_ADDR64:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tOP_ADDR64\n"));											\
		break;															\
	case OP_TCPSTART:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tOP_TCPPAR\n"));											\
		break;															\
	case SW_OWNER:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tunused LE\n"));											\
		break;															\
	default:															\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tunknown Opcode!!!\n"));									\
	}																	\
	if ((Opcode & OP_BUFFER) == OP_BUFFER) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tControl: 0x%x\n", RXLE_GET_CTRL(pLE)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tBufLen: 0x%x\n", RXLE_GET_LEN(pLE)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tLowAddr: 0x%x\n", RXLE_GET_ADDR(pLE)));					\
	}																	\
	if ((Opcode & OP_ADDR64) == OP_ADDR64) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tHighAddr: 0x%x\n", RXLE_GET_ADDR(pLE)));				\
	}																	\
	if ((Opcode & OP_TCPSTART) == OP_TCPSTART) {						\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tTCP Sum Start 1 : 0x%x\n", RXLE_GET_STACS1(pLE)));		\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,					\
			("\tTCP Sum Start 2 : 0x%x\n", RXLE_GET_STACS2(pLE)));		\
	}																	\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_RX,						\
		("=====================\n"));									\
}

#define SK_DBG_DUMP_TX_LE(pLE)	{										\
	SK_U8	Opcode;														\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("=== TX_LIST_ELEMENT @addr: %p cont: %02x %02x %02x %02x %02x %02x %02x %02x\n",	\
		pLE, ((SK_U8 *) pLE)[0], ((SK_U8 *) pLE)[1], ((SK_U8 *) pLE)[2],\
		((SK_U8 *) pLE)[3], ((SK_U8 *) pLE)[4], ((SK_U8 *) pLE)[5],		\
		((SK_U8 *) pLE)[6], ((SK_U8 *) pLE)[7]));						\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("\t (16bit) %04x %04x %04x %04x\n",							\
		((SK_U16 *) pLE)[0], ((SK_U16 *) pLE)[1], ((SK_U16 *) pLE)[2],	\
		((SK_U16 *) pLE)[3]));											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("\t (32bit) %08x %08x\n",										\
		((SK_U32 *) pLE)[0], ((SK_U32 *) pLE)[1]));						\
	Opcode = TXLE_GET_OPC(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("\tOwn belongs to %s\n", ((Opcode & HW_OWNER) == HW_OWNER) ?	\
		"Hardware" : "Software"));										\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("\tOpc: 0x%x ",Opcode));										\
	switch (Opcode & (~HW_OWNER)) {										\
	case OP_TCPCHKSUM:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPCHKSUM\n"));										\
		break;															\
	case OP_TCPIS:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPIS\n"));											\
		break;															\
	case OP_TCPLCK:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPLCK\n"));											\
		break;															\
	case OP_TCPLW:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPLW\n"));											\
		break;															\
	case OP_TCPLSW:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPLSW\n"));											\
		break;															\
	case OP_TCPLISW:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_TCPLISW\n"));										\
		break;															\
	case OP_ADDR64:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_ADDR64\n"));											\
		break;															\
	case OP_VLAN:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_VLAN\n"));											\
		break;															\
	case OP_ADDR64VLAN:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_ADDR64VLAN\n"));										\
		break;															\
	case OP_LRGLEN:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_LRGLEN\n"));											\
		break;															\
	case OP_LRGLENVLAN:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_LRGLENVLAN\n"));										\
		break;															\
	case OP_BUFFER:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_BUFFER\n"));											\
		break;															\
	case OP_PACKET:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_PACKET\n"));											\
		break;															\
	case OP_LARGESEND:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_LARGESEND\n"));										\
		break;															\
	case OP_MSS:		/* Yukon-Extreme only */						\
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_MSS\n"))												\
		break;															\
	case OP_MSSVLAN:	/* Yukon-Extreme only */						\
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_MSSVLAN\n"))											\
		break;															\
	case OP_LSOV2:		/* Yukon-Extreme only */						\
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tOP_LSOV2\n"))											\
		break;															\
	case SW_OWNER:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tunused LE\n"));											\
		break;															\
	default:															\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tunknown Opcode!!!\n"));									\
	}																	\
	if ((Opcode & OP_BUFFER) == OP_BUFFER) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tControl: 0x%x\n", TXLE_GET_CTRL(pLE)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tBufLen: 0x%x\n", TXLE_GET_LEN(pLE)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tLowAddr: 0x%x\n", TXLE_GET_ADDR(pLE)));					\
	}																	\
	if ((Opcode & OP_ADDR64) == OP_ADDR64) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tHighAddr: 0x%x\n", TXLE_GET_ADDR(pLE)));				\
	}																	\
	if ((Opcode & OP_VLAN) == OP_VLAN) {								\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tVLAN Id: 0x%x\n", TXLE_GET_VLAN(pLE)));					\
	}																	\
	if ((Opcode & OP_LRGLEN) == OP_LRGLEN) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tLarge send length: 0x%x\n", TXLE_GET_LSLEN(pLE)));		\
	}																	\
	if ((Opcode & OP_MSS) == OP_MSS) {		/* Yukon-Extreme only */	\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tMSS value: 0x%x\n", TXLE_GET_MSSVAL(pLE)));				\
	}																	\
	if ((Opcode & OP_LSOV2) == OP_LSOV2) {	/* Yukon-Extreme only */	\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,					\
			("\tLSOv2 lenght: 0x%x\n", TXLE_GET_LSOV2(pLE)));			\
	}																	\
	if ((Opcode &(~HW_OWNER)) <= OP_ADDR64) {							\
		if ((Opcode & OP_TCPWRITE) == OP_TCPWRITE) {					\
			SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,				\
				("\tTCP Sum Write: 0x%x\n", TXLE_GET_WRICS(pLE)));		\
		}																\
		if ((Opcode & OP_TCPSTART) == OP_TCPSTART) {					\
			SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,				\
				("\tTCP Sum Start: 0x%x\n", TXLE_GET_STACS(pLE)));		\
		}																\
		if ((Opcode & OP_TCPINIT) == OP_TCPINIT) {						\
			SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,				\
				("\tTCP Sum Init: 0x%x\n", TXLE_GET_INICS(pLE)));		\
		}																\
		if ((Opcode & OP_TCPLCK) == OP_TCPLCK) {						\
			SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,				\
				("\tTCP Sum Lock: 0x%x\n", TXLE_GET_LCKCS(pLE)));		\
		}																\
	}																	\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_TX,						\
		("=====================\n"));									\
}

#define SK_DBG_DUMP_ST_LE(pLE)	{										\
	SK_U8	Opcode;														\
	SK_U16	HighVal;													\
	SK_U32	LowVal;														\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("=== ST_LIST_ELEMENT @addr: %p contains: %02x %02x %02x %02x %02x %02x %02x %02x\n",\
		pLE, ((SK_U8 *) pLE)[0], ((SK_U8 *) pLE)[1], ((SK_U8 *) pLE)[2],\
		((SK_U8 *) pLE)[3], ((SK_U8 *) pLE)[4], ((SK_U8 *) pLE)[5],		\
		((SK_U8 *) pLE)[6], ((SK_U8 *) pLE)[7]));						\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("\t (16bit) %04x %04x %04x %04x\n",							\
		((SK_U16 *) pLE)[0], ((SK_U16 *) pLE)[1], ((SK_U16 *) pLE)[2],	\
		((SK_U16 *) pLE)[3]));											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("\t (32bit) %08x %08x\n",										\
		((SK_U32 *) pLE)[0], ((SK_U32 *) pLE)[1]));						\
	Opcode = STLE_GET_OPC(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("\tOwn belongs to %s\n", ((Opcode & HW_OWNER) == SW_OWNER) ?	\
		"Hardware" : "Software"));										\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("\tOpc: 0x%x",	Opcode));										\
	Opcode &= (~HW_OWNER);												\
	switch (Opcode) {													\
	case OP_RXSTAT:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXSTAT\n"));											\
		break;															\
	case OP_RXTIMESTAMP:												\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXTIMESTAMP\n"));									\
		break;															\
	case OP_RXVLAN:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXVLAN\n"));											\
		break;															\
	case OP_RXCHKS:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXCHKS\n"));											\
		break;															\
	case OP_RXCHKSVLAN:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXCHKSVLAN\n"));										\
		break;															\
	case OP_RXTIMEVLAN:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RXTIMEVLAN\n"));										\
		break;															\
	case OP_RSS_HASH:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_RSS_HASH\n"));										\
		break;															\
	case OP_TXINDEXLE:													\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_TXINDEXLE\n"));										\
		break;															\
	case OP_MACSEC:		/* Yukon-Extreme only */						\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_MACSEC\n"));											\
		break;															\
	case OP_MACSECVLAN:	/* Yukon-Extreme only */						\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tOP_MACSECVLAN\n"));										\
		break;															\
	case HW_OWNER:														\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tunused LE\n"));											\
		break;															\
	default:															\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tunknown status list element!!!\n"));					\
	}																	\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("\tPort: %c\n", 'A' + (CSS_GET_PORT(STLE_GET_LINK(pLE)))));	\
	if (Opcode == OP_RXSTAT) {											\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tFrameLen: 0x%x\n", STLE_GET_LEN(pLE)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tFrameStat: 0x%x\n", STLE_GET_FRSTATUS(pLE)));			\
		if (HW_IS_EXT_LE_FORMAT(pAc)) {									\
			SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,				\
				("\tChecksum Status Field: 0x%x\n", STLE_GET_LINK(pLE)));\
		}																\
	}																	\
	if ((Opcode & OP_RXVLAN) == OP_RXVLAN) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tVLAN Id: 0x%x\n", STLE_GET_VLAN(pLE)));					\
	}																	\
	if ((Opcode & OP_RXTIMESTAMP) == OP_RXTIMESTAMP) {					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTimestamp: 0x%x\n", STLE_GET_TIST(pLE)));				\
	}																	\
	if ((Opcode & OP_RXCHKS) == OP_RXCHKS) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTCP: 0x%x 0x%x\n", STLE_GET_TCP1(pLE),					\
			STLE_GET_TCP2(pLE)));										\
	}																	\
	if (Opcode == OP_TXINDEXLE) {										\
		STLE_GET_DONE_IDX(pLE, LowVal, HighVal);						\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTx Index TxA1: 0x%x\n",									\
			STLE_GET_DONE_IDX_TXA1(LowVal,HighVal)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTx Index TxS1: 0x%x\n",									\
			STLE_GET_DONE_IDX_TXS1(LowVal,HighVal)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTx Index TxA2: 0x%x\n",									\
			STLE_GET_DONE_IDX_TXA2(LowVal,HighVal)));					\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tTx Index TxS2: 0x%x\n",									\
			STLE_GET_DONE_IDX_TXS2(LowVal,HighVal)));					\
	}																	\
	if ((Opcode & OP_MACSEC) == OP_MACSEC) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,					\
			("\tMACSec Status Field\n", STLE_GET_MX_STAT(pLE)));		\
	}																	\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,						\
		("=====================\n"));									\
}

#ifdef USE_POLLING_UNIT
#define SK_DBG_DUMP_PO_LE(pLE)	{										\
	SK_U8	Opcode;														\
	SK_U16	Idx;														\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("=== PO_LIST_ELEMENT @addr: %p cont: %02x %02x %02x %02x %02x %02x %02x %02x\n",	\
		pLE, ((SK_U8 *) pLE)[0], ((SK_U8 *) pLE)[1], ((SK_U8 *) pLE)[2],\
		((SK_U8 *) pLE)[3], ((SK_U8 *) pLE)[4], ((SK_U8 *) pLE)[5],		\
		((SK_U8 *) pLE)[6], ((SK_U8 *) pLE)[7]));						\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\t (16bit) %04x %04x %04x %04x\n",							\
		((SK_U16 *) pLE)[0], ((SK_U16 *) pLE)[1], ((SK_U16 *) pLE)[2],	\
		((SK_U16 *) pLE)[3]));											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\t (32bit) %08x %08x\n",										\
		((SK_U32 *) pLE)[0], ((SK_U32 *) pLE)[1]));						\
	Opcode = POLE_GET_OPC(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		 ("\tOwn belongs to %s\n", ((Opcode & HW_OWNER) == HW_OWNER) ?	\
		  "Hardware" : "Software"));									\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		 ("\tOpc: 0x%x ",Opcode));										\
	if ((Opcode & ~HW_OWNER) == OP_PUTIDX) {							\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,					\
			("\tOP_PUTIDX\n"));											\
	}																	\
	else {																\
		SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,					\
			("\tunknown Opcode!!!\n"));									\
	}																	\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\tPort %c\n", 'A' + POLE_GET_LINK(pLE)));						\
	Idx = POLE_GET_TXAIDX(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\tTxA Index is 0x%X and %svalid\n", Idx,						\
		(Idx & PU_PUTIDX_VALID) ? "" : "not "));						\
	Idx = POLE_GET_TXSIDX(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\tTxS Index is 0x%X and %svalid\n", Idx,						\
		(Idx & PU_PUTIDX_VALID) ? "" : "not "));						\
	Idx = POLE_GET_RXIDX(pLE);											\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("\tRx Index is 0x%X and %svalid\n", Idx,						\
		(Idx & PU_PUTIDX_VALID) ? "" : "not "));						\
	SK_DBG_MSG(pAc, SK_DBGMOD_HWM, SK_DBGCAT_INIT,						\
		("=====================\n"));									\
}
#endif /* USE_POLLING_UNIT */

#else	/* !DEBUG */

#define SK_DBG_DUMP_RX_LE(pLE)
#define SK_DBG_DUMP_TX_LE(pLE)
#define SK_DBG_DUMP_ST_LE(pLE)
#define SK_DBG_DUMP_PO_LE(pLE)

#endif	/* !DEBUG */

/******************************************************************************
 *
 * Macros for list element tables
 *
 */

#define LE_SIZE sizeof(SK_HWLE)
#define LE_TAB_SIZE(NumElements)	((NumElements) * LE_SIZE)

/* total number of list elements in table */
#define NUM_LE_IN_TABLE(pTable)		((pTable)->Num)

/* Number of unused list elements in table
 * this macro always returns the number of free listelements - 1
 * this way we want to guarantee that always one LE remains unused
 */
#define NUM_FREE_LE_IN_TABLE(pTable)								\
	( ((pTable)->Put >= (pTable)->Done) ?							\
	(NUM_LE_IN_TABLE(pTable) - (pTable)->Put + (pTable)->Done - 1) :\
	((pTable)->Done - (pTable)->Put - 1) )

/* get next unused Rx list element */
#define GET_RX_LE(pLE, pTable) {									\
	pLE = &(pTable)->pLETab[(pTable)->Put];							\
	(pTable)->Put = ((pTable)->Put + 1) & (NUM_LE_IN_TABLE(pTable) - 1);\
}

/* get next unused Tx list element */
#define GET_TX_LE(pLE, pTable)	GET_RX_LE(pLE, pTable)

/* get next status list element expected to be finished by HW */
#define GET_ST_LE(pLE, pTable) {									\
	pLE = &(pTable)->pLETab[(pTable)->Done];						\
	(pTable)->Done = ((pTable)->Done +1) & (NUM_LE_IN_TABLE(pTable) - 1);\
}

#ifdef USE_POLLING_UNIT
/* get next polling unit list element for port */
#define GET_PO_LE(pLE, pTable, Port)	(pLE = &(pTable)->pLETab[(Port)])
}
#endif /* USE_POLLING_UNIT */

#define GET_PUT_IDX(pTable)			((pTable)->Put)

#define UPDATE_HWPUT_IDX(pTable)	((pTable)->HwPut = (pTable)->Put)

/*
 * get own bit of next status LE
 * if the result is != 0 there has been at least one status LE finished
 */
#define OWN_OF_FIRST_LE(pTable)									\
	(STLE_GET_OPC(&(pTable)->pLETab[(pTable)->Done]) & HW_OWNER)

#define SET_DONE_INDEX(pTable, Idx)	(pTable)->Done = (Idx)

#define GET_DONE_INDEX(pTable)	((pTable)->Done)

#ifdef SAFE_BUT_SLOW

/* check own bit of LE before current done idx */
#define CHECK_STLE_OVERFLOW(pTable, IsOk) {						\
		unsigned i;												\
		if ((i = (pTable)->Done) == 0) {						\
			i = NUM_LE_IN_TABLE(pTable);						\
		}														\
		else {													\
			i = i - 1;											\
		}														\
		if (STLE_GET_OPC(&(pTable)->pLETab[i]) == HW_OWNER) {	\
			(IsOk) = SK_TRUE;									\
		}														\
		else {													\
			(IsOk) = SK_FALSE;									\
		}														\
	}


/*
 * for Yukon-2 the hardware is not polling the list elements, so it
 * is not necessary to change the own-bit of Rx or Tx LEs before
 * reusing them
 * but it might make debugging easier if one simply can see whether
 * a LE has been worked on
 */

#define CLEAR_LE_OWN(pTable, Idx)								\
	STLE_SET_OPC(&(pTable)->pLETab[(Idx)], SW_OWNER)

/*
 * clear all own bits starting from old done index up to the LE before
 * the new done index
 */
#define CLEAR_LE_OWN_FROM_DONE_TO(pTable, To) {					\
		int i;													\
		i = (pTable)->Done;										\
		while (i != To) {										\
			CLEAR_LE_OWN(pTable, i);							\
			i = (i + 1) & (NUM_LE_IN_TABLE(pTable) - 1);		\
		}														\
	}

#else	/* !SAFE_BUT_SLOW */

#define CHECK_STLE_OVERFLOW(pTable, IsOk)
#define CLEAR_LE_OWN(pTable, Idx)
#define CLEAR_LE_OWN_FROM_DONE_TO(pTable, To)

#endif	/* !SAFE_BUT_SLOW */


/* typedefs ******************************************************************/

typedef struct s_LetRxTx {
	SK_U16	VlanId;			/* VLAN Id given down last time */
	SK_U16	TcpWp;			/* TCP Checksum Write Position */
	SK_U16	TcpSp1;			/* TCP Checksum Calculation Start Position 1 */
	SK_U16	TcpSp2;			/* TCP Checksum Calculation Start Position 2 */
	SK_U16	MssValue;		/* Maximum Segment Size */
	SK_U16	Reserved1;		/* reserved word for future extensions */
	SK_U16	Reserved2;		/* reserved word for future extensions */
	SK_U16	Reserved3;		/* reserved word for future extensions */
} SK_LET_RX_TX;

typedef struct s_LetStat {
	SK_U32	RxTimeStamp;	/* Receive Timestamp */
	SK_U32	RssHashValue;	/* RSS Hash Value */
	SK_BOOL	RssIsIpV6;		/* RSS Hash Value: IPv6 packet detected */
	SK_BOOL	RssIsIp;		/* RSS Hash Value: IP packet detected */
	SK_BOOL	RssIsTcp;		/* RSS Hash Value: IP+TCP packet detected */
	SK_BOOL	CalcHashIpV4;	/* RSS is computed over source IPv4 address */
							/* and destination IPv4 address */
	SK_BOOL	CalcHashTCPIPv4;/* RSS is computed over source IPv4 address, */
							/* destination IPv4 address, source TCP port and */
							/* destination TCP port */
	SK_BOOL	CalcHashIPv6;	/* RSS is computed over source IPv6 address */
							/* and destination IPv6 address */
	SK_BOOL	CalcHashTCPIPv6;/* RSS is computed over source IPv6 address, */
							/* destination IPv6 address, source TCP port and */
							/* destination TCP port */
	SK_BOOL	CalcHashIPv6Ex;	/* RSS is computed over home address */
							/* (if not present, source IPv6 address is used) */
							/* and routing header type 2 (if not present, */
							/* destination IPv6 address is used) */
	SK_BOOL	CalcHashTCPIPv6Ex;	/* RSS is computed over home address */
							/* (if not present, source IPv6 address is used) */
							/* and routing header type 2 (if not present, */
							/* destination IPv6 address is used), */
							/* source TCP port and destiantion TCP port */
	SK_U16	VlanId;			/* VLAN Id given received by Status BMU */
	SK_U16	TcpSum1;		/* TCP checksum 1 (status BMU) */
	SK_U16	TcpSum2;		/* TCP checksum 2 (status BMU) */
	SK_U32	MacSecStatus;	/* MAC Security status for packet */
} SK_LET_STAT;

typedef union s_LetBmuSpec {
	SK_LET_RX_TX	RxTx;	/* Rx/Tx BMU specific variables */
	SK_LET_STAT		Stat;	/* Status BMU specific variables */
} SK_LET_BMU_S;

typedef struct s_LeTable {
	/* all LEs between Done and HWPut are owned by the hardware */
	/* all LEs between Put and Done can be used from software */
	/* all LEs between HWPut and Put are currently processed in DriverSend */
	unsigned Done;			/* done index - consumed from HW and available */
	unsigned Put;			/* put index - to be given to hardware */
	unsigned HwPut;			/* put index actually given to hardware */
	unsigned Num;			/* total number of list elements */
	SK_HWLE *pLETab;		/* virtual address of list element table */
	SK_U32	pPhyLETABLow;	/* physical address of list element table */
	SK_U32	pPhyLETABHigh;	/* physical address of list element table */
	/* values to remember in order to save some LEs */
	SK_U32	BufHighAddr;	/* high address given down last time */
	SK_LET_BMU_S Bmu;		/* contains BMU specific information */
	SK_U32	Private;		/* driver private variable free usable */
	SK_U16	TcpInitCsum;	/* init checksum */
#if defined(SK_LSO_V2) && defined(SK_EXTREME)
	unsigned DoneCorrection;	/* number of LSOv2 LEs send modulo Num */
	unsigned BadDone;			/* real done index from HW - in Done is a corrected value */
#endif
} SK_LE_TABLE;

/* function prototypes ********************************************************/

#ifndef	SK_KR_PROTO

/*
 * public functions in sky2le.c
 */
extern void SkGeY2SetPutIndex(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int StartAddrPrefUnit,
	SK_LE_TABLE *pLETab);

extern void SkGeY2InitPrefetchUnit(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Queue,
	SK_LE_TABLE *pLETab);

extern void SkGeY2InitStatBmu(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_LE_TABLE *pLETab);

extern void SkGeY2InitPollUnit(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_LE_TABLE *pLETab);

extern void SkGeY2InitSingleLETable(
	SK_AC	*pAC,
	SK_LE_TABLE *pLETab,
	unsigned int NumLE,
	void	*pVMem,
	SK_U32	PMemLowAddr,
	SK_U32	PMemHighAddr);

#else	/* SK_KR_PROTO */
extern void SkGeY2SetPutIndex();
extern void SkGeY2InitPrefetchUnit();
extern void SkGeY2InitStatBmu();
extern void SkGeY2InitPollUnit();
extern void SkGeY2InitSingleLETable();
#endif	/* SK_KR_PROTO */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __INC_SKY2LE_H */

