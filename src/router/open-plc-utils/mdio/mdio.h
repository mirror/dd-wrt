/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   mdio.h - mdio related definitions and declarations;
 *
 *.  Atheros Powerline Toolkit for HomePlug AV;
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *--------------------------------------------------------------------*/

#ifndef MDIO_HEADER
#define MDIO_HEADER

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define MDIO_VERBOSE (1 << 0)
#define MDIO_SILENCE (1 << 1)

#define MDIO16_USE(x)           (((x) & 0x0001) << 0)
#define MDIO16_RSVD(x)          (((x) & 0x001F) << 1)
#define MDIO16_CNT(x)           (((x) & 0x03FF) << 6)
#define MDIO16_START(use,rsvd,cnt)      (MDIO16_USE (use) | MDIO16_RSVD (rsvd) | MDIO16_CNT (cnt))

#define MDIO16_SRT(x)           (((x) & 0x0003) << 0)
#define MDIO16_OP(x)            (((x) & 0x0003) << 2)
#define MDIO16_PHY(x)           (((x) & 0x001F) << 4)
#define MDIO16_REG(x)           (((x) & 0x001F) << 9)
#define MDIO16_TA(x)            (((x) & 0x0003) << 14)
#define MDIO16_INSTR(srt,op,phy,reg,ta) (MDIO16_SRT (srt) | MDIO16_OP (op) | MDIO16_PHY (phy) | MDIO16_REG (reg) | MDIO16_TA (ta))

#define MDIO32_HI_ADDR_SHIFT 9
#define MDIO32_LO_ADDR_SHIFT 1
#define MDIO32_HI_ADDR_MASK (0x000003FF << MDIO32_HI_ADDR_SHIFT)
#define MDIO32_LO_ADDR_MASK 0x000001FC

#define MDIO32_CODE_SHIFT 3
#define MDIO32_CODE_MASK (0x03 << CODE_SHIFT)
#define MDIO32_CODE_HI_ADDR 0x03
#define MDIO32_CODE_LO_ADDR 0x02

#define MDIO32_HI_ADDR(a) ((a & MDIO32_HI_ADDR_MASK) >> MDIO32_HI_ADDR_SHIFT)
#define MDIO32_LO_ADDR(a) ((a & MDIO32_LO_ADDR_MASK) >> MDIO32_LO_ADDR_SHIFT)

#define MDIO32_INSTR(addr, data, mask) MDIO16_INSTR (1, 1, (MDIO32_CODE_HI_ADDR << MDIO32_CODE_SHIFT), 0x00, 2), MDIO32_HI_ADDR (addr), 0xFFFF, MDIO16_INSTR (1, 1, (MDIO32_CODE_LO_ADDR << MDIO32_CODE_SHIFT) | ((MDIO32_LO_ADDR (addr) & 0xE0) >> 5), MDIO32_LO_ADDR (addr) & 0x1F, 2), (data & 0x0000FFFF), mask & 0x0000FFFF, MDIO16_INSTR (1, 1, (MDIO32_CODE_LO_ADDR << MDIO32_CODE_SHIFT) | ((MDIO32_LO_ADDR (addr) & 0xE0) >> 5), (MDIO32_LO_ADDR (addr) & 0x1F) | 0x01, 2), (data & 0xFFFF0000) >> 16, (mask & 0xFFFF0000) >> 16

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

