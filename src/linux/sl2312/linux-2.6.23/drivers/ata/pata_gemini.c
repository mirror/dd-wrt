/*
 *  Support for Gemini PATA
 *
 *  Copyright (C) 2009 Janos Laube <janos.dev@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/leds.h>

#include <asm/arch/hardware.h>

#define DRV_NAME		"pata_gemini"
#define DRV_VERSION		"0.1"

#define GEMINI_PATA_PORTS	1
#define PIO_TIMING_REG		(ap->ioaddr.bmdma_addr + 0x10)
#define MDMA_TIMING_REG		(ap->ioaddr.bmdma_addr + 0x11)
#define UDMA_TIMING0_REG	(ap->ioaddr.bmdma_addr + 0x12)
#define UDMA_TIMING1_REG	(ap->ioaddr.bmdma_addr + 0x13)
#define CLK_MOD_REG		(ap->ioaddr.bmdma_addr + 0x14)

static unsigned char PIO_TIMING[5] = {
	0xaa, 0xa3, 0xa1, 0x33, 0x31
};

static unsigned char TIMING_UDMA_50M[6] = {
	0x33, 0x31, 0x21, 0x21, 0x11, 0x91
};

static unsigned char TIMING_UDMA_66M[7] = {
	0x44, 0x42, 0x31, 0x21, 0x11, 0x91, 0x91
};

#define ATA_BASE_SHT(drv_name)					\
	.module			= THIS_MODULE,			\
	.name			= drv_name,			\
	.ioctl			= ata_scsi_ioctl,		\
	.queuecommand		= ata_scsi_queuecmd,		\
	.can_queue		= ATA_DEF_QUEUE,		\
	.this_id		= ATA_SHT_THIS_ID,		\
	.cmd_per_lun		= ATA_SHT_CMD_PER_LUN,		\
	.emulated		= ATA_SHT_EMULATED,		\
	.use_clustering		= ATA_SHT_USE_CLUSTERING,	\
	.proc_name		= drv_name,			\
	.slave_configure	= ata_scsi_slave_config,	\
	.slave_destroy		= ata_scsi_slave_destroy,	\
	.bios_param		= ata_std_bios_param		

#define ATA_NCQ_SHT(drv_name)					\
	ATA_BASE_SHT(drv_name),					\
	.change_queue_depth	= ata_scsi_change_queue_depth


static struct scsi_host_template gemini_pata_sht = {
	ATA_NCQ_SHT(DRV_NAME),
	.can_queue	= 1,
	.sg_tablesize	= 128,
	.dma_boundary	= 0xffffU,
};

#define SL2312_IDE_DMA_OFFSET			0x00
#define SL2312_IDE_PIO_TIMING_OFFSET	0x10
#define SL2312_IDE_MDMA_TIMING_OFFSET	0x11		// only support 1 multi-word DMA device
#define SL2312_IDE_UDMA_TIMING0_OFFSET	0x12		// for master
#define SL2312_IDE_UDMA_TIMING1_OFFSET	0x13		// for slave
#define SL2312_IDE_CLK_MOD_OFFSET		0x14		
#define SL2312_IDE_CMD_OFFSET			0x20
#define SL2312_IDE_CTRL_OFFSET			0x36

#define SATA_REG_PHY_STATUS0				0x08
#define SATA_REG_PHY_STATUS1				0x0C


void gemini_phy_reset (struct ata_port *ap)
{
	u32 status_p1,reg;
	unsigned long timeout = jiffies + (HZ * 1);
	reg = readl(IO_ADDRESS(SL2312_GLOBAL_BASE)+GLOBAL_RESET_REG);
	reg |=RESET_SATA1;
	writel(reg,IO_ADDRESS(SL2312_GLOBAL_BASE)+GLOBAL_RESET_REG);	// Reset SATA module
		
	msleep(10);
	
	do{
		msleep(100);
		status_p1 = inl(ap->ioaddr.scr_addr + SATA_REG_PHY_STATUS1);
		if(status_p1&0x01)
			break;
	}while (time_before(jiffies, timeout));
	if(!(status_p1&0x01)){
//		ap->device[gemini_sata_probe_flag&0x00000001].class = ATA_DEV_NONE;
		printk("PHY not ready!!\n");
		return ;
	}


	if((status_p1&0x01))		// device attach and link estabilished
		ata_port_probe(ap);
	else
		ata_port_disable(ap);
	
	ap->cbl = ATA_CBL_SATA;
	
	ata_bus_reset(ap);
}


static void gemini_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	unsigned int udma	= adev->dma_mode;
	u8 speed		= udma;
	unsigned int drive_dn	= 0;
	u8 reg;

	reg = ioread8(CLK_MOD_REG);
	reg |= (1 << (4 + (drive_dn & 0x01)));
	iowrite8(reg, CLK_MOD_REG);

	if ((speed == XFER_UDMA_6) || (speed == XFER_UDMA_3)
		|| (speed == XFER_UDMA_4) || (speed & XFER_MW_DMA_0))
	{
		reg = ioread8(CLK_MOD_REG);
		reg |= (1 << (drive_dn & 0x01));
		iowrite8(reg, CLK_MOD_REG);
		iowrite8(TIMING_UDMA_66M[speed & ~XFER_UDMA_0],
			UDMA_TIMING0_REG);
	}
	else
	{
		reg = ioread8(CLK_MOD_REG);
		reg &= ~(1 << (drive_dn & 0x01));
		iowrite8(reg, CLK_MOD_REG);
		iowrite8(TIMING_UDMA_50M[speed & ~XFER_UDMA_0],
			UDMA_TIMING0_REG);
	}
	return;
}

static void gemini_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	unsigned int pio = adev->pio_mode - XFER_PIO_0;
	iowrite8(PIO_TIMING[pio], PIO_TIMING_REG);
}

unsigned int gemini_qc_issue(struct ata_queued_cmd *qc)
{
	ledtrig_ide_activity();
	return ata_qc_issue_prot(qc);
}

static struct ata_port_operations gemini_pata_port_ops = {
	.port_disable		= ata_port_disable,
	.tf_load		= ata_tf_load,
	.tf_read		= ata_tf_read,
	.exec_command		= ata_exec_command,
	.check_status		= ata_check_status,
	.dev_select		= ata_std_dev_select,
	.bmdma_setup		= ata_bmdma_setup,
	.bmdma_start		= ata_bmdma_start,
	.bmdma_stop		= ata_bmdma_stop,
	.bmdma_status		= ata_bmdma_status,
	.error_handler		= ata_bmdma_error_handler,
	.qc_prep		= ata_qc_prep,
	.freeze			= ata_bmdma_freeze,
	.thaw			= ata_bmdma_thaw,
	.post_internal_cmd	= ata_bmdma_post_internal_cmd,
	.data_xfer		= ata_data_xfer,
	.irq_clear		= ata_bmdma_irq_clear,
	.irq_on			= ata_irq_on,
	.irq_ack		= ata_irq_ack,
	.port_start		= ata_port_start,
	.set_dmamode		= gemini_set_dmamode,
	.set_piomode		= gemini_set_piomode,
	.qc_issue		= gemini_qc_issue,
//	.scr_read		= gemini_sata_scr_read,
//	.scr_write		= gemini_sata_scr_write,
	.phy_reset		= gemini_phy_reset,
};

static struct ata_port_info gemini_pata_portinfo = {
	.flags		= ATA_FLAG_NO_LEGACY | ATA_FLAG_MMIO,
	.udma_mask	= ATA_UDMA7,
	.pio_mask	= 0x1F,
	.port_ops	= &gemini_pata_port_ops,
};

static irqreturn_t gemini_pata_interrupt(int irq, void *dev)
{
	struct ata_host *host = dev;
	unsigned int i, handled = 0;

	spin_lock_irq(&host->lock);
	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap;

		ap = host->ports[i];
		if (ap && !(ap->flags & ATA_FLAG_DISABLED)) {
			struct ata_queued_cmd *qc;
			qc = ata_qc_from_tag(ap, ap->active_tag);

			if (qc && (qc->tf.ctl & ATA_NIEN))
				ap->ops->check_status(ap);
			else if (qc && (!(qc->tf.flags & ATA_TFLAG_POLLING)) &&
					(qc->flags & ATA_QCFLAG_ACTIVE))
				handled |= ata_host_intr(ap, qc);
			else
				ap->ops->check_status(ap);
		}
		else
			ap->ops->check_status(ap);
	}
	spin_unlock_irq(&host->lock);

	return IRQ_RETVAL(handled);
}



static int gemini_pata_platform_probe(struct platform_device *pdev)
{
	struct ata_host *host;
	struct resource *res;
	const struct ata_port_info *ppi[] = {&gemini_pata_portinfo, 0};
	unsigned int irq, i;
	void __iomem *mmio_base;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res)
		return -ENODEV;
	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	mmio_base = devm_ioremap(&pdev->dev, res->start,
		res->end - res->start + 1);

	printk("pata_gemini: configuring port with irq %d, base %p\n",
			irq, mmio_base);

	host = ata_host_alloc_pinfo(&pdev->dev, ppi, GEMINI_PATA_PORTS);
	if (!host)
		return -ENOMEM;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
		struct ata_ioports *ioaddr = &ap->ioaddr;

		ioaddr->bmdma_addr		= mmio_base;
		ioaddr->cmd_addr		= mmio_base + 0x20;
		ioaddr->altstatus_addr	=
		ioaddr->ctl_addr		= mmio_base + 0x36;
		ata_std_ports(ioaddr);
		host->ports[i]->cbl = ATA_CBL_SATA;
	}
	return ata_host_activate(host, irq, gemini_pata_interrupt,
		IRQF_SHARED, &gemini_pata_sht);
}

static int gemini_pata_platform_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ata_host *host = dev_get_drvdata(dev);
	ata_host_detach(host);
	return 0;
}

static struct platform_driver gemini_pata_driver = {
	.probe		= gemini_pata_platform_probe,
	.remove		= gemini_pata_platform_remove,
	.driver = {
		.name = DRV_NAME,
	}
};

static int __init gemini_pata_module_init(void)
{
	unsigned int reg;
	u8 phy_status;
	unsigned long timeout = jiffies + (HZ * 1);

	/* iomux 0, no slave mode */
	reg = readl(IO_ADDRESS(SL2312_GLOBAL_BASE) + GLOBAL_MISC_REG);
	reg &= ~0x07000000;
	writel(reg, IO_ADDRESS(SL2312_GLOBAL_BASE) + GLOBAL_MISC_REG);

	/* disabling port if no drive is present */
	do
	{
		msleep(100);
		phy_status = readb(IO_ADDRESS(SL2312_SATA_BASE) + 0x08);
		if (phy_status & 0x01) break;
	} while (time_before(jiffies, timeout));
	if (!(phy_status & 0x01))
		writel(0x00, IO_ADDRESS(SL2312_SATA_BASE) + 0x18);

	do
	{
		msleep(100);
		phy_status = readb(IO_ADDRESS(SL2312_SATA_BASE) + 0x0C);
		if (phy_status & 0x01) break;
	} while (time_before(jiffies, timeout));
	if (!(phy_status & 0x01))
		writel(0x00, IO_ADDRESS(SL2312_SATA_BASE) + 0x1C);

	return platform_driver_register(&gemini_pata_driver);
}

static void __exit gemini_pata_module_exit(void)
{
	platform_driver_unregister(&gemini_pata_driver);
}

module_init(gemini_pata_module_init);
module_exit(gemini_pata_module_exit);

MODULE_AUTHOR("Janos Laube <janos.dev@gmail.com>");
MODULE_DESCRIPTION("Parallel ATA driver for Gemini SoC's");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:" DRV_NAME);
