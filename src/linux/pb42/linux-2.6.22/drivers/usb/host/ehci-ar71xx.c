/*
 *  Bus Glue for Atheros AR71xx built-in EHCI controller.
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros' 2.6.15 BSP
 *	Copyright (C) 2007 Atheros Communications, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/mach-ar7100/ar7100.h>

extern int usb_disabled(void);

static void 
ar7100_start_ehc(struct platform_device *dev)
{
    int mask = AR7100_RESET_USB_HOST|AR7100_RESET_USB_PHY;

	printk(KERN_DEBUG __FILE__
		": starting AR7100 EHCI USB Controller...");

    ar7100_reg_rmw_set(AR7100_RESET, mask);
    mdelay(1000);
    ar7100_reg_rmw_clear(AR7100_RESET, mask);

    //ar7100_reg_wr(AR7100_USB_CONFIG, 0x20);
    //ar7100_reg_rmw_clear(AR7100_USB_CONFIG, 0x4);

    /*Turning on the Buff and Desc swap bits */
    ar7100_reg_wr(AR7100_USB_CONFIG, 0x30000);

    /* WAR for HW bug. Here it adjusts the duration between two SOFS */
    /* Was: ar7100_reg_wr(AR7100_USB_FLADJ_VAL,0x20400); */
    ar7100_reg_wr(AR7100_USB_FLADJ_VAL,0x20c00);

    mdelay(900);
    printk("done. reset %#x usb config %#x\n", ar7100_reg_rd(AR7100_RESET),
            ar7100_reg_rd(AR7100_USB_CONFIG));
}



static int ehci_ar71xx_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int ret;

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci->sbrn = 0x20;
	ehci->has_synopsys_hc_bug = 1;

	ehci_reset(ehci);

	ret = ehci_init(hcd);
	if (ret)
		return ret;

	ehci_port_power(ehci, 0);

	return 0;
}

static int ehci_ar91xx_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int ret;

	ehci->caps = hcd->regs + 0x100;
	ehci->regs = hcd->regs + 0x100 +
			HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci->is_tdi_rh_tt = 1; /*Informs USB Subsystem abt embedded TT */
	ehci->sbrn = 0x20;

	ehci_reset(ehci);

	ret = ehci_init(hcd);
	if (ret)
		return ret;

	ehci_port_power(ehci, 0);

	return 0;
}



static int ehci_ar71xx_probe(const struct hc_driver *driver,
			     struct usb_hcd **hcd_out,
			     struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no IRQ specified for %s\n",
			pdev->dev.bus_id);
		return -ENODEV;
	}
	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no base address specified for %s\n",
			pdev->dev.bus_id);
		return -ENODEV;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, pdev->dev.bus_id);
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start	= res->start;
	hcd->rsrc_len	= res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		ret = -EBUSY;
		goto err_put_hcd;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		ret = -EFAULT;
		goto err_release_region;
	}
	ar7100_start_ehc(pdev);

	ret = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (ret)
		goto err_iounmap;

	return 0;

 err_iounmap:
	iounmap(hcd->regs);

 err_release_region:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
 err_put_hcd:
	usb_put_hcd(hcd);
	return ret;
}

static void ehci_ar71xx_remove(struct usb_hcd *hcd,
			       struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static const struct hc_driver ehci_ar71xx_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Atheros AR71xx built-in EHCI controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	.reset			= ehci_ar71xx_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,

	.get_frame_number	= ehci_get_frame,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef CONFIG_PM
	.hub_suspend		= ehci_hub_suspend,
	.hub_resume		= ehci_hub_resume,
#endif
};

static const struct hc_driver ehci_ar91xx_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Atheros AR91xx built-in EHCI controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	.reset			= ehci_ar91xx_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,

	.get_frame_number	= ehci_get_frame,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef CONFIG_PM
	.hub_suspend		= ehci_hub_suspend,
	.hub_resume		= ehci_hub_resume,
#endif
};


extern int is_ar9000;

static int ehci_ar71xx_driver_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	int ret;

	if (usb_disabled())
		return -ENODEV;


	if (is_ar9000)
		ret = ehci_ar71xx_probe(&ehci_ar91xx_hc_driver, &hcd, pdev);
	else
		ret = ehci_ar71xx_probe(&ehci_ar71xx_hc_driver, &hcd, pdev);


	return ret;
}

static int ehci_ar71xx_driver_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	ehci_ar71xx_remove(hcd, pdev);
	return 0;
}

MODULE_ALIAS("platform:ar71xx-ehci");

static struct platform_driver ehci_ar71xx_driver = {
	.probe		= ehci_ar71xx_driver_probe,
	.remove		= ehci_ar71xx_driver_remove,
	.driver = {
		.name	= "ar71xx-ehci",
	}
};
