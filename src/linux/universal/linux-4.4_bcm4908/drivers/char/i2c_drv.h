#ifndef __I2CDRV
#define __I2CDRV

#include <asm/rt2880/rt_mmap.h>

#define	RT2880_I2C_READ_STR		"read"	/* I2C read operation */
#define	RT2880_I2C_WRITE_STR		"write"	/* I2C read operation */

#define RT2880_I2C_READ        3
#define RT2880_I2C_WRITE       5

#define I2C_DEV_NAME	"i2cM0"

typedef struct i2c_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} I2C_WRITE;


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/

#define	RT2880_REG(x)						(*((volatile u32 *)(x)))
#define	RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)

#define RSTCTRL_I2C_RESET		RALINK_I2C_RST

#define RT2880_I2C_REG_BASE		(RALINK_I2C_BASE)
#define RT2880_I2C_CONFIG_REG		(RT2880_I2C_REG_BASE+0x00)
#define RT2880_I2C_CLKDIV_REG		(RT2880_I2C_REG_BASE+0x04)
#define RT2880_I2C_DEVADDR_REG		(RT2880_I2C_REG_BASE+0x08)
#define RT2880_I2C_ADDR_REG		(RT2880_I2C_REG_BASE+0x0C)
#define RT2880_I2C_DATAOUT_REG	 	(RT2880_I2C_REG_BASE+0x10)
#define RT2880_I2C_DATAIN_REG  		(RT2880_I2C_REG_BASE+0x14)
#define RT2880_I2C_STATUS_REG  		(RT2880_I2C_REG_BASE+0x18)
#define RT2880_I2C_STARTXFR_REG		(RT2880_I2C_REG_BASE+0x1C)
#define RT2880_I2C_BYTECNT_REG		(RT2880_I2C_REG_BASE+0x20)


/* I2C_CFG register bit field */
#define I2C_CFG_ADDRLEN_8				(7<<5)	/* 8 bits */
#define I2C_CFG_DEVADLEN_7				(6<<2)	/* 7 bits */
#define I2C_CFG_ADDRDIS					(1<<1)	/* disable address transmission*/
#define I2C_CFG_DEVADDIS				(1<<0)	/* disable evice address transmission */


#define IS_BUSY		(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x01)
#define IS_SDOEMPTY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x02)
#define IS_DATARDY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x04)


/*
 * max SCLK : 400 KHz (2.7V)
 * assumed that BUS CLK is 150 MHZ 
 * so DIV 375
 * SCLK = PB_CLK / CLKDIV -> CLKDIV = PB_CLK / SCLK = PB_CLK / 0.4
 */

/*
 * Example :
 * 	In RT3052, System clock is 40 / 3 = 13.3
 *	Hence, CLKDIV = 13.3 / 0.4 = 33	
 * 	In RT2880, System Clock is 133Mhz
 *	Hence, CLKDIV = 133 / 0.4 = 332.5 -> Use 333 ( If use 150Mhz, then 150 / 0.4 = 375 )
 */
#if (!defined (FPGA_BOARD_RT2880)) && (!defined (FPGA_BOARD_RT3052))
#define CLKDIV_VALUE	333
#else
#define CLKDIV_VALUE	60
#endif 


#define i2c_busy_loop 	(CLKDIV_VALUE*30)
#define max_ee_busy_loop	(CLKDIV_VALUE*25)
						  

/* 
 * AT24C01A/02/04/08A/16A (1K, 2K, 4K, 8K, 16K) 
 *	-- address : 8-bits
 * AT24C512 (512K)
 *  -- address : two 8-bits
 */    
#if (CONFIG_EEPROM_ADDRESS_BYTES == 2)
#define ADDRESS_BYTES	2
#else
#define ADDRESS_BYTES	1
#endif

/* 
 * sequential reads
 * because BYTECNT REG max 64 (6-bits)
 * , max READ_BLOCK is 64 
 */
#define READ_BLOCK		16

/*
 * AT24C01A/02 (1K, 2K)  have 8-byte Page
 */
#define WRITE_BLOCK		8


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define READ_CMD	0x01
#define WRITE_CMD	0x00


#define I2C_CFG_DEFAULT			(I2C_CFG_ADDRLEN_8  | \
								 I2C_CFG_DEVADLEN_7 | \
								 I2C_CFG_ADDRDIS)


#define ATMEL_ADDR		(0xA0>>1)
#define WM8751_ADDR		(0x36>>1)

#endif
