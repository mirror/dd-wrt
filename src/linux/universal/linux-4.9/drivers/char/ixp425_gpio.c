/*
 * ixp425_gpio.c : Intel IXP425 GPIO driver Linux
 *                 Copyright (C) 2004, Tim Harvey <tim_harvey@yahoo.com>
 *                 Hautespot Networks, www.hautespot.net
 *
 * Description:
 *
 *    This driver allows access to the IXP425 Digital IO
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * TODO: 
 *   - extend to allow interrupts
 *
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>

#include <asm/uaccess.h>	/* copy_to_user, copy_from_user */
//#include <asm-arm/arch-ixp4xx/gpio.h>

//#include <linux/ixp425-gpio.h>
#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/ixp4xx-regs.h>

extern spinlock_t gpio_lock;

#define PLD_SCL_GPIO 6
#define PLD_SDA_GPIO 7

#define SCL_LO()  gpio_line_set(PLD_SCL_GPIO, IXP4XX_GPIO_LOW);
#define SCL_HI()  gpio_line_set(PLD_SCL_GPIO, IXP4XX_GPIO_HIGH);
#define SCL_EN()  gpio_line_config(PLD_SCL_GPIO, IXP4XX_GPIO_OUT);
#define SDA_LO()  gpio_line_set(PLD_SDA_GPIO, IXP4XX_GPIO_LOW);
#define SDA_HI()  gpio_line_set(PLD_SDA_GPIO, IXP4XX_GPIO_HIGH);
#define SDA_EN()  gpio_line_config(PLD_SDA_GPIO, IXP4XX_GPIO_OUT);
#define SDA_DIS() gpio_line_config(PLD_SDA_GPIO, IXP4XX_GPIO_IN);
#define SDA_IN(x) gpio_line_get(PLD_SDA_GPIO, &x);

#define SCL_STB() SCL_LO(); SCL_HI()
#define SDA_BIT_0() SDA_LO(); SCL_STB()
#define SDA_BIT_1() SDA_HI(); SCL_STB()
#define PLD_RST() RST_HI(); SCL_STB(); RST_LO()

#define CLK_LO()      SCL_LO()
#define CLK_HI()      SCL_HI()
#define CLK_EN()      SCL_EN()
#define DATA_LO()     SDA_LO()
#define DATA_HI()     SDA_HI()
#define DATA_EN()     SDA_EN()
#define DATA_DIS()    SDA_DIS()
#define DATA_IN(x)     SDA_IN(x)

static int eeprom_start(unsigned char b) {

  int i = 0;

  DATA_HI();
  DATA_EN();
  CLK_EN();
  CLK_HI();
  DATA_LO();
  CLK_LO();

  for (i = 7; i >= 0; i--) {
    if (b & (1 << i))
    {
      DATA_HI();
    }
    else
    {
      DATA_LO();
    }
    CLK_HI();
    CLK_LO();
  }

  DATA_DIS();
  CLK_HI();
  DATA_IN(i);
  CLK_LO();
  DATA_EN();

  return i;
}

static int
eeprom_putb(unsigned char b)
{
    int i;


  for (i = 7; i >= 0; i--) {
  if (b & (1 << i))
  {
      DATA_HI();
  }
  else
  {
      DATA_LO();
  }
  CLK_HI();
  CLK_LO();
    }
    DATA_DIS();
    CLK_HI();
    DATA_IN(i);
    CLK_LO();

    DATA_HI();
    DATA_EN();

    return i;
}

static int
eeprom_getb(void)
{
    int i, j;

  int tmp = 0;

  DATA_DIS();
  for (i = 7; i >= 0; i--) {
    CLK_HI();
    DATA_IN(j);
    tmp |= j << i;
    CLK_LO();
  }
    return tmp;
}

unsigned int
pld_read_gpio_b(int bit)
{
  unsigned int i;
  spin_lock(&gpio_lock);
    eeprom_start(0xad);
    i = eeprom_getb();
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
  return (i >> bit) & 0x1;
}
unsigned int
pld_read_gpio_b2(int bit)
{
  unsigned int i;
  spin_lock(&gpio_lock);
    eeprom_start(0xaf);
    i = eeprom_getb();
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
  return (i >> bit) & 0x1;
}


unsigned int
pld_read_gpio(void)
{
  unsigned int i;
  spin_lock(&gpio_lock);
    eeprom_start(0xad);
    i = eeprom_getb();
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
  return i;
}

unsigned int
pld_read_gpio2(void)
{
  unsigned int i;
  spin_lock(&gpio_lock);
    eeprom_start(0xaf);
    i = eeprom_getb();
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
  return i;
}


void
pld_write_gpio(int byte)
{
  //printk(KERN_INFO "%s: Enabling LED\n", driver_name);

  spin_lock(&gpio_lock);
    eeprom_start(0xac);
    eeprom_putb(byte);
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
}

void
pld_write_gpio2(int byte)
{
  //printk(KERN_INFO "%s: Enabling LED\n", driver_name);

  spin_lock(&gpio_lock);
    eeprom_start(0xae);
    eeprom_putb(byte);
    DATA_LO();
    CLK_HI();
    DATA_HI();
    CLK_LO();
    CLK_HI();
  spin_unlock(&gpio_lock);
}



static unsigned char *iobase;

void setLED(int led, int status)
{
static unsigned char staticb=0xff;

#ifdef CONFIG_MACH_CAMBRIA
if (!status)
    staticb|=1<<led;
else
    staticb&=~(1<<led);
writeb(staticb,iobase);
#endif
}
EXPORT_SYMBOL(setLED);

struct gpio_bit {
  unsigned char bit;
  unsigned char state;
};

#define DEVICE_NAME "gpio"
#define GPIO_MAJOR 127
//#define DEBUG

/*
 * GPIO interface
 */

/* /dev/gpio */
static long gpio_ioctl(struct file *file,
                     unsigned int cmd, unsigned long arg);

/* /proc/driver/gpio */
static ssize_t gpio_read_proc(struct file *file, char __user *buffer,
		  size_t size, loff_t *ppos);
		  
static unsigned char gpio_status;        /* bitmapped status byte.       */

#define ERROR   (-1)
#define OK                      0
#define ixp425GPIOLineConfig    gpio_line_config
#define ixp425GPIOLineGet       gpio_line_get
#define ixp425GPIOLineSet       gpio_line_set
#define sysMicroDelay(x)        udelay(x)
#define IXP425_GPIO_SIG         int

#define GPIO_IS_OPEN             0x01    /* means /dev/gpio is in use     */

/*
 *      Now all the various file operations that we export.
 */
static long gpio_ioctl(struct file *file,
                         unsigned int cmd, unsigned long arg)
{
	struct gpio_bit bit;
	IXP425_GPIO_SIG val;
	int temp;
//printk("enter gpio ioctl\n");
	if (copy_from_user(&bit, (struct gpio_bit *)arg, 
				sizeof(bit)))
		return -EFAULT;
//printk("ixp425_gpio: ioctl cmd 0x%02x, bit %d, state %d\n", cmd, bit.bit, bit.state);
#ifdef DEBUG
printk("ixp425_gpio: ioctl cmd 0x%02x, bit %d, state %d\n", cmd, bit.bit, bit.state);
#endif

        switch (cmd) {

        case GPIO_GET_BIT:
if (bit.bit < 16)
{
		ixp425GPIOLineGet(bit.bit, &val);
		bit.state = val;
#ifdef DEBUG
             	printk("ixp425_gpio: Read _bit 0x%02x %s\n", bit.bit, (bit.state==0)?"LOW":"HIGH");
#endif
}
else if (bit.bit < 24)
{
	bit.state = pld_read_gpio_b(bit.bit - 16);
}
else if (bit.bit < 32)
{
	bit.state = pld_read_gpio_b2(bit.bit - 24);
}
		return copy_to_user((void *)arg, &bit, sizeof(bit)) ? -EFAULT : 0;
        case 5:
#ifdef DEBUG
             	printk("ixp425_gpio: Write _bit 0x%02x %s\n", bit.bit, (bit.state==0)?"LOW":"HIGH");
#endif
if (bit.bit < 16)
{
		val = bit.state;
		ixp425GPIOLineSet(bit.bit, val);
}
else if (bit.bit < 24)
{
	temp = pld_read_gpio();
	if (bit.state == 1)
		temp |= (0x1 << (bit.bit - 16));
	else
		temp &= ~(0x1 << (bit.bit - 16));
	pld_write_gpio(temp);
}
else if (bit.bit < 32)
{
	temp = pld_read_gpio2();
	if (bit.state == 1)
		temp |= (0x1 << (bit.bit - 24));
	else
		temp &= ~(0x1 << (bit.bit - 24));
	pld_write_gpio2(temp);
}
#ifdef CONFIG_MACH_CAMBRIA
else if (bit.bit < 40)
{
	if (bit.state == 1)
	    setLED(bit.bit-32,1);
	else
	    setLED(bit.bit-32,0);
}
#endif
		return OK;
	case GPIO_GET_CONFIG:
if (bit.bit < 16)
{
          	ixp425GPIOLineConfig(bit.bit, bit.state);
#ifdef DEBUG
             	printk("ixp425_gpio: Read config _bit 0x%02x %s\n", bit.bit, (bit.state==2)?"IN":"OUT");
#endif
}
else if (bit.bit < 24)
{
	bit.state = pld_read_gpio_b(bit.bit - 16);
}
else if (bit.bit < 32)
{
	bit.state = pld_read_gpio_b2(bit.bit - 24);
}
		return copy_to_user((void *)arg, &bit, sizeof(bit)) ? -EFAULT : 0;
	case GPIO_SET_CONFIG:
if (bit.bit < 16)
{
		val = bit.state;
#ifdef DEBUG
             	printk("ixp425_gpio: Write config _bit 0x%02x %s\n", bit.bit, (bit.state==2)?"IN":"OUT");
#endif
          	ixp425GPIOLineConfig(bit.bit, bit.state);
}
else if (bit.bit < 24)
{
	temp = pld_read_gpio();
	if (bit.state == 2)
	{
		temp |= (0x1 << (bit.bit - 16));
		pld_write_gpio(temp);
	}
}
else if (bit.bit < 32)
{
	temp = pld_read_gpio2();
	if (bit.state == 2)
	{
		temp |= (0x1 << (bit.bit - 24));
		pld_write_gpio2(temp);
	}
}
		return OK;
	}
	return -EINVAL;
}


static int gpio_open(struct inode *inode, struct file *file)
{
        if (gpio_status & GPIO_IS_OPEN)
                return -EBUSY; 

        gpio_status |= GPIO_IS_OPEN;

        return 0;
}


static int gpio_release(struct inode *inode, struct file *file)
{
        /*
         * Turn off all interrupts once the device is no longer
         * in use and clear the data.
         */

        gpio_status &= ~GPIO_IS_OPEN;

        return 0;
}


/*
 *      The various file operations we support.
 */

static struct file_operations gpio_fops = {
        .owner          = THIS_MODULE,
        .unlocked_ioctl          = gpio_ioctl,
        .open           = gpio_open,
        .release        = gpio_release,
};

static struct miscdevice gpio_dev =
{
        .minor          = 0,
        .name           = "gpio",
        .fops           = &gpio_fops,
};




#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *dir;

/*
 *      Info exported via "/proc/driver/gpio".
 */
static int gpio_get_status(char *buf)
{
        char *p;

        p = buf;

        /*
         */
	u8 val = 0;
	int i;
	int bit;
	for (i = 0; i < 16; i++) {
		gpio_line_get(i, &bit);
		if (bit)
			val |= (1 << i);
	}
        p += sprintf(p, "gpio_config\t: 0x%04x\n", val);

        return p - buf;
}


static ssize_t gpio_read_proc(struct file *file, char __user *buffer,
		  size_t size, loff_t *ppos)
{
	char buf[128];
	gpio_get_status (buf);
	return simple_read_from_buffer(buffer, size, ppos, buf, strlen(buf));
}


static const struct file_operations fops_info = {
	.read = gpio_read_proc,
	.llseek = default_llseek,
};
#endif /* CONFIG_PROC_FS */


static int __init gpio_init_module(void)
{
        int retval;

        /* register /dev/gpio file ops */
	retval = register_chrdev(GPIO_MAJOR, DEVICE_NAME, &gpio_fops);
        if(retval < 0)
                return retval;
#ifdef CONFIG_MACH_CAMBRIA
	iobase = ioremap_nocache(0x53F40000, 0x1000);
	writeb(0xff, iobase);
	printk(KERN_INFO "gpio virtual base %p\n",iobase);
#endif
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *res;
	dir = proc_mkdir("driver/gpio", NULL);
	if (!dir) {
		misc_deregister(&gpio_dev);
		return -ENOMEM;
	}
        /* register /proc/driver/gpio */
	res = proc_create("info", 644, dir,&fops_info);

//	res = create_proc_entry("info", 644, dir);
	if (!res) {
		misc_deregister(&gpio_dev);
		return -ENOMEM;
	}
#endif

	printk("ixp425_gpio: IXP425 GPIO driver loaded\n");

	return 0; 
}

static void __exit gpio_cleanup_module(void)
{
	remove_proc_entry ("info", dir);
        misc_deregister(&gpio_dev);

	printk("ixp425_gpio: IXP425 GPIO driver unloaded\n");
}

module_init(gpio_init_module);
module_exit(gpio_cleanup_module);

MODULE_AUTHOR("Tim Harvey <tim_harvey@yahoo.com>");
MODULE_LICENSE("GPL");
