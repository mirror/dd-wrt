#if !defined(MODULE_MANAGE_H)

#define MODULE_MANAGE_H

#include "hba_exp.h"
#include "core_exp.h"
#ifdef RAID_DRIVER
#include "raid_exp.h"
#endif

/*
 * Module definition
 */
typedef struct _Module_Interface
{
	MV_U8 module_id;
	MV_U32 (*get_mem_size)(enum Resource_Type type, MV_U16 max_io);
	void (*module_initialize)(MV_PVOID extension, MV_U32 extension_size, MV_U16 max_io);
	void (*module_start)(MV_PVOID extension);
	void (*module_stop)(MV_PVOID extension);
	void (*module_notification)(MV_PVOID extension, enum Module_Event event, MV_PVOID param);
	void (*module_sendrequest)(MV_PVOID extension, PMV_Request pReq);
	void (*module_reset)(MV_PVOID extension);
	void (*module_monitor)(MV_PVOID extension);
} Module_Interface, *PModule_Interface;

/*
 * Module Management
 */
typedef struct _Module_Header
{
	/* 
	 * Here is the hidden module header. 
	 * Module is not aware of this except for HBA module management.
	 */
	MV_U8		header_size;		/* Module header size */
	MV_U8		module_id;			/* It's also the module is in enum Module_Id */
	MV_U8		reserved0[2];
	MV_U32		extension_size;		/* size of the extension, header is not included. */
	MV_PVOID	hba_extension;		/* point to the pHBA extension, not the header. */

} Module_Header, * PModule_Header;

/* Size must be 64 bit rounded. */
#define MODULE_HEADER_SIZE	ROUNDING(sizeof(Module_Header), 8)

#define Module_GetModuleHeader(extension)	\
	((PModule_Header)((MV_PTR_INTEGER)extension-MODULE_HEADER_SIZE))

#define Module_GetModuleId(extension)		\
	(Module_GetModuleHeader(extension)->module_id)

#define Module_GetHBAExtension(extension)	\
	(Module_GetModuleHeader(extension)->hba_extension)

#define Module_IsStarted(p_module_manage, module_id)	\
	(p_module_manage->status&=(1<<(module_id)))

#define head_to_hba(head)	\
	((MV_PTR_INTEGER)head+MODULE_HEADER_SIZE)

typedef struct _Module_Resource
{
	/* Extension assigned to this module */
	MV_PVOID	    module_extension;
	MV_U32		    extension_size;	/* Extension size */
	MV_U32		    uncached_size;	/* Uncached memory size */
        /* Uncached memory virtual address */
	MV_PVOID	    uncached_address;	
	/* Uncached memory physical address */
	MV_PHYSICAL_ADDR    uncached_physical_address;	
} Module_Resource, *PModule_Resource;

typedef struct _Module_Manage
{
	Module_Resource resource[MAX_MODULE_NUMBER];
        /* One bit for one module. If started, the bit is set. */
	MV_U8			status;	
	MV_U8			reserved0[7];
} Module_Manage, *PModule_Manage;

void HBA_ModuleStarted(MV_PVOID This);

MV_U32 mod_get_mem_size(PHBA_Extension pHBA, enum Resource_Type type, 
			MV_U16 max_io);

void 
Module_AssignModuleExtension(
	IN MV_PVOID device_extension, 
	IN MV_U16 max_io
	);

void
Module_AssignUncachedMemory(
	IN PModule_Manage module_manage,
	IN MV_PVOID virtual_addr,
	IN MV_PHYSICAL_ADDR physical_addr,
	IN MV_U32 memory_size,
	IN MV_U16 max_io,
	IN MV_U8 module_id
	);

void 
Module_InitializeAll(
	IN PModule_Manage p_module_manage,
	IN MV_U16 max_io
	);

void 
Module_StartAll(
	IN PModule_Manage p_module_manage, 
	IN MV_U8 begin_module
	);

void Module_ShutdownAll(IN PModule_Manage p_module_manage);

void *mv_hba_init_ext(struct pci_dev *dev);
void mv_hba_release_ext(PHBA_Extension phba);

#endif /* MODULE_MANAGE_H */

