/*
 * This file contains glue for Atheros ar7240 spi flash interface
 * Primitives are ar7240_spi_*
 * mtd flash implements are ar7240_flash_*
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#include <linux/magic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <asm/delay.h>
#include <asm/io.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
#else
#include <asm/div64.h>
#include <linux/jiffies.h>
#endif





#include "ar7240.h"
#include "wzrg450nh_flash.h"

#ifdef	CONFIG_BUFFALO
#include <linux/proc_fs.h>
#endif	//CONFIG_BUFFALO

/* this is passed in as a boot parameter by bootloader */
extern int __ath_flash_size;

#ifdef	CONFIG_BUFFALO
#undef	AR7240_FLASH_MAX_BANKS
#define	AR7240_FLASH_MAX_BANKS	2
#endif	//CONFIG_BUFFALO
/*
 * statics
 */
#ifdef	CONFIG_BUFFALO
static void ar7240_spi_write_enable(unsigned int cs);
static void ar7240_spi_poll(unsigned int cs);
static void ar7240_spi_fast_read(unsigned int cs, uint32_t addr, uint8_t *data, int len);
static void ar7240_spi_read(unsigned int cs, uint32_t addr, uint8_t *data, int len);
#else	//CONFIG_BUFFALO
static void ar7240_spi_write_enable(void);
static void ar7240_spi_poll(void);
#endif	//CONFIG_BUFFALO
static void ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len);
static void ar7240_spi_sector_erase(uint32_t addr);



#ifdef	CONFIG_BUFFALO
//
//	BUFFALO EXTENTION for DIAG_LED
//
static	__u32	_s_lastaddr	= 0;
static	__u32	_f_lastwrite= 0;
static	__u32	_f_init		= 1;
static	__u32	_f_on		= 0;
static	__u32	_s_jiffies	= 0;
#define	_S_LASTADDR_MASK	0xFFFFC000	//16KBytes

#define	IS_NEW_SEQUENCE(F_WRITE)		( ((F_WRITE)==0 && _f_lastwrite) || (jiffies-_s_jiffies)>(HZ/10) )

#define	DBGMSG(S)	do { printk(KERN_DEBUG "%s[%08lX]::%s\n", __FUNCTION__, jiffies, (S)); } while(0)

#if 0 //	defined(CONFIG_MACH_AR7240)
//#include	"asm/mach-ar7240/ar7240.h"
#define	DIAG_GPIO_NO			(1<<14)
#define	DIAG_INIT(F_WRITE)		do { if(IS_NEW_SEQUENCE(F_WRITE)){_f_on=0;} while(_f_init) { ar7240_reg_rmw_set(AR7240_GPIO_OE, DIAG_GPIO_NO); _f_init=0; } } while(0)
#define	DIAG_ON()				do { ar7240_reg_wr(AR7240_GPIO_CLEAR, DIAG_GPIO_NO); _s_jiffies=jiffies; } while(0)
#define	DIAG_OFF()				do { ar7240_reg_wr(AR7240_GPIO_SET, DIAG_GPIO_NO); _s_jiffies=jiffies; } while(0)
#define	DIAG_BLINK()			do { _f_on = (_f_on+1) & 0x1; if (_f_on) DIAG_ON(); else DIAG_OFF(); } while(0)

#else	//defined  CONFIG_AR9100
#define	DIAG_GPIO_NO			(1<<1)
#define	DIAG_INIT(F_WRITE)		do {  } while(0)
#define	DIAG_ON()				do {  } while(0)
#define	DIAG_OFF()				do {  } while(0)
#define	DIAG_BLINK()			do {  } while(0)
#endif	//defined  CONFIG_AR9100
#endif	//CONFIG_BUFFALO


#define down mutex_lock
#define up mutex_unlock
#define init_MUTEX mutex_init
#define DECLARE_MUTEX(a) struct mutex a

static struct mutex ar7240_flash_sem;

/* GLOBAL FUNCTIONS */
void
ar7240_flash_spi_down(void)
{
	mutex_lock(&ar7240_flash_sem);
}

void
ar7240_flash_spi_up(void)
{
	mutex_unlock(&ar7240_flash_sem);
}

EXPORT_SYMBOL(ar7240_flash_spi_down);
EXPORT_SYMBOL(ar7240_flash_spi_up);

#ifdef	CONFIG_BUFFALO
u_int32_t	g_buffalo_clock_div	= 3;
u_int32_t	g_buffalo_current_div;

static	u_int32_t	ar7240_cpu_freq = 0, ar7240_ahb_freq, ar7240_ddr_freq;

static	void	get_ahb_clock(void)
{
    uint32_t pll, pll_div, ahb_div, ddr_div, freq, ref_div;

    if (ar7240_cpu_freq)
        return;

    pll = ar7240_reg_rd(AR7240_PLL_CONFIG);

    pll_div  = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK);
    ref_div  = (pll >> REF_DIV_SHIFT) & REF_DIV_MASK;
    ddr_div  = ((pll >> DDR_DIV_SHIFT) & DDR_DIV_MASK) + 1;
    ahb_div  = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1)*2;

    freq     = pll_div * ref_div * 5000000;

    ar7240_cpu_freq = freq;
    ar7240_ddr_freq = freq/ddr_div;
    ar7240_ahb_freq = ar7240_cpu_freq/ahb_div;
}

static void	buffalo_spi_clock_update(void)
{
	if (g_buffalo_clock_div != g_buffalo_current_div) {
		if (g_buffalo_clock_div>=0 && g_buffalo_clock_div<64) {
			u_int32_t	last_value;
			ar7240_flash_spi_down();


			ar7240_reg_wr(AR7240_SPI_FS, 1);

			last_value	= ar7240_reg_rd(AR7240_SPI_CLOCK) & 0x3f;

			//	write value
			ar7240_reg_wr_nf(AR7240_SPI_CLOCK, 0x40 | (g_buffalo_clock_div & 0x3f));
			g_buffalo_current_div	= ar7240_reg_rd(AR7240_SPI_CLOCK) & 0x3f;


#define		AR7240_SPI_CMD_WRITE_SR	0x1
			ar7240_spi_write_enable(0);
			ar7240_spi_bit_banger(0, AR7240_SPI_CMD_WRITE_SR);
			ar7240_spi_bit_banger(0, 0x0);
			ar7240_spi_go(0);
			ar7240_spi_poll(0);

			ar7240_reg_wr(AR7240_SPI_FS, 0);

#if	0
			{
				unsigned long flags;
				//	disable interrupts
				local_irq_save(flags);
				local_irq_disable();


				ar7240_reg_wr_nf(AR7240_PLL_CONFIG, (ar7240_reg_rd(AR7240_PLL_CONFIG) & (~((unsigned int)PLL_CONFIG_LOCKED_MASK))));
				ar7240_reg_wr_nf(AR7240_CPU_CLOCK_CONTROL, CLOCK_CONTROL_RST_SWITCH_MASK|CLOCK_CONTROL_CLOCK_SWITCH_MASK);
				ar7240_reg_rmw_clear(AR7240_CPU_CLOCK_CONTROL, CLOCK_CONTROL_RST_SWITCH_MASK);


				//	wait
				for ( ; 0==(ar7240_reg_rd(AR7240_PLL_CONFIG) & PLL_CONFIG_LOCKED_MASK) ; )
					;


				//	enable interrupts
				local_irq_enable();
				local_irq_restore(flags);
			}
#endif



			//	read-back
			printk("SPI clock changed %d --> %d\n", ar7240_ahb_freq / (2 << last_value), ar7240_ahb_freq / (2 << g_buffalo_current_div));

			ar7240_flash_spi_up();
		}
	}
}

//
//	for BUFFALO debug
//
static	int	atoi(const char *p)
{
	int		ret		= 0;
	int		minus	= 0;
	do	{
		if (p==NULL)
			break;

		for (; *p==' ' ; p++)
			;

		if (*p=='-') {
			minus	= 1;
			p++;
		}

		for (; p && *p>='0' && *p<='9' ; p++)
			ret	= (ret * 10) + (*p - '0');
	} while(0);

	return	(minus ? -ret : ret);
}


#endif	//CONFIG_BUFFALO

#define AR7240_FLASH_SIZE_2MB          (2*1024*1024)
#define AR7240_FLASH_SIZE_4MB          (4*1024*1024)
#define AR7240_FLASH_SIZE_8MB          (8*1024*1024)
#define AR7240_FLASH_SIZE_16MB          (16*1024*1024)
#define AR7240_FLASH_PG_SIZE_256B       256

#ifndef ST25P28
#define AR7240_FLASH_SECTOR_SIZE_64KB  (64*1024)
#else
#define AR7240_FLASH_SECTOR_SIZE_256KB  (256*1024)
#endif

#ifdef	CONFIG_BUFFALO
#define AR7240_FLASH_SIZE_12MB         (12*1024*1024)
#define AR7240_FLASH_SIZE_32MB         (32*1024*1024)
#endif	//CONFIG_BUFFALO

#define AR7240_FLASH_NAME               "ar7240-nor0"
/*
 * bank geometry
 */
#ifndef	CONFIG_BUFFALO
typedef struct ar7240_flash_geom {
    uint32_t     size;
    uint32_t     sector_size;
    uint32_t     nsectors;
    uint32_t     pgsize;
} ar7240_flash_geom_t; 

ar7240_flash_geom_t flash_geom_tbl[AR7240_FLASH_MAX_BANKS] = {
	{
		.size		= AR7240_FLASH_SIZE_8MB,
		.sector_size	= AR7240_FLASH_SECTOR_SIZE_64KB,
		.pgsize		= AR7240_FLASH_PG_SIZE_256B
	}
};
#else	//CONFIG_BUFFALO
typedef struct ar7240_flash_geom {
	const char *name;
	u_int8_t	 chip_id[3];
    uint32_t     size;
    uint32_t     sector_size;
    uint32_t     nsectors;
    uint32_t     pgsize;
}ar7240_flash_geom_t; 

ar7240_flash_geom_t g_flash_geom_tbl[] = {
					//	chip-id					dev-size	blk-siz		n-sec	page-siz
	  {	"S25FL064P",	{ 0x01, 0x02, 0x16 },	0x00800000,	0x10000,	   0,	 256	}		//S25FL064P
	, {	"M25P64V6P",	{ 0x20, 0x20, 0x17 },	0x00800000,	0x10000,	   0,	 256	}		//S25FL064P
	, {	"MX25L64-45E",	{ 0xc2, 0x20, 0x17 },	0x00800000,	0x10000,	   0,	 256	}		//MX25L64-45E
	, {	"MX25L128-45E",	{ 0xc2, 0x20, 0x18 },	0x01000000,	0x10000,	   0,	 256	}		//MX25L128-45E
	, {	"W25Q128BV",	{ 0xef, 0x40, 0x18 },	0x01000000,	0x10000,	   0,	 256	}		//W25Q128BV
	, {	"S25FL128P",	{ 0x01, 0x20, 0x18 },	0x01000000,	0x10000,	   0,	 256	}		//S25FL128P
	, {	NULL,			{ 0x00, 0x00, 0x00 },	0x00000000,	0x00000,	   0,	   0	}		//dummy
};

ar7240_flash_geom_t	g_flash_geoms[AR7240_FLASH_MAX_BANKS];


#endif	//CONFIG_BUFFALO

static struct mtd_partition dir_parts[] = {
      {name: "RedBoot", offset: 0, size:0x50000,},
      {name: "linux", offset: 0x60000, size:0x1f90000,},
      {name: "rootfs", offset: 0x0, size:0x2b0000,},
      {name: "ddwrt", offset: 0x0, size:0x2b0000,},
      {name: "nvram", offset: 0x3d0000, size:0x10000,},
      {name: "FIS directory", offset: 0x3e0000, size:0x10000,},
      {name: "board_config", offset: 0x3f0000, size:0x10000,},
      {name: "fullflash", offset: 0x3f0000, size:0x10000,},
      {name: "uboot-env", offset: 0x40000, size:0x10000,},
      {name:NULL,},
};


          
static int
ar7240_flash_probe(void)
{
	return 0;
}

static int
ar7240_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    int nsect, s_curr, s_last;
	uint64_t  res;


	if (instr->addr + instr->len > mtd->size)
		return (-EINVAL);

    ar7240_flash_spi_down();

#	ifdef	CONFIG_BUFFALO
	DIAG_INIT(0);
	_f_lastwrite	= 0;
#	endif	//CONFIG_BUFFALO

	res = instr->len;
	do_div(res, mtd->erasesize);
	nsect = res;

	if (((uint32_t)instr->len) % mtd->erasesize)
		nsect ++;

	res = instr->addr;
	do_div(res,mtd->erasesize);
	s_curr = res;

	s_last  = s_curr + nsect;

    do {
#	ifdef	CONFIG_BUFFALO
		DIAG_BLINK();
#	endif	//CONFIG_BUFFALO

        ar7240_spi_sector_erase(s_curr * AR7240_SPI_SECTOR_SIZE);
    } while (++s_curr < s_last);

    ar7240_spi_done();

#	ifdef	CONFIG_BUFFALO
	DIAG_OFF();
#	endif	//CONFIG_BUFFALO

    ar7240_flash_spi_up();

    if (instr->callback) {
	instr->state |= MTD_ERASE_DONE;
        instr->callback(instr);
    }



    return 0;
}

#ifndef	CONFIG_BUFFALO
static int
ar7240_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
                  size_t *retlen, u_char *buf)
{
    uint32_t addr = from | 0xbf000000;

	if (!len)
		return (0);
	if (from + len > mtd->size)
		return (-EINVAL);

    ar7240_flash_spi_down();

    memcpy(buf, (uint8_t *)(addr), len);
    *retlen = len;

    ar7240_flash_spi_up();

    return 0;
}
#else	//CONFIG_BUFFALO
static unsigned int	ar7240_spi_get_cs(unsigned int addr)
{
	if (g_flash_geoms[0].size) {
		return	(g_flash_geoms[0].size <= addr);
	} else {
		return	(g_flash_geoms[1].size);
	}
}

static int
ar7240_flash_read_dmc(unsigned int cs, void *buf, size_t len)
{
	u_int8_t	*data	= buf;
	size_t		i;
	ar7240_flash_spi_down();

	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_READ_DMC);
	ar7240_spi_delay_8(cs);
//	ar7240_spi_delay_8(cs);
//	ar7240_spi_delay_8(cs);

	for (i=0; i<len ; i++, data++) {
		ar7240_spi_delay_8(cs);
		*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
	}

	ar7240_spi_done();

	{
		u_int8_t	msg[256];
		size_t		msglen=0;
		for (i=0 ; i<len ; i++) {
			if ((i%16)==0) {
				if (msglen)	printk("%s\n", msg);
				memset(msg,0,sizeof(msg));
				msglen	= sprintf(msg, "%04X : %02X", i, data[i]);
			} else {
				msglen	+= sprintf(msg+msglen, " %02X", data[i]);
			}
		}
		if (msglen)	printk("%s\n", msg);
	}

	ar7240_flash_spi_up();

	return	0;
}

static int
ar7240_flash_read_manid(unsigned int cs, void *buf, size_t len)
{
	u_int8_t	*data	= buf;
	size_t		i;
	ar7240_flash_spi_down();

	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_READ_MANID);
	ar7240_spi_send_addr(cs,0);
//	ar7240_spi_delay_8(cs);

	for (i=0; i<len ; i++, data++) {
		ar7240_spi_delay_8(cs);
		*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
	}

	ar7240_spi_done();

	{
		u_int8_t	msg[256];
		size_t		msglen=0;
		for (i=0, data=buf ; i<len ; i++) {
			if ((i%16)==0) {
				if (msglen)	printk("%s\n", msg);
				memset(msg,0,sizeof(msg));
				msglen	= sprintf(msg, "%04X : %02X", i, data[i]);
			} else {
				msglen	+= sprintf(msg+msglen, " %02X", data[i]);
			}
		}
		if (msglen)	printk("%s\n", msg);
	}

	ar7240_flash_spi_up();

	return	0;
}

static int
ar7240_flash_read_id(unsigned int cs, void *buf, size_t len)
{
	u_int8_t	*data	= buf;
	size_t		i;
	ar7240_flash_spi_down();

	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_READ_ID);
//	ar7240_spi_delay_8(cs);
//	ar7240_spi_delay_8(cs);
//	ar7240_spi_delay_8(cs);

	for (i=0; i<len ; i++, data++) {
		ar7240_spi_delay_8(cs);
		*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
	}

	ar7240_spi_done();

	{
		u_int8_t	msg[256];
		size_t		msglen=0;
		for (data=buf, i=0 ; i<len ; i++) {
			if ((i%16)==0) {
				if (msglen)	printk("%s\n", msg);
				memset(msg,0,sizeof(msg));
				msglen	= sprintf(msg, "%04X : %02X", i, data[i]);
			} else {
				msglen	+= sprintf(msg+msglen, " %02X", data[i]);
			}
		}
		if (msglen)	printk("%s\n", msg);
	}

	ar7240_flash_spi_up();

	return	0;
}

static int
ar7240_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
                  size_t *retlen, u_char *buf)
{
    uint32_t addr0, addr1;
    size_t	len0, len1;

	if (!len)
		return (0);
	if (from + len > mtd->size)
		return (-EINVAL);


    ar7240_flash_spi_down();

    if (ar7240_spi_get_cs(from)) {
    	len0	= 0;
    	addr0	= 0;
    	len1	= len;
    	addr1	= (from - g_flash_geoms[0].size) | 0xbe000000;
    } else if (ar7240_spi_get_cs(from+len)) {
    	len0	= g_flash_geoms[0].size - from;
    	addr0	= from | 0xbf000000;
    	len1	= len - len0;
    	addr1	= 0xbe000000;
    } else {
    	len0	= len;
    	addr0	= from | 0xbf000000;
    	len1	= 0;
    	addr1	= 0;
    }
	if (len0)	memcpy(buf, (uint8_t *)(addr0), len0);
//	if (len1)	memcpy(buf+len0, (uint8_t *)(addr1), len1);
//	if (len0)	ar7240_spi_fast_read(0, addr0, buf, len0);
	if (len1)	ar7240_spi_fast_read(1, addr1, buf+len0, len1);
//	if (len0)	ar7240_spi_read(0, addr0, buf, len0);
//	if (len1)	ar7240_spi_read(1, addr1, buf+len0, len1);
//	if (len1) {
//		if (addr1 < 0xbe200000)	 ar7240_spi_read(1, addr1, buf+len0, len1);
//		else					 ar7240_spi_fast_read(1, addr1, buf+len0, len1);
//	}

    *retlen = len;

    ar7240_flash_spi_up();

    return 0;
}
#endif	//CONFIG_BUFFALO

static int
ar7240_flash_write(struct mtd_info *mtd, loff_t to, size_t len,
                    size_t *retlen, const u_char *buf)
{
    int total = 0, len_this_lp, bytes_this_page;
    uint32_t addr = 0;
    u_char *mem;


    ar7240_flash_spi_down();

#	ifdef	CONFIG_BUFFALO
	DIAG_INIT(1);
	_f_lastwrite	= 1;
#	endif	//CONFIG_BUFFALO

	while (total < len) {
		mem = (u_char *) (buf + total);
		addr = to + total;
		bytes_this_page =
		    AR7240_SPI_PAGE_SIZE - (addr % AR7240_SPI_PAGE_SIZE);
		len_this_lp = min(((int)len - total), bytes_this_page);

#		ifdef	CONFIG_BUFFALO
		if ( (addr&_S_LASTADDR_MASK) != _s_lastaddr) {
			DIAG_BLINK();
			_s_lastaddr	= addr&_S_LASTADDR_MASK;
		}
#		endif	//CONFIG_BUFFALO

        ar7240_spi_write_page(addr, mem, len_this_lp);
        total += len_this_lp;
    }

    ar7240_spi_done();

#	ifdef	CONFIG_BUFFALO
	DIAG_OFF();
#	endif	//CONFIG_BUFFALO

    ar7240_flash_spi_up();

    *retlen = len;


    return 0;
}


/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
static int __init ar7240_flash_init(void)
{
    int i, np;
    ar7240_flash_geom_t *geom;
    struct mtd_info *mtd;
    struct mtd_partition *mtd_parts;
    uint8_t index;
    
    mutex_init(&ar7240_flash_sem);

#ifdef	CONFIG_BUFFALO
    //	enable cs#1

	if (is_ar7242())
	{
		ar7240_reg_rmw_set(AR7240_GPIO_FUNCTIONS, (1 << 13));
	}else
	{
		ar7240_reg_rmw_set(AR7240_GPIO_FUNCTIONS, (1 << 12));
	}

#endif	//CONFIG_BUFFALO

#ifdef	CONFIG_BUFFALO
	get_ahb_clock();
//	buffalo_spi_clock_update();
#else	//CONFIG_BUFFALO
    ar7240_reg_wr_nf(AR7240_SPI_CLOCK, 0x43);
#endif	//CONFIG_BUFFALO


	printk("check spi banks %d\n", AR7240_FLASH_MAX_BANKS);


	for(i = 0; i < AR7240_FLASH_MAX_BANKS; i++) {
		u_int8_t	chip_id[3];
		int			j;

		struct {										//	offset
			u_int8_t	man_id;							//	00h
			u_int8_t	dev_id_0;						//	01h
			u_int8_t	dev_id_1;						//	02h
			u_int8_t	dev_id_2;						//	03h
			u_int8_t	pad1[0x10 - 0x04];				//	04h - 0fh
			u_int8_t	qry[3];							//	10h - 12h
			u_int8_t	cmdset_id[2];					//	13h - 14h
			u_int8_t	addr_table[2];					//	15h - 16h
			u_int8_t	oem_cmdset_id[2];				//	17h - 18h
			u_int8_t	oem_addr_table[2];				//	19h - 1ah
			u_int8_t	VccMin;							//	1bh
			u_int8_t	VccMax;							//	1ch
			u_int8_t	VppMin;							//	1dh
			u_int8_t	VppMax;							//	1eh
			u_int8_t	WordWriteTimeoutTyp;			//	1fh
			u_int8_t	BufWriteTimeoutTyp;				//	20h
			u_int8_t	BlockEraseTimeoutTyp;			//	21h
			u_int8_t	ChipEraseTimeoutTyp;			//	22h
			u_int8_t	WordWriteTimeoutMax;			//	23h
			u_int8_t	BufWriteTimeoutMax;				//	24h
			u_int8_t	BlockEraseTimeoutMax;			//	25h
			u_int8_t	ChipEraseTimeoutMax;			//	26h
			u_int8_t	DevSize;						//	27h
			u_int16_t	InterfaceDesc;					//	28h - 29h
			u_int16_t	MaxBufWriteSize;				//	2ah - 2bh
			u_int8_t	NumEraseRegions;				//	2ch
			struct {
				u_int8_t	nEraseNum;					//	2dh, 31h, 35h, 39h
				u_int8_t	nEraseSiz0;					//	2eh, 32h, 36h, 3ah
				u_int8_t	nEraseSiz1;					//	2fh, 33h, 37h, 3bh
				u_int8_t	nEraseSiz2;					//	30h, 34h, 38h, 3ch
			} EraseRegionInfo[4];
			u_int8_t	pad2[0x40 - 0x3d];				//	3dh - 3fh
			u_int8_t	pri[3];							//	40h - 42h
			u_int8_t	MajVer;							//	43h
			u_int8_t	MinVer;							//	44h
			u_int8_t	SupportUnlock;					//	45h
			u_int8_t	SupportEraseSuspend;			//	46h
			u_int8_t	SupportSectorProtect;			//	47h
			u_int8_t	SupportSectorUnprotect;			//	48h
			u_int8_t	TypeSectorProtect;				//	49h
			u_int8_t	TypeSimulaneousOpe;				//	4ah
			u_int8_t	TypeBurstMode;					//	4bh
			u_int8_t	TypePageMode;					//	4ch
			u_int8_t	AccSupplyMin;					//	4dh
			u_int8_t	AccSupplyMax;					//	4eh
			u_int8_t	WriteProtection;				//	4fh
			u_int8_t	SupportProgramSuspend;			//	50h
		} cfi_buf;

#if	0
		struct	{
			u_int32_t	dmc_signature;					//	00h - 03h	0x50444653
			u_int8_t	dmc_MinVer;
			u_int8_t	dmc_MajVer;
			u_int8_t	nHeader;						//	06h
			u_int8_t	pad1[1];						//	07h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h
			u_int8_t	;								//	00h

		} dmc_buf;
#endif

		memset(&chip_id[0], 0, sizeof(chip_id));
		ar7240_flash_read_id(i, &chip_id[0], sizeof(chip_id));

		for (j=0 ; j<sizeof(g_flash_geom_tbl)/sizeof(g_flash_geom_tbl[0]) ; j++) {
			if (memcmp(g_flash_geom_tbl[j].chip_id, chip_id, sizeof(g_flash_geom_tbl[j].chip_id))==0) {
				if (g_flash_geom_tbl[j].name) {
					printk("found %s device on bank#%d\n", g_flash_geom_tbl[j].name, i);
					g_flash_geoms[i]	= g_flash_geom_tbl[j];
					if (g_flash_geoms[i].sector_size!=0 && g_flash_geoms[i].nsectors==0)
						g_flash_geoms[i].nsectors	= g_flash_geoms[i].size / g_flash_geoms[i].sector_size;
					break;
				}
			}
		}

#if	0
		ar7240_flash_read_manid(i, &cfi_buf, sizeof(cfi_buf));
		memset(&cfi_buf, 0, sizeof(cfi_buf));
		ar7240_flash_read_id(i, &cfi_buf, sizeof(cfi_buf));
		if (cfi_buf.qry[0]=='Q' && cfi_buf.qry[1]=='R' && cfi_buf.qry[2]=='Y') {
			g_flash_geoms[i].size			= 2 << cfi_buf.DevSize;
			g_flash_geoms[i].sector_size	= 64 * 1024;
			g_flash_geoms[i].nsectors		= g_flash_geoms[i].size / g_flash_geoms[i].sector_size;
			g_flash_geoms[i].pgsize			= 2 << cfi_buf.MaxBufWriteSize;
		}
#endif
	}
	printk("SPI flash size total:%d Mbytes\n", (g_flash_geoms[0].size + g_flash_geoms[1].size) >> 20);

	mtd         =  kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!mtd) {
		printk("Cant allocate mtd stuff\n");
		return -1;
	}
	memset(mtd, 0, sizeof(struct mtd_info));


	mtd->name               =   AR7240_FLASH_NAME;
	mtd->type               =   MTD_NORFLASH;
	mtd->flags              =   (MTD_CAP_NORFLASH|MTD_WRITEABLE); 
	mtd->size               =   g_flash_geoms[0].size + g_flash_geoms[1].size;
	mtd->erasesize          =   g_flash_geoms[0].sector_size;
	mtd->numeraseregions    =   0;
	mtd->eraseregions       =   NULL;
	mtd->owner              =   THIS_MODULE;
	mtd->_erase              =   ar7240_flash_erase;
	mtd->_read               =   ar7240_flash_read;
	mtd->_write              =   ar7240_flash_write;
	mtd->writesize          =   1;
	mtd->priv				= g_flash_geoms;

	int offset = 0;
	struct squashfs_super_block *sb;

	char buf[512];
	int retlen;
	unsigned int rootsize,len;

		while ((offset + mtd->erasesize) < mtd->size) {
			mtd_read(mtd,offset,512,&retlen,&buf[0]);
			if (*((__u32 *)buf) == SQUASHFS_MAGIC_SWAP) {
				printk(KERN_INFO "\nfound squashfs at %X\n",offset);
				sb = (struct squashfs_super_block *)buf;
				dir_parts[2].offset = offset;

				dir_parts[2].size = le64_to_cpu(sb->bytes_used);
				len = dir_parts[2].offset + dir_parts[2].size;
				len += (mtd->erasesize - 1);
				len &= ~(mtd->erasesize - 1);
				dir_parts[2].size = (len & 0x1ffffff) - dir_parts[2].offset;
				dir_parts[3].offset = dir_parts[2].offset + dir_parts[2].size;
				dir_parts[6].offset = mtd->size - mtd->erasesize;	// board config
				dir_parts[6].size = mtd->erasesize;
				dir_parts[5].offset = dir_parts[6].offset;	//fis config
				dir_parts[5].size = mtd->erasesize;
				dir_parts[4].offset = dir_parts[5].offset - mtd->erasesize;	//nvram
				dir_parts[4].size = mtd->erasesize;
				dir_parts[3].size = dir_parts[4].offset - dir_parts[3].offset;
				rootsize = dir_parts[4].offset - offset;	//size of rootfs aligned to nvram offset
				dir_parts[1].size = (dir_parts[2].offset -dir_parts[1].offset) + rootsize;
				break;
			}
		offset+=4096;
		}
		dir_parts[7].offset = 0;	// linux + nvram = phy size
		dir_parts[7].size = mtd->size;	// linux + nvram = phy size
		add_mtd_partitions(mtd, dir_parts, 9);

    return 0;
}

static void __exit ar7240_flash_exit(void)
{
    /*
     * nothing to do
     */
}


/*
 * Primitives to implement flash operations
 */
#ifdef	CONFIG_BUFFALO
static void
ar7240_spi_write_enable(unsigned int cs)  
{
    ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
    ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_WREN);             
    ar7240_spi_go(cs);
}

static void
ar7240_spi_read(unsigned int cs, uint32_t addr, uint8_t *data, int len)
{

	if (cs) {
#if	1
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
		for (; len>0 ; len--, addr++, data++) {
			ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
			ar7240_spi_bit_banger_1(0x3);        
			ar7240_spi_send_addr(cs,addr);
			ar7240_spi_delay_8(cs);
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
		}
		ar7240_spi_done();
#else
#if	0
		//	同じ結果
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
        for (; len>0 ; len--, data++) {
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
	        ar7240_spi_bit_banger_1(0x3);        
			ar7240_spi_send_addr(cs,addr);
			ar7240_spi_delay_8(cs);
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
        }
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
		ar7240_spi_done();
#else
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
        ar7240_spi_bit_banger_1(0x3);        
		ar7240_spi_send_addr(cs,addr);
		ar7240_spi_delay_8(cs);
        for (; len>0 ; len--, data++) {
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
        }
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
		ar7240_spi_done();
#endif
#endif
	} else {
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
        ar7240_spi_bit_banger_0(0x3);        
		ar7240_spi_send_addr(cs,addr);
		ar7240_spi_delay_8(cs);
        for (; len>0 ; len--, data++) {
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
        }
		ar7240_spi_done();
	}
}

static void
ar7240_spi_fast_read(unsigned int cs, uint32_t addr, uint8_t *data, int len)
{

	if (cs) {
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
		ar7240_spi_bit_banger_1(AR7240_SPI_CMD_FAST_READ);        
		ar7240_spi_send_addr(cs,addr);
		ar7240_spi_delay_8(cs);
		for (; len>0 ; len--, data++) {
			ar7240_spi_delay_8(cs);
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
		}
		ar7240_spi_done();
#if		0		//OK pattern
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
		for (; len>0 ; len--, addr++, data++) {
			ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
			ar7240_spi_bit_banger_1(AR7240_SPI_CMD_FAST_READ);        
			ar7240_spi_send_addr(cs,addr);
			ar7240_spi_delay_8(cs);
			ar7240_spi_delay_8(cs);
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
		}
		ar7240_spi_done();
#endif	//CONFIG_BUFFALO

	} else {
		ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
		ar7240_spi_bit_banger_0(AR7240_SPI_CMD_FAST_READ);        
		ar7240_spi_send_addr(cs,addr);
		ar7240_spi_delay_8(cs);
		for (; len>0 ; len--, data++) {
			ar7240_spi_delay_8(cs);
			*data	= (uint8_t)ar7240_reg_rd(AR7240_SPI_RD_STATUS);
		}
		ar7240_spi_done();
	}
}

static void
ar7240_spi_poll(unsigned int cs)   
{
    int rd;                                                 

	if (cs) {
	    do {
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
	        ar7240_spi_bit_banger_1(AR7240_SPI_CMD_RD_STATUS);        
	        ar7240_spi_delay_8(cs);
	        rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);               
	    }while(rd);
	} else {
	    do {
	        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
	        ar7240_spi_bit_banger_0(AR7240_SPI_CMD_RD_STATUS);        
	        ar7240_spi_delay_8(cs);
	        rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);               
	    }while(rd);
	}
}

static void
ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
    int i;
    uint8_t ch;
    unsigned int cs = ar7240_spi_get_cs(addr);

    ar7240_spi_write_enable(cs);
    ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_PAGE_PROG);
    ar7240_spi_send_addr(cs,addr);

    if (cs) {
	    for(i = 0; i < len; i++) {
	        ch = *(data + i);
	        ar7240_spi_bit_banger_1(ch);
	    }
    } else {
	    for(i = 0; i < len; i++) {
	        ch = *(data + i);
	        ar7240_spi_bit_banger_0(ch);
	    }
    }


    ar7240_spi_go(cs);
    ar7240_spi_poll(cs);
}

static void
ar7240_spi_sector_erase(uint32_t addr)
{
    unsigned int cs = ar7240_spi_get_cs(addr);
    ar7240_spi_write_enable(cs);
    ar7240_spi_bit_banger(cs,AR7240_SPI_CMD_SECTOR_ERASE);
    ar7240_spi_send_addr(cs,addr);
    ar7240_spi_go(cs);
    ar7240_spi_poll(cs);
}
#else	//CONFIG_BUFFALO
static void
ar7240_spi_write_enable()  
{
    ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
    ar7240_spi_bit_banger(AR7240_SPI_CMD_WREN);             
    ar7240_spi_go();
}

static void
ar7240_spi_poll()   
{
    int rd;                                                 

    do {
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
        ar7240_spi_bit_banger(AR7240_SPI_CMD_RD_STATUS);        
        ar7240_spi_delay_8();
        rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);               
    }while(rd);
}

static void
ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
    int i;
    uint8_t ch;

    ar7240_spi_write_enable();
    ar7240_spi_bit_banger(AR7240_SPI_CMD_PAGE_PROG);
    ar7240_spi_send_addr(addr);

    for(i = 0; i < len; i++) {
        ch = *(data + i);
        ar7240_spi_bit_banger(ch);
    }

    ar7240_spi_go();
    ar7240_spi_poll();
}

static void
ar7240_spi_sector_erase(uint32_t addr)
{
	ar7240_spi_write_enable();
	ar7240_spi_bit_banger(AR7240_SPI_CMD_SECTOR_ERASE);
	ar7240_spi_send_addr(addr);
	ar7240_spi_go();
#if 0
	/*
	 * Do not touch the GPIO's unnecessarily. Might conflict
	 * with customer's settings.
	 */
	display(0x7d);
#endif
	ar7240_spi_poll();
}
#endif	//CONFIG_BUFFALO

module_init(ar7240_flash_init);
module_exit(ar7240_flash_exit);
