/*
 * Linux Driver for 61xx
 * Copyright (C) 2006 Marvell Technology Group Ltd.. All Rights Reserved.
 * linux_main.c
 * lily initialized on Feb. 15 2006
 *
 *  ioctl handler has been implemented.
 *  June 2006, Zax Liu < zaxl at marvell dot com >
 *
 *  implement ioctl the 2.6.11 plus way ( not rely on BKL )
 *  July 2006, Albert Ke < ake at marvell dot com >
 *
 */

#include "mv_os.h"
#include "mv_include.h"

#include "hba_header.h"

#include "linux_main.h"
#include "linux_iface.h"
#include "linux_helper.h"

#include "com_define.h" 
#include "com_type.h"
#include "mv_config.h"
#include "mv_include.h"

/* 
 * module parameter 
 *
 * refer to ../common/com_dbg.h 
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 7)
unsigned int mv_dbg_opts = 0;
module_param(mv_dbg_opts, uint, S_IRWXU | S_IRWXG);
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 7) */

static const struct pci_device_id mv_pci_ids[] = {
	{PCI_DEVICE(VENDOR_ID, DEVICE_ID_THORLITE_0S1P)},
	{PCI_DEVICE(VENDOR_ID, DEVICE_ID_THORLITE_2S1P)},
	{PCI_DEVICE(VENDOR_ID, DEVICE_ID_THOR_4S1P)},
	{PCI_DEVICE(VENDOR_ID, DEVICE_ID_THOR_4S1P_NEW)},
	{PCI_DEVICE(VENDOR_ID, DEVICE_ID_THORLITE_2S1P_WITH_FLASH)},
	{0}
};

static struct list_head mv_hba_ext_list;

unsigned int mv_device_count;
/* TODO : try to get rid of this - A.C. */
PHBA_Extension mv_device_extension_list[MV_DEVICE_MAX_SLOT];

int mv_major = -1;
extern struct file_operations mv_fops;

static void release_host(PHBA_Extension phba)
{
	unsigned long flags;
	int i;
	struct pci_dev *pcidev = phba->pcidev;

	scsi_remove_host(phba->host);
	scsi_host_put(phba->host);

	phba->host = NULL;

	hba_send_shutdown_req(phba);

	spin_lock_irqsave(&phba->lock, flags);
	del_timer_sync(&phba->timer);
	spin_unlock_irqrestore(&phba->lock, flags);

	free_irq(phba->pcidev->irq, phba);

	for ( i=0; i<MAX_BASE_ADDRESS; i++ )
		if (pci_resource_flags(pcidev, i) & IORESOURCE_MEM)
			iounmap(phba->Base_Address[i]);

	mv_hba_release_ext(phba);
	mv_device_count--;
}


/* notifier block to get notified on system shutdown/halt/reboot/down */
static int mv_linux_halt(struct notifier_block *nb, unsigned long event,
			 void *buf)
{
	PHBA_Extension phba = NULL;
	unsigned long flags;

	switch (event) {
	case SYS_RESTART:
	case SYS_HALT:
	case SYS_POWER_OFF:
		list_for_each_entry(phba, &mv_hba_ext_list, next) {
			hba_send_shutdown_req(phba);
			
			spin_lock_irqsave(&phba->lock, flags);
			del_timer_sync(&phba->timer);
			spin_unlock_irqrestore(&phba->lock, flags);
		}
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block mv_linux_notifier = {
	mv_linux_halt, NULL, 0
};


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
static irqreturn_t mv_intr_handler(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t mv_intr_handler(int irq, void *dev_id)
#endif
{
	/* MV_FALSE should be equal to IRQ_NONE (0) */
	irqreturn_t retval = MV_FALSE;
	unsigned long flags;
	MV_PVOID pcore;

	PHBA_Extension pHBA = (PHBA_Extension)dev_id;
	PModule_Manage module_manage = &pHBA->Module_Manage;
	
	spin_lock_irqsave(&pHBA->lock, flags);
	pcore = module_manage->resource[MODULE_CORE].module_extension;
	retval = Core_InterruptServiceRoutine(pcore);
	spin_unlock_irqrestore(&pHBA->lock, flags);

	return IRQ_RETVAL(retval);
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 7)
static enum scsi_eh_timer_return mv_linux_timed_out(struct scsi_cmnd *cmd)
{
	static int i;

	MV_DBG(DMSG_SCSI, "__MV__ scmd timed out : ");
	MV_DBG( DMSG_SCSI,
		"%p (%d/%d/%d cdb=(%x-%x-%x)).\n", 
		cmd, mv_scmd_channel(cmd), 
		mv_scmd_target(cmd), mv_scmd_lun(cmd),
		*(cmd->cmnd), *(cmd->cmnd+1), 
		*(cmd->cmnd+2) );

	if ( i++ > 5 )
		return EH_NOT_HANDLED;
	else
		return EH_RESET_TIMER;
}
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 7) */


static int mv_linux_queue_command(struct scsi_cmnd *pSCmd, 
				  void (*done) (struct scsi_cmnd *))
{
	struct Scsi_Host *phost = mv_scmd_host(pSCmd);
	PModule_Header pheader = get_hba_ext_header(phost);
	PHBA_Extension pHBA = (PHBA_Extension)head_to_hba(pheader);
	PMV_Request pReq;
	unsigned long flags;

	if ( done == NULL ) {
		MV_PRINT( ": in queuecommand, done function can't be NULL\n");
		return 0;
    	}

	spin_lock_irqsave(&pHBA->lock, flags);

	MV_DBG(DMSG_SCSI_FREQ,
	       "mv_linux_queue_command %p (%d/%d/%d/%d cdb=(%x-%x-%x))\n", 
	       pSCmd, phost->host_no, mv_scmd_channel(pSCmd), 
	       mv_scmd_target(pSCmd), mv_scmd_lun(pSCmd),
	       *(pSCmd->cmnd), *(pSCmd->cmnd+1), 
	       *(pSCmd->cmnd+2));

	pSCmd->result = 0;
 	pSCmd->scsi_done = done;
	MV_SCp(pSCmd)->bus_address = 0;
	MV_SCp(pSCmd)->mapped = 0;
	MV_SCp(pSCmd)->map_atomic = 0;
	
	if ( mv_scmd_channel(pSCmd) ) {
		pSCmd->result = DID_BAD_TARGET << 16;
		goto done;
	}

	/* 
	 * Get mv_request resource and translate the scsi_cmnd request to mv_request.
	 */
	MV_DASSERT( !List_Empty(&pHBA->Free_Request) );
	pReq = List_GetFirstEntry((&pHBA->Free_Request), MV_Request, Queue_Pointer);
	if ( pReq == NULL ) {
		spin_unlock_irqrestore(&pHBA->lock, flags);
		return SCSI_MLQUEUE_HOST_BUSY;
	}
	
	if ( !TranslateOSRequest(pHBA,pSCmd, pReq) ) {
		/* 
		 * Even TranslateOSRequest failed, 
		 * it still should set some of the variables to the MV_Request
		 * especially MV_Request.Org_Req and MV_Request.Scsi_Status;
		 */
		MV_DBG( DMSG_HBA,
			"ERROR - Translation from OS Request failed.\n" );
		/* this is tihs */
		pHBA->Io_Count++;

		HBARequestCallback(pHBA, pReq);
		spin_unlock_irqrestore(&pHBA->lock, flags);
		return 0;
	}

	/* 
	 * Queue this request. 
	 * Cannot return with BUSY when core driver is not ready. It'll fail hibernation. 
	 */
	List_AddTail(&pReq->Queue_Pointer, &pHBA->Waiting_Request);
	pHBA->Io_Count++;

	if ( pHBA->State != DRIVER_STATUS_STARTED ) {
		MV_ASSERT(0);
		/*if ( pHBA->State==DRIVER_STATUS_IDLE )
		  {
		  pHBA->State = DRIVER_STATUS_STARTING;
		  Module_StartAll(module_manage, MODULE_CORE);
		  }*/
	} else {
		HBA_HandleWaitingList(pHBA);
	}
	spin_unlock_irqrestore(&pHBA->lock, flags);

	return 0;
done:
	pSCmd->scsi_done(pSCmd);
	spin_unlock_irqrestore(&pHBA->lock, flags);
	return 0;
}

#if 0
static void ac_dump_info(struct scsi_cmnd *cmd)
{
	return;
}

static int mv_linux_abort(struct scsi_cmnd *cmd)
{
	struct Scsi_Host *host;
	PHBA_Extension phba;
	int  ret = FAILED;

	MV_PRINT("__MV__ abort command %p.\n", cmd);

	return ret;
}
#endif /* 0 */

static int mv_linux_reset (struct scsi_cmnd *cmd)
{
	MV_PRINT("__MV__ reset handler %p.\n", cmd);
	return FAILED;
}

struct scsi_host_template mv_driver_template = {
	module:                         THIS_MODULE,                         
	name:                           "Marvell 88SE61xx Storage Controller",
	proc_name:                      mv_driver_name,
	proc_info:                      mv_linux_proc_info,
	queuecommand:                   mv_linux_queue_command,
#if 0
	eh_abort_handler:		mv_linux_abort,
	eh_device_reset_handler:	mv_linux_reset,
	eh_bus_reset_handler:		mv_linux_reset,
#endif /* 0 */
	eh_host_reset_handler:		mv_linux_reset,
#if  LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 7) && \
	LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 16)
	eh_timed_out:                   mv_linux_timed_out,
#endif
	/* save 2 for ioctl */
	can_queue:                      MAX_REQUEST_NUMBER-2,
	this_id:                        -1,
	max_sectors:                    MV_MAX_TRANSFER_SECTOR,
	sg_tablesize:                   MAX_SG_ENTRY,
	cmd_per_lun:                    MAX_REQUEST_NUMBER-2,
	use_clustering:                 DISABLE_CLUSTERING,
	emulated:                       0
};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 15) /* should be .16 */
static struct scsi_transport_template mv_transport_template = {
        .eh_timed_out   =  mv_linux_timed_out,
};
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 17) */

static int mv_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	unsigned int ret = PCIBIOS_SUCCESSFUL;
	struct Scsi_Host *shost = NULL;
	PHBA_Extension   phba  = NULL;
	
	ret = pci_enable_device(dev);
	if (ret) {
		printk("THOR : enable device failed.\n");
		return ret;
	}
	
	ret = pci_request_regions(dev, mv_driver_name);
	if (ret)
		goto err_req_region;
	
	
	if ( !pci_set_dma_mask(dev, DMA_64BIT_MASK) ) {
		ret = pci_set_consistent_dma_mask(dev, DMA_64BIT_MASK);
		if (ret) {
			ret = pci_set_consistent_dma_mask(dev, 
							  DMA_32BIT_MASK);
			if (ret)
				goto err_dma_mask;
		}
	} else {
		ret = pci_set_dma_mask(dev, DMA_32BIT_MASK);
		if (ret)
			goto err_dma_mask;
		
		ret = pci_set_consistent_dma_mask(dev, DMA_32BIT_MASK);
		if (ret) 
			goto err_dma_mask;
		
	}
		
	pci_set_master(dev);

	printk("Marvell (S)ATA Controller is found, using IRQ %d.\n",
	       dev->irq);
	
	phba = (PHBA_Extension) mv_hba_init_ext(dev);

	if ( NULL == phba ) {
		ret = -ENOMEM;
		goto err_dma_mask;
	}
	
	list_add_tail(&phba->next, &mv_hba_ext_list);
		
	/* increase hba counter? */

	spin_lock_init(&phba->lock);

	Module_InitializeAll(&phba->Module_Manage, MAX_REQUEST_NUMBER);

	init_timer(&phba->timer);
	sema_init(&phba->sem, 0);
	init_completion(&phba->cmpl);
	
	spin_lock_irq(&phba->lock);
	Module_StartAll(&phba->Module_Manage, MODULE_CORE);
	spin_unlock_irq(&phba->lock);

	shost = scsi_host_alloc(&mv_driver_template, sizeof(void *));
	
	if ( NULL == shost ) {
		printk("THOR : Unable to allocate a scsi host.\n");
		goto err_host_alloc;
	}
	
	/* TODO : a saner way is needed - A.C. */
	*((PModule_Header *)shost->hostdata) = \
		Module_GetModuleHeader(phba);

	phba->host = shost;

	shost->irq          = dev->irq;
	shost->max_id       = MV_MAX_TARGET_NUMBER;
	shost->max_lun      = MV_MAX_LUN_NUMBER;
	shost->max_channel  = 0;
	shost->max_cmd_len  = 16;
	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 15) /* should be .16 */
        shost->transportt   = &mv_transport_template;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 19)
	if ((ret = request_irq(dev->irq, mv_intr_handler, IRQF_SHARED,
				mv_driver_name, phba)) < 0) {
#else
	if ((ret = request_irq(dev->irq, mv_intr_handler, SA_SHIRQ,
				mv_driver_name, phba)) < 0) {
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 19) */
		printk("THOR : Error upon requesting IRQ %d.\n", dev->irq);
		goto  err_request_irq;
	}

	/* wait for MODULE(CORE,RAID,HBA) init */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&phba->hba_sync, 1);
	if (0 == __hba_wait_for_atomic_timeout(&phba->hba_sync, 30 * HZ)) {
#else
	if (0 == wait_for_completion_timeout(&phba->cmpl, 30 * HZ)) {
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
		ret = -ENODEV;
		goto err_wait_irq;

	}

	if (mv_device_count == 0) {
		register_reboot_notifier(&mv_linux_notifier); 
		hba_house_keeper_run();
	}
	
	/* TODO : I'm sure there's a better/saner way - A.C. */
	if (-1 == mv_major) {
		if ((mv_major = register_chrdev(0, 
						mv_driver_name, 
						&mv_fops)) < 0) {
			printk("THOR : Failed to register character device");
			ret = -ENODEV;
			goto err_register_chrdev;
		}
	}
	
	mv_device_extension_list[mv_device_count++] = phba;

	if (0 != (ret = scsi_add_host(shost, &dev->dev)))
		goto err_add_host;

	scsi_scan_host(shost);

	return 0;

err_add_host:
	if (mv_major >= 0)
		unregister_chrdev(mv_major, mv_driver_name);

err_register_chrdev:
	if (mv_device_count == 0) {
		unregister_reboot_notifier(&mv_linux_notifier); 
	}
	
err_wait_irq:
	free_irq(dev->irq, phba);	

err_request_irq:
	scsi_host_put(shost);

err_host_alloc:
	list_del(&phba->next);
	Module_ShutdownAll(&phba->Module_Manage);
	mv_hba_release_ext(phba);

err_dma_mask:
	pci_release_regions(dev);

err_req_region:
	pci_disable_device(dev);

	return ret;
}

static void mv_remove(struct pci_dev *dev)
{
	PHBA_Extension phba;
	
	list_for_each_entry(phba, &mv_hba_ext_list, next) {
		if ( phba->pcidev == dev ) {
			list_del(&phba->next);
			release_host(phba);

			pci_release_regions(dev);
			pci_disable_device(dev);
			break; /* one hba for one pci device */
		}
	}

	if (mv_device_count == 0)
		unregister_reboot_notifier(&mv_linux_notifier); 
}

static struct pci_driver mv_pci_driver = {
	.name     = "mv_thor",
	.id_table = mv_pci_ids,
	.probe    = mv_probe,
	.remove   = mv_remove,
};

static int __init mv_linux_driver_init(void)
{
	/* default to only show no msg */
	mv_dbg_opts = 0;

	INIT_LIST_HEAD(&mv_hba_ext_list);
	/* bg thread init - refer to hba_timer.[ch] */
	hba_house_keeper_init();
	
	return pci_register_driver(&mv_pci_driver);
}

static void __exit mv_linux_driver_exit(void)
{
	if (mv_major >= 0) {
		unregister_chrdev(mv_major, mv_driver_name);
	}

	hba_house_keeper_exit();
	
	pci_unregister_driver(&mv_pci_driver);
}

MODULE_AUTHOR ("Marvell Semiconductor Inc.,");
MODULE_DESCRIPTION ("thor SATA hba driver");

//MODULE_LICENSE("Proprietary");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(pci, mv_pci_ids);

module_init(mv_linux_driver_init);
module_exit(mv_linux_driver_exit);

