#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/gpio_buttons.h>
#include <linux/etherdevice.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/dm9000.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/eeprom.h>
#include <falcon/lantiq_soc.h>

#include <dev-gpio-leds.h>

#include "../machtypes.h"

#include "devices.h"

#define EASY98000_GPIO_LED_0 9
#define EASY98000_GPIO_LED_1 10
#define EASY98000_GPIO_LED_2 11
#define EASY98000_GPIO_LED_3 12
#define EASY98000_GPIO_LED_4 13
#define EASY98000_GPIO_LED_5 14

static unsigned char ltq_ethaddr[6] = {0};

static int __init falcon_set_ethaddr(char *str)
{
	sscanf(str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		&ltq_ethaddr[0], &ltq_ethaddr[1], &ltq_ethaddr[2],
		&ltq_ethaddr[3], &ltq_ethaddr[4], &ltq_ethaddr[5]);
	return 0;
}
__setup("ethaddr=", falcon_set_ethaddr);

static struct mtd_partition easy98000_nor_partitions[] =
{
	{
		.name	= "uboot",
		.offset	= 0x0,
		.size	= 0x40000,
	},
	{
		.name	= "uboot_env",
		.offset	= 0x40000,
		.size	= 0x40000,	/* 2 sectors for redundant env. */
	},
	{
		.name	= "linux",
		.offset	= 0x80000,
		.size	= 0xF80000,	/* map only 16 MiB */
	},
};

static struct physmap_flash_data easy98000_nor_flash_data = {
	.nr_parts	= ARRAY_SIZE(easy98000_nor_partitions),
	.parts		= easy98000_nor_partitions,
};

static struct flash_platform_data easy98000_spi_flash_platform_data = {
	.name = "sflash",
	.parts = easy98000_nor_partitions,
	.nr_parts = ARRAY_SIZE(easy98000_nor_partitions)
};

static struct spi_board_info easy98000_spi_flash_data __initdata = {
	.modalias		= "m25p80",
	.bus_num		= 0,
	.chip_select		= 0,
	.max_speed_hz		= 10 * 1000 * 1000,
	.mode			= SPI_MODE_3,
	.platform_data		= &easy98000_spi_flash_platform_data
};

static struct gpio_led easy98000_gpio_leds[] __initdata = {
	{
		.name		= "easy98000:green:0",
		.gpio		= EASY98000_GPIO_LED_0,
		.active_low	= 0,
	}, {
		.name		= "easy98000:green:1",
		.gpio		= EASY98000_GPIO_LED_1,
		.active_low	= 0,
	}, {
		.name		= "easy98000:green:2",
		.gpio		= EASY98000_GPIO_LED_2,
		.active_low	= 0,
	}, {
		.name		= "easy98000:green:3",
		.gpio		= EASY98000_GPIO_LED_3,
		.active_low	= 0,
	}, {
		.name		= "easy98000:green:4",
		.gpio		= EASY98000_GPIO_LED_4,
		.active_low	= 0,
	}, {
		.name		= "easy98000:green:5",
		.gpio		= EASY98000_GPIO_LED_5,
		.active_low	= 0,
	}
};

#define CONFIG_DM9000_BASE		0x14000000
#define DM9000_IO			(CONFIG_DM9000_BASE + 3)
#define DM9000_DATA			(CONFIG_DM9000_BASE + 1)

static struct dm9000_plat_data dm9000_plat_data = {
	.flags = DM9000_PLATF_8BITONLY,
};

static struct resource dm9000_resources[] = {
	MEM_RES("dm9000_io", DM9000_IO, 1),
	MEM_RES("dm9000_data", DM9000_DATA, 1),
	[2] = {
		/* with irq (210 -> gpio 110) the driver is very unreliable */
		.start	= -1,		/* use polling */
		.end	= -1,
		.flags	= IORESOURCE_IRQ | IRQF_TRIGGER_LOW,
	},
};

static struct platform_device dm9000_platform = {
	.name = "dm9000",
	.id = 0,
	.num_resources	= ARRAY_SIZE(dm9000_resources),
	.resource	= dm9000_resources,
	.dev = {
		.platform_data = (void *) &dm9000_plat_data,
	}
};

extern int easy98000_addon_has_dm9000(void);
static void __init register_davicom(void)
{
	if (!easy98000_addon_has_dm9000())
		return;

	if (!is_valid_ether_addr(ltq_ethaddr))
		random_ether_addr(dm9000_plat_data.dev_addr);
	else {
		memcpy(dm9000_plat_data.dev_addr, ltq_ethaddr, 6);
		/* change to "Locally Administered Address" */
		dm9000_plat_data.dev_addr[0] |= 0x2;
	}
	platform_device_register(&dm9000_platform);
}

static struct i2c_gpio_platform_data easy98000_i2c_gpio_data = {
	.sda_pin	= 107,
	.scl_pin	= 108,
};

static struct platform_device easy98000_i2c_gpio_device = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev = {
		.platform_data	= &easy98000_i2c_gpio_data,
	}
};

void __init register_easy98000_cpld(void)
{
	platform_device_register_simple("easy98000_cpld_led", 0, NULL, 0);
	platform_device_register_simple("easy98000_addon", 0, NULL, 0);
}

/* setup gpio based spi bus/device for access to the eeprom on the board */
#define SPI_GPIO_MRST	102
#define SPI_GPIO_MTSR	103
#define SPI_GPIO_CLK	104
#define SPI_GPIO_CS0	105
#define SPI_GPIO_CS1	106
#define SPI_GPIO_BUS_NUM	1

static struct spi_gpio_platform_data easy98000_spi_gpio_data = {
	.sck		= SPI_GPIO_CLK,
	.mosi		= SPI_GPIO_MTSR,
	.miso		= SPI_GPIO_MRST,
	.num_chipselect	= 2,
};

static struct platform_device easy98000_spi_gpio_device = {
	.name			= "spi_gpio",
	.id			= SPI_GPIO_BUS_NUM,
	.dev.platform_data	= &easy98000_spi_gpio_data,
};

static struct spi_eeprom at25160n = {
	.byte_len	= 16 * 1024 / 8,
	.name		= "at25160n",
	.page_size	= 32,
	.flags		= EE_ADDR2,
};

static struct spi_board_info easy98000_spi_gpio_devices __initdata = {
	.modalias		= "at25",
	.bus_num		= SPI_GPIO_BUS_NUM,
	.max_speed_hz		= 1000 * 1000,
	.mode			= SPI_MODE_3,
	.chip_select		= 1,
	.controller_data	= (void *) SPI_GPIO_CS1,
	.platform_data		= &at25160n,
};

static void __init easy98000_spi_gpio_init(void)
{
	spi_register_board_info(&easy98000_spi_gpio_devices, 1);
	platform_device_register(&easy98000_spi_gpio_device);
}

static void __init easy98000_init_common(void)
{
	falcon_register_i2c();
	platform_device_register(&easy98000_i2c_gpio_device);
	register_davicom();
	ltq_add_device_gpio_leds(-1, ARRAY_SIZE(easy98000_gpio_leds),
					easy98000_gpio_leds);
	register_easy98000_cpld();
	easy98000_spi_gpio_init();
}

static void __init easy98000_init(void)
{
	easy98000_init_common();
	ltq_register_nor(&easy98000_nor_flash_data);
}

static void __init easy98000sf_init(void)
{
	easy98000_init_common();
	falcon_register_spi_flash(&easy98000_spi_flash_data);
}

static void __init easy98000nand_init(void)
{
	easy98000_init_common();
	falcon_register_nand();
}

MIPS_MACHINE(LANTIQ_MACH_EASY98000,
			"EASY98000",
			"EASY98000 Eval Board",
			easy98000_init);

MIPS_MACHINE(LANTIQ_MACH_EASY98000SF,
			"EASY98000SF",
			"EASY98000 Eval Board (Serial Flash)",
			easy98000sf_init);

MIPS_MACHINE(LANTIQ_MACH_EASY98000NAND,
			"EASY98000NAND",
			"EASY98000 Eval Board (NAND Flash)",
			easy98000nand_init);
