/* $Id: gpio.h 19454 2008-05-06 06:48:10Z saulius $ */

/*
 * Generic GPIO interface
 * This contains the part of the user interface to the gpio service.
 */
#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h> 

/* defines a specific GPIO bit number and state */
struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

#define GPIO_MAJOR    127

/*
 * ioctl calls that are permitted to the /dev/gpio interface
 */
#define GPIO_GET_BIT	0x0000001
#define GPIO_SET_BIT	0x0000002
#define GPIO_GET_CONFIG	0x0000003
#define GPIO_SET_CONFIG 0x0000004



#define GPIO_CONFIG_OUT  1
#define GPIO_CONFIG_IN   2





#define GPIO0	0x4D000000
#define GPIO_DATA_SET	0x10
#define GPIO_DATA_CLR	0x14

void gpio_init();
void gpio_close(void);
void write_bit(uint32_t bit, uint32_t val);
uint32_t read_bit(uint32_t bit);
uint32_t gpio_readl(void *ptr);
void gpio_writel(uint32_t value, void *ptr);

#endif // _GPIO_H_
