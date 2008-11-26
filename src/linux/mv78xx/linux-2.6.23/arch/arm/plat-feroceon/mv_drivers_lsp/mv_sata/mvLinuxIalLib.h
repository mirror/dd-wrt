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
/*******************************************************************************
* mvLinuxIalLib - Header File for Linux IAL Lib.
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*       None.
*
*
*******************************************************************************/
#ifndef __INCmvLinuxIalLibh
#define __INCmvLinuxIalLibh

#include "mvLinuxIalHt.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION (2,4,23)
#define irqreturn_t         void
#define IRQ_RETVAL(foo)
#endif
#define MV_LINUX_ASYNC_TIMER_PERIOD       ((MV_IAL_ASYNC_TIMER_PERIOD * HZ) / 1000)

struct pci_dev;
struct IALAdapter;
struct IALHost;
struct mv_comp_info;


/* Adapter Initialization */
int mv_ial_lib_allocate_edma_queues(struct IALAdapter *pAdapter);

void mv_ial_lib_free_edma_queues(struct IALAdapter *pAdapter);

int mv_ial_lib_init_channel(struct IALAdapter *pAdapter, MV_U8 channelNum);

void mv_ial_lib_free_channel(struct IALAdapter *pAdapter, MV_U8 channelNum);

/* PRD Table Generation */
#ifndef MV_PRD_TABLE_SIZE
 #define MV_PRD_TABLE_SIZE                  64 /* 64 entries max in PRD table */
#endif


int mv_ial_lib_prd_destroy(struct IALHost *pHost);
int mv_ial_lib_prd_init(struct IALHost *);



int mv_ial_lib_generate_prd(MV_SATA_ADAPTER *pMvSataAdapter, struct scsi_cmnd *SCpnt,
                            struct mv_comp_info *);


/* Interrupt Service Routine*/
irqreturn_t mv_ial_lib_int_handler (int irq, void *dev_id);


/* Event Notification */
MV_BOOLEAN mv_ial_lib_udma_command_completion_call_back(MV_SATA_ADAPTER *pMvSataAdapter,
                                           MV_U8 channelNum,
                                           MV_COMPLETION_TYPE comp_type,
                                           void *commandId,
                                           MV_U16 responseFlags,
                                           MV_U32 timeStamp,
                                           MV_STORAGE_DEVICE_REGISTERS *registerStruct);

MV_BOOLEAN mv_ial_lib_event_notify(MV_SATA_ADAPTER *pMvSataAdapter, MV_EVENT_TYPE eventType,
                             MV_U32 param1, MV_U32 param2);
void asyncStartTimerFunction(unsigned long data);

/* SCSI done queuing and callback */
void mv_ial_lib_add_done_queue (struct IALAdapter *pAdapter,
                                MV_U8 channel,
                                struct scsi_cmnd   *scsi_cmnd);

struct scsi_cmnd * mv_ial_lib_get_first_cmnd (struct IALAdapter *pAdapter,
                                       MV_U8 channel);

void mv_ial_lib_do_done (struct scsi_cmnd *cmnd);

void mv_ial_block_requests(struct IALAdapter *pAdapter, MV_U8 channelIndex);

#endif /* __INCmvLinuxIalLibh */
