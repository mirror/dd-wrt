MV_CPU_DEC_WIN SYSMAP_RD_78XX0_MASA[] = {

/*     	base low      base high      size        	WinNum		enable/disable */
	{{SDRAM_CS0_BASE ,    0,      SDRAM_CS0_SIZE }, 0xFFFFFFFF,	EN },/*  0 */
	{{SDRAM_CS1_BASE ,    0,      SDRAM_CS1_SIZE }, 0xFFFFFFFF,	EN },/*  1 */
	{{SDRAM_CS2_BASE ,    0,      SDRAM_CS2_SIZE }, 0xFFFFFFFF,	EN },/*  2 */
	{{SDRAM_CS3_BASE ,    0,      SDRAM_CS3_SIZE }, 0xFFFFFFFF,	EN },/*  3 */
	{{DEVICE_CS0_BASE,    0,      DEVICE_CS0_SIZE}, 10,		EN },/*  4 */
	{{DEVICE_CS1_BASE,    0,      DEVICE_CS1_SIZE}, 11,		EN },/*  5 */
	{{DEVICE_CS2_BASE,    0,      DEVICE_CS2_SIZE}, 12,		EN },/*  6 */
	{{DEVICE_CS3_BASE,    0,      DEVICE_CS3_SIZE}, 9,		EN },/*  7 */
	{{BOOTDEV_CS_BASE,    0,      BOOTDEV_CS_SIZE}, 13,		EN },/*  8 */
#if !defined (CONFIG_MV78XX0_Z0)
	{{SPI_CS_BASE,        0,      SPI_CS_SIZE}, 	13,		DIS},/*  9 */
#endif
	{{PCI0_IO_BASE   ,    0,      PCI0_IO_SIZE   }, 0,		DIS},/*  10 */
	{{PCI0_MEM0_BASE ,    0,      PCI0_MEM0_SIZE }, 0,		EN },/* 11 */
	{{PCI1_IO_BASE   ,    0,      PCI1_IO_SIZE   }, 1,		DIS},/* 12 */
	{{PCI1_MEM0_BASE ,    0,      PCI1_MEM0_SIZE }, 1,		EN },/* 13 */
	{{PCI2_IO_BASE   ,    0,      PCI2_IO_SIZE   }, 2,		DIS},/* 14 */
	{{PCI2_MEM0_BASE ,    0,      PCI2_MEM0_SIZE }, 2,		EN },/* 15 */
	{{PCI3_IO_BASE   ,    0,      PCI3_IO_SIZE   }, 3,		DIS},/* 16 */
	{{PCI3_MEM0_BASE ,    0,      PCI3_MEM0_SIZE }, 3,		EN },/* 17 */
	{{PCI4_IO_BASE   ,    0,      PCI4_IO_SIZE   }, 4,		DIS},/* 18 */
	{{PCI4_MEM0_BASE ,    0,      PCI4_MEM0_SIZE }, 4,		EN },/* 19 */
	{{PCI5_IO_BASE   ,    0,      PCI5_IO_SIZE   }, 5,		DIS},/* 20 */
	{{PCI5_MEM0_BASE ,    0,      PCI5_MEM0_SIZE }, 5,		EN },/* 21 */
	{{PCI6_IO_BASE   ,    0,      PCI6_IO_SIZE   }, 6,		DIS},/* 22 */
	{{PCI6_MEM0_BASE ,    0,      PCI6_MEM0_SIZE }, 6,		EN },/* 23 */
	{{PCI7_IO_BASE   ,    0,      PCI7_IO_SIZE   }, 7,		DIS},/* 24 */
	{{PCI7_MEM0_BASE ,    0,      PCI7_MEM0_SIZE }, 7,		EN },/* 25 */
	{{CRYPTO_ENG_BASE ,    0,      CRYPTO_SIZE    }, 8,	EN},/* 26 */
	{{INTER_REGS_BASE,    0,      INTER_REGS_SIZE}, 14,		EN },/* 27 */
    	/* Table terminator */
   	{{TBL_TERM, TBL_TERM, TBL_TERM}, TBL_TERM, TBL_TERM}
};

