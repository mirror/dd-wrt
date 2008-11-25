/* $Id: gpio.c 19454 2008-05-06 06:48:10Z saulius $ */

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


#define u8 unsigned char
#define u32 unsigned long 

#include "gpio.h"

void *iomem;

void gpio_init(void)
{
	int iofd;
	uint32_t dir;
	unsigned long ioaddr = GPIO0;

	iofd = open("/dev/mem", O_RDWR);
	if (iofd < 0)
	{
		printf("Can't open memory.\n");
		exit(-1);
	}

	iomem = mmap((void*)ioaddr, getpagesize(),
		     PROT_READ|PROT_WRITE,
		     MAP_SHARED|MAP_FIXED,
		     iofd, ioaddr);

	if (iomem == MAP_FAILED)
	{
		printf("MAP_FAILED!\n");
		exit(-2);
	}

	close(iofd);
	
	
	dir = gpio_readl(iomem + 0x08);
	gpio_writel(dir | 1<<12 | 1<<13 | 1<<8 | 1<<7 | 1<<6 , iomem + 0x08);
	dir = gpio_readl(iomem + 0x08);
}

uint32_t gpio_readl(void *ptr)
{
	uint32_t *data = ptr;
	uint32_t value;

	value = *data;
	return *data;
}

void gpio_writel(uint32_t value, void *ptr)
{
	uint32_t *data = ptr;
	*data = value;
}


void gpio_close(void)
{
	munmap(iomem, getpagesize());
}

void write_bit(uint32_t bit, uint32_t val)
{
	if(val == 0)
		gpio_writel(1<<bit, iomem + GPIO_DATA_CLR);
	
	if(val == 1)
		gpio_writel(1<<bit, iomem + GPIO_DATA_SET);
}


uint32_t read_bit(uint32_t bit)
{
	uint32_t data;
		
	data = gpio_readl(iomem + 0x4);
	
	// TODO: finish	
	return 0;
}
