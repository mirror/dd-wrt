/*
 *  EHCI Host Controller driver
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/platform_device.h>

#define IO_ADDRESS(x)      (((x&0xfff00000)>>4)|(x & 0x000fffff)|0xF0000000)

#define otg_set(port, bits) writel(readl(hcd->regs + port) | bits, hcd->regs + port)

#define otg_clear(port, bits) writel(readl(hcd->regs + port) & ~bits, hcd->regs + port)

#define BIT(x) (1 << x)

#define OTGC_INT_BSRPDN                           BIT(0)
#define OTGC_INT_ASRPDET                          BIT(4)
#define OTGC_INT_AVBUSERR                         BIT(5)
#define OTGC_INT_RLCHG                            BIT(8)
#define OTGC_INT_IDCHG                            BIT(9)
#define OTGC_INT_OVC                              BIT(10)
#define OTGC_INT_BPLGRMV                          BIT(11)
#define OTGC_INT_APLGRMV                          BIT(12)

#define OTGC_INT_A_TYPE                           (OTGC_INT_ASRPDET|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG|OTGC_INT_APLGRMV)
#define OTGC_INT_B_TYPE                           (OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG)

static void fotg2xx_otgc_role_change(struct usb_hcd *hcd);

static void fotg2xx_otgc_init(struct usb_hcd *hcd)
{
	/***************** OTG HW INIT *****************/

	printk("USB OTG2xx role detected: %s, ",
		((readl(hcd->regs + 0x80) >> 20) & 1)?"Peripheral":"Host");

	/* register 0x80, bit21 - 0: A-Device, 1: B-Device */
	if (readl(hcd->regs + 0x80) & (1 << 21))
		printk("B-Device (may be unsupported!)\n");
	else
		printk("A-Device\n");

	/* clear A_SRP_RESP_TYPE */
	otg_clear(0x80, BIT(8));

	/* clear INT B: bits AVBUSERR | OVC | RLCHG | IDCHG */
	otg_clear(0x88, OTGC_INT_B_TYPE);

	/* set INT A: bits ASRPDET | AVBUSERR | OVC | RLCHG | IDCHG | APLGRMV */
	otg_set(0x88, OTGC_INT_A_TYPE);

	otg_clear(0xc4, BIT(1)); /* unMASK OTG INT */

	/* WILIBOX: setup MISC register, fixes timing problems */
	otg_set(0x40, 0xD);

	fotg2xx_otgc_role_change(hcd);
}

static void fotg2xx_otgp_close(struct usb_hcd *hcd)
{
	u32 wTemp;

	/* usb glob int dis */
	otg_clear(0x100, BIT(2));

	/* mask perip. set */
	otg_set(0xc4, BIT(0));

	/* clear int status (?) */
	wTemp = readl(hcd->regs + 0x140);
	wTemp = readl(hcd->regs + 0x144);
	wTemp = readl(hcd->regs + 0x148);
	wTemp = readl(hcd->regs + 0x14C);

	otg_set(0x140, 0);
	otg_set(0x144, 0);
	otg_set(0x148, 0);
	otg_set(0x14C, 0);
}

static void fotg2xx_otgh_close(struct usb_hcd *hcd)
{
	u32 wTemp;

	/* <1>.Enable Interrupt Mask
	mdwOTGC_GINT_MASK_HOST_Set(); */
	otg_set(0xc4, BIT(2));

	/* <2>.Clear the Interrupt status
	wTemp=mdwHost20_USBINTR_Rd();
	wTemp=wTemp&0x0000003F;
	mdwHost20_USBSTS_Set(wTemp); */

	wTemp = readl(hcd->regs + 0x18);
	wTemp = wTemp & 0x0000003F;
	writel(wTemp, hcd->regs + 0x14);
}

static void fotg2xx_otgh_open(struct usb_hcd *hcd)
{
	/* clear A_SRP_DET */
	otg_clear(0x80, BIT(7));
	/* clear HOST INT MASK */
	otg_clear(0xc4, BIT(2));
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

		/* set global bus reg to VBUS on */
		writel(readl(IO_ADDRESS(0x40000000) + 0x30) | ((BIT(21)|BIT(22))),
		       IO_ADDRESS(0x40000000) + 0x30);

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
	fotg2xx_otgp_close(hcd);
	fotg2xx_otgh_open(hcd);
}

static int fotg2xx_ehci_hc_reset(struct usb_hcd *hcd)
{
	int result;
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);

	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));

	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	ehci->hcs_params = readl(&ehci->caps->hcs_params);

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
             fotg2xx_otgc_init(hcd);
          }
       else{//Changfe to A Type
             fotg2xx_otgc_init(hcd);
           }

      }else{//else of " if (wINTStatus&OTGC_INT_IDCHG) "

  //<2>.Check for RL_Change
           if (wINTStatus&OTGC_INT_RLCHG)
               {

                     fotg2xx_otgc_role_change(hcd);
               }

 //<3>.Error Detect
     if (wINTStatus&OTGC_INT_AVBUSERR)
        {
         printk("%s[%d]: VBus error!\n",__FILE__,__LINE__);

        }
     if (wINTStatus&OTGC_INT_OVC)
       {
         printk("%s[%d]: Overcurrent detected!\n",__FILE__,__LINE__);

       }


           //<3>.Check for Type-A/Type-B Interrupt
           if ((readl(hcd->regs + 0x80) & BIT(21)) == 0)
              {//For Type-A Interrupt
             	if (wINTStatus&OTGC_INT_A_TYPE)
                  {

                   if (wINTStatus&OTGC_INT_ASRPDET)
                      {
                       //<1>.SRP detected => then set global variable
                             printk("%s[%d] SRP detected, but not implemented!\n",__FILE__,__LINE__);

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
	u32 val;

	/* OTG Interrupt Status Register */
	val = readl(hcd->regs + 0x84);

	/* OTG stuff */
	if (val) {
		/* supposed to do "INT STS Clr" - XXX */
		writel(readl(hcd->regs + 0x84) | val, hcd->regs + 0x84);

		fotg2xx_int_isr(hcd, val);

		/* supposed to do "INT STS Clr" - XXX */
		writel(readl(hcd->regs + 0x84) | val, hcd->regs + 0x84);

		return IRQ_HANDLED;
	}

	if ((readl(hcd->regs + 0x80) & BIT(20)) == 0) { /* Role is HOST */
		if (readl(hcd->regs + 0xC0) & BIT(2)) { /* INT STS HOST */
			/* leave this for ehci irq handler */
			return IRQ_NONE;
		}
	} else
		printk("%s: received irq for peripheral - don't know what to do!\n", __FILE__);

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
	//.shutdown		= ehci_shutdown,
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
};



/**
 * usb_hcd_fotg2xx_probe - initialize FOTG2XX-based HCDs
 * @drvier: Driver to be used for this HCD
 * @pdev: USB Host Controller being probed
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller.
 *
 */
static int fotg2xx_ehci_probe(struct platform_device *pdev)
{
	const struct hc_driver *driver = &fotg2xx_ehci_hc_driver;
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int retval;

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


	/* set global reg to mini-A host */
	writel(readl(IO_ADDRESS(0x40000000) + 0x30) & ~(BIT(30)|BIT(29)),
	       IO_ADDRESS(0x40000000) + 0x30);

	/* USB0&USB1 - VBUS off */
	writel(readl(IO_ADDRESS(0x40000000) + 0x30) & ~(BIT(21)|BIT(22)),
	       IO_ADDRESS(0x40000000) + 0x30);

	if ( (readl(hcd->regs) == 0x01000010) &&
		(readl(hcd->regs + 4) == 0x00000001) &&
		(readl(hcd->regs + 8) == 0x00000006) )
		printk("Found Faraday OTG 2XX controller (base = 0x%08lX)\n", (unsigned long) hcd->rsrc_start);
	else {
		dev_dbg(&pdev->dev, "fotg2xx id mismatch: found %d.%d.%d\n",
		       readl(hcd->regs + 0x00),
		       readl(hcd->regs + 0x04),
		       readl(hcd->regs + 0x08));
		retval = -ENODEV;
		goto err4;
	}

	platform_set_drvdata(pdev, hcd);

	/* set ChipEnable  */
	otg_set(0x100, BIT(5));

	/* set HalfSpeedEnable */
	otg_set(0x100, BIT(1));

	/* mask interrupts - peripheral, otg, host, hi-active (bits 0,1,2,3) */
	//otg_clear(0xc4, BIT(3)); /* hi active */
	otg_set(0xc4, BIT(3)); /* hi active */

	otg_set(0xc4, BIT(2)); /* host */
	otg_set(0xc4, BIT(1)); /* otg */
	otg_set(0xc4, BIT(0)); /* peripheral */

	/* register additional interrupt - here we check otg status */
	if ((request_irq(irq, &fotg2xx_ehci_irq, IRQF_SHARED|IRQF_DISABLED,
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

/**
 * usb_hcd_fotg2xx_remove - shutdown processing for FOTG2XX-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_hcd_fotg2xx_probe().
 *
 */
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

MODULE_ALIAS("fotg2xx-ehci");

static struct platform_driver fotg2xx_ehci_driver = {
	.probe = fotg2xx_ehci_probe,
	.remove = fotg2xx_ehci_remove,
	.driver = {
		.name = "ehci-hcd-FOTG2XX",
	},
};
