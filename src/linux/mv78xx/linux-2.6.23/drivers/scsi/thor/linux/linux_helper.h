/*
 *  Yet another header added to solve the dependence hell
 *
 *
 *  July 6th, 2006 A.C. <ake at marvell dot com>
 *
 */
#ifndef __LINUX_HELPER_H__
#define __LINUX_HELPER_H__

#include "mv_os.h"
#include "hba_header.h"

MV_BOOLEAN TranslateOSRequest( IN PHBA_Extension pHBA,
			       IN struct scsi_cmnd * pSCmd,
			       OUT PMV_Request pReq );

void HBARequestCallback( MV_PVOID This,
			 PMV_Request pReq );



void GenerateSGTable( IN PHBA_Extension pHBA,
		      IN struct scsi_cmnd *SCpnt,
		      OUT PMV_SG_Table pSGTable );


MV_BOOLEAN TranslateSCSIRequest(PHBA_Extension pHBA, 
				struct scsi_cmnd *pSCmd, 
				PMV_Request pReq );

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
int __hba_wait_for_atomic_timeout(atomic_t *atomic, unsigned long timeout);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */

void __hba_dump_req_info(PMV_Request preq);

void hba_send_shutdown_req(PHBA_Extension phba);
void HBA_HandleWaitingList(PHBA_Extension pHBA);
#endif /*__LINUX_HELPER_H__*/

