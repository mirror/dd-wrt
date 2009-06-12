/*
 * misc_lzma.c
 * originally written for xscale based linux kernel decompression
 * now adapted for AR531X based redboot stub and kernel loader
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 * this stub will load and decompress redboot if the reset button is pushed, otherwise it parses the redboot directory for a partition named linux*, vmlinux* or kernel*
 * if such a partition has been found, it will be decompressed and executed, if not. redboot is started. if a decompression error occures while loading the linux partition, 
 * the redboot is started too.
 * take care about the ramconfig.h header since it must contain the correct ram size and gpio button value for the reset button
 * this code is partially based on redboot and linux sources
 */

#ifdef STANDALONE_DEBUG
#define putstr printf
#else

#include <linux/kernel.h>

#include <asm/uaccess.h>
#include "uncompress.h"
#include "spiflash.h"
#include "printf.h"
#include "ramconfig.h"

#endif

#define __ptr_t void *

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;

static uch *inbuf;		/* input buffer */

static unsigned insize = 0;	/* valid bytes in inbuf */
static unsigned inptr = 0;	/* index of next byte to be processed in inbuf */
static unsigned outcnt;		/* bytes in output buffer */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

static int fill_inbuf(void);
static void flush_window(void);
static void error(char *m);

extern unsigned char input_data[];
extern unsigned char input_data_end[];

static ulg output_ptr = 0;
static uch *output_data;
static ulg bytes_out;

extern int end;
static ulg free_mem_ptr;
static ulg free_mem_ptr_end;

#define _LZMA_IN_CB

#include "lib/LzmaDecode.h"
static int read_byte(unsigned char **buffer, UInt32 * bufferSize);
#include "lib/LzmaDecode.c"

int bootoffset = 0x800004bc;

/*
 * Do the lzma decompression
 */

static void print_hex(int val)
{
	static char *xlate = "0123456789abcdef";
	int i;

	puts("0x");

	for (i = 28; i >= 0; i -= 4) {
		putc(xlate[(val >> i) & 0xf]);
	}
}

static int disaster = 0;
static int lzma_unzip(void)
{

	unsigned int i;
	unsigned int uncompressedSize = 0;
	unsigned char *workspace;
	unsigned int lc, lp, pb;

	// lzma args
	i = get_byte();
	lc = i % 9, i = i / 9;
	lp = i % 5, pb = i / 5;

	// skip dictionary size
	for (i = 0; i < 4; i++)
		get_byte();
	// get uncompressed size
	int a, b, c, d;
	a = get_byte();
	b = get_byte();
	c = get_byte();
	d = get_byte();
	uncompressedSize = (a) + (b << 8) + (c << 16) + (d << 24);
	if (uncompressedSize > 0x400000 || lc > 3 || pb > 3 || lp > 3) {
		if (disaster) {
			error
			    ("\r\ndata corrupted in recovery RedBoot too, this is a disaster condition. please re-jtag\r\n");
		}
		disaster = 1;
		puts("\r\ndata corrupted!\r\nswitching to recovery RedBoot\r\nloading");
		inbuf = input_data;
		insize = &input_data_end[0] - &input_data[0];
		inptr = 0;
		output_data = (uch *) 0x80000400;
		bootoffset = 0x800004bc;
		return lzma_unzip();

	}
	workspace = output_data + uncompressedSize;
	// skip high order bytes
	for (i = 0; i < 4; i++)
		get_byte();
	// decompress kernel
	if (LzmaDecode
	    (workspace, ~0, lc, lp, pb, (unsigned char *)output_data,
	     uncompressedSize, &i) == LZMA_RESULT_OK) {
		if (i != uncompressedSize) {
			if (disaster) {
				error
				    ("data corrupted in recovery RedBoot too, this is a disaster condition. please re-jtag\r\n");
			}
			disaster = 1;
			puts("\r\ndata corrupted!\r\nswitching to recovery RedBoot\r\nloading");
			inbuf = input_data;
			insize = &input_data_end[0] - &input_data[0];
			inptr = 0;
			output_data = (uch *) 0x80000400;
			bootoffset = 0x800004bc;
			return lzma_unzip();
		}
		//copy it back to low_buffer
		bytes_out = i;
		output_ptr = i;
		return 0;
	}
	return 1;
}

/*if (((unsigned int)offset % 4) == 0) {
	vall = *(unsigned int *)buffer;
	buffer += 4;
}
    if (buffer>=BufferLim)
	{
	ExtraBytes=1;
	return 0xff;
	}
  return *(((unsigned char *)&vall) + (offset++ & 3));
*/

static unsigned int icnt = 0;
static int read_byte(unsigned char **buffer, UInt32 * bufferSize)
{
	static unsigned char val;
	*bufferSize = 1;
	val = get_byte();
	*buffer = &val;
	if (icnt++ % (1024 * 10) == 0)
		puts(".");
	return LZMA_RESULT_OK;
}

struct fis_image_desc {
	unsigned char name[16];	// Null terminated name
	unsigned long flash_base;	// Address within FLASH of image
	unsigned long mem_base;	// Address in memory where it executes
	unsigned long size;	// Length of image
	unsigned long entry_point;	// Execution entry point
	unsigned long data_length;	// Length of actual data
	unsigned char _pad[256 - (16 + 7 * sizeof(unsigned long))];
	unsigned long desc_cksum;	// Checksum over image descriptor
	unsigned long file_cksum;	// Checksum over image data
};

static unsigned int sectorsize = 0x10000;
static unsigned int linuxaddr = 0xbfc10000;
/*
 * searches for a directory entry named linux* vmlinux* or kernel and returns its flash address (it also initializes entrypoint and load address)
 */
unsigned int getLinux(void)
{
	int count;
	unsigned char *p = (unsigned char *)(0xa8800000 - (sectorsize * 2));
	struct fis_image_desc *fis = (struct fis_image_desc *)p;
	while (fis->name[0] != 0xff && count < 10) {
		if (!strncmp(fis->name, "linux", 5)
		    || !strncmp(fis->name, "vmlinux", 7)
		    || !strcmp(fis->name, "kernel")) {
			puts("found bootable image: ");
			puts(fis->name);
			puts(" at ");
			print_hex(fis->flash_base);
			puts(" entrypoint ");
			print_hex(fis->entry_point);
			puts("\r\n");
			bootoffset = fis->entry_point;
			output_data = (uch *) fis->mem_base;
			return fis->flash_base;
		}
		p += 256;
		fis = (struct fis_image_desc *)p;
		count++;
	}
	puts("no bootable image found, try default location 0xbfc10000\r\n");
	bootoffset = 0x80041000;
	output_data = 0x80041000;
	return 0xbfc10000;
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int resettrigger = 0;

int fill_inbuf(void)
{
	if (insize != 0)
		error("ran out of input data");
	if (resettrigger) {
		inbuf = linuxaddr;
		insize = 0x400000;
		inptr = 1;
	} else {
		inbuf = input_data;
		insize = &input_data_end[0] - &input_data[0];
		inptr = 1;
	}
	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */

#ifndef arch_error
#define arch_error(x)
#endif

static void error(char *x)
{
	arch_error(x);

	puts("\r\n\r\n");
	puts(x);
	puts("\r\n\r\n -- System halted");

	while (1) ;		/* Halt */
}

/*void __div0(void)
{
	error("division by zero");
}*/
#define AR2316_DSLBASE          0xB1000000	/* RESET CONTROL MMR */
#define AR2316_GPIO_DI          (AR2316_DSLBASE + 0x0088)

static int getGPIO(int nr)
{
	volatile unsigned int *gpio = (unsigned int *)AR2316_GPIO_DI;
	if ((*gpio & 1 << nr) == (1 << nr))
		return 1;
	return 0;
}

/*
 * checks if the reset button is pressed, return 1 if the button is pressed and 0 if not
 */
static int resetTouched(void)
{
	int trigger = getGPIO(RESETBUTTON & 0x0f);
	if (RESETBUTTON & 0xf0)
		trigger = 1 - trigger;
	return trigger;
}

#define RTC_DENOMINATOR 100
#define RTC_PERIOD 110000000 / RTC_DENOMINATOR

#define MACRO_START do {
#define MACRO_END   } while (0)
static unsigned int cyg_hal_clock_period;

#define HAL_CLOCK_INITIALIZE( _period_ )        \
MACRO_START                                 \
    asm volatile (                              \
        "mtc0 $0,$9\n"                          \
        "nop; nop; nop\n"                       \
        "mtc0 %0,$11\n"                         \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_period_)                         \
        );                                      \
    cyg_hal_clock_period = _period_;            \
MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )   \
MACRO_START                                 \
    asm volatile (                              \
        "mtc0 $0,$9\n"                          \
        "nop; nop; nop\n"                       \
        "mtc0 %0,$11\n"                         \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_period_)                         \
        );                                      \
MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )              \
MACRO_START                                 \
    register unsigned int result;                 \
    asm volatile (                              \
        "mfc0   %0,$9\n"                        \
        : "=r"(result)                          \
        );                                      \
    *(_pvalue_) = result;                       \
MACRO_END

/*
 * 
 */
static void delay_us(int us)
{
	unsigned int val1, val2;
	int diff;
	long usticks;
	long ticks;

	// Calculate the number of counter register ticks per microsecond.

	usticks = (RTC_PERIOD * RTC_DENOMINATOR) / 1000000;

	// Make sure that the value is not zero. This will only happen if the
	// CPU is running at < 2MHz.
	if (usticks == 0)
		usticks = 1;

	while (us > 0) {
		int us1 = us;

		// Wait in bursts of less than 10000us to avoid any overflow
		// problems in the multiply.
		if (us1 > 10000)
			us1 = 10000;

		us -= us1;

		ticks = us1 * usticks;

		HAL_CLOCK_READ(&val1);
		while (ticks > 0) {
			do {
				HAL_CLOCK_READ(&val2);
			} while (val1 == val2);
			diff = val2 - val1;
			if (diff < 0)
				diff += RTC_PERIOD;
			ticks -= diff;
			val1 = val2;
		}
	}
}

#ifndef STANDALONE_DEBUG

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
#define AR2316_DSLBASE          0xB1000000	/* RESET CONTROL MMR */
#define AR2316_RESET            (AR2316_DSLBASE + 0x0004)
#define AR2316_IF_CTL           (AR2316_DSLBASE + 0x0018)
#define AR2316_ENDIAN_CTL       (AR2316_DSLBASE + 0x000c)

#define CONFIG_ETHERNET             0x00000040	/* Ethernet byteswap */

#define AR2316_AHB_ARB_CTL      (AR2316_DSLBASE + 0x0008)
#define ARB_CPU                     0x00000001	/* CPU, default */
#define ARB_WLAN                    0x00000002	/* WLAN */
#define ARB_MPEGTS_RSVD             0x00000004	/* MPEG-TS */
#define ARB_LOCAL                   0x00000008	/* LOCAL */
#define ARB_PCI                     0x00000010	/* PCI */
#define ARB_ETHERNET                0x00000020	/* Ethernet */
#define ARB_RETRY                   0x00000100	/* retry policy, debug only */

#define AR531XPLUS_SPI              0xB1300000	/* SPI FLASH MMR */

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

typedef unsigned int AR531X_REG;

#undef sysRegRead
#undef sysRegWrite
#define sysRegRead(phys)	\
	(*(volatile AR531X_REG *)(KSEG1|phys))

#define sysRegWrite(phys, val)	\
	((*(volatile AR531X_REG *)(KSEG1|phys)) = (val))

struct flashconfig {
	__u32 byte_cnt;
	__u32 sector_cnt;
	__u32 sector_size;
	__u32 cs_addrmask;
} flashconfig_tbl[MAX_FLASH] = {
	{
	0, 0, 0, 0}, {
	STM_1MB_BYTE_COUNT, STM_1MB_SECTOR_COUNT, STM_1MB_SECTOR_SIZE, 0x0},
	{
	STM_2MB_BYTE_COUNT, STM_2MB_SECTOR_COUNT, STM_2MB_SECTOR_SIZE, 0x0},
	{
	STM_4MB_BYTE_COUNT, STM_4MB_SECTOR_COUNT, STM_4MB_SECTOR_SIZE, 0x0},
	{
	STM_8MB_BYTE_COUNT, STM_8MB_SECTOR_COUNT, STM_8MB_SECTOR_SIZE, 0x0},
	{
	STM_16MB_BYTE_COUNT, STM_16MB_SECTOR_COUNT,
		    STM_16MB_SECTOR_SIZE, 0x0}
};

struct opcodes {
	__u16 code;
	__s8 tx_cnt;
	__s8 rx_cnt;
} stm_opcodes[] = {
	{
	STM_OP_WR_ENABLE, 1, 0}, {
	STM_OP_WR_DISABLE, 1, 0}, {
	STM_OP_RD_STATUS, 1, 1}, {
	STM_OP_WR_STATUS, 1, 0}, {
	STM_OP_RD_DATA, 4, 4}, {
	STM_OP_FAST_RD_DATA, 5, 0}, {
	STM_OP_PAGE_PGRM, 8, 0}, {
	STM_OP_SECTOR_ERASE, 4, 0}, {
	STM_OP_BULK_ERASE, 1, 0}, {
	STM_OP_DEEP_PWRDOWN, 1, 0}, {
STM_OP_RD_SIG, 4, 1},};

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

static __u32 spiflash_sendcmd(int op, u32 addr)
{
	u32 reg;
	u32 mask;
	struct opcodes *ptr_opcode;

	ptr_opcode = &stm_opcodes[op];
	do {
		reg = spiflash_regread32(SPI_FLASH_CTL);
	} while (reg & SPI_CTL_BUSY);

	spiflash_regwrite32(SPI_FLASH_OPCODE,
			    ((u32)ptr_opcode->code) | (addr << 8));

	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt |
	    (ptr_opcode->rx_cnt << 4) | SPI_CTL_START;

	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	do {
		reg = spiflash_regread32(SPI_FLASH_CTL);
	} while (reg & SPI_CTL_BUSY);

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
		puts("Read of flash device signature failed!\r\n");
		return (0);
	}

	return (flash_size);
}

static int flash_erase_nvram(unsigned int flashsize, unsigned int blocksize)
{
	unsigned int res;
	unsigned int offset = flashsize - (blocksize * 3);
	puts("erasing nvram....\r\n");
	spiflash_sendcmd(STM_OP_WR_ENABLE, 0);
	do {
		res = spiflash_sendcmd(STM_OP_RD_STATUS, 0);
		if ((res & 0x3) == 0x2) {
			break;
		}
		delay_us(200000);
		spiflash_sendcmd(STM_OP_WR_ENABLE, 0);
	} while (1);
	spiflash_sendcmd(STM_OP_SECTOR_ERASE, offset);
	while (true) {
		res = spiflash_sendcmd(STM_OP_RD_STATUS, 0);
		if ((res & STM_STATUS_WIP) == 0) {
			break;
		}
	}
	puts("done\r\n");
	return 0;
}

ulg
decompress_kernel(ulg output_start, ulg free_mem_ptr_p, ulg free_mem_ptr_end_p)
{
	output_data = (uch *) output_start;	/* Points to kernel start */
	free_mem_ptr = free_mem_ptr_p;
	free_mem_ptr_end = free_mem_ptr_end_p;

	arch_decomp_setup();

	puts("MicroRedBoot v1.1, (c) 2009 DD-WRT.COM (");
	puts(__DATE__);
	puts(")\r\n");
	if (resetTouched()) {
		puts("Reset Button triggered\r\nBooting Recovery RedBoot\r\n");
		int count = 5;
		while (count--) {
			if (!resetTouched()) // check if reset button is unpressed again
				break;
			delay_us(1000000);
		}
		if (!count) {
			puts("reset button 5 seconds pushed, erasing nvram\r\n");
			int index = 0;
			if (!(index = spiflash_probe_chip())) {
				puts("Found no serial flash device, cannot reset to factory defaults\r\n");
			} else {
				puts("Found Flash device SIZE=");
				print_hex(flashconfig_tbl[index].byte_cnt);
				puts(" SECTORSIZE=");
				print_hex(flashconfig_tbl[index].sector_size);
				sectorsize = flashconfig_tbl[index].sector_size;
				puts("\r\n");
				flash_erase_nvram(flashconfig_tbl[index].
						  byte_cnt,
						  flashconfig_tbl[index].
						  sector_size);
			}

		}
		bootoffset = 0x800004bc;
		resettrigger = 0;
	} else {
		linuxaddr = getLinux();
		puts("Booting Linux\r\n");
		resettrigger = 1;
		/* initialize clock */
		HAL_CLOCK_INITIALIZE(RTC_PERIOD);
		unsigned int mask = RESET_ENET0 | RESET_EPHY0;
		unsigned int regtmp;
		
		/* important, enable ethernet bus, if the following lines are not initialized linux will not be able to use the ethernet mac, taken from redboot source */
		regtmp = sysRegRead(AR2316_AHB_ARB_CTL);
		regtmp |= ARB_ETHERNET;
		sysRegWrite(AR2316_AHB_ARB_CTL, regtmp);

		regtmp = sysRegRead(AR2316_RESET);
		sysRegWrite(AR2316_RESET, regtmp | mask);
		delay_us(10000);

		regtmp = sysRegRead(AR2316_RESET);
		sysRegWrite(AR2316_RESET, regtmp & ~mask);
		delay_us(10000);

		regtmp = sysRegRead(AR2316_IF_CTL);
		regtmp |= IF_TS_LOCAL;
		sysRegWrite(AR2316_IF_CTL, regtmp);

		regtmp = sysRegRead(AR2316_ENDIAN_CTL);
		regtmp &= ~CONFIG_ETHERNET;
		sysRegWrite(AR2316_ENDIAN_CTL, regtmp);

		int index = 0;
		if (!(index = spiflash_probe_chip())) {
			puts("Found no serial flash device\r\n");
		} else {
			puts("Found Flash device SIZE=");
			print_hex(flashconfig_tbl[index].byte_cnt);
			puts(" SECTORSIZE=");
			print_hex(flashconfig_tbl[index].sector_size);
			sectorsize = flashconfig_tbl[index].sector_size;
			puts("\r\n");

		}
	}
	puts("loading");
	lzma_unzip();
	puts("\r\n\r\n\r\n");

	return output_ptr;
}
#else

char output_buffer[1500 * 1024];

int main()
{
	output_data = output_buffer;
	puts("Uncompressing Linux...");
	lzma_unzip();
	puts("done.\r\n");
	return 0;
}
#endif
