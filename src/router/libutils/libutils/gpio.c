/*
 * gpio.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

static void set_linux_gpio(int pin, int value)
{
	char str[32];
	char strdir[64];
	int fd;
	int tries = 0;
	snprintf(str, sizeof(str), "/sys/class/gpio/gpio%d/value", pin);
	snprintf(strdir, sizeof(strdir), "/sys/class/gpio/gpio%d/direction", pin);
new_try:;
	fd = open(str, O_RDONLY);
	if (fd == -1) {
		if (writeint("/sys/class/gpio/export", pin))
			return; //prevent deadlock
		if ((tries++) < 10)
			goto new_try;
		else {
			fprintf(stderr, "gpio %d has a problem\n", pin);
			return;
		}
	}
	close(fd);
	writestr(strdir, "out");
	writeint(str, value);
}

static int get_linux_gpio(int pin)
{
	char str[32];
	char strdir[64];
	FILE *fp;
	int fd;
	int tries = 0;
	int val = 0;
	sprintf(str, "/sys/class/gpio/gpio%d/value", pin);
	sprintf(strdir, "/sys/class/gpio/gpio%d/direction", pin);
new_try:;
	fp = fopen(str, "rb");
	if (!fp) {
		if (writeint("/sys/class/gpio/export", pin))
			return 0; // prevent deadlock
		if ((tries++) < 10)
			goto new_try;
		else {
			fprintf(stderr, "gpio %d has a problem\n", pin);
			return 0;
		}
	}
	fclose(fp);
	writestr(strdir, "in");
	fp = fopen(str, "rb");
	if (fp) {
		fscanf(fp, "%d", &val);
		fclose(fp);
	}
	return val;
}

#if defined(HAVE_UNIWIP) || defined(HAVE_OCTEON)
void set_gpio(int pin, int value)
{
	set_linux_gpio(pin, value);
}

int get_gpio(int pin)
{
	return get_linux_gpio(pin);
}

#elif defined(HAVE_WDR4900)
void set_gpio(int gpio, int value)
{
	switch (gpio) {
	case 0: // system
		writeint("/sys/devices/platform/leds/leds/tp-link:blue:system/brightness", value);
		break;
	case 1: // usb1
		writeint("/sys/devices/platform/leds/leds/tp-link:green:usb1/brightness", value);
		break;
	case 2: // usb2
		writeint("/sys/devices/platform/leds/leds/tp-link:green:usb2/brightness", value);
		break;
	case 3: // usbpower
		writeint("/sys/devices/platform/leds/leds/tp-link:usb:power/brightness", value);
		break;
	}
}

int get_gpio(int gpio)
{
	return get_linux_gpio(480 + gpio);
}

#elif defined(HAVE_WRT1900AC)

int get_gpio(int pin)
{
	return get_linux_gpio(pin);
}

void set_gpio(int gpio, int value)
{
	//value 0 off 255 on
	if (value == 1)
		value = 255;
	//fprintf(stderr, "GPIO %d value %d\n", gpio, value);
	int brand = getRouterBrand();
	if (brand == ROUTER_WRT_1900AC) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/mamba:white:power/brightness", value);
			break;
		case 1: // 2G
			writeint("/sys/class/leds/mamba:white:wlan_2g/brightness", value);
			break;
		case 2: // 5G
			writeint("/sys/class/leds/mamba:white:wlan_5g/brightness", value);
			break;
		case 3: // 5G
			writeint("/sys/class/leds/mamba:white:esata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/mamba:white:usb3_1/brightness", value);
			break;
		case 5:
			writeint("/sys/class/leds/mamba:white:usb2/brightness", value);
			break;
		case 6:
			writeint("/sys/class/leds/mamba:white:wan/brightness", value);
			break;
		case 7:
			writeint("/sys/class/leds/mamba:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/mamba:white:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/mamba:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/mamba:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}

	if (brand == ROUTER_WRT_1200AC) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/caiman:white:power/brightness", value);
			break;
		case 1: // 2G
			writeint("/sys/class/leds/pca963x:caiman:white:wlan_2g/brightness", value);
			break;
		case 2: // 5G
			writeint("/sys/class/leds/pca963x:caiman:white:wlan_5g/brightness", value);
			break;
		case 3:
			writeint("/sys/class/leds/caiman:white:sata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/pca963x:caiman:white:usb3_1/brightness", value);
			break;
		case 5: // 5G
			writeint("/sys/class/leds/pca963x:caiman:white:usb2/brightness", value);
			break;
		case 6: // power
			writeint("/sys/class/leds/pca963x:caiman:white:wan/brightness", value);
			break;
		case 7: // power
			writeint("/sys/class/leds/pca963x:caiman:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/pca963x:caiman:white:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/pca963x:caiman:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/pca963x:caiman:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}

	if (brand == ROUTER_WRT_1900ACV2) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/cobra:white:power/brightness", value);
			break;
		case 1: // 2G
			writeint("/sys/class/leds/pca963x:cobra:white:wlan_2g/brightness", value);
			break;
		case 2: // 5G
			writeint("/sys/class/leds/pca963x:cobra:white:wlan_5g/brightness", value);
			break;
		case 3:
			writeint("/sys/class/leds/cobra:white:sata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/pca963x:cobra:white:usb3_1/brightness", value);
			break;
		case 5: // 5G
			writeint("/sys/class/leds/pca963x:cobra:white:usb2/brightness", value);
			break;
		case 6: // power
			writeint("/sys/class/leds/pca963x:cobra:white:wan/brightness", value);
			break;
		case 7: // power
			writeint("/sys/class/leds/pca963x:cobra:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/pca963x:cobra:white:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/pca963x:cobra:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/pca963x:cobra:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}

	if (brand == ROUTER_WRT_1900ACS) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/shelby:white:power/brightness", value);
			break;
		case 1: // 2G
			writeint("/sys/class/leds/pca963x:shelby:white:wlan_2g/brightness", value);
			break;
		case 2: // 5G
			writeint("/sys/class/leds/pca963x:shelby:white:wlan_5g/brightness", value);
			break;
		case 3:
			writeint("/sys/class/leds/shelby:white:sata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/pca963x:shelby:white:usb3_1/brightness", value);
			break;
		case 5: // 5G
			writeint("/sys/class/leds/pca963x:shelby:white:usb2/brightness", value);
			break;
		case 6: // power
			writeint("/sys/class/leds/pca963x:shelby:white:wan/brightness", value);
			break;
		case 7: // power
			writeint("/sys/class/leds/pca963x:shelby:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/pca963x:shelby:white:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/pca963x:shelby:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/pca963x:shelby:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}

	if (brand == ROUTER_WRT_3200ACM) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/rango:white:power/brightness", value);
			break;
		case 3:
			writeint("/sys/class/leds/rango:white:sata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/pca963x:rango:white:usb3_1/brightness", value);
			break;
		case 5: // 5G
			writeint("/sys/class/leds/pca963x:rango:white:usb2/brightness", value);
			break;
		case 6: // power
			writeint("/sys/class/leds/pca963x:rango:white:wan/brightness", value);
			break;
		case 7: // power
			writeint("/sys/class/leds/pca963x:rango:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/pca963x:rango:white:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/pca963x:rango:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/pca963x:rango:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}

	if (brand == ROUTER_WRT_32X) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/venom:blue:power/brightness", value);
			break;
		case 3:
			writeint("/sys/class/leds/venom:blue:sata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/pca963x:venom:blue:usb3_1/brightness", value);
			break;
		case 5: // 5G
			writeint("/sys/class/leds/pca963x:venom:blue:usb2/brightness", value);
			break;
		case 6: // power
			writeint("/sys/class/leds/pca963x:venom:blue:wan/brightness", value);
			break;
		case 7: // power
			writeint("/sys/class/leds/pca963x:venom:amber:wan/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/pca963x:venom:blue:usb3_2/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/pca963x:venom:blue:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/pca963x:venom:amber:wps/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	}
}
#elif defined(HAVE_ALPINE)

int get_gpio(int gpio)
{
	return get_linux_gpio(gpio);
}

void set_gpio(int gpio, int value)
{
	set_linux_gpio(gpio, value);
}
#elif defined(HAVE_IPQ6018)
int get_gpio(int gpio)
{
	return get_linux_gpio(gpio + 512);
}

void set_gpio(int gpio, int value)
{
	//value 0 off 255 on
	if (value == 1)
		value = 255;
	int brand = getRouterBrand();
	if (brand == ROUTER_LINKSYS_MR7350) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/red:status/brightness", value);
			break;
		case 1:
			writeint("/sys/class/leds/green:status/brightness", value);
			break;
		case 2:
			writeint("/sys/class/leds/blue:status/brightness", value);
			break;
		default:
			set_linux_gpio(gpio, value);
			break;
		}
	} else
		set_linux_gpio(gpio + 512, value);
}

#elif defined(HAVE_IPQ806X)

int get_gpio(int gpio)
{
	int brand = getRouterBrand();
	if (brand == ROUTER_HABANERO || brand == ROUTER_ASUS_AC58U || brand == ROUTER_LINKSYS_EA8300) {
		return get_linux_gpio(gpio);
	}
	return get_linux_gpio(gpio + 443);
}

void set_gpio(int gpio, int value)
{
	//value 0 off 255 on
	if (value == 1)
		value = 255;
	//fprintf(stderr, "GPIO %d value %d\n", gpio, value);
	int brand = getRouterBrand();

	if (brand == ROUTER_NETGEAR_R7500 || brand == ROUTER_NETGEAR_R7500V2 || brand == ROUTER_NETGEAR_R7800) {
		switch (gpio) {
		case 0: // power
			writeint("/sys/class/leds/r7X00:white:power/brightness", value);
			break;
		case 1: // 2G
			break;
		case 2: // 5G
			break;
		case 3:
			writeint("/sys/class/leds/r7X00:white:esata/brightness", value);
			break;
		case 4:
			writeint("/sys/class/leds/r7X00:amber:usb1/brightness", value);
			break;
		case 5:
			writeint("/sys/class/leds/r7X00:amber:usb3/brightness", value);
			break;
		case 6:
			writeint("/sys/class/leds/r7X00:white:wan/brightness", value);
			break;
		case 7:
			writeint("/sys/class/leds/r7X00:white:internet/brightness", value);
			break;
		case 8:
			writeint("/sys/class/leds/r7X00:white:rfkill/brightness", value);
			break;
		case 9:
			writeint("/sys/class/leds/r7X00:white:wps/brightness", value);
			break;
		case 10:
			writeint("/sys/class/leds/r7X00:amber:status/brightness", value);
			break;
		default:
			if (gpio <= 64)
				set_linux_gpio(gpio + 443, value);
			else
				set_linux_gpio(gpio, value);
			break;
		}
	} else if (brand == ROUTER_LINKSYS_EA8500) {
		switch (gpio) {
		case 0:
			writeint("/sys/class/leds/ea8500:white:power/brightness", value);
			break;
		case 1:
			writeint("/sys/class/leds/ea8500:green:wifi/brightness", value);
			break;
		case 2:
			writeint("/sys/class/leds/ea8500:green:wps/brightness", value);
			break;
		default:
			if (gpio <= 64)
				set_linux_gpio(gpio + 443, value);
			else
				set_linux_gpio(gpio, value);
			break;
		}
	} else if (brand == ROUTER_HABANERO || brand == ROUTER_ASUS_AC58U || brand == ROUTER_LINKSYS_EA8300) {
		set_linux_gpio(gpio, value);
	} else {
		if (gpio <= 64)
			set_linux_gpio(gpio + 443, value);
		else
			set_linux_gpio(gpio, value);
	}
}

#elif defined(HAVE_AR531X) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_ADM5120)

void set_gpio(int gpio, int value)
{
	FILE *in;
	char buf[64];
#if defined(HAVE_ARCHERC25)
#define GPIOMAX 128
#elif defined(HAVE_DANUBE)
#define GPIOMAX 64
#else
#define GPIOMAX 32
#endif
	if (gpio < GPIOMAX) {
#ifdef HAVE_E380AC
		if (gpio ==
		    3) { // the red led on gpio 3 cannot be controlled by output register. it will switch on if direction is set to output and will go off if direction is input
			sprintf(buf, "/proc/gpio/%d_dir", gpio);
			writeint(buf, value);
			return;
		}
#endif
#if defined(HAVE_ARCHERC7V4) || defined(HAVE_ARCHERC25)
		if (gpio >= 24) {
			sysprintf("echo %d > /sys/devices/platform/leds-gpio/leds/generic_%d/brightness", value, gpio);
			return;
		}
#endif
		sprintf(buf, "/proc/gpio/%d_dir", gpio);
		if (writestr(buf, "1"))
			return;
		sprintf(buf, "/proc/gpio/%d_out", gpio);
	} else
#if defined(HAVE_ERC)
		if (gpio >= 55) {
		set_linux_gpio(gpio, value);
		return;
	} else
#endif
#ifdef HAVE_DANUBE
		if (gpio >= 200)
		sprintf(buf, "/proc/gpiostp/%d_out", gpio - 200);
	else
#endif
	{
		sprintf(buf, "/proc/wl0gpio/%d_out", (gpio - GPIOMAX));
	}

	writeint(buf, value);
}

int get_gpio(int gpio)
{
	FILE *in;
	int ret;
	char buf[64];
	if (gpio < GPIOMAX) {
		sprintf(buf, "/proc/gpio/%d_dir", gpio);
		if (writestr(buf, "0"))
			return -1;
		sprintf(buf, "/proc/gpio/%d_in", gpio);
		in = fopen(buf, "rb");
	} else {
		sprintf(buf, "/proc/wl0gpio/%d_dir", (gpio - GPIOMAX));

		if (writestr(buf, "0"))
			return -1;
		sprintf(buf, "/proc/wl0gpio/%d_in", (gpio - GPIOMAX));
		in = fopen(buf, "rb");
		if (in == NULL) {
			sprintf(buf, "/proc/wl0gpio/%d_out", (gpio - GPIOMAX));
			in = fopen(buf, "rb");
		}
	}
	if (in == NULL)
		return 0;
	fscanf(in, "%d", &ret);
	fclose(in);
	return ret;
#undef GPIOMAX
}

#elif defined(HAVE_NORTHSTAR)

static void set_gpio_base(int pin, int value)
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

void set_hc595(int pin, int value)
{
	int gpioout = open("/dev/gpio/hc595", O_RDWR);
	unsigned int gpio;
	gpio = pin << 4 | value;
	write(gpioout, &gpio, sizeof(gpio));
	close(gpioout);
}

void set_gpio(int pin, int value)
{
	if (pin >= 40) {
		set_hc595(pin - 40, value);
		return;
	}
	set_gpio_base(pin, value);
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
#elif defined(HAVE_XSCALE)
#define u8 unsigned char
#define u32 unsigned long

// #include <linux/ixp425-gpio.h>

#include <asm/hardware.h>
#include <asm-arm/arch-ixp4xx/ixp4xx-regs.h>

#define IXP4XX_GPIO_OUT 0x1
#define IXP4XX_GPIO_IN 0x2

struct gpio_bit {
	unsigned char bit;
	unsigned char state;
};

char *filename = "/dev/gpio";

void set_gpio(int gpio, int value)
{
	int file;
	static struct gpio_bit _bit;

	/*
	 * open device 
	 */
	if ((file = open(filename, O_RDWR)) == -1) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
		return;
	}

	/*
	 * Config bit as output 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_OUT;
	if (ioctl(file, GPIO_SET_CONFIG, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
		close(file);
		return;
	}
	/*
	 * Write data 
	 */
	_bit.bit = gpio;
	_bit.state = value;
	if (ioctl(file, 5, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
	}
	//      fprintf(stderr,"done\n");

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
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
		return 1;
	}

	/*
	 * Config pin as input 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_IN;
	if (ioctl(file, GPIO_SET_CONFIG, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
		close(file);
		return 1;
	}

	/*
	 * Read data 
	 */
	_bit.bit = gpio;
	if (ioctl(file, GPIO_GET_BIT, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
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

#define GPIO_GET_BIT 0x0000001
#define GPIO_SET_BIT 0x0000002
#define GPIO_GET_CONFIG 0x0000003
#define GPIO_SET_CONFIG 0x0000004

#define IXP4XX_GPIO_OUT 0x1
#define IXP4XX_GPIO_IN 0x2

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
		fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
		return;
	}

	/*
	 * Config bit as output 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_OUT;
	if (ioctl(file, GPIO_SET_CONFIG, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
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
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
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
		fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
		return 1;
	}

	/*
	 * Config pin as input 
	 */
	_bit.bit = gpio;
	_bit.state = IXP4XX_GPIO_IN;
	if (ioctl(file, GPIO_SET_CONFIG, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
		close(file);
		return 1;
	}

	/*
	 * Read data 
	 */
	_bit.bit = gpio;
	if (ioctl(file, GPIO_GET_BIT, (unsigned long)&_bit) < 0) {
		/*
		 * ERROR HANDLING; you can check errno to see what went wrong 
		 */
		fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
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

#define GPIO_DEV "/dev/gpio"
enum {
	gpio_in,
	gpio_out,
};
enum {
#if defined(HAVE_MT7620)
	gpio2300,
	gpio3924,
	gpio7140,
	gpio72,
#else
	gpio2300,
#endif
};

#define RALINK_GPIO6332_SET_DIR 0x51
#define RALINK_GPIO6332_SET_DIR_IN 0x13
#define RALINK_GPIO6332_SET_DIR_OUT 0x14
#define RALINK_GPIO6332_READ 0x52
#define RALINK_GPIO6332_WRITE 0x53
#define RALINK_GPIO6332_SET 0x22
#define RALINK_GPIO6332_CLEAR 0x32

#define RALINK_GPIO9564_SET_DIR 0x61
#define RALINK_GPIO9564_SET_DIR_IN 0x15
#define RALINK_GPIO9564_SET_DIR_OUT 0x16
#define RALINK_GPIO9564_READ 0x62
#define RALINK_GPIO9564_WRITE 0x63
#define RALINK_GPIO9564_SET 0x23
#define RALINK_GPIO9564_CLEAR 0x33

#define RALINK_GPIO_SET_DIR 0x01
#define RALINK_GPIO_SET_DIR_IN 0x11
#define RALINK_GPIO_SET_DIR_OUT 0x12
#define RALINK_GPIO_READ 0x02
#define RALINK_GPIO_WRITE 0x03
#define RALINK_GPIO_SET 0x21
#define RALINK_GPIO_CLEAR 0x31
#define RALINK_GPIO_READ_INT 0x02 //same as read
#define RALINK_GPIO_WRITE_INT 0x03 //same as write
#define RALINK_GPIO_SET_INT 0x21 //same as set
#define RALINK_GPIO_CLEAR_INT 0x31 //same as clear
#define RALINK_GPIO_ENABLE_INTP 0x08
#define RALINK_GPIO_DISABLE_INTP 0x09
#define RALINK_GPIO_REG_IRQ 0x0A
#define RALINK_GPIO_LED_SET 0x41

#define RALINK_GPIO7140_SET_DIR_IN 0x15
#define RALINK_GPIO7140_SET_DIR_OUT 0x16
#define RALINK_GPIO7140_READ 0x62
#define RALINK_GPIO7140_WRITE 0x63
#define RALINK_GPIO72_SET_DIR_IN 0x17
#define RALINK_GPIO72_SET_DIR_OUT 0x18
#define RALINK_GPIO72_READ 0x72
#define RALINK_GPIO72_WRITE 0x73
#define RALINK_GPIO3924_SET_DIR_IN 0x13
#define RALINK_GPIO3924_SET_DIR_OUT 0x14
#define RALINK_GPIO3924_READ 0x52
#define RALINK_GPIO3924_WRITE 0x53

int gpio_set_dir_in(int gpio)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	int req;
	int val;
#ifdef HAVE_MT7621
	if (gpio <= 95 && gpio >= 64) {
		req = RALINK_GPIO9564_SET_DIR_IN;
		val = 1 << (gpio - 64);
	} else if (gpio <= 63 && gpio >= 32) {
		req = RALINK_GPIO6332_SET_DIR_IN;
		val = 1 << (gpio - 32);
	} else
#elif defined(HAVE_MT7620)
	if (gpio == 72) {
		req = RALINK_GPIO72_SET_DIR_IN;
		val = 1 << (gpio - 72);
	} else if (gpio <= 71 && gpio >= 40) {
		req = RALINK_GPIO7140_SET_DIR_IN;
		val = 1 << (gpio - 40);
	} else if (gpio <= 39 && gpio >= 24) {
		req = RALINK_GPIO3924_SET_DIR_IN;
		val = 1 << (gpio - 24);
	} else
#endif
	{
		req = RALINK_GPIO_SET_DIR_IN;
		val = 1 << gpio;
	}
	if (ioctl(fd, req, val) < 0) {
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
	int req;
	int val;
#ifdef HAVE_MT7621
	if (gpio <= 95 && gpio >= 64) {
		req = RALINK_GPIO9564_SET_DIR_OUT;
		val = 1 << (gpio - 64);
	} else if (gpio <= 63 && gpio >= 32) {
		req = RALINK_GPIO6332_SET_DIR_OUT;
		val = 1 << (gpio - 32);
	} else
#elif defined(HAVE_MT7620)
	if (gpio == 72) {
		req = RALINK_GPIO72_SET_DIR_OUT;
		val = 1 << (gpio - 72);
	} else if (gpio <= 71 && gpio >= 40) {
		req = RALINK_GPIO7140_SET_DIR_OUT;
		val = 1 << (gpio - 40);
	} else if (gpio <= 39 && gpio >= 24) {
		req = RALINK_GPIO3924_SET_DIR_OUT;
		val = 1 << (gpio - 24);
	} else
#endif
	{
		req = RALINK_GPIO_SET_DIR_OUT;
		val = 1 << gpio;
	}
	if (ioctl(fd, req, val) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_read_bit(int gpio, int *value)
{
	int fd;

	*value = 0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	int req;
	int val;
#ifdef HAVE_MT7621
	if (gpio <= 95 && gpio >= 64) {
		req = RALINK_GPIO9564_READ;
		val = 1 << (gpio - 64);
	} else if (gpio <= 63 && gpio >= 32) {
		req = RALINK_GPIO6332_READ;
		val = 1 << (gpio - 32);
	} else
#elif defined(HAVE_MT7620)
	if (gpio == 72) {
		req = RALINK_GPIO72_READ;
		val = 1 << (gpio - 72);
	} else if (gpio <= 71 && gpio >= 40) {
		req = RALINK_GPIO7140_READ;
		val = 1 << (gpio - 40);
	} else if (gpio <= 39 && gpio >= 24) {
		req = RALINK_GPIO3924_READ;
		val = 1 << (gpio - 24);
	} else
#endif
	{
		req = RALINK_GPIO_READ;
		val = 1 << gpio;
	}

	if (ioctl(fd, req, value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	if (*value & val)
		*value = 1;
	else
		*value = 0;
	return 0;
}

int gpio_write_bit(int gpio, int setvalue)
{
	int fd;
	int value;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	int req;
	int wreq;
	int val;
#ifdef HAVE_MT7621
	if (gpio <= 95 && gpio >= 64) {
		req = RALINK_GPIO9564_READ;
		wreq = RALINK_GPIO9564_WRITE;
		val = 1 << (gpio - 64);
	} else if (gpio <= 63 && gpio >= 32) {
		req = RALINK_GPIO6332_READ;
		wreq = RALINK_GPIO6332_WRITE;
		val = 1 << (gpio - 32);
	} else
#elif defined(HAVE_MT7620)
	if (gpio == 72) {
		req = RALINK_GPIO72_READ;
		wreq = RALINK_GPIO72_WRITE;
		val = 1 << (gpio - 72);

	} else if (gpio <= 71 && gpio >= 40) {
		req = RALINK_GPIO7140_READ;
		wreq = RALINK_GPIO7140_WRITE;
		val = 1 << (gpio - 40);
	} else if (gpio <= 39 && gpio >= 24) {
		req = RALINK_GPIO3924_READ;
		wreq = RALINK_GPIO3924_WRITE;
		val = 1 << (gpio - 24);
	} else
#endif
	{
		req = RALINK_GPIO_READ;
		wreq = RALINK_GPIO_WRITE;
		val = 1 << gpio;
	}

	if (ioctl(fd, req, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	if (setvalue)
		value |= val;
	else
		value &= ~val;

	if (ioctl(fd, wreq, value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

void set_gpio(int pin, int value)
{
	if (pin > 95) {
		set_linux_gpio(pin, value);
	} else {
		gpio_set_dir_out(pin);
		gpio_write_bit(pin, value);
	}
}

int get_gpio(int pin)
{
	int value;
	if (pin > 95)
		get_linux_gpio(pin);

	gpio_set_dir_in(pin);
	gpio_read_bit(pin, &value);
	return value;
}

#elif HAVE_OPENRISC

#define GPIO_CMD_GET_BTN_RST 1
#define GPIO_CMD_SET_BTN_RST 2
#define GPIO_CMD_GET_LEDS 3
#define GPIO_CMD_SET_LEDS 4
#define GPIO_CMD_SET_LED_POWER 5
#define GPIO_CMD_SET_LED_BLUE 6
#define GPIO_CMD_SET_LED_GREEN 7
#define GPIO_CMD_SET 8
#define GPIO_CMD_GET 9
#define GPIO_CMD_SET_CTRL 10
#define GPIO_CMD_GET_CTRL 11
#define GPIO_CMD_SET_IRQMASK 12
#define GPIO_CMD_GET_IRQMASK 13
#define GPIO_CMD_SET_CHANGE 14 //!< obsolete
#define GPIO_CMD_GET_CHANGE 15
#define GPIO_CMD_SET_CHANGES 16 //!< obsolete
#define GPIO_CMD_GET_CHANGES 17
#define GPIO_CMD_SET_BUZZER 18
#define GPIO_CMD_GET_BUZZER 19
#define GPIO_CMD_SET_BUZZER_FRQ 20
#define GPIO_CMD_GET_BUZZER_FRQ 21
#define GPIO_CMD_SET_LED_BTN_WLAN 22
#define GPIO_CMD_GET_BTN_WLAN 23

struct gpio_struct {
	unsigned long mask;
	unsigned long value;
};

#define GPIO_DEV "/dev/misc/gpio"
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
		ioctl(fd, cmd, &value); // silently ignore
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
			close(fd); // silently ignore errors
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

void set_gpio(int pin, int value)
{
	int fd;
	switch (pin) {
	case 16: // main LED
		writeint("/sys/devices/platform/leds-gpio/leds/user1/brightness", value);
		break;
	default:
		set_linux_gpio(pin, value);
		break;
	}
}

int get_gpio(int pin)
{
	return get_linux_gpio(pin);
}

#elif HAVE_VENTANA
void set_gpio(int pin, int value)
{
	switch (pin) {
	case 102:
		writestr("/sys/class/leds/user1/trigger", "none");
		writeint("/sys/class/leds/user1/brightness", value ? 255 : 0);
		break;
	case 103:
		writestr("/sys/class/leds/user2/trigger", "none");
		writeint("/sys/class/leds/user2/brightness", value ? 255 : 0);
		break;
	case 111:
		writestr("/sys/class/leds/user3/trigger", "none");
		writeint("/sys/class/leds/user3/brightness", value ? 255 : 0);
		break;
	default:
		set_linux_gpio(pin, value);
		break;
	}
}

int get_gpio(int pin)
{
	int val = 0;
	switch (pin) {
	case 102:
	case 103:
	case 111:
		break;
	default:
		val = get_linux_gpio(pin);
	}
	return val;
}

#else //e.g. Broadcom...

/*
 * External clk/data based shift register 
 * This should be modified for boards with different control pattern
 * such as active mode (high/low), trigger mode (high/low), etc
 *
 * Netgear WNDR4000 uses 8-bit serial-in/parallel-out shift register 74HC164
 */

/* Direct ctrl mode */
#define WNDR4000_GPIO_USB (1)
#define WNDR4000_GPIO_WIFI (2)
#define WNDR4000_GPIO_RESET (3)
#define WNDR4000_GPIO_WPS (4)
#define WNDR4000_GPIO_ROBO_RESET (5)

/* Clk/Data extended ctrl mode */
#define WNDR4000_GPIO_EXT_CTRL_DATA (6)
#define WNDR4000_GPIO_EXT_CTRL_CLK (7)

/* Extended LED max shift times */
#define WNDR4000_EXT_LED_MAX_SHIFTS (8 - 1) /* 8 extended pins */

/* Extended LED shift defines, not gpio pins, all active low */
#define WNDR4000_GPIO_LED_PWR_GREEN (0)
#define WNDR4000_GPIO_LED_PWR_AMBER (1)
#define WNDR4000_GPIO_LED_WAN (2)
#define WNDR4000_GPIO_LED_WLAN_2G (3)
#define WNDR4000_GPIO_LED_WLAN_5G (4)
#define WNDR4000_GPIO_LED_USB (5)
#define WNDR4000_GPIO_LED_WPS (6)
#define WNDR4000_GPIO_LED_WLAN (7)
void set_gpio_normal(int pin, int value);

void ext_output_value(unsigned int led_status, int clk, int data, int max_shifts)
{
	int i;

	set_gpio_normal(data, 1); /* init off, pull high */
	set_gpio_normal(clk, 0); /* init reset */

	for (i = max_shifts; i >= 0; i--) {
		if (led_status & (1 << i))
			set_gpio_normal(data, 0); /* on, pull low */
		else
			set_gpio_normal(data, 1); /* off, pull high */

		set_gpio_normal(clk, 1); /* pull high to trigger */
		set_gpio_normal(clk, 0); /* reset to low */
	}
}

void gpio_control_clk_data(int pin, int value, int clk, int data, int max_shifts)
{
	int old = 0;
	int ext_led_new;
	FILE *in;

	if (pin < 0 || pin > max_shifts)
		return;

	if (in = fopen("/tmp/.ext_led_value", "rb")) {
		fscanf(in, "%d", &old);
		fclose(in);
	}

	if (value)
		ext_led_new = old | (1 << pin); /* set pin bit */
	else
		ext_led_new = old & (~(1 << pin)); /* clear pin bit */

	if (ext_led_new == old)
		return;

	if (in = fopen("/tmp/.ext_led_value", "wb")) {
		fprintf(in, "%d", ext_led_new);
		fclose(in);
	}

	ext_output_value(ext_led_new, clk, data, max_shifts);
}

void set_gpio_normal(int pin, int value)
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

void set_gpio(int pin, int value)
{
	int brand = getRouterBrand();
	if (brand == ROUTER_NETGEAR_WNDR4000 || brand == ROUTER_NETGEAR_R6200) {
		gpio_control_clk_data(pin, value, WNDR4000_GPIO_EXT_CTRL_CLK, WNDR4000_GPIO_EXT_CTRL_DATA,
				      WNDR4000_EXT_LED_MAX_SHIFTS);
	} else {
		set_gpio_normal(pin, value);
	}
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

#ifdef TEST
int main(int argc, char **argv)
{
	unsigned int gpio;
	unsigned int old_gpio = -1;
	unsigned int pin;

	if (argc != 3) {
		fprintf(stderr, "%s <poll | enable | disable> <pin>\n", argv[0]);
		exit(1);
	}

	pin = atoi(argv[2]);
	if (!strcmp(argv[1], "poll")) {
		while (1) {
			gpio = get_gpio(pin);
			if (gpio != old_gpio)
				fprintf(stdout, "%02X\n", gpio);
			old_gpio = gpio;
		}
	} else if (!strcmp(argv[1], "init")) {
		gpio = get_gpio(pin);
	} else if (!strcmp(argv[1], "enable")) {
		gpio = 1;
		set_gpio(pin, gpio);
	} else if (!strcmp(argv[1], "disable")) {
		gpio = 0;
		set_gpio(pin, gpio);
	}

	return 0;
}
#endif
