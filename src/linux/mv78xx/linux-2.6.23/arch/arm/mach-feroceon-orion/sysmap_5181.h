
/* 
 *  System memory mapping 
 */

/* SDRAM: actual mapping is auto detected */
#define SDRAM_CS0_BASE  0x00000000
#define SDRAM_CS0_SIZE  _256M

#define SDRAM_CS1_BASE  0x10000000
#define SDRAM_CS1_SIZE  _256M

#define SDRAM_CS2_BASE  0x20000000
#define SDRAM_CS2_SIZE  _256M

#define SDRAM_CS3_BASE  0x30000000
#define SDRAM_CS3_SIZE  _256M

/* PEX */
#define PEX0_MEM_BASE 0xe0000000
#define PEX0_MEM_SIZE _128M

#define PEX0_IO_BASE 0xf2000000
#define PEX0_IO_SIZE _1M

/* PCI0: IO and memory space */
#define PCI0_MEM_BASE  0xe8000000
#define PCI0_MEM_SIZE  _128M

#define PCI0_IO_BASE    0xf2100000
#define PCI0_IO_SIZE    _1M

/* Chip select base addresses and sizes */
#define DEVICE_CS0_BASE 0xfa000000
#define DEVICE_CS0_SIZE _2M

#define DEVICE_CS1_BASE 0xfc000000
#define DEVICE_CS1_SIZE _32M

#define DEVICE_CS2_BASE 0xfa800000
#define DEVICE_CS2_SIZE _1M

#define BOOTDEV_CS_BASE	0xF4000000
#define BOOTDEV_CS_SIZE _16M

#if defined (CONFIG_MV_INCLUDE_PCI) && defined (CONFIG_MV_INCLUDE_PEX)

#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#define PCI_IF1_MEM0_BASE 	PCI0_MEM_BASE
#define PCI_IF1_MEM0_SIZE 	PCI0_MEM_SIZE
#define PCI_IF1_IO_BASE 	PCI0_IO_BASE
#define PCI_IF1_IO_SIZE 	PCI0_IO_SIZE

#elif defined (CONFIG_MV_INCLUDE_PCI)

#define PCI_IF0_MEM0_BASE 	PCI0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PCI0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PCI0_IO_BASE
#define PCI_IF0_IO_SIZE 	PCI0_IO_SIZE

#elif defined (CONFIG_MV_INCLUDE_PEX)

#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#endif

struct map_desc MEM_TABLE[] = 	{	
  { PCI_IF0_MEM0_BASE,  __phys_to_pfn(PCI_IF0_MEM0_BASE),   	PCI_IF0_MEM0_SIZE,  	MT_DEVICE},
  { PCI_IF1_MEM0_BASE,  __phys_to_pfn(PCI_IF1_MEM0_BASE),   	PCI_IF1_MEM0_SIZE, 	MT_DEVICE},
  { INTER_REGS_BASE, 	__phys_to_pfn(INTER_REGS_BASE), 	SZ_1M,  	     	MT_DEVICE},
  { PCI_IF0_IO_BASE,   	__phys_to_pfn(PCI_IF0_IO_BASE),   	PCI_IF0_IO_SIZE, 	MT_DEVICE},
  { PCI_IF1_IO_BASE,   	__phys_to_pfn(PCI_IF1_IO_BASE),   	PCI_IF1_IO_SIZE,  	MT_DEVICE},  
  { DEVICE_CS0_BASE, 	__phys_to_pfn(DEVICE_CS0_BASE), 	DEVICE_CS0_SIZE, 	MT_DEVICE},
  { DEVICE_CS2_BASE, 	__phys_to_pfn(DEVICE_CS2_BASE), 	DEVICE_CS2_SIZE, 	MT_DEVICE}
};


MV_CPU_DEC_WIN SYSMAP_RD_88F5181_GTW_FE_GE[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x2       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{PCI0_MEM_BASE  ,    0,      PCI0_MEM_SIZE  } ,0x3       ,EN},
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   } ,0x1       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE} ,0x5	  ,DIS},
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE} ,0x6	  ,DIS},
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE} ,0x7	  ,DIS},
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE} ,0x4       ,EN},
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_RD_88F5181_88SX7042_2xSATA[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x2       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{PCI0_MEM_BASE  ,    0,      PCI0_MEM_SIZE  } ,0x3       ,EN},
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   } ,0x1       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE} ,0x5	  ,DIS}, 
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE} ,0x6	  ,EN},	/* Big NOR Flash */
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE} ,0x7	  ,DIS},
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE} ,0x4       ,EN},	/* Boot Flash */
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_DB_88F5181_5281_DDR1_BP[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x2       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{PCI0_MEM_BASE  ,    0,      PCI0_MEM_SIZE  } ,0x3       ,EN},
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   } ,0x1       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE} ,0x5	  ,DIS},
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE} ,0x6	  ,EN},	/* Big NOR Flash */
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE} ,0x7	  ,EN}, /* Seven Segment */
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE} ,0x4       ,EN}, /* Boot Flash */
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_DB_88F5181_5281_DDR2_BP[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x2       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{PCI0_MEM_BASE  ,    0,      PCI0_MEM_SIZE  } ,0x3       ,EN},
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   } ,0x1       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE} ,0x5	  ,DIS},	
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE} ,0x6	  ,EN},	/* Big NOR Flash */		
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE} ,0x7	  ,DIS},
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE} ,0x4       ,EN},	/* Boot Flash */
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

MV_CPU_DEC_WIN SYSMAP_DB_88F5x81_BP[] = {
  	 /* base low        base high    size       	WinNum     enable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE } ,0xFFFFFFFF,DIS},
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE } ,0xFFFFFFFF,DIS},
	{{PEX0_MEM_BASE  ,    0,      PEX0_MEM_SIZE  } ,0x2       ,EN},
	{{PEX0_IO_BASE   ,    0,      PEX0_IO_SIZE   } ,0x0       ,EN},
	{{PCI0_MEM_BASE  ,    0,      PCI0_MEM_SIZE  } ,0x3       ,EN},
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   } ,0x1       ,EN},
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE} ,0x8       ,EN},
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE} ,0x5	  ,EN},
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE} ,0x6	  ,EN},
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE} ,0x7	  ,EN},
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE} ,0x4       ,EN},
	{{TBL_TERM,TBL_TERM, TBL_TERM} ,TBL_TERM  ,TBL_TERM}		
};

