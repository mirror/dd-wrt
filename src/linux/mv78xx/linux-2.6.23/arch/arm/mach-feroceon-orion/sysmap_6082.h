
/* 
 *  System memory mapping 
 */

/* SDRAM: actual mapping is auto detected */
#define SDRAM_CS0_BASE  0x00000000
#define SDRAM_CS0_SIZE  _16M

#define SDRAM_CS1_BASE  0x01000000
#define SDRAM_CS1_SIZE  _16M

#define SDRAM_CS2_BASE  0x20000000
#define SDRAM_CS2_SIZE  _256M

#define SDRAM_CS3_BASE  0x30000000
#define SDRAM_CS3_SIZE  _256M

/* PEX */
#define PEX0_MEM_BASE 0xe0000000
#define PEX0_MEM_SIZE _128M

#define PEX0_IO_BASE 0xf2000000
#define PEX0_IO_SIZE _1M

/* Device: CS0 - N_FLASH, CS1 - M_FLASH, CS2 - S_FLASH, CS3 - BOOTROM, CS4 - DEV_BOOCS */
#define DEVICE_CS0_BASE 0xf9000000
#define DEVICE_CS0_SIZE _2M

#define DEVICE_CS1_BASE 0xf8000000
#define DEVICE_CS1_SIZE _256K

#define DEVICE_CS2_BASE 0xfa000000
#define DEVICE_CS2_SIZE _8M

#define DEVICE_CS3_BASE 0xfb000000
#define DEVICE_CS3_SIZE _1M

#define DEVICE_CS4_BASE 0xfc000000
#define DEVICE_CS4_SIZE _1M


#define CRYPT_ENG_BASE	0xF0000000
#define CRYPT_ENG_SIZE	_64K

/* Tailgate has no PCI interface - only PEX */
#if defined (CONFIG_MV_INCLUDE_PEX)

#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#endif

struct map_desc MEM_TABLE[] = 	{	
 { PCI_IF0_MEM0_BASE,   __phys_to_pfn(PCI_IF0_MEM0_BASE),   PCI_IF0_MEM0_SIZE,  MT_DEVICE },	
 { CRYPT_ENG_BASE, __phys_to_pfn(CRYPT_ENG_BASE), CRYPT_ENG_SIZE, MT_DEVICE},					
 { INTER_REGS_BASE, __phys_to_pfn(INTER_REGS_BASE), SZ_1M,  	     MT_DEVICE },				
 { PCI_IF0_IO_BASE,   __phys_to_pfn(PCI_IF0_IO_BASE),   PCI_IF0_IO_SIZE,  MT_DEVICE },			
 { DEVICE_CS1_BASE, __phys_to_pfn(DEVICE_CS1_BASE), DEVICE_CS1_SIZE, MT_DEVICE},			
 { DEVICE_CS0_BASE, __phys_to_pfn(DEVICE_CS0_BASE), DEVICE_CS0_SIZE, MT_DEVICE},				
 { DEVICE_CS2_BASE, __phys_to_pfn(DEVICE_CS2_BASE), DEVICE_CS2_SIZE, MT_DEVICE},				
 { DEVICE_CS3_BASE, __phys_to_pfn(DEVICE_CS2_BASE), DEVICE_CS2_SIZE, MT_DEVICE},				
 { DEVICE_CS4_BASE, __phys_to_pfn(DEVICE_CS2_BASE), DEVICE_CS2_SIZE, MT_DEVICE}				
};

MV_CPU_DEC_WIN SYSMAP_DB_88F6082_BP[] = {
   	/* base low        base high    size       			WinNum     enable */
	{{SDRAM_CS0_BASE ,    0, SDRAM_CS0_SIZE } ,			0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0, SDRAM_CS1_SIZE } ,			0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0, PEX0_MEM_SIZE  } ,			0x1       ,EN},
	{{PEX0_IO_BASE   ,    0, PEX0_IO_SIZE   } ,			0x0       ,EN},
	{{INTER_REGS_BASE,    0, INTER_REGS_SIZE} ,			0x2       ,EN},
	{{DEVICE_CS0_BASE,    0, DEVICE_CS0_SIZE} ,			0x3	  ,EN},
	{{DEVICE_CS1_BASE,    0, DEVICE_CS1_SIZE} ,			0x4	  ,EN},
	{{DEVICE_CS2_BASE,    0, DEVICE_CS2_SIZE} ,			0x5	  ,EN},
	{{DEVICE_CS3_BASE,    0, DEVICE_CS3_SIZE} ,			0x6	  ,DIS},
	{{DEVICE_CS4_BASE,    0, DEVICE_CS4_SIZE} ,	                0x6       ,EN},
	{{CRYPT_ENG_BASE,     0, CRYPT_ENG_SIZE} ,			0x7  	  ,EN},
   	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};


