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


#ifndef __INCmv802_3h
#define __INCmv802_3h


/* includes */
#include "mvTypes.h"

/* Defines */
#define MV_MAX_ETH_DATA     1500

/* 802.3 types */
#define MV_IP_TYPE                  0x0800
#define MV_IP_ARP_TYPE              0x0806
#define MV_APPLE_TALK_ARP_TYPE      0x80F3
#define MV_NOVELL_IPX_TYPE          0x8137
#define MV_EAPOL_TYPE				0x888e



/* Encapsulation header for RFC1042 and Ethernet_tunnel */

#define MV_RFC1042_SNAP_HEADER     {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00}

#define MV_ETH_SNAP_LSB             0xF8


#define	MV_MAC_ADDR_SIZE		(6)
#define MV_MAC_STR_SIZE     (20)

/* This macro checks for a multicast mac address    */
#define MV_IS_MULTICAST_MAC(mac)  (((mac)[0] & 0x1) == 1)


/* This macro checks for an broadcast mac address     */
#define MV_IS_BROADCAST_MAC(mac)            \
       (((mac)[0] == 0xFF) &&       \
        ((mac)[1] == 0xFF) &&       \
        ((mac)[2] == 0xFF) &&       \
        ((mac)[3] == 0xFF) &&       \
        ((mac)[4] == 0xFF) &&       \
        ((mac)[5] == 0xFF))


/* Typedefs */
typedef struct
{
    MV_U8     pDA[MV_MAC_ADDR_SIZE];
    MV_U8     pSA[MV_MAC_ADDR_SIZE];
    MV_U16    typeOrLen;

} MV_802_3_HEADER;


#endif /* __INCmv802_3h */
