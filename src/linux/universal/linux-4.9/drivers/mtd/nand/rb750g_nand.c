#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>

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

/* ------------------ RB750G nand driver -------------------- */


#define GPO_NAND_nCE		(1 << 0)
#define GPO_NAND_READ		(1 << 1)
#define GPO_NAND_ALE		(1 << 2)
#define GPO_NAND_CLE		(1 << 3)
#define GPIO_NAND_RDY		(1 << 5)
#define GPO_SSR_STRB		(1 << 6)
#define GPO_NAND_nRW		(1 << 8)
#define GPO_LATCH_EN		(1 << 11)

#define GPO_NAND_DATA(x) ((x) & 0xff)
#define GPI_NAND_DATA(x) ((x) & 0xff)

#define ENABLE_GPIO0_SPI	0x01000
#define ENABLE_GPIO1_SPI	0x02000
#define ENABLE_GPIO1_I2S	0x20000

static void rb750g_gpio_init(void) {
	GPIO_REG(GPIO_S1_REG) = (GPO_NAND_nCE
				 | GPO_NAND_nRW
				 | GPO_SSR_STRB
				 | GPO_LATCH_EN);
	GPIO_REG(GPIO_S0_REG) = (GPO_NAND_READ 
				 | GPO_NAND_CLE
				 | GPO_NAND_ALE);
	GPIO_REG(GPIO_OE_REG) |= (GPO_NAND_nCE
				  | GPO_NAND_READ 
				  | GPO_NAND_CLE 
				  | GPO_NAND_ALE 
				  | GPO_NAND_nRW 
				  | GPO_SSR_STRB
				  | GPO_LATCH_EN);
	GPIO_REG(GPIO_FN_REG) &= ~(ENABLE_GPIO1_I2S |
				   ENABLE_GPIO1_SPI |
				   ENABLE_GPIO0_SPI);
}

static int rb750g_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPIO_NAND_RDY;
}

static void rb750g_write_bytes_gpio(const uint8_t *data, unsigned cnt) {
	int i;
	unsigned gpo;
	unsigned gpo_orig = GPIO_REG(GPIO_O1_REG);
	
	GPIO_REG(GPIO_S0_REG) = GPO_NAND_READ;
	GPIO_REG(GPIO_S0_REG);
	GPIO_REG(GPIO_S0_REG) = GPO_LATCH_EN;
	GPIO_REG(GPIO_S0_REG);
	GPIO_REG(GPIO_OE_REG) |= GPO_NAND_DATA(0xff);
	gpo = GPIO_REG(GPIO_O1_REG) & ~(GPO_NAND_nRW | GPO_NAND_DATA(0xff));

	for (i = 0; i < cnt; ++i) {
		unsigned val = data[i];
		unsigned gpov = gpo | GPO_NAND_DATA(val);

		GPIO_REG(GPIO_O1_REG) = gpov;
		GPIO_REG(GPIO_O1_REG);
		GPIO_REG(GPIO_O1_REG) = gpov | GPO_NAND_nRW;
		GPIO_REG(GPIO_O1_REG);
	}
	
	GPIO_REG(GPIO_OE_REG) &= ~GPIO_NAND_RDY;
	GPIO_REG(GPIO_O1_REG) = gpo_orig;
	GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_S1_REG) = GPO_LATCH_EN;
	GPIO_REG(GPIO_S1_REG);
}

static int rb750g_read_bytes_gpio(uint8_t *data, unsigned cnt,
				  const uint8_t *verify) {

	int i;
	int ret = 0;
	unsigned gpo_orig = GPIO_REG(GPIO_O1_REG);

	GPIO_REG(GPIO_S1_REG) = GPO_NAND_READ;
	GPIO_REG(GPIO_S1_REG);
	GPIO_REG(GPIO_S0_REG) = GPO_LATCH_EN;
	GPIO_REG(GPIO_S0_REG);
	GPIO_REG(GPIO_OE_REG) &= ~GPO_NAND_DATA(0xff);
	
	for (i = 0; i < cnt; ++i) {
		unsigned char val;
		GPIO_REG(GPIO_S0_REG) = GPO_NAND_nRW;
		GPIO_REG(GPIO_S0_REG);
		
		val = GPI_NAND_DATA(GPIO_REG(GPIO_IN_REG));
		GPIO_REG(GPIO_S1_REG) = GPO_NAND_nRW;
		GPIO_REG(GPIO_S1_REG);

		if (data) {
			data[i] = val;
		}
		else if (verify) {
			if (verify[i] != val) {
				ret = -EFAULT;
				break;
			}
		}
	}	

	GPIO_REG(GPIO_OE_REG) |= (GPO_NAND_DATA(0xff) ^ GPIO_NAND_RDY);
	GPIO_REG(GPIO_O1_REG) = gpo_orig;
	GPIO_REG(GPIO_O1_REG);
	GPIO_REG(GPIO_S1_REG) = GPO_LATCH_EN;
	GPIO_REG(GPIO_S1_REG);
	return ret;
}

static void rb750g_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl) {
	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned gpo = GPIO_REG(GPIO_O1_REG);
		gpo &= ~(GPO_NAND_ALE | GPO_NAND_CLE | GPO_NAND_nCE);

		if (ctrl & NAND_CLE) {
			gpo |= GPO_NAND_CLE;
		}
		if (ctrl & NAND_ALE) {
			gpo |= GPO_NAND_ALE;
		}
		if (!(ctrl & NAND_NCE)) {
			gpo |= GPO_NAND_nCE;
		}
		GPIO_REG(GPIO_O1_REG) = gpo;
		GPIO_REG(GPIO_O1_REG);		/* flush */
	}

	if (cmd != NAND_CMD_NONE) {
		uint8_t data = cmd;
		rb750g_write_bytes_gpio(&data, 1);
	}
}

static uint8_t rb750g_read_byte(struct mtd_info *mtd)
{
	uint8_t data;
	rb750g_read_bytes_gpio(&data, 1, NULL);
	return data;
}

static void rb750g_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	rb750g_write_bytes_gpio(buf, len);
}

static void rb750g_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	rb750g_read_bytes_gpio(buf, len, NULL);
}

static int rb750g_nand_probe(struct platform_device *pdev)
{
	printk("RB750G nand\n");
	rb750g_gpio_init();
	memset(&rnand, 0, sizeof(rnand));

	rnand.cmd_ctrl = rb750g_hwcontrol;
	rnand.dev_ready = rb750g_dev_ready;
	rnand.read_byte = rb750g_read_byte;
	rnand.write_buf = rb750g_write_buf;
	rnand.read_buf = rb750g_read_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct platform_driver rb750g_nand_driver = {
	.probe	= rb750g_nand_probe,
	.driver	= {
		.name = "rb750g-nand",
		.owner = THIS_MODULE,
	},
};

/* ------------------ common init/exit code -------------------- */

static int __init rb750g_nand_init(void)
{
	gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	return platform_driver_register(&rb750g_nand_driver);
}

static void __exit rb750g_nand_exit(void)
{
	iounmap(gpio_base);

	platform_driver_unregister(&rb750g_nand_driver);
}

module_init(rb750g_nand_init);
module_exit(rb750g_nand_exit);
