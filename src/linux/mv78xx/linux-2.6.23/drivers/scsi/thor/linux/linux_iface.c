/*
 * Linux Driver for 61xx
 * linux_procinfo.c
 * lily initialized on Feb. 15 2006
 *
 * restructured & expanded to be linux_iface.c - Kernel/CLI interface
 * August 2006
 * Albert Ke < ake at marvell dot com >
 *
 */

#include "mv_os.h"
#include "mv_include.h"
#include "linux_iface.h"
#include "linux_main.h"
#include "linux_helper.h"


extern PHBA_Extension mv_device_extension_list[MV_DEVICE_MAX_SLOT];
extern unsigned int mv_device_count;
extern void HBA_HandleWaitingList(PHBA_Extension pHBA);

static int mv_open(struct inode *inode, struct file *file);
#ifdef HAVE_UNLOCKED_IOCTL
static long mv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int mv_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		    unsigned long arg);
#endif /* HAVE_UNLOCKED_IOCTL */

#define IOCTL_BUF_LEN 8192
static unsigned char ioctl_buf[IOCTL_BUF_LEN];

struct file_operations mv_fops = {
	.owner   =    THIS_MODULE,
	.open    =    mv_open,
#ifdef HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = mv_ioctl,
#else
	.ioctl   =    mv_ioctl,
#endif
	.release =    NULL
};

void IOHBARequestCallback(MV_PVOID This, PMV_Request pReq)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;

	List_Add(&pReq->Queue_Pointer, &pHBA->Free_Request);
	pHBA->Io_Count--;
}

void ioctlcallback(MV_PVOID This, PMV_Request pReq)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;

	List_Add(&pReq->Queue_Pointer, &pHBA->Free_Request);
	pHBA->Io_Count--;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&pHBA->hba_sync, 0);
#else
	complete(&pHBA->cmpl);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
}

#ifdef RAID_DRIVER
static MV_U16 API2Driver_ID(MV_U16 API_ID)
{
	MV_U16	returnID = API_ID;
	returnID &= 0xfff;
	return returnID;
}

static LD_Info ldinfo[LDINFO_NUM] = {{0}};
static int mv_proc_ld_info(struct Scsi_Host *host)
{
	PModule_Header pheader = get_hba_ext_header(host);
	PHBA_Extension pHBA = (PHBA_Extension)head_to_hba(pheader);
	PMV_Request pReq;
	MV_U8 Cdb[MAX_CDB_SIZE]; 
	MV_U16 LD_ID = 0XFF;
	unsigned long flags;

	Cdb[0] = APICDB0_LD;
	Cdb[1] = APICDB1_LD_GETINFO;
	Cdb[2] = LD_ID & 0xff;
	Cdb[3] = API2Driver_ID(LD_ID)>>8;
	
	spin_lock_irqsave(&pHBA->lock, flags);
	pReq = List_GetFirstEntry((&pHBA->Free_Request), MV_Request, Queue_Pointer);
	if (pReq == NULL) {
		spin_unlock_irqrestore(&pHBA->lock, flags);
		return -1;/*FAIL.*/
	}

	pReq->Cmd_Initiator = pHBA;
	pReq->Org_Req = pReq;/*No ideas.*/
	pReq->Device_Id = CONSOLE_ID;
	pReq->Cmd_Flag = 0;

	if (SCSI_IS_READ(Cdb[0]))
		pReq->Cmd_Flag |= CMD_FLAG_DATA_IN;
	if ( SCSI_IS_READ(Cdb[0]) || SCSI_IS_WRITE(Cdb[0]) )
		pReq->Cmd_Flag |= CMD_FLAG_DMA;
	
	pReq->Data_Transfer_Length = LDINFO_NUM*sizeof(LD_Info);
	memset(ldinfo, 0, LDINFO_NUM*sizeof(LD_Info));
	pReq->Data_Buffer = ldinfo;
	SGTable_Init(&pReq->SG_Table, 0);
	memcpy(pReq->Cdb, Cdb, MAX_CDB_SIZE);
	pReq->Context = NULL;
	pReq->LBA.value = 0;    
	pReq->Sector_Count = 0; 
	pReq->Completion = ioctlcallback;
	List_Add(&pReq->Queue_Pointer, &pHBA->Waiting_Request);
	pHBA->Io_Count++;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&pHBA->hba_sync, 1);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
	HBA_HandleWaitingList(pHBA);
	spin_unlock_irqrestore(&pHBA->lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	if ( !__hba_wait_for_atomic_timeout(&pHBA->hba_sync, 
					    HBA_REQ_TIMER_IOCTL*HZ) ) {
#else
	if (wait_for_completion_timeout(&pHBA->cmpl, 
					HBA_REQ_TIMER_IOCTL*HZ) == 0) {
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
		MV_DBG(DMSG_HBA, "__MV__ ioctl req timed out.\n");
	        return -1; /*FAIL.*/
	}

	return 0;/*SUCCESS.*/
}


static char* mv_ld_status(int status)
{
	switch (status) {
	case LD_STATUS_FUNCTIONAL:
		return "online";
	case LD_STATUS_DEGRADE:
		return "degraded";		
	case LD_STATUS_DELETED:
		return "deleted";
	case LD_STATUS_PARTIALLYOPTIMAL:
		return "partially optimal";
	case LD_STATUS_OFFLINE:
		return "offline";
	case LD_STATUS_MIGRATION:
		return "migration";
	}
	return "Unknown Status";
}

static char* mv_ld_raid_mode(int status)
{
	switch (status) {
	case LD_MODE_RAID0:
		return "RAID0";
	case LD_MODE_RAID1:
		return "RAID1";
	case LD_MODE_RAID10:
		return "RAID10";
	case LD_MODE_RAID1E:
		return "RAID1E";
	case LD_MODE_RAID5:
		return "RAID5";
	case LD_MODE_RAID50:
		return "RAID50";
	case LD_MODE_RAID6:
		return "RAID6";
	case LD_MODE_JBOD:
		return "JBOD";
	}
	return "Unknown RAID Mode";
}

static char* mv_ld_bga_status(int status)
{
	switch (status) {
	case LD_BGA_STATE_RUNNING:
		return "running";
	case LD_BGA_STATE_ABORTED:
		return "aborted";
	case LD_BGA_STATE_PAUSED:
		return "paused";
	case LD_BGA_STATE_AUTOPAUSED:
		return "auto paused";
	case LD_BGA_STATE_DDF_PENDING:
		return "DDF pending";
	}
	return "N/A";
}

static int mv_ld_get_status(struct Scsi_Host *host, MV_U16 ldid, LD_Status *ldstatus)
{
	PModule_Header pheader = get_hba_ext_header(host);
	PHBA_Extension pHBA = (PHBA_Extension)head_to_hba(pheader);
	PMV_Request pReq;
	MV_U8 Cdb[MAX_CDB_SIZE]; 
	MV_U16 LD_ID = ldid;/*0XFF;*/
	unsigned long flags;

	Cdb[0] = APICDB0_LD;
	Cdb[1] = APICDB1_LD_GETSTATUS;
	Cdb[2] = LD_ID & 0xff;
	Cdb[3] = API2Driver_ID(LD_ID)>>8;
	
	spin_lock_irqsave(&pHBA->lock, flags);
	pReq = List_GetFirstEntry((&pHBA->Free_Request), MV_Request, Queue_Pointer);
	if (pReq == NULL) {
		spin_unlock_irqrestore(&pHBA->lock, flags);
		return -1;/*FAIL.*/
	}

	pReq->Cmd_Initiator = pHBA;
	pReq->Org_Req = pReq;/*No ideas.*/
	pReq->Device_Id = CONSOLE_ID;
	pReq->Cmd_Flag = 0;

	if (SCSI_IS_READ(Cdb[0]))
		pReq->Cmd_Flag |= CMD_FLAG_DATA_IN;
	if ( SCSI_IS_READ(Cdb[0]) || SCSI_IS_WRITE(Cdb[0]) )
		pReq->Cmd_Flag |= CMD_FLAG_DMA;

	/* Data Buffer */
	pReq->Data_Transfer_Length = sizeof(LD_Status);
	memset(ldstatus,0,sizeof(LD_Status));
	pReq->Data_Buffer = ldstatus;
	
	SGTable_Init(&pReq->SG_Table, 0);
	memcpy(pReq->Cdb, Cdb, MAX_CDB_SIZE);
	pReq->Context = NULL;
	pReq->LBA.value = 0;    
	pReq->Sector_Count = 0; 
	pReq->Completion = ioctlcallback;
	List_Add(&pReq->Queue_Pointer, &pHBA->Waiting_Request);
	pHBA->Io_Count++;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&pHBA->hba_sync, 1);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
	HBA_HandleWaitingList(pHBA);
	spin_unlock_irqrestore(&pHBA->lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	if ( !__hba_wait_for_atomic_timeout(&pHBA->hba_sync, 
					    HBA_REQ_TIMER_IOCTL*HZ) ) {
#else
	if ( !wait_for_completion_timeout(&pHBA->cmpl, 
					  HBA_REQ_TIMER_IOCTL*HZ) ) {
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
		MV_DBG(DMSG_HBA, "__MV__ ioctl req timed out.\n");
	        return -1; /*FAIL.*/
	}

	return 0;/*SUCCESS.*/
}

static int mv_ld_show_status(char *buf, PLD_Status pld_status)
{
	char *str, *str1;
	int ret = 0;

	if ( LD_BGA_STATE_RUNNING == pld_status->BgaState)
	{
		if (LD_BGA_REBUILD == pld_status->Bga)
			str = "rebuilding";
		else if (LD_BGA_INIT_QUICK == pld_status->Bga || LD_BGA_INIT_BACK == pld_status->Bga)
			str = "initializing";
		else if (LD_BGA_CONSISTENCY_CHECK == pld_status->Bga || LD_BGA_CONSISTENCY_FIX == pld_status->Bga)
			str = "synchronizing";
		else if (LD_BGA_MIGRATION == pld_status->Bga)
			str = "extending";
		else
			str = "unknown bga action";
		ret = sprintf(buf, "  %s is %d%% done", str, pld_status->BgaPercentage);
	}
	else if ( (LD_BGA_STATE_ABORTED == pld_status->BgaState) || (LD_BGA_STATE_PAUSED == pld_status->BgaState) || (LD_BGA_STATE_AUTOPAUSED == pld_status->BgaState))
	{
		if (LD_BGA_REBUILD == pld_status->Bga)
			str = "rebuilding";
		else if (LD_BGA_INIT_QUICK == pld_status->Bga || LD_BGA_INIT_BACK == pld_status->Bga)
			str = "initializing";
		else if (LD_BGA_CONSISTENCY_CHECK == pld_status->Bga || LD_BGA_CONSISTENCY_FIX == pld_status->Bga)
			str = "synchronizing";
		else if (LD_BGA_MIGRATION == pld_status->Bga)
			str = "extending";
		else
			str = "unknown bga action";

		if (LD_BGA_STATE_ABORTED == pld_status->BgaState)
			str1 = "aborted";
		else if (LD_BGA_STATE_PAUSED == pld_status->BgaState)
			str1 = "paused";
		else if (LD_BGA_STATE_AUTOPAUSED == pld_status->BgaState)
			str1 = "auto paused";
		else
			str1 = "aborted";
		ret = sprintf(buf, "  %s is %s", str, str1);
	}
	return ret;
}
#endif /*RAID_DRIVER*/

int mv_linux_proc_info(struct Scsi_Host *pSHost, char *pBuffer, 
		       char **ppStart,off_t offset, int length, int inout)
{
	int len = 0;
	int datalen = 0;/*use as a temp flag.*/
#ifdef RAID_DRIVER
	int i = 0;
	int j = 0;
	int ret = -1;
	LD_Status ld_status;
	char *tmp = NULL;
	int tmplen = 0;
#endif	
	if (!pSHost || !pBuffer)
	        return (-ENOSYS);
	if (inout == 1) {
	/* User write is not supported. */
		return (-ENOSYS);
	}

	len = sprintf(pBuffer,"Marvell Thor Driver , Version %s\n", mv_version_linux);
	
#ifdef RAID_DRIVER
	if ( mv_proc_ld_info(pSHost) == -1 ) {
		len = sprintf(pBuffer,"Marvell Thor Driver is busy NOW, please try later.\n");
		goto out;
	} else {
		for (i = 0; i < MAX_LD_SUPPORTED; i++) {
			if (ldinfo[i].Status != LD_STATUS_INVALID) {
				if (ldinfo[i].Status == LD_STATUS_OFFLINE
				        && ldinfo[i].BGAStatus == LD_BGA_STATE_RUNNING) {
					ldinfo[i].BGAStatus = LD_BGA_STATE_AUTOPAUSED;
				}
				if (ldinfo[i].Status == LD_STATUS_MISSING) {
					ldinfo[i].Status = LD_STATUS_OFFLINE;
				}
			} else {
				break;
			}
		}
	}
	
	len += sprintf(pBuffer+len,"Index RAID\tStatus  \t\tBGA Status\n");
	for ( i = 0 ; i < LDINFO_NUM ; i++) {
		if ( ldinfo[i].Size.value == 0 ) {
			if ( i == 0 ) {
				len += sprintf(pBuffer+len,"NO Logical Disk\n");
			}
			break;
		}

		len += sprintf(pBuffer+len,
			"%-5d %s\t%s",
			ldinfo[i].ID,
			mv_ld_raid_mode(ldinfo[i].RaidMode),
			mv_ld_status(ldinfo[i].Status)
			);

		tmplen = 24 -strlen(mv_ld_status(ldinfo[i].Status));
		while (j < tmplen) {
			len += sprintf(pBuffer+len, "%s", " ");
			j++;
		}
		j = 0;

		len += sprintf(pBuffer+len, "%s", mv_ld_bga_status(ldinfo[i].BGAStatus));

		if (ldinfo[i].BGAStatus != LD_BGA_STATE_NONE) {
			ret = mv_ld_get_status(pSHost,ldinfo[i].ID,&ld_status);
			if (ret == 0) {
				if (ld_status.Status != LD_STATUS_INVALID) {
					if (ld_status.Status == LD_STATUS_MISSING)
						ld_status.Status = LD_STATUS_OFFLINE;
					ld_status.BgaState = ldinfo[i].BGAStatus;
				}
				len += mv_ld_show_status(pBuffer+len,&ld_status);
				ret = -1;
			}
		}

		tmp = NULL;
		tmplen = 0;
		len += sprintf(pBuffer+len,"\n");
	}

out:
#endif
		
	datalen = len - offset;
	if (datalen < 0 ) {
		datalen = 0;
		*ppStart = pBuffer + len;
	} else {
		*ppStart = pBuffer + offset;
	}
	return datalen;
} 

/*
 *Character Device Interface.
 */

static int mv_open(struct inode *inode, struct file *file)
{
	unsigned int minor_number;
	int retval = -ENODEV;
	unsigned long flags = 0;
	
	MV_LOCK_IRQSAVE(&inode->i_lock, flags);
	minor_number = MINOR(inode->i_rdev);
	if (minor_number >= mv_device_count) {
		printk("MV : No such device.\n");
		goto out;
	}
	retval = 0;
out:
	MV_UNLOCK_IRQRESTORE(&inode->i_lock, flags);
	return retval;
}

#ifdef HAVE_UNLOCKED_IOCTL
static long mv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int mv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, 
		    unsigned long arg)
#endif /* HAVE_UNLOCKED_IOCTL */ 
{
	PHBA_Extension	pHBA;
	PMV_Request    pReq = NULL;
	int error = 0;
	int ret   = 0;
	int sptdwb_size = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
	int console_id  = CONSOLE_ID;
	unsigned long flags;
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER psptdwb = NULL;

#ifdef HAVE_UNLOCKED_IOCTL
	pHBA = mv_device_extension_list[MINOR(file->f_dentry->d_inode->i_rdev)];
#else
	pHBA = mv_device_extension_list[MINOR(inode->i_rdev)];
#endif /* HAVE_UNLOCKED_IOCTL */ 

	if (cmd == 1) {
		if ( copy_to_user((void *)arg,
				(void *)&console_id,
				sizeof(int)) != 0 ) {
			MV_DBG( DMSG_IOCTL, 
				"Marvell : Get CONSOLE_ID Error.\n" );
			return -EIO;
		}
		return 0;
	}

	if (cmd == 2) {
		if ( copy_to_user((void *)arg,
				(void *)&mv_device_count,
				sizeof(unsigned int)) != 0 ) {
			MV_DBG( DMSG_IOCTL, 
				"Marvell : Get Device Number Error.\n" );
			return -EIO;
		}
		return 0;
	}

	psptdwb = kmalloc(sptdwb_size, GFP_ATOMIC);
	
	if ( NULL == psptdwb ) 
		return -ENOMEM;

	error = copy_from_user(psptdwb, (void *)arg, sptdwb_size);

	if (error) {
		ret = -EIO;
		goto clean_psp;
	}

	if (psptdwb->sptd.DataTransferLength) {
		if ( psptdwb->sptd.DataTransferLength > IOCTL_BUF_LEN ) {
			MV_DBG(DMSG_HBA, "__MV__ not enough buf space.\n");
			ret = -ENOMEM;
			goto clean_psp;
		}
			
		psptdwb->sptd.DataBuffer = ioctl_buf;
		memset(ioctl_buf, 0, psptdwb->sptd.DataTransferLength);
		
		error = copy_from_user( psptdwb->sptd.DataBuffer,
					((PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)arg)->sptd.DataBuffer,
					psptdwb->sptd.DataTransferLength);
		if (error) {
			ret = -EIO;
			goto clean_pspbuf;
		}
	} else {
		psptdwb->sptd.DataBuffer = NULL;
	}

	spin_lock_irqsave(&pHBA->lock, flags);
	
	/*Translate request to MV_REQUEST*/	
	pReq = List_GetFirstEntry((&pHBA->Free_Request), MV_Request, Queue_Pointer);	
	if ( NULL == pReq ) {
		ret = -ENOMEM;
		spin_unlock_irqrestore(&pHBA->lock, flags);
		goto clean_pspbuf;
	}

	pReq->Cmd_Initiator = pHBA;
	pReq->Org_Req = pReq;/*No ideas.*/
	pReq->Scsi_Status = psptdwb->sptd.ScsiStatus;
	pReq->Device_Id = psptdwb->sptd.TargetId;
	pReq->Cmd_Flag = 0;

	if (psptdwb->sptd.DataTransferLength == 0) {
		pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
	} else {
		if (SCSI_IS_READ(psptdwb->sptd.Cdb[0]))
			pReq->Cmd_Flag |= CMD_FLAG_DATA_IN;
		if ( SCSI_IS_READ(psptdwb->sptd.Cdb[0]) || SCSI_IS_WRITE(psptdwb->sptd.Cdb[0]) )
			pReq->Cmd_Flag |= CMD_FLAG_DMA;
	}

	pReq->Data_Transfer_Length = psptdwb->sptd.DataTransferLength;
	pReq->Data_Buffer = psptdwb->sptd.DataBuffer;
	pReq->Sense_Info_Buffer = psptdwb->Sense_Buffer;

	SGTable_Init(&pReq->SG_Table, 0);
	if ( psptdwb->sptd.DataTransferLength ) {
		/*GenerateSGTable(pHBA, pSCmd, &pReq->SG_Table);*/
	}
	
	memcpy(pReq->Cdb, psptdwb->sptd.Cdb, MAX_CDB_SIZE);
	pReq->Context = NULL;
	pReq->LBA.value = 0;    
	pReq->Sector_Count = 0; 
	pReq->Completion = ioctlcallback;

	List_Add(&pReq->Queue_Pointer, &pHBA->Waiting_Request);
    	pHBA->Io_Count++;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&pHBA->hba_sync, 1);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
	/*Handle Request.*/
	HBA_HandleWaitingList(pHBA);
	spin_unlock_irqrestore(&pHBA->lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	if ( !__hba_wait_for_atomic_timeout(&pHBA->hba_sync, 
					    HBA_REQ_TIMER_IOCTL*HZ) ) {
#else
	if ( !wait_for_completion_timeout(&pHBA->cmpl, 
					  HBA_REQ_TIMER_IOCTL*HZ) ) {
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
		MV_DBG(DMSG_HBA, "__MV__ ioctl req timed out.\n");
	        ret = -EIO;
	        goto clean_pspbuf;
	}

	if (psptdwb->sptd.DataTransferLength) {
		error = copy_to_user(((PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)arg)->sptd.DataBuffer,
				     psptdwb->sptd.DataBuffer,
				     psptdwb->sptd.DataTransferLength);
		if (error) {
			ret = -EIO;
			goto clean_pspbuf;
		}
	}
		
	error = copy_to_user((void*)arg, psptdwb, sptdwb_size);
	
	if (error)
		ret = -EIO;

clean_pspbuf:
/* use static buf instead.
	if (psptdwb->sptd.DataBuffer)
	kfree(psptdwb->sptd.DataBuffer);
*/
clean_psp:
	kfree(psptdwb);
	return ret;
}

