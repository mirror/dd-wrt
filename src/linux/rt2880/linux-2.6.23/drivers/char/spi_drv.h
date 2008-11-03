#ifndef __SPIDRV
#define __SPIDRV

#include <asm/rt2880/rt_mmap.h>

#define	RT2880_SPI_DUMP_STR		"dump"	/* Dump Content Command Prompt    */
#define	RT2880_SPI_READ_STR		"read"	/* SPI read operation */
#define	RT2880_SPI_WRITE_STR		"write"	/* SPI read operation */

#define RT2880_SPI_DUMP        2
#define RT2880_SPI_READ        3
#define RT2880_SPI_WRITE       5
#define RT2880_SPI_INIT_VTSS_NOVLAN   7
#define RT2880_SPI_INIT_VTSS_VLAN     9
#define RT2880_SPI_VTSS_READ   11
#define RT2880_SPI_VTSS_WRITE  13

#define SPI_DEV_NAME	"spiS0"

typedef struct spi_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} SPI_WRITE;

typedef struct spi_vtss_data {
	unsigned int blk;
	unsigned int sub;
	unsigned int reg;
	unsigned long value;
} SPI_VTSS;

/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
#define RT2880_REG(x)		(*((volatile u32 *)(x)))

#define RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)
#define RSTCTRL_SPI_RESET		RALINK_SPI_RST

#define RT2880_SPI_REG_BASE		(RALINK_SPI_BASE)
#define RT2880_SPISTAT_REG		(RT2880_SPI_REG_BASE+0x00)
#define RT2880_SPICFG_REG		(RT2880_SPI_REG_BASE+0x10)
#define RT2880_SPICTL_REG		(RT2880_SPI_REG_BASE+0x14)
#define RT2880_SPIDATA_REG		(RT2880_SPI_REG_BASE+0x20)


/* SPICFG register bit field */
#define SPICFG_LSBFIRST				(0<<8)
#define SPICFG_MSBFIRST				(1<<8)

#define SPICFG_RXCLKEDGE_FALLING	(1<<5)		/* rx on the falling edge of the SPICLK signal */
#define SPICFG_TXCLKEDGE_FALLING	(1<<4)		/* tx on the falling edge of the SPICLK signal */

#define SPICFG_SPICLK_DIV2			(0<<0)		/* system clock rat / 2  */
#define SPICFG_SPICLK_DIV4			(1<<0)		/* system clock rat / 4  */
#define SPICFG_SPICLK_DIV8			(2<<0)		/* system clock rat / 8  */
#define SPICFG_SPICLK_DIV16			(3<<0)		/* system clock rat / 16  */
#define SPICFG_SPICLK_DIV32			(4<<0)		/* system clock rat / 32  */
#define SPICFG_SPICLK_DIV64			(5<<0)		/* system clock rat / 64  */
#define SPICFG_SPICLK_DIV128		(6<<0)		/* system clock rat / 128 */

#define SPICFG_SPICLKPOL		(1<<6)		/* spi clk*/

/* SPICTL register bit field */
#define SPICTL_HIZSDO				(1<<3)
#define SPICTL_STARTWR				(1<<2)
#define SPICTL_STARTRD				(1<<1)
#define SPICTL_SPIENA_ON			(0<<0)		/* #cs low active */
#define SPICTL_SPIENA_OFF			(1<<0)


#define IS_BUSY		(RT2880_REG(RT2880_SPISTAT_REG) & 0x01)

#define spi_busy_loop 3000
#define max_ee_busy_loop 500


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define WREN_CMD	0x06
#define WRDI_CMD	0x04
#define RDSR_CMD	0x05
#define WRSR_CMD	0x01
#define READ_CMD	0x03
#define WRITE_CMD	0x02

/* STATUS REGISTER BIT */
#define RDY 0	/*  Busy Indicator Bit */
#define WEN 1	/*  Write Enable Bit   */
#define BP0 2	/* Block Write Protect Bit */
#define BP1 3	/* Block Write Protect Bit */
#define WPEN 7	/* Software Write Protect Enable Bit */


#define ENABLE	1
#define DISABLE	0

#endif
