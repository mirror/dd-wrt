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

#ifndef __INCmvFlashComH
#define __INCmvFlashComH

#include "mvCtrlEnvLib.h"


/* Vendor Ids */
#define AMD_MANUF	0x01	/* AMD	   manuf. ID in D23..D16, D7..D0 */
#define FUJ_MANUF	0x04	/* FUJITSU manuf. ID in D23..D16, D7..D0 */
#define ATM_MANUF	0x1F	/* ATMEL */
#define STM_MANUF	0x20	/* STM (Thomson) manuf. ID in D23.. -"- */
#define SST_MANUF	0xBF	/* SST	   manuf. ID in D23..D16, D7..D0 */
#define MT_MANUF	0x89	/* MT	   manuf. ID in D23..D16, D7..D0 */
#define INTEL_MANUF	0x89	/* INTEL   manuf. ID in D23..D16, D7..D0 */
#define INTEL_ALT_MANUF	0xB0	/* alternate INTEL namufacturer ID	*/
#define MX_MANUF	0xC2	/* MXIC	   manuf. ID in D23..D16, D7..D0 */
#define TOSH_MANUF	0x98	/* TOSHIBA manuf. ID in D23..D16, D7..D0 */


#define MAX_SECTOR_NUM  300

#define FLASH_BAD_SEC_NUM 	0xFFFFFFFF
#define FLASH_WR_ERASED 	0xFFFFFFFF
#define FLASH_MASK_16BIT 	0xFFFF
#define FLASH_MASK_8BIT 	0xFF
/* typedefs */

/* This structure describes a flash sector                  */
typedef struct _mvFlashSector
{
    MV_U32  baseOffs;   /* Sector base offset related to flash base. 	*/
    MV_U32  size;	/* sector size in bytes            		*/
    MV_BOOL protect;    /* Sector write protect indicator   		*/
}MV_FLASH_SECTOR;

/* This structures describes a flash Sectors structure type */
typedef enum flashSecType
{
    REGULAR,
    BOTTOM,
    TOP
}FLASH_SEC_TYPE;

typedef struct flashStruct
{
    MV_U32         flashVen;   /* AMD/Intel/...    */    
    MV_U32         flashId;	   /* Combined device & vendor ID  				*/
    MV_U32         size;       /* Total Flash size in bytes    				*/
    MV_U32         sectorNum;  /* Flash total sector number    				*/
    FLASH_SEC_TYPE secType;    /* Sector structure type        				*/
    MV_U32	       secFragNum; /* In case B/T flash then this is the Number */
							   /* of Sector defined in the Frag list. 		*/
    MV_U32	      *pSecSizeFragList; /* a list of the sectors' sizes in the */
							   /* B/T sector part. 							*/
    MV_BOOL	       HwProtect;  /* HW protection is supported				*/
    MV_U32         HwBuffLen;  /* support for hw buffering (0 - not,        */
                               /* > 0 - hw buff length).                    */
}FLASH_STRUCT;

/* This struct describes a Flash device     */
typedef struct _mvFlashInfo
{
    FLASH_STRUCT    flashSpec;
    MV_U32          baseAddr;               /* Flash base address           */
    MV_FLASH_SECTOR sector[MAX_SECTOR_NUM]; /* Flash sector list            */
    MV_U32    	    busWidth;               /* Width of the Flash bus       */
    MV_U32    	    devWidth;               /* Width of single Flash device */

} MV_FLASH_INFO;




/* get info */
MV_U32      mvFlashNumOfDevGet   (MV_FLASH_INFO *pFlash);
MV_BOOL	    mvFlashIsHwLock      (MV_FLASH_INFO *pFlash);
MV_BOOL     mvFlashSecLockGet    (MV_FLASH_INFO *pFlash, MV_U32 secNum);
MV_U32      mvFlashInWhichSec    (MV_FLASH_INFO *pFlash, MV_U32 offset);
MV_U32      mvFlashSecSizeGet    (MV_FLASH_INFO *pFlash, MV_U32 sectorNumber);
MV_U32      mvFlashNumOfSecsGet  (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashSizeGet       (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashSecOffsGet    (MV_FLASH_INFO *pFlash, MV_U32 sectorNum);
MV_U32      mvFlashVenIdGet      (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashDevIdGet      (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashBaseAddrGet   (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashBusWidthGet   (MV_FLASH_INFO *pFlash);
MV_U32      mvFlashDevWidthGet   (MV_FLASH_INFO *pFlash);
FLASH_SEC_TYPE	mvFlashSecTypeGet(MV_FLASH_INFO *pFlash);

/* flash Utils */
MV_U32 	flashDataExt(MV_FLASH_INFO *pFlash, MV_U32 data);
MV_U32 	flashAddrExt(MV_FLASH_INFO *pFlash, MV_U32 data, MV_U32 secNum);
MV_VOID flashCmdSet (MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 secNum, 
					                                              MV_U32 data);
MV_VOID	flashPrint  (MV_FLASH_INFO *pFlash);
MV_VOID flashBusWidthWr(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 data);
MV_VOID flashBusWidthDataWr(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 data);
MV_U32 	flashBusWidthRd(MV_FLASH_INFO *pFlash, MV_U32 addr);

#endif /* __INCmvFlashComH */
