#ifndef _LINUX_MAIN_H
#define _LINUX_MAIN_H

#include "mv_os.h"

#include "hba_header.h"

extern PHBA_Extension g_Linux_HBA;

struct _MV_SCP {
	MV_U16           mapped;
	MV_U16           map_atomic;
	BUS_ADDRESS bus_address;
};

#define MV_SCp(cmd) ((struct _MV_SCP *) &((struct scsi_cmnd *)cmd)->SCp)

#define get_hba_ext_header(phost) (*(PModule_Header *)phost->hostdata)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define mv_scmd_host(cmd)    cmd->device->host
#define mv_scmd_channel(cmd) cmd->device->channel
#define mv_scmd_target(cmd)  cmd->device->id
#define mv_scmd_lun(cmd)     cmd->device->lun
#else
#define mv_scmd_host(cmd)    cmd->host
#define mv_scmd_channel(cmd) cmd->channel
#define mv_scmd_target(cmd)  cmd->target
#define mv_scmd_lun(cmd)     cmd->lun
#endif

#define SCSI_IS_INSTANT(cmd) \
             ((cmd == SCSI_CMD_INQUIRY) || \
	      (cmd == SCSI_CMD_READ_CAPACITY_10) || \
	      (cmd == SCSI_CMD_READ_CAPACITY_16) || \
	      (cmd == SCSI_CMD_REPORT_LUN) || \
	      (cmd == SCSI_CMD_MODE_SENSE_6) || \
	      (cmd == SCSI_CMD_MODE_SENSE_10))

#define LO_BUSADDR(x) ((MV_U32)(x))
#define HI_BUSADDR(x) (sizeof(BUS_ADDRESS)>4? (x)>>32 : 0)

#endif /*_LINUX_MAIN_H*/

