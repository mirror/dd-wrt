#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <asm/bootinfo.h>
#include <asm/rb/boards.h>
#include <asm/rb/rb400.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;

#define GPIO_BASE	0x18040000
#define GPIO_OE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_O1_REG	    0x0008
#define GPIO_S1_REG	    0x000c
#define GPIO_S0_REG	    0x0010
#define GPIO_FN_REG	    0x0028

#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

static void __iomem *gpio_base;
static int no_latch = 0;
static int has_tiny = 0;
static void nolatch_select_chip(struct mtd_info *mtd, int chipnr);

/* ------------------ RB700 nand driver -------------------- */

#define NAND_tDS	20
#define NAND_tDH	10
#define NAND_tWP	25
#define NAND_tWH	15

#define NAND_tREA	30
#define NAND_tRP	25
#define NAND_tREH	15

#define   GPO_RB700_NAND_nCE	(1 << 11)
#define   GPI_RB700_NAND_RDY	(1 << 12)
#define   GPO_RB700_NAND_CLE	(1 << 14)
#define   GPO_RB700_NAND_ALE	(1 << 15)
#define   GPO_RB700_NAND_nRE	(1 << 16)
#define   GPO_RB700_NAND_nWE	(1 << 17)

#define   GPO_NAND_DATA(x) ((x) << 1)
#define   GPI_NAND_DATA(x) ((x) >> 1)

#define GPFN_RB700_JTAG_DISABLE	(1 << 0)
#define GPFN_RB700_UART_EN	((1 << 1) | (1 << 15))
#define GPFN_RB700_ETH_LEDS_EN	(0x1f << 3)
#define GPFN_RB700_SPI_EN	(1 << 18)
#define GPFN_RB700_SPI_CS2_EN	(1 << 14)

static unsigned gpo_beepled = 0;
static unsigned gpo_beepled_mask = 0;

void rb700_beepled(int on) {
	if (gpo_beepled_mask == 0) return;
	if (on) {
		gpo_beepled = gpo_beepled_mask;
	}
	else {
		gpo_beepled = 0;
	}
	rb700_change_gpo(0, 0);
}
EXPORT_SYMBOL(rb700_beepled);

int rb700_change_gpo(unsigned _off, unsigned _on) {
	static unsigned on = (GPO_RB700_LATCH_EN |
			      GPO_RB700_nULED |
			      GPO_RB700_nLINK0 |
			      GPO_RB700_nLINK1 |
			      GPO_RB700_nLINK2 |
			      GPO_RB700_nLINK3 |
			      GPO_RB700_nLINK4);
	static unsigned off = 0;
	static unsigned oe = 0;
	static DEFINE_SPINLOCK(lock);
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);

	on = (on | _on) & ~_off;
	off = (off | _off) & ~_on;
	if (!oe) oe = GPIO_REG(GPIO_OE_REG);

	if (no_latch) {
		if ((on ^ gpo_beepled) & GPO_RB700_nULED) {
			GPIO_REG(GPIO_OE_REG) &= ~GPO_RB700_NOLATCH_nULED;
		}
		else {
			GPIO_REG(GPIO_OE_REG) |= GPO_RB700_NOLATCH_nULED;
			GPIO_REG(GPIO_S0_REG) = GPO_RB700_NOLATCH_nULED;
		}
		GPIO_REG(GPIO_S1_REG) = on & (GPO_RB700_MON_SEL |
					      GPO_RB700_USB_nPWROFF);
		GPIO_REG(GPIO_S0_REG) = off & (GPO_RB700_MON_SEL |
					       GPO_RB700_USB_nPWROFF);
	}
	else if (on & GPO_RB700_LATCH_EN) {
		GPIO_REG(GPIO_OE_REG) |= oe | _on | _off;
		GPIO_REG(GPIO_S0_REG) = off ^ gpo_beepled;
		GPIO_REG(GPIO_S1_REG) = on ^ gpo_beepled;
	}
	else if (_off & GPO_RB700_LATCH_EN) {
		oe = GPIO_REG(GPIO_OE_REG);
		GPIO_REG(GPIO_S0_REG) = GPO_RB700_LATCH_EN;
		GPIO_REG(GPIO_S0_REG);			/* flush */
	}

	spin_unlock_irqrestore(&lock, flags);
	return 1;
}
EXPORT_SYMBOL(rb700_change_gpo);

static void rb700_gpio_init(void) {
	int o1 = GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_S1_REG) = (GPO_RB700_NAND_nCE |
				 GPO_RB700_NAND_nWE |
				 GPO_RB700_NAND_nRE);
	GPIO_REG(GPIO_S0_REG) = (GPO_RB700_NAND_CLE |
				 GPO_RB700_NAND_ALE);

	GPIO_REG(GPIO_OE_REG) &= ~(GPI_RB700_NAND_RDY |
				   GPO_NAND_DATA(0xff));
	GPIO_REG(GPIO_OE_REG) |= (GPO_RB700_NAND_CLE |
				  GPO_RB700_NAND_ALE |
				  GPO_RB700_NAND_nCE |
				  GPO_RB700_NAND_nWE |
				  GPO_RB700_NAND_nRE);
	if (mips_machtype == MACH_MT_RB711R3
	    || mips_machtype == MACH_MT_RB711G
		) {
		gpo_beepled_mask &= ~GPO_RB700_nLINK4;
		rb700_change_gpo(GPO_RB700_nLINK4, 0);
	}
	/* keep power led the same as it came from BIOS */
	rb700_change_gpo((~o1) & GPO_RB700_nPLED,
			 o1 & GPO_RB700_nPLED);
}

static int rb700_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPI_RB700_NAND_RDY;
}

static void rb700_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned gpo;

	GPIO_REG(GPIO_OE_REG) |= GPO_NAND_DATA(0xff);
	gpo = GPIO_REG(GPIO_O1_REG) & ~(GPO_RB700_NAND_nWE |
					GPO_NAND_DATA(0xff));
	/* XXX: any GPIO output val change in parallel (for led) may get lost */

	for (i = 0; i < cnt; ++i) {
		unsigned val = data[i];
		unsigned gpov = gpo | GPO_NAND_DATA(val);

		GPIO_REG(GPIO_O1_REG) = gpov;
		GPIO_REG(GPIO_O1_REG);
//		ndelay(max(NAND_tDS, NAND_tWP));

		GPIO_REG(GPIO_O1_REG) = gpov | GPO_RB700_NAND_nWE;
		GPIO_REG(GPIO_O1_REG);
//		ndelay(max(NAND_tDH, NAND_tWH));
	}

	GPIO_REG(GPIO_OE_REG) &= ~GPO_NAND_DATA(0xff);
	GPIO_REG(GPIO_OE_REG);
}

static int rb700_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {
	int i;
	for (i = 0; i < cnt; ++i) {
		uint8_t val;

		GPIO_REG(GPIO_S0_REG) = GPO_RB700_NAND_nRE;
		GPIO_REG(GPIO_S0_REG);
//		ndelay(NAND_tREA);

		val = GPI_NAND_DATA(GPIO_REG(GPIO_IN_REG));

//		if (NAND_tRP > NAND_tREA) ndelay(NAND_tRP - NAND_tREA);
		GPIO_REG(GPIO_S1_REG) = GPO_RB700_NAND_nRE;
//		GPIO_REG(GPIO_S1_REG);	/* XXX: works without this */
//		ndelay(NAND_tREH);

		if (data) {
			data[i] = val;
		}
		else if (verify) {
			if (verify[i] != val) return -EFAULT;
		}
	}
	return 0;
}

static void rb700_select_chip(struct mtd_info *mtd, int chipnr)
{
	unsigned fn = GPIO_REG(GPIO_FN_REG);
	if (chipnr >= 0) {
		/* enable NAND functionality */
		rb700_change_gpo(GPO_RB700_LATCH_EN, 0);
		fn |= GPFN_RB700_JTAG_DISABLE;
		fn &= ~(GPFN_RB700_ETH_LEDS_EN |
			GPFN_RB700_SPI_EN |
			GPFN_RB700_SPI_CS2_EN);
		GPIO_REG(GPIO_FN_REG) = fn;
		GPIO_REG(GPIO_FN_REG);
		GPIO_REG(GPIO_OE_REG) &= ~(GPI_RB700_NAND_RDY |
					   GPO_NAND_DATA(0xff));
		GPIO_REG(GPIO_S1_REG) = GPO_RB700_NAND_nRE | GPO_RB700_NAND_nWE;
		GPIO_REG(GPIO_S1_REG);

		/* nCE -> low */
		GPIO_REG(GPIO_S0_REG) = GPO_RB700_NAND_nCE;
	}
	else {
		/* nCE -> high */
		GPIO_REG(GPIO_S1_REG) = GPO_RB700_NAND_nCE;
		GPIO_REG(GPIO_S1_REG);

		/* disable NAND functionality */
		GPIO_REG(GPIO_OE_REG) |= (GPO_RB700_nULED | GPO_RB700_nPLED);
		fn &= ~GPFN_RB700_JTAG_DISABLE;
		fn |= GPFN_RB700_SPI_EN;
		if (has_tiny) fn |= GPFN_RB700_SPI_CS2_EN;
		GPIO_REG(GPIO_FN_REG) = fn;
		GPIO_REG(GPIO_FN_REG);
		rb700_change_gpo(0, GPO_RB700_LATCH_EN);
	}
}

static void rb700_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned gpo = GPIO_REG(GPIO_O1_REG);
		gpo &= ~(GPO_RB700_NAND_ALE |
			 GPO_RB700_NAND_CLE);

		if (ctrl & NAND_CLE) {
			gpo |= GPO_RB700_NAND_CLE;
		}
		if (ctrl & NAND_ALE) {
			gpo |= GPO_RB700_NAND_ALE;
		}
		GPIO_REG(GPIO_O1_REG) = gpo;
		GPIO_REG(GPIO_O1_REG);		/* flush */
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		rb700_write_bytes_gpio(&data, 1);
	}
}

static uint8_t rb700_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	rb700_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void rb700_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	rb700_write_bytes_gpio(buf, len);
}

static void rb700_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	rb700_read_bytes_gpio(buf, len, NULL);
}

#ifdef DEBUG_SPEED
static void rb700_test_speed(void) {
	char buf[1024];
	unsigned long ticks;
	unsigned kb;

	rb700_select_chip(NULL, 0);
	GPIO_REG(GPIO_S1_REG) = GPO_RB700_NAND_nCE;
	GPIO_REG(GPIO_S1_REG);

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		rb700_read_bytes_gpio(buf, 1024, NULL);
		++kb;
	}
	printk("read speed is %u KB/s\n", kb * 10);

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		rb700_write_bytes_gpio(buf, 1024);
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
	rb700_select_chip(NULL, -1);
}
#endif

static int rb700_nand_probe(struct platform_device *pdev)
{
	unsigned long options = (unsigned long)pdev->dev.platform_data;
	printk("RB700 nand %lu\n", options);
	no_latch = options & 1;
	has_tiny = options & 2;
	gpo_beepled_mask = (GPO_RB700_nULED |
			    GPO_RB700_nLINK0 |
			    GPO_RB700_nLINK1 |
			    GPO_RB700_nLINK2 |
			    GPO_RB700_nLINK3 |
			    GPO_RB700_nLINK4);
	rb700_gpio_init();
#ifdef DEBUG_SPEED
	rb700_test_speed();
#endif
	memset(&rnand, 0, sizeof(rnand));

	rnand.select_chip = no_latch ? nolatch_select_chip : rb700_select_chip;
	rnand.cmd_ctrl = rb700_hwcontrol;
	rnand.dev_ready = rb700_dev_ready;
	rnand.read_byte = rb700_read_byte;
	rnand.write_buf = rb700_write_buf;
	rnand.read_buf = rb700_read_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver rb700_nand_driver = {
	.probe	= rb700_nand_probe,
	.driver	= {
		.name = "rb700-nand",
		.owner = THIS_MODULE,
	},
};

/* ---------------- support for RB700 nand without latch ------------------ */

#define   GPO_NOLATCH_NAND_nCE	(1 << 0)

static void nolatch_select_chip(struct mtd_info *mtd, int chipnr)
{
	unsigned fn = GPIO_REG(GPIO_FN_REG);
	if (chipnr >= 0) {
		/* nCE -> low */
		GPIO_REG(GPIO_S0_REG) = GPO_NOLATCH_NAND_nCE;
		GPIO_REG(GPIO_S0_REG);

		/* enable NAND functionality */
		fn |= GPFN_RB700_JTAG_DISABLE;
		fn &= ~(GPFN_RB700_ETH_LEDS_EN |
			GPFN_RB700_SPI_EN |
			GPFN_RB700_SPI_CS2_EN);
		GPIO_REG(GPIO_FN_REG) = fn;
		GPIO_REG(GPIO_OE_REG) &= ~(GPI_RB700_NAND_RDY |
					   GPO_NAND_DATA(0xff));
	}
	else {
		/* disable NAND functionality */
		fn &= ~GPFN_RB700_JTAG_DISABLE;
		fn |= GPFN_RB700_SPI_EN | GPFN_RB700_SPI_CS2_EN;
		GPIO_REG(GPIO_FN_REG) = fn;

		/* nCE -> high */
		GPIO_REG(GPIO_S1_REG) = GPO_NOLATCH_NAND_nCE;
		GPIO_REG(GPIO_S1_REG);
	}
}

/* ------------------ common init/exit code -------------------- */

static int __init rb700_nand_init(void)
{
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	return platform_driver_register(&rb700_nand_driver);
}

static void __exit rb700_nand_exit(void)
{
	iounmap(gpio_base);

	platform_driver_unregister(&rb700_nand_driver);
}

module_init(rb700_nand_init);
module_exit(rb700_nand_exit);
