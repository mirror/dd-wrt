/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  smi connected RTL8366
* Abstract : 
* Author : Yu-Mei Pan (ympan@realtek.com.cn)                
*  $Id: smi.c,v 1.5 2007/05/23 11:30:59 abelshie Exp $
*/

#include <asm/uaccess.h>
#include "rtl8366s_types.h"
#include "gpio.h"
#include "smi.h"

#define DELAY						10000
#define CLK_DURATION(clk)			{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)		{ /*if (!(ok)) return FAILED; */}

#define smi_SDA 0x0002		/* GPIO used for SMI Data signal */
#define smi_SCK 0x0004		/* GPIO used for SMI Clock Generation */
//gpioID smi_RST;     /* GPIO used for reset swtich */


#define ack_timer					5
#define max_register				0x018A 

int _rt2880_initGpioPin(uint32 port,int dir_value)
{
		uint32 value,tmp;
		
		tmp = le32_to_cpu(*(volatile u32 *)(RT2880_REG_PIODIR));

		if(dir_value==RT2880GPIO_DIR_OUT)
			value=tmp | port;
		else
			value=tmp & (0xffffffff^port);

		
		*(volatile u32 *)(RT2880_REG_PIODIR) = cpu_to_le32(value);
		return SUCCESS;
}

int _rt2880_setGpioDataBit(uint32 port,uint32 set_value)
{	
		uint32 value,tmp;
		
		tmp = le32_to_cpu(*(volatile u32 *)(RT2880_REG_PIODATA));
		
		if(set_value==1)
			value=tmp | port;
		else
			value=tmp & (0xffffffff^port);
		
		*(volatile u32 *)(RT2880_REG_PIODATA) = cpu_to_le32(value);	
		return SUCCESS;	
}

int _rt2880_getGpioDataBit(uint32 port,uint32 *get_value)
{
		uint32 tmp;
		
		*get_value=0;
		tmp = le32_to_cpu(*(volatile u32 *)(RT2880_REG_PIODATA));

		*get_value=tmp & port;
		if(*get_value>0)
			*get_value=1;
		else
			*get_value=0;

		return SUCCESS;	
}

void _rt2880_drvMutexLock()
{
	*(volatile u32 *)(RT2880_REG_INTDIS) = cpu_to_le32(0x8);
	return;	
};
void _rt2880_drvMutexUnlock()
{
	*(volatile u32 *)(RT2880_REG_INTENA) = cpu_to_le32(0x8);
	return;
}

void _smi_start(void)
{
	/* change GPIO pin to Output only */
	_rt2880_initGpioPin(smi_SDA, RT2880GPIO_DIR_OUT);
	_rt2880_initGpioPin(smi_SCK, RT2880GPIO_DIR_OUT);
	
	/* Initial state: SCK: 0, SDA: 1 */
	_rt2880_setGpioDataBit(smi_SCK, 0);
	_rt2880_setGpioDataBit(smi_SDA, 1);
	CLK_DURATION(DELAY);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	_rt2880_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);

	/* CLK 2: */
	_rt2880_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SDA, 0);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SDA, 1);
}



void _smi_writeBit(uint16 signal, uint32 bitLen)
{
	for( ; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			_rt2880_setGpioDataBit(smi_SDA, 1);	
		else 
			_rt2880_setGpioDataBit(smi_SDA, 0);	
		CLK_DURATION(DELAY);

		/* clocking */
		_rt2880_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_rt2880_setGpioDataBit(smi_SCK, 0);
	}
}



void _smi_readBit(uint32 bitLen, uint32 *rData) 
{
	uint32 u;

	/* change GPIO pin to Input only */
	_rt2880_initGpioPin(smi_SDA,RT2880GPIO_DIR_IN);

	for (*rData = 0; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* clocking */
		_rt2880_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_rt2880_getGpioDataBit(smi_SDA, &u);
		_rt2880_setGpioDataBit(smi_SCK, 0);
		*rData |= (u << (bitLen - 1));
	}

	/* change GPIO pin to Output only */
	_rt2880_initGpioPin(smi_SDA, RT2880GPIO_DIR_OUT);
}



void _smi_stop(void)
{

	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SDA, 0);	
	_rt2880_setGpioDataBit(smi_SCK, 1);	
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SDA, 1);	
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 1);

    /* add a click */
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_rt2880_setGpioDataBit(smi_SCK, 1);


	/* change GPIO pin to Output only */
	_rt2880_initGpioPin(smi_SDA, RT2880GPIO_DIR_IN);
	_rt2880_initGpioPin(smi_SCK, RT2880GPIO_DIR_IN);


}

#if 0
int32 smi_reset(uint32 port, uint32 pinRST)
{
	gpioID gpioId;
	int32 res;

	/* Initialize GPIO port A, pin 7 as SMI RESET */
	gpioId = GPIO_ID(port, pinRST);
	res = _rt2880_initGpioPin(gpioId,RT2880GPIO_DIR_OUT);
	if (res != SUCCESS)
		return res;
	smi_RST = gpioId;

	_rt2880_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(1000000);
	_rt2880_setGpioDataBit(smi_RST, 0);	
	CLK_DURATION(1000000);
	_rt2880_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(1000000);

	/* change GPIO pin to Input only */
	_rt2880_initGpioPin(smi_RST,RT2880GPIO_DIR_IN);

	return SUCCESS;
}
#endif

int32 smi_init(void)
{
	int32 res;
	
//GPIO MODE Setting
	u32 gpiomode;
	//res = smi_read(0x0105,&buf);
	
	#define RALINK_SYSCTL_ADDR		0xB0000000 // system control
	#define RALINK_REG_GPIOMODE		(RALINK_SYSCTL_ADDR + 0x60)
	#define RALINK_GPIOMODE_I2C		0x01
	#define RALINK_GPIOMODE_DFT		(RALINK_GPIOMODE_I2C)
	//config these pins to gpio mode
	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	gpiomode |= RALINK_GPIOMODE_DFT;
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);
//GPIO MODE Setting end	

	/*test write and read function pass 0x0101=>0x8366*/
	uint32 buf;
	smi_read(0x0105,&buf);
	printk("smi_init check[0x%04x]=0x%04x\n",0x0105,buf);	
	smi_read(0x0105,&buf);
	printk("smi_init check[0x%04x]=0x%04x\n",0x0105,buf);	

	/* change GPIO pin to Input only */
	/* Initialize GPIO port C, pin 0 as SMI SDA0 */
	res = _rt2880_initGpioPin(smi_SDA,RT2880GPIO_DIR_OUT);
	if (res != SUCCESS)
	{
		printk("RTL8366RS driver initSDA failed\n");
		return res;
	}
	
	/* Initialize GPIO port C, pin 1 as SMI SCK0 */
	res = _rt2880_initGpioPin(smi_SCK,RT2880GPIO_DIR_OUT);
	if (res != SUCCESS)
	{
		printk("RTL8366RS driver initSCK failed\n");
		return res;
	}

	_rt2880_setGpioDataBit(smi_SDA, 1);	
	_rt2880_setGpioDataBit(smi_SCK, 1);	
	
	//SetVLan();
	
	return SUCCESS;
}

int32 smi_read(uint32 mAddrs, uint32 *rData)
{
	uint32 rawData=0, ACK;
	uint8  con;
	uint32 ret = SUCCESS;
/*
	if ((mAddrs > max_register) || (rData == NULL))  return	FAILED;
*/

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	_rt2880_drvMutexLock();

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

	_rt2880_drvMutexUnlock();/*enable CPU interrupt*/

	return ret;
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{
/*
	if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return	FAILED;
*/
	int8 con;
	uint32 ACK;
	uint32 ret = SUCCESS;	

	/*Disable CPU interrupt to ensure that the SMI operation is atomic. 
	  The API is based on RTL865X, rewrite the API if porting to other platform.*/
   	_rt2880_drvMutexLock();

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

	_rt2880_drvMutexUnlock();/*enable CPU interrupt*/
	
	return ret;
}

void SetVLan()
{
	uint32 ret=0;
	
	//rtl8366s_initChip();
	/* set port 5 as CPU port */
	//rtl8366s_setCPUPort (5, FALSE, FALSE);	
	/* initialize VLAN */
	
	ret=rtl8366s_initVlan();
	if(ret==SUCCESS)
		printk("RTL8366RS driver SetVLan initialized\n");
	else
		printk("RTL8366RS driver SetVLan failed\n");
		
	/*
	all the ports are in the default VLAN 1 after VLAN initialized, modify it as follows
	VLAN1 member: port0, port5;
	VLAN2 member: port1, port2, port3, port4, port5
	VLAN3 member: port0, port1, port2, port3, port4, port5
	*/

	rtl8366s_setVlan(1, 0x21, 0x1f);
	rtl8366s_setVlan(2, 0x3e, 0x1f);
	rtl8366s_setVlan(3, 0x3f, 0x1f);

	/* set PVID for each port */
	rtl8366s_setVlanPVID(0, 1, 0);
	rtl8366s_setVlanPVID(1, 2, 0);
	rtl8366s_setVlanPVID(2, 2, 0);
	rtl8366s_setVlanPVID(3, 2, 0);
	rtl8366s_setVlanPVID(4, 2, 0);
	rtl8366s_setVlanPVID(5, 3, 0); 

#if 1
	int i;
	uint32 VID;
	uint32 priority;
	
	for(i=0; i<6;i++)
	{
		rtl8366s_getVlanPVID(i,&VID,&priority);
		printk("port=%d,VID=%d,priority=%d\n",i,VID,priority);
	}
#endif
}

