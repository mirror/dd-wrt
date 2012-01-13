/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7100_phy.h"
#include "ag7100.h"
#include "rtl8366_smi.h"

#define DEBUG_MSG(arg)
//#define DEBUG_MSG(arg) printk arg


#ifdef CONFIG_TPLINK
#define I2CMASTER_SDA_PIN 18//gpio 19
#define I2CMASTER_SCL_PIN 19//gpio 20
#elif CONFIG_BUFFALO
#define I2CMASTER_SDA_PIN 19//gpio 19
#define I2CMASTER_SCL_PIN 20//gpio 20
#else
//dir 825
#define I2CMASTER_SDA_PIN 5//gpio 19
#define I2CMASTER_SCL_PIN 7//gpio 20
#endif


#define SWITCH_RESET 21//gpio 21

/*
 * set_data_in()
 */
static inline void set_data_in(void)
{
	//DEBUG_MSG(("datain i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
	//DEBUG_MSG(("datainv i=%x,g=%x\n",~(1<<I2CMASTER_SDA_PIN),~(1<<I2CMASTER_SDA_PIN)));
	//*(volatile int *)(0xb8040014) &= ~(1<<I2CMASTER_SDA_PIN);//disable int
	*(volatile int *)(0xb8040000) &= ~(1<<I2CMASTER_SDA_PIN);//change to input
	//DEBUG_MSG(("datain2 i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
}

/*
 * set_data_out()
 */
static inline void set_data_out(void)
{
	//DEBUG_MSG(("dataout i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
	//DEBUG_MSG(("dataoutv i=%x,g=%x\n",~(1<<I2CMASTER_SDA_PIN),(1<<I2CMASTER_SDA_PIN)));
	//*(volatile int *)(0xb8040014) &= ~(1<<I2CMASTER_SDA_PIN);//disable int
	*(volatile int *)(0xb8040000) |= (1<<I2CMASTER_SDA_PIN);//change to output	
	//DEBUG_MSG(("dataout2 i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
}

/*
 * set_clock_in()
 */
static inline void set_clock_in(void)
{
	//DEBUG_MSG(("clockin i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
	//DEBUG_MSG(("clockinv i=%x,g=%x\n",~(1<<I2CMASTER_SCL_PIN),~(1<<I2CMASTER_SCL_PIN)));
	//*(volatile int *)(0xb8040014) &= ~(1<<I2CMASTER_SCL_PIN);//disable int
	*(volatile int *)(0xb8040000) &= ~(1<<I2CMASTER_SCL_PIN);//change to input	
	//DEBUG_MSG(("clockin2 i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));		
}

/*
 * set_clock_out()
 */
static inline void set_clock_out(void)
{
	//DEBUG_MSG(("clockout i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
	//DEBUG_MSG(("clockoutv i=%x,g=%x\n",~(1<<I2CMASTER_SCL_PIN),(1<<I2CMASTER_SCL_PIN)));
	//*(volatile int *)(0xb8040014) &= ~(1<<I2CMASTER_SCL_PIN);//disable int
	*(volatile int *)(0xb8040000) |= (1<<I2CMASTER_SCL_PIN);//change to output	
	//DEBUG_MSG(("clockout2 i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
}

/*
 * set_data_pin()
 */
static inline void set_data_pin(uint32_t v)
{		
	if (v) {//high		
		*(volatile int *)(0xb8040008) |= 1<<I2CMASTER_SDA_PIN;
	} else {//low	
		*(volatile int *)(0xb8040008) &= ~(1<<I2CMASTER_SDA_PIN);	
		
	}
}

/*
 * get_data_pin()
 */
static inline uint32_t get_data_pin(void)
{	
	 //DEBUG_MSG(("v=%x,m=%x\n",(*(volatile unsigned long *)0xb8040004),(1<<I2CMASTER_SDA_PIN)));
	 if((*(volatile unsigned long *)0xb8040004) & (1<<I2CMASTER_SDA_PIN))
	 	return 1;
	 else
	 	return 0;			
}

/*
 * set_clock_pin()
 */
static inline void set_clock_pin(uint32_t v)
{	
	if (v) {//hifh		
		*(volatile int *)(0xb8040008) |= 1<<I2CMASTER_SCL_PIN;
	} else {//low		
		*(volatile int *)(0xb8040008) &= ~(1<<I2CMASTER_SCL_PIN);		
	}
}

#define DELAY 2//us
#define ack_timer					5
#define max_register				0x018A 

static int init_smi = 0;

uint32_t smi_init(void)
{
	if(init_smi == 1)
	{
		return 0;
	}
	init_smi = 1;
	//DEBUG_MSG(("smi i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));	
	/* change GPIO pin to Input only */
	set_data_in();
	set_clock_in();	
	
	//DEBUG_MSG(("smi2 i=%x,g=%x\n",*(volatile int *)(0xb8040014),*(volatile int *)(0xb8040000)));
	udelay(DELAY);		
	return 0;
}

void _smi_start(void)
{
	/* change GPIO pin to Output only */
	set_data_out();
	set_clock_out();	
	udelay(DELAY);
	/* Initial state: SCK:0,SDA:1 */
	set_clock_pin(0);
	set_data_pin(1);
	udelay(DELAY);

	/* SCK:0 -> 1 -> 0 */
	set_clock_pin(1);
	udelay(DELAY);
	set_clock_pin(0);
	udelay(DELAY);

	/* SCK:1,SDA:0->SCK:0,SDA:1 */
	set_clock_pin(1);
	udelay(DELAY);
	set_data_pin(0);
	udelay(DELAY);
	set_clock_pin(0);
	udelay(DELAY);
	set_data_pin(1);	
}

void _smi_stop(void)
{
	/* SCK:1,SDA:0->SCK:1,SDA:1->SCK:0->1 */
	udelay(DELAY);
	set_data_pin(0);	
	set_clock_pin(1);	
	udelay(DELAY);
	set_data_pin(1);	
	udelay(DELAY);
	set_clock_pin(1);
	udelay(DELAY);
	set_clock_pin(0);
	udelay(DELAY);
	set_clock_pin(1);

    /* SCK:0->1 */
	udelay(DELAY);
	set_clock_pin(0);
	udelay(DELAY);
	set_clock_pin(1);

	/* change GPIO pin to Output only */
	set_data_in();
	set_clock_in();

}

#if 0
uint32_t smi_reset(void)
{
	/* Initialize GPIO pin x as SMI RESET */	
	*(volatile int *)(0xb8040000) |= (1<<SWITCH_RESET);//change to output	
		
	*(volatile int *)(0xb8040008) &= ~(1<<SWITCH_RESET);//pin low
	udelay(1000000);
	*(volatile int *)(0xb8040008) |= 1<<I2CMASTER_SCL_PIN;//pin high
	udelay(1000000);
	*(volatile int *)(0xb8040008) &= ~(1<<SWITCH_RESET);//pin low
	
	/* change GPIO pin to Input only */
	*(volatile int *)(0xb8040000) &= ~(1<<SWITCH_RESET);//change to input

	return 0;
}
#endif

void _smi_writeBit(uint16_t signal, uint32_t bitLen)
{
	for( ; bitLen > 0; bitLen--)
	{
		udelay(DELAY);
		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			set_data_pin(1);	
		else 
			set_data_pin(0);	
		udelay(DELAY);

		/* clocking */
		set_clock_pin(1);
		udelay(DELAY);
		set_clock_pin(0);
	}
}

void _smi_readBit(uint32_t bitLen, uint32_t *rData) 
{
	uint32_t u;

	/* change GPIO pin to Input only */
	set_data_in();
			
	for (*rData = 0; bitLen > 0; bitLen--)
	{
		udelay(DELAY);
		/* clocking */
		set_clock_pin(1);
		udelay(DELAY);		
		u = get_data_pin();
		//DEBUG_MSG(("u=%x\n",u));
		set_clock_pin(0);
		*rData |= (u << (bitLen - 1));		
		//DEBUG_MSG(("b=%d,*rData=%x\n",bitLen,*rData));		
	}

	/* change GPIO pin to Output only */
	set_data_out();
}

uint32_t smi_read(uint32_t mAddrs, uint32_t *rData)
{
	uint32_t rawData = 0, ACK;
	uint8_t  con;
	uint32_t ret = 0;
/*
	if ((mAddrs > max_register) || (rData == NULL))  return	1;
*/
	//DEBUG_MSG(("smi read mAddrs=0x%x,rData=0x%x\n", mAddrs ,*rData));
	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	//rtlglue_drvMutexLock();

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x1, 1);						/* 1: issue READ command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing READ command*/
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		//DEBUG_MSG(("CTRL read fail\n"));
		ret = 1;
	}
	//DEBUG_MSG(("LSB addrs=0x%x\n",(mAddrs&0xff)));
	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */	
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		 //DEBUG_MSG(("LSB addrs fail\n"));
		 ret = 1;
	}	 
	//DEBUG_MSG(("MSB addrs=0x%x\n",(mAddrs>>8)));
	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK by RTL8369 */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		 //DEBUG_MSG(("MSB addrs fail\n"));
		 ret = 1;
	}	 

	_smi_readBit(8, &rawData);					/* Read DATA [7:0] */
	//DEBUG_MSG(("rLSB data=0x%x\n",rawData));
	*rData = rawData&0xff; 
	//DEBUG_MSG(("*rData=0x%x\n",*rData));
	_smi_writeBit(0x00, 1);						/* ACK by CPU */

	_smi_readBit(8, &rawData);					/* Read DATA [15: 8] */
	//DEBUG_MSG(("rMSB data=0x%x\n",rawData));
	_smi_writeBit(0x01, 1);						/* ACK by CPU */
	*rData |= (rawData<<8);
	//DEBUG_MSG(("*rData=0x%x\n",*rData));

	_smi_stop();

	//rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	//DEBUG_MSG(("*rData=0x%x\n",*rData));
	return ret;
}

uint32_t smi_write(uint32_t mAddrs, uint32_t rData)
{

	uint8_t con;
	uint32_t ACK;
	uint32_t ret = 0;	
	
/*
	if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return	1;
*/
	//DEBUG_MSG(("smi write mAddrs=0x%x,rData=0x%x\n", mAddrs ,rData));
	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	//rtlglue_drvMutexLock();

	_smi_start();								/* Start SMI */

	_smi_writeBit(0x0a, 4); 					/* CTRL code: 4'b1010 */

	_smi_writeBit(0x4, 3);						/* CTRL code: 3'b100 */

	_smi_writeBit(0x0, 1);						/* 0: issue WRITE command */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for issuing WRITE command*/
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){ 
		//DEBUG_MSG(("CTRL write fail\n"));
		ret = 1;
	}	
	//DEBUG_MSG(("LSB addrs=0x%x\n",(mAddrs&0xff)));
	_smi_writeBit((mAddrs&0xff), 8); 			/* Set reg_addr[7:0] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		 //DEBUG_MSG(("LSB addrs fail\n"));
		 ret = 1;
	}	 
	//DEBUG_MSG(("MSB addrs=0x%x\n",(mAddrs>>8)));
	_smi_writeBit((mAddrs>>8), 8); 				/* Set reg_addr[15:8] */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for setting reg_addr[15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		 //DEBUG_MSG(("MSB addrs fail\n"));
		 ret = 1;
	}
	//DEBUG_MSG(("LSB data=0x%x\n",(rData&0xff)));
	_smi_writeBit((rData&0xff), 8);				/* Write Data [7:0] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);					/* ACK for writting data [7:0] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		 //DEBUG_MSG(("LSB data fail\n"));
		 ret = 1;
	}	 
	//DEBUG_MSG(("MSB data=0x%x\n",(rData>>8)));
	_smi_writeBit((rData>>8), 8);					/* Write Data [15:8] out */

	con = 0;
	do {
		con++;
		_smi_readBit(1, &ACK);						/* ACK for writting data [15:8] */
	} while ((ACK != 0) && (con < ack_timer));
	if (ACK != 0){
		//DEBUG_MSG(("MSB data fail\n"));
		ret = 1;
	}	

	_smi_stop();	

	//rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
	
	return ret;
}

/****************************************************************************/
/****************************************************************************/
uint32_t switch_reg_read(uint32_t reg,uint32_t *value)
{    
    uint32_t regData=0;
	uint32_t retVal;
	if(*value)
		*value=0;
	//DEBUG_MSG(("sw read reg=0x%x,data=0x%x\n", reg ,*value));
	retVal = smi_read(reg, &regData);
	if (retVal != 0){
		DEBUG_MSG(("smi_read fail\n"));
		return 1;
	}	
	*value = regData;    
	//DEBUG_MSG(("sw read reg=0x%x,data=0x%x\n", reg ,*value));
    return 0;
}

void switch_reg_write(uint32_t reg, uint32_t data)
{
    uint32_t retVal;
	//DEBUG_MSG(("sw write reg=0x%x,data=0x%x\n", reg ,data));
	retVal = smi_write(reg, data);
	if (retVal != 0) 
		DEBUG_MSG(("smi_write fail\n"));		

}

int rtl_chip_type_select(void)
{
	uint32 data;
	smi_init();
	smi_read(0x5C, &data);	// dummy
	if(smi_read(0x5C, &data) != SUCCESS)
		return CHIP_TYPE_UNKNOWN;

	printk(KERN_EMERG "realtek phy id %d\n",data);
	switch(data)
	{
		case 0x6027:
			printk(KERN_EMERG "rtl_chip_type_select:RTL8366SR\n");		
			return CHIP_TYPE_RTL8366SR;
		case 0x5937:
			printk(KERN_EMERG "rtl_chip_type_select:RTL8366RB\n");		
			return CHIP_TYPE_RTL8366RB;
	}
	printk(KERN_EMERG "rtl_chip_type_select:Unknown %X\n",data);		
	return CHIP_TYPE_UNKNOWN;
}
