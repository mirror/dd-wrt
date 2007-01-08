/*
 * gpio.c - demonstration of how to use ixp425-gpio driver
 *
 * Author: Tim Harvey (tim_harvey at yahoo dot com)
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define u8 unsigned char
#define u32 unsigned long 

//#include <linux/ixp425-gpio.h>

#include <include/asm-arm/hardware.h>
#include <include/asm-arm/arch-ixp4xx/ixp4xx-regs.h>

#define IXP4XX_GPIO_OUT 		0x1
#define IXP4XX_GPIO_IN  		0x2

struct gpio_bit {
  unsigned char bit;
  unsigned char state;
};



char *filename = "/dev/gpio"; 

void read_bit(int bit);
void write_bit(int bit, int val);

int main (int argc, char** argv) {
  int bit = -1;
  int val = -1;

  if (argc > 1) {
    int i;
    for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-') {
         switch (argv[i][1]) {
           case 'r':  // read bit
             sscanf(argv[i+1], "%d", &bit); 
             read_bit(bit);
             break;
           case 'w':  // write bit
             sscanf(argv[i+1], "%d", &bit); 
             sscanf(argv[i+2], "%d", &val); 
             write_bit(bit, val);
             break;
           default:
             fprintf(stderr,"Unknown argument %s", argv[i]);
             break;
         }
      } else {
      }
    }
  }
  exit(0);

}

void read_bit(int bit) {
  int file;
  struct gpio_bit _bit;

  /* open device */
  if ( (file = open(filename, O_RDONLY)) == -1) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
    exit(1);
  }

  /* Config pin as input */
  _bit.bit = bit;
  _bit.state = IXP4XX_GPIO_IN;
  if ( ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }

  /* Read data */
  _bit.bit = bit;
  if ( ioctl(file, GPIO_GET_BIT, (long)&_bit) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }
    
  fprintf(stderr,"Bit %d=%d\n", _bit.bit, _bit.state);

  close(file);
}


void write_bit(int bit, int val) {
  int file;
  struct gpio_bit _bit;

  /* open device */
  if ( (file = open(filename, O_RDWR)) == -1) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: could not open %s (%d)\n", filename, errno);
    exit(1);
  }

  /* Config bit as output */
  _bit.bit = bit;
  _bit.state = IXP4XX_GPIO_OUT;
  if ( ioctl(file, GPIO_SET_CONFIG, (long)&_bit) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }

  /* Write data */
  _bit.bit = bit;
  _bit.state = val;
  if ( ioctl(file, GPIO_SET_BIT, (unsigned long)&_bit) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    fprintf(stderr, "Error: ioctl failed: %s (%d)\n", strerror(errno), errno);
    exit(1);
  }
    
  close(file);
}
