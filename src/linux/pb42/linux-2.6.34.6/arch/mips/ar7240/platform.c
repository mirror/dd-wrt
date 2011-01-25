#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <linux/console.h>
#include <asm/serial.h>

#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>

#include <asm/mach-ar7240/ar7240.h>
#include <asm/mach-ar71xx/ar71xx.h>

#ifdef CONFIG_WASP_SUPPORT
extern uint32_t ath_ref_clk_freq;
#else
extern uint32_t ar7240_ahb_freq;
#endif

/* 
 * OHCI (USB full speed host controller) 
 */
static struct resource ar7240_usb_ohci_resources[] = {
	[0] = {
		.start		= AR7240_USB_OHCI_BASE,
		.end		= AR7240_USB_OHCI_BASE + AR7240_USB_WINDOW - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= AR7240_CPU_IRQ_USB,
        .end	    = AR7240_CPU_IRQ_USB,
		.flags		= IORESOURCE_IRQ,
	},
};

/* 
 * The dmamask must be set for OHCI to work 
 */
static u64 ohci_dmamask = ~(u32)0;
static struct platform_device ar7240_usb_ohci_device = {
	.name		= "ar7240-ohci",
	.id		    = 0,
	.dev = {
		.dma_mask		= &ohci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ar7240_usb_ohci_resources),
	.resource	= ar7240_usb_ohci_resources,
};

/* 
 * EHCI (USB full speed host controller) 
 */
static struct resource ar7240_usb_ehci_resources[] = {
	[0] = {
		.start		= AR7240_USB_EHCI_BASE,
		.end		= AR7240_USB_EHCI_BASE + AR7240_USB_WINDOW - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= AR7240_CPU_IRQ_USB,
		.end		= AR7240_CPU_IRQ_USB,
		.flags		= IORESOURCE_IRQ,
	},
};

/* 
 * The dmamask must be set for EHCI to work 
 */
static u64 ehci_dmamask = ~(u32)0;

static struct platform_device ar7240_usb_ehci_device = {
	.name		= "ar71xx-ehci",
	.id		    = 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ar7240_usb_ehci_resources),
	.resource	= ar7240_usb_ehci_resources,
};



static struct resource ar7240_uart_resources[] = {
	{
		.start = AR7240_UART_BASE,
		.end = AR7240_UART_BASE+0x0fff,
		.flags = IORESOURCE_MEM,
	},
};

#define AR71XX_UART_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP)

static struct plat_serial8250_port ar7240_uart_data[] = {
	{
                .mapbase        = AR7240_UART_BASE,
                .irq            = AR7240_MISC_IRQ_UART,
                .flags          = AR71XX_UART_FLAGS,
                .iotype         = UPIO_MEM32,
                .regshift       = 2,
                .uartclk        = 0, /* ar7240_ahb_freq, */
	},
        { },
};

static struct platform_device ar7240_uart = {
	 .name               = "serial8250",
        .id                 = 0,
        .dev.platform_data  = ar7240_uart_data,
        .num_resources      = 1, 
        .resource           = ar7240_uart_resources

};

static struct platform_device *ar7241_platform_devices[] __initdata = {
	&ar7240_usb_ehci_device
};

static struct platform_device *ar7240_platform_devices[] __initdata = {
	&ar7240_usb_ohci_device
};

static struct platform_device *ar724x_platform_devices[] __initdata = {
	&ar7240_uart
};

extern __init ap91_pci_init(u8 *cal_data, u8 *mac_addr);

static void *getCalData(int slot)
{
u8 *base;
for (base=(u8 *) KSEG1ADDR(0x1f000000);base<KSEG1ADDR (0x1fff0000);base+=0x1000) {
	u32 *cal = (u32 *)base;
	if (*cal==0xa55a0000 || *cal==0x5aa50000) { //protection bit is always zero on inflash devices, so we can use for match it
		if (slot) {
			base+=0x4000;
		}
		printk(KERN_INFO "found calibration data for slot %d on 0x%08X\n",slot,base);
		return base;
	}
    }
return NULL;
}

enum ar71xx_soc_type ar71xx_soc;
EXPORT_SYMBOL_GPL(ar71xx_soc);

int __init ar7240_platform_init(void)
{
	int ret;
	void *ee;
        /* need to set clock appropriately */
#ifdef CONFIG_WASP_SUPPORT
	ar7240_uart_data[0].uartclk = ath_ref_clk_freq;
#else
	ar7240_uart_data[0].uartclk = ar7240_ahb_freq;
#endif
	ret = platform_add_devices(ar724x_platform_devices, 
                                ARRAY_SIZE(ar724x_platform_devices));

        if (ret < 0)
		return ret; 

	if (is_ar7241() || is_ar7242()  || is_ar9330() || is_wasp()) {
	    ret = platform_add_devices(ar7241_platform_devices, 
                                ARRAY_SIZE(ar7241_platform_devices));
        }
        if (is_ar7240()) {
	    ret = platform_add_devices(ar7240_platform_devices, 
                                ARRAY_SIZE(ar7240_platform_devices));
        }
        
	ee = getCalData(0);
	ap91_pci_init(ee, NULL);
return ret;
}

arch_initcall(ar7240_platform_init);
    