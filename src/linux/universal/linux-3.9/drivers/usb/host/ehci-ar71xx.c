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
#if defined (CONFIG_MACH_AR7240) || defined (CONFIG_MACH_HORNET)
#include <asm/mach-ar7240/ar7240.h>
#else
#include <asm/mach-ar7100/ar7100.h>
#endif

/*
 * AR9130 Debug functions
 */
#define AR9130_DEBUG_FUNCTION               (0x00000001)
#define AR9130_DEBUG_INTERRUPT              (0x00000002)
#define AR9130_DEBUG_ENDPOINT               (0x00000004)
#define AR9130_DEBUG_PORTSTATUS             (0x00000008)
#define AR9130_DEBUG_DEVICE                 (0x00000010)
#define AR9130_DEBUG_MEMORY                 (0x00000020)
#define AR9130_DEBUG_QUEUEHEAD              (0x00000040)
#define AR9130_DEBUG_DTD                    (0x00000080)
#define AR9130_DEBUG_OTG                    (0x00000100)


#ifdef AR9130_USB_DEBUG
#define ar9130_debug(level, format, arg...)             \
do {                                                    \
    if (level & ar9130_debug_level) {                   \
        printk(format, ##arg);                          \
    }                                                   \
} while (0)
#else
#define ar9130_debug(level, format, arg...)             \
    do { (void)(level); } while (0)
#endif

#define ar9130_debug_fn(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_FUNCTION, format, ##arg)
#define ar9130_debug_ep(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_ENDPOINT, format, ##arg)
#define ar9130_debug_ps(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_PORTSTATUS, format, ##arg)
#define ar9130_debug_int(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_INTERRUPT, format, ##arg)
#define ar9130_debug_dev(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_DEVICE, format, ##arg)
#define ar9130_debug_mem(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_MEMORY, format, ##arg)
#define ar9130_debug_qh(format, arg...)                 \
        ar9130_debug(AR9130_DEBUG_QUEUEHEAD, format, ##arg)
#define ar9130_debug_dtd(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_DTD, format, ##arg)
#define ar9130_debug_otg(format, arg...)                \
        ar9130_debug(AR9130_DEBUG_OTG, format, ##arg)

#define ar9130_error(format, arg...)                    \
        printk(format, ##arg)
#define ar9130_warn(format, arg...)                     \
        printk(format, ##arg)


/* Device/Host Capability Reg */
#define AR9130_NON_EHCI_DCCPARAMS           (0x124)

/* Device/Host Timer Reg */
#define AR9130_NON_EHCI_TIMER0LD            (0x80)
#define AR9130_NON_EHCI_TIMER0CTRL          (0x84)
#define AR9130_NON_EHCI_TIMER1LD            (0x88)
#define AR9130_NON_EHCI_TIMER1CTRL          (0x8C)

/* Device/Host Operational Reg (non-ehci) */
#define AR9130_EHCI_EXT_BURSTSZ             (0x160)
#define AR9130_EHCI_EXT_TTCTRL              (0x15C)
#define AR9130_EHCI_EXT_USBMODE             (0x1A8)
#define AR9130_EHCI_EXT_TXFILL              (0x164)
#define AR9130_EHCI_EXT_ULPI                (0x170)
#define AR9130_EHCI_EXT_OTGSC               (0x1A4)

#if defined (CONFIG_MACH_AR7240) || defined (CONFIG_MACH_HORNET)

#define AR9130_RESET_USB_HOST               (AR7240_RESET_USB_HOST)
#define AR9130_RESET_USB_PHY                (AR7240_RESET_USB_PHY)
#define AR9130_RESET_USBSUS_OVRIDE	    (AR7240_RESET_USBSUS_OVRIDE)
#define AR9130_USB_MODE                     (AR7240_USB_MODE)

#define AR9130_RESET                        (AR7240_RESET_BASE + 0x1C)
#define AR9130_USB_CONFIG                   (AR7240_USB_CONFIG_BASE + 0x4)
#define AR9130_USB_FLADJ_VAL                (AR7240_USB_CONFIG_BASE)

#define ar9130_reg_rmw_set(_reg,_mask)      ar7240_reg_rmw_set(_reg,_mask)
#define ar9130_reg_rmw_clear(_reg,_mask)    ar7240_reg_rmw_clear(_reg,_mask)
#define ar9130_reg_wr(_phys,_val)           ar7240_reg_wr(_phys,_val)
#define ar9130_reg_rd(_phys)                ar7240_reg_rd(_phys)

#else

#define AR9130_RESET_USB_HOST               (AR7100_RESET_USB_HOST)
#define AR9130_RESET_USB_PHY                (AR7100_RESET_USB_PHY)
#define AR9130_RESET_USBSUS_OVRIDE	    (AR7100_RESET_USBSUS_OVRIDE)

#define AR9130_RESET                        (AR7100_RESET)
#define AR9130_USB_CONFIG                   (AR7100_USB_CONFIG_BASE + 0x4)
#define AR9130_USB_FLADJ_VAL                (AR7100_USB_CONFIG_BASE)

#define ar9130_reg_rmw_set(_reg,_mask)      ar7100_reg_rmw_set(_reg,_mask)
#define ar9130_reg_rmw_clear(_reg,_mask)    ar7100_reg_rmw_clear(_reg,_mask)
#define ar9130_reg_wr(_phys,_val)           ar7100_reg_wr(_phys,_val)
#define ar9130_reg_rd(_phys)                ar7100_reg_rd(_phys)

#endif

extern int usb_disabled(void);
extern int is_ar9000;

static void 
ar7100_start_ehc(struct platform_device *dev)
{
    int mask = AR9130_RESET_USB_HOST|AR9130_RESET_USB_PHY;

	printk(KERN_DEBUG __FILE__
		": starting AR7100 EHCI USB Controller...");

    ar9130_reg_rmw_set(AR9130_RESET, mask);
    mdelay(1000);
    ar9130_reg_rmw_clear(AR9130_RESET, mask);

    //ar7100_reg_wr(AR7100_USB_CONFIG, 0x20);
    //ar7100_reg_rmw_clear(AR7100_USB_CONFIG, 0x4);

    /*Turning on the Buff and Desc swap bits */
    ar9130_reg_wr(AR9130_USB_CONFIG, 0x30000);

    /* WAR for HW bug. Here it adjusts the duration between two SOFS */
    /* Was: ar7100_reg_wr(AR7100_USB_FLADJ_VAL,0x20400); */
    ar9130_reg_wr(AR9130_USB_FLADJ_VAL,0x20c00);

    mdelay(900);
    printk("done. reset %#x usb config %#x\n", ar9130_reg_rd(AR9130_RESET),
            ar9130_reg_rd(AR9130_USB_CONFIG));
}

static void ehci_port_power (struct ehci_hcd *ehci, int is_on)
{
	unsigned port;

	if (!HCS_PPC (ehci->hcs_params))
		return;

	ehci_dbg (ehci, "...power%s ports...\n", is_on ? "up" : "down");
	for (port = HCS_N_PORTS (ehci->hcs_params); port > 0; )
		(void) ehci_hub_control(ehci_to_hcd(ehci),
				is_on ? SetPortFeature : ClearPortFeature,
				USB_PORT_FEAT_POWER,
				port--, NULL, 0);
	/* Flush those writes */
	ehci_readl(ehci, &ehci->regs->command);
	msleep(20);
}



static int ehci_ar71xx_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int ret;

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci,ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci->has_synopsys_hc_bug = 1;
	ehci->sbrn = 0x20;

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
	ehci->regs = hcd->regs + 0x100 + HC_LENGTH(ehci, readl(&ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);



	ret = ehci_init(hcd);
	if (ret)
		return ret;

	hcd->has_tt = 1; /*Informs USB Subsystem abt embedded TT */
	ehci->sbrn = 0x20;
	ehci_reset(ehci);

	ehci_port_power(ehci, 0);

	return 0;
}



static int ehci_ar71xx_probe(const struct hc_driver *driver,
			     struct usb_hcd **hcd_out,
			     struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct resource *res;
	int irq;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no IRQ specified\n");
		return -ENODEV;
	}
	irq = res->start;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no base address specified\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, pdev->dev.bus->name);
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
	if (!is_ar9000)
	    ar7100_start_ehc(pdev);
	else{
	    ehci =hcd_to_ehci(hcd);
    ehci->caps = hcd->regs + 0x100;     /* Device/Host Capa Reg*/
    ehci->regs = hcd->regs + 0x140;     /* Device/Host Oper Reg*/

    ar9130_debug_dev("hcd->regs %p \n",hcd->regs);
    ar9130_debug_dev("Host Capability Reg %p \n",ehci->caps);
    ar9130_debug_dev("Host Operational Reg %p \n",ehci->regs);

    /* Added 5_29_07 */
#ifdef CONFIG_WASP_SUPPORT

#define AR7240_RESET_USB_PHY_PLL_PWD_EXT	(1 << 15)

    /* USB Soft Reset Sequence - Power Down the USB PHY PLL */
	ar9130_reg_rmw_set(AR9130_RESET, AR9130_RESET_USBSUS_OVRIDE | AR7240_RESET_USB_PHY_ANALOG | AR7240_RESET_USB_PHY_PLL_PWD_EXT);
	mdelay(10);

    /* USB Soft Reset Sequence - Power Up the USB PHY PLL */
    ar9130_reg_wr(AR9130_RESET,(ar9130_reg_rd(AR9130_RESET) & ~(AR7240_RESET_USB_PHY_PLL_PWD_EXT)));
    mdelay(10);

	ar9130_reg_wr(AR9130_RESET,
		      ((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST))
		       | AR9130_RESET_USBSUS_OVRIDE));
	mdelay(10);

	ar9130_reg_wr(AR9130_RESET,
		      ((ar9130_reg_rd(AR9130_RESET) &
			~(AR9130_RESET_USB_PHY | AR7240_RESET_USB_PHY_ANALOG)) |
		       AR9130_RESET_USBSUS_OVRIDE));
	mdelay(10);


#else

#ifdef CONFIG_MACH_HORNET
    ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USBSUS_OVRIDE);
    mdelay(10);

    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST)) |
                AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);

    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_PHY)) |
                AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);
#else        
#ifdef CONFIG_MACH_AR7240

#ifdef CONFIG_WASP_SUPPORT
    /* USB Soft Reset Sequence - Power Down the USB PHY PLL */
	ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USBSUS_OVRIDE | AR9130_RESET_USB_PHY_ANALOG | AR9130_RESET_USB_PHY_PLL_PWD_EXT);
	mdelay(10);

    /* USB Soft Reset Sequence - Power Up the USB PHY PLL */
	ar9130_reg_wr(AR9130_RESET,(ar9130_reg_rd(AR9130_USB_RESET) & ~(AR9130_RESET_USB_PHY_PLL_PWD_EXT)));
    mdelay(10);
#else

    ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USBSUS_OVRIDE |AR7240_RESET_USB_PHY_ANALOG);
    mdelay(10);
#endif	
    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST)) |  AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);

    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_PHY | AR7240_RESET_USB_PHY_ANALOG)) | AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);
#else
    ar9130_reg_rmw_set(AR9130_RESET,AR9130_RESET_USBSUS_OVRIDE);
    mdelay(10);
    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST)) | AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);

    ar9130_reg_wr(AR9130_RESET,((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_PHY)) | AR9130_RESET_USBSUS_OVRIDE));
    mdelay(10);


#endif
#endif
#endif

	
	}

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
	.endpoint_reset		= ehci_endpoint_reset,

	.get_frame_number	= ehci_get_frame,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef CONFIG_PM
	.hub_suspend		= ehci_hub_suspend,
	.hub_resume		= ehci_hub_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
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
	.endpoint_reset		= ehci_endpoint_reset,

	.get_frame_number	= ehci_get_frame,

	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef CONFIG_PM
	.hub_suspend		= ehci_hub_suspend,
	.hub_resume		= ehci_hub_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};



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
