

#include "mvSysHwConfig.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include <asm/mach/map.h>

/* for putstr */
/* #include <asm/arch/uncompress.h> */

MV_CPU_DEC_WIN* mv_sys_map(void);
u32 mv_pci_mem_size_get(int ifNum);
u32 mv_pci_io_base_get(int ifNum);
u32 mv_pci_io_size_get(int ifNum);
u32 mv_pci_mem_base_get(int ifNum);
#if defined(CONFIG_MV_INCLUDE_CESA)
u32 mv_crypo_base_get(void);
#endif
#if defined(CONFIG_MV_INCLUDE_DEVICE_CS1)
u32 mv_device_cs1_base_get(void);
#endif


#if defined(CONFIG_MV88F5182)
#include "sysmap_5182.h"
#elif defined(CONFIG_MV88F5181L)
#include "sysmap_5181L.h"
#elif defined(CONFIG_MV88W8660)
#include "sysmap_860.h"
#elif defined(CONFIG_MV88F5181)
#include "sysmap_5181.h"
#elif defined(CONFIG_MV88F5082)
#include "sysmap_5082.h"
#elif defined(CONFIG_MV88F6082)
#include "sysmap_6082.h"
#elif defined(CONFIG_MV88F6183)
#include "sysmap_6183.h"
#else
#error "unknown arch"
#endif

MV_CPU_DEC_WIN* mv_sys_map(void)
{
	char temp_str[80];
	sprintf(temp_str, "Board ID = %d\n", mvBoardIdGet());
	switch(mvBoardIdGet()) {
#if defined(CONFIG_MV88F5181L) 
		case DB_88F5181L_DDR2_2XTDM:
			return SYSMAP_DB_88F5181L_DDR2_2XTDM;
		case RD_88F5181L_VOIP_FE:
		case RD_88F5181L_VOIP_GE:
			return SYSMAP_RD2_5181L_FE_GE;
		case RD_88F5181L_VOIP_FXO_GE: 
			return SYSMAP_RD2_5181L_FXO;
#elif defined(CONFIG_MV88W8660)
		case RD_88W8660_DDR1:
			return SYSMAP_RD_88W8860_DDR1;
		case RD_88W8660_AP82S_DDR1:
			return SYSMAP_RD_88W8660_AP82S_DDR1;
		case DB_88W8660_DDR2: 
			return SYSMAP_DB_88W8660_DDR2; 
#elif defined(CONFIG_MV88F5181)
		case RD_88F5181_GTW_FE:
		case RD_88F5181_GTW_GE:
			return SYSMAP_RD_88F5181_GTW_FE_GE;
		case RD_88F5181_POS_NAS:
			return SYSMAP_RD_88F5181_88SX7042_2xSATA;
		case DB_88F5181_5281_DDR1:
			return SYSMAP_DB_88F5181_5281_DDR1_BP;
		case DB_88F5181_5281_DDR2:
			return SYSMAP_DB_88F5181_5281_DDR2_BP;
		case DB_88F5X81_DDR2:
		case DB_88F5X81_DDR1:
			return SYSMAP_DB_88F5x81_BP;
#elif defined(CONFIG_MV88F6082)
		case DB_88F6082_BP:
		case RD_88F6082_NAS:
		case DB_88F6082L_BP:
		case RD_88F6082_MICRO_DAS_NAS:
			return SYSMAP_DB_88F6082_BP;
#elif defined(CONFIG_MV88F5182)
		case RD_88F5182_2XSATA3:
		case RD_88F5182_2XSATA: 
			return SYSMAP_RD_88F5182;
		case DB_88F5182_DDR2: 
		case DB_88F5182_DDR2_A:
			return SYSMAP_DB_88F5182_DDR2_BP;
#elif defined(CONFIG_MV88F5082)
		case DB_88F5082_DDR2:
			return SYSMAP_DB_88F5082_BP;
		case RD_88F5082_2XSATA:
		case RD_88F5082_2XSATA3:
			return SYSMAP_RD_88F5082;

#elif defined(CONFIG_MV88F6183)
	case DB_88F6183_BP:
    case RD_88F6183_AP:
		return SYSMAP_DB_88F6183_BP;
	case RD_88F6183_GP:
		return SYSMAP_RD_88F6183_GP;

#endif
		default:
			printk("ERROR: can't find system address map\n");
				return NULL;
        }
}

u32 mv_pci_mem_base_get(int ifNum)
{
	if(ifNum == 0)
		return PCI_IF0_MEM0_BASE;
#if defined (CONFIG_MV_INCLUDE_PCI) && defined (CONFIG_MV_INCLUDE_PEX)
	else
		 return PCI_IF1_MEM0_BASE;
#else
		printk("%s: error, IF1 not exists\n", __FUNCTION__);
		return -1;
#endif
}

u32 mv_pci_mem_size_get(int ifNum)
{
	if(ifNum == 0)
		return PCI_IF0_MEM0_SIZE;
#if defined (CONFIG_MV_INCLUDE_PCI) && defined (CONFIG_MV_INCLUDE_PEX)
	else
		 return PCI_IF1_MEM0_SIZE;
#else
		printk("%s: error, IF1 not exists\n", __FUNCTION__);
		return -1;
#endif
}
u32 mv_pci_io_base_get(int ifNum)
{
	if(ifNum == 0)
		return PCI_IF0_IO_BASE;
#if defined (CONFIG_MV_INCLUDE_PCI) && defined (CONFIG_MV_INCLUDE_PEX)
	else
		 return PCI_IF1_IO_BASE;
#else
		printk("%s: error, IF1 not exists\n", __FUNCTION__);
		return -1;
#endif
}

u32 mv_pci_io_size_get(int ifNum)
{
	if(ifNum == 0)
		return PCI_IF0_IO_SIZE;
#if defined (CONFIG_MV_INCLUDE_PCI) && defined (CONFIG_MV_INCLUDE_PEX)
	else
		 return PCI_IF1_IO_SIZE;
#else
		printk("%s: error, IF1 not exists\n", __FUNCTION__);
		return -1;
#endif

}

#if defined(CONFIG_MV_INCLUDE_CESA)
u32 mv_crypo_base_get(void)
{
	return CRYPT_ENG_BASE;
}
#endif

void __init mv_map_io(void)
{
        iotable_init(MEM_TABLE, ARRAY_SIZE(MEM_TABLE));
}

#if defined(CONFIG_MV_INCLUDE_DEVICE_CS1)
u32 mv_device_cs1_base_get(void)
{
	return DEVICE_CS1_BASE;
}
#endif

