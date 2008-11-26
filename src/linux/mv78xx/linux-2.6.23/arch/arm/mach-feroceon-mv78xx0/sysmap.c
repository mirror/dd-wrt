

#include "mvSysHwConfig.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "mvCommon.h"
#include <asm/mach/map.h>

/* for putstr */
/* #include <asm/arch/uncompress.h> */

MV_CPU_DEC_WIN* mv_sys_map(void);
u32 mv_pci_mem_size_get(int ifNum);
u32 mv_pci_io_base_get(int ifNum);
u32 mv_pci_io_size_get(int ifNum);
MV_TARGET mv_pci_io_target_get(int ifNum);
u32 mv_pci_mem_base_get(int ifNum);
#if defined(CONFIG_MV_INCLUDE_CESA)
u32 mv_crypto_base_get(void);
#endif
#if defined(CONFIG_MV_INCLUDE_DEVICE_CS1)
u32 mv_device_cs1_base_get(void);
#endif

#include "mvDB_78XX0.h"
#include "mvRD_78XX0_AMC.h"
#include "mvRD_78XX0_H3C.h"
#include "mvRD_78XX0_MASA.h"

#include "sysmap_db78xx0.h"
#include "sysmap_rd78xx0_amc.h"
#include "sysmap_rd78xx0_mASA.h"
#include "sysmap_rd78xx0_H3C.h"



static u32 mv_pci_mem_base[] = 
{
	PCI0_MEM0_BASE,
	PCI1_MEM0_BASE,
	PCI2_MEM0_BASE,
	PCI3_MEM0_BASE,
	PCI4_MEM0_BASE, 
	PCI5_MEM0_BASE, 
	PCI6_MEM0_BASE, 
	PCI7_MEM0_BASE, 
};

static u32 mv_pci_io_base[] = 
{
	PCI0_IO_BASE,
	PCI1_IO_BASE,
	PCI2_IO_BASE,
	PCI3_IO_BASE,
	PCI4_IO_BASE, 
	PCI5_IO_BASE, 
	PCI6_IO_BASE, 
	PCI7_IO_BASE, 
};


static MV_TARGET mv_pci_io_target[] = 
{
	PCI0_IO,
	PCI1_IO,
	PCI2_IO,
	PCI3_IO,
	PCI4_IO, 
	PCI5_IO, 
	PCI6_IO, 
	PCI7_IO, 
};

	

u32 mv_pci_mem_base_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
		return mv_pci_mem_base[ifNum];
	else
		panic("%s: error, PCI %d does not exist\n", __FUNCTION__, ifNum);
}

u32 mv_pci_mem_size_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
		return PCIx_MEM0_SIZE;
	else
		panic("%s: error, PCI %d does not exist\n", __FUNCTION__, ifNum);
	return 0;
}

u32 mv_pci_io_base_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
		return mv_pci_io_base[ifNum];
	else
		panic("%s: error, PCI %d does not exist\n", __FUNCTION__, ifNum);
}

u32 mv_pci_io_size_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
		return PCIx_IO_SIZE;
	else
		panic("%s: error, PCI %d does not exist\n", __FUNCTION__, ifNum);
	return 0;
}

MV_TARGET mv_pci_io_target_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
		return mv_pci_io_target[ifNum];
	else
		panic("%s: error, PCI %d does not exist\n", __FUNCTION__, ifNum);
	return 0;
}


int mv_is_pci_io_mapped(int ifNum)
{
	switch(mvBoardIdGet()) {
	case DB_78XX0_ID:
	case DB_78200_ID:
	case RD_78XX0_H3C_ID:
		if (ifNum < mvCtrlPciIfMaxIfGet())	
			return 1;
	default:
		return 0;
	}
}

u32 mv_pci_irqnum_get(int ifNum)
{
	if (ifNum < mvCtrlPciIfMaxIfGet())	
	{
		return IRQ_PEX_INT(ifNum);
	}
	return 0;
}


MV_CPU_DEC_WIN* mv_sys_map(void)
{
	switch(mvBoardIdGet()) {
	case DB_78XX0_ID:
		return SYSMAP_DB_78XX0;
	case DB_78200_ID:
		return SYSMAP_DB_78200;		
	case RD_78XX0_AMC_ID:
		return SYSMAP_RD_78XX0_AMC; 
	case RD_78XX0_MASA_ID:
                return SYSMAP_RD_78XX0_MASA;
	case RD_78XX0_H3C_ID:
                return SYSMAP_RD_78XX0_H3C;
	default:
		printk("ERROR: can't find system address map\n");
		return NULL;
        }
}

int mv_pci_if_num_to_skip(void)
{
	switch(mvBoardIdGet()) {
	case DB_78XX0_ID:
	case DB_78200_ID:
		return 2; 
	case RD_78XX0_H3C_ID:
		return 3;
	default:
		return -1;
	}
}


#if defined(CONFIG_MV_INCLUDE_CESA)
u32 mv_crypto_base_get(void)
{
	return CRYPTO_ENG_BASE;
}
EXPORT_SYMBOL(mv_crypto_base_get);
#endif

void __init mv_map_io(void)
{
        MV_U32 id = mvBoardIdGet();
	switch(id) {
	case DB_78XX0_ID:
		iotable_init(DB_78XX0_MEM_TABLE, ARRAY_SIZE(DB_78XX0_MEM_TABLE));
		break;
	case DB_78200_ID:
		iotable_init(DB_78200_MEM_TABLE, ARRAY_SIZE(DB_78200_MEM_TABLE));
		break;
	case RD_78XX0_AMC_ID:
		iotable_init(RD_78XX0_AMC_MEM_TABLE, ARRAY_SIZE(RD_78XX0_AMC_MEM_TABLE));
                break;
	case RD_78XX0_MASA_ID:
                iotable_init(RD_78XX0_MASA_MEM_TABLE, ARRAY_SIZE(RD_78XX0_MASA_MEM_TABLE));
                break;
	case RD_78XX0_H3C_ID:
                iotable_init(RD_78XX0_H3C_MEM_TABLE, ARRAY_SIZE(RD_78XX0_H3C_MEM_TABLE));                
                break;
	default:
		printk("ERROR: can't find system address map for board id 0x%X\n", id);
		BUG();
        }	
}


