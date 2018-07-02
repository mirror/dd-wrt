//! @file vsopenrisc.h
//! @brief OpenRISC hardware API
//!
//! GPIO		Mapped		Function\n
//! --------------------------------------------\n
//! 0		PCIINTA		PCI Interrupt\n
//! 1		INTISA		ISA Interrupt\n
//! 2		INTIO		Digital IO Interrupt\n
//! 3		INTSER		Serial Interrupt\n
//! 4\n
//! 5		JIFFYOUT	Jiffy output\n
//! 6		\n
//! 7\n
//! 8\n
//! 9\n
//! 10		GreenLED	Green LED\n
//! 11		Buzzer		Buzzer\n
//! 12		GPIO12		MiniPCI LED1GRNN(out)\n
//! 13		BlueLED		Blue LED\n
//! 14		PowerLED	Power LED\n
//! 15		SWReset		Reset by Software
//! @author VScom (http://www.vscom.de)

#ifndef _VSOPENRISC_H_
#define _VSOPENRISC_H_


#define VS_SYSID_ALEKTO 1
#define VS_SYSID_ALENA 2
#define VS_SYSID_ARETE 3
#define VS_SYSID_BALIOS 11

#define TIOCGEPLD	0x5470
#define TIOCSEPLD	0x5471

//! @name EPLD modes
//@{
#define EPLD_PORTOFF					0x05  //!< turn the drivers off
#define CAP_EPLD_PORTOFF				0x0001  //!< turn the drivers off capability

#define EPLD_RS232					0x03  //!< RS232 mode
#define CAP_EPLD_RS232					0x0002  //!< RS232 mode capability

#define EPLD_RS422					0x01  //!< RS422 mode
#define CAP_EPLD_RS422					0x0004  //!< RS422 mode capability

#define EPLD_RS485_ART_4W				0x08  //!< RS485 4 wire direction control by ART 
#define CAP_EPLD_RS485_ART_4W				0x0008  //!< RS485 4 wire direction control by ART capability 

#define EPLD_RS485_ART_2W				0x0C  //!< RS485 2 wire direction control by ART
#define CAP_EPLD_RS485_ART_2W				0x0010  //!< RS485 2 wire direction control by ART capability

#define EPLD_RS485_ART_ECHO				0x04  //!< RS485 2 wire direction control by ART with echo
#define CAP_EPLD_RS485_ART_ECHO				0x0020  //!< RS485 2 wire direction control by ART with echo capability

#define EPLD_RS485_RTS_4W				0x0A  //!< RS485 4 wire direction control by RTS
#define CAP_EPLD_RS485_RTS_4W				0x0040  //!< RS485 4 wire direction control by RTS capability

#define EPLD_RS485_RTS_2W				0x0E  //!< RS485 2 wire direction control by RTS
#define CAP_EPLD_RS485_RTS_2W				0x0080  //!< RS485 2 wire direction control by RTS capability

#define EPLD_RS485_RTS_ECHO				0x06  //!< RS485 2 wire direction control by RTS with echo
#define CAP_EPLD_RS485_RTS_ECHO				0x0100  //!< RS485 2 wire direction control by RTS with echo capability

#define EPLD_CAN					0x09  //!< CAN mode
#define CAP_EPLD_CAN					0x0200  //!< CAN mode capability

#define CAP_EPLD_RS485					(CAP_EPLD_RS485_ART_4W|CAP_EPLD_RS485_ART_2W|CAP_EPLD_RS485_ART_ECHO|CAP_EPLD_RS485_RTS_4W|CAP_EPLD_RS485_RTS_2W|CAP_EPLD_RS485_RTS_ECHO) 						//!< All supported RS485 modes
#define CAP_EPLD_RS_ALL					(CAP_EPLD_RS232|CAP_EPLD_RS422|CAP_EPLD_RS485)	//!< All supported RS modes
//@}

//! @name GPIO bits macros
//@{
#define GPIO_BIT_0 0x01		//!< bit 0
#define GPIO_BIT_1 0x02		//!< bit 1
#define GPIO_BIT_2 0x04		//!< bit 2
#define GPIO_BIT_3 0x08		//!< bit 3
#define GPIO_BIT_4 0x10		//!< bit 4
#define GPIO_BIT_5 0x20		//!< bit 5
#define GPIO_BIT_6 0x40		//!< bit 6
#define GPIO_BIT_7 0x80		//!< bit 7
//@}

#define DRV_IN_USE_NONE 0x0
#define DRV_IN_USE_TTY  0x1
#define DRV_IN_USE_CAN  0x2

struct epld_struct
{
	unsigned long port;
	unsigned long reg_shift;
	unsigned char value;
};

/* synchronization functions for serial and CAN interfaces */
int check_and_set_dev_open_status(int dev_nr, unsigned char flag);
int check_dev_open_status(int dev_nr, unsigned char flag);
void clear_dev_open_status(int dev_nr, unsigned char flag);

#define LED_POWER				0x01	//!< power LED red
#define LED_BLUE				0x02	//!< WLAN LED blue
#define LED_GREEN				0x04	//!< LED green
#define LED_BTN_WLAN				0x08	//!< LED button wlan (Arete)

#define NUMBER_OF_GPIOS			8		//!< Number of digital IO channels

#define GPIO_VAL_DATA			0
#define GPIO_VAL_CTRL			1
#define GPIO_VAL_IRQMASK		2
#define GPIO_VAL_CHANGE			3
#define GPIO_VAL_CHANGES		4
#define GPIO_VAL_MAX			5

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
#define GPIO_CMD_MAX			GPIO_CMD_GET_BTN_WLAN

struct gpio_struct {
	unsigned long mask;
	unsigned long value;
};

#endif

