/*
 * proc_gpio: AR5315 GPIO pins in /proc/gpio/
 * by olg 
 * modification for AR7240 support by Sebastian Gottschall <s.gottschall@newmedia-net.de>
 * GPL'ed
 * some code stolen from Yoshinori Sato <ysato@users.sourceforge.jp>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_from_user */

#include "ar7240.h"

#define PROCFS_MAX_SIZE 64
extern const char *get_arch_type (void);
struct proc_dir_entry *proc_gpio, *gpio_dir;

//Masks for data exchange through "void *data" pointer
#define GPIO_IN (1<<5)
#define GPIO_OUT (1<<6)
#define GPIO_DIR (1<<7)
#define PIN_MASK 0x1f
#define GPIO_CR_M(x)                (1 << (x))	/* mask for i/o */


static void cleanup_proc (void);

//The buffer used to store the data returned by the proc file
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;


static int
gpio_proc_read (char *buf, char **start, off_t offset,
		int len, int *eof, void *data)
{
  u32 reg = 0;
      if ((unsigned int) data & GPIO_IN)
	reg = ar7240_reg_rd(AR7240_GPIO_IN);
      if ((unsigned int) data & GPIO_OUT)
	reg = ar7240_reg_rd(AR7240_GPIO_OUT);
      if ((unsigned int) data & GPIO_DIR)
	reg = ar7240_reg_rd(AR7240_GPIO_OE);

  if (GPIO_CR_M (((unsigned int) data) & PIN_MASK) & reg)
    buf[0] = '1';
  else
    buf[0] = '0';
  buf[1] = 0;

  *eof = 1;

  return (2);

}

static int
gpio_proc_info_read (char *buf, char **start, off_t offset,
		     int len, int *eof, void *data)
{
  *eof = 1;
      return (sprintf
	      (buf, "GPIO_IN   %#08X \nGPIO_OUT  %#08X \nGPIO_DIR  %#08X \n",
	       ar7240_reg_rd(AR7240_GPIO_IN), ar7240_reg_rd(AR7240_GPIO_OUT),
	       ar7240_reg_rd(AR7240_GPIO_OE)));
}

static int
gpio_proc_write (struct file *file, const char *buffer, unsigned long count,
		 void *data)
{
  u32 reg = 0;

  /* get buffer size */
  procfs_buffer_size = count;
  if (procfs_buffer_size > PROCFS_MAX_SIZE)
    {
      procfs_buffer_size = PROCFS_MAX_SIZE;
    }
  /* write data to the buffer */
  if (copy_from_user (procfs_buffer, buffer, procfs_buffer_size))
    {
      return -EFAULT;
    }

  procfs_buffer[procfs_buffer_size] = 0;

      if ((unsigned int) data & GPIO_IN)
	reg = ar7240_reg_rd(AR7240_GPIO_IN);
      if ((unsigned int) data & GPIO_OUT)
	reg = ar7240_reg_rd(AR7240_GPIO_OUT);
      if ((unsigned int) data & GPIO_DIR)
	reg = ar7240_reg_rd(AR7240_GPIO_OE);

  if (procfs_buffer[0] == '0' || procfs_buffer[0] == 'i')
    reg = reg & ~(GPIO_CR_M (((unsigned int) data) & PIN_MASK));
  if (procfs_buffer[0] == '1' || procfs_buffer[0] == 'o')
    reg = reg | GPIO_CR_M (((unsigned int) data) & PIN_MASK);


      if ((unsigned int) data & GPIO_IN)
	{
          ar7240_reg_wr(AR7240_GPIO_IN, reg);
	}
      if ((unsigned int) data & GPIO_OUT)
	{
          ar7240_reg_wr(AR7240_GPIO_OUT, reg);
	}
      if ((unsigned int) data & GPIO_DIR)
	{
          ar7240_reg_wr(AR7240_GPIO_OE, reg);
	}

  return procfs_buffer_size;
}

void ar7100_set_gpio(int gpio, int val)
{
u32 reg = ar7240_reg_rd(AR7240_GPIO_OE);
reg |= 1<<gpio;
ar7240_reg_wr(AR7240_GPIO_OE, reg);
(void)ar7240_reg_rd(AR7240_GPIO_OE); /* flush write to hardware */
reg = ar7240_reg_rd(AR7240_GPIO_OUT);
if (val)
    reg|=1<<gpio;
else
    reg&=~(1<<gpio);
ar7240_reg_wr(AR7240_GPIO_OUT, reg);
}

int ar7100_get_gpio(int gpio)
{
u32 reg = ar7240_reg_rd(AR7240_GPIO_OE);
reg&=~(1<<gpio);
ar7240_reg_wr(AR7240_GPIO_OE, reg);
reg = ar7240_reg_rd(AR7240_GPIO_IN);
if (reg&(1<<gpio))
    return 1;
else
    return 0;
}
EXPORT_SYMBOL(ar7100_set_gpio);
EXPORT_SYMBOL(ar7100_get_gpio);



typedef	u32					gpio_words;
#define	GPIO_IGN_MASK		(GPIO_PIN(0) | GPIO_PIN(9) | GPIO_PIN(10))			//IGNORE  GPIO[0]:SI_CS1, GPIO[9-10]:UART

#define	GPIO_PIN_MAX		12
#define	GPIO_PIN(n)			(1 << (n))
#define	GPIO_PIN_MASK(_VAL)	(((gpio_words)(_VAL)) & (((((gpio_words)1)<<GPIO_PIN_MAX)-1) & (~(gpio_words)GPIO_IGN_MASK)) )


#define	GPIO_WL0_MAX		10
#define	GPIO_WL0(n)			(1 << ((n)+GPIO_PIN_MAX))
#define	GPIO_WL0_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x4048)				//AR9220 GPIO IN/OUT REGISTER	--> PCI MAP 0xB0000000 + OFFSET [0x4048]
#define	GPIO_WL0_MASK(_VAL)	(((gpio_words)(_VAL)) & (((gpio_words)1<<GPIO_WL0_MAX)-1))
#define	GPIO_WL0_TO(_VAL)	GPIO_WL0_MASK(((gpio_words)(_VAL))>>(GPIO_PIN_MAX))	//the value to AR9220 register
#define	GPIO_WL0_FROM(_VAL)	(GPIO_WL0_MASK(_VAL)<<(GPIO_PIN_MAX))				//the value from AR9220 register

#define	GPIO_WL1_MAX		10
#define	GPIO_WL1(n)			(1 << ((n)+GPIO_PIN_MAX+GPIO_WL0_MAX))
#define	GPIO_WL1_ADDR		KSEG1ADDR(AR71XX_PCI_MEM_BASE + 0x00010000 + 0x4048)				//AR9223 GPIO IN/OUT REGISTER	--> PCI MAP 0xB0010000 + OFFSET [0x4048]
#define	GPIO_WL1_MASK(_VAL)	(((gpio_words)(_VAL)) & (((gpio_words)1<<GPIO_WL1_MAX)-1))
#define	GPIO_WL1_TO(_VAL)	GPIO_WL1_MASK(((gpio_words)(_VAL))>>(GPIO_PIN_MAX+GPIO_WL0_MAX))	//the value to AR9223 register
#define	GPIO_WL1_FROM(_VAL)	(GPIO_WL1_MASK(_VAL)<<(GPIO_PIN_MAX+GPIO_WL0_MAX))					//the value from AR9223 register


int get_wl0_gpio(int gpio)
{
	register	gpio_words	wl0	= (gpio_words)ar7100_reg_rd(GPIO_WL0_ADDR);	//ar9280 register [0x4048]
        if (wl0&(1<<gpio));
    	    return 1;
    	return 0;
}
int get_wl1_gpio(int gpio)
{
	register	gpio_words	wl1	= (gpio_words)ar7100_reg_rd(GPIO_WL1_ADDR);	//ar9280 register [0x4048]
        if (wl1&(1<<gpio));
    	    return 1;
    	return 0;
}

void set_wl0_gpio(int gpio,int val)
{
	register	gpio_words	wl0	= (gpio_words)ar7100_reg_rd(GPIO_WL0_ADDR);	//ar9280 register [0x4048]
	if (val)
	    wl0|=1<<gpio;
	else
	    wl0&=(~(1<<gpio));
	ar7100_reg_rmw_set(GPIO_WL0_ADDR, wl0);	//ar9283 register [0x4048]
	ar7100_reg_rmw_clear(GPIO_WL0_ADDR, wl0);
}

void set_wl1_gpio(int gpio,int val)
{
	register	gpio_words	wl1	= (gpio_words)ar7100_reg_rd(GPIO_WL1_ADDR);	//ar9280 register [0x4048]
	if (val)
	wl1|=1<<gpio;
	else
	wl1&=(~(1<<gpio));
	
	ar7100_reg_rmw_set(GPIO_WL0_ADDR, wl1);	//ar9283 register [0x4048]
	ar7100_reg_rmw_clear(GPIO_WL0_ADDR, wl1);
}


#define USB_LED_OFF 1
#define USB_LED_ON 0


void ap_usb_led_on(void)
{
#ifdef CONFIG_WZRG300NH2
set_wl0_gpio(4,0);
#else
#ifdef AP_USB_LED_GPIO
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_ON);
#endif
#endif
}
EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
#ifdef CONFIG_WZRG300NH2
set_wl0_gpio(4,1);
#else
#ifdef AP_USB_LED_GPIO
	ar7100_set_gpio(AP_USB_LED_GPIO, USB_LED_OFF);
#endif
#endif
}
EXPORT_SYMBOL(ap_usb_led_off);

static __init int
register_proc (void)
{
  unsigned char i, flag = 0;
  char proc_name[64];
  int gpiocount = 64;

  /* create directory gpio */
  gpio_dir = proc_mkdir ("gpio", NULL);
  if (gpio_dir == NULL)
    goto fault;

  for (i = 0; i < gpiocount * 3; i++)	//create for every GPIO "x_in"," x_out" and "x_dir"
    {
      if (i / gpiocount == 0)
	{
	  flag = GPIO_IN;
	  sprintf (proc_name, "%i_in", i);
	}
      if (i / gpiocount == 1)
	{
	  flag = GPIO_OUT;
	  sprintf (proc_name, "%i_out", i % gpiocount);
	}
      if (i / gpiocount == 2)
	{
	  flag = GPIO_DIR;
	  sprintf (proc_name, "%i_dir", i % gpiocount);
	}

      proc_gpio = create_proc_entry (proc_name, S_IRUGO, gpio_dir);
      if (proc_gpio)
	{
	  proc_gpio->read_proc = gpio_proc_read;
	  proc_gpio->write_proc = gpio_proc_write;
	  proc_gpio->data = ((i % gpiocount) | flag);
	}
      else
	goto fault;

    }

  proc_gpio = create_proc_entry ("info", S_IRUGO, gpio_dir);
  if (proc_gpio)
    {
      proc_gpio->read_proc = gpio_proc_info_read;
    }
  else
    goto fault;

  printk (KERN_NOTICE "gpio_proc: module loaded and /proc/gpio/ created\n");
  return 0;

fault:
  cleanup_proc ();
  return -EFAULT;
}

static void
cleanup_proc (void)
{
  unsigned char i;
  char proc_name[64];
  int gpiocount=64;

  for (i = 0; i < gpiocount; i++)
    {
      sprintf (proc_name, "%i_in", i);
      remove_proc_entry (proc_name, gpio_dir);
      sprintf (proc_name, "%i_out", i);
      remove_proc_entry (proc_name, gpio_dir);
      sprintf (proc_name, "%i_dir", i);
      remove_proc_entry (proc_name, gpio_dir);
    }
  remove_proc_entry ("info", gpio_dir);
  remove_proc_entry ("gpio", NULL);
  printk (KERN_INFO "gpio_proc: unloaded and /proc/gpio/ removed\n");

}


module_init (register_proc);
module_exit (cleanup_proc);

MODULE_AUTHOR ("Sebastian Gottschall");
MODULE_DESCRIPTION ("AR7240 GPIO pins in /proc/gpio/");
MODULE_LICENSE ("GPL");
