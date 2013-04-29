/*
 * arch/arm/mach-ixp4xx/cambria-setup.c
 *
 * Cambria board-setup 
 *
 * Copyright (C) 2008 Gateworks Corporation
 *
 * Author: Chris Lang <clang@gateworks.com
 *
 *
 * Based largely on ixdp425_setup.c
 * 		Author: Deepak Saxena <dsaxena@plexity.net>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/i2c/at24.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/slab.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/io.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/leds.h>


static struct flash_platform_data cambria_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource cambria_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device cambria_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data = &cambria_flash_data,
	},
	.num_resources	= 1,
	.resource	= &cambria_flash_resource,
};

static struct ixp4xx_i2c_pins cambria_i2c_gpio_pins = {
	.sda_pin	= CAMBRIA_SDA_PIN,
	.scl_pin	= CAMBRIA_SCL_PIN,
};

static struct platform_device cambria_i2c_controller = {
	.name		= "IXP4XX-I2C",
	.id		= 0,
	.dev		= {
		.platform_data = &cambria_i2c_gpio_pins,
	},
	.num_resources	= 0
};

static struct resource cambria_uart_resources[] = {
	{
		.start		= IXP4XX_UART1_BASE_PHYS,
		.end		= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port cambria_uart_data[] = {
	{
		.mapbase	= IXP4XX_UART1_BASE_PHYS,
		.membase	= (char *)IXP4XX_UART1_BASE_VIRT + REG_OFFSET,
		.irq		= IRQ_IXP4XX_UART1,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL,
	},
	{ },
};

static struct platform_device cambria_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= cambria_uart_data,
	.num_resources		= 1,
	.resource		= cambria_uart_resources
};




static struct resource cambria_pata_resources[] = {
  {
    .flags  = IORESOURCE_MEM
  },
  {
    .flags  = IORESOURCE_MEM,
  },
  {
    .name = "intrq",
    .start  = IRQ_IXP4XX_GPIO12,
    .end  = IRQ_IXP4XX_GPIO12,
    .flags  = IORESOURCE_IRQ,
  },
};

static struct ixp4xx_pata_data cambria_pata_data = {
  .cs0_bits = 0xbfff3c03,
	.cs1_bits = 0xbfff3c03,
};

static struct platform_device cambria_pata = {
  .name     = "pata_ixp4xx_cf",
  .id     = 0,
  .dev.platform_data      = &cambria_pata_data,
  .num_resources    = ARRAY_SIZE(cambria_pata_resources),
  .resource   = cambria_pata_resources,
};

static struct platform_device cambria_leds_pld = {
  .name   = "IXP4XX-PLD-LED",
  .id   = -1,
  .num_resources  = 0,
  //.resource = avila_led_resources,
};

static struct platform_device cambria_leds_mem = {
  .name   = "IXP4XX-MEM-LED",
  .id   = -1,
  .num_resources  = 0,
  //.resource = avila_led_resources,
};

static struct resource cambria_usb0_resources[] = {
	{
		.start	= 0xCD000000,
		.end	= 0xCD000300,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 32,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource cambria_usb1_resources[] = {
	{
		.start	= 0xCE000000,
		.end	= 0xCE000300,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 33,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 ehci_dma_mask = ~(u32)0;

static struct platform_device cambria_usb0_device =  {
	.name		= "ixp4xx-ehci",
	.id		= 0,
	.resource	= cambria_usb0_resources,
	.num_resources	= ARRAY_SIZE(cambria_usb0_resources),
	.dev = {
		.dma_mask		= &ehci_dma_mask,
		.coherent_dma_mask	= 0xffffffff,
	},
};

static struct platform_device cambria_usb1_device = {
	.name		= "ixp4xx-ehci",
	.id		= 1,
	.resource	= cambria_usb1_resources,
	.num_resources	= ARRAY_SIZE(cambria_usb1_resources),
	.dev = {
		.dma_mask		= &ehci_dma_mask,
		.coherent_dma_mask	= 0xffffffff,
	},
};




static struct platform_device *cambria_devices[] __initdata = {
	&cambria_i2c_controller,
	&cambria_flash,
	&cambria_uart,
	&cambria_leds_pld,
	&cambria_leds_mem
};

static struct at24_platform_data cambria_eeprom_info = {
	.byte_len	= 1024,
	.page_size	= 16,
	.flags		= AT24_FLAG_READONLY,
//	.setup		= at24_setup,
};

static struct i2c_board_info __initdata cambria_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("ds1672", 0x68),
	},
	{
		I2C_BOARD_INFO("ad7418", 0x28),
	},
	{
		I2C_BOARD_INFO("24c08", 0x51),		
		.platform_data	= &cambria_eeprom_info
	},
};


static struct gpio_led generic_leds_gpio[] __initdata = {
	{
		.name		= "generic_0",
		.gpio		= 0,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_1",
		.gpio		= 1,
		.active_low	= 0,
		
	}, 
	{
		.name		= "generic_2",
		.gpio		= 2,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_3",
		.gpio		= 3,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_4",
		.gpio		= 4,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_5",
		.gpio		= 5,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_6",
		.gpio		= 6,
		.active_low	= 0,
	}, 
	{
		.name		= "generic_7",
		.gpio		= 7,
		.active_low	= 0,
	}, 
};

extern void setLED(int led, int status);

void cambria_led_set_value(struct gpio_chip *chip,
				  unsigned offset, int value)
{
	setLED(offset,value);
}

int cambria_led_get_value(struct gpio_chip *chip, unsigned offset)
{
	return 0;
}

static int cambria_gpio_direction_input(struct gpio_chip *chip,
				       unsigned offset)
{

	return 0;
}

static int cambria_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	setLED(offset,value);
	return 0;
}


static struct gpio_chip cambria_gpio_chip = {
	.label			= "cambria-led",
	.get			= cambria_led_get_value,
	.set			= cambria_led_set_value,
	.direction_input	= cambria_gpio_direction_input,
	.direction_output	= cambria_gpio_direction_output,
	.base			= 0,
	.ngpio			= 8,
};

void __init cambria_add_device_leds_gpio(int id, unsigned num_leds,
					struct gpio_led *leds)
{
	struct platform_device *pdev;
	struct gpio_led_platform_data pdata;
	struct gpio_led *p;
	int err;

	p = kmalloc(num_leds * sizeof(*p), GFP_KERNEL);
	if (!p)
		return;

	memcpy(p, leds, num_leds * sizeof(*p));

	pdev = platform_device_alloc("leds-gpio", id);
	if (!pdev)
		goto err_free_leds;

	memset(&pdata, 0, sizeof(pdata));
	pdata.num_leds = num_leds;
	pdata.leds = p;

	err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
	if (err)
		goto err_put_pdev;

	err = platform_device_add(pdev);
	if (err)
		goto err_put_pdev;

	return;

err_put_pdev:
	platform_device_put(pdev);

err_free_leds:
	kfree(p);
}




static void __init cambria_init(void)
{
	ixp4xx_sys_init();

	cambria_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	cambria_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + ixp4xx_exp_bus_size + ixp4xx_exp_bus_size - 1;


  platform_add_devices(cambria_devices, ARRAY_SIZE(cambria_devices));

  cambria_pata_resources[0].start = 0x53e00000;
  cambria_pata_resources[0].end = 0x53e3ffff;

  cambria_pata_resources[1].start = 0x53e40000;
  cambria_pata_resources[1].end = 0x53e7ffff;

  cambria_pata_data.cs0_cfg = IXP4XX_EXP_CS3;
  cambria_pata_data.cs1_cfg = IXP4XX_EXP_CS3;


  platform_device_register(&cambria_usb0_device);
  platform_device_register(&cambria_usb1_device);
  platform_device_register(&cambria_pata);
  i2c_register_board_info(0, cambria_i2c_board_info, ARRAY_SIZE(cambria_i2c_board_info));
  
  int err;
  err = gpiochip_add(&cambria_gpio_chip);
  if (err)
	panic("cannot add Cambria GPIO chip, error=%d", err);

	int i;
	for (i=0;i<sizeof(generic_leds_gpio)/sizeof(struct gpio_led);i++) {
		generic_leds_gpio[i].default_state = LEDS_GPIO_DEFSTATE_KEEP;
	}

    cambria_add_device_leds_gpio(-1,sizeof(generic_leds_gpio)/sizeof(struct gpio_led),generic_leds_gpio);

}


MACHINE_START(CAMBRIA, "Gateworks Cambria Series")
  /* Maintainer: Chris Lang <clang@gateworks.com> */
  .map_io   = ixp4xx_map_io,
  .init_early	= ixp4xx_init_early,
  .init_irq = ixp4xx_init_irq,
  .init_time	= ixp4xx_timer_init,
  .atag_offset  = 0x0100,
  .init_machine = cambria_init,
#if defined(CONFIG_PCI)
  .dma_zone_size	= SZ_64M,
#endif
    .restart	= ixp4xx_restart,
MACHINE_END

