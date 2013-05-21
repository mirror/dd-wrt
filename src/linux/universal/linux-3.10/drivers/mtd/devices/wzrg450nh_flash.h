#ifndef _AR7240_SPI_H
#define _AR7240_SPI_H

#define	CONFIG_BUFFALO

#if 0
#define AR7240_SPI_FS           0x1f000000
#define AR7240_SPI_CLOCK        0x1f000004
#define AR7240_SPI_WRITE        0x1f000008
#define AR7240_SPI_READ         0x1f000000
#define AR7240_SPI_RD_STATUS    0x1f00000c

#define AR7240_SPI_CS_DIS       0x70000
#endif
#define AR7240_SPI_CE_LOW       0x60000
#define AR7240_SPI_CE_HIGH      0x60100
#ifdef	CONFIG_BUFFALO
#undef	AR7240_SPI_CE_LOW
#undef	AR7240_SPI_CE_HIGH
#define AR7240_SPI_CE_LOW0		0x60000
#define AR7240_SPI_CE_LOW1		0x50000
#define AR7240_SPI_CE_HIGH0		0x60100
#define AR7240_SPI_CE_HIGH1		0x50100
#endif	//CONFIG_BUFFALO

#if 0
#define AR7240_SPI_CMD_WREN         0x06
#define AR7240_SPI_CMD_RD_STATUS    0x05
#define AR7240_SPI_CMD_FAST_READ    0x0b
#define AR7240_SPI_CMD_PAGE_PROG    0x02
#define AR7240_SPI_CMD_SECTOR_ERASE 0xd8
#endif
#ifdef	CONFIG_BUFFALO
#define AR7240_SPI_CMD_READ_ID		0x9F
#define AR7240_SPI_CMD_READ_MANID	0x90
#define AR7240_SPI_CMD_READ_DMC		0x5a
#endif	//CONFIG_BUFFALO

#ifdef ST25P28
#define AR7240_SPI_SECTOR_SIZE      (1024*256)
#define AR7240_SPI_PAGE_SIZE        64
#else
#define AR7240_SPI_SECTOR_SIZE      (1024*64)
#define AR7240_SPI_PAGE_SIZE        256
#endif

#ifdef	CONFIG_BUFFALO
#undef	AR7240_FLASH_MAX_BANKS
#define AR7240_FLASH_MAX_BANKS  2
#ifdef	ST25P28
#error
#endif
#else	//CONFIG_BUFFALO
#define AR7240_FLASH_MAX_BANKS  1
#endif	//CONFIG_BUFFALO

#define display(_x)     ar7240_reg_wr_nf(0x18040008, (_x))

/*
 * primitives
 */

#define ar7240_be_msb(_val, __i) (((_val) & (1 << (7 - __i))) >> (7 - __i))

#ifdef	CONFIG_BUFFALO
#define ar7240_spi_bit_banger(_cs,_byte)  do {									\
	if (_cs) {																	\
	    int _i;																	\
	    for(_i = 0; _i < 8; _i++) {												\
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE,									\
	                        AR7240_SPI_CE_LOW1 | ar7240_be_msb(_byte, _i));		\
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE,									\
	                        AR7240_SPI_CE_HIGH1 | ar7240_be_msb(_byte, _i));	\
	    }																		\
	} else {																	\
	    int _i;																	\
	    for(_i = 0; _i < 8; _i++) {												\
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE,									\
	                        AR7240_SPI_CE_LOW0 | ar7240_be_msb(_byte, _i));		\
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE,									\
	                        AR7240_SPI_CE_HIGH0 | ar7240_be_msb(_byte, _i));	\
	    }																		\
	}																			\
}while(0);

#define ar7240_spi_bit_banger_0(_byte)  do {        \
    int _i;                                      \
    for(_i = 0; _i < 8; _i++) {                    \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_LOW0 | ar7240_be_msb(_byte, _i));  \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_HIGH0 | ar7240_be_msb(_byte, _i)); \
    }       \
}while(0);

#define ar7240_spi_bit_banger_1(_byte)  do {        \
    int _i;                                      \
    for(_i = 0; _i < 8; _i++) {                    \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_LOW1 | ar7240_be_msb(_byte, _i));  \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_HIGH1 | ar7240_be_msb(_byte, _i)); \
    }       \
}while(0);

#else	//CONFIG_BUFFALO
#define ar7240_spi_bit_banger(_byte)  do {        \
    int _i;                                      \
    for(_i = 0; _i < 8; _i++) {                    \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_LOW | ar7240_be_msb(_byte, _i));  \
        ar7240_reg_wr_nf(AR7240_SPI_WRITE,      \
                        AR7240_SPI_CE_HIGH | ar7240_be_msb(_byte, _i)); \
    }       \
}while(0);
#endif	//CONFIG_BUFFALO

#ifdef	CONFIG_BUFFALO
#define ar7240_spi_go(_cs) do {										\
	if (_cs) {														\
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW1);		\
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);		\
	} else {														\
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW0);		\
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);		\
	}																\
}while(0);
#else	//CONFIG_BUFFALO
#define ar7240_spi_go() do {        \
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CE_LOW); \
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS); \
}while(0);
#endif	//CONFIG_BUFFALO


#ifdef	CONFIG_BUFFALO
#define ar7240_spi_send_addr(_cs,_addr) do {				\
	if (_cs) {												\
		uint32_t	__adr	= (_addr) & 0xFFFFFF;			\
		/*printk("ar7240_spi_send_addr: cs=0x%x addr=0x%x\n", (unsigned)_cs, (unsigned)__adr);*/	\
		ar7240_spi_bit_banger_1(((__adr & 0xff0000) >> 16));\
		ar7240_spi_bit_banger_1(((__adr & 0x00ff00) >> 8));	\
		ar7240_spi_bit_banger_1(__adr & 0x0000ff);			\
	} else {												\
		ar7240_spi_bit_banger_0(((_addr & 0xff0000) >> 16));\
		ar7240_spi_bit_banger_0(((_addr & 0x00ff00) >> 8));	\
		ar7240_spi_bit_banger_0(_addr & 0x0000ff);			\
	}														\
}while(0);
#else	//CONFIG_BUFFALO
#define ar7240_spi_send_addr(_addr) do {                    \
    ar7240_spi_bit_banger(((addr & 0xff0000) >> 16));                 \
    ar7240_spi_bit_banger(((addr & 0x00ff00) >> 8));                 \
    ar7240_spi_bit_banger(addr & 0x0000ff);                 \
}while(0);
#endif	//CONFIG_BUFFALO

#ifdef	CONFIG_BUFFALO
#define ar7240_spi_delay_8(_cs)    ar7240_spi_bit_banger(_cs,0)
#else	//CONFIG_BUFFALO
#define ar7240_spi_delay_8()    ar7240_spi_bit_banger(0)
#endif	//CONFIG_BUFFALO

#ifdef	CONFIG_BUFFALO
#define ar7240_spi_done()	do	{									\
								ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);	\
 								ar7240_reg_wr(AR7240_SPI_FS, 0);	\
							} while(0)
#else	//CONFIG_BUFFALO
#define ar7240_spi_done()       ar7240_reg_wr(AR7240_SPI_FS, 0)
#endif	//CONFIG_BUFFALO

#endif /*_AR7240_SPI_H*/
