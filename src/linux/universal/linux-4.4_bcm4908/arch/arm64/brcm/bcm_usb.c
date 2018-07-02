#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/*
 ****************************************************************************
 * File Name  : bcm63xx_usb.c
 *
 * Description: This file contains the initilzation and registration routines
 * to enable USB controllers on bcm63xxx boards. 
 *
 *
 ***************************************************************************/

#if defined(CONFIG_USB) || defined(CONFIG_USB_MODULE)

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/bug.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>

#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <pmc_usb.h>

#include <boardparms.h>

extern void bcm_set_pinmux(unsigned int pin_num, unsigned int mux_num);


#define CAP_TYPE_EHCI       0x00
#define CAP_TYPE_OHCI       0x01
#define CAP_TYPE_XHCI       0x02

static struct usb_ehci_pdata bcm_ehci_pdata = {0};
static struct usb_ohci_pdata bcm_ohci_pdata = {0};

#if defined(CONFIG_BCM96858)
static struct usb_ehci_pdata bcm_ehci1_pdata = {0};
static struct usb_ohci_pdata bcm_ohci1_pdata = {0};
static struct platform_device *ehci1_dev;
static struct platform_device *ohci1_dev;
#endif

static struct platform_device *xhci_dev;
static struct platform_device *ehci_dev;
static struct platform_device *ohci_dev;


static __init struct platform_device *bcm_add_usb_host(int type, int id,
                        uint32_t mem_base, uint32_t mem_size, int irq,
                        const char *devname, void *private_data)
{
    struct resource res[2];
    struct platform_device *pdev;
    static const u64 usb_dmamask = DMA_BIT_MASK(32);

    memset(&res, 0, sizeof(res));
    res[0].start = mem_base;
    res[0].end   = mem_base + (mem_size -1);
    res[0].flags = IORESOURCE_MEM;

    res[1].flags = IORESOURCE_IRQ;
    res[1].start = res[1].end = irq;

    pdev = platform_device_alloc(devname, id);
    if(!pdev)
    {
        printk(KERN_ERR "Error Failed to allocate platform device for devname=%s id=%d\n",
                devname, id);
        return 0;
    }

    platform_device_add_resources(pdev, res, 2);

    pdev->dev.dma_mask = (u64 *)&usb_dmamask;
    pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

    if(private_data)
    {
        pdev->dev.platform_data = private_data;
    }

    if(platform_device_add(pdev))
    {
        printk(KERN_ERR "Error Failed to add platform device for devname=%s id=%d\n",
                devname, id);
        return 0;
    }

    return pdev;
}


#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM94908)

#define XHCI_ECIRA_BASE USB_XHCI_BASE + 0xf90

uint32_t xhci_ecira_read(uint32_t reg)
{
    volatile uint32_t *addr;
    uint32_t value;

    addr = (uint32_t *)(XHCI_ECIRA_BASE + 8);
    *addr =reg;

    addr = (uint32_t *)(XHCI_ECIRA_BASE + 0xc);
    value = *addr; 

    return value;
}

void xhci_ecira_write(uint32_t reg, uint32_t value)
{

    volatile uint32_t *addr;

    addr = (uint32_t *)(XHCI_ECIRA_BASE + 8);
    *addr =reg;

    addr = (uint32_t *)(XHCI_ECIRA_BASE + 0xc);
    *addr =value; 
}

#endif

#if defined(CONFIG_BCM963138)
static void bcm63138B0_manual_usb_ldo_start(void)
{
#if 0
    /* first disable FSM but also leave it that way
       to allow normal suspend/resume */
    USBH_CTRL->utmi_ctl_1 &= ~0x08008000;

    /* reset USB 2.0 PLL */
    USBH_CTRL->pll_ctl &= ~(1 << 30); /*pll_resetb=0*/
    mdelay(1);
    USBH_CTRL->pll_ctl |= ( 1 << 30); /*pll_resetb=1*/
    mdelay(10);
#else
    USBH_CTRL->pll_ctl &= ~(1 << 30); /*pll_resetb=0*/
    USBH_CTRL->utmi_ctl_1 = 0; 
    USBH_CTRL->pll_ldo_ctl = 4; /*ldo_ctl=core_rdy */
    USBH_CTRL->pll_ctl |= ( 1 << 31); /*pll_iddq=1*/
    mdelay(10);
    USBH_CTRL->pll_ctl &= ~( 1 << 31); /*pll_iddq=0*/
    USBH_CTRL->pll_ldo_ctl |= 1; /*ldo_ctl.AFE_LDO_PWRDWNB=1*/
    USBH_CTRL->pll_ldo_ctl |= 2; /*ldo_ctl.AFE_BG_PWRDWNB=1*/
    mdelay(1);
    USBH_CTRL->utmi_ctl_1 = 0x00020002;/* utmi_resetb &ref_clk_sel=0; */ 
    USBH_CTRL->pll_ctl |= ( 1 << 30); /*pll_resetb=1*/
    mdelay(10);
#endif
}    
static void bcm63138B0_usb3_erdy_nump_bypass(void)
{
    uint32_t value;

    value = xhci_ecira_read(0xa20c);
    value |= 0x10000;
    xhci_ecira_write(0xa20c, value);
}

#endif

#define MDIO_USB2   0
#define MDIO_USB3   (1 << 31)

static uint32_t usb_mdio_read(volatile uint32_t *mdio, uint32_t reg, int mode)
{
    uint32_t data;

    data = (reg << 16) | mode;
    mdio[0] = data;
    data |= (1 << 24);
    mdio[0] = data;
    mdelay(1);
    data &= ~(1 << 24);
    mdelay(1);

    return (mdio[1] & 0xffff);
}

static void usb_mdio_write(volatile uint32_t *mdio, uint32_t reg, uint32_t val, int mode)
{
    uint32_t data;
    data = (reg << 16) | val | mode;
    *mdio = data;
    data |= (1 << 25);
    *mdio = data;
    mdelay(1);
    data &= ~(1 << 25);
    *mdio = data;
}

static void usb2_eye_fix(void)
{
    /* Updating USB 2.0 PHY registers */
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, 0x80a0, MDIO_USB2);
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x0a, 0xc6a0, MDIO_USB2);
}

static void usb3_ssc_enable(void)
{
    uint32_t val;

    /* Enable USB 3.0 TX spread spectrum */
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, 0x8040, MDIO_USB3);
    val = usb_mdio_read((void *)&USBH_CTRL->mdio, 0x01, MDIO_USB3) | 0x0f;
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x01, val, MDIO_USB3);

    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, 0x9040, MDIO_USB3);
    val = usb_mdio_read((void *)&USBH_CTRL->mdio, 0x01, MDIO_USB3) | 0x0f;
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x01, val, MDIO_USB3);
}

#if defined(CONFIG_BCM963138)
static int bcm_usb_host_hw_init_63138(void)
{
    short usb_gpio;
    uint32_t val;

    printk("++++ Powering up USB blocks\n");
    if(pmc_usb_power_up(PMC_USB_HOST_ALL))
    {
        printk(KERN_ERR "+++ Failed to Power Up USB Host\n");
        return -1;
    }

    mdelay(1);

    /* adjust over current & port power polarity */
    USBH_CTRL->setup |= (USBH_IOC);
    if(BpGetUsbPwrFlt0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) !=  BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IOC);
        }
    }

    USBH_CTRL->setup |= (USBH_IPP);
    if(BpGetUsbPwrOn0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) != BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IPP);
        }
    }

    /*enable SSC for usb3.0 */
    usb3_ssc_enable();

    mdelay(300);

    /*initialize XHCI settings*/
    bcm63138B0_manual_usb_ldo_start();
    USBH_CTRL->usb_pm |= XHC_SOFT_RESETB;
    USBH_CTRL->usb30_ctl1 &= ~PHY3_PLL_SEQ_START;

    USBH_CTRL->usb30_ctl1 |= PHY3_PLL_SEQ_START;


    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix();

    /*initialize EHCI & OHCI settings*/
    USBH_CTRL->bridge_ctl &= ~(EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP);

    /* reset host controllers for possible fake overcurrent indications */ 
    val = USBH_CTRL->usb_pm;
    USBH_CTRL->usb_pm = 0;
    USBH_CTRL->usb_pm = val;
    mdelay(1);

    bcm63138B0_usb3_erdy_nump_bypass();

#if defined(CONFIG_USB_UAS)
    {
        /* workaround for UAS to work. To do, define these register in map.h */
        volatile unsigned int* USB_XHCI_EC_ECHHST = (volatile unsigned int*)(USB_XHCI_BASE+0xfa0);
        *USB_XHCI_EC_ECHHST = (*USB_XHCI_EC_ECHHST)&0x7fffffff;
    }
#endif

    // Reduce accesses to DDR during USB idle time when
    // Self-Refresh feature is compiled in
    USBH_CTRL->usb_simctl |= USBH_OHCI_MEM_REQ_DIS;

    return 0;
}

#elif defined(CONFIG_BCM963148)

static int bcm_usb_host_hw_init_63148(void)
{
    short usb_gpio;
    uint32_t val;

    printk("++++ Powering up USB blocks\n");
    if(pmc_usb_power_up(PMC_USB_HOST_ALL))
    {
        printk(KERN_ERR "+++ Failed to Power Up USB Host\n");
        return -1;
    }

    mdelay(1);

    /* adjust over current & port power polarity */
    USBH_CTRL->setup |= (USBH_IOC);
    if(BpGetUsbPwrFlt0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) !=  BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IOC);
        }
    }

    USBH_CTRL->setup |= (USBH_IPP);
    if(BpGetUsbPwrOn0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) != BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IPP);
        }
    }

    /*enable SSC for usb3.0 */
    usb3_ssc_enable();

    mdelay(300);

    /*initialize XHCI settings*/
    USBH_CTRL->usb30_ctl1 |= USB3_IOC;
    USBH_CTRL->usb30_ctl1 |= XHC_SOFT_RESETB;


    USBH_CTRL->usb30_ctl1 |= PHY3_PLL_SEQ_START;

    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix();


    /*initialize EHCI & OHCI settings*/
    USBH_CTRL->bridge_ctl &= ~(EHCI_ENDIAN_SWAP | OHCI_ENDIAN_SWAP);

    /* reset host controllers for possible fake overcurrent indications */ 
    val = USBH_CTRL->usb_pm;
    USBH_CTRL->usb_pm = 0;
    USBH_CTRL->usb_pm = val;
    mdelay(1);

    // Reduce accesses to DDR during USB idle time when
    // Self-Refresh feature is compiled in
    USBH_CTRL->usb_simctl |= USBH_OHCI_MEM_REQ_DIS;

    return 0;
}
#elif defined(CONFIG_BCM94908)

static void usb3_enable_pipe_reset(void)
{
    uint32_t val;

    /* Re-enable USB 3.0 pipe reset */
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, 0x8000, MDIO_USB3);
    val = usb_mdio_read((void *)&USBH_CTRL->mdio, 0x0f, MDIO_USB3) | 0x200;
    usb_mdio_write((void *)&USBH_CTRL->mdio, 0x0f, val, MDIO_USB3);
}


static void usb3_enable_sigdet(void)
{
    uint32_t val, ofs;
    int ii;

    ofs = 0;
    for (ii = 0; ii < 2; ++ii)
    {
        /* Set correct default for sigdet */
        usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, (0x8080 + ofs), MDIO_USB3);
        val = usb_mdio_read((void *)&USBH_CTRL->mdio, 0x05, MDIO_USB3);
        val = (val & ~0x800f) | 0x800d;
        usb_mdio_write((void *)&USBH_CTRL->mdio, 0x05, val, MDIO_USB3);
        ofs = 0x1000;
    }
}


static void usb3_enable_skip_align(void)
{
    uint32_t val, ofs;
    int ii;

    ofs = 0;
    for (ii = 0; ii < 2; ++ii)
    {
        /* Set correct default for SKIP align */
        usb_mdio_write((void *)&USBH_CTRL->mdio, 0x1f, (0x8060 + ofs), MDIO_USB3);
        val = usb_mdio_read((void *)&USBH_CTRL->mdio, 0x01, MDIO_USB3) | 0x200;
        usb_mdio_write((void *)&USBH_CTRL->mdio, 0x01, val, MDIO_USB3);
        ofs = 0x1000;
    }
}

static void usb3_enable_recovery_pipe_reset(void)
{
    uint32_t value, reg;
    int ii;

    reg = 0xc410;
    for (ii = 0; ii < 2; ++ii)
    {
        value = xhci_ecira_read(reg);
        value |= (1 << 29);
        xhci_ecira_write(reg, value);
	reg += 0x40;
    }

}

static int bcm_usb_host_hw_init_4908(void)
{
    short usb_gpio;
    uint32_t val;

    printk("++++ Powering up USB blocks\n");
    if(pmc_usb_power_up(PMC_USB_HOST_ALL))
    {
        printk(KERN_ERR "+++ Failed to Power Up USB Host\n");
        return -1;
    }

    mdelay(1);

    /* adjust over current & port power polarity */
    USBH_CTRL->setup |= (USBH_IOC);
    if(BpGetUsbPwrFlt0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) !=  BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IOC);
        }
    }

    USBH_CTRL->setup |= (USBH_IPP);
    if(BpGetUsbPwrOn0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) != BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IPP);
        }
    }

    /*enable USB PHYs*/
    USBH_CTRL->usb_pm &= ~(USB_PWRDWN);
    mdelay(1);

    /*initialize XHCI settings*/

    /*enable SSC for usb3.0 */
    usb3_ssc_enable();

    usb3_enable_pipe_reset();
    usb3_enable_sigdet();
    usb3_enable_skip_align();

    mdelay(300);

    USBH_CTRL->usb30_ctl1 |= PHY3_PLL_SEQ_START;
    USBH_CTRL->usb_pm |= XHC_SOFT_RESETB;

    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix();

    /*initialize EHCI & OHCI settings*/
    /* no swap for data & desciptors */    
    USBH_CTRL->bridge_ctl &= ~(0xf); /*clear lower 4 bits */

    /* reset host controllers for possible fake overcurrent indications */ 
    val = USBH_CTRL->usb_pm;
    USBH_CTRL->usb_pm = 0;
    USBH_CTRL->usb_pm = val;
    mdelay(1);

    // Reduce accesses to DDR during USB idle time when
    // Self-Refresh feature is compiled in
    USBH_CTRL->usb_simctl |= USBH_OHCI_MEM_REQ_DIS;

    return 0;
}
#elif defined(CONFIG_BCM96858)

static int bcm_usb_host_hw_init_6858(void)
{
    short usb_gpio;
    uint32_t val;

    printk("++++ Powering up USB blocks\n");
    if(pmc_usb_power_up(PMC_USB_HOST_ALL))
    {
        printk(KERN_ERR "+++ Failed to Power Up USB Host\n");
        return -1;
    }

    mdelay(1);

    /* adjust over current & port power polarity */
    USBH_CTRL->setup |= (USBH_IOC);
    if(BpGetUsbPwrFlt0(&usb_gpio) == BP_SUCCESS)
    {
        if((usb_gpio & BP_ACTIVE_MASK) !=  BP_ACTIVE_LOW)
        {
            USBH_CTRL->setup &= ~(USBH_IOC);
        }
    }

    val = USBH_CTRL->setup;

    /*by default we use strap to determine polarity of port power */
    if(!(val & USBH_STRAP_IPP_SEL))
    {
        USBH_CTRL->setup |= (USBH_IPP);
        if(BpGetUsbPwrOn0(&usb_gpio) == BP_SUCCESS)
        {
            if((usb_gpio & BP_ACTIVE_MASK) != BP_ACTIVE_LOW)
            {
                USBH_CTRL->setup &= ~(USBH_IPP);
            }
        }
    }

    /*enable SSC for usb3.0 */
    usb3_ssc_enable();

    mdelay(300);
    /*initialize XHCI settings*/
    USBH_CTRL->usb_pm &= ~(USB_PWRDWN);
    mdelay(1);

    USBH_CTRL->usb30_ctl1 |= PHY3_PLL_SEQ_START;
    USBH_CTRL->usb_pm |= XHC_SOFT_RESETB;

    /*adjust the default AFE settings for better eye diagrams */
    usb2_eye_fix();

    /*initialize EHCI & OHCI settings*/
    /* no swap for data & desciptors */    
    USBH_CTRL->bridge_ctl &= ~(0xf); /*clear lower 4 bits */

    /* reset host controllers for possible fake overcurrent indications */ 
    val = USBH_CTRL->usb_pm;
    USBH_CTRL->usb_pm = 0;
    USBH_CTRL->usb_pm = val;
    mdelay(1);

    // Reduce accesses to DDR during USB idle time when
    // Self-Refresh feature is compiled in
    USBH_CTRL->usb_simctl |= USBH_OHCI_MEM_REQ_DIS;

    return 0;
}
#endif

#define _BCM_USB_HOST_HW_INIT(chip_num)  bcm_usb_host_hw_init_##chip_num()
#define BCM_USB_HOST_HW_INIT(chip_num)  _BCM_USB_HOST_HW_INIT(chip_num)

static __init int bcm_add_usb_hosts(void)
{
   
     int err;

     err = BCM_USB_HOST_HW_INIT(CONFIG_BCM_CHIP_NUMBER);
     if(err < 0)
     {
         printk(KERN_ERR "++++ USB Host HW initialization failed \n");
         return err;
     }


    /* add to platform devices */
    xhci_dev = bcm_add_usb_host(CAP_TYPE_XHCI, 0, USB_XHCI_PHYS_BASE,
        0x1000, INTERRUPT_ID_USB_XHCI, "xhci-hcd", NULL);
    ehci_dev = bcm_add_usb_host(CAP_TYPE_EHCI, 0, USB_EHCI_PHYS_BASE,
        0x100, INTERRUPT_ID_USB_EHCI, "ehci-platform", &bcm_ehci_pdata);
    ohci_dev = bcm_add_usb_host(CAP_TYPE_OHCI, 0, USB_OHCI_PHYS_BASE,
        0x100, INTERRUPT_ID_USB_OHCI, "ohci-platform", &bcm_ohci_pdata);
#if defined(CONFIG_BCM96858)
    ehci1_dev = bcm_add_usb_host(CAP_TYPE_EHCI, 1, USB_EHCI1_PHYS_BASE,
        0x100, INTERRUPT_ID_USB_EHCI1, "ehci-platform", &bcm_ehci1_pdata);
    ohci1_dev = bcm_add_usb_host(CAP_TYPE_OHCI, 1, USB_OHCI1_PHYS_BASE,
        0x100, INTERRUPT_ID_USB_OHCI1, "ohci-platform", &bcm_ohci1_pdata);
#endif

#if defined(CONFIG_BCM94908)
    usb3_enable_recovery_pipe_reset();
#endif
    return 0;
}

#if defined CONFIG_USB_MODULE || defined CONFIG_USB_XHCI_HCD_MODULE
static void bcm_mod_cleanup(void)
{
    // we want to just disable usb interrupts and power down usb
    // we'll probably be restart later, re-add resources ok then?
    platform_device_del(xhci_dev);
    platform_device_del(ehci_dev);
    platform_device_del(ohci_dev);
#if defined(CONFIG_BCM96858)
    platform_device_del(ehci1_dev);
    platform_device_del(ohci1_dev);
#endif
    pmc_usb_power_down(PMC_USB_HOST_ALL);
    mdelay(1);
}

module_init(bcm_add_usb_hosts);
module_exit(bcm_mod_cleanup);

MODULE_LICENSE("GPL");
#else
arch_initcall(bcm_add_usb_hosts);
#endif

#endif /* defined(CONFIG_USB) || defined(CONFIG_USB_MODULE) */
#endif
