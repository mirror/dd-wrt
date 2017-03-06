#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <asm/rb/rb400.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;

#define GPIO_BASE	0x18040000
#define GPIO_OE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_O1_REG	    0x0008
#define GPIO_S1_REG	    0x000c
#define GPIO_S0_REG	    0x0010

#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

static void __iomem *gpio_base;

/* ------------------ AR9330 nand driver -------------------- */

#define   GPO_AR9330_NAND_nCE	(1 << 1)
#define   GPO_AR9330_NAND_CLE	(1 << 11)
#define   GPO_AR9330_NAND_ALE	(1 << 12)
#define   GPO_AR9330_NAND_nWE	(1 << 18)
#define   GPO_AR9330_NAND_nRE	(1 << 19)
#define   GPI_AR9330_NAND_RDY	(1 << 24)

#define   GPO_NAND_DATA(x)	((((x) & 0x1f) << 20) | (((x) & 0xe0) << 21))
#define   GPI_NAND_DATA(x)	((((x) >> 20) & 0x1f) | (((x) >> 21) & 0xe0))

static int ar9330_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPI_AR9330_NAND_RDY;
}

static void ar9330_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned gpo;

	GPIO_REG(GPIO_OE_REG) |= GPO_NAND_DATA(0xff);
	gpo = GPIO_REG(GPIO_O1_REG) & ~(GPO_AR9330_NAND_nWE |
					GPO_NAND_DATA(0xff));
	/* XXX: any GPIO output val change in parallel (for led) may get lost */

	for (i = 0; i < cnt; ++i) {
		unsigned val = data[i];
		unsigned gpov = gpo | GPO_NAND_DATA(val);

		GPIO_REG(GPIO_O1_REG) = gpov;
		GPIO_REG(GPIO_S1_REG) = GPO_AR9330_NAND_nWE;
	}

	GPIO_REG(GPIO_OE_REG) &= ~GPO_NAND_DATA(0xff);
}

static int ar9330_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {
	int i;
	for (i = 0; i < cnt; ++i) {
		uint8_t val;

		GPIO_REG(GPIO_S0_REG) = GPO_AR9330_NAND_nRE;
		GPIO_REG(GPIO_S0_REG);

		val = GPI_NAND_DATA(GPIO_REG(GPIO_IN_REG));

		GPIO_REG(GPIO_S1_REG) = GPO_AR9330_NAND_nRE;

		if (data) {
			data[i] = val;
		}
		else if (verify) {
			if (verify[i] != val) return -EFAULT;
		}
	}
	return 0;
}

static void ar9330_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned gpo = 0;
		if (!(ctrl & NAND_NCE)) {
			gpo |= GPO_AR9330_NAND_nCE;
		}
		if (ctrl & NAND_CLE) {
			gpo |= GPO_AR9330_NAND_CLE;
		}
		if (ctrl & NAND_ALE) {
			gpo |= GPO_AR9330_NAND_ALE;
		}
		GPIO_REG(GPIO_S1_REG) = gpo;
		GPIO_REG(GPIO_S0_REG) = gpo ^ (GPO_AR9330_NAND_nCE |
					       GPO_AR9330_NAND_ALE |
					       GPO_AR9330_NAND_CLE);
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		ar9330_write_bytes_gpio(&data, 1);
	}
}

static uint8_t ar9330_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	ar9330_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void ar9330_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	ar9330_write_bytes_gpio(buf, len);
}

static void ar9330_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	ar9330_read_bytes_gpio(buf, len, NULL);
}


#ifdef DEBUG_SPEED
static void ar9330_test_speed(void) {
	char buf[1024];
	unsigned long ticks;
	unsigned kb;

	GPIO_REG(GPIO_S1_REG) = GPO_AR9330_NAND_nCE;
	GPIO_REG(GPIO_S1_REG);

	/* wait for "start of" clock tick */
	kb = 0;
	ticks = jiffies;
	while (ticks == jiffies)
		/* nothing */;
	ticks = jiffies + HZ / 10;

	while ((long)(jiffies - ticks) < 0) {
		ar9330_read_bytes_gpio(buf, 1024, NULL);
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
		ar9330_write_bytes_gpio(buf, 1024);
		++kb;
	}
	printk("write speed is %u KB/s\n", kb * 10);
}
#endif

static int ar9330_nand_probe(struct platform_device *pdev)
{
	printk("AR9330 nand\n");
#ifdef DEBUG_SPEED
	ar9330_test_speed();
#endif
	memset(&rnand, 0, sizeof(rnand));

	rnand.cmd_ctrl = ar9330_hwcontrol;
	rnand.dev_ready = ar9330_dev_ready;
	rnand.read_byte = ar9330_read_byte;
	rnand.write_buf = ar9330_write_buf;
	rnand.read_buf = ar9330_read_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver ar9330_nand_driver = {
	.probe	= ar9330_nand_probe,
	.driver	= {
		.name = "ar9330-nand",
		.owner = THIS_MODULE,
	},
};

/* ------------------ common init/exit code -------------------- */

static int __init ar9330_nand_init(void)
{
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	return platform_driver_register(&ar9330_nand_driver);
}

static void __exit ar9330_nand_exit(void)
{
	iounmap(gpio_base);

	platform_driver_unregister(&ar9330_nand_driver);
}

module_init(ar9330_nand_init);
module_exit(ar9330_nand_exit);
