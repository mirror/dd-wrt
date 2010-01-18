/*
 *  EHCI Host Controller driver
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 */

#include <linux/platform_device.h>
#include <mach/hardware.h>

#define otg_set(port, bits) writel(readl(hcd->regs + port) | bits, hcd->regs + port)

#define otg_clear(port, bits) writel(readl(hcd->regs + port) & ~bits, hcd->regs + port)

#define GLOBAL_ISR			0xC0
#define GLOBAL_ICR			0xC4

#define HCD_MISC			0x40

#define OTGC_SCR			0x80
#define OTGC_INT_EN			0x88

#define GLOBAL_INT_POLARITY		(1 << 3)
#define GLOBAL_INT_MASK_HC		(1 << 2)
#define GLOBAL_INT_MASK_OTG		(1 << 1)
#define GLOBAL_INT_MASK_DEV		(1 << 0)

#define OTGC_SCR_ID			(1 << 21)
#define OTGC_SCR_CROLE			(1 << 20)
#define OTGC_SCR_VBUS_VLD		(1 << 19)
#define OTGC_SCR_A_SRP_RESP_TYPE	(1 << 8)
#define OTGC_SCR_A_SRP_DET_EN		(1 << 7)
#define OTGC_SCR_A_SET_B_HNP_EN		(1 << 6)
#define OTGC_SCR_A_BUS_DROP		(1 << 5)
#define OTGC_SCR_A_BUS_REQ		(1 << 4)

#define OTGC_INT_APLGRMV		(1 << 12)
#define OTGC_INT_BPLGRMV		(1 << 11)
#define OTGC_INT_OVC			(1 << 10)
#define OTGC_INT_IDCHG			(1 << 9)
#define OTGC_INT_RLCHG			(1 << 8)
#define OTGC_INT_AVBUSERR		(1 << 5)
#define OTGC_INT_ASRPDET		(1 << 4)
#define OTGC_INT_BSRPDN			(1 << 0)

#define OTGC_INT_A_TYPE		(OTGC_INT_ASRPDET|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG|OTGC_INT_APLGRMV)
#define OTGC_INT_B_TYPE		(OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG)

static void fotg2xx_otgc_role_change(struct usb_hcd *hcd);

static void fotg2xx_otgc_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned int reg;

	reg = __raw_readl(hcd->regs + OTGC_SCR);
	ehci_info(ehci, "role detected: %s, ",
		  (reg & OTGC_SCR_CROLE) ? "Peripheral" : "Host");

	if (reg & OTGC_SCR_ID)
		ehci_info(ehci, "B-Device (may be unsupported!)\n");
	else
		ehci_info(ehci, "A-Device\n");

	/* Enable the SRP detect */
	reg &= ~OTGC_SCR_A_SRP_RESP_TYPE;
	__raw_writel(reg, hcd->regs + OTGC_SCR);

	reg = __raw_readl(hcd->regs + OTGC_INT_EN);
	/* clear INT B: bits AVBUSERR | OVC | RLCHG | IDCHG */
	reg &= ~OTGC_INT_B_TYPE;
	/* set INT A: bits ASRPDET | AVBUSERR | OVC | RLCHG | IDCHG | APLGRMV */
	reg |= OTGC_INT_A_TYPE;
	__raw_writel(reg, hcd->regs + OTGC_INT_EN);

	reg = __raw_readl(hcd->regs + GLOBAL_ICR);
	reg &= ~GLOBAL_INT_MASK_OTG;
	__raw_writel(reg, hcd->regs + GLOBAL_ICR);

	/* setup MISC register, fixes timing problems */
	reg = __raw_readl(hcd->regs + HCD_MISC);
	reg |= 0xD;
	__raw_writel(reg, hcd->regs + HCD_MISC);

	fotg2xx_otgc_role_change(hcd);
}

static void fotg2xx_otgh_close(struct usb_hcd *hcd)
{
	unsigned int reg;

	/* <1>.Enable Interrupt Mask */
	reg = __raw_readl(hcd->regs + GLOBAL_ICR);
	reg |= GLOBAL_INT_MASK_HC;
	__raw_writel(reg, hcd->regs + GLOBAL_ICR);

	/* <2>.Clear the Interrupt status */
	reg = __raw_readl(hcd->regs + 0x18);
	reg &= 0x0000003F;
	__raw_writel(reg, hcd->regs + 0x14);
}

static void fotg2xx_otgh_open(struct usb_hcd *hcd)
{
	unsigned int reg;

	reg = __raw_readl(hcd->regs + OTGC_SCR);
	reg &= ~OTGC_SCR_A_SRP_DET_EN;
	__raw_writel(reg, hcd->regs + OTGC_SCR);

	reg = __raw_readl(hcd->regs + GLOBAL_ICR);
	reg &= ~GLOBAL_INT_MASK_HC;
	__raw_writel(reg, hcd->regs + GLOBAL_ICR);
}

/* change to host role */
static void fotg2xx_otgc_role_change(struct usb_hcd *hcd)
{

	/* clear A_SET_B_HNP_EN */
	otg_clear(0x80, BIT(6));

	/*** Enable VBUS driving */
	if (readl(hcd->regs + 0x80) & BIT(19))
		printk("VBUS already enabled\n");
	else {
		int cnt = 0;

		/* clear A_BUS_DROP */
		otg_clear(0x80, BIT(5));

		/* set A_BUS_REQ */
		otg_set(0x80, BIT(4));
#if 0
		/* set global bus reg to VBUS on */
		writel(readl(IO_ADDRESS(0x40000000) + 0x30) | ((BIT(22)|BIT(23))),
		       IO_ADDRESS(0x40000000) + 0x30);
#endif
	switch (hcd->rsrc_start) {
		case 0x68000000:
		writel(readl(IO_ADDRESS(0x40000000) + 0x30) | (BIT(22)),
		       IO_ADDRESS(0x40000000) + 0x30);
			break;
		case 0x69000000:
		writel(readl(IO_ADDRESS(0x40000000) + 0x30) | (BIT(23)),
		       IO_ADDRESS(0x40000000) + 0x30);
			break;
	}

		if (readl(hcd->regs + 0x80) & (1<<19)) {
			printk("Waiting for VBus");
			while ( !(readl(hcd->regs + 0x80) & (1<<19)) && (cnt < 80) ) {
				printk(".");
				cnt++;
			}
			printk("\n");
		} else
			printk("VBUS enabled.\n");
		mdelay(1);
	}
	fotg2xx_otgh_open(hcd);
}

static int fotg2xx_ehci_hc_reset(struct usb_hcd *hcd)
{
	int result;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));

	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);
	hcd->has_tt = 1;

	result = ehci_halt(ehci);
	if (result)
		return result;

	result = ehci_init(hcd);
	if (result)
		return result;

	ehci_port_power(ehci, 0);

	return result;
}

/*********************************************************
 Name: OTGC_INT_ISR
 Description:This interrupt service routine belongs to the OTG-Controller
            <1>.Check for ID_Change
            <2>.Check for RL_Change
            <3>.Error Detect
 Input: wINTStatus
 Output:void
*********************************************************/
void fotg2xx_int_isr(struct usb_hcd *hcd, u32 wINTStatus)
{
//  u32 wTempCounter;

  //<1>.Check for ID_Change
  if (wINTStatus&OTGC_INT_IDCHG)
      {
       if ((readl(hcd->regs + 0x80) & BIT(21)) != 0)
          {//Change to B Type
       //      fotg2xx_otgc_init(hcd);
          }
       else{//Change to A Type
      //       fotg2xx_otgc_init(hcd);
           }

      }else{//else of " if (wINTStatus&OTGC_INT_IDCHG) "

  //<2>.Check for RL_Change
           if (wINTStatus&OTGC_INT_RLCHG)
                     fotg2xx_otgc_role_change(hcd);

 //<3>.Error Detect
     if (wINTStatus&OTGC_INT_AVBUSERR)
         printk("VBus error!\n");

     if (wINTStatus&OTGC_INT_OVC)
         printk("Overcurrent detected!\n");

           //<3>.Check for Type-A/Type-B Interrupt
           if ((readl(hcd->regs + 0x80) & BIT(21)) == 0)
              {//For Type-A Interrupt
             	if (wINTStatus&OTGC_INT_A_TYPE)
                  {

                   if (wINTStatus&OTGC_INT_ASRPDET)
                      {
                       //<1>.SRP detected => then set global variable
                             printk("SRP detected, but not implemented!\n");

#if 0
                       //<2>.Turn on the V Bus
                             pFTC_OTG->otg.state = OTG_STATE_A_WAIT_VRISE;
                             OTGC_enable_vbus_draw_storlink(1);
                             pFTC_OTG->otg.state = OTG_STATE_A_HOST;
                       //<3>.Should waiting for Device-Connect Wait 300ms
                             INFO(pFTC_OTG,">>> OTG-A Waiting for OTG-B Connect,\n");
                             wTempCounter=0;
                             while(mwHost20_PORTSC_ConnectStatus_Rd()==0)
                                  {
                                   mdelay(1);
                                   wTempCounter++;
                                   if (wTempCounter>300)//Waiting for 300 ms
                                      {
                                       mdwOTGC_Control_A_SRP_DET_EN_Clr();
                                       INFO(pFTC_OTG,">>> OTG-B do not connect under 300 ms...\n");
                               	       break;
                                      }
                                   }
                       //<4>.If Connect => issue quick Reset
                            if (mwHost20_PORTSC_ConnectStatus_Rd()>0)
                                {mdelay(300);//For OPT-A Test
                                 OTGH_host_quick_Reset();
                                 OTGH_Open();
                                 pFTC_OTG->otg.host->A_Disable_Set_Feature_HNP=0;
                                }
#endif

                       }


                  }
              }else
              {//For Type-B Interrupt

              }
         }   //end of " if (wINTStatus&OTGC_INT_IDCHG) "
}

static irqreturn_t fotg2xx_ehci_irq (int irq, void * devid)
{
	struct usb_hcd *hcd = devid;
	struct ehci_hcd *ehci = hcd_to_ehci (hcd);
	u32 val;

	spin_lock_irq(&ehci->lock);
	/* OTG Interrupt Status Register */
	val = readl(hcd->regs + 0x84);

	/* OTG stuff */
	if (val) {
		/* supposed to do "INT STS Clr" - XXX */
		//writel(readl(hcd->regs + 0x84) | val, hcd->regs + 0x84);

//		fotg2xx_int_isr(hcd, val);

		/* supposed to do "INT STS Clr" - XXX */
		writel(readl(hcd->regs + 0x84) | val, hcd->regs + 0x84);
		spin_unlock_irq(&ehci->lock);
		return IRQ_HANDLED;
	}

	if ((readl(hcd->regs + 0x80) & BIT(20)) == 0) { /* Role is HOST */
		if (readl(hcd->regs + 0xC0) & BIT(2)) { /* INT STS HOST */
			/* leave this for ehci irq handler */
			spin_unlock_irq(&ehci->lock);
			return IRQ_NONE;
		}
	} else
		printk("received irq for peripheral - don't know what to do!\n");
	spin_unlock_irq(&ehci->lock);
	/* do not call the ehci irq handler */
	return IRQ_HANDLED;
}

static int fotg2xx_ehci_run (struct usb_hcd *hcd)
{
	int retval;

	retval = ehci_run(hcd);

	fotg2xx_otgh_close(hcd);
	fotg2xx_otgc_init(hcd);

	return retval;
}

static const struct hc_driver fotg2xx_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "FOTG2XX EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,
	.reset			= fotg2xx_ehci_hc_reset,
	.start			= fotg2xx_ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.get_frame_number	= ehci_get_frame,
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,
};

static int fotg2xx_ehci_probe(struct platform_device *pdev)
{
	const struct hc_driver *driver = &fotg2xx_ehci_hc_driver;
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int retval;
	long mask;

	pr_debug("initializing FOTG2XX-SOC USB Controller\n");

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			pdev->dev.bus_id);
		return -ENODEV;
	}

	irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev, pdev->dev.bus_id);
	if (!hcd) {
		retval = -ENOMEM;
		goto err1;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			pdev->dev.bus_id);
		retval = -ENODEV;
		goto err2;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto err2;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (hcd->regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto err3;
	}

	/* Setup for mini-A host and VBUS depending on Device */
	switch (hcd->rsrc_start) {
		case 0x68000000:
			/* USB0 */
			mask =  BIT(29) | BIT(22);
			break;
		case 0x69000000:
			/* USB1 */
			mask =  BIT(30) | BIT(23);
			break;
		default:
			dev_err(&pdev->dev, "fotg2xx id mismatch: found %d.%d.%d\n",
					readl(hcd->regs + 0x00), readl(hcd->regs + 0x04),
					readl(hcd->regs + 0x08));
			retval = -ENODEV;
			goto err4;
	}

	writel(readl(IO_ADDRESS(0x40000000) + 0x30) & ~mask,
			IO_ADDRESS(0x40000000) + 0x30);

	if ( (readl(hcd->regs) == 0x01000010) &&
		(readl(hcd->regs + 4) == 0x00000001) &&
		(readl(hcd->regs + 8) == 0x00000006) )
		printk("Found Faraday OTG 2XX controller (base = 0x%08lX)\n", (unsigned long) hcd->rsrc_start);
	else {
		dev_err(&pdev->dev, "fotg2xx id mismatch: found %d.%d.%d\n",
		       readl(hcd->regs + 0x00),
		       readl(hcd->regs + 0x04),
		       readl(hcd->regs + 0x08));
		retval = -ENODEV;
		goto err4;
	}

	platform_set_drvdata(pdev, hcd);

	/* register additional interrupt - here we check otg status */

	/* mask interrupts - peripheral, otg, host, hi-active (bits 0,1,2,3) */
	otg_set(0xc4, BIT(3)); /* hi active */
	otg_set(0xc4, BIT(2)); /* host */
	otg_set(0xc4, BIT(1)); /* otg */
//	otg_set(0xc4, BIT(0)); /* peripheral */

	if ((request_irq(irq, &fotg2xx_ehci_irq, IRQF_SHARED,
		 hcd->irq_descr, hcd)) != 0) {
		dev_dbg(&pdev->dev, "error requesting irq %d\n", irq);
		retval = -EFAULT;
		goto err4;
	}

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval != 0)
		goto err4;

	return retval;

err4:
	iounmap(hcd->regs);
err3:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err2:
	usb_put_hcd(hcd);
err1:
	dev_err(&pdev->dev, "init %s fail, %d\n", pdev->dev.bus_id, retval);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

int fotg2xx_ehci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd =
		(struct usb_hcd *)platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	iounmap(hcd->regs);
	usb_put_hcd(hcd);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

MODULE_ALIAS("platform:ehci-fotg2xx");

static struct platform_driver fotg2xx_ehci_driver = {
	.probe = fotg2xx_ehci_probe,
	.remove = fotg2xx_ehci_remove,
	.driver = {
		.name = "ehci-fotg2xx",
	},
};
