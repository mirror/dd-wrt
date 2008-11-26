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

#ifndef __MV_TSU_IOCTL_H__
#define __MV_TSU_IOCTL_H__

#define TSU_STATUS_ERROR_GET(status)		((status >> 28) & 0x1)
#define TSU_STATUS_ERROR_SET(status)		(status = status | (0x1 << 28))
#define TSU_STATUS_TMSSTMP_GET(status)		(status & 0xFFFFFFF)
#define TSU_STATUS_TMSSTMP_SET(status,tms)	(status = status | (tms & 0xFFFFFFF))

struct tsu_tmstmp_info {
	unsigned int timestamp;
	unsigned char enable_tms;
};


enum tsu_aggr_mode {
	aggrModeDisabled,
	aggrMode1,
	aggrMode2
};

struct tsu_buff_info {
	enum tsu_aggr_mode aggr_mode;
	unsigned char aggr_mode2_tmstmp_off;
	unsigned char aggr_num_packets;

	unsigned int num_ts_desc;
	unsigned int num_done_q_entries;
	int pkt_size;
};


struct tsu_stat {
	unsigned int ts_if_err;
	unsigned int fifo_ovfl;
	unsigned int ts_conn_err;
	unsigned int clk_sync_exp;
};


#define MVTSU_IOC_MAGIC  'T'

#define MVTSU_IOCFREQSET	_IOW(MVTSU_IOC_MAGIC,1, unsigned int)
#define MVTSU_IOCTXTMSSET	_IOW(MVTSU_IOC_MAGIC,2, struct tsu_tmstmp_info)
#define MVTSU_IOCTXDONE		_IO(MVTSU_IOC_MAGIC,3)
#define MVTSU_IOCRDPKTATONCE	_IOW(MVTSU_IOC_MAGIC,4, unsigned int)
#define MVTSU_IOCBUFFPARAMGET	_IOR(MVTSU_IOC_MAGIC,5, struct tsu_buff_info)
#define MVTSU_IOCGETSTAT	_IOR(MVTSU_IOC_MAGIC,6, struct tsu_stat)
#define MVTSU_IOCCLEARSTAT	_IO(MVTSU_IOC_MAGIC,7)

#endif /* __MV_TSU_IOCTL_H__ */
