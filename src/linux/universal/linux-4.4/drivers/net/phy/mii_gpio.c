/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/phy.h>

MODULE_DESCRIPTION("MDIO by GPIO");
MODULE_AUTHOR("Winder Sung <winder@annapurnalabs.com>");
MODULE_LICENSE("GPL");

#define GPIO_MDC_PIN     0
#define GPIO_MDIO_PIN    1

void Mdc_Pulse(void)  /*Clock line */
{
    /* 1 Mhz */
    udelay(1);
    gpio_set_value(GPIO_MDC_PIN, 0);
    udelay(1);
    gpio_set_value(GPIO_MDC_PIN, 1);
    udelay(1);
}

void Idle(void)
{
    gpio_direction_output(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
}

void Preamble(void)
{
    char i;

    gpio_direction_output(GPIO_MDIO_PIN, 1);
    /* Transmit Preamble 11....11(32 bits) */
    for(i=0 ; i < 32 ; i++)
        Mdc_Pulse();
    /* Transmit Start of Frame '01' */
    gpio_set_value(GPIO_MDIO_PIN, 0);
    Mdc_Pulse();
    gpio_set_value(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
}

unsigned int phy_reg_read_by_gpio(char phy_addr,char phy_reg)
{
    char i;
    u16 phy_val;

    Preamble();
    /*OP Code 10*/
    gpio_direction_output(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
    gpio_set_value(GPIO_MDIO_PIN, 0);
    Mdc_Pulse();
    /*5 bits PHY addr*/
    for(i = 0; i < 5; i++)
    {
        if(phy_addr & 0x10)
        {
            gpio_set_value(GPIO_MDIO_PIN, 1);
        }
        else
        {
            gpio_set_value(GPIO_MDIO_PIN, 0);
        }
        Mdc_Pulse();
        phy_addr <<= 1;
    }
    /*5 bits PHY reg*/
    for(i = 0; i < 5; i++)
    {
        if(phy_reg & 0x10)
        {
            gpio_set_value(GPIO_MDIO_PIN, 1);
        }
        else
        {
            gpio_set_value(GPIO_MDIO_PIN, 0);
        }
        Mdc_Pulse();
        phy_reg <<= 1;
    }
    /*Turnaround Z*/
    gpio_set_value(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
    gpio_direction_input(GPIO_MDIO_PIN);
    /*Read 16 bits Data*/
    phy_val = 0x0000;
    for ( i = 0; i < 16; i++)
    {
        Mdc_Pulse();
        if (1 == gpio_get_value(GPIO_MDIO_PIN))
            phy_val |= 0x0001;
        if (i < 15)
            phy_val <<= 1;
    }
    Idle();
    gpio_direction_output(GPIO_MDIO_PIN, 1);

    return phy_val;
}

void phy_reg_write_by_gpio(char phy_addr,char phy_reg, unsigned int phy_val)
{
    char i;
    u16 Temp;

    Preamble();
    /*OP Code 01*/
    gpio_direction_output(GPIO_MDIO_PIN, 0);
    Mdc_Pulse();
    gpio_set_value(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
    /*5 bits PHY addr*/
    for(i = 0; i < 5; i++)
    {
        if(phy_addr & 0x10)
        {
            gpio_set_value(GPIO_MDIO_PIN, 1);
        }
        else
        {
            gpio_set_value(GPIO_MDIO_PIN, 0);
        }
        Mdc_Pulse();
        phy_addr <<= 1;
    }
    /*5 bits PHY reg*/
    for(i = 0; i < 5; i++)
    {
        if(phy_reg & 0x10)
        {
            gpio_set_value(GPIO_MDIO_PIN, 1);
        }
        else
        {
            gpio_set_value(GPIO_MDIO_PIN, 0);
        }
        Mdc_Pulse();
        phy_reg <<= 1;
    }
    /*Turnaround 10*/
    gpio_set_value(GPIO_MDIO_PIN, 1);
    Mdc_Pulse();
    gpio_set_value(GPIO_MDIO_PIN, 0);
    Mdc_Pulse();
    /*Write 16 bits Data*/
    Temp = 0x8000;
    for ( i = 0; i < 16; i++)
    {
        if(phy_val & Temp)
        {
            gpio_set_value(GPIO_MDIO_PIN, 1);
        }
        else
        {
            gpio_set_value(GPIO_MDIO_PIN, 0);
        }
        Mdc_Pulse();
        Temp >>= 1;
    }
    Idle();
}
static int
al_mdio_gpio_read(struct mii_bus *bp, int mii_id, int reg)
{
	return phy_reg_read_by_gpio(mii_id, reg);
}

static int
al_mdio_gpio_write(struct mii_bus *bp, int mii_id, int reg, u16 val)
{
	phy_reg_write_by_gpio(mii_id, reg, val);
	return 0;
}

static int __init mdiobus_probe(struct platform_device *pdev)
{
    int status;
    u16 phy_addr, phy_reg;
    struct mii_bus * p_bus = NULL;
    u16 phy_val;

    pr_info("mido_gpio module init\n");

    /* MDC */
    status = gpio_request(GPIO_MDC_PIN, "gpio_as_mdc");
    if (status < 0)
    {
        printk("request GPIO%d failed(%d)!\n", GPIO_MDC_PIN, status);
        return status;
    }
    gpio_direction_output(GPIO_MDC_PIN, 1);

    /* MDIO */
    status = gpio_request(GPIO_MDIO_PIN, "gpio_as_mdio");
    if (status < 0)
    {
        printk("request GPIO%d failed(%d)!\n", GPIO_MDIO_PIN, status);
        return status;
    }
    gpio_direction_output(GPIO_MDIO_PIN, 0);

    phy_reg_write_by_gpio(0x18,0x0, 0x0);
    phy_val = phy_reg_read_by_gpio(0x10,0x0);
    if (phy_val == 0x1302) {
        printk("QCA8337 is found\n");
    }

	p_bus = mdiobus_alloc();
	p_bus->name = "mdio-al";
	p_bus->read = &al_mdio_gpio_read;
	p_bus->write = &al_mdio_gpio_write;
	p_bus->parent = &pdev->dev;
	snprintf(p_bus->id, MII_BUS_ID_SIZE, "fixed-gpio");
//	#if 0
	if(mdiobus_register(p_bus))
		printk("mdio bus register fail!\n");
//	#endif
	mutex_init(&p_bus->mdio_lock);
    return 0;
}


struct mii_bus * al_get_mdiobus_by_gpio(void)
{

	struct mii_bus * p_bus = NULL;

	p_bus = mdiobus_alloc();
	p_bus->name = "gpio test";
	p_bus->read = &al_mdio_gpio_read;
	p_bus->write = &al_mdio_gpio_write;
//	#if 0
	if(mdiobus_register(p_bus))
		printk("mdio bus register fail!\n");
//	#endif
	mutex_init(&p_bus->mdio_lock);
	return p_bus;
	
}

EXPORT_SYMBOL(al_get_mdiobus_by_gpio);

static void __exit mdio_gpio_exit(void)
{
    printk(KERN_INFO "mdio_gpio module exit\n");
    gpio_free(GPIO_MDC_PIN);
    gpio_free(GPIO_MDIO_PIN);
}

static struct platform_driver al_mdiobus_driver = {
	.driver = {
		.name		= "mdio-al",
	},
	.probe		= mdiobus_probe,
};


module_platform_driver(al_mdiobus_driver);
