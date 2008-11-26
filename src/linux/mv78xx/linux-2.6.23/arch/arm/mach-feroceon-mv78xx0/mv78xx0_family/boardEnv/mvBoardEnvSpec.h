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

#ifndef __INCmvBoardEnvSpech
#define __INCmvBoardEnvSpech

#include "mvSysHwConfig.h"

#define DB_MODEL_BACKPLAIN	0x0100

/* Clocks stuff */
#define MV_BOARD_DEFAULT_TCLK	166000000	/* Default Tclk 166MHz 		*/
#define MV_BOARD_DEFAULT_SYSCLK	200000000	/* Default SysClk 200MHz 	*/

/*  MPP[0] = GE_TXCLK		    MPP[1] = GE_TXCTL		    */
/*  MPP[2] = GE_RXCTL		    MPP[3] = GE_RXCLK		    */
/*  MPP[4] = GE_TXD0                MPP[5] = GE_TXD1		    */
/*  MPP[6] = GE_TXD2		    MPP[7] = GE_TXD3		    */
#define RD_MASA_MPP0_7   0x22222222             	
   
/*  MPP[8] = GE_RXD0		    MPP[9]  = GE_RXD1		    */
/*  MPP[10] = GE_RXD2		    MPP[11] = GE_RXD3		    */
/*  MPP[12] = DC	   	    MPP[13] = SysRst		    */
/*  MPP[14] = SATA_ACT		    MPP[15] = SATA_ACT		    */
#define RD_MASA_MPP8_15  0x33302222             	
   
/*  MPP[16] = DC	   	    MPP[17] = DC		    */
/*  MPP[18] = DC		    MPP[19] = DC		    */
/*  MPP[20] = DC	   	    MPP[21] = DC		    */
/*  MPP[22] = DC		    MPP[23] = DC		    */
#define RD_MASA_MPP16_23 0x00000000

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define RD_MASA_MPP24_31 0x00000000

/*  MPP[0] = GE_TXCLK		    MPP[1] = GE_TXCTL		    */
/*  MPP[2] = GE_RXCTL		    MPP[3] = GE_RXCLK		    */
/*  MPP[4] = GE_TXD0                MPP[5] = GE_TXD1		    */
/*  MPP[6] = GE_TXD2		    MPP[7] = GE_TXD3		    */
#define RD_AMC_MPP0_7   0x22222222             	
   
/*  MPP[8] = GE_RXD0		    MPP[9]  = GE_RXD1		    */
/*  MPP[10] = GE_RXD2		    MPP[11] = GE_RXD3		    */
/*  MPP[12] = DC	   	    MPP[13] = DC		    */
/*  MPP[14] = DC		    MPP[15] = DC		    */
#define RD_AMC_MPP8_15  0x00002222             	
   
/*  MPP[16] = DC	   	    MPP[17] = DC		    */
/*  MPP[18] = DC		    MPP[19] = DC		    */
/*  MPP[20] = DC	   	    MPP[21] = DC		    */
/*  MPP[22] = DC		    MPP[23] = DC		    */
#define RD_AMC_MPP16_23 0x00000000

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define RD_AMC_MPP24_31 0x00000000

/*  MPP[0] = GE_TXCLK		    MPP[1] = GE_TXCTL		    */
/*  MPP[2] = GE_RXCTL		    MPP[3] = GE_RXCLK		    */
/*  MPP[4] = GE_TXD0                MPP[5] = GE_TXD1		    */
/*  MPP[6] = GE_TXD2		    MPP[7] = GE_TXD3		    */
#define RD_H3C_MPP0_7   0x22222222             	
   
/*  MPP[8] = GE_RXD0		    MPP[9]  = GE_RXD1		    */
/*  MPP[10] = GE_RXD2		    MPP[11] = GE_RXD3		    */
/*  MPP[12] = DC	   	    MPP[13] = DC		    */
/*  MPP[14] = DC		    MPP[15] = DC		    */
#define RD_H3C_MPP8_15  0x00302222             	
   
/*  MPP[16] = DC	   	    MPP[17] = DC		    */
/*  MPP[18] = DC		    MPP[19] = DC		    */
/*  MPP[20] = DC	   	    MPP[21] = DC		    */
/*  MPP[22] = DC		    MPP[23] = DC		    */
#define RD_H3C_MPP16_23 0x00000000

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define RD_H3C_MPP24_31 0x00000000

/*  MPP[0] = GE_TXCLK		    MPP[1] = GE_TXCTL		    */
/*  MPP[2] = GE_RXCTL		    MPP[3] = GE_RXCLK		    */
/*  MPP[4] = GE_TXD0                MPP[5] = GE_TXD1		    */
/*  MPP[6] = GE_TXD2		    MPP[7] = GE_TXD3		    */
#define DB_78XX0_MPP0_7   0x22222222             	
   
/*  MPP[8] = GE_RXD0		    MPP[9]  = GE_RXD1		    */
/*  MPP[10] = GE_RXD2		    MPP[11] = GE_RXD3		    */
/*  MPP[12] = GPIO[12] - USB1_VBUS  MPP[13] = MV_SYS_RST	    */
/*  MPP[14] = SATA1_ACT		    MPP[15] = SATA2_ACT		    */
#define DB_78XX0_MPP8_15  0x33302222             	
   
/*  MPP[16] = UART2_TXD		    MPP[17] = UART2_RXD 	    */
/*  MPP[18] = UART0_CTS             MPP[19] = UART0_RTS		    */
/*  MPP[20] = UART1_CTS             MPP[21] = UART1_RTS             */
/*  MPP[22] = UART3_TXD             MPP[23] = UART3_RXD             */
#define DB_78XX0_MPP16_23 0x44444444

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define DB_78XX0_MPP24_31 0x00000000

/*  MPP[0] = GE_TXCLK		    MPP[1] = GE_TXCTL		    */
/*  MPP[2] = GE_RXCTL		    MPP[3] = GE_RXCLK		    */
/*  MPP[4] = GE_TXD0                MPP[5] = GE_TXD1		    */
/*  MPP[6] = GE_TXD2		    MPP[7] = GE_TXD3		    */
#define DB_78200_MPP0_7   0x22222222             	
   
/*  MPP[8] = GE_RXD0		    MPP[9]  = GE_RXD1		    */
/*  MPP[10] = GE_RXD2		    MPP[11] = GE_RXD3		    */
/*  MPP[12] = GPIO[12] - USB1_VBUS  MPP[13] = MV_SYS_RST	    */
/*  MPP[14] = SATA1_ACT		    MPP[15] = SATA2_ACT		    */
#define DB_78200_MPP8_15  0x22222222             	
   
/*  MPP[16] = UART2_TXD		    MPP[17] = UART2_RXD 	    */
/*  MPP[18] = UART0_CTS             MPP[19] = UART0_RTS		    */
/*  MPP[20] = UART1_CTS             MPP[21] = UART1_RTS             */
/*  MPP[22] = UART3_TXD             MPP[23] = UART3_RXD             */
#define DB_78200_MPP16_23 0x22222222

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define DB_78200_MPP24_31 0x22222222

/*  MPP[24] = DC	   	    MPP[25] = DC		    */
/*  MPP[26] = DC		    MPP[26] = DC		    */
/*  MPP[28] = DC	   	    MPP[27] = DC		    */
/*  MPP[30] = DC		    MPP[28] = DC		    */
#define DB_78200_MPP32_39 0x11112222

#define DB_78XX0_HAS_PTP_BRIDGE	1

/* 7-Segment stuff */
#define MV_BOARD_7SEG_BASE		    	((RD_78XX0_H3C_ID == mvBoardIdGet())? \
							DEVICE_CS2_BASE : DEVICE_CS1_BASE)
#define MV_BOARD_7SEG_PARAM		    	0x003E07CF  /*  8 bit */ 

/* Board main flash */
#define	MV_BOARD_FLASH_BASE_ADRS		DEVICE_CS0_BASE 
#define MV_BOARD_FLASH_BUS_WIDTH		32 	    	/* Two byte width bus 	  */
#define MV_BOARD_FLASH_DEVICE_WIDTH		16 	    	/* 16-bit width Flash mode*/
#define MV_BOARD_FLASH_PARAM			(mvBoardFlashParamGet())
#define MV_BOARD_FLASH_PARAM_VAL		0x803E07CF  /* 32 bit */

/* NAND flash */
#define MV_NAND_CS			        DEV_CS2
#define MV_NAND_78200_CS			DEV_CS3
#define MV_NAND_CARE		                0
#define MV_BOARD_NFLASH_DEVICE_WIDTH		8 	    	/* 8-bit width Flash mode*/
#define	MV_BOARD_NFLASH_BASE_ADRS		DEVICE_CS2_BASE /* empty */
#define MV_BOARD_NFLASH_PARAM			0x003E07CF      /* 8 bit */

/* Boot Flash definitions */
#define	MV_BOARD_BOOT_FLASH_BASE_ADRS		BOOTDEV_CS_BASE
#define MV_BOARD_BOOT_FLASH_BUS_WIDTH		8  	    	/* One byte width bus 	  */
#define MV_BOARD_BOOT_FLASH_DEVICE_WIDTH	8  	    	/* 8-bit width Flash mode */
#define MV_BOARD_BOOT_FLASH_PARAM		0x003E07CF  	/* 8 bit */ 
#define MV_BOARD_78200_BOOT_FLASH_PARAM		0x403E07CF  	/* 16 bit */ 


#define DEVICE_CS0_PARAM			MV_BOARD_FLASH_PARAM
#define DEVICE_78200_CS0_PARAM			0
#define DEVICE_CS1_PARAM			MV_BOARD_7SEG_PARAM
#define DEVICE_CS2_PARAM			MV_BOARD_NFLASH_PARAM
#define DEVICE_78200_CS2_PARAM			0

#define DEVICE_CS3_PARAM			0
#define DEVICE_78200_CS3_PARAM			MV_BOARD_NFLASH_PARAM

#define DEVICE_CS0_PARAM_WR			0xF0F0F
#define DEVICE_CS1_PARAM_WR			0xF0F0F
#define DEVICE_CS2_PARAM_WR			0xF0F0F
#define DEVICE_CS3_PARAM_WR			0xF0F0F

#define BOOTDEV_CS_PARAM			MV_BOARD_BOOT_FLASH_PARAM
#define BOOTDEV_78200_CS_PARAM			MV_BOARD_78200_BOOT_FLASH_PARAM

#define DRAM_SLOT_NUM           		2

/* Debug Led stuff. GPP pin assigments. Board depended. */
#define MV_BOARD_LED_NUM			0

/* Other GPIOs */
/* DB-78100-A-BP */
#define DB_78XX0_LED_GPP_PIN(ledNum)    	(0)
#define DB_78XX0_SW_RESET			13
#define DB_78XX0_USB_VBUS_PIN(usb)		12

/* DB-78200-A-BP */
#define DB_78200_SW_RESET			7
#define DB_78200_GPIO_OUTPUT_EN			(BIT7)    		

/* RD-AMC */
#define RD_AMC_LED_GPP_PIN(ledNum)    		(0)
#define RD_AMC_SW_RESET				25
#define RD_AMC_USB_VBUS_PIN(usb)		((usb == 0) ? 22 : ((usb == 1) ? 12 : 23))
/* GPP 12, 13, 16-19 are output */
#define RD_AMC_GPIO_OUTPUT_EN			(BIT12 | BIT13 | BIT16 | BIT17 | BIT18 | BIT19 | BIT25)    		

#define RD_H3C_SW_RESET				13
#define RD_H3C_SW_RESET_SELECT			14
#define RD_H3C_USB_VBUS_PIN(usb)		12
/* GPP 14 is output */
#define RD_H3C_GPIO_OUTPUT_EN			(BIT14)    		

#define RD_MASA_LED_GPP_PIN(ledNum)    		23
#define RD_MASA_DEBUG_LED_GPP_PIN(ledNum)    	22
#define RD_MASA_SW_RESET			13
/* GPP 12, 13, 16-19 are output */
#define RD_MASA_GPIO_OUTPUT_EN			(BIT17 | BIT18 | BIT22 | BIT23)    		
#define RD_MASA_USB_OC				BIT16
#define RD_MASA_USB_PWR				BIT17
#define RD_MASA_HDD_PWR				BIT18
#define RD_MASA_SW_INT				BIT19
#define RD_MASA_RTC_INT				BIT20
#define RD_MASA_PHY_INT				BIT21
#define RD_78XX0_MASA_INT_MASK			(RD_MASA_USB_OC | RD_MASA_SW_INT | \
						 RD_MASA_RTC_INT | RD_MASA_PHY_INT)

#ifdef  MV_INCLUDE_MODEM_ON_TTYS1 
#define DB_78XX0_SERIAL_DTR			2
#define DB_78XX0_SERIAL_DSR			3
#define DB_78XX0_SERIAL_DCD			1
#define DB_78XX0_SERIAL_RI			19
#endif 

/* GPIO interrupt connectivity on the DB-MV78XX0 board */

/* I2C bus addresses */
#define MV_BOARD_CTRL_I2C_ADDR			0x0     /* Controller slave addr */
#define MV_BOARD_CTRL_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM_I2C_CHANNEL		0x0
#ifdef RD_MV78XX0_MASA
/* SODIMM only two bit TWSI address */
#define MV_BOARD_DIMM0_I2C_ADDR			0x50
#define MV_BOARD_DIMM1_I2C_ADDR			0x51
#else
#define MV_BOARD_DIMM0_I2C_ADDR			0x56
#define MV_BOARD_DIMM1_I2C_ADDR			0x54
#endif
#define MV_BOARD_DIMM0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_CPU0_I2C_ADDR	    		0x51
#define MV_BOARD_CPU0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_CPU1_I2C_ADDR	    		0x51        /* Both CPU use the same I2C EEPROM */
#define MV_BOARD_CPU1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_SYS_I2C_ADDR	    		0x50
#define MV_BOARD_SYS_I2C_ADDR_TYPE 		ADDR7_BIT
#if defined (MV78XX0_Z0)
#define MV_BOARD_RTC_I2C_CHANNEL		0x0
#else
#define MV_BOARD_RTC_I2C_CHANNEL		0x1
#endif
#define MV_BOARD_RTC_I2C_ADDR	    		0x68
#define MV_BOARD_RTC_I2C_ADDR_TYPE 		ADDR7_BIT

/* Ethernet stuff */
#define MV78XX0_ETH_MAX_PORTS	     2
#define MV78200_ETH_MAX_PORTS	     4

#define BOARD_ETH_START_PORT_NUM	0
#define BOARD_ETH_END_PORT_NUM		3
#define BOARD_ETH_PORT_NUM  		4
#define MV_ETH_PORT_SGMII		{0, 0, 0, 0}; 
#define MV_ETH_PORT_PHY_ADDR		{0x8, 0x9, 0xA, 0xB};

#define MV_BOARD_MAX_USB_IF		(mvBoardGetMaxUsbIf())

#define MV_BOARD_MAX_TWSI_DEV		8
#define MV_BOARD_MAX_PCI_DEV            5

/* Supported clocks */
#define MV_BOARD_TCLK_100MHZ	100000000   
#define MV_BOARD_TCLK_125MHZ	125000000	
#define MV_BOARD_TCLK_133MHZ	133333334   
#define MV_BOARD_TCLK_150MHZ	150000000   
#define MV_BOARD_TCLK_166MHZ	166666667   
#define MV_BOARD_TCLK_200MHZ	200000000   

#define MV_BOARD_SYSCLK_100MHZ	100000000   
#define MV_BOARD_SYSCLK_125MHZ	125000000   
#define MV_BOARD_SYSCLK_133MHZ	133333334   
#define MV_BOARD_SYSCLK_150MHZ	150000000   
#define MV_BOARD_SYSCLK_166MHZ	166666667   
#define MV_BOARD_SYSCLK_200MHZ	200000000   
#define MV_BOARD_SYSCLK_160MHZ	160000000
#define MV_BOARD_SYSCLK_233MHZ	233333334 
#define MV_BOARD_SYSCLK_250MHZ	250000000
#define MV_BOARD_SYSCLK_267MHZ	266666667 
#define MV_BOARD_SYSCLK_300MHZ	300000000
#define MV_BOARD_SYSCLK_333MHZ	333333334 
#define MV_BOARD_SYSCLK_400MHZ	400000000   


#define SGMII_OUTBAND_AN
#define PHY_UPDATE_TIMEOUT	10000
#endif /* __INCmvBoardEnvSpech */
