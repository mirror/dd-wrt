/* $Id: oled.c 19454 2008-05-06 06:48:10Z saulius $ */

#include "oled.h"
#include "string.h"
#include "fonts.h"
#include "gpio.h"
#include "font_microtech_10.h"
//#include "font_ocr_14.h"


void oled_delay(unsigned long int t)
{
	unsigned long int j;

	for(j = 0; j < t; j++)
	{
	}
}

void oled_port_prepare(void)
{
//	write_bit(SCLK, 1);			//set(DATA_DIR, SCLK);
//	write_bit(SDIN, 1);			//set(DATA_DIR, SDIN);
//	write_bit(RES_n, 1);		//set(CTRL_DIR, RES_n);
//	write_bit(SDIN, 1);			//set(CTRL_DIR, CS_n);
//	write_bit(CS_n, 1);			//set(CTRL_DIR, DC_n);
//		file = gpio_init();
		gpio_init();
		
		
}

void oled_reset(void)
{
	// repeat 2 times
	write_bit(RES_n, 0);		//clr(CTRL, RES_n); //RES_n = 0;
	oled_delay(DELAY_CAL2);
	write_bit(RES_n, 1);		//set(CTRL, RES_n); //RES_n = 1;
	oled_delay(DELAY_CAL2);
	write_bit(RES_n, 0);		//clr(CTRL, RES_n); //RES_n = 0;
	oled_delay(DELAY_CAL2);
	write_bit(RES_n, 1);		//set(CTRL, RES_n); //RES_n = 1;
	oled_delay(DELAY_CAL2);
}

void oled_write_cmd(unsigned char command)
{
	write_bit(CS_n, 0);			//clr(CTRL, CS_n); //CS_n =0;
	oled_delay(DELAY_CAL);	
	write_bit(DC_n, 0);			//clr(CTRL, DC_n); //DC_n =0;
	oled_delay(DELAY_CAL);	

	unsigned char i;
	for(i = 0; i<8; i++)
	{
		unsigned char tmp;
		write_bit(SCLK, 0);		//clr(DATA, SCLK); // low
		oled_delay(DELAY_CAL);	
		
		tmp = check(command, (7-i));
		if(tmp != 0)
		{
			write_bit(SDIN, 1);		//set(DATA, SDIN); // data
			oled_delay(DELAY_CAL);	
			write_bit(SCLK, 1);		//set(DATA, SCLK); // hi
			oled_delay(DELAY_CAL);	
		}
				
		if(tmp == 0)
		{
			write_bit(SDIN, 0);		//clr(DATA, SDIN); // data
			oled_delay(DELAY_CAL);	
			write_bit(SCLK, 1);		//set(DATA, SCLK); // hi
			oled_delay(DELAY_CAL);	
		}
	}
	oled_delay(DELAY_CAL);	
	write_bit(SCLK, 0);				//set(CTRL, CS_n); //CS_n=1;
}

void oled_write_data(unsigned char data)
{
	write_bit(CS_n, 0);				//clr(CTRL, CS_n); //CS_n =0;
	oled_delay(DELAY_CAL);	
	write_bit(DC_n, 1);				//set(CTRL, DC_n); //DC_n =0;
	oled_delay(DELAY_CAL);	

	unsigned char i;
	for(i = 0; i<8; i++)
	{
		unsigned char tmp;
		write_bit(SCLK, 0);			//clr(DATA, SCLK); // low
		oled_delay(DELAY_CAL);	
		
		tmp = check(data, (7-i));
		if(tmp != 0)
		{
			//set(CTRL, 1);
			write_bit(SDIN, 1);		//set(DATA, SDIN); //data
			oled_delay(DELAY_CAL);	
			write_bit(SCLK, 1);		//set(DATA, SCLK); // hi
			oled_delay(DELAY_CAL);	
		}
				
		if(tmp == 0)
		{
			//clr(CTRL, 1);
			write_bit(SDIN, 0);		//clr(DATA, SDIN); // data
			oled_delay(DELAY_CAL);	
			write_bit(SCLK, 1);		//set(DATA, SCLK); // hi
			oled_delay(DELAY_CAL);	
		}
	}
	oled_delay(DELAY_CAL);	
	write_bit(CS_n, 1);				//set(CTRL, CS_n); //CS_n=1;
}



void oled_init(void)
{
	oled_write_cmd(0x81); // contrast
	oled_write_cmd(0x00); // 0..0xFF
	oled_write_cmd(0xAF); // OLED on
	
	write_bit(1, 1);
}

void oled_flip_horizontal(unsigned char f)
{
	if(f==1)
  	oled_write_cmd(0xC8); // reversed scan 
  else
  	oled_write_cmd(0xC0); // reversed scan 
}

void oled_invert(unsigned char i)
{
	if(i==1)
  	oled_write_cmd(0xA7); 
  else
  	oled_write_cmd(0xA6); 
}


void oled_flip_vertical(unsigned char f)
{
	if(f==1)
		oled_write_cmd(0xA1); // reversed scan
	else
		oled_write_cmd(0xA0); // reversed scan
}

void oled_print(unsigned char line, unsigned char pos, unsigned char invert, char *txt)
{
	int pos2 = pos*6 + 2;
	int l;
	unsigned char p;

	oled_write_cmd(0x00 + (pos2 & 0x0F));
	oled_write_cmd(0x10 + ((pos2 & 0xF0) >> 4));
	
	oled_write_cmd(0xB0 + line);
	
	for(l = 0; l<strlen(txt); l++)
	{
		for(p = 0; p<6; p++)
		{
			if(invert==1)
				oled_write_data(~font_normal[((unsigned char)txt[l])*6+p]);		
			else if(invert==2)
					oled_write_data(font_bold[((unsigned char)txt[l])*6+p]);
			else
				oled_write_data(font_normal[((unsigned char)txt[l])*6+p]);
		}
	}
}

void oled_print2(unsigned char x, unsigned char y, char *text)
{
	int l;
  int w, h;
  int i = 0;
  int pos = 0;

	for(l = 0; l<strlen(text); l++)
  {
		i = 0;
		for(w = 0; w<MicroTech10FontInfo[(unsigned char)text[l]].Width; w++)
		for(h = 0; h<=MicroTech10FontInfo[(unsigned char)text[l]].Height/8; h++)
		{
			oled_write_cmd(0x00 + ((w + pos + x + 2) & 0x0F));
			oled_write_cmd(0x10 + (((w + pos + x + 2) & 0xF0) >> 4));
			oled_write_cmd(0xB0 + h + y);

			//paint_byte(img, w + pos + y, h+x, MicroTech10FontInfo[text[l]].Ptr[i++]);
//			oled_write_data(OCR14FontInfo[(unsigned char)text[l]].Ptr[i++]);
			oled_write_data(MicroTech10FontInfo[(unsigned char)text[l]].Ptr[i++]);
		}
		pos += w;
	}

}




void oled_brightness(unsigned char br)
{
	oled_write_cmd(0x81); // contrast
	oled_write_cmd(br); // 0..0xFF
}

void oled_clear(void)
{
	int r;
	int i;
	for(r=0; r<8; r++)
	{
		oled_write_cmd(0x10);
		oled_write_cmd(0x02);
		oled_write_cmd(0xB0+r);
		for (i=0;i<128;i++)
		{
			oled_write_data(0x00);
		}
	}
}

void oled_clear_line(unsigned char r)
{
		int i;
		oled_write_cmd(0x10);
		oled_write_cmd(0x02);
		oled_write_cmd(0xB0+r);
		for (i=0;i<128;i++)
		{
			oled_write_data(0x00);
		}
}

void oled_close(void)
{
	gpio_close();
}
