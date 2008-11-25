/* $Id: oled.h 19454 2008-05-06 06:48:10Z saulius $ */

#ifndef OLED_H
#define OLED_H

/*
#define RES_n 	7		//RES
#define CS_n    6		//CS
#define DC_n 		8		//A0
#define SCLK		12	//D0
#define SDIN		13	//D1
*/

#define RES_n 	12	//RES
#define CS_n    13	//CS
#define DC_n 		8		//A0
#define SCLK		7		//D0
#define SDIN		6		//D1

#define DELAY_CAL 0
#define DELAY_CAL2 1000


#define set(y,x) (y |= (1 << x))
#define clr(y,x) (y &=~(1 << x))
#define check(y,x) (y &(1 << x))

void oled_port_prepare(void);
void oled_reset(void);
void oled_init(void);
void oled_clear(void);
void oled_write_cmd(unsigned char command);
void oled_write_data(unsigned char data);
void oled_print(unsigned char line, unsigned char pos, unsigned char invert, char *txt);
void oled_print2(unsigned char x, unsigned char y, char *text);
void oled_brightness(unsigned char br);
void oled_flip_horizontal(unsigned char f);
void oled_flip_vertical(unsigned char f);
void oled_invert(unsigned char i);
void oled_clear_line(unsigned char r);
void oled_close(void);

#endif
