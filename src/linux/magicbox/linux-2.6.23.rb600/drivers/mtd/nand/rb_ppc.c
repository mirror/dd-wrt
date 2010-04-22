#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/of_platform.h>
#include <asm/of_device.h>
#include <linux/delay.h>
#include <asm/io.h>

static int common_probe(void);

static struct mtd_info rmtd;
static struct nand_chip rnand;

struct rbppc_info {
	void *gpi;
	void *gpo;
	void *localbus;

	unsigned gpio_rdy;
	unsigned gpio_nce;
	unsigned gpio_cle;
	unsigned gpio_ale;
	unsigned gpio_ctrls;
};

static int rbppc_nand_ready(struct mtd_info *mtd) {
	struct nand_chip *chip = mtd->priv;
	struct rbppc_info *priv = chip->priv;

	return in_be32(priv->gpi) & priv->gpio_rdy;
}

static void rbppc_nand_hwcontrol(struct mtd_info *mtd,
				 int cmd, unsigned int ctrl) {
	struct nand_chip *chip = mtd->priv;
	struct rbppc_info *priv = chip->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned val = in_be32(priv->gpo);
		if (!(val & priv->gpio_nce)) {
			/* make sure Local Bus has done NAND operations */
			readb(priv->localbus);
		}

		if (ctrl & NAND_CLE) {
			val |= priv->gpio_cle;
		} else {
			val &= ~priv->gpio_cle;
		}
		if (ctrl & NAND_ALE) {
			val |= priv->gpio_ale;
		} else {
			val &= ~priv->gpio_ale;
		}
		if (!(ctrl & NAND_NCE)) {
			val |= priv->gpio_nce;
		} else {
			val &= ~priv->gpio_nce;
		}
		out_be32(priv->gpo, val);

		/* make sure GPIO output has changed */
		val ^= in_be32(priv->gpo);
		if (val & priv->gpio_ctrls) {
			printk(KERN_ERR
			       "NAND GPO change failed 0x%08x\n", val);
		}
	}

	if (cmd != NAND_CMD_NONE) writeb(cmd, chip->IO_ADDR_W);
}

static void rbppc_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	memcpy(buf, chip->IO_ADDR_R, len);
}

static int rbppc_nand_probe(struct of_device *pdev,
			    const struct of_device_id *match)
{
	struct device_node *gpio;
	struct device_node *nnand;
	struct resource res;
	struct rbppc_info *info;
	void *baddr;
	const unsigned *rdy, *nce, *cle, *ale;

	printk("RB_PPC NAND\n");

	info = kmalloc(sizeof(*info), GFP_KERNEL);

	rdy = get_property(pdev->node, "rdy", NULL);
	nce = get_property(pdev->node, "nce", NULL);
	cle = get_property(pdev->node, "cle", NULL);
	ale = get_property(pdev->node, "ale", NULL);

	if (!rdy || !nce || !cle || !ale) {
		printk("rbppc nand error: gpios properties are missing\n");
		goto err;
	}
	if (rdy[0] != nce[0] || rdy[0] != cle[0] || rdy[0] != ale[0]) {
		printk("rbppc nand error: "
		       "different gpios are not supported\n");
		goto err;
	}

	gpio = of_find_node_by_phandle(rdy[0]);
	if (!gpio) {
		printk("rbppc nand error: no gpio<%x> node found\n", *rdy);
		goto err;
	}
	if (of_address_to_resource(gpio, 0, &res)) {
		printk("rbppc nand error: no reg property in gpio found\n");
		goto err;
	}
	info->gpo = ioremap_nocache(res.start, res.end - res.start + 1);

	if (!of_address_to_resource(gpio, 1, &res)) {
		info->gpi = ioremap_nocache(res.start, res.end - res.start + 1);
	} else {
		info->gpi = info->gpo;
	}
	of_node_put(gpio);

	info->gpio_rdy = 1 << (31 - rdy[1]);
	info->gpio_nce = 1 << (31 - nce[1]);
	info->gpio_cle = 1 << (31 - cle[1]);
	info->gpio_ale = 1 << (31 - ale[1]);
	info->gpio_ctrls = info->gpio_nce | info->gpio_cle | info->gpio_ale;

	nnand = of_find_node_by_name(NULL, "nnand");
	if (!nnand) {
		printk("rbppc nand error: no nnand found\n");
		goto err;
	}
	if (of_address_to_resource(nnand, 0, &res)) {
		printk("rbppc nand error: no reg property in nnand found\n");
		goto err;
	}
	of_node_put(nnand);
	info->localbus = ioremap_nocache(res.start, res.end - res.start + 1);
	
	if (of_address_to_resource(pdev->node, 0, &res)) {
	    printk("rbppc nand error: no reg property found\n");
	    goto err;
	}
	baddr = ioremap_nocache(res.start, res.end - res.start + 1);

	memset(&rnand, 0, sizeof(rnand));
	rnand.cmd_ctrl = rbppc_nand_hwcontrol;
    	rnand.dev_ready = rbppc_nand_ready;
    	rnand.read_buf = rbppc_read_buf;
	rnand.IO_ADDR_W = baddr;
	rnand.IO_ADDR_R = baddr;
	rnand.priv = info;

	return common_probe();

  err:
	kfree(info);
	return -1;
}

/* common for all NAND chips */
							     
static struct mtd_partition partition_info[] = {
    {
        name: "RouterBoard NAND Boot",
        offset: 0,
	size: 4 * 1024 * 1024
    },
    {
        name: "RouterBoard NAND Main",
	offset: MTDPART_OFS_NXTBLK,
	size: MTDPART_SIZ_FULL
    }
};

static unsigned init_ok = 0;

unsigned get_rbnand_block_size(void) {
	if (init_ok) return rmtd.writesize; else return 0;
}

EXPORT_SYMBOL(get_rbnand_block_size);

static int common_probe(void) {
	memset(&rmtd, 0, sizeof(rmtd));

	rnand.ecc.mode = NAND_ECC_SOFT;
	rnand.chip_delay = 25;
	rnand.options |= NAND_NO_AUTOINCR;
	rmtd.priv = &rnand;

	if (nand_scan(&rmtd, 1) && nand_scan(&rmtd, 1)
	    && nand_scan(&rmtd, 1)  && nand_scan(&rmtd, 1)) {
		printk("RBxxx nand device not found\n");
		return -ENXIO;
	}

	add_mtd_partitions(&rmtd, partition_info, 2);
	init_ok = 1;
	return 0;
}

static struct of_device_id rbppc_nand_ids[] = {
	{ .name = "nand" },
	{}
};

static struct of_platform_driver rbppc_nand_driver = {
	.name   = "nand",
	.probe	= rbppc_nand_probe,
	.match_table = rbppc_nand_ids,
	.driver	= {
		.name = "rbppc-nand",
		.owner = THIS_MODULE,
	},
};

static int __init rbppc_nand_init(void)
{
	of_register_platform_driver(&rbppc_nand_driver);
	return 0;
}

static void __exit rbppc_nand_exit(void)
{
	of_unregister_platform_driver(&rbppc_nand_driver);
}
							     
module_init(rbppc_nand_init);
module_exit(rbppc_nand_exit);
