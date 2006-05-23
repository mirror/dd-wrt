#include <linux/kernel.h>	/* printk() */
#include <linux/module.h>	/* module to be loadable */
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/ioport.h>	/* request_mem_region() */
#include <asm/io.h>		/* ioremap() */
#include <asm/bootinfo.h>
#include <asm/rc32434/dev.h>
#include <asm/rc32434/gpio.h>
#include <asm/rb/rb100.h>

#include "ata.h"

#define REQUEST_MEM_REGION 0
#define DEBUG 0
#define USE_APM	0

#if DEBUG
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#define RC32434_CF_GPIO_NUM	13
#define RC32434_CF_GPIO_BIT	(1 << RC32434_CF_GPIO_NUM)
#define RC32434_IRQ_CFRDY	(8 + 4 * 32 + RC32434_CF_GPIO_NUM)    /* 149 */
static GPIO_t rcgpio = (GPIO_t)GPIO_VirtualAddress;

/* CFRDY is connected to GPIO4/INTX_1 */
#define ADM5120_CF_GPIO_NUM	4
#define ADM5120_CF_IRQ_LEVEL_BIT	0x20	/* GPIO4 = 0x20, GPIO2 = 0x10 */
#define ADM5120_IRQ_CFRDY	INT_LVL_EXTIO_1

static unsigned irq_cfrdy = 0;


#define SECS	1000000		/* unit for wait_not_busy() is 1us */

unsigned long cfdev_base = 0;

unsigned cf_head = 0;
unsigned cf_cyl = 0;
unsigned cf_spt = 0;
unsigned cf_sectors = 0;
static unsigned cf_block_size = 1;

static void *baddr = 0;

#define DBUF ((volatile void *)((unsigned long)baddr | ATA_DBUF_OFFSET))

enum trans_state {
	TS_IDLE,
	TS_AFTER_RESET,
	TS_READY,
	TS_CMD,
	TS_TRANS
};

/* local state */
static enum trans_state tstate = TS_IDLE;
static char *tbuf;
static unsigned tbuf_size;
static unsigned tlba_offset;
static unsigned tsector_count;
static unsigned tsectors_left;
static int tread;
static unsigned tcmd;

static int async_mode = 0;
static unsigned long irq_enable_time;
#if DEBUG
static unsigned long busy_time;
#endif

static void cf_do_tasklet(unsigned long unused);
DECLARE_TASKLET(cf_tasklet, cf_do_tasklet, 0);

static struct timer_list timeout_timer;

static int (*cfrdy)(void) = 0;
static void (*prepare_cf_irq)(void) = 0;
static volatile U32 *ilevel_addr = 0;
static unsigned ilevel_bit = 0;

static void (*read_sector)(void *dst) = 0;
static void (*write_sector)(const void *src) = 0;

static inline void
wareg(u8 val, unsigned reg)
{
	writeb(val, (unsigned long) baddr | ATA_REG_OFFSET | reg);
}

static inline u8
rareg(unsigned reg)
{
	return readb((unsigned long) baddr | ATA_REG_OFFSET | reg);
}

static int
rc32434_cfrdy(void)
{
	return rcgpio->gpiod & RC32434_CF_GPIO_BIT;
}

static int
adm5120_cfrdy(void)
{
	return ADM5120_SW_REG(GPIO_conf0_REG) & GPIO4_INPUT_MASK;
}

static void
rc32434_prepare_cf_irq(void)
{
	/* interrupt on cf ready (not busy) */
	rcgpio->gpioilevel |= RC32434_CF_GPIO_BIT;
	/* clear interrupt status */	
	rcgpio->gpioistat &= ~RC32434_CF_GPIO_BIT;
}

static void
adm5120_prepare_cf_irq(void)
{
	/* interrupt on cf ready (not busy) */
	ADM5120_INTC_REG(IRQ_LEVEL_REG) |= ADM5120_CF_IRQ_LEVEL_BIT;
	// FIXME: how to clear interrupt status?
}

static int
init_cfdev_base(void)
{
		printk("RB500 CF\n");
		DEV_t devt = (DEV_t)DEV_VirtualAddress;

		if (devt->dev[1].devmask == 0) {
			printk(KERN_ERR
			       "DEV1 in Device Controller"
			       " is not mapped anywhere!\n");
			return 0;
		}
		/* setup CFRDY GPIO as input */
		rcgpio->gpiofunc &= ~RC32434_CF_GPIO_BIT;
		rcgpio->gpiocfg &= ~RC32434_CF_GPIO_BIT;

		cfdev_base = devt->dev[1].devbase;
		cfrdy = &rc32434_cfrdy;
		prepare_cf_irq = &rc32434_prepare_cf_irq;
		irq_cfrdy = RC32434_IRQ_CFRDY;
		ilevel_addr = &rcgpio->gpioilevel;
		ilevel_bit = RC32434_CF_GPIO_BIT;

		printk(KERN_INFO
		       "DC DEV1 base 0x%08lx ctrl 0x%08x tctrl 0x%08x"
		       " cfrdy %d\n",
		       cfdev_base,
		       devt->dev[1].devc,
		       devt->dev[1].devtc,
		       (*cfrdy)());
	return 1;
}

static int
cf_present(void)
{
	/* TODO: read and configure CIS into memory mapped mode
	 * TODO:   parse CISTPL_CONFIG on CF+ cards to get base address (0x200)
	 */

	int i;
	for (i = 0; i < 0x10; ++i) {
		if (rareg(i) != 0xff)
			return 1;
	}
	return 0;
}

extern inline int
is_busy(void)
{
	return rareg(ATA_REG_ST) & ATA_REG_ST_BUSY;
}

static int
wait_not_busy(int to_us, int wait_for_busy)
{
	int us_passed = 0;
	int false_cfrdy = 0;
	if (wait_for_busy && !is_busy()) {
		/* busy must appear within 400ns,
		 * but it may dissapear before we see it
		 *  => must not wait for busy in a loop
		 */
		ndelay(400);
	}

	do {
		if (us_passed)
			udelay(1);	/* never reached in async mode */
		if (!is_busy()) {
#if !DEBUG
			if (us_passed > 1 * SECS) {
#endif
				printk(KERN_WARNING
				       "cf-mips:   not busy ok (after %dus)"
				       ", status 0x%02x\n",
				       us_passed, (unsigned) rareg(ATA_REG_ST));
#if !DEBUG
			}
#endif
			return CF_TRANS_OK;
		}
		if (us_passed == 1 * SECS) {
			printk(KERN_WARNING "cf-mips: wait not busy %dus..\n",
			       to_us);
		}
		if (async_mode) {
			timeout_timer.expires = jiffies + (to_us * HZ / SECS);
			irq_enable_time = jiffies;
			(*prepare_cf_irq)();
			if (!(*cfrdy)()) {
				add_timer(&timeout_timer);
				enable_irq(irq_cfrdy);
				return CF_TRANS_IN_PROGRESS;
			}
			++false_cfrdy;
			if (false_cfrdy > 10) {
				printk(KERN_WARNING
				       "cf-mips: cfrdy() does not work"
				       ", using sync mode\n");
				async_mode = 0;
				free_irq(irq_cfrdy, NULL);
			}
			continue;
		}
		++us_passed;
	} while (us_passed < to_us);

	printk(KERN_ERR "cf-mips:  wait not busy timeout (%dus)"
	       ", status 0x%02x, state %d\n",
	       to_us, (unsigned) rareg(ATA_REG_ST), tstate);
	return CF_TRANS_FAILED;
}

static void
cf_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	/* While tasklet has not disabled irq, irq will be retried all the time
	 * because of ILEVEL matching GPIO pin status => deadlock.
	 * To avoid this, we change ILEVEL to 0.
	 */
	if (!(*cfrdy)()) return;	// false interrupt (only for ADM5120)
	*ilevel_addr &= ~ilevel_bit;

#if DEBUG
	busy_time = jiffies - irq_enable_time;
#endif
	del_timer(&timeout_timer);
	tasklet_schedule(&cf_tasklet);
}

static int
do_reset(void)
{
	printk(KERN_INFO "cf-mips: resetting..\n");

	wareg(ATA_REG_DC_SRST, ATA_REG_DC);
	udelay(1);		/* FIXME: how long should we wait here? */
	wareg(0, ATA_REG_DC);

	return wait_not_busy(30 * SECS, 1);
}

static int
set_multiple(void)
{
	if (cf_block_size <= 1)
		return CF_TRANS_OK;

	wareg(cf_block_size, ATA_REG_SC);
	wareg(ATA_REG_DH_BASE | ATA_REG_DH_LBA, ATA_REG_DH);
	wareg(ATA_CMD_SET_MULTIPLE, ATA_REG_CMD);

	return wait_not_busy(10 * SECS, 1);
}

static int
set_cmd(void)
{
	DEBUGP(KERN_INFO "cf-mips: ata cmd 0x%02x\n", tcmd);

	if (tsector_count) {
		wareg(tsector_count & 0xff, ATA_REG_SC);
		wareg(tlba_offset & 0xff, ATA_REG_SN);
		wareg((tlba_offset >> 8) & 0xff, ATA_REG_CL);
		wareg((tlba_offset >> 16) & 0xff, ATA_REG_CH);
	}
	wareg(((tlba_offset >> 24) & 0x0f) | ATA_REG_DH_BASE | ATA_REG_DH_LBA,
	      ATA_REG_DH);	/* select drive on all commands */
	wareg(tcmd, ATA_REG_CMD);
	return wait_not_busy(10 * SECS, 1);
}

static void
read_sector4(void *dst) {
	unsigned tcnt = CF_SECT_SIZE / 4;
	u32 *qbuf = (u32 *)dst;
	do {
		*qbuf = *(volatile u32 *)DBUF;
		++qbuf;
		--tcnt;
	} while (tcnt);
}

static void
read_sector2(void *dst) {
	unsigned tcnt = CF_SECT_SIZE / 2;
	u16 *qbuf = (u16 *)dst;
	do {
		*qbuf = *(volatile u16 *)DBUF;
		++qbuf;
		--tcnt;
	} while (tcnt);
}

static void
read_sector1(void *dst) {
	unsigned tcnt = CF_SECT_SIZE;
	u8 *qbuf = (u8 *)dst;
	do {
		*qbuf = *(volatile u8 *)DBUF;
		++qbuf;
		--tcnt;
	} while (tcnt);
}

static void
write_sector4(const void *src) {
	unsigned tcnt = CF_SECT_SIZE / 4;
	const u32 *qbuf = (const u32 *)src;
	do {
		*(volatile u32 *)DBUF = *qbuf;
		++qbuf;
		--tcnt;
	} while (tcnt);
}

static void
write_sector2(const void *src) {
	unsigned tcnt = CF_SECT_SIZE / 2;
	const u16 *qbuf = (const u16 *)src;
	do {
		*(volatile u16 *)DBUF = *qbuf;
		++qbuf;
		--tcnt;
	} while (tcnt);
}

static void
write_sector1(const void *src) {
	unsigned tcnt = CF_SECT_SIZE / 1;
	const u8 *qbuf = (const u8 *)src;
	do {
		*(volatile u8 *)DBUF = *qbuf;
		++qbuf;
		--tcnt;
	} while (tcnt);
}


static int
do_trans(void)
{
	int res;
	unsigned st;

	while (tsectors_left) {
		static int transfered;
		transfered = 0;

		st = rareg(ATA_REG_ST);
		if (!(st & ATA_REG_ST_DRQ)) {
			printk(KERN_ERR
			       "cf-mips: do_trans without DRQ (status 0x%x)!\n",
			       st);
			if (st & ATA_REG_ST_ERR) {
				int errId = rareg(ATA_REG_ERR);
				printk(KERN_ERR
				       "cf-mips: %s error, status 0x%x, errid 0x%x\n",
				       (tread ? "read" : "write"), st, errId);
			}
			return CF_TRANS_FAILED;
		}
		do {
			if (tbuf_size == 0)
				tbuf = cf_get_next_buf(&tbuf_size);
			if (tread)
				(*read_sector)(tbuf);
			else
				(*write_sector)(tbuf);

			--tsectors_left;
			tbuf += CF_SECT_SIZE;
			tbuf_size -= CF_SECT_SIZE;
			++transfered;
		} while (transfered != cf_block_size && tsectors_left > 0);

		res = wait_not_busy(10 * SECS, 1);
		if (res != CF_TRANS_OK)
			return res;
	};

	st = rareg(ATA_REG_ST);
	if (st & (ATA_REG_ST_DRQ | ATA_REG_ST_DWF | ATA_REG_ST_ERR)) {
		if (st & ATA_REG_ST_DRQ) {
			printk(KERN_ERR
			       "cf-mips: DRQ after all %d sectors are %s"
			       ", status 0x%x\n",
			       tsector_count, (tread ? "read" : "written"), st);
		} else if (st & ATA_REG_ST_DWF) {
			printk(KERN_ERR "cf-mips: write fault, status 0x%x\n",
			       st);
		} else {
			int errId = rareg(ATA_REG_ERR);
			printk(KERN_ERR
			       "cf-mips: %s error, status 0x%x, errid 0x%x\n",
			       (tread ? "read" : "write"), st, errId);
		}
		return CF_TRANS_FAILED;
	}
	return CF_TRANS_OK;
}

static int
cf_do_state(void)
{
	int res;
	switch (tstate) {	/* fall through everywhere */
	case TS_IDLE:
		tstate = TS_READY;
		if (is_busy()) {
			tstate = TS_AFTER_RESET;
			res = do_reset();
			if (res != CF_TRANS_OK)
				break;
		}
	case TS_AFTER_RESET:
		if (tstate == TS_AFTER_RESET) {
			tstate = TS_READY;
			res = set_multiple();
			if (res != CF_TRANS_OK)
				break;
		}
	case TS_READY:
		tstate = TS_CMD;
		res = set_cmd();
		if (res != CF_TRANS_OK)
			break;;
	case TS_CMD:
		tstate = TS_TRANS;
	case TS_TRANS:
		res = do_trans();
		break;
	default:
		printk(KERN_ERR "cf-mips: BUG: unknown tstate %d\n", tstate);
		return CF_TRANS_FAILED;
	}
	if (res != CF_TRANS_IN_PROGRESS)
		tstate = TS_IDLE;
	return res;
}

static void
cf_do_tasklet(unsigned long unused)
{
	int res;

	if (tstate == TS_IDLE)
		return;		/* can happen when irq is first registered */
	disable_irq(irq_cfrdy);

#if DEBUG
	DEBUGP(KERN_WARNING
	       "cf-mips:   not busy ok (after %lucs), status 0x%02x\n",
	       busy_time, (unsigned) rareg(ATA_REG_ST));
#endif

	res = cf_do_state();
	if (res == CF_TRANS_IN_PROGRESS)
		return;
	cf_async_trans_done(res);
}

static void
cf_async_timeout(unsigned long unused)
{
	disable_irq(irq_cfrdy);
	printk(KERN_ERR "cf-mips:  wait not busy timeout (%lucs)"
	       ", status 0x%02x, state %d\n",
	       jiffies - irq_enable_time, (unsigned) rareg(ATA_REG_ST), tstate);
	tstate = TS_IDLE;
	cf_async_trans_done(CF_TRANS_FAILED);
}

int
cf_do_transfer(char *buf, unsigned buf_size, unsigned lba_offset,
	       unsigned sector_count, int cmd_read)
{
	tbuf = buf;
	tbuf_size = buf_size;
	tlba_offset = lba_offset;
	tsector_count = sector_count;
	tsectors_left = tsector_count;
	tread = cmd_read;
	tcmd = (cf_block_size == 1 ?
		(tread ? ATA_CMD_READ_SECTORS : ATA_CMD_WRITE_SECTORS) :
		(tread ? ATA_CMD_READ_MULTIPLE : ATA_CMD_WRITE_MULTIPLE));

	if (tstate != TS_IDLE) {
		printk(KERN_ERR "cf-mips: BUG: tstate %d at cf_transfer\n",
		       tstate);
		tstate = TS_IDLE;
	}
	if (tsector_count > ATA_MAX_SECT_PER_CMD) {
		printk(KERN_WARNING "cf-mips: sector count %u out of range\n",
		       tsector_count);
		return CF_TRANS_FAILED;
	}
	if (tlba_offset + tsector_count > cf_sectors) {
		printk(KERN_WARNING "cf-mips: lba offset %u out of range\n",
		       tlba_offset);
		return CF_TRANS_FAILED;
	}
	return cf_do_state();
}

static int
do_testread(void)
{
	unsigned data_size;
	DEBUGP(KERN_INFO "cf-mips: test read..\n");

	data_size = 4;
	while (data_size > 0) {
		unsigned st;
		unsigned i;
		u8 buf[CF_SECT_SIZE];

		tlba_offset = 0;
		tsector_count = 0;
		tcmd = ATA_CMD_IDENTIFY_DRIVE;
		if (set_cmd() != CF_TRANS_OK)
			return 0;

		st = rareg(ATA_REG_ST);
		if (!(st & ATA_REG_ST_DRQ)) {
			printk(KERN_ERR
			       "cf-mips: do_trans without DRQ (status 0x%x)!\n",
			       st);
			if (st & ATA_REG_ST_ERR) {
				int errId = rareg(ATA_REG_ERR);
				printk(KERN_ERR
				       "cf-mips: %s error, status 0x%x, errid 0x%x\n",
				       (tread ? "read" : "write"), st, errId);
			}
			return 0;
		}

		for (i = 0 ; i < CF_SECT_SIZE; i += data_size) {
			st = rareg(ATA_REG_ST);
			if (!(st & ATA_REG_ST_DRQ)) break;

			switch(data_size) {
			case 4:
				*(u32 *)(&buf[i]) = *(volatile u32 *)DBUF;
				break;
			case 2:
				*(u16 *)(&buf[i]) = *(volatile u16 *)DBUF;
				break;
			case 1:
				buf[i] = *(volatile u8 *)DBUF;
				break;
			}
		}
		int bad_addr_line = 7;
		for (i = 0 ; i < CF_SECT_SIZE; i += 2) {
			if (buf[i] != buf[i + 1]) {
				bad_addr_line &= ~1;
			}
			if ((i & 3) == 0 &&
			    *(u16 *)(&buf[i]) != *(u16 *)(&buf[i + 2])) {
				bad_addr_line &= ~2;
			}
			if ((i & 7) == 0 &&
			    *(u32 *)(&buf[i]) != *(u32 *)(&buf[i + 4])) {
				bad_addr_line &= ~4;
			}
		}
		if (i < CF_SECT_SIZE) {
			printk("cf-mips: width %d bytes failed: "
			       "DRQ lost too fast (after %d bytes)\n",
			       data_size, i);
		}
		else if ((rareg(ATA_REG_ST) & ATA_REG_ST_DRQ) != 0) {
			printk("cf-mips: width %d bytes failed: "
			       "DRQ lasts too long\n",
			       data_size);
		}
		else if (bad_addr_line) {
			printk("cf-mips: width %d bytes failed: "
			       "bad addr line %d\n",
			       data_size, bad_addr_line);
		}
		else {
			printk(KERN_INFO "cf-mips: using data width %d bytes\n",
			       data_size);
			switch(data_size) {
			case 4:
				read_sector = &read_sector4;
				write_sector = &write_sector4;
				break;
			case 2:
				read_sector = &read_sector2;
				write_sector = &write_sector2;
				break;
			case 1:
				read_sector = &read_sector1;
				write_sector = &write_sector1;
				break;
			}
			return 1;
		}
		if (do_reset() != CF_TRANS_OK) {
			printk(KERN_ERR "cf-mips: cf reset failed\n");
			return 0;
		}
		data_size >>= 1;
	}
	printk(KERN_ERR "cf-mips: all data width settings failed!\n");
	return 0;
}

static int
do_identify(void)
{
	u16 sbuf[CF_SECT_SIZE >> 1];
	int res;

	tbuf = (char *) sbuf;
	tbuf_size = CF_SECT_SIZE;
	tlba_offset = 0;
	tsector_count = 0;
	tsectors_left = 1;
	tread = 1;
	tcmd = ATA_CMD_IDENTIFY_DRIVE;

	DEBUGP(KERN_INFO "cf-mips: identify drive..\n");
	res = cf_do_state();
	if (res == CF_TRANS_IN_PROGRESS) {
		printk(KERN_ERR "cf-mips: BUG: async identify cmd\n");
		return CF_TRANS_FAILED;
	}
	if (res != CF_TRANS_OK)
		return 0;

	cf_head = sbuf[3];
	cf_cyl = sbuf[1];
	cf_spt = sbuf[6];
	cf_sectors = ((unsigned) sbuf[7] << 16) | sbuf[8];

	printk(KERN_INFO
	       "cf-mips: %s detected, C/H/S=%d/%d/%d sectors=%u (%uMB)\n",
	       (sbuf[0] == 0x848A ? "CF card" : "ATA drive"), cf_cyl, cf_head,
	       cf_spt, cf_sectors, cf_sectors >> 11);
	return 1;
}

#if USE_APM
static int
set_apm_level(unsigned level)
{
	if (level == ATA_APM_DISABLED) {
		wareg(ATA_FEATURE_DISABLE_APM, ATA_REG_FEAT);
	} else {
		wareg(ATA_FEATURE_ENABLE_APM, ATA_REG_FEAT);
		wareg(level, ATA_REG_SC);
	}
	wareg(ATA_REG_DH_BASE | ATA_REG_DH_LBA, ATA_REG_DH);
	wareg(ATA_CMD_SET_FEATURES, ATA_REG_CMD);

	if (wait_not_busy(10 * SECS, 1) != CF_TRANS_OK) {
		printk(KERN_ERR
		       "cf-mips: failed to set APM level %u: busy!\n", level);
		return 0;
	}
	if ((rareg(ATA_REG_ST) & ATA_REG_ST_ERR) == 0) {
		DEBUGP(KERN_INFO "cf-mips: changed APM level to %u\n", level);
		return 1;
	} else {
		printk(KERN_WARNING "cf-mips: failed to change APM level to %u"
		       ", status 0x%02x, error 0x%02x\n",
		       level, (unsigned) rareg(ATA_REG_ST),
		       (unsigned) rareg(ATA_REG_ERR));
		return 1;
	}
}
#endif

static void
init_multiple(void)
{
	DEBUGP(KERN_INFO "cf-mips: detecting block size\n");
	int res;

	cf_block_size = 128;	/* max block size = 128 sectors (64KB) */
	do {
		wareg(cf_block_size, ATA_REG_SC);
		wareg(ATA_REG_DH_BASE | ATA_REG_DH_LBA, ATA_REG_DH);
		wareg(ATA_CMD_SET_MULTIPLE, ATA_REG_CMD);

		res = wait_not_busy(10 * SECS, 1);
		if (res != CF_TRANS_OK) {
			printk(KERN_ERR
			       "cf-mips: failed to detect block size: busy!\n");
			cf_block_size = 1;
			return;
		}
		if ((rareg(ATA_REG_ST) & ATA_REG_ST_ERR) == 0)
			break;
		cf_block_size /= 2;
	} while (cf_block_size > 1);

	printk(KERN_INFO "cf-mips: multiple sectors = %d\n", cf_block_size);
}

int
cf_init(void)
{
	if (!init_cfdev_base())
		return -EINVAL;

#if REQUEST_MEM_REGION
	if (!request_mem_region(cfdev_base, CFDEV_BUF_SIZE, "cf-mips")) {
	     printk(KERN_WARNING
		    "cf-mips: cf_init: cf mem region 0x%x/0x%x"
		    " is already in use\n",
		    cfdev_base, CFDEV_BUF_SIZE);
	     return -EBUSY;
	}
#endif
	baddr = ioremap_nocache(cfdev_base, CFDEV_BUF_SIZE);
	if (!baddr) {
		printk(KERN_ERR
		       "cf-mips: cf_init: ioremap for (%lx,%x) failed\n",
		       cfdev_base, CFDEV_BUF_SIZE);
		return -EBUSY;
	}

	if (!cf_present()) {
		printk(KERN_WARNING "cf-mips: cf card not present\n");
		iounmap(baddr);
		return -ENODEV;
	}

	if (do_reset() != CF_TRANS_OK) {
		printk(KERN_ERR "cf-mips: cf reset failed\n");
		iounmap(baddr);
		return -EBUSY;
	}

	if (!do_testread()) {
		printk(KERN_ERR "cf-mips: cf test read failed\n");
		iounmap(baddr);
		return -EBUSY;
	}

	if (!do_identify()) {
		printk(KERN_ERR "cf-mips: cf identify failed\n");
		iounmap(baddr);
		return -EBUSY;
	}

#if USE_APM
	set_apm_level(ATA_APM_WITH_STANDBY);
#endif
	init_multiple();

	if (!is_busy() && !(*cfrdy)()) {
		printk(KERN_WARNING
		       "cf-mips: cfrdy() does not work, using sync mode\n");
	}
	else {
		init_timer(&timeout_timer);
		timeout_timer.function = cf_async_timeout;
		timeout_timer.data = 0;

		(*prepare_cf_irq)();
		if (request_irq(irq_cfrdy, cf_irq_handler, 0,
				"CF Mips", NULL)) {
			printk(KERN_ERR "cf-mips: failed to get irq\n");
			iounmap(baddr);
			return -EBUSY;
		}
		disable_irq(irq_cfrdy);
		async_mode = 1;
	}

	return 0;
}

void
cf_cleanup(void)
{
	iounmap(baddr);
	if (async_mode) {
		free_irq(irq_cfrdy, NULL);
	}
#if REQUEST_MEM_REGION
	release_mem_region(cfdev_base, CFDEV_BUF_SIZE);
#endif
}
