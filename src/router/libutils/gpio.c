/*
 * gpio.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 * generic GPIO abstraction module for all supported platforms
 */
/*
 */
#include <string.h>
#include <unistd.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#if defined(HAVE_AR531X) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_ADM5120)

void set_gpio(int gpio, int value)
{
	FILE *in;
	char buf[64];
	if (gpio < 32) {
		sprintf(buf, "/proc/gpio/%d_dir", gpio);
		in = fopen(buf, "wb");
		if (in == NULL)
			return;
		fprintf(in, "1");
		fclose(in);
		sprintf(buf, "/proc/gpio/%d_out", gpio);
	} else {
		sprintf(buf, "/proc/wl0gpio/%d_out", gpio);
	}
	in = fopen(buf, "wb");
	if (in == NULL)
		return;
	fprintf(in, "%d", value);
	fclose(in);
}

int get_gpio(int gpio)
{
	FILE *in;
	int ret;
	char buf[64];

	sprintf(buf, "/proc/gpio/%d_dir", gpio);
	in = fopen(buf, "wb");
	if (in == NULL)
		return 0;
	fprintf(in, "0");
	fclose(in);
	sprintf(buf, "/proc/gpio/%d_in", gpio);
	in = fopen(buf, "rb");
	if (in == NULL)
		return 0;
	fscanf(in, "%d", &ret);
	fclose(in);
	return ret;
}

#elif HAVE_XSCALE
#define u8 unsigned char
#define u32 unsigned long

// #include <linux/ixp425-gpio.h>

#include <asm/hardware.h>
#include <asm-arm/arch-ixp4xx/ixp4xx-regs.h>

#define IXP4XX_GPIO_OUT 		0x1
#define IXP4XX_GPIO_IN  		0x2

struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

char *filename = "/dev/gpio";

void set_gpio(int gpio, int value)
{
	int file;
	struct gpio_bit _bit;

	/*
	 * open device 
	 */
	if ((file = open(filename, O_RDWR)) == -1) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: could not open %s (%d)\n", filename,
			errno);
		return;
	}

	/*
	 * Config bit as output 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_OUT;
	if (ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return;
	}

	/*
	 * Write data 
	 */
	_bit.bit = gpio;
	_bit.state = value;
	if (ioctl(file, GPIO_SET_BIT, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
	}

	close(file);

}

int get_gpio(int gpio)
{
	int file;
	struct gpio_bit _bit;

	/*
	 * open device 
	 */
	if ((file = open(filename, O_RDONLY)) == -1) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		return 1;
	}

	/*
	 * Config pin as input 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_IN;
	if (ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return 1;
	}

	/*
	 * Read data 
	 */
	_bit.bit = gpio;
	if (ioctl(file, GPIO_GET_BIT, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return 1;
	}

	close(file);
	return _bit.state;
}

#elif HAVE_STORM
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#define u8 unsigned char
#define u32 unsigned long

#define GPIO_GET_BIT	0x0000001
#define GPIO_SET_BIT	0x0000002
#define GPIO_GET_CONFIG	0x0000003
#define GPIO_SET_CONFIG 0x0000004

#define IXP4XX_GPIO_OUT 		0x1
#define IXP4XX_GPIO_IN  		0x2

struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

char *filename = "/dev/gpio";

void set_gpio(int gpio, int value)
{
	int file;
	struct gpio_bit _bit;

	/*
	 * open device 
	 */
	if ((file = open(filename, O_RDWR)) == -1) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: could not open %s (%d)\n", filename,
			errno);
		return;
	}

	/*
	 * Config bit as output 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_OUT;
	if (ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return;
	}

	/*
	 * Write data 
	 */
	_bit.bit = gpio;
	_bit.state = value;
	if (ioctl(file, GPIO_SET_BIT, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
	}

	close(file);

}

int get_gpio(int gpio)
{
	int file;
	struct gpio_bit _bit;

	/*
	 * open device 
	 */
	if ((file = open(filename, O_RDONLY)) == -1) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: could not open %s (%d)\n", filename,
			errno);
		return 1;
	}

	/*
	 * Config pin as input 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_IN;
	if (ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return 1;
	}

	/*
	 * Read data 
	 */
	_bit.bit = gpio;
	if (ioctl(file, GPIO_GET_BIT, (long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n",
			strerror(errno), errno);
		close(file);
		return 1;
	}

	close(file);
	return _bit.state;
}

#elif HAVE_RT2880
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#define GPIO_DEV	"/dev/gpio"
#define	RALINK_GPIO_SET_DIR		0x01
#define RALINK_GPIO_SET_DIR_IN		0x11
#define RALINK_GPIO_SET_DIR_OUT		0x12
#define	RALINK_GPIO_READ		0x02
#define	RALINK_GPIO_WRITE		0x03
#define	RALINK_GPIO_SET			0x21
#define	RALINK_GPIO_CLEAR		0x31
#define	RALINK_GPIO_READ_BIT		0x04
#define	RALINK_GPIO_WRITE_BIT		0x05
#define	RALINK_GPIO_READ_BYTE		0x06
#define	RALINK_GPIO_WRITE_BYTE		0x07
#define	RALINK_GPIO_READ_INT		0x02	//same as read
#define	RALINK_GPIO_WRITE_INT		0x03	//same as write
#define	RALINK_GPIO_SET_INT		0x21	//same as set
#define	RALINK_GPIO_CLEAR_INT		0x31	//same as clear
#define RALINK_GPIO_ENABLE_INTP		0x08
#define RALINK_GPIO_DISABLE_INTP	0x09
#define RALINK_GPIO_REG_IRQ		0x0A
#define RALINK_GPIO_LED_SET		0x41

int gpio_set_dir_in(int gpio)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, gpio) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_set_dir_out(int gpio)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_SET_DIR_OUT, gpio) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

#define RALINK_GPIO_DATA_LEN		24

int gpio_read_bit(int idx, int *value)
{
	int fd, req;

	*value = 0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (0L <= idx && idx < RALINK_GPIO_DATA_LEN)
		req = RALINK_GPIO_READ_BIT | (idx << RALINK_GPIO_DATA_LEN);
	else {
		close(fd);
		printf("gpio_read_bit: index %d out of range\n", idx);
		return -1;
	}
	if (ioctl(fd, req, value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_write_bit(int idx, int value)
{
	int fd, req;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	value &= 1;
	if (0L <= idx && idx < RALINK_GPIO_DATA_LEN)
		req = RALINK_GPIO_WRITE_BIT | (idx << RALINK_GPIO_DATA_LEN);
	else {
		close(fd);
		printf("gpio_write_bit: index %d out of range\n", idx);
		return -1;
	}
	if (ioctl(fd, req, value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

void set_gpio(int pin, int value)
{
	gpio_set_dir_out(pin);
	gpio_write_bit(pin, value);
}

int get_gpio(int pin)
{
	int value;
	gpio_set_dir_in(pin);
	gpio_read_bit(pin, &value);
	return value;
}

#elif HAVE_OPENRISC

#define GPIO_CMD_GET_BTN_RST	1
#define GPIO_CMD_SET_BTN_RST	2
#define GPIO_CMD_GET_LEDS		3
#define GPIO_CMD_SET_LEDS		4
#define GPIO_CMD_SET_LED_POWER	5
#define GPIO_CMD_SET_LED_BLUE	6
#define GPIO_CMD_SET_LED_GREEN	7
#define GPIO_CMD_SET			8
#define GPIO_CMD_GET			9
#define GPIO_CMD_SET_CTRL		10
#define GPIO_CMD_GET_CTRL		11
#define GPIO_CMD_SET_IRQMASK	12
#define GPIO_CMD_GET_IRQMASK	13
#define GPIO_CMD_SET_CHANGE		14	//!< obsolete
#define GPIO_CMD_GET_CHANGE		15
#define GPIO_CMD_SET_CHANGES	16	//!< obsolete
#define GPIO_CMD_GET_CHANGES	17
#define GPIO_CMD_SET_BUZZER		18
#define GPIO_CMD_GET_BUZZER		19
#define GPIO_CMD_SET_BUZZER_FRQ	20
#define GPIO_CMD_GET_BUZZER_FRQ	21
#define GPIO_CMD_SET_LED_BTN_WLAN	22
#define GPIO_CMD_GET_BTN_WLAN	23

struct gpio_struct {
	unsigned long mask;
	unsigned long value;
};

#define GPIO_DEV	"/dev/misc/gpio"
void set_gpio(int pin, int value)
{
	int fd, req;
	int cmd;

	switch (pin) {
	case 0:
		cmd = GPIO_CMD_SET_BTN_RST;
		break;
	case 1:
		cmd = GPIO_CMD_SET_LED_BLUE;
		break;
	case 2:
		cmd = GPIO_CMD_SET_LED_POWER;
		break;
	case 3:
		cmd = GPIO_CMD_SET_LED_GREEN;
		break;
	case 4:
		cmd = GPIO_CMD_SET_BUZZER;
		break;
	case 5:
		cmd = GPIO_CMD_SET_LED_BTN_WLAN;
		break;
	}
	if (pin > 5 && pin < 16)
		return;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return;
	}
	if (pin >= 16 && pin < 128) {
		pin -= 16;
		struct gpio_struct set;
		set.mask = 1 << pin;
		cmd = GPIO_CMD_SET_CTRL;
		set.value = 1 << pin;
		if (ioctl(fd, cmd, &set) < 0) {
			perror("ioctl");
		}

		cmd = GPIO_CMD_SET;
		if (value)
			set.value = value << pin;
		else
			set.value = 0;
		if (ioctl(fd, cmd, &set) < 0) {
			perror("ioctl");
		}
	} else {
		ioctl(fd, cmd, &value);	// silently ignore                      
	}
	close(fd);
}

int get_gpio(int pin)
{
	int fd, req;
	int cmd, value;

	switch (pin) {
	case 0:
		cmd = GPIO_CMD_GET_BTN_RST;
		break;
	case 1:
	case 2:
	case 3:
		cmd = GPIO_CMD_GET_LEDS;
		break;
	case 4:
		cmd = GPIO_CMD_GET_BUZZER;
		break;
	case 5:
		cmd = GPIO_CMD_GET_BTN_WLAN;
		break;
	}
	if (pin > 5 && pin < 16)
		return 0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (pin >= 16 && pin < 128) {
		pin -= 16;

		struct gpio_struct set;
		set.mask = 1 << pin;
		cmd = GPIO_CMD_SET_CTRL;
		set.value = 0;
		if (ioctl(fd, cmd, &set) < 0) {
			perror("ioctl");
		}
		cmd = GPIO_CMD_GET;
		if (ioctl(fd, cmd, &value) < 0) {
			perror("ioctl");
			close(fd);
			return -1;
		}
		value = value & (1 << pin);
		if (value)
			value = 1;
	} else {
		if (ioctl(fd, cmd, &value) < 0) {
			close(fd);	// silently ignore errors
			return 0;
		}
		close(fd);
		if (cmd >= 1 && cmd <= 3) {
			if ((value & 0x2) && cmd == 1)
				return 1;
			if ((value & 0x1) && cmd == 2)
				return 1;
			if ((value & 0x4) && cmd == 3)
				return 1;
			return 0;
		}
		if (pin == 5) {
			if (value)
				return 1;
			else
				return 0;
		}
		return value;
	}
	close(fd);
	return value;
}

#elif HAVE_LAGUNA
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#define IOC_GPIODEV_MAGIC  'B'

#define _IOC_NRBITS	8
#define _IOC_TYPEBITS	8

/*
 * Let any architecture override either of the following before
 * including this file.
 */

#ifndef _IOC_SIZEBITS
# define _IOC_SIZEBITS	14
#endif

#ifndef _IOC_DIRBITS
# define _IOC_DIRBITS	2
#endif

#define _IOC_NRMASK	((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK	((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK	((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK	((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 */

#ifndef _IOC_NONE
# define _IOC_NONE	0U
#endif

#ifndef _IOC_WRITE
# define _IOC_WRITE	1U
#endif

#ifndef _IOC_READ
# define _IOC_READ	2U
#endif

#define _IOC(dir,type,nr,size) \
	(((dir)  << _IOC_DIRSHIFT) | \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT) | \
	 ((size) << _IOC_SIZESHIFT))

#define _IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)

#define GPIO_GET        _IO(IOC_GPIODEV_MAGIC, 10)
#define GPIO_SET        _IO(IOC_GPIODEV_MAGIC, 11)
#define GPIO_CLEAR      _IO(IOC_GPIODEV_MAGIC, 12)
#define GPIO_DIR_IN     _IO(IOC_GPIODEV_MAGIC, 13)
#define GPIO_DIR_OUT    _IO(IOC_GPIODEV_MAGIC, 14)

void set_gpio(int pin, int value)
{
	int fd;
	if ((fd = open("/dev/misc/gpio", O_RDWR)) < 0) {
		printf("Error whilst opening /dev/gpio\n");
		return;
	}

	switch (pin) {
	case 16:		// main LED
		sysprintf
		    ("echo %d > /sys/devices/platform/leds-gpio/leds/user1/brightness",
		     value);
		break;
	default:
		ioctl(fd, GPIO_DIR_OUT, pin);
		if (value)
			ioctl(fd, GPIO_SET, pin);
		else
			ioctl(fd, GPIO_CLEAR, pin);
		break;
	}
	close(fd);
}

int get_gpio(int pin)
{
	int fd;
	if ((fd = open("/dev/misc/gpio", O_RDWR)) < 0) {
		printf("Error whilst opening /dev/gpio\n");
		return -1;
	}
	ioctl(fd, GPIO_DIR_IN, pin);
	int ret = ioctl(fd, GPIO_GET, pin);
	close(fd);
	return ret;

}

#else

void set_gpio(int pin, int value)
{
	int gpioouten = open("/dev/gpio/outen", O_RDWR);
	int gpioout = open("/dev/gpio/out", O_RDWR);
	unsigned int gpio;

	read(gpioouten, &gpio, sizeof(gpio));
	gpio |= 1 << pin;
	write(gpioouten, &gpio, sizeof(gpio));

	read(gpioout, &gpio, sizeof(gpio));
	if (value) {
		gpio |= (1 << pin);
	} else {
		gpio &= ~(1 << pin);
	}
	write(gpioout, &gpio, sizeof(gpio));
	close(gpioout);
	close(gpioouten);
}

int get_gpio(int pin)
{
	unsigned int gpio;
	int gpioouten = open("/dev/gpio/outen", O_RDWR);
	int gpioin = open("/dev/gpio/in", O_RDWR);

	read(gpioouten, &gpio, sizeof(gpio));
	gpio &= ~(1 << pin);
	write(gpioouten, &gpio, sizeof(gpio));
	read(gpioin, &gpio, sizeof(gpio));
	gpio = (gpio & (1 << pin)) ? 1 : 0;
	close(gpioin);
	close(gpioouten);
	return gpio;
}

#endif
