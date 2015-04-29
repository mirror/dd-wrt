/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  smi connected RTL8366
* Abstract : 
* Author : Yu-Mei Pan (ympan@realtek.com.cn)                
*  $Id: smi.c,v 1.1 2007-10-15 02:37:31 abelshie Exp $
*/

/*
#include "rtl8368s_types.h"
#include "gpio.h"
#include "smi.h"
*/
#include "rtl_types.h"
#include "gpio.h"
#include "smi.h"      /*RTL8651B file*/
#include "asicRegs.h"
#include "rtl8368s_types.h"

#define MDC_MDIO_OPERATION          0   /* Set 1 to use MDC/MDIO access */
                                        /* set 0 to use GPIO emulate I2C access*/
#define MDC_MDIO_DUMMY_ID           0
#define MDC_MDIO_CTRL_REG           13
#define MDC_MDIO_DATA_REG           14
#define MDC_MDIO_PREAMBLE_LEN       32

#define MDC_MDIO_ADDR_OP           0x0000
#define MDC_MDIO_DATA_OP           0x4000

#define DELAY						10000
#define CLK_DURATION(clk)			{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)		{ /*if (!(ok)) return FAILED; */}


gpioID smi_SCK;		/* GPIO used for SMI Clock Generation */
gpioID smi_SDA;		/* GPIO used for SMI Data signal */
gpioID smi_RST;     /* GPIO used for reset swtich */


#define ack_timer					5
#define max_register				0x018A 


void _smi_start(void)
{

	/* change GPIO pin to Output only */
	_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initial state: SCK: 0, SDA: 1 */
	_rtl865x_setGpioDataBit(smi_SCK, 0);
	_rtl865x_setGpioDataBit(smi_SDA, 1);
	CLK_DURATION(DELAY);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	_rtl865x_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);

	/* CLK 2: */
	_rtl865x_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SDA, 0);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SDA, 1);

}



void _smi_writeBit(uint16 signal, uint32 bitLen)
{
	for( ; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			_rtl865x_setGpioDataBit(smi_SDA, 1);	
		else 
			_rtl865x_setGpioDataBit(smi_SDA, 0);	
		CLK_DURATION(DELAY);

		/* clocking */
		_rtl865x_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_rtl865x_setGpioDataBit(smi_SCK, 0);
	}
}



void _smi_readBit(uint32 bitLen, uint32 *rData) 
{
	uint32 u;

	/* change GPIO pin to Input only */
	_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

	for (*rData = 0; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* clocking */
		_rtl865x_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_rtl865x_getGpioDataBit(smi_SDA, &u);
		_rtl865x_setGpioDataBit(smi_SCK, 0);

		*rData |= (u << (bitLen - 1));
	}

	/* change GPIO pin to Output only */
	_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
}



void _smi_stop(void)
{

	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SDA, 0);	
	_rtl865x_setGpioDataBit(smi_SCK, 1);	
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SDA, 1);	
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 1);

    /* add a click */
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rtl865x_setGpioDataBit(smi_SCK, 1);


	/* change GPIO pin to Output only */
	_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);


}


int32 smi_reset(uint32 port, uint32 pinRST)
{
	gpioID gpioId;
	int32 res;

	/* Initialize GPIO port A, pin 7 as SMI RESET */
	gpioId = GPIO_ID(port, pinRST);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
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

	return SUCCESS;
}


int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA)
{
	gpioID gpioId;
	int32 res;

	/* change GPIO pin to Input only */
	/* Initialize GPIO port C, pin 0 as SMI SDA0 */
	gpioId = GPIO_ID(port, pinSDA);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_SDA = gpioId;


	/* Initialize GPIO port C, pin 1 as SMI SCK0 */
	gpioId = GPIO_ID(port, pinSCK);
	res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (res != SUCCESS)
		return res;
	smi_SCK = gpioId;


	_rtl865x_setGpioDataBit(smi_SDA, 1);	
	_rtl865x_setGpioDataBit(smi_SCK, 1);	
	return SUCCESS;
}




int32 smi_read(uint32 mAddrs, uint32 *rData)
{
#if MDC_MDIO_OPERATION

    /* Write address control code to register 13 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL_REG, MDC_MDIO_ADDR_OP);

    /* Write address to register 14 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_REG, mAddrs);

    /* Write data control code to register 13 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL_REG, MDC_MDIO_DATA_OP);

    /* Read data from register 14 */
    MDC_MDIO_READ(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_REG, rData);

    return SUCCESS;
#else
	uint32 rawData=0, ACK;
	uint8  con;
	uint32 ret = SUCCESS;
/*
	if ((mAddrs > max_register) || (rData == NULL))  return	FAILED;
*/

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	rtlglue_drvMutexLock();

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x1, 1);						/* 1: issue READ command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing READ command*/
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */	
	} while ((ACK != 0) && (con < ack_timer));

	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK by RTL8369 */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_readBit(8, &rawData);					/* Read DATA [7:0] */
	*rData = rawData&0xff; 

	_smi_writeBit(0x00, 1);						/* ACK by CPU */

	_smi_readBit(8, &rawData);					/* Read DATA [15: 8] */

	_smi_writeBit(0x01, 1);						/* ACK by CPU */
	*rData |= (rawData<<8);

	_smi_stop();

	rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

	return ret;
#endif /* end of #if MDC_MDIO_OPEARTION */
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{
#if MDC_MDIO_OPERATION

    /* Write address control code to register 13 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL_REG, MDC_MDIO_ADDR_OP);

    /* Write address to register 14 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_REG, mAddrs);

    /* Write data control code to register 13 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL_REG, MDC_MDIO_DATA_OP);

    /* Write data to register 14 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_REG, rData);

    return SUCCESS;
#else
	int8 con;
	uint32 ACK;
	uint32 ret = SUCCESS;	

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	rtlglue_drvMutexLock();

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x0, 1);						/* 0: issue WRITE command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing WRITE command*/
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit(rData&0xff, 8);				/* Write Data [7:0] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for writting data [7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_writeBit(rData>>8, 8);					/* Write Data [15:8] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);						/* ACK for writting data [15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0) ret = FAILED;

	_smi_stop();	

	rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	
	return ret;
#endif /* end of #if MDC_MDIO_OPEARTION */
}







