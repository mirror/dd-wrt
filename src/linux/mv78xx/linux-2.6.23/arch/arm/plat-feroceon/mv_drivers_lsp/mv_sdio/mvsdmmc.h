/*
 *
 *  Copyright (C) All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _MVSDMMC_INCLUDE
#define _MVSDMMC_INCLUDE

#define MVSDMMC_DMA_SIZE		65536

#define MVSDMMC_CMD_TIMEOUT		2 /* 100 usec*/
/*
 * The base MMC clock rate
 */
#define MVSDMMC_CLOCKRATE_MIN		100000
#define MVSDMMC_CLOCKRATE_MAX		50000000

#define MVSDMMC_BASE_FAST_CLOCK		100000000 /* FIXME: Should be according 
to device */


/* SDIO register */
#define SDIO_SYS_ADDR_LOW		0x000
#define SDIO_SYS_ADDR_HI		0x004
#define SDIO_BLK_SIZE			0x008
#define SDIO_BLK_COUNT			0x00c
#define SDIO_ARG_LOW			0x010
#define SDIO_ARG_HI			0x014
#define SDIO_XFER_MODE			0x018
#define SDIO_CMD			0x01c
#define SDIO_RSP(i)			(0x020 + ((i)<<2))
#define SDIO_RSP0			0x020
#define SDIO_RSP1			0x024
#define SDIO_RSP2			0x028
#define SDIO_RSP3			0x02c
#define SDIO_RSP4			0x030
#define SDIO_RSP5			0x034
#define SDIO_RSP6			0x038
#define SDIO_RSP7			0x03c
#define SDIO_BUF_DATA_PORT		0x040
#define SDIO_RSVED			0x044
#define SDIO_PRESENT_STATE0		0x048
#define CARD_BUSY			((unsigned short)0x1 << 1)
#define CMD_INHIBIT			((unsigned short)0x1 << 0)
#define CMD_TXACTIVE			((unsigned short)0x1 << 8)
#define CMD_RXACTIVE			((unsigned short)0x1 << 9)
#define CMD_AUTOCMD12ACTIVE		((unsigned short)0x1 << 14)
#define SDIO_PRESENT_STATE1		0x04c
#define SDIO_HOST_CTRL			0x050
#define SDIO_BLK_GAP_CTRL		0x054
#define SDIO_CLK_CTRL			0x058
#define SDIO_SW_RESET			0x05c
#define SDIO_NOR_INTR_STATUS		0x060
#define SDIO_ERR_INTR_STATUS		0x064
#define SDIO_NOR_STATUS_EN		0x068
#define SDIO_ERR_STATUS_EN		0x06c
#define SDIO_NOR_INTR_EN		0x070
#define SDIO_ERR_INTR_EN		0x074
#define SDIO_AUTOCMD12_ERR_STATUS	0x078
#define SDIO_CURR_BLK_SIZE		0x07c
#define SDIO_CURR_BLK_COUNT		0x080
#define SDIO_AUTOCMD12_ARG_LOW		0x084
#define SDIO_AUTOCMD12_ARG_HI		0x088
#define SDIO_AUTOCMD12_INDEX		0x08c
#define SDIO_AUTO_RSP0			0x090
#define SDIO_AUTO_RSP1			0x094
#define SDIO_AUTO_RSP2			0x098
#define SDIO_CLK_DIV			0x128



/* SDIO_CMD */
#define SDIO_CMD_NO_RESPONSE		(0<<0)
#define SDIO_CMD_136_RESPONSE		(1<<0)
#define SDIO_CMD_48_RESPONSE		(2<<0)
#define SDIO_CMD_48_RESPONSE_BUSY	(3<<0)

#define SDIO_CMD_CHECK_DATACRC16	(1<<2)
#define SDIO_CMD_CHECK_CMDCRC		(1<<3)
#define SDIO_CMD_INDX_CHECK		(1<<4)
#define SDIO_CMD_DATA_PRESENT		(1<<5)
#define SDIO_UNEXPEXTED_RESP		(1<<7)





/* SDIO_XFER_MODE */
#define SDIO_XFER_MODE_STOP_CLK		(1<<5)
#define SDIO_XFER_MODE_HW_WR_DATA_EN	(1<<1)
#define SDIO_XFER_MODE_AUTO_CMD12	(1<<2)
#define SDIO_XFER_MODE_INT_CHK_EN	(1<<3)
#define SDIO_XFER_MODE_TO_HOST		(1<<4)


/************** SDIO_HOST_CTRL  ****/
#define SDIO_HOST_CTRL_PUSH_PULL_EN 		1
#define SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY 	0
#define SDIO_HOST_CTRL_CARD_TYPE_IO_ONLY 	(1<<1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MEM_COMBO 	(2<<1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MMC 	(3<<1)
#define SDIO_HOST_CTRL_BIG_ENDEAN 		(1<<3)
#define SDIO_HOST_CTRL_LSB_FIRST 		(1<<4)
#define SDIO_HOST_CTRL_ID_MODE_LOW_FREQ 	(1<<5)
#define SDIO_HOST_CTRL_HALF_SPEED 		(1<<6)
#define SDIO_HOST_CTRL_DATA_WIDTH_4_BITS 	(1<<9)
#define SDIO_HOST_CTRL_HI_SPEED_EN 		(1<<10)
#define SDIO_HOST_CTRL_TMOUT_SHIFT 		11
#define SDIO_HOST_CTRL_TMOUT_EN 		(1<<15)

#define SDIO_HOST_CTRL_DFAULT_OPEN_DRAIN 	\
		(0xF<<SDIO_HOST_CTRL_TMOUT_SHIFT)
#define SDIO_HOST_CTRL_DFAULT_PUSH_PULL 	\
		((0xF<<SDIO_HOST_CTRL_TMOUT_SHIFT)|SDIO_HOST_CTRL_PUSH_PULL_EN)

/* nor status mask */
#define SDIO_NOR_ERROR			((unsigned short)0x1 << 15)
#define SDIO_NOR_UNEXP_RSP		((unsigned short)0x1 << 14)
#define SDIO_NOR_AUTOCMD12_DONE		((unsigned short)0x1 << 13)
#define SDIO_NOR_SUSPENSE_ON		((unsigned short)0x1 << 12)
#define SDIO_NOR_LMB_FF_8W_AVAIL	((unsigned short)0x1 << 11)
#define SDIO_NOR_LMB_FF_8W_FILLED	((unsigned short)0x1 << 10)
#define SDIO_NOR_READ_WAIT_ON		((unsigned short)0x1 << 9)
#define SDIO_NOR_CARD_INT		((unsigned short)0x1 << 8)
#define SDIO_NOR_READ_READY		((unsigned short)0x1 << 5)
#define SDIO_NOR_WRITE_READY		((unsigned short)0x1 << 4)
#define SDIO_NOR_DMA_INI		((unsigned short)0x1 << 3)
#define SDIO_NOR_BLK_GAP_EVT		((unsigned short)0x1 << 2)
#define SDIO_NOR_XFER_DONE		((unsigned short)0x1 << 1)
#define SDIO_NOR_CMD_DONE		((unsigned short)0x1 << 0)

/* err status mask */
#define SDIO_ERR_CRC_STATUS		((unsigned short)0x1 << 14)
#define SDIO_ERR_CRC_STARTBIT		((unsigned short)0x1 << 13)
#define SDIO_ERR_CRC_ENDBIT		((unsigned short)0x1 << 12)
#define SDIO_ERR_RESP_TBIT		((unsigned short)0x1 << 11)
#define SDIO_ERR_SIZE			((unsigned short)0x1 << 10)
#define SDIO_ERR_CMD_STARTBIT		((unsigned short)0x1 << 9)
#define SDIO_ERR_AUTOCMD12		((unsigned short)0x1 << 8)
#define SDIO_ERR_DATA_ENDBIT		((unsigned short)0x1 << 6)
#define SDIO_ERR_DATA_CRC		((unsigned short)0x1 << 5)
#define SDIO_ERR_DATA_TIMEOUT		((unsigned short)0x1 << 4)
#define SDIO_ERR_CMD_INDEX		((unsigned short)0x1 << 3)
#define SDIO_ERR_CMD_ENDBIT		((unsigned short)0x1 << 2)
#define SDIO_ERR_CMD_CRC		((unsigned short)0x1 << 1)
#define SDIO_ERR_CMD_TIMEOUT		((unsigned short)0x1 << 0)

#define SDIO_POLL_MASK 			(0xffff) /* enable all for polling */


#define SDIO_CMD_RSP_NONE		((unsigned short)0x0)
#define SDIO_CMD_RSP_136		((unsigned short)0x1)
#define SDIO_CMD_RSP_48			((unsigned short)0x2)
#define SDIO_CMD_RSP_48BUSY		((unsigned short)0x3)


#endif /* _MVSDMMC_INCLUDE */
