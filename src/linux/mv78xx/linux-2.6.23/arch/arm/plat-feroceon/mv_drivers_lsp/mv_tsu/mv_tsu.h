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
*******************************************************************************/

#ifndef __MV_TSU_H__
#define __MV_TSU_H__


#define MV_TSU_MAJOR		MV_TS_MAJOR

#define TSU_DFLT_AGGR_MODE 	MV_TSU_AGGR_MODE_DISABLED
#define TSU_DFLT_TMSTMP_OFFS	4
#define TSU_DFLT_AGGR_PCKT_NUM	1
#define TSU_DFLT_TS_DESC_NUM	64
#define TSU_DFLT_TS_DONEQ_NUM	TSU_DFLT_TS_DESC_NUM * TSU_DFLT_AGGR_PCKT_NUM
#define TSU_DFLT_DATA_READ_AT_ONCE	0
#define TSU_DFLT_CLOCK_RATE	20000000


#define TSU_DFLT_INT_MASK	(TSU_INT_TS_IF_ERROR |		\
                                 TSU_INT_FIFO_OVFL_ERROR |	\
                                 TSU_INT_TS_CONN_ERROR |	\
                                 TSU_INT_CLOCK_SYNC_EXP | 0x7F)

#endif /* __MV_TSU_H__ */
