/*
 * ar5315.c - AR2315/AR2316/AR2317/AR2318 specific system functions 
 *
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 */
#include "mips32.c"
#include "spiflash.h"

/* definition of basic flash mappings */
static unsigned int sectorsize = 0x10000;
static unsigned int linuxaddr = 0xbf010000;
static unsigned int flashbase = 0xbf000000;
static unsigned int flashsize = 0x800000;

#define AR7100_PCI_MEM_BASE             0xB1000000	/* 128M */
#define AR7100_APB_BASE                 0xB1800000	/* 384M */
#define AR7100_GE0_BASE                 0xB1900000	/* 16M */
#define AR7100_GE1_BASE                 0xB1a00000	/* 16M */
#define AR7100_USB_EHCI_BASE            0xB1b00000
#define AR7100_USB_OHCI_BASE            0xB1c00000
#define AR7100_SPI_BASE                 0xB1f00000

#define AR7100_DDR_CTL_BASE             AR7100_APB_BASE+0x00000000
#define AR7100_CPU_BASE                 AR7100_APB_BASE+0x00010000
#define AR7100_UART_BASE                AR7100_APB_BASE+0x00020000
#define AR7100_USB_CONFIG_BASE          AR7100_APB_BASE+0x00030000
#define AR7100_GPIO_BASE                AR7100_APB_BASE+0x00040000
#define AR7100_PLL_BASE                 AR7100_APB_BASE+0x00050000
#define AR7100_RESET_BASE               AR7100_APB_BASE+0x00060000

#define AR7100_GPIO_OE                  AR7100_GPIO_BASE+0x0
#define AR7100_GPIO_IN                  AR7100_GPIO_BASE+0x4
#define AR7100_GPIO_OUT                 AR7100_GPIO_BASE+0x8
#define AR7100_GPIO_SET                 AR7100_GPIO_BASE+0xc
#define AR7100_GPIO_CLEAR               AR7100_GPIO_BASE+0x10
#define AR7100_GPIO_INT_ENABLE          AR7100_GPIO_BASE+0x14
#define AR7100_GPIO_INT_TYPE            AR7100_GPIO_BASE+0x18
#define AR7100_GPIO_INT_POLARITY        AR7100_GPIO_BASE+0x1c
#define AR7100_GPIO_INT_PENDING         AR7100_GPIO_BASE+0x20
#define AR7100_GPIO_INT_MASK            AR7100_GPIO_BASE+0x24

#define AR7100_RESET_GE1_MAC                (1 << 13)
#define AR7100_RESET_GE1_PHY                (1 << 12)
#define AR7100_RESET_GE0_MAC                (1 << 9)
#define AR7100_RESET_GE0_PHY                (1 << 8)

#define AR7100_RESET                  AR7100_RESET_BASE+0x24

#define disable_watchdog() \
{ 					\
}					\

static int getGPIO(int nr)
{
	volatile unsigned int *gpio = (unsigned int *)AR7100_GPIO_IN;
	if ((*gpio & 1 << nr) == (1 << nr))
		return 1;
	return 0;
}

#define ag7100_reset_mask(_no) (_no) ? (AR7100_RESET_GE1_MAC |  \
                                        AR7100_RESET_GE1_PHY)   \
                                     : (AR7100_RESET_GE0_MAC |  \
                                        AR7100_RESET_GE0_PHY)

#define ar7100_reg_rmw_set(_reg, _mask)  do {                        \
    ar7100_reg_wr((_reg), (ar7100_reg_rd((_reg)) | (_mask)));      \
    ar7100_reg_rd((_reg));                                           \
}while(0);

#define ar7100_reg_rmw_clear(_reg, _mask)  do {                        \
    ar7100_reg_wr((_reg), (ar7100_reg_rd((_reg)) & ~(_mask)));      \
    ar7100_reg_rd((_reg));                                           \
}while(0);

#define ar7100_reg_rd(_phys)    (*(volatile unsigned int *)_phys)
#define ar7100_reg_wr_nf(_phys, _val) \
                    ((*(volatile unsigned int *)KSEG1ADDR(_phys)) = (_val))

#define ar7100_reg_wr(_phys, _val) do {     \
                    ar7100_reg_wr_nf(_phys, _val);  \
                    ar7100_reg_rd(_phys);       \
}while(0);

static void enable_ethernet(void)
{
	unsigned int mask = ag7100_reset_mask(0);
	/*
	 * put into reset, hold, pull out.
	 */
	ar7100_reg_rmw_set(AR7100_RESET, mask);
	udelay(1000 * 1000);
	ar7100_reg_rmw_clear(AR7100_RESET, mask);
	udelay(1000 * 1000);
	udelay(20);
}

#define FLASH_1MB  1
#define FLASH_2MB  2
#define FLASH_4MB  3
#define FLASH_8MB  4
#define FLASH_16MB 5
#define MAX_FLASH  6

#define STM_PAGE_SIZE           256

#define SFI_WRITE_BUFFER_SIZE   4
#define SFI_FLASH_ADDR_MASK     0x00ffffff

#define STM_8MBIT_SIGNATURE     0x13
#define STM_M25P80_BYTE_COUNT   1048576
#define STM_M25P80_SECTOR_COUNT 16
#define STM_M25P80_SECTOR_SIZE  0x10000

#define STM_16MBIT_SIGNATURE    0x14
#define STM_M25P16_BYTE_COUNT   2097152
#define STM_M25P16_SECTOR_COUNT 32
#define STM_M25P16_SECTOR_SIZE  0x10000

#define STM_32MBIT_SIGNATURE    0x15
#define STM_M25P32_BYTE_COUNT   4194304
#define STM_M25P32_SECTOR_COUNT 64
#define STM_M25P32_SECTOR_SIZE  0x10000

#define STM_64MBIT_SIGNATURE    0x16
#define STM_M25P64_BYTE_COUNT   8388608
#define STM_M25P64_SECTOR_COUNT 128
#define STM_M25P64_SECTOR_SIZE  0x10000

#define STM_128MBIT_SIGNATURE   0x17
#define STM_M25P128_BYTE_COUNT   16777216
#define STM_M25P128_SECTOR_COUNT 256
#define STM_M25P128_SECTOR_SIZE  0x10000

#define STM_1MB_BYTE_COUNT   STM_M25P80_BYTE_COUNT
#define STM_1MB_SECTOR_COUNT STM_M25P80_SECTOR_COUNT
#define STM_1MB_SECTOR_SIZE  STM_M25P80_SECTOR_SIZE
#define STM_2MB_BYTE_COUNT   STM_M25P16_BYTE_COUNT
#define STM_2MB_SECTOR_COUNT STM_M25P16_SECTOR_COUNT
#define STM_2MB_SECTOR_SIZE  STM_M25P16_SECTOR_SIZE
#define STM_4MB_BYTE_COUNT   STM_M25P32_BYTE_COUNT
#define STM_4MB_SECTOR_COUNT STM_M25P32_SECTOR_COUNT
#define STM_4MB_SECTOR_SIZE  STM_M25P32_SECTOR_SIZE
#define STM_8MB_BYTE_COUNT   STM_M25P64_BYTE_COUNT
#define STM_8MB_SECTOR_COUNT STM_M25P64_SECTOR_COUNT
#define STM_8MB_SECTOR_SIZE  STM_M25P64_SECTOR_SIZE
#define STM_16MB_BYTE_COUNT   STM_M25P128_BYTE_COUNT
#define STM_16MB_SECTOR_COUNT STM_M25P128_SECTOR_COUNT
#define STM_16MB_SECTOR_SIZE  STM_M25P128_SECTOR_SIZE

#define AR7100_SPI_FS           AR7100_SPI_BASE+0x0
#define AR7100_SPI_CLOCK        AR7100_SPI_BASE+0x4
#define AR7100_SPI_WRITE        AR7100_SPI_BASE+0x8
#define AR7100_SPI_READ         AR7100_SPI_BASE+0x0
#define AR7100_SPI_RD_STATUS    AR7100_SPI_BASE+0xc

#define AR7100_SPI_CS_DIS       0x70000
#define AR7100_SPI_CE_LOW       0x60000
#define AR7100_SPI_CE_HIGH      0x60100

#define AR7100_SPI_CMD_WREN         0x06
#define AR7100_SPI_CMD_RD_STATUS    0x05
#define AR7100_SPI_CMD_FAST_READ    0x0b
#define AR7100_SPI_CMD_PAGE_PROG    0x02
#define AR7100_SPI_CMD_SECTOR_ERASE 0xd8

#define AR7100_SPI_SECTOR_SIZE      (1024*64)
#define AR7100_SPI_PAGE_SIZE        256

#define display(_x)     ar7100_reg_wr_nf(0x18040008, (_x))

/*
 * primitives
 */

#define ar7100_be_msb(_val, _i) (((_val) & (1 << (7 - _i))) >> (7 - _i))

static void ar7100_spi_bit_banger(unsigned char _byte)
{
	do {
		int i;
		for (i = 0; i < 8; i++) {
			ar7100_reg_wr_nf(AR7100_SPI_WRITE,
					 AR7100_SPI_CE_LOW |
					 ar7100_be_msb(_byte, i));
			ar7100_reg_wr_nf(AR7100_SPI_WRITE,
					 AR7100_SPI_CE_HIGH |
					 ar7100_be_msb(_byte, i));
		}
	} while (0);
}

static void ar7100_spi_go(void)
{
	do {
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CE_LOW);
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
	} while (0);
}

void ar7100_spi_send_addr(unsigned int _addr)
{
	do {
		ar7100_spi_bit_banger(((_addr & 0xff0000) >> 16));
		ar7100_spi_bit_banger(((_addr & 0x00ff00) >> 8));
		ar7100_spi_bit_banger(_addr & 0x0000ff);
	} while (0);
}

#define ar7100_spi_delay_8()    ar7100_spi_bit_banger(0)
#define ar7100_spi_done()       ar7100_reg_wr_nf(AR7100_SPI_FS, 0)

struct flashconfig {
	__u32 byte_cnt;
	__u32 sector_cnt;
	__u32 sector_size;
	__u32 cs_addrmask;
} static flashconfig_tbl[MAX_FLASH] = {
	{0, 0, 0, 0},		//
	{STM_1MB_BYTE_COUNT, STM_1MB_SECTOR_COUNT, STM_1MB_SECTOR_SIZE, 0x0},	//
	{STM_2MB_BYTE_COUNT, STM_2MB_SECTOR_COUNT, STM_2MB_SECTOR_SIZE, 0x0},	//
	{STM_4MB_BYTE_COUNT, STM_4MB_SECTOR_COUNT, STM_4MB_SECTOR_SIZE, 0x0},	//
	{STM_8MB_BYTE_COUNT, STM_8MB_SECTOR_COUNT, STM_8MB_SECTOR_SIZE, 0x0},	//
	{STM_16MB_BYTE_COUNT, STM_16MB_SECTOR_COUNT, STM_16MB_SECTOR_SIZE, 0x0}	//
};

struct opcodes {
	__u16 code;
	__s8 tx_cnt;
	__s8 rx_cnt;
} static stm_opcodes[] = {
	{STM_OP_WR_ENABLE, 1, 0},	//
	{STM_OP_WR_DISABLE, 1, 0},	//
	{STM_OP_RD_STATUS, 1, 1},	//
	{STM_OP_WR_STATUS, 1, 0},	//
	{STM_OP_RD_DATA, 4, 4},	//
	{STM_OP_FAST_RD_DATA, 5, 0},	//
	{STM_OP_PAGE_PGRM, 8, 0},	//
	{STM_OP_SECTOR_ERASE, 4, 0},	//
	{STM_OP_BULK_ERASE, 1, 0},	//
	{STM_OP_DEEP_PWRDOWN, 1, 0},	//
	{STM_OP_RD_SIG, 4, 1},	//
};

#define busy_wait(condition, wait) \
	do { \
		while (condition) { \
			if (!wait) \
			    udelay(1); \
			else \
			    udelay(wait*1000); \
		} \
	} while (0)

static int spiflash_probe_chip(void)
{
	unsigned int sig;
	int flash_size;

	ar7100_reg_wr_nf(AR7100_SPI_CLOCK, 0x43);

	sig = 0x777777;

	ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
	ar7100_spi_bit_banger(0x9f);
	ar7100_spi_delay_8();
	ar7100_spi_delay_8();
	ar7100_spi_delay_8();
	ar7100_spi_done();
	/* rd = ar7100_reg_rd(AR7100_SPI_RD_STATUS); */
	ar7100_reg_wr_nf(AR7100_SPI_FS, 1);
	sig = ar7100_reg_rd(AR7100_SPI_READ);
	ar7100_reg_wr_nf(AR7100_SPI_FS, 0);

	switch (sig) {
	case STM_8MBIT_SIGNATURE:
		flash_size = FLASH_1MB;
		break;
	case STM_16MBIT_SIGNATURE:
		flash_size = FLASH_2MB;
		break;
	case STM_32MBIT_SIGNATURE:
		flash_size = FLASH_4MB;
		break;
	case STM_64MBIT_SIGNATURE:
		flash_size = FLASH_8MB;
		break;
	case STM_128MBIT_SIGNATURE:
		flash_size = FLASH_16MB;
		break;
	default:
		puts("Read of flash device signature failed!\n");
		return (0);
	}

	return (flash_size);
}

static void ar7100_spi_write_enable()
{
	ar7100_reg_wr_nf(AR7100_SPI_FS, 1);
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
	ar7100_spi_bit_banger(AR7100_SPI_CMD_WREN);
	ar7100_spi_go();
}

static void ar7100_spi_poll()
{
	int rd;

	do {
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
		ar7100_spi_bit_banger(AR7100_SPI_CMD_RD_STATUS);
		ar7100_spi_delay_8();
		rd = (ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 1);
	} while (rd);
}

static unsigned int getPartition(char *name);

/* erases nvram partition on the detected location or simply returns if no nvram was detected */

static int flash_erase_nvram(unsigned int flashsize, unsigned int blocksize)
{
	unsigned int res;
	unsigned int offset = nvramdetect;
	struct opcodes *ptr_opcode;
	__u32 temp, reg;
	if (!nvramdetect) {
		nvramdetect = getPartition("cfg");
	}
	if (!nvramdetect) {
		puts("nvram can and will not erased, since nvram was not detected on this device (maybe dd-wrt isnt installed)!\n");
		return -1;
	}
	printf("erasing nvram at [0x%08X]\n", nvramdetect);

	ar7100_spi_write_enable();
	ar7100_spi_bit_banger(AR7100_SPI_CMD_SECTOR_ERASE);
	ar7100_spi_send_addr(offset);
	ar7100_spi_go();
	display(0x7d);
	ar7100_spi_poll();

	puts("done\n");
	return 0;
}

static int flashdetected = 0;
/* detects spi flash and its size */
static int flashdetect(void)
{
	if (flashdetected)
		return 0;
	flashsize = 8 * 1024 * 1024;
	flashbase = 0xa8000000;
	int index = 0;
	if (!(index = spiflash_probe_chip())) {
		puts("Found no serial flash device, cannot reset to factory defaults\n");
		return -1;
	} else {
		flashsize = flashconfig_tbl[index].byte_cnt;
		sectorsize = flashconfig_tbl[index].sector_size;
		if (flashsize == 8 * 1024 * 1024)
			flashbase = 0xa8000000;
		else
			flashbase = 0xbfc00000;
		printf
		    ("Found Flash device SIZE=0x%08X SECTORSIZE=0x%08X FLASHBASE=0x%08X\n",
		     flashsize, sectorsize, flashbase);
	}
	flashdetected = 1;
	return 0;

}
