#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/nand.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <asm/io.h>
#include "../mtdcore.h"

extern int rb_nand_probe(struct nand_chip *nand, int booter);

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

static int rbppc_nand_probe(struct platform_device *pdev)
{
	struct device_node *gpio;
	struct device_node *nnand;
	struct resource res;
	struct rbppc_info *info;
	void *baddr;
	const unsigned *rdy, *nce, *cle, *ale;

	printk("RB_PPC NAND\n");

	info = kmalloc(sizeof(*info), GFP_KERNEL);

	rdy = of_get_property(pdev->dev.of_node, "rdy", NULL);
	nce = of_get_property(pdev->dev.of_node, "nce", NULL);
	cle = of_get_property(pdev->dev.of_node, "cle", NULL);
	ale = of_get_property(pdev->dev.of_node, "ale", NULL);

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
	
	if (of_address_to_resource(pdev->dev.of_node, 0, &res)) {
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

	return rb_nand_probe(&rnand, 0);

  err:
	kfree(info);
	return -1;
}

/* common for all NAND chips */
							     
static struct of_device_id rbppc_nand_ids[] = {
	{ .name = "nand" },
	{}
};

static struct platform_driver rbppc_nand_driver = {
//	.name   = "nand",
	.probe	= rbppc_nand_probe,
	.driver	= {
		.name = "rbppc-nand",
		.of_match_table = rbppc_nand_ids,
		.owner = THIS_MODULE,
	},
};

static int __init rbppc_nand_init(void)
{
	return platform_driver_register(&rbppc_nand_driver);
}

static void __exit rbppc_nand_exit(void)
{
	platform_driver_unregister(&rbppc_nand_driver);
}
							     
module_init(rbppc_nand_init);
module_exit(rbppc_nand_exit);
