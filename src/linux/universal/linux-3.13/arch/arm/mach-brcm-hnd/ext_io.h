#include <asm/ioctl.h>

#define EXTIO_IOC_MAGIC 'x'

#define EXTIO_IOCRESET _IO(EXTIO_IOC_MAGIC, 0)
#define EXTIO_IOCSFUNCTION _IOW(EXTIO_IOC_MAGIC, 1, int)
#define EXTIO_IOCGOUTLEN _IOR(EXTIO_IOC_MAGIC, 2, int)
#define EXTIO_IOC_MAXNR 2

#define EXTERNAL_IF_BASE	0x1A000000
#define ASYNC_IF_BASE		0x1000000

/*
	0~2 bit -> WAN LED
	3~5 bit -> internet LED
*/
#define WAN_LED			0xc7
#define INTERNET_LED		0xf8

/*
	color of LED
*/
#define ILED_SOLID_GREEN	0x03
#define ILED_OFF		0x07
#define ILED_SOLID_AMBER	0x04
#define WLED_FLASHING_GREEN	0x30

#define GOT_IP                  0x01
#define RELEASE_IP              0x02
#define GET_IP_ERROR            0x03
#define RELEASE_WAN_CONTROL     0x04
#define USB_DATA_ACCESS		0x05	//For WRTSL54GS
#define USB_CONNECT		0x06	//For WRTSL54GS
#define USB_DISCONNECT		0x07	//For WRTSL54GS

//////////////////////////////////////////
//					//
//	Model Name: WRTSL54GS		//
//					//
//////////////////////////////////////////
/*
	0~1 bit -> USB Port1 LED
	2~3 bit -> USB Port2 LED
*/
#define USB_PORT1_LED		0xfc
#define USB_PORT2_LED		0xf3

#define USB_LED1_OFF		0x03
#define USB_LED1_BLINKING	0x02
#define USB_LED1_ON		0x00
#define USB_LED2_OFF		0x0c
#define USB_LED2_BLINKING	0x08
#define USB_LED2_ON		0x00
extern int iswrt350n;
extern int gpio_kernel_api(unsigned int cmd, unsigned int mask, unsigned int val);

#define USB_SET_LED(cmd) \
{ \
    if (iswrt350n) \
	switch(cmd) \
	{ \
		case USB_DATA_ACCESS: \
			gpio_kernel_api(0, 0x00000400, 0); \
			gpio_kernel_api(0, 0x00000800, 0x00000800); \
			break; \
		case USB_CONNECT: \
			gpio_kernel_api(0, 0x400, 0); \
			gpio_kernel_api(0, 0x800, 0); \
			break; \
		case USB_DISCONNECT: \
			gpio_kernel_api(0, 0x400, 0x400); \
			gpio_kernel_api(0, 0x800, 0x800); \
			break; \
	} \
}
/*
	unsigned int value; \
	switch(cmd) \
	{ \
		case USB_DATA_ACCESS: \
			value = (value & USB_PORT1_LED) | USB_LED1_BLINKING; \
			*(volatile u8*)(KSEG1ADDR(EXTERNAL_IF_BASE)+ASYNC_IF_BASE)=value; \
			break; \
		case USB_CONNECT: \
			value = (value & USB_PORT1_LED) | USB_LED1_ON; \
			*(volatile u8*)(KSEG1ADDR(EXTERNAL_IF_BASE)+ASYNC_IF_BASE)=value; \
			break; \
		case USB_DISCONNECT: \
			value = (value & USB_PORT1_LED) | USB_LED1_OFF; \
			*(volatile u8*)(KSEG1ADDR(EXTERNAL_IF_BASE)+ASYNC_IF_BASE)=value; \
			break; \
	} \
*/
//}

