/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  smi connected RTL8366
* Abstract : 
* Author : Yu-Mei Pan (ympan@realtek.com.cn)                
*  $Id: smi.c,v 1.5 2007/05/23 11:30:59 abelshie Exp $
*/


#include <linux/types.h>
//#include <linux/spinlock_types.h>
//#include <linux/interrupt.h>
#include <asm/addrspace.h>
#include <asm/types.h>

#include "ar7100_soc.h"
#include "rtl8368s_types.h"
#include "smi.h"

#define DELAY						10000
#define CLK_DURATION(clk)			{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)		{ /*if (!(ok)) return FAILED; */}


#define smi_SCK		19		/* GPIO used for SMI Clock Generation */
#define smi_SDA		18		/* GPIO used for SMI Data signal */
#define	GPIO_DIR_OUT		0
#define GPIO_DIR_IN			1
#define GPIO_INT_DISABLE	0
#define GPIO_INT_ENABLE		1



#define ack_timer					5
#define max_register				0x018A 

/* initialize spinlock statically because smi_init is not called modified by tiger */
//spinlock_t spinlock = SPIN_LOCK_UNLOCKED;

void ar9100_configGpioPin(int gpio, int dir, int intr)
{
	if (dir == GPIO_DIR_OUT)
		ar7100_reg_rmw_set(AR7100_GPIO_OE, (1 << gpio))
	else
		ar7100_reg_rmw_clear(AR7100_GPIO_OE, (1 << gpio));

	if (intr == GPIO_INT_ENABLE)
		ar7100_reg_rmw_set(AR7100_GPIO_INT_MASK, (1 << gpio))
	else
		ar7100_reg_rmw_clear(AR7100_GPIO_INT_MASK, (1 << gpio));
}

void
ar9100_gpio_out_val(int gpio, int val)
{
    if (val & 0x1) {
        ar7100_reg_rmw_set(AR7100_GPIO_OUT, (1 << gpio));
    }
    else {
        ar7100_reg_rmw_clear(AR7100_GPIO_OUT, (1 << gpio));
    }
}

void
ar9100_gpio_in_val(int gpio, uint32_t * u)
{
    *u = 0x1 & (ar7100_reg_rd(AR7100_GPIO_IN) >> gpio);
}


void _smi_start(void)
{

	/* change GPIO pin to Output only */
	ar9100_configGpioPin(smi_SDA, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	ar9100_configGpioPin(smi_SCK, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initial state: SCK: 0, SDA: 1 */
	ar9100_gpio_out_val(smi_SCK, 0);
	ar9100_gpio_out_val(smi_SDA, 1);
	CLK_DURATION(DELAY);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	ar9100_gpio_out_val(smi_SCK, 1);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 0);
	CLK_DURATION(DELAY);

	/* CLK 2: */
	ar9100_gpio_out_val(smi_SCK, 1);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SDA, 0);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 0);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SDA, 1);

}



void _smi_writeBit(uint16_t signal, uint32_t bitLen)
{
	for( ; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			ar9100_gpio_out_val(smi_SDA, 1);	
		else 
			ar9100_gpio_out_val(smi_SDA, 0);	
		CLK_DURATION(DELAY);

		/* clocking */
		ar9100_gpio_out_val(smi_SCK, 1);
		CLK_DURATION(DELAY);
		ar9100_gpio_out_val(smi_SCK, 0);
	}
}



void _smi_readBit(uint32_t bitLen, uint32_t *rData) 
{
	uint32_t u;

	/* change GPIO pin to Input only */
	ar9100_configGpioPin(smi_SDA, GPIO_DIR_IN, GPIO_INT_DISABLE);

	for (*rData = 0; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* clocking */
		ar9100_gpio_out_val(smi_SCK, 1);
		CLK_DURATION(DELAY);
		ar9100_gpio_in_val(smi_SDA, &u);
		ar9100_gpio_out_val(smi_SCK, 0);
	
		*rData |= (u << (bitLen - 1));
		//printk("loop: %x %x %x\n", bitLen, u, *rData);
	}

	/* change GPIO pin to Output only */
	ar9100_configGpioPin(smi_SDA, GPIO_DIR_OUT, GPIO_INT_DISABLE);
}



void _smi_stop(void)
{

	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SDA, 0);	
	ar9100_gpio_out_val(smi_SCK, 1);	
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SDA, 1);	
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 1);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 0);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 1);

    /* add a click */
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 0);
	CLK_DURATION(DELAY);
	ar9100_gpio_out_val(smi_SCK, 1);


	/* change GPIO pin to Output only */
	ar9100_configGpioPin(smi_SDA, GPIO_DIR_IN, GPIO_INT_DISABLE);
	ar9100_configGpioPin(smi_SCK, GPIO_DIR_IN, GPIO_INT_DISABLE);


}

#if 0
int smi_reset(uint32_t port, uint32_t pinRST)
{
	gpioID gpioId;
	int res;

	/* Initialize GPIO port A, pin 7 as SMI RESET */
	gpioId = GPIO_ID(port, pinRST);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != 0)
		return res;
	smi_RST = gpioId;

	_rtl865x_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(1000000);
	_rtl865x_setGpioDataBit(smi_RST, 0);	
	CLK_DURATION(1000000);
	_rtl865x_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(1000000);

	/* change GPIO pin to Input only */
	_rtl865x_initGpioPin(smi_RST, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

	return 0;
}
#endif

void smi_init()
{
	/* change GPIO pin to Input only */
	/* Initialize GPIO port C, pin 0 as SMI SDA0 */
	ar9100_configGpioPin(smi_SDA, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initialize GPIO port C, pin 1 as SMI SCK0 */
	ar9100_configGpioPin(smi_SCK, GPIO_DIR_OUT, GPIO_INT_DISABLE);

	ar9100_gpio_out_val(smi_SDA, 1);	
	ar9100_gpio_out_val(smi_SCK, 1);	

	//spin_lock_init(&spinlock);
}




int32 smi_read(uint32 mAddrs, uint32 *rData)
{
	uint32_t rawData=0, ACK;
	uint8_t  con;
	uint32_t ret = 0;
/*
	if ((mAddrs > max_register) || (rData == NULL))  return	1;
*/

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	//rtlglue_drvMutexLock();
   	int flags;

    //spin_lock_irqsave(&spinlock, flags);

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x1, 1);						/* 1: issue READ command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing READ command*/
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = 1;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */	
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = 1;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK by RTL8369 */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_readBit(8, &rawData);					/* Read DATA [7:0] */
	//printk("raw1:%x\n", rawData);
	*rData = rawData & 0xff; 

	_smi_writeBit(0x00, 1);						/* ACK by CPU */

	_smi_readBit(8, &rawData);					/* Read DATA [15: 8] */
	//printk("raw2:%x\n", rawData);
	_smi_writeBit(0x01, 1);						/* ACK by CPU */
	*rData |= ((rawData & 0xff)<<8);

	_smi_stop();

	//rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	//spin_unlock_irqrestore(&spinlock, flags);
	
	return ret;
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{
/*
	if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return	1;
*/
	char con;
	uint32_t ACK;
	uint32_t ret = 0;	

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	//rtlglue_drvMutexLock();
	int flags;

    //spin_lock_irqsave(&spinlock, flags);
	
	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x0, 1);						/* 0: issue WRITE command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing WRITE command*/
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_writeBit(rData&0xff, 8);				/* Write Data [7:0] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for writting data [7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_writeBit(rData>>8, 8);					/* Write Data [15:8] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);						/* ACK for writting data [15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = 1;

	_smi_stop();	

	//rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	//spin_unlock_irqrestore(&spinlock, flags);
	
	return ret;
}







