#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <asm/rb/rb400.h>

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static struct nand_chip rnand;

static struct spi_device *spi = 0;

#define GPIO_BASE	0x18040000
#define GPIO_OE_REG	    0x0000
#define GPIO_IN_REG	    0x0004
#define GPIO_O1_REG	    0x0008
#define GPIO_S1_REG	    0x000c
#define GPIO_S0_REG	    0x0010
#define GPIO_FN_REG	    0x0028

#define GPIO_REG(x)	(*(volatile unsigned *)((unsigned) gpio_base + (x)))

static void __iomem *gpio_base;

/* ------------------ RB400 SPI nand driver -------------------- */

#define   GPI_NAND_RDY		(1 << 5)

static int rb400_dev_ready(struct mtd_info *mtd) {
	return GPIO_REG(GPIO_IN_REG) & GPI_NAND_RDY;
}

static void rb400_nandio_write(const uint8_t *data, unsigned cnt) {
	static const uint8_t cmd = SPI_CMD_WRITE_NAND;
	struct spi_transfer t[3] = {
		{
			.tx_buf = &cmd,
			.len = 1,
		},
		{
			.tx_buf = data,
			.len = cnt,
			.fast_write = 1,
		},
		{
			.len = 1,
			.fast_write = 1,
		},
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	spi_message_add_tail(&t[2], &m);
	spi_sync(spi, &m);
}

static int rb400_nandio_read_verify_slow(uint8_t *rdata, const uint8_t *vdata,
					 unsigned cnt) {
	static const uint8_t cmd[2] = { SPI_CMD_READ_NAND, 0 };
	struct spi_transfer t[2] = {
		{
			.tx_buf = &cmd,
			.len = 2,
		},
		{
			.tx_buf = vdata,
			.rx_buf = rdata,
			.len = cnt,
			.verify = (vdata != NULL),
		},
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	return spi_sync(spi, &m);
}

static int rb400_nandio_read_verify(uint8_t *rdata, const uint8_t *vdata,
				    unsigned cnt) {
	unsigned size32 = cnt & ~31;
	if (size32) {
		unsigned addr = SPI_READ_NAND_OFS;
		if (rb400_spiflash_read_verify(addr, rdata, vdata, size32)) {
			return -1;
		}
		cnt -= size32;
		if (cnt == 0) return 0;
		if (rdata) rdata += size32;
		if (vdata) vdata += size32;
	}
	return rb400_nandio_read_verify_slow(rdata, vdata, cnt);
}

static void rb400_hwcontrol(struct mtd_info *mtd, int cmd,
			    unsigned int ctrl) {
	static uint8_t data[8];
	static unsigned dlen = 0;

	if (ctrl & NAND_CTRL_CHANGE) {
		uint8_t cfg = 0;

		if (ctrl & NAND_CLE) {
			cfg |= CFG_BIT_CLE;
		}
		if (ctrl & NAND_ALE) {
			cfg |= CFG_BIT_ALE;
		}
		if (!(ctrl & NAND_NCE)) {
			cfg |= CFG_BIT_nCE;
		}
		if (dlen) {
			rb400_nandio_write(data, dlen);
			dlen = 0;
		}
		rb400_change_cfg(CFG_BIT_nCE | CFG_BIT_CLE | CFG_BIT_ALE,
				 cfg);
	}

	if (cmd != NAND_CMD_NONE) {
		data[dlen] = cmd;
		++dlen;
		if (dlen == sizeof(data)) {
			rb400_nandio_write(data, dlen);
			dlen = 0;
		}
	}
}

static uint8_t rb400_read_byte(struct mtd_info *mtd)
{
	uint8_t byte = 0;
	rb400_nandio_read_verify(&byte, NULL, 1);
	return byte;
}

static void rb400_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	rb400_nandio_write(buf, len);
}

static void rb400_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	rb400_nandio_read_verify(buf, NULL, len);
}

static int rb400_spi_nand_probe(struct spi_device *_spi)
{
	printk("RB400 spi nand\n");
	memset(&rnand, 0, sizeof(rnand));

	if (!gpio_base) {
		gpio_base = ioremap_nocache(GPIO_BASE, PAGE_SIZE);
		if (!gpio_base)
			return -ENOMEM;
	}

	GPIO_REG(GPIO_OE_REG) &= ~GPI_NAND_RDY;

	spi = _spi;

	rnand.cmd_ctrl = rb400_hwcontrol;
	rnand.dev_ready = rb400_dev_ready;
	rnand.read_byte = rb400_read_byte;
	rnand.write_buf = rb400_write_buf;
	rnand.read_buf = rb400_read_buf;

	return rb_nand_probe(&rnand, 1);
}

static struct spi_driver rb400_spi_nand_driver = {
	.probe	= rb400_spi_nand_probe,
	.driver	= {
		.name = "rb400-spi-nand",
		.owner = THIS_MODULE,
	},
};

/* ------------------ common init/exit code -------------------- */

static int __init rb400_nand_init(void)
{
	return spi_register_driver(&rb400_spi_nand_driver);
}

static void __exit rb400_nand_exit(void)
{
	if (gpio_base)
		iounmap(gpio_base);

	spi_unregister_driver(&rb400_spi_nand_driver);
}

module_init(rb400_nand_init);
module_exit(rb400_nand_exit);
