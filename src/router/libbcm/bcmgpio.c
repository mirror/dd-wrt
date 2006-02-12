/*
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * Functions to read/write GPIO pins
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include <bcmgpio.h>



#if BCM_DEBUG
#define GPIO_ERROR(fmt, args...) printf("%s: " fmt "\n" , __FUNCTION__ , ## args)
#else
#define GPIO_ERROR(fmt, args...)
#endif

/* GPIO registers */
#define BCMGPIO_REG_IN		0
#define BCMGPIO_REG_OUT		1
#define BCMGPIO_REG_OUTEN	2
#define BCMGPIO_REG_CONTROL 	3

#define BCMGPIO_MAX_FD		4

#define BCMGPIO_STRB_MS_TIME	50	/* 50 ms */
#define BCMGPIO_STRB_NS_TIME 	(BCMGPIO_STRB_MS_TIME * 1000 * 1000)	/* in ns units */

/* GPIO information */
typedef struct {
	int connected;				/* is gpio being used? */
	bcmgpio_dirn_t dirn;			/* direction: IN or OUT */
	int owner;				/* did Chip Common own this gpio? */
	unsigned long on_time;			/* in 10 ms units */
	unsigned long strobe_period;		/* in 10 ms units */
	unsigned long timer_count;		/* 10 ms tick counter */
	unsigned long strobe_count;		/* strobe counter */
	unsigned long tot_strobes;		/* total number of strobes */
	unsigned long orig_state;		/* original state of GPIO before blinking */
	int strobing;				/* is gpio strobing? */
	int *strobe_done;			/* pointer to memory which is used to signal strobe completion */
	bcm_timer_id timer_id;			/* id of the strobe timer */
} bcmgpio_info_t;

/* Bitmask of connected pins */
unsigned long connect_mask;

/* Global containing the file descriptors of the GPIO kernel drivers */
static int bcmgpio_fd[BCMGPIO_MAX_FD];

/* Global containing information about each GPIO pin */
static bcmgpio_info_t bcmgpio_info[BCMGPIO_MAXPINS];

/* Kernel GPIO driver names */
static char *drvnames [ ] = {
	"/dev/gpio/in",
	"/dev/gpio/out",
	"/dev/gpio/outen",
	"/dev/gpio/control"
};

/* Static function prototypes */

/* Generic functions to read/write Chip Common core's GPIO registers on the AP */
static int bcmgpio_drvinit ();
static void bcmgpio_drvcleanup ();
static int bcmgpio_drvread (int gpioreg, unsigned long *value);
static int bcmgpio_drvwrite (int gpioreg, unsigned long *value);

static void bcmgpio_toggle (unsigned long gpio_mask);
static void bcmgpio_timercb (bcm_timer_id tid, int gpio_pin);

/**********************************************************************************************
 *  Functions visible to this file only
******************************************************************************************** */
static int 
bcmgpio_drvinit ()
{
	int count;

	for (count = 0; count < BCMGPIO_MAX_FD; count++)
		bcmgpio_fd[count] = -1;

	for (count = 0; count < BCMGPIO_MAX_FD; count++) {
		bcmgpio_fd[count] = open(drvnames[count], O_RDWR);

		if (bcmgpio_fd[count] == -1) {
			GPIO_ERROR ("Failed to open %s\n", drvnames[count]);
			goto GPIO_INIT_ERR;
		}
	}

	return 0;

GPIO_INIT_ERR:
	for (count = 0; count < BCMGPIO_MAX_FD; count++) {
		if (bcmgpio_fd[count] != -1) {
			close (bcmgpio_fd[count]);
			bcmgpio_fd[count] = -1;
		}
	}

	return -1;
}

static void 
bcmgpio_drvcleanup ()
{
	int count;

	for (count = 0; count < BCMGPIO_MAX_FD; count++) {
		if (bcmgpio_fd[count] != -1) {
			close (bcmgpio_fd[count]);
			bcmgpio_fd[count] = -1;
		}
	}
}

static int 
bcmgpio_drvread (int gpioreg, unsigned long *value)
{
	if (bcmgpio_fd[gpioreg] != -1) {
		return (read (bcmgpio_fd[gpioreg], value, sizeof(unsigned long)));
	}
	else {
		GPIO_ERROR ("Driver %s is not open\n", drvnames[gpioreg]);
		return -1;
	}
}

static int 
bcmgpio_drvwrite (int gpioreg, unsigned long *value)
{
	if (bcmgpio_fd[gpioreg] != -1) {
		return (write (bcmgpio_fd[gpioreg], value, sizeof(unsigned long)));
	}
	else {
		GPIO_ERROR ("Driver %s is not open\n", drvnames[gpioreg]);
		return -1;
	}
}

static void 
bcmgpio_toggle (unsigned long gpio_mask)
{
	unsigned long regval;

	bcmgpio_drvread (BCMGPIO_REG_OUT, &regval);

	if (regval & gpio_mask)
		regval &= ~gpio_mask;
	else
		regval |= gpio_mask;

	bcmgpio_drvwrite (BCMGPIO_REG_OUT, &regval);
}


static void 
bcmgpio_timercb (bcm_timer_id tid, int gpio_pin)
{
	unsigned long bitmask;

	if (bcmgpio_info[gpio_pin].strobing) {
		bitmask = (unsigned long) 1 << gpio_pin;

		bcmgpio_info[gpio_pin].timer_count++;

		if (bcmgpio_info[gpio_pin].timer_count == bcmgpio_info[gpio_pin].on_time) {
			bcmgpio_toggle (bitmask);
		}
		else if (bcmgpio_info[gpio_pin].timer_count > bcmgpio_info[gpio_pin].on_time) {
			if (bcmgpio_info[gpio_pin].timer_count == bcmgpio_info[gpio_pin].strobe_period) {
				bcmgpio_info[gpio_pin].timer_count = 0;

				if (bcmgpio_info[gpio_pin].tot_strobes > 0) {
					bcmgpio_info[gpio_pin].strobe_count++;

					if (bcmgpio_info[gpio_pin].strobe_count == bcmgpio_info[gpio_pin].tot_strobes) {
						bcmgpio_strobe_stop (gpio_pin);
						bcmgpio_out (bitmask, bcmgpio_info[gpio_pin].orig_state);
						if (bcmgpio_info[gpio_pin].strobe_done != NULL)
							*(bcmgpio_info[gpio_pin].strobe_done) = 1;
						return;
					}
				}

				bcmgpio_toggle (bitmask);
			}
		}
	}
}


/**********************************************************************************************
 *  GPIO functions 
******************************************************************************************** */
int 
bcmgpio_connect (int gpio_pin, bcmgpio_dirn_t gpio_dirn)
{
	unsigned long regctrl;
	unsigned long regouten;
	unsigned long bitmask;

	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	
	assert ((gpio_dirn == BCMGPIO_DIRN_IN) || (gpio_dirn == BCMGPIO_DIRN_OUT));

	if (connect_mask == 0) {
		if (bcmgpio_drvinit () != 0) 
			return -1;
	}

	bcmgpio_drvread (BCMGPIO_REG_CONTROL, &regctrl);
	bcmgpio_drvread (BCMGPIO_REG_OUTEN, &regouten);

	if (bcmgpio_info[gpio_pin].connected)
		return -1;

	memset(&bcmgpio_info[gpio_pin], 0, sizeof(bcmgpio_info_t));

	bcmgpio_info[gpio_pin].connected = 1;
	bcmgpio_info[gpio_pin].dirn = gpio_dirn;

	bitmask = ((unsigned long) 1 << gpio_pin);

	/* Bit is set if Chip Common does not own this GPIO */ 
	if (bitmask & regctrl) {
		/* Take over this GPIO, from original owner */
		regctrl &= ~bitmask;
	}
	else {
		bcmgpio_info[gpio_pin].owner = 1;
	}

	if (gpio_dirn == BCMGPIO_DIRN_IN)
		regouten &= ~bitmask;
	else
		regouten |= bitmask;

	connect_mask |= bitmask;

	bcmgpio_drvwrite (BCMGPIO_REG_CONTROL, &regctrl);
	bcmgpio_drvwrite (BCMGPIO_REG_OUTEN, &regouten);

	return 0;
}


int 
bcmgpio_disconnect (int gpio_pin)
{
	unsigned long bitmask;
	unsigned long regctrl;

	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	

	bitmask = ((unsigned long) 1 << gpio_pin);

	if ((connect_mask & bitmask) == 0)
		return -1;

	bcmgpio_drvread (BCMGPIO_REG_CONTROL, &regctrl);

	if (! bcmgpio_info[gpio_pin].connected)
		return -1;

	if (bcmgpio_info[gpio_pin].strobing)
		bcmgpio_strobe_stop (gpio_pin);

	bcmgpio_info[gpio_pin].connected = 0;

	/* 
	 * If Chip Common was not the original owner of this GPIO
	 * then surrender ownership 
	 */
	if (! bcmgpio_info[gpio_pin].owner)
		regctrl |= bitmask;

	bcmgpio_drvwrite (BCMGPIO_REG_CONTROL, &regctrl);

	connect_mask &= ~bitmask;

	if (connect_mask == 0)
		bcmgpio_drvcleanup ();

	return 0;
}

int 
bcmgpio_in (unsigned long gpio_mask, unsigned long *value)
{
	unsigned long regin;

	if ((gpio_mask & connect_mask) != gpio_mask)
		return -1;

	bcmgpio_drvread (BCMGPIO_REG_IN, &regin);
	*value = regin & gpio_mask;

	return 0;
}


int 
bcmgpio_out (unsigned long gpio_mask, unsigned long value)
{
	unsigned long regout;

	if ((gpio_mask & connect_mask) != gpio_mask)
		return -1;

	bcmgpio_drvread (BCMGPIO_REG_OUT, &regout);
	regout &= ~gpio_mask;
	regout |= (value & gpio_mask);
	bcmgpio_drvwrite (BCMGPIO_REG_OUT, &regout);

	return 0;
}


int 
bcmgpio_strobe_start (int gpio_pin, bcmgpio_strobe_t *strobe_info)
{
	unsigned long regout;
	int status;
	struct itimerspec its;

	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	

	if (! strobe_info->timer_module) {
		GPIO_ERROR ("bcmgpio_strobe: Invalid timer module ID\n");
		return -1;
	}

	if (! bcmgpio_info[gpio_pin].connected)
		return -1;

	if (bcmgpio_info[gpio_pin].dirn == BCMGPIO_DIRN_IN)
		return -1;

	if (bcmgpio_info[gpio_pin].strobing)
		return 0;

	if ((status = bcm_timer_create (strobe_info->timer_module, &bcmgpio_info[gpio_pin].timer_id)) != 0) {
		bcmgpio_info[gpio_pin].timer_id = 0;
		GPIO_ERROR ("bcmgpio_strobe: Timer creation failed with error %d\n", status);
		return -1;
	}

	if ((status = bcm_timer_connect (bcmgpio_info[gpio_pin].timer_id, (bcm_timer_cb) bcmgpio_timercb, (int) gpio_pin)) != 0) {
		bcm_timer_delete (bcmgpio_info[gpio_pin].timer_id);
		bcmgpio_info[gpio_pin].timer_id = 0;
		GPIO_ERROR ("bcmgpio_strobe: Timer connect failed with error %d\n", status);
		return -1;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = BCMGPIO_STRB_NS_TIME;
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = BCMGPIO_STRB_NS_TIME;
		
	if ((status = bcm_timer_settime (bcmgpio_info[gpio_pin].timer_id, &its)) != 0) {
		bcm_timer_delete (bcmgpio_info[gpio_pin].timer_id);
		bcmgpio_info[gpio_pin].timer_id = 0;
		GPIO_ERROR ("bcmgpio_strobe: Timer set failed with error %d\n", status);
		return -1;
	}

	bcmgpio_drvread (BCMGPIO_REG_OUT, &regout);

	bcmgpio_info[gpio_pin].orig_state = regout & ((unsigned long) 1 << gpio_pin);
			
	bcmgpio_info[gpio_pin].strobe_period = strobe_info->strobe_period_in_ms / BCMGPIO_STRB_MS_TIME;
	bcmgpio_info[gpio_pin].on_time = 
		(strobe_info->duty_percent * bcmgpio_info[gpio_pin].strobe_period) / 100;
	bcmgpio_info[gpio_pin].tot_strobes = strobe_info->num_strobes;
	bcmgpio_info[gpio_pin].strobe_count = 0;
	bcmgpio_info[gpio_pin].timer_count = 0;

	bcmgpio_info[gpio_pin].strobing = 1;

	bcmgpio_info[gpio_pin].strobe_done = strobe_info->strobe_done;
	if (bcmgpio_info[gpio_pin].strobe_done != NULL)
		*(bcmgpio_info[gpio_pin].strobe_done) = 0;

	return 0;
}


int
bcmgpio_strobe_stop (int gpio_pin)
{
	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	

	if (! bcmgpio_info[gpio_pin].connected)
		return -1;

	if (bcmgpio_info[gpio_pin].strobing) {
		bcmgpio_info[gpio_pin].strobing = 0;

		if (bcmgpio_info[gpio_pin].timer_id != 0) {
			bcm_timer_delete (bcmgpio_info[gpio_pin].timer_id);
			bcmgpio_info[gpio_pin].timer_id = 0;
		}
	}

	return 0;
}

