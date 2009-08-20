/*
 * Broadcom USB remote download definitions
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: usbrdl.h,v 13.13.2.2 2008/06/19 23:49:33 Exp $
 */

/* cmds */
#define DL_GETSTATE		0	/* returns the rdl_state_t struct */
#define DL_CHECK_CRC		1	/* currently unused */
#define DL_GO			2	/* execute downloaded image */
#define DL_START		3	/* initialize dl state */
#define DL_REBOOT		4	/* reboot the device in 2 seconds */
#define DL_GETVER		5	/* returns the bootrom_id_t struct */
#define DL_GO_PROTECTED		6	/* execute the downloaded code and set reset event
					 * to occur in 2 seconds.  It is the reponsibility
					 * of the downloaded code to clear this event
					 */
#define DL_EXEC			7	/* jump to a supplied address */
#define DL_RESETCFG		8	/* To support single enum on dongle
					 * - Not used by bootloader
					 */
#define DL_DEFER_RESP_OK	9	/* Potentially defer the response to setup
					 * if resp unavailable
					 */


#define	DL_HWCMD_MASK		0xfc	/* Mask for hardware commands: */
#define	DL_RDHW			0x10	/* Read a hardware address */
#define	DL_RDHW32		0x10	/* Read a 32 bit word */
#define	DL_RDHW16		0x11	/* Read 16 bits */
#define	DL_RDHW8		0x12	/* Read an 8 bit byte */
#define	DL_WRHW			0x14	/* Write a hardware address */

#define DL_CMD_RDHW		1	/* read data from a backplane address */
#define DL_CMD_WRHW		2	/* write data to a backplane address */

/* states */
#define DL_WAITING	0	/* waiting to rx first pkt that includes the hdr info */
#define DL_READY	1	/* hdr was good, waiting for more of the compressed image */
#define DL_BAD_HDR	2	/* hdr was corrupted */
#define DL_BAD_CRC	3	/* compressed image was corrupted */
#define DL_RUNNABLE	4	/* download was successfull, waiting for go cmd */
#define DL_START_FAIL	5	/* failed to initialise correctly */
#define DL_NVRAM_TOOBIG	6	/* host specified nvram data exceeds DL_NVRAM value */

#define TIMEOUT		100	/* Timeout for usb commands */

struct bcm_device_id {
	char *name;
	unsigned int vend;
	unsigned int prod;
};

typedef struct {
	unsigned int state;
	unsigned int bytes;
} rdl_state_t;

typedef struct {
	unsigned int chip;
	unsigned int chiprev;
} bootrom_id_t;

/* struct for backplane address read or write command */
typedef struct {
	unsigned int cmd;	/* tag to identify the cmd */
	unsigned int addr;	/* backplane address for write */
	unsigned int len;	/* length of data: 1, 2, 4 bytes */
	unsigned int data;	/* data to write */
} hwacc_t;

typedef void (*exec_fn_t)(void *sih);

#define USB_CTRL_IN (USB_TYPE_VENDOR | 0x80 | USB_RECIP_INTERFACE)
#define USB_CTRL_OUT (USB_TYPE_VENDOR | 0 | USB_RECIP_INTERFACE)

#define RDL_CHUNK	1500  /* size of each dl transfer */

/* bootloader makes special use of trx header "offsets" array */
#define TRX_OFFSETS_DLBASE_IDX  0       /* RAM start address for download */
#define TRX_OFFSETS_JUMPTO_IDX  1       /* RAM address for jumpto after download */
#define TRX_OFFSETS_NVM_LEN_IDX 2       /* Length of appended NVRAM data */
