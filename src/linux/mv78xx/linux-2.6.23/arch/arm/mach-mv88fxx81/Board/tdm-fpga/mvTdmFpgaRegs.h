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

#ifndef _MV_TDM_FPGA_REGS_H_
#define _MV_TDM_FPGA_REGS_H_

#include "mvTdmFpga.h"

#define FPGA_BASE		0xfa800000
#define FPGA_SIZE		0x100000	/* 1M */

/* Address base and offsets */
#define FPGA_DTX		(FPGA_BASE)
#define FPGA_DRX		(FPGA_BASE|0x800)
#define FPGA_STATUS		(FPGA_BASE|0x1000)
#define FPGA_CONTROL		(FPGA_BASE|0x1800)
#define FPGA_SPI_ACCESS		(FPGA_BASE|0x2000)
#define FPGA_SPI_READ_DATA	(FPGA_BASE|0x2800)

/* Bit definitions for some FPGA regs */
#define TXDBLOCK_INT_LATCH	1
#define RXDBLOCK_INT_LATCH	(1<<1)
#define SPI_READ_RDY_INT_LATCH	(1<<2)
#define SLIC_INT_LATCH		(1<<3)
#define RXD_SLIC_REGFLAG	(1<<4)
#define RXD_ARM_REGFLAG		(1<<5)
#define TXD_SLIC_REGFLAG	(1<<6)
#define TXD_ARM_REGFLAG		(1<<7)
#define TXDBLOCK_INT_MASK	1
#define RXDBLOCK_INT_MASK	(1<<1)
#define PROSLIC_RESET_CONTROL	(1<<2)
#define SLIC_INT_MASK		(1<<3)
#define SPI_READ_ACCESS		0x8000
#define SPI_WRITE_ACCESS	0
#define SPI_BUSY		0x100






#endif /*_MV_TDM_FPGA_REGS_H_*/

