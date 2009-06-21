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
static unsigned int linuxaddr = 0xbfc10000;
static unsigned int flashbase = 0xa8000000;
static unsigned int flashsize = 0x800000;

#define AR2316_DSLBASE          0xB1000000	/* RESET CONTROL MMR */
#define AR531XPLUS_SPI              0xB1300000	/* SPI FLASH MMR */
#define AR2316_GPIO_DI          (AR2316_DSLBASE + 0x0088)
#define AR2316_RESET            (AR2316_DSLBASE + 0x0004)
#define AR2316_IF_CTL           (AR2316_DSLBASE + 0x0018)
#define AR2316_ENDIAN_CTL       (AR2316_DSLBASE + 0x000c)
#define AR2316_WDC              (AR2316_DSLBASE + 0x003c)
#define AR2316_AHB_ARB_CTL      (AR2316_DSLBASE + 0x0008)
#define RESET_WARM_WLAN0_MAC        0x00000001	/* warm reset WLAN0 MAC */
#define RESET_WARM_WLAN0_BB         0x00000002	/* warm reset WLAN0 BaseBand */
#define RESET_MPEGTS_RSVD           0x00000004	/* warm reset MPEG-TS */
#define RESET_PCIDMA                0x00000008	/* warm reset PCI ahb/dma */
#define RESET_MEMCTL                0x00000010	/* warm reset memory controller */
#define RESET_LOCAL                 0x00000020	/* warm reset local bus */
#define RESET_I2C_RSVD              0x00000040	/* warm reset I2C bus */
#define RESET_SPI                   0x00000080	/* warm reset SPI interface */
#define RESET_UART0                 0x00000100	/* warm reset UART0 */
#define RESET_IR_RSVD               0x00000200	/* warm reset IR interface */
#define RESET_EPHY0                 0x00000400	/* cold reset ENET0 phy */
#define RESET_ENET0                 0x00000800	/* cold reset ENET0 mac */

#define IF_TS_LOCAL                 2

#define CONFIG_ETHERNET             0x00000040	/* Ethernet byteswap */

#define ARB_CPU                     0x00000001	/* CPU, default */
#define ARB_WLAN                    0x00000002	/* WLAN */
#define ARB_MPEGTS_RSVD             0x00000004	/* MPEG-TS */
#define ARB_LOCAL                   0x00000008	/* LOCAL */
#define ARB_PCI                     0x00000010	/* PCI */
#define ARB_ETHERNET                0x00000020	/* Ethernet */
#define ARB_RETRY                   0x00000100	/* retry policy, debug only */

#define disable_watchdog() \
{ 					\
}					\

static int getGPIO(int nr)
{
	volatile unsigned int *gpio = (unsigned int *)AR2316_GPIO_DI;
	if ((*gpio & 1 << nr) == (1 << nr))
		return 1;
	return 0;
}

static void enable_ethernet(void)
{
	unsigned int mask = RESET_ENET0 | RESET_EPHY0;
	unsigned int regtmp;
	regtmp = sysRegRead(AR2316_AHB_ARB_CTL);
	regtmp |= ARB_ETHERNET;
	sysRegWrite(AR2316_AHB_ARB_CTL, regtmp);

	regtmp = sysRegRead(AR2316_RESET);
	sysRegWrite(AR2316_RESET, regtmp | mask);
	udelay(10000);

	regtmp = sysRegRead(AR2316_RESET);
	sysRegWrite(AR2316_RESET, regtmp & ~mask);
	udelay(10000);

	regtmp = sysRegRead(AR2316_IF_CTL);
	regtmp |= IF_TS_LOCAL;
	sysRegWrite(AR2316_IF_CTL, regtmp);

	regtmp = sysRegRead(AR2316_ENDIAN_CTL);
	regtmp &= ~CONFIG_ETHERNET;
	sysRegWrite(AR2316_ENDIAN_CTL, regtmp);
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

#define SPI_FLASH_MMR           AR531XPLUS_SPI_MMR

#define SPI_WRITE_ENABLE    0
#define SPI_WRITE_DISABLE   1
#define SPI_RD_STATUS       2
#define SPI_WR_STATUS       3
#define SPI_RD_DATA         4
#define SPI_FAST_RD_DATA    5
#define SPI_PAGE_PROGRAM    6
#define SPI_SECTOR_ERASE    7
#define SPI_BULK_ERASE      8
#define SPI_DEEP_PWRDOWN    9
#define SPI_RD_SIG          10
#define SPI_MAX_OPCODES     11

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

static __u32 spiflash_regread32(int reg)
{
	volatile __u32 *data = (__u32 *)(AR531XPLUS_SPI + reg);

	return (*data);
}

static void spiflash_regwrite32(int reg, __u32 data)
{
	volatile __u32 *addr = (__u32 *)(AR531XPLUS_SPI + reg);

	*addr = data;
	return;
}

#define busy_wait(condition, wait) \
	do { \
		while (condition) { \
			if (!wait) \
			    udelay(1); \
			else \
			    udelay(wait*1000); \
		} \
	} while (0)

static __u32 spiflash_sendcmd(int op, u32 addr)
{
	u32 reg;
	u32 mask;
	struct opcodes *ptr_opcode;

	ptr_opcode = &stm_opcodes[op];
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	spiflash_regwrite32(SPI_FLASH_OPCODE,
			    ((u32)ptr_opcode->code) | (addr << 8));

	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt |
	    (ptr_opcode->rx_cnt << 4) | SPI_CTL_START;

	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	busy_wait(spiflash_regread32(SPI_FLASH_CTL) & SPI_CTL_BUSY, 0);

	if (!ptr_opcode->rx_cnt)
		return 0;

	reg = (__u32)spiflash_regread32(SPI_FLASH_DATA);

	switch (ptr_opcode->rx_cnt) {
	case 1:
		mask = 0x000000ff;
		break;
	case 2:
		mask = 0x0000ffff;
		break;
	case 3:
		mask = 0x00ffffff;
		break;
	default:
		mask = 0xffffffff;
		break;
	}
	reg &= mask;

	return reg;
}

static int spiflash_probe_chip(void)
{
	unsigned int sig;
	int flash_size;

	sig = spiflash_sendcmd(SPI_RD_SIG, 0);

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

/* erases nvram partition on the detected location or simply returns if no nvram was detected */

static int flash_erase_nvram(unsigned int flashsize, unsigned int blocksize)
{
	unsigned int res;
	unsigned int offset = nvramdetect;
	struct opcodes *ptr_opcode;
	__u32 temp, reg;
	if (!nvramdetect) {
		puts("nvram can and will not erased, since nvram was not detected on this device (maybe dd-wrt isnt installed)!\n");
		return -1;
	}
	printf("erasing nvram at [0x%08X]\n", nvramdetect);

	ptr_opcode = &stm_opcodes[SPI_SECTOR_ERASE];

	temp = ((__u32)offset << 8) | (__u32)(ptr_opcode->code);
	spiflash_sendcmd(SPI_WRITE_ENABLE, 0);
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	spiflash_regwrite32(SPI_FLASH_OPCODE, temp);

	reg =
	    (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt |
	    SPI_CTL_START;
	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	busy_wait(spiflash_sendcmd(SPI_RD_STATUS, 0) & SPI_STATUS_WIP, 20);

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
