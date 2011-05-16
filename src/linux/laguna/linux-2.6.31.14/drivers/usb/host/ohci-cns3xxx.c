
#include <linux/platform_device.h>
#include <mach/board.h>

#define cns3xxx_ioremap      ioremap
#define cns3xxx_iounmap      iounmap        

static int __devinit
cns3xxx_ohci_start (struct usb_hcd *hcd)
{
	struct ohci_hcd		*ohci = hcd_to_ohci (hcd);
	int			ret;

	if ((ret = ohci_init(ohci)) < 0)
		return ret;

	ohci->num_ports = 1;

	if ((ret = ohci_run(ohci)) < 0) {
		err("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}
	return 0;
}

static const struct hc_driver cns3xxx_ohci_hc_driver = {
	.description 		= hcd_name,
	.product_desc 		= "CNS3XXX OHCI Host controller",
	.hcd_priv_size 		= sizeof(struct ohci_hcd),
	.irq 			= ohci_irq,
	.flags 			= HCD_USB11 | HCD_MEMORY,
	.start 			= cns3xxx_ohci_start,
	.stop 			= ohci_stop,
	.shutdown 		= ohci_shutdown,
	.urb_enqueue 		= ohci_urb_enqueue,
	.urb_dequeue 		= ohci_urb_dequeue,
	.endpoint_disable 	= ohci_endpoint_disable,
	.get_frame_number 	= ohci_get_frame,
	.hub_status_data 	= ohci_hub_status_data,
	.hub_control 		= ohci_hub_control,
#ifdef CONFIG_PM
	.bus_suspend 		= ohci_bus_suspend,
	.bus_resume 		= ohci_bus_resume,
#endif
	.start_port_reset 	= ohci_start_port_reset,
};

static int cns3xxx_ohci_probe(struct platform_device *pdev)
{
        struct usb_hcd *hcd = NULL;
        const struct hc_driver *driver = &cns3xxx_ohci_hc_driver;
        struct resource *res;
        int irq;
        int retval;

        if (usb_disabled())
                return -ENODEV;

        res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
        if (!res) {
                dev_err(&pdev->dev,
                        "Found HC with no IRQ. Check %s setup!\n",
                        dev_name(&pdev->dev));
                return -ENODEV;
        }
        irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!res) {
                dev_err(&pdev->dev,
                        "Found HC with no register addr. Check %s setup!\n",
                        dev_name(&pdev->dev));
                retval = -ENODEV;
                goto err1;
        }
        hcd->rsrc_start = res->start;
        hcd->rsrc_len = res->end - res->start + 1;

#ifdef CNS3XXX_USB_OHCI_BASE_VIRT
        hcd->regs = (void __iomem *) CNS3XXX_USB_OHCI_BASE_VIRT;
#else
        if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
                                driver->description)) {
                dev_dbg(&pdev->dev, "controller already in use\n");
                retval = -EBUSY;
                goto err1;
        }

        hcd->regs = cns3xxx_ioremap(hcd->rsrc_start, hcd->rsrc_len);

        if (hcd->regs == NULL) {
                dev_dbg(&pdev->dev, "error mapping memory\n");
                retval = -EFAULT;
                goto err2;
        }
#endif

	ohci_hcd_init(hcd_to_ohci(hcd));

        retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval == 0)
		return retval;

#ifndef CNS3XXX_USB_OHCI_BASE_VIRT
        cns3xxx_iounmap(hcd->regs);

err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
#endif

err1:
	usb_put_hcd(hcd);
	return retval;
}

static int cns3xxx_ohci_remove(struct platform_device *pdev)
{
        struct usb_hcd *hcd = platform_get_drvdata(pdev);

        usb_remove_hcd(hcd);
#ifndef CNS3XXX_USB_OHCI_BASE_VIRT
        cns3xxx_iounmap(hcd->regs);
        release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
#endif
        usb_put_hcd(hcd);

        return 0;
}

MODULE_ALIAS("platform:cns3xxx-ohci");

static struct platform_driver ohci_hcd_cns3xxx_driver = {
        .probe = cns3xxx_ohci_probe,
        .remove = cns3xxx_ohci_remove,
        .driver = {
                .name = "cns3xxx-ohci",
        },
};
