/*
 * ar5312.c - AR5312/AR2313 specific system functions 
 *
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 */


#include "mips32.c"

static unsigned int sectorsize = 0x10000;
static unsigned int linuxaddr = 0xbe010000;
static unsigned int flashbase = 0xbe000000;
static unsigned int flashsize = 0x800000;

#define AR531X_APBBASE  0xbc000000
#define AR531X_GPIO     (AR531X_APBBASE + 0x2000)
#define AR531X_GPIO_DI      (AR531X_GPIO + 0x04)
#define AR531X_RESETTMR (AR531X_APBBASE + 0x3000)
#define AR531X_WDC      (AR531X_RESETTMR + 0x0008)
#define AR531X_RESET    (AR531X_RESETTMR + 0x0020)
#define AR531X_ENABLE   (AR531X_RESETTMR + 0x0080)
#define ENABLE_ENET0              0x0002

#define RESET_ENET0          0x00000020	/* cold reset ENET0 mac */
#define RESET_EPHY0          0x00000008	/* cold reset ENET0 phy */

#define disable_watchdog() \
{ 					\
	sysRegWrite(AR531X_WDC, 0); 	\
}					\

static int getGPIO(int nr)
{
	volatile unsigned int *gpio = (unsigned int *)AR531X_GPIO_DI;
	if ((*gpio & 1 << nr) == (1 << nr))
		return 1;
	return 0;
}

static void enable_ethernet(void)
{
	unsigned int mask = RESET_ENET0 | RESET_EPHY0;
	unsigned int regtmp;

	regtmp = sysRegRead(AR531X_RESET);
	sysRegWrite(AR531X_RESET, regtmp | mask);
	udelay(15000);

	/* Pull out of reset */
	regtmp = sysRegRead(AR531X_RESET);
	sysRegWrite(AR531X_RESET, regtmp & ~mask);
	udelay(25);
	mask = ENABLE_ENET0;
	regtmp = sysRegRead(AR531X_ENABLE);
	sysRegWrite(AR531X_ENABLE, regtmp | mask);
}

typedef unsigned char FLASH_DATA_T;
#define FLASH_P2V( _a_ ) ((volatile FLASH_DATA_T *)((unsigned int)((_a_))))
#define FLASH_BLANKVALUE		(FLASH_DATA_T)(0xff)
#define FLASHWORD(x)			((FLASH_DATA_T)(x))
#define FLASH_POLLING_TIMEOUT	(3000000)
#define FLASH_READ_ID                   FLASHWORD( 0x90 )
#define FLASH_WP_STATE                  FLASHWORD( 0x90 )
#define FLASH_RESET                     FLASHWORD( 0xF0 )
#define FLASH_PROGRAM                   FLASHWORD( 0xA0 )
#define FLASH_BLOCK_ERASE               FLASHWORD( 0x30 )
#define FLASH_Query						FLASHWORD( 0x98 )	// Add by Jason for CFI support

#define FLASH_DATA                      FLASHWORD( 0x80 )	// Data complement
#define FLASH_BUSY                      FLASHWORD( 0x40 )	// "Toggle" bit
#define FLASH_ERR                       FLASHWORD( 0x20 )
#define FLASH_SECTOR_ERASE_TIMER        FLASHWORD( 0x08 )

#define FLASH_UNLOCKED                  FLASHWORD( 0x00 )
#define FLASH_WP_ADDR                  	(4)

#define FLASH_SETUP_ADDR1              	(0xAAA)
#define FLASH_SETUP_ADDR2              	(0x555)
#define FLASH_VENDORID_ADDR            	(0x0)
#define FLASH_DEVICEID_ADDR            	(0x2)
#define FLASH_DEVICEID_ADDR2            (0x1c)
#define FLASH_DEVICEID_ADDR3           	(0x1e)
//#define FLASH_WP_ADDR                         (0x12)
#define FLASH_SETUP_CODE1               FLASHWORD( 0xAA )
#define FLASH_SETUP_CODE2               FLASHWORD( 0x55 )
#define FLASH_SETUP_ERASE               FLASHWORD( 0x80 )
#define FLASH_ERR_OK			0x0
#define FLASH_ERR_DRV_TIMEOUT		-1

typedef struct {
	unsigned char devid;
	unsigned char *name;
	unsigned char size;	//in megabyte
} FLASHDEV;
static const FLASHDEV flashdevs[] = {
	{.devid = 0xc9,.name = "MX29LV640", .size=8},
	{.devid = 0xa8,.name = "MX29LV320", .size=4},
};
static int flashdetected = 0;

static int flashdetect(void)
{
	if (flashdetected)
		return 0;
	flashdetected=1;
	volatile FLASH_DATA_T *ROM;
	volatile FLASH_DATA_T *f_s1, *f_s2;
	FLASH_DATA_T id[4];
	FLASH_DATA_T w;
	long timeout = 50000;

	ROM = (volatile FLASH_DATA_T *)((unsigned int)0xbe000000);
	*(FLASH_P2V(ROM)) = FLASH_RESET;

	f_s1 = FLASH_P2V(ROM + FLASH_SETUP_ADDR1);
	f_s2 = FLASH_P2V(ROM + FLASH_SETUP_ADDR2);

	*f_s1 = FLASH_RESET;
	w = *(FLASH_P2V(ROM));

	*f_s1 = FLASH_SETUP_CODE1;
	*f_s2 = FLASH_SETUP_CODE2;
	*f_s1 = FLASH_READ_ID;

	id[0] = -1;
	id[1] = -1;

	// Manufacturers' code
	id[0] = *(FLASH_P2V(ROM + FLASH_VENDORID_ADDR));
	// Part number
	id[1] = *(FLASH_P2V(ROM + FLASH_DEVICEID_ADDR));
	id[2] = *(FLASH_P2V(ROM + FLASH_DEVICEID_ADDR2));
	id[3] = *(FLASH_P2V(ROM + FLASH_DEVICEID_ADDR3));

	*(FLASH_P2V(ROM)) = FLASH_RESET;

	// Stall, waiting for flash to return to read mode.
	int i;
	int found=0;
	for (i=0;i<sizeof(flashdevs)/sizeof(FLASHDEV);i++)
	    {
	    if (flashdevs[i].devid == id[1])
		{
		printf("FLASH: %s with %dM detected\n",flashdevs[i].name,flashdevs[i].size);
		flashsize = flashdevs[i].size*1024*1024;
		found=1;
		break;
		}
	    }
	if (!found)
	printf("Device not known: FLASH MANID: %X DEVID: %X DEVID2: %X DEVID3: %X\n", id[0],id[1], id[2], id[3]);
	while ((--timeout != 0) && (w != *(FLASH_P2V(ROM)))) ;
	return 0;
}

#define AR531X_FLASHCTL 0xb8400000
#define AR531X_FLASHCTL0        (AR531X_FLASHCTL + 0x00)
#define AR531X_FLASHCTL1        (AR531X_FLASHCTL + 0x04)
#define AR531X_FLASHCTL2        (AR531X_FLASHCTL + 0x08)
#define FLASHCTL_IDCY   0x0000000f	/* Idle cycle turn around time */
#define FLASHCTL_IDCY_S 0
#define FLASHCTL_WST1   0x000003e0	/* Wait state 1 */
#define FLASHCTL_WST1_S 5
#define FLASHCTL_RBLE   0x00000400	/* Read byte lane enable */
#define FLASHCTL_WST2   0x0000f800	/* Wait state 2 */
#define FLASHCTL_WST2_S 11
#define FLASHCTL_AC     0x00070000	/* Flash address check (added) */
#define FLASHCTL_AC_S   16
#define FLASHCTL_AC_128K 0x00000000
#define FLASHCTL_AC_256K 0x00010000
#define FLASHCTL_AC_512K 0x00020000
#define FLASHCTL_AC_1M   0x00030000
#define FLASHCTL_AC_2M   0x00040000
#define FLASHCTL_AC_4M   0x00050000
#define FLASHCTL_AC_8M   0x00060000
#define FLASHCTL_AC_RES  0x00070000	/* 16MB is not supported */
#define FLASHCTL_E      0x00080000	/* Flash bank enable (added) */
#define FLASHCTL_BUSERR 0x01000000	/* Bus transfer error status flag */
#define FLASHCTL_WPERR  0x02000000	/* Write protect error status flag */
#define FLASHCTL_WP     0x04000000	/* Write protect */
#define FLASHCTL_BM     0x08000000	/* Burst mode */
#define FLASHCTL_MW     0x30000000	/* Memory width */
#define FLASHCTL_MWx8   0x00000000	/* Memory width x8 */
#define FLASHCTL_MWx16  0x10000000	/* Memory width x16 */
#define FLASHCTL_MWx32  0x20000000	/* Memory width x32 (not supported) */
#define FLASHCTL_ATNR   0x00000000	/* Access type == no retry */
#define FLASHCTL_ATR    0x80000000	/* Access type == retry every */
#define FLASHCTL_ATR4   0xc0000000	/* Access type == retry every 4 */

static int flash_erase_nvram(unsigned int flashsize, unsigned int blocksize)
{
	int i, ticks;
	unsigned short val;
	if (!nvramdetect) {
		puts("nvram can and will not erased, since nvram was not detected on this device (maybe dd-wrt isnt installed)!\n");
		return;
	}
	unsigned int flash_ctl = sysRegRead(AR531X_FLASHCTL0);

	FLASH_DATA_T id[4];
//    puts("read id\n");
//    flash_query(id);
//    printf("FLASH MANID: %X DEVID: %X DEVID2: %X DEVID3: %X\n",id[0],id[1],id[2],id[3]);

	printf("erasing nvram at [0x%08X]\n", nvramdetect);

	volatile FLASH_DATA_T *ROM, *BANK;
	volatile FLASH_DATA_T *b_p = (FLASH_DATA_T *) (nvramdetect);
	volatile FLASH_DATA_T *b_v;
	volatile FLASH_DATA_T *f_s0, *f_s1, *f_s2;
	int timeout = 50000;
	FLASH_DATA_T state;
	int len;
	BANK = ROM =
	    (volatile FLASH_DATA_T *)((unsigned long)nvramdetect &
				      ~(0x800000 - 1));
	f_s0 = FLASH_P2V(BANK);
	f_s1 = FLASH_P2V(BANK + FLASH_SETUP_ADDR1);
	f_s2 = FLASH_P2V(BANK + FLASH_SETUP_ADDR2);
	len = blocksize;
	int res = FLASH_ERR_OK;

	*f_s1 = FLASH_SETUP_CODE1;
	*f_s2 = FLASH_SETUP_CODE2;
	*f_s1 = FLASH_WP_STATE;
	state = *FLASH_P2V(b_p + FLASH_WP_ADDR);
	*f_s0 = FLASH_RESET;

	if (FLASH_UNLOCKED != state) {
		*FLASH_P2V(ROM) = FLASH_RESET;
	}

	b_v = FLASH_P2V(b_p);

	*f_s1 = FLASH_SETUP_CODE1;
	*f_s2 = FLASH_SETUP_CODE2;
	*f_s1 = FLASH_SETUP_ERASE;
	*f_s1 = FLASH_SETUP_CODE1;
	*f_s2 = FLASH_SETUP_CODE2;
	*b_v = FLASH_BLOCK_ERASE;
	timeout = FLASH_POLLING_TIMEOUT;
	while (1) {
		state = *b_v;
		if ((state & FLASH_SECTOR_ERASE_TIMER)
		    == FLASH_SECTOR_ERASE_TIMER)
			break;
		udelay(1);
		if (--timeout == 0) {
			puts("flash erase timeout\n");
			res = FLASH_ERR_DRV_TIMEOUT;
			break;
		}
	}
	if (FLASH_ERR_OK == res) {
		timeout = FLASH_POLLING_TIMEOUT;
		while (1) {
			state = *b_v;
			if (FLASH_BLANKVALUE == state) {
				break;
			}
			udelay(1);
			if (--timeout == 0) {
				puts("flash erase timeout while waiting for erase complete\n");
				res = FLASH_ERR_DRV_TIMEOUT;
				break;
			}
		}
	}

	if (FLASH_ERR_OK != res)
		*FLASH_P2V(ROM) = FLASH_RESET;

	b_v = FLASH_P2V(b_p++);
	if (*b_v != FLASH_BLANKVALUE) {
		if (FLASH_ERR_OK == res) {
			puts("erase verify failed\n");
		} else {
			puts("nvram erase done\n");
		}
		return 0;
	}

}
