#if !defined(HBA_INTERNAL_H)

#define HBA_INTERNAL_H

#include "hba_header.h"

struct _HBA_Extension
{
	/* Device extention */
	MV_PVOID host_data;

	struct list_head        next;
	struct pci_dev 		*pcidev;
	spinlock_t              lock;
	struct semaphore	sem;
	struct timer_list	timer;
	struct Scsi_Host	*host;

	MV_PVOID 	uncached_virtual_address[MAX_MODULE_NUMBER];
	MV_U32          uncached_size[MAX_MODULE_NUMBER];
	dma_addr_t      uncached_physical[MAX_MODULE_NUMBER];


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_t                hba_sync;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
	struct completion       cmpl;

	/* System resource */
	MV_PVOID                Base_Address[MAX_BASE_ADDRESS];

	MV_U32                  State;	
	/* Is OS during hibernation or crash dump? */
	MV_BOOLEAN              Is_Dump;
	/* Outstanding requests count */
	MV_U8                   Io_Count;
	/* Maximum requests number we can handle */
	MV_U16                  Max_Io;

	/* Adapter information */
	MV_U8                   Adapter_Bus_Number;
	MV_U8                   Adapter_Device_Number;
	MV_U16                  Vendor_Id;
	MV_U16                  Device_Id;
	MV_U8                   Revision_Id;
	MV_U8                   Reserved0;

	/* Module management related variables */
	struct _Module_Manage   Module_Manage;

	/* Timer module */
	struct _Timer_Module    Timer_Module;

	/* Free MV_Request queue */
	List_Head               Free_Request;
	/* MV_Request waiting queue */
	List_Head               Waiting_Request;

#ifdef SUPPORT_EVENT
	List_Head               Stored_Events;
	List_Head               Free_Events;
	MV_U32	                SequenceNumber;
	MV_U8                   Num_Stored_Events;
#endif /* SUPPORT_EVENT */

#ifdef CACHE_MODULE_SUPPORT
	MV_PVOID                cache_res;
#endif
	/* 
	 * Memory pool can be used as variable data structures like timer 
	 * This item must always be put at the end of this data structure.
	 */
	MV_U8                   Memory_Pool[1];
};

#define DRIVER_STATUS_IDLE      1    /* The first status */
#define DRIVER_STATUS_STARTING  2    /* Begin to start all modules */
#define DRIVER_STATUS_STARTED   3    /* All modules are all settled. */

#endif /* HBA_INTERNAL_H */
