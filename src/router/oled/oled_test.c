/* $Id: oled_test.c 19454 2008-05-06 06:48:10Z saulius $ */

#include "oled.h"
#include "wilibox.h"
#include "gpio.h"

#define	LED0_RED 3
#define	LED0_GRN 2

#define	LED1_RED 1
#define	LED1_GRN 5

#define	LED2_GRN 17
	
#define BUTTON	14


// ------------------------------------------------------------
int main(void)
{ 
	oled_port_prepare();
	oled_reset();
	oled_init();
	oled_clear();
	oled_flip_horizontal(0);
	oled_flip_vertical(0);
	oled_brightness(0xFF);

/*
	oled_print(0,0,0,"1234567890");
	oled_print(1,0,0,"ABCDEFGHIJKLMN");
	oled_print(2,0,0,"OPQRSTUVWXYZ");

	oled_print(3,0,0,"abcdefghijklmn");
	oled_print(4,0,0,"opqrstuvwxyz");

	oled_print(6,0,0,"^`_{|}:'<=>?@");
*/

/*
	for(r=0; r<7; r++) // KAZKODEL SUSIGADINA STATIC VAIZDAS!!!
	{
		oled_write_cmd(0x10);
		oled_write_cmd(0x00);
		oled_write_cmd(0xB0+r);
		for (i=0;i<128;i++)
		{
			oled_write_data(picwilibox[pos++]);
		}
	}
*/


	oled_print2(0, 0, "Wilibox");
	oled_print2(0, 2, "-------");
	oled_print2(0, 4, "1234567890");
	oled_print(7,0,0,"123456789012345678901");


	read_bit(BUTTON);
	write_bit(LED2_GRN, 0);
	
	oled_close();
	return  0;
}
