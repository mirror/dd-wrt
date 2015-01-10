/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#define RECONFIG_PARTITION_SIZE 1

#define MTD_BOOT_PART_SIZE  0x80000
#define MTD_CONFIG_PART_SIZE    0x20000
#define MTD_FACTORY_PART_SIZE   0x20000

extern unsigned int  CFG_BLOCKSIZE;
#define LARGE_MTD_BOOT_PART_SIZE       (CFG_BLOCKSIZE<<2)
#define LARGE_MTD_CONFIG_PART_SIZE     (CFG_BLOCKSIZE<<2)
#define LARGE_MTD_FACTORY_PART_SIZE    (CFG_BLOCKSIZE<<1)

/*=======================================================================*/
/* NAND PARTITION Mapping                                                  */
/*=======================================================================*/
//#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition g_pasStatic_Partition[] = {
	{
                name:           "ALL",
                size:           MTDPART_SIZ_FULL,
                offset:         0,
        },
        /* Put your own partition definitions here */
        {
                name:           "Bootloader",
                size:           MTD_BOOT_PART_SIZE,
                offset:         0,
        }, {
                name:           "Config",
                size:           MTD_CONFIG_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "Factory",
                size:           MTD_FACTORY_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
        }, {
                name:           "Kernel_RootFS",
                size:           MTD_KERN_PART_SIZE + MTD_ROOTFS_PART_SIZE,
                offset:         MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE,
#endif
#else //CONFIG_RT2880_ROOTFS_IN_RAM
        }, {
                name:           "Kernel",
                size:           0x10000,
                offset:         MTDPART_OFS_APPEND,
#endif
#ifdef CONFIG_DUAL_IMAGE
        }, {
                name:           "Kernel2",
                size:           MTD_KERN2_PART_SIZE,
                offset:         MTD_KERN2_PART_OFFSET,
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "RootFS2",
                size:           MTD_ROOTFS2_PART_SIZE,
                offset:         MTD_ROOTFS2_PART_OFFSET,
#endif
#endif
        }

};

#define NUM_PARTITIONS ARRAY_SIZE(g_pasStatic_Partition)
extern int part_num;	// = NUM_PARTITIONS;
//#endif
#undef RECONFIG_PARTITION_SIZE

