/******************************************************************************
 *
 * Name:    skgeasfapi.h
 * Project: asf/ipmi
 * Purpose: asf/ipmi interface in windows driver
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

#ifndef _INC_SKGEASFAPI_H_
#define _INC_SKGEASFAPI_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


/* Function definitions */
extern int   SkFwApiGet (SK_AC *pAC , SK_IOC IoC);
extern void  SkFwIsr (SK_AC *pAC , SK_IOC IoC);
extern void  SetRamAddr(SK_AC *, int, int, int);
extern SK_U8 SkAccessRamBuf(SK_AC  *pAC, SK_U32 RamAddrOffset, SK_U32 RamAddrMaxValue);


/* Buffer defines */
#define HOST_ROM_ADDR          0x8300000
#define HOST_READ_OFFSET       0x27000
#define HOST_READ_ADDR         HOST_ROM_ADDR+HOST_READ_OFFSET
#define HOST_READ_QWORD        HOST_READ_OFFSET/8
#define HOST_READ_BUFSIZE      0x5000 /* 20*1024 */
#define HOST_READ_LAST_QWORD   ((HOST_READ_BUFSIZE+0x10)/0x8)+HOST_READ_QWORD
#define HOST_WRITE_ADDR        HOST_READ_ADDR+HOST_READ_BUFSIZE+0x18
#define HOST_WRITE_QWORD       ((HOST_READ_BUFSIZE+0x18)/0x8)+HOST_READ_QWORD
#define HOST_WRITE_BUFSIZE     0x800 /* 2*1024 */
#define HOST_WRITE_LAST_QWORD  ((HOST_WRITE_BUFSIZE+0x8)/0x8)+HOST_WRITE_QWORD

/* Additional defines for new driver/firmware FIFO interface */
#define HOST_READ_DATA_BUFSIZE      0x810  /* 0x800  (data) + 0x10 (pointer/status) */
#define HOST_READ_PACKET_BUFSIZE    0x4800 /* 0x47f0 (data) + 0x10 (pointer/status) */
#define HOST_READ_DATA_QWORD        0x4e01 /* 0x27008/8 */
#define HOST_READ_DATA_LAST_QWORD   0x4f02 /* 0x27810/8 */
#define HOST_READ_PACKET_QWORD      0x4f03 /* 0x27818/8 */
#define HOST_READ_PACKET_LAST_QWORD 0x5802 /* 0x2c010/8 */
#define HOST_WRITE_DATA_QWORD       0x5803 /* 0x2c018/8 */

#define SK_ST_FIFOTYPE        3
#define SK_ST_BUFADDR_LOW     0x0 /* To be defined if needed... */
#define SK_ST_BUFADDR_HIGH    0x0

#define SKGE_DGRAM_CMD         1    /* Command */
#define SKGE_DGRAM_RESULT      2    /* Command result */
#define SKGE_DGRAM_PACKET      3    /* Packet */
#define SKGE_DGRAM_MSG2FW      4    /* Message to FW application */
#define SKGE_DGRAM_MSGFW       5    /* Message from FW application */
#define SKGE_DGRAM_STATUS      7    /* Status from the FW to DRV */
#define SKGE_DGRAM_APP_VERSION 8    /* Version string of the firmware application code */

/* FIFO communication path command section */
#define	FW_CMD_SET_IPV4_CONFIG        0x0001
#define	FW_CMD_SET_LINK_MODE          0x0003

#define	FW_CMD_SET_IPV4_CONFIG_LENGTH 16
#define	FW_CMD_SET_LINK_MODE_LENGTH   5

#define	FW_RESULT                     0x8000

#define	FW_CMD_RESULT_LENGTH      6

#define	FW_RESULT_OK              0
#define	FW_RESULT_ERR             1
#define	FW_RESULT_ERR_UNKNOWN_CMD 2


/* Driver commands */
#define SK_COMMAND_SLEEP           1
#define SK_COMMAND_SA_UPDATE       2


/* Datagram structure */
struct _FwCmdHdr {  /* Main message header */
#ifdef SK_LITTLE_ENDIAN
	SK_U16 Cmd;     /* Command type */
	SK_U16 Len;     /* Complete command length (hdr+payload) */
#else
	SK_U16 Len;     /* Complete command length (hdr+payload) */
	SK_U16 Cmd;     /* Command type */
#endif
};
typedef struct _FwCmdHdr FwCmdHdr;

struct _FifoDgram {
	SK_U16 DgramLen;		/* len FifoDgram + Payload + Padding */
	SK_U16 PayloadLen;		/* len of Payload */
	SK_U32 Id;				/* Id of Payload */
};
typedef struct _FifoDgram FifoDgram;

struct _DgamMsg {			/* main message header */
	SK_U16 cmd;				/* message type */
	SK_U16 len;				/* complete message length (hdr+payload) */
	SK_U16 data;
};
typedef struct _DgamMsg DgamMsg;

struct _CmdSetIpv4Config {  /* Set IPv4 configuration */
	FwCmdHdr Hdr;
	SK_U32   Ip;            /* IPv4 */
	SK_U32   Mask;          /* IPv4 */
	SK_U32   Gateway;       /* IPv4 */
};
typedef struct _CmdSetIpv4Config CmdSetIpv4Config;

struct _FwCmdResult {  /* Pass results back */
	FwCmdHdr Hdr;
#ifdef SK_LITTLE_ENDIAN
	SK_U16   Result;
	SK_U16   Endianess;
#else
	SK_U16   Endianess;
	SK_U16   Result;
#endif
};
typedef struct _FwCmdResult FwCmdResult;


#endif  /* _INC_SKGEASF_H_ */
