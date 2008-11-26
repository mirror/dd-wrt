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


/* For future use */
#define BD_ID_DATA_START_OFFS	0x0
#define BD_DETECT_SEQ_OFFS		0x0
#define BD_SYS_NUM_OFFS			0x4
#define BD_NAME_OFFS			0x8



/* MPP possible values in the remarks should be updated from the board
   sheet or taken from HW team */


/* I2C bus addresses */
#define MV_BOARD_CTRL_I2C_ADDR			0x0     /* Controller slave addr */
#define MV_BOARD_CTRL_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM0_I2C_ADDR			0x56
#define MV_BOARD_DIMM0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM1_I2C_ADDR			0x54
#define MV_BOARD_DIMM1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_EEPROM_I2C_ADDR	    	0x51
#define MV_BOARD_EEPROM_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR	   	0x50
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR_TYPE 	ADDR7_BIT

/* Eeprom board data */
#define MV_BOARD_ID_EEPROM                      MV_BOARD_MAIN_EEPROM_I2C_ADDR
#define MV_BOARD_ID_EEPROM_OFFSET0              0x1F0		/* last 16byte in 0.5KByte EEPROMS */
#define MV_BOARD_ID_EEPROM_OFFSET1              0x1FF0		/* last 16byte in 8KByte EEPROMS */
#define MV_BOARD_I2C_MAGIC                      0xFEEDFEED


#define BOOT_FLASH_INDEX					0
#define MAIN_FLASH_INDEX					1

/* Boot Flash definitions */
#define	MV_BOARD_BOOT_FLASH_BASE_ADRS		mvBoardGetDeviceBaseAddr(BOOT_FLASH_INDEX,	\
																	 BOARD_DEV_NOR_FLASH)
#define MV_BOARD_BOOT_FLASH_BUS_WIDTH	    mvBoardGetDeviceBusWidth(BOOT_FLASH_INDEX,	\
																	  BOARD_DEV_NOR_FLASH)
#define MV_BOARD_BOOT_FLASH_DEVICE_WIDTH	mvBoardGetDeviceWidth(BOOT_FLASH_INDEX,	\
																   BOARD_DEV_NOR_FLASH)

/* Board main flash */
#define	MV_BOARD_FLASH_BASE_ADRS    	mvBoardGetDeviceBaseAddr(MAIN_FLASH_INDEX,	\
																	 BOARD_DEV_NOR_FLASH)
#define MV_BOARD_FLASH_BUS_WIDTH		mvBoardGetDeviceBusWidth(MAIN_FLASH_INDEX,	\
																	  BOARD_DEV_NOR_FLASH)
#define MV_BOARD_FLASH_DEVICE_WIDTH		mvBoardGetDeviceWidth(MAIN_FLASH_INDEX,	\
																   BOARD_DEV_NOR_FLASH)


/* Clocks stuff */
#define MV_BOARD_DEFAULT_TCLK	166666667	/* Default Tclk 166MHz 		*/
#define MV_BOARD_DEFAULT_SYSCLK	200000000	/* Default SysClk 200MHz 	*/
#define MV_BOARD_DEFAULT_PCLK	400000000	/* Default Pclock 400 MHZ*/
#define MV_DB_FPGA_CPU_CLK	25000000
#define MV_DB_FPGA_TCLK		25000000
#define MV_BOARD_REF_CLOCK	3686400		/* Refrence Clock 3.6864MHz 	*/


/* Supported clocks */
#define MV_BOARD_TCLK_100MHZ	100000000   
#define MV_BOARD_TCLK_125MHZ	125000000	
#define MV_BOARD_TCLK_133MHZ	133333333   
#define MV_BOARD_TCLK_150MHZ	150000000  
#define MV_BOARD_TCLK_166MHZ	166666667   
#define MV_BOARD_TCLK_200MHZ	200000000   

#define MV_BOARD_SYSCLK_100MHZ	100000000   
#define MV_BOARD_SYSCLK_125MHZ	125000000  
#define MV_BOARD_SYSCLK_133MHZ	133333333   
#define MV_BOARD_SYSCLK_150MHZ	150000000  
#define MV_BOARD_SYSCLK_166MHZ	166666667   
#define MV_BOARD_SYSCLK_200MHZ	200000000   
#define MV_BOARD_SYSCLK_233MHZ	233333333   
#define MV_BOARD_SYSCLK_250MHZ	250000000   
#define MV_BOARD_SYSCLK_266MHZ	266666667





#include "mv88F1281BoardEnv.h"
#include "mv88F5082BoardEnv.h"
#include "mv88F5182BoardEnv.h"
#include "mv88F5181LBoardEnv.h"
#include "mv88W8660BoardEnv.h"
#include "mv88F5181BoardEnv.h"
#include "mv88F5180NBoardEnv.h"
#include "mv88F6082BoardEnv.h"
#include "mv88F6183BoardEnv.h"
#include "mvCustomerBoardEnv.h"


#endif /* __INCmvBoardEnvSpech */
