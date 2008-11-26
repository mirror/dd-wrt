/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "mvSysHwConfig.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include <asm/mach/map.h>

/* for putstr */
/* #include <asm/arch/uncompress.h> */

MV_CPU_DEC_WIN* mv_sys_map(void);

#if defined(CONFIG_MV_INCLUDE_CESA)
u32 mv_crypo_base_get(void);
#endif

struct map_desc  MEM_TABLE[] =	{
	/* no use for pex mem remap */	
  /*{ PEX0_MEM_BASE,  		__phys_to_pfn(PEX0_MEM_BASE),   	PEX0_MEM_SIZE,  	MT_DEVICE},*/
  { INTER_REGS_BASE, 		__phys_to_pfn(INTER_REGS_BASE), 	SZ_1M,  	     	MT_DEVICE},
  { PEX0_IO_BASE,   		__phys_to_pfn(PEX0_IO_BASE),   	 	PEX0_IO_SIZE,  		MT_DEVICE},
  { NFLASH_CS_BASE, 		__phys_to_pfn(NFLASH_CS_BASE), 		NFLASH_CS_SIZE, 	MT_DEVICE},
  { SPI_CS_BASE, 		__phys_to_pfn(SPI_CS_BASE), 		SPI_CS_SIZE, 		MT_DEVICE},
  { CRYPT_ENG_BASE, 		__phys_to_pfn(CRYPT_ENG_BASE), 		CRYPT_ENG_SIZE, 	MT_DEVICE}
};

MV_CPU_DEC_WIN SYSMAP_88F6281[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x1       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{NFLASH_CS_BASE,     0,      NFLASH_CS_SIZE}  ,0x2	  ,EN}, 
	{{SPI_CS_BASE,        0,      SPI_CS_SIZE    } ,0x5       ,EN},
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 0x6	  ,DIS},
 	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 0x4	  ,DIS},
	{{CRYPT_ENG_BASE,     0,      CRYPT_ENG_SIZE}  ,0x7  	  ,EN},
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_88F6180[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x1       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{NFLASH_CS_BASE,     0,      NFLASH_CS_SIZE } ,0x2	  ,EN}, 
	{{SPI_CS_BASE,        0,      SPI_CS_SIZE}     ,0x5       ,EN}, 
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 0x6	  ,DIS},
 	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 0x4	  ,DIS},
	{{CRYPT_ENG_BASE,     0,      CRYPT_ENG_SIZE}  ,0x7  	  ,EN},
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_88F6192[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x1       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{NFLASH_CS_BASE,     0,      NFLASH_CS_SIZE}  ,0x2	  ,EN}, 
	{{SPI_CS_BASE,        0,      SPI_CS_SIZE}     ,0x5       ,EN},
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 0x6	  ,DIS},
 	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 0x4	  ,DIS},
	{{CRYPT_ENG_BASE,     0,      CRYPT_ENG_SIZE}  ,0x7  	  ,EN},
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};



MV_CPU_DEC_WIN* mv_sys_map(void)
{
	switch(mvBoardIdGet()) {
	case DB_88F6281_BP_ID:
	case RD_88F6281_ID: 
		return SYSMAP_88F6281;
	case DB_88F6192_BP_ID:
	case RD_88F6192_ID:
		return SYSMAP_88F6192;
	case RD_88F6180_ID:
	case DB_88F6180_BP_ID:
		return SYSMAP_88F6180;
	default:
		printk("ERROR: can't find system address map\n");
		return NULL;
        }
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
