/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  smi connected RTL8366
* Abstract : 
* Author : Yu-Mei Pan (ympan@realtek.com.cn)                
*  $Id: smi.c,v 1.5 2007/05/23 11:30:59 abelshie Exp $
*/

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>     
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <asm/system.h>   
#include <linux/delay.h>

#include "rtl_smi.h"
#include "rtl2880.h"



#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

enum PORTID
{
	PORT0 =  0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT_MAX
};


extern int32 rtl8366s_initChip(void);
extern int32 rtl8366s_initVlan(void);
extern int32 rtl8366s_setVlan_with_fid(uint32 vid, uint32 mbrmsk, uint32 untagmsk, uint32 fid);
extern int32 rtl8366s_setVlanPVID(enum PORTID port, uint32 vid, uint32 priority);

#ifdef CONFIG_RALINK_RT3052
#define DELAY		 	1000	
#else
#define DELAY		 	100	
#endif
#define CLK_DURATION(clk)	{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)	{ /*if (!(ok)) return FAILED; */}

gpioID smi_SCK;		/* GPIO used for SMI Clock Generation */
gpioID smi_SDA;		/* GPIO used for SMI Data signal */
gpioID smi_RST;     	/* GPIO used for reset swtich */


#ifdef CONFIG_RALINK_RT3052
#define ack_timer	5
#else
#define ack_timer	100
#endif
#define max_register	0x018A 

#define GPIO_DIR_IN 	0
#define GPIO_DIR_OUT	1

int _initGpioDir(int gpioNO, int gpioDIR)
{
	uint32 tmpReg;

	tmpReg = cpu_to_le32(*(volatile uint32 *)(RT2880_REG_PIODIR));

//	printf("%x %d %d\n", tmpReg, gpioNO, gpioDIR); 

	if(gpioDIR == GPIO_DIR_OUT)
		tmpReg |= (1 << gpioNO);
	else
		tmpReg &= ~(1 << gpioNO);

	*(volatile uint32 *)(RT2880_REG_PIODIR) = cpu_to_le32(tmpReg);
//	printf("%x\n", tmpReg); 
	return SUCCESS;
}

int _setGpioDataBit(int gpioNO, int bitData)
{
        uint32 tmpReg;
        tmpReg = cpu_to_le32(*(volatile uint32 *)(RT2880_REG_PIODATA));
//	printf("%x set bit%d=%d\n", tmpReg, gpioNO, bitData); 

        if(bitData == 1)
		tmpReg |= (1 << gpioNO);
        else
		tmpReg &= ~(1 << gpioNO);

        *(volatile uint32 *)(RT2880_REG_PIODATA) = cpu_to_le32(tmpReg);
	
        tmpReg = cpu_to_le32(*(volatile uint32 *)(RT2880_REG_PIODATA));
//	printf("%x\n", tmpReg); 

        tmpReg = cpu_to_le32(*(volatile uint32 *)(RT2880_REG_PIODIR));
//	printf("%x\n", tmpReg); 

	return SUCCESS;
}

int _getGpioDataBit(int gpioNO, int *bitData)
{
        uint32 tmpReg;
        tmpReg = cpu_to_le32(*(volatile uint32 *)(RT2880_REG_PIODATA));

        if( (tmpReg & (1 << gpioNO)) == 0)
		*bitData = 0;
        else
                *bitData = 1;

	return SUCCESS;
}

int GPIO_ID(int port, int pin)
{
    return pin;
}

int _drvMutexLock(void)
{
        return 0;
}

int _drvMutexUnlock(void)
{
        return 0;
}

void _smi_start(void)
{

	/* change GPIO pin to Output only */
	_initGpioDir(smi_SDA, GPIO_DIR_OUT);
	_initGpioDir(smi_SCK, GPIO_DIR_OUT);
	
	/* Initial state: SCK: 0, SDA: 1 */
	_setGpioDataBit(smi_SCK, 0);
	_setGpioDataBit(smi_SDA, 1);
	CLK_DURATION(DELAY);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);

	/* CLK 2: */
	_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SDA, 0);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SDA, 1);

}


void _smi_writeBit(uint16 signal, uint32 bitLen)
{
	for( ; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* prepare data */
		if ( signal & (1<<(bitLen-1)) ) 
			_setGpioDataBit(smi_SDA, 1);	
		else 
			_setGpioDataBit(smi_SDA, 0);	
		CLK_DURATION(DELAY);

		/* clocking */
		_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_setGpioDataBit(smi_SCK, 0);
	}
}



void _smi_readBit(uint32 bitLen, uint32 *rData) 
{
	uint32 u;

	/* change GPIO pin to Input only */
	_initGpioDir(smi_SDA, GPIO_DIR_IN);

	for (*rData = 0; bitLen > 0; bitLen--)
	{
		CLK_DURATION(DELAY);

		/* clocking */
		_setGpioDataBit(smi_SCK, 1);
		CLK_DURATION(DELAY);
		_getGpioDataBit(smi_SDA, &u);
		_setGpioDataBit(smi_SCK, 0);

		*rData |= (u << (bitLen - 1));
	}

	/* change GPIO pin to Output only */
	_initGpioDir(smi_SDA, GPIO_DIR_OUT);
}



void _smi_stop(void)
{

	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SDA, 0);	
	_setGpioDataBit(smi_SCK, 1);	
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SDA, 1);	
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 1);

    /* add a click */
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 0);
	CLK_DURATION(DELAY);
	_setGpioDataBit(smi_SCK, 1);


	/* change GPIO pin to Output only */
	_initGpioDir(smi_SDA, GPIO_DIR_IN);
	_initGpioDir(smi_SCK, GPIO_DIR_IN);


}

int32 smi_reset(uint32 port, uint32 pinRST)
{
	gpioID gpioId;
	int32 res;

	/* Initialize GPIO port A, pin 7 as SMI RESET */
	gpioId = GPIO_ID(port, pinRST);
	res = _initGpioDir(gpioId, GPIO_DIR_OUT);
	if (res != SUCCESS)
		return res;
	smi_RST = gpioId;

	_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(100000);
	_setGpioDataBit(smi_RST, 0);	
	CLK_DURATION(100000);
	_setGpioDataBit(smi_RST, 1);
	CLK_DURATION(100000);

	/* change GPIO pin to Input only */
	_initGpioDir(smi_RST, GPIO_DIR_IN);

	return SUCCESS;
}


int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA)
{
	gpioID gpioId;
	int32 res;
	uint32 tmpReg;
#ifdef CONFIG_RALINK_RT3052
	tmpReg = le32_to_cpu(*(volatile uint32 *)(RT2880_REG_GPIOMODE));
	*(volatile uint32 *)(RT2880_REG_GPIOMODE) = cpu_to_le32(tmpReg | 0x1); // I2C mode
#else
	tmpReg = le32_to_cpu(*(volatile uint32 *)(RT2880_REG_GPIOMODE));
	*(volatile uint32 *)(RT2880_REG_GPIOMODE) = cpu_to_le32(tmpReg | 0x4 | 0x20);
#endif

 
	/* change GPIO pin to Input only */
	/* Initialize GPIO port C, pin 0 as SMI SDA0 */
	gpioId = GPIO_ID(port, pinSDA);
	res = _initGpioDir(gpioId, GPIO_DIR_OUT);
	if (res != SUCCESS)
		return res;
	smi_SDA = gpioId;


	/* Initialize GPIO port C, pin 1 as SMI SCK0 */
	gpioId = GPIO_ID(port, pinSCK);
	res = _initGpioDir(gpioId, GPIO_DIR_OUT);
	if (res != SUCCESS)
		return res;
	smi_SCK = gpioId;


	_setGpioDataBit(smi_SDA, 1);	
	_setGpioDataBit(smi_SCK, 1);	
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
   	_drvMutexLock();

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

	_drvMutexUnlock();/*enable CPU interrupt*/

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
   	_drvMutexLock();

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

	_drvMutexUnlock();/*enable CPU interrupt*/
	
	return ret;
}

int rtl_smi_ioctl (struct inode *inode, struct file *filp, \
                     unsigned int cmd, unsigned long arg)
{
        unsigned char w_byte[4];
        int i;
        unsigned int address, size;
        unsigned long value;


        switch (cmd) {

        default :
                printk(KERN_EMERG "rtl_smi_drv: command format error\n");
        }

        return 0;
}
#ifdef CONFIG_RTL8366_SWITCH	

int rtl8366_init(void)
{
	int    retVal=0;

	retVal|=rtl8366s_initChip();
	retVal|=rtl8366s_initVlan();

#ifdef CONFIG_BELKIN
	retVal|=rtl8366s_setVlan_with_fid(1, 0x3e, 0x1e, 1);
	retVal|=rtl8366s_setVlan_with_fid(2, 0x21, 0x01, 2);
	retVal|=rtl8366s_setVlanPVID(0, 2, 0);
	retVal|=rtl8366s_setVlanPVID(1, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(2, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(3, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(4, 1, 0);
   	retVal|=rtl8366s_setVlanPVID(5, 1, 0);
#else
	retVal|=rtl8366s_setVlan_with_fid(1, 0x2F, 0xF,  1);
	retVal|=rtl8366s_setVlan_with_fid(2, 0x30, 0x10, 2);
	retVal|=rtl8366s_setVlanPVID(0, 1, 0);
	retVal|=rtl8366s_setVlanPVID(1, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(2, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(3, 1, 0);
    	retVal|=rtl8366s_setVlanPVID(4, 2, 0);
   	retVal|=rtl8366s_setVlanPVID(5, 1, 0);
#endif
	return retVal;
}
#endif

int rtl_smi_init(void)
{
	int i, ii=0;
	uint32 buf;
	int32 res;
	//for(i=0; i<10 ;i++)
//rex
printk(KERN_EMERG "initialize Realtek VLAN config\n");
//	*RT2880_REG_SYSCFG &= RT2880GPIO_UART_MASK;
		//config these pins to gpio mode
//*(volatile uint32 *)(RT2880_REG_GPIOMODE) =RT2880_GPIOMODE_SPI;
//*(volatile uint32 *)(RT2880_REG_PIODIR) = RT2880GPIO_DIR;

	for(i=0; i<3 ;i++)
	{
		if (i)
		{
	        _initGpioDir(smi_SDA, GPIO_DIR_IN);
	        _initGpioDir(smi_SCK, GPIO_DIR_IN);
	        udelay(500);
	        }
#if defined(CONFIG_BR6474ND) //Kyle

	        smi_reset(0, 11);               // GPIO-11 for RESET SWITCH
	        smi_init(0, 4, 5);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 4, 5);              // use GPIO-4 for CLK, 5 for SDA

//	        smi_reset(0, 6);               // GPIO-6 for RESET SWITCH
//	        smi_init(0, 23, 22);              // use GPIO-23 for CLK, 22 for SDA
//	        for(ii=0; ii<0x200 +0x200*i; ii++)
//	                udelay(5000);
//	        smi_init(0, 23, 22);              // use GPIO-23 for CLK, 22 for SDA
#elif defined(CONFIG_AR690W)
	        smi_reset(0, 5);               // GPIO-11 for RESET SWITCH
	        smi_init(0, 4, 3);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 4, 3);              // use GPIO-4 for CLK, 5 for SDA
#elif defined(CONFIG_RT15N)
	        smi_reset(0, 7);               // GPIO-11 for RESET SWITCH
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
#elif defined(CONFIG_RTL8366RB_SWITCH)
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
		res = smi_read(0x0509,&buf);
		printk(KERN_EMERG "%d smi_init check[0x%04x]=0x%04x\n",res,0x0509,buf);
#elif defined(CONFIG_BELKIN)
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 2, 1);              // use GPIO-4 for CLK, 5 for SDA
		res = smi_read(0x0509,&buf);


#else
	        smi_reset(0, 11);               // GPIO-11 for RESET SWITCH
	        smi_init(0, 4, 5);              // use GPIO-4 for CLK, 5 for SDA
	        for(ii=0; ii<0x200 +0x200*i; ii++)
	                udelay(5000);
	        smi_init(0, 4, 5);              // use GPIO-4 for CLK, 5 for SDA
#endif
                udelay(500);

#ifdef CONFIG_RTL8366RB_SWITCH	
int switch_reset();
		if(switch_reset() == 0)    
#else
		if(rtl8366_init() == 0)
#endif
			break;
	}
		
	return 0;
}


