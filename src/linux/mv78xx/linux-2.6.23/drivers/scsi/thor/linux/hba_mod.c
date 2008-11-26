#include "mv_include.h"
#include "linux_main.h"
#include "hba_mod.h"

#include "hba_inter.h"

/*
 * Pre-defined module function table
 * Please match this predefined module set with the enum Module_Id
 * Pay attention: If you want to change the following data structure,
 * please change Module_Id as well.
 */
Module_Interface module_set[MAX_MODULE_NUMBER] = 
{
	{
		MODULE_HBA,
		HBA_ModuleGetResourceQuota,
		HBA_ModuleInitialize,
		HBA_ModuleStart,
		HBA_ModuleShutdown,
		NULL, /* - USE ME AND DIE!! HBA_ModuleNotification, */
		HBA_ModuleSendRequest,
		HBA_ModuleReset,
		HBA_ModuleMonitor,
	#ifdef SUPPORT_VIRTUAL_AND_PHYSICAL_SG
		HBA_ModuleGetVirtualSG
	#endif
	},
#ifdef CACHE_MODULE_SUPPORT
	{
		MODULE_CACHE,
		Cache_ModuleGetResourceQuota,
		Cache_ModuleInitialize,
		Cache_ModuleStart,
		Cache_ModuleShutdown,
		Cache_ModuleNotification,
		Cache_ModuleSendRequest,
		Cache_ModuleReset,
		Cache_ModuleMonitor,
	#ifdef SUPPORT_VIRTUAL_AND_PHYSICAL_SG
		Cache_ModuleGetVirtualSG
	#endif
	},
#endif
#ifdef RAID_DRIVER
	{
		MODULE_RAID,
		RAID_ModuleGetResourceQuota,
		RAID_ModuleInitialize,
		RAID_ModuleStart,
		RAID_ModuleShutdown,
		RAID_ModuleNotification,
		RAID_ModuleSendRequest,
		RAID_ModuleReset,
		RAID_ModuleMonitor,
	#ifdef SUPPORT_VIRTUAL_AND_PHYSICAL_SG
		RAID_ModuleGetVirtualSG
	#endif
	},
#endif
	{
		MODULE_CORE,
		Core_ModuleGetResourceQuota,
		Core_ModuleInitialize,
		Core_ModuleStart,
		Core_ModuleShutdown,
		Core_ModuleNotification,
		Core_ModuleSendRequest,
		Core_ModuleReset,
		Core_ModuleMonitor,
	#ifdef SUPPORT_VIRTUAL_AND_PHYSICAL_SG
		NULL
	#endif
	}
};

void Module_InitializeAll(PModule_Manage p_module_manage, MV_U16 max_io)
{
	MV_I8 i = 0;
	MV_PVOID module_extension = NULL;
	MV_U32 extension_size = 0;

	/* Module initialization is one synchronized function. */
	for ( i=MAX_MODULE_NUMBER-1; i>=0; i-- )
	{
		/* I use this chance to check whether the module_set matches with Module_Id */
		MV_ASSERT( module_set[i].module_id==i );

		if ( module_set[i].module_initialize )
		{
			module_extension = p_module_manage->resource[i].module_extension;
			extension_size = p_module_manage->resource[i].extension_size;
			module_set[i].module_initialize(module_extension, extension_size, max_io);
		}
	}
}

void Module_StartAll(PModule_Manage p_module_manage, MV_U8 begin_module)
{
	MV_I8 i = 0;

	/* 
	 * Start module from the lower level, the first one is the core driver.
	 * Every time we only start one module.
	 */
	for ( i=begin_module; i>=0; i-- )
	{
		MV_ASSERT(begin_module<MAX_MODULE_NUMBER);
		if ( module_set[i].module_start )
		{
			module_set[i].module_start(
				p_module_manage->resource[i].module_extension);
			return;
		}

		/* If the module_start function is NULL, continue to the next. */
		p_module_manage->status |= (1<<i);
	}
}

MV_U32 mod_get_mem_size(PHBA_Extension pHBA, enum Resource_Type type,
			MV_U16 max_io)
{
        int i = 0;
        unsigned long quota = 0;
        unsigned long  oneQuota = 0;

        for (i=0; i<MAX_MODULE_NUMBER; i++) {
                if (module_set[i].get_mem_size != NULL) {
                        oneQuota = module_set[i].get_mem_size(type, max_io);
                        quota += ROUNDING(oneQuota, 8);
                        MV_DBG(DMSG_KERN,
                               "%s quota for module %d is 0x%lx.\n", 
                               type == RESOURCE_CACHED_MEMORY? "Cached memory" : "Uncached memory",
                               i, 
                               oneQuota);

                        if (oneQuota) {
                                if (type == RESOURCE_UNCACHED_MEMORY) {
                                        MV_PVOID uncached_virtual = NULL;
                                        uncached_virtual = pci_alloc_consistent(pHBA->pcidev,
                                                                                                  oneQuota,
                                                                                                  &pHBA->uncached_physical[i]);
                                        pHBA->uncached_size[i] = oneQuota;
                                        if (uncached_virtual != NULL)
                                                pHBA->uncached_virtual_address[i] = uncached_virtual;
#ifdef CACHE_MODULE_SUPPORT
                                        else if (i == MODULE_CACHE) 
                                                MV_DPRINT(("Module %d asks for uncached memory failed.\n", i));
#endif
                                        else
                                                return -1;
                                }
                        }
                }
        }

        /* Each extension needs one extension header which is hidden from module. */
        if ( type==RESOURCE_CACHED_MEMORY )
                quota += MODULE_HEADER_SIZE * MAX_MODULE_NUMBER;

        MV_DBG(DMSG_KERN, "%s quota totally is 0x%lx.\n",
                type==RESOURCE_CACHED_MEMORY? "Cached memory" : "Uncached memory",
                quota);

        return quota;
}

void Module_AssignModuleExtension(MV_PVOID device_extension, 
				  MV_U16 max_io)
{
	MV_PTR_INTEGER ptemp = (MV_PTR_INTEGER)device_extension;
	PHBA_Extension pHBA = NULL;
	PModule_Manage module_manage = NULL;
	PModule_Header header = NULL;
	MV_U8 module_id;
	MV_U32 require;

	MV_ASSERT(MODULE_HBA==0);
	pHBA = (PHBA_Extension)( (MV_PTR_INTEGER)device_extension+MODULE_HEADER_SIZE );
	module_manage = &pHBA->Module_Manage;

	for (module_id=0; module_id<MAX_MODULE_NUMBER; module_id++) {
		if (module_set[module_id].get_mem_size==NULL )
			continue;

		require = module_set[module_id].get_mem_size(RESOURCE_CACHED_MEMORY, max_io);
		require = ROUNDING(require, 8);
		
		header = (PModule_Header)ptemp;
		header->extension_size = require;
		header->header_size = MODULE_HEADER_SIZE;
		header->module_id = module_id;
		header->hba_extension = pHBA;

		module_manage->resource[module_id].module_extension = (MV_PVOID)(ptemp+MODULE_HEADER_SIZE);
		module_manage->resource[module_id].extension_size = require;

		ptemp += MODULE_HEADER_SIZE+require;
	}
}

void
Module_AssignUncachedMemory(
	IN PModule_Manage module_manage,
	IN MV_PVOID virtual_addr,
	IN MV_PHYSICAL_ADDR physical_addr,
	IN MV_U32 memory_size,
	IN MV_U16 max_io,
	MV_U8 module_id
	)
{
	MV_PTR_INTEGER temp_virtual = (MV_PTR_INTEGER)virtual_addr;
	//MV_PHYSICAL_ADDR temp_physical = physical_addr;

	MV_U32 require;

	/* Assign Uncached Memory */
	if ( module_set[module_id].get_mem_size == NULL )
		return;

	require = module_set[module_id].get_mem_size(RESOURCE_UNCACHED_MEMORY,
						     max_io);
	require = ROUNDING(require, 8);	

	module_manage->resource[module_id].uncached_size = require;
	module_manage->resource[module_id].uncached_address = (MV_PVOID)virtual_addr;
	module_manage->resource[module_id].uncached_physical_address = physical_addr;

	temp_virtual += require;
	/* Do we have enough uncached memory? */
	MV_ASSERT( (temp_virtual-(MV_PTR_INTEGER)virtual_addr)<=memory_size );
}

void Module_ShutdownAll(PModule_Manage p_module_manage)
{
	MV_I8 i = 0;
	MV_PVOID module_extension = NULL;

	/* Module stop is one synchronized function. */
	for ( i=MAX_MODULE_NUMBER-1; i>=0; i-- )
	{
		if ( module_set[i].module_stop )
		{
			module_extension = p_module_manage->resource[i].module_extension;
			module_set[i].module_stop(module_extension);
		}
	}
}


void *mv_hba_init_ext(struct pci_dev *dev)
{
	int i;

	PModule_Header pheader;
	PHBA_Extension phba;
	PModule_Manage pmod;

	unsigned long total_size = 0;
	unsigned long size = 0;

	unsigned long addr;
	unsigned long range;

	dma_addr_t    dma_addr;
	BUS_ADDRESS   bus_addr;
	MV_PHYSICAL_ADDR phy_addr;
	

	/* allocate normal (CACHED) mem */
	for (i=0; i<MAX_MODULE_NUMBER; i++) {
		size = module_set[i].get_mem_size(RESOURCE_CACHED_MEMORY,
						  MAX_REQUEST_NUMBER);

		if ( 0 != size )
			total_size += ROUNDING(size, 8);
		
		WARN_ON(size != ROUNDING(size, 8));
		
	}

	/* init hba ext structure */
	total_size += ROUNDING(MODULE_HEADER_SIZE * MAX_MODULE_NUMBER, 8);

	MV_DBG(DMSG_HBA, "THOR : Memory quota is 0x%lx bytes.\n",
	       total_size);

	pheader = (PModule_Header) vmalloc(total_size);
	if ( NULL == pheader )
		return NULL;

	memset(pheader, 0, total_size);
	Module_AssignModuleExtension(pheader, MAX_REQUEST_NUMBER);
	
	phba = (PHBA_Extension) head_to_hba(pheader);
	phba->host_data = pheader;
	phba->pcidev    = dev;
	phba->Vendor_Id = dev->vendor;
	phba->Device_Id = dev->device;

	/* map pci resource */
	if (pci_read_config_byte(dev, PCI_REVISION_ID, &phba->Revision_Id)) {
		printk("THOR : Failed to get hba's revision id.\n");
		goto ext_err_mem;
	}
	
	for (i=0; i<MAX_BASE_ADDRESS; i++) {
		addr  = pci_resource_start(dev, i);
		range = pci_resource_len(dev, i);

		if (pci_resource_flags(dev, i) & IORESOURCE_MEM)
			phba->Base_Address[i] =(MV_PVOID) ioremap(addr, range);
		else
			phba->Base_Address[i] =(MV_PVOID) addr;

		MV_DBG(DMSG_HBA, "THOR : BAR %d : %p.\n", i, 
		       phba->Base_Address[i]);
	}
	
	/* allocate consistent dma memory (uncached) */
	size = 0;
	total_size = 0;
	pmod = &phba->Module_Manage;
	
	for (i=0; i<MAX_MODULE_NUMBER; i++) {
	MV_DBG(DMSG_HBA, "THOR : i = %d module_set[i].get_mem_size %p\n", i,
	       module_set[i].get_mem_size);
	
		size = module_set[i].get_mem_size(RESOURCE_UNCACHED_MEMORY,
						  MAX_REQUEST_NUMBER);
		if (0 == size) 
			continue;
		WARN_ON(size != ROUNDING(size, 8));

		size = ROUNDING(size, 8);
		pmod->resource[i].uncached_address = (MV_PVOID) \
			pci_alloc_consistent(dev, size, &dma_addr);

		if ( NULL == pmod->resource[i].uncached_address )
			goto ext_err_dma;
		
		pmod->resource[i].uncached_size = size;
		bus_addr = (BUS_ADDRESS) dma_addr;
		phy_addr.low  = LO_BUSADDR(bus_addr);
		phy_addr.high = HI_BUSADDR(bus_addr);
		pmod->resource[i].uncached_physical_address = phy_addr;
		
	}

	MV_DBG(DMSG_HBA, "THOR : HBA ext struct init'ed at %p.\n", phba);
	return phba;

ext_err_dma:
	for (i=0; i<MAX_MODULE_NUMBER; i++) {
		if ( pmod->resource[i].uncached_size ) {
			phy_addr = pmod->resource[i].uncached_physical_address;
			dma_addr = (dma_addr_t) ( phy_addr.low | \
						  ((u64) phy_addr.high)<<32 );
			pci_free_consistent(dev, 
					    pmod->resource[i].uncached_size,
					    pmod->resource[i].uncached_address,
					    dma_addr);
		}
	}
ext_err_mem:
	vfree(pheader);
	return NULL;
}

void mv_hba_release_ext(PHBA_Extension phba)
{
	int i;
	
	dma_addr_t dma_addr;
	MV_PHYSICAL_ADDR phy_addr;
	
	PModule_Manage pmod  = &phba->Module_Manage;
	
	for (i=0; i<MAX_MODULE_NUMBER; i++) {
		if ( pmod->resource[i].uncached_size ) {
			phy_addr = pmod->resource[i].uncached_physical_address;
			dma_addr = (dma_addr_t) ( phy_addr.low | \
						  ((u64) phy_addr.high)<<32 );
			pci_free_consistent(phba->pcidev, 
					    pmod->resource[i].uncached_size,
					    pmod->resource[i].uncached_address,
					    dma_addr);
		}
	}
	
	vfree(phba->host_data);
}
