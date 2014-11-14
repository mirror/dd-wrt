#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/platform_device.h>
#include <ar231x_platform.h>
#include <ar231x.h>
#include "devices.h"
#include "ar5312.h"
#include "ar2315.h"

struct ar231x_board_config ar231x_board;
int ar231x_devtype = DEV_TYPE_UNKNOWN;
const struct ar231x_gpiodev *ar231x_gpiodev;
EXPORT_SYMBOL(ar231x_gpiodev);

static struct resource ar231x_eth0_res[] = {
	{
		.name = "eth0_membase",
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "eth0_irq",
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource ar231x_eth1_res[] = {
	{
		.name = "eth1_membase",
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "eth1_irq",
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device ar231x_eth[] = {
	{
		.id = 0,
		.name = "ar231x-eth",
		.resource = ar231x_eth0_res,
		.num_resources = ARRAY_SIZE(ar231x_eth0_res)
	},
	{
		.id = 1,
		.name = "ar231x-eth",
		.resource = ar231x_eth1_res,
		.num_resources = ARRAY_SIZE(ar231x_eth1_res)
	}
};

static struct resource ar231x_wmac0_res[] = {
	{
		.name = "wmac0_membase",
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "wmac0_irq",
		.flags = IORESOURCE_IRQ,
	}
};

static struct resource ar231x_wmac1_res[] = {
	{
		.name = "wmac1_membase",
		.flags = IORESOURCE_MEM,
	},
	{
		.name = "wmac1_irq",
		.flags = IORESOURCE_IRQ,
	}
};


static struct platform_device ar231x_wmac[] = {
	{
		.id = 0,
		.name = "ar231x-wmac",
		.resource = ar231x_wmac0_res,
		.num_resources = ARRAY_SIZE(ar231x_wmac0_res),
		.dev.platform_data = &ar231x_board,
	},
	{
		.id = 1,
		.name = "ar231x-wmac",
		.resource = ar231x_wmac1_res,
		.num_resources = ARRAY_SIZE(ar231x_wmac1_res),
		.dev.platform_data = &ar231x_board,
	},
};


static const char *devtype_strings[] = {
	[DEV_TYPE_AR5312] = "Atheros AR5312",
	[DEV_TYPE_AR2312] = "Atheros AR2312",
	[DEV_TYPE_AR2313] = "Atheros AR2313",
	[DEV_TYPE_AR2315] = "Atheros AR2315",
	[DEV_TYPE_AR2316] = "Atheros AR2316",
	[DEV_TYPE_AR2317] = "Atheros AR2317",
	[DEV_TYPE_AR2318] = "Atheros AR2318",
	[DEV_TYPE_UNKNOWN] = "Atheros (unknown)",
};

const char *get_system_type(void)
{
	if ((ar231x_devtype >= ARRAY_SIZE(devtype_strings)) ||
		!devtype_strings[ar231x_devtype])
		return devtype_strings[DEV_TYPE_UNKNOWN];
	return devtype_strings[ar231x_devtype];
}


const char *get_arch_type(void)
{
	switch (ar231x_devtype) {
#ifdef CONFIG_ATHEROS_AR5312
	case DEV_TYPE_AR5312:
	case DEV_TYPE_AR2312:
	case DEV_TYPE_AR2313:
		return "Atheros AR5312";
#endif
#ifdef CONFIG_ATHEROS_AR5315
	case DEV_TYPE_AR2315:
	case DEV_TYPE_AR2316:
	case DEV_TYPE_AR2317:
	case DEV_TYPE_AR2318:
		return "Atheros AR5315";
#endif
	}
	return "Atheros (unknown)";
}
EXPORT_SYMBOL(get_arch_type);

int __init
ar231x_add_ethernet(int nr, u32 base, int irq, void *pdata)
{
	struct resource *res;

	ar231x_eth[nr].dev.platform_data = pdata;
	res = &ar231x_eth[nr].resource[0];
	res->start = base;
	res->end = base + 0x2000 - 1;
	res++;
	res->start = irq;
	res->end = irq;
	return platform_device_register(&ar231x_eth[nr]);
}

void __init
ar231x_serial_setup(u32 mapbase, unsigned int uartclk)
{
	struct uart_port s;

	memset(&s, 0, sizeof(s));

	s.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST;
	s.iotype = UPIO_MEM;
	s.irq = AR531X_MISC_IRQ_UART0;
	s.regshift = 2;
	s.mapbase = mapbase;
	s.uartclk = uartclk;
	s.membase = (void __iomem *)s.mapbase;

	early_serial_setup(&s);
}

int __init
ar231x_add_wmac(int nr, u32 base, int irq)
{
	struct resource *res;

	ar231x_wmac[nr].dev.platform_data = &ar231x_board;
	res = &ar231x_wmac[nr].resource[0];
	res->start = base;
	res->end = base + 0x10000 - 1;
	res++;
	res->start = irq;
	res->end = irq;
	return platform_device_register(&ar231x_wmac[nr]);
}

static int __init ar231x_register_devices(void)
{
	static struct resource res = {
		.start = 0xFFFFFFFF,
	};

	platform_device_register_simple("GPIODEV", 0, &res, 1);
	ar5312_init_devices();
	ar2315_init_devices();

	return 0;
}

device_initcall(ar231x_register_devices);
