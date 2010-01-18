
#ifndef __IT8712_H__
#define __IT8712_H__

#include "asm/arch/sl2312.h"

#define IT8712_IO_BASE			SL2312_LPC_IO_BASE
//#define IT8712_IO_BASE			0x27000000
// Device LDN
#define LDN_SERIAL1				0x01
#define LDN_SERIAL2				0x02
#define LDN_PARALLEL			0x03
#define LDN_KEYBOARD			0x05
#define LDN_MOUSE				0x06
#define LDN_GPIO				0x07

#define IT8712_UART1_PORT      	0x3F8
#define IT8712_UART2_PORT      	0x2F8

#define IT8712_GPIO_BASE		0x800	// 0x800-0x804 for GPIO set1-set5

void LPCSetConfig(char LdnNumber, char Index, char data);
char LPCGetConfig(char LdnNumber, char Index);

#endif
