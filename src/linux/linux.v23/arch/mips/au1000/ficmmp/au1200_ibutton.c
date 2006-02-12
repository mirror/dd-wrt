/* ---------------------------------------------------------------------- 
 *  mtwilson_keys.c
 *
 *  Copyright (C) 2003 Intrinsyc Software Inc.
 *
 *  Intel Personal Media Player buttons
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  May 02, 2003 : Initial version [FB]
 *
 ------------------------------------------------------------------------*/

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/input.h>

#include <asm/au1000.h>
#include <asm/uaccess.h>
#include <asm/au1xxx_gpio.h>
#include <asm/irq.h>
#include <asm/keyboard.h>
#include <linux/time.h>

#define DRIVER_VERSION	"V1.0"
#define DRIVER_AUTHOR	"FIC"
#define DRIVER_DESC		"FIC Travis Media Player Button Driver"
#define DRIVER_NAME		"Au1200Button"

#define BUTTON_MAIN		(1<<1)
#define BUTTON_SELECT	(1<<6)
#define BUTTON_GUIDE	(1<<12)
#define BUTTON_DOWN		(1<<17)
#define BUTTON_LEFT		(1<<19)
#define BUTTON_RIGHT	(1<<26)
#define BUTTON_UP		(1<<28)

#define BUTTON_MASK (\
    BUTTON_MAIN   \
    | BUTTON_SELECT	\
    | BUTTON_GUIDE	\
    | BUTTON_DOWN	\
    | BUTTON_LEFT	\
    | BUTTON_RIGHT	\
    | BUTTON_UP		\
    )

#define BUTTON_INVERT (\
    BUTTON_MAIN   \
    | 0				\
    | BUTTON_GUIDE	\
    | 0				\
    | 0				\
    | 0				\
    | 0				\
    )

char button_map[32]={0,KEY_S,0,0,0,0,KEY_ENTER,0,0,0,0,0,KEY_G,0,0,0,0,KEY_DOWN,0,KEY_LEFT,0,0,0,0,0,0,KEY_RIGHT,0,KEY_UP,0,0,0};
//char button_map[32]={0,0,0,0,0,0,KEY_ENTER,0,0,0,0,0,KEY_G,0,0,0,0,KEY_DOWN,0,KEY_LEFT,0,0,0,0,0,0,KEY_RIGHT,0,KEY_UP,0,0,0};

//char button_map[32]={0,KEY_TAB,0,0,0,0,KEY_M,0,0,0,0,0,KEY_S,0,0,0,0,KEY_DOWN,0,KEY_LEFT,0,0,0,0,0,0,KEY_RIGHT,0,KEY_UP,0,0,0};
//char button_map[32]={0,0,0,0,0,0,KEY_M,0,0,0,0,0,KEY_S,0,0,0,0,KEY_DOWN,0,KEY_LEFT,0,0,0,0,0,0,KEY_RIGHT,0,KEY_UP,0,0,0};

#define BUTTON_COUNT (sizeof (button_map) / sizeof (button_map[0]))

struct input_dev dev;
struct timeval cur_tv;

static unsigned int old_tv_usec = 0;

static unsigned int read_button_state(void)
{
	unsigned int state;

	state = au_readl(SYS_PINSTATERD) & BUTTON_MASK; /* get gpio status */

	state ^= BUTTON_INVERT;		/* invert main & guide button */

	/* printk("au1200_ibutton.c: button state [0x%X]\r\n",state); */
	return state;
}

//This function returns 0 if the allowed microseconds have elapsed since the last call to ths function, otherwise it returns 1 to indicate a bounce condition
static unsigned int bounce() 
{

	unsigned int elapsed_time;

	do_gettimeofday (&cur_tv);    

	if (!old_tv_usec) {
		old_tv_usec = cur_tv.tv_usec;
		return 0;
	}

	if(cur_tv.tv_usec > old_tv_usec) {
		/* If there hasn't been rollover */
		elapsed_time =  ((cur_tv.tv_usec - old_tv_usec));
	}
	else {
		/* Accounting for rollover */
		elapsed_time =  ((1000000 - old_tv_usec + cur_tv.tv_usec));
	}

	if (elapsed_time > 250000) {
		old_tv_usec = 0;	/* reset the bounce time */
		return 0;
	}

	return 1;
}

/* button interrupt handler */
static void button_interrupt(int irq, void *dev, struct pt_regs *regs)
{

	unsigned int i,bit_mask, key_choice;
	u32 button_state;
	
	/* Report state to upper level */
	
	button_state = read_button_state() & BUTTON_MASK; /* get new gpio status */

	/* Return if this is a repeated (bouncing) event */
	if(bounce())
		return;

	/* we want to make keystrokes */
	for( i=0; i< BUTTON_COUNT; i++) {
		bit_mask = 1<<i;
		if (button_state & bit_mask) {
			key_choice = button_map[i];
			/* toggle key down */
			input_report_key(dev, key_choice, 1);
			/* toggle key up */
			input_report_key(dev, key_choice, 0);
			printk("ibutton gpio %d stat %x scan code %d\r\n", 
					i, button_state, key_choice);
			/* Only report the first key event; it doesn't make 
			 * sense for two keys to be pressed at the same time, 
			 * and causes problems with the directional keys 
			 * return;	
			 */
		}
	}
}

static int 
button_translate(unsigned char scancode, unsigned char *keycode, char raw_mode) 
{
	static int prev_scancode;
	
	printk( "ibutton.c: translate: scancode=%x raw_mode=%x\n", 
			scancode, raw_mode);

	if (scancode == 0xe0 || scancode == 0xe1) {
		prev_scancode = scancode;
		return 0;
	}

	if (scancode == 0x00 || scancode == 0xff) {
		prev_scancode = 0;
		return 0;
	}

	*keycode = scancode;

	return 1;
}

/* init button hardware */
static int button_hw_init(void)
{
	unsigned int	ipinfunc=0;	

	printk("au1200_ibutton.c: Initializing buttons hardware\n");

	// initialize GPIO pin function assignments	

	ipinfunc = au_readl(SYS_PINFUNC);

	ipinfunc &= ~(SYS_PINFUNC_DMA | SYS_PINFUNC_S0A | SYS_PINFUNC_S0B);	
	au_writel( ipinfunc ,SYS_PINFUNC);
	
	ipinfunc |=  (SYS_PINFUNC_S0C);
	au_writel( ipinfunc ,SYS_PINFUNC);
	
	return 0;
}

/* button driver init */
static int __init button_init(void)
{
	int ret, i;
	unsigned int flag=0;

	printk("au1200_ibutton.c: button_init()\r\n");
	
	button_hw_init();
	
	/* register all button irq handler */
	
	for(i=0; i< sizeof(button_map)/sizeof(button_map[0]); i++)
	{
		/* register irq <-- gpio 1 ,6 ,12 , 17 ,19 , 26 ,28 */
		if(button_map[i] != 0)	
		{
			ret = request_irq(AU1000_GPIO_0 + i , 
					&button_interrupt , SA_INTERRUPT , 
					DRIVER_NAME , &dev);
			if(ret) flag |= 1<<i;
		}
	}

	printk("au1200_ibutton.c: request_irq,ret:0x%x\r\n",ret);
	
	if (ret) {
		printk("au1200_ibutton.c: request_irq:%X failed\r\n",flag);
		return ret;
	}
		
	dev.name = DRIVER_NAME;
	dev.evbit[0] = BIT(EV_KEY) | BIT(EV_REP);

	for (i=0;i<sizeof(button_map)/sizeof(button_map[0]);i++)
	{
		dev.keybit[LONG(button_map[i])] |= BIT(button_map[i]);
	}
	
	input_register_device(&dev);

	/* ready to receive interrupts */

	return 0;
}

/* button driver exit */
static void __exit button_exit(void)
{
	int i;
	
	for(i=0;i<sizeof(button_map)/sizeof(button_map[0]);i++)
	{
		if(button_map[i] != 0)
		{
			free_irq( AU1000_GPIO_0 + i, &dev);
		}
	}
	
	input_unregister_device(&dev);
	
	printk("au1200_ibutton.c: button_exit()\r\n");
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");
