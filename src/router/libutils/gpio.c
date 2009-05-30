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

#if  defined(HAVE_AR531X) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_ADM5120)

void set_gpio(int gpio, int value)
{
	FILE *in;
	char buf[64];

	sprintf(buf, "/proc/gpio/%d_dir", gpio);
	in = fopen(buf, "wb");
	if (in == NULL)
		return;
	fprintf(in, "1");
	fclose(in);
	sprintf(buf, "/proc/gpio/%d_out", gpio);
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
		return;
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
		return;
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
