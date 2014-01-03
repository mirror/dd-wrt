#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "ralink_gpio.h"
#include <smi.h>
#include <rtk_types.h>
#include <rtk_error.h>
#include <rtk_api_ext.h>
#include <rtl8370_asicdrv_phy.h>
#include <rtl8370_asicdrv_port.h>


#include <linux/delay.h>	// msleep()


extern int rt_gpio_ioctl(unsigned int req, int idx, unsigned long arg);

extern int
ralink_gpio_write_bit(int idx, int value);

extern int
ralink_gpio_read_bit(int idx, int *value);

extern int
ralink_initGpioPin(unsigned int idx, int dir);



#define SR3			1

#define NAME			"rtl8367m"
#define RTL8367M_DEVNAME	"rtl8367m"
int rtl8367m_major = 206;

MODULE_DESCRIPTION("Realtek RTL8367M support");
MODULE_AUTHOR("Jiahao");
MODULE_LICENSE("GPL");

//#define DELAY			10000
#define DELAY			166
#define CLK_DURATION(clk)	{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)	{ /*if (!(ok)) return RT_ERR_FAILED; */}

typedef int gpioID;

#define GPIO_DIR_OUT	1
#define GPIO_DIR_IN	0

gpioID smi_SCK = 2;	/* GPIO used for SMI Clock Generation */	/* GPIO2 */
gpioID smi_SDA = 1;	/* GPIO used for SMI Data signal */		/* GPIO1 */
gpioID smi_RST = 7;	/* GPIO used for reset swtich */		/* GPIO7, not used */

#define ack_timer	5
#define max_register	0x018A

#define CONTROL_REG_PORT_POWER_BIT	0x800
#define LAN_PORT_1			3
#define LAN_PORT_2			2
#define LAN_PORT_3			1
#define LAN_PORT_4			0

void _smi_start(void)
{

    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_OUT);
    ralink_initGpioPin(smi_SCK, GPIO_DIR_OUT);
    
    /* Initial state: SCK: 0, SDA: 1 */
    ralink_gpio_write_bit(smi_SCK, 0);
    ralink_gpio_write_bit(smi_SDA, 1);
    CLK_DURATION(DELAY);

    /* CLK 1: 0 -> 1, 1 -> 0 */
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);

    /* CLK 2: */
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 1);

}



void _smi_writeBit(uint16 signal, uint32 bitLen)
{
    for( ; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* prepare data */
        if ( signal & (1<<(bitLen-1)) ) 
            ralink_gpio_write_bit(smi_SDA, 1);    
        else 
            ralink_gpio_write_bit(smi_SDA, 0);    
        CLK_DURATION(DELAY);

        /* clocking */
        ralink_gpio_write_bit(smi_SCK, 1);
        CLK_DURATION(DELAY);
        ralink_gpio_write_bit(smi_SCK, 0);
    }
}



void _smi_readBit(uint32 bitLen, uint32 *rData) 
{
    uint32 u;

    /* change GPIO pin to Input only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_IN);

    for (*rData = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* clocking */
        ralink_gpio_write_bit(smi_SCK, 1);
        CLK_DURATION(DELAY);
        ralink_gpio_read_bit(smi_SDA, &u);
        ralink_gpio_write_bit(smi_SCK, 0);

        *rData |= (u << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_OUT);
}



void _smi_stop(void)
{

    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 0);    
    ralink_gpio_write_bit(smi_SCK, 1);    
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 1);    
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);

    /* add a click */
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);


    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_IN);
    ralink_initGpioPin(smi_SCK, GPIO_DIR_IN);


}



int32 smi_read(uint32 mAddrs, uint32 *rData)
{
    uint32 rawData=0, ACK;
    uint8  con;
    uint32 ret = RT_ERR_OK;
/*
    if ((mAddrs > max_register) || (rData == NULL))  return    RT_ERR_FAILED;
*/

    /*Disable CPU interrupt to ensure that the SMI operation is atomic. 
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
//	Jiahao++
//       rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370 */

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x1, 1);                        /* 1: issue READ command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing READ command*/
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */    
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK by RTL8369 */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_readBit(8, &rawData);                    /* Read DATA [7:0] */
    *rData = rawData&0xff; 

    _smi_writeBit(0x00, 1);                        /* ACK by CPU */

    _smi_readBit(8, &rawData);                    /* Read DATA [15: 8] */

    _smi_writeBit(0x01, 1);                        /* ACK by CPU */
    *rData |= (rawData<<8);

    _smi_stop();
//	Jiahao++
//    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

    return ret;
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{
/*
    if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return    RT_ERR_FAILED;
*/
    int8 con;
    uint32 ACK;
    uint32 ret = RT_ERR_OK;    

    /*Disable CPU interrupt to ensure that the SMI operation is atomic. 
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
//	Jiahao++
//       rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370*/

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x0, 1);                        /* 0: issue WRITE command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing WRITE command*/
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData&0xff, 8);                /* Write Data [7:0] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for writting data [7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData>>8, 8);                    /* Write Data [15:8] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                        /* ACK for writting data [15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_stop();    
//	Jiahao++
//    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/
    
    return ret;
}

int rt_gpio_ioctl(unsigned int req, int idx, unsigned long arg)
{
	unsigned long tmp;
/*
	unsigned long PIODATA0;
	unsigned long PIODATA1;
	unsigned long PIODATA2;
*/
	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_READ_BIT:
/*
		printk("RALINK_REG_PIODATA: %x\n",  RALINK_REG_PIODATA);
		printk("RALINK_REG_PIO3924DATA: %x\n",  RALINK_REG_PIO3924DATA);
		printk("RALINK_REG_PIO5140DATA: %x\n",  RALINK_REG_PIO5140DATA);
		PIODATA0 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		PIODATA1 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		PIODATA2 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		printk("*RALINK_REG_PIODATA: %x\n",	PIODATA0);
		printk("*RALINK_REG_PIO3924DATA: %x\n",	PIODATA1);
		printk("*RALINK_REG_PIO5140DATA: %x\n",	PIODATA2);
*/

		if (idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

		if (idx <= 23)
			tmp = (tmp >> idx) & 1L;
		else if (idx <= 39)
			tmp = (tmp >> (idx-24)) & 1L;
		else
			tmp = (tmp >> (idx-40)) & 1L;
		return tmp;

		break;
	case RALINK_GPIO_WRITE_BIT:
		if (idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (arg & 1L)
				tmp |= (1L << idx);
			else
				tmp &= ~(1L << idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-24));
			else
				tmp &= ~(1L << (idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-40));
			else
				tmp &= ~(1L << (idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;
	default:
		return -1;
	}
	return 0;
}

int
ralink_gpio_write_bit(int idx, int value)
{
	unsigned int req;
	value &= 1;
       
	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_WRITE_BIT;
	else
		return -1;

	return rt_gpio_ioctl(req, idx, value);
}

int
ralink_gpio_read_bit(int idx, int *value)
{
	unsigned int req;
	*value = 0;

	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_READ_BIT;
	else
		return -1;

	if ((*value = rt_gpio_ioctl(req, idx, value)) < 0)
		return -1;

	return *value;
}

int
ralink_initGpioPin(unsigned int idx, int dir)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (dir == GPIO_DIR_OUT)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
		}
	}
	else if (dir == GPIO_DIR_IN)
	{		
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
		}
	}
	else
		return -1;

	if (idx <= 23) {
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
	else {
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
	}

	return 0;
}

void LANWANPartition()
{
	rtk_portmask_t fwd_mask;
 
	/* LAN */
	fwd_mask.bits[0] = 0x10F;
	rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
	rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
	rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
	rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
	rtk_port_isolation_set(8, fwd_mask);
 
	/* WAN */
	fwd_mask.bits[0] = 0x210;
	rtk_port_isolation_set(4, fwd_mask);
	rtk_port_isolation_set(9, fwd_mask);
 
	/* EFID setting LAN */
	rtk_port_efid_set(LAN_PORT_1, 0);
	rtk_port_efid_set(LAN_PORT_2, 0);
	rtk_port_efid_set(LAN_PORT_3, 0);
	rtk_port_efid_set(LAN_PORT_4, 0);
	rtk_port_efid_set(8, 0);
 
	/* EFID setting WAN */
	rtk_port_efid_set(4, 1);
	rtk_port_efid_set(9, 1);
}

static int wan_stb_g = 0;

void LANWANPartition_adv(int wan_stb_x)
{
	rtk_portmask_t fwd_mask;
 
	/* LAN */
	if (wan_stb_x == 0)		// P0,P1,P2,P3
	{				// default mode
		fwd_mask.bits[0] = 0x10F;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
	else if (wan_stb_x == 1)	// P0,P1,P2
	{
		fwd_mask.bits[0] = 0x107;
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
	else if (wan_stb_x == 2)	// P0,P1,P3
	{
		fwd_mask.bits[0] = 0x10B;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
	else if (wan_stb_x == 3)	// P0,P2,P3
	{
		fwd_mask.bits[0] = 0x10D;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
 	else if (wan_stb_x == 4)	// P1,P2,P3
	{
		fwd_mask.bits[0] = 0x10E;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
	else if (wan_stb_x == 5)	// P2,P3
	{
		fwd_mask.bits[0] = 0x10C;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
 	else if (wan_stb_x == 6)	// P0,P1
	{
		fwd_mask.bits[0] = 0x103;
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(8, fwd_mask);
	}
 
	/* WAN */
	if (wan_stb_x == 0)		// P4
	{				// default mode
		fwd_mask.bits[0] = 0x210;
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 1)	// P3,P4
	{
		fwd_mask.bits[0] = 0x218;
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 2)	// P2,P4
	{
		fwd_mask.bits[0] = 0x214;
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 3)	// P1,P4
	{
		fwd_mask.bits[0] = 0x212;
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 4)	// P0,P4
	{
		fwd_mask.bits[0] = 0x211;
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 5)	// P0,P1,P4
	{
		fwd_mask.bits[0] = 0x213;
		rtk_port_isolation_set(LAN_PORT_3, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_4, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
	else if (wan_stb_x == 6)	// P2,P3,P4
	{
		fwd_mask.bits[0] = 0x21C;
		rtk_port_isolation_set(LAN_PORT_2, fwd_mask);
		rtk_port_isolation_set(LAN_PORT_1, fwd_mask);
		rtk_port_isolation_set(4, fwd_mask);
		rtk_port_isolation_set(9, fwd_mask);
	}
 
	/* EFID setting LAN */
	if (wan_stb_x == 0)		// P0,P1,P2,P3
	{				// default mode
		rtk_port_efid_set(LAN_PORT_1, 0);
		rtk_port_efid_set(LAN_PORT_2, 0);
		rtk_port_efid_set(LAN_PORT_3, 0);
		rtk_port_efid_set(LAN_PORT_4, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 1)	// P0,P1,P2
	{
		rtk_port_efid_set(LAN_PORT_2, 0);
		rtk_port_efid_set(LAN_PORT_3, 0);
		rtk_port_efid_set(LAN_PORT_4, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 2)	// P0,P1,P3
	{
		rtk_port_efid_set(LAN_PORT_1, 0);
		rtk_port_efid_set(LAN_PORT_3, 0);
		rtk_port_efid_set(LAN_PORT_4, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 3)	// P0,P2,P3
	{
		rtk_port_efid_set(LAN_PORT_1, 0);
		rtk_port_efid_set(LAN_PORT_2, 0);
		rtk_port_efid_set(LAN_PORT_4, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 4)	// P1,P2,P3
	{
		rtk_port_efid_set(LAN_PORT_1, 0);
		rtk_port_efid_set(LAN_PORT_2, 0);
		rtk_port_efid_set(LAN_PORT_3, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 5)	// P2,P3
	{
		rtk_port_efid_set(LAN_PORT_1, 0);
		rtk_port_efid_set(LAN_PORT_2, 0);
		rtk_port_efid_set(8, 0);
	}
	else if (wan_stb_x == 6)	// P0,P1
	{
		rtk_port_efid_set(LAN_PORT_4, 0);
		rtk_port_efid_set(LAN_PORT_3, 0);
		rtk_port_efid_set(8, 0);
	}
 
	/* EFID setting WAN */
	if (wan_stb_x == 0)		// P4
	{				// default mode
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 1)	// P3,P4
	{
		rtk_port_efid_set(LAN_PORT_1, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 2)	// P2,P4
	{
		rtk_port_efid_set(LAN_PORT_2, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 3)	// P1,P4
	{
		rtk_port_efid_set(LAN_PORT_3, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 4)	// P0,P4
	{
		rtk_port_efid_set(LAN_PORT_4, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 5)	// P0,P1,P4
	{
		rtk_port_efid_set(LAN_PORT_3, 1);
		rtk_port_efid_set(LAN_PORT_4, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
	else if (wan_stb_x == 6)	// P2,P3,P4
	{
		rtk_port_efid_set(LAN_PORT_2, 1);
		rtk_port_efid_set(LAN_PORT_1, 1);
		rtk_port_efid_set(4, 1);
		rtk_port_efid_set(9, 1);
	}
}

rtk_api_ret_t rtk_port_linkStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    if ((retVal = rtl8370_setAsicPHYReg(port,RTL8370_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;  

    /*Get PHY status register*/
    if (RT_ERR_OK != rtl8370_getAsicPHYReg(port,PHY_STATUS_REG,&phyData))
        return RT_ERR_FAILED;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;
    }
    else
    {
        *pLinkStatus = 0;
    }

    return RT_ERR_OK;
}

typedef struct {
	unsigned int idx;
	unsigned int value;
} asus_gpio_info;

typedef struct {
	unsigned int link[5];
	unsigned int speed[5];
} phyState;

unsigned int txDelay_user = 1;
unsigned int rxDelay_user = 3;

int rtl8367m_ioctl(struct inode *inode, struct file *file, unsigned int req,
		unsigned long arg)
{
	rtk_api_ret_t retVal;
	rtk_port_t port;
	rtk_stat_port_cntr_t pPort_cntrs;
	rtk_port_linkStatus_t pLinkStatus = 0;
	rtk_data_t pSpeed;
	rtk_data_t pDuplex;
	unsigned long tmp;
	asus_gpio_info info;
	phyState pS;
//	unsigned long gpiomode;
	int port_user = 0;
	int wan_stb_x = 0;
	rtk_data_t txDelay_ro, rxDelay_ro;
	unsigned int regData;
	rtk_port_phy_data_t pData;
	unsigned int control_rate;

	switch(req) {
	case 0:					// check WAN port phy status
		if (wan_stb_g == 0)		// P4
		{				// default mode
			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 1)	// P3,P4
		{
			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 2)	// P2,P4
		{
			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 3)	// P1,P4
		{
			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 4)	// P0,P4
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 5)	// P0,P1,P4
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 6)	// P2,P3,P4
		{
			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = 4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}

		pLinkStatus = 0;
		put_user(pLinkStatus, (unsigned int __user *)arg);
		break;

	case 3:					// check LAN ports phy status
		if (wan_stb_g == 0)		// P0,P1,P2,P3
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 1)	// P0,P1,P2
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 2)	// P0,P1,P3
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 3)	// P0,P2,P3
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
 		else if (wan_stb_g == 4)	// P1,P2,P3
 		{
			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 5)	// P2,P3
		{
			port = LAN_PORT_2;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_1;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}
		else if (wan_stb_g == 6)	// P0,P1
		{
			port = LAN_PORT_4;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			if (pLinkStatus)
			{
				put_user(pLinkStatus, (unsigned int __user *)arg);
				break;
			}

			port = LAN_PORT_3;
			rtk_port_linkStatus_get(port, &pLinkStatus);
			put_user(pLinkStatus, (unsigned int __user *)arg);
			break;
		}

		pLinkStatus = 0;
		put_user(pLinkStatus, (unsigned int __user *)arg);
		break;

	case 2:			// show state of RTL8367M GMAC1
		port = 9;
		retVal = rtk_stat_port_getAll(port, &pPort_cntrs);

		if (retVal == RT_ERR_OK)
		{
			printk("ifInOctets:%d \ndot3StatsFCSErrors:%d \ndot3StatsSymbolErrors:%d \ndot3InPauseFrames:%d \ndot3ControlInUnknownOpcodes:%d \netherStatsFragments:%d \netherStatsJabbers:%d \nifInUcastPkts:%d \netherStatsDropEvents:%d \netherStatsOctets:%d \netherStatsUndersizePkts:%d \netherStatsOversizePkts:%d \netherStatsPkts64Octets:%d \netherStatsPkts65to127Octets:%d \netherStatsPkts128to255Octets:%d \netherStatsPkts256to511Octets:%d \netherStatsPkts512to1023Octets:%d \netherStatsPkts1024toMaxOctets:%d \netherStatsMcastPkts:%d \netherStatsBcastPkts:%d \nifOutOctets:%d \ndot3StatsSingleCollisionFrames:%d \ndot3StatsMultipleCollisionFrames:%d \ndot3StatsDeferredTransmissions:%d \ndot3StatsLateCollisions:%d \netherStatsCollisions:%d \ndot3StatsExcessiveCollisions:%d \ndot3OutPauseFrames:%d \ndot1dBasePortDelayExceededDiscards:%d \ndot1dTpPortInDiscards:%d \nifOutUcastPkts:%d \nifOutMulticastPkts:%d \nifOutBrocastPkts:%d \noutOampduPkts:%d \ninOampduPkts:%d \npktgenPkts:%d\n\n",
				pPort_cntrs.ifInOctets,
				pPort_cntrs.dot3StatsFCSErrors,
				pPort_cntrs.dot3StatsSymbolErrors,
				pPort_cntrs.dot3InPauseFrames,
				pPort_cntrs.dot3ControlInUnknownOpcodes,
				pPort_cntrs.etherStatsFragments,
				pPort_cntrs.etherStatsJabbers,
				pPort_cntrs.ifInUcastPkts,
				pPort_cntrs.etherStatsDropEvents,
				pPort_cntrs.etherStatsOctets,
				pPort_cntrs.etherStatsUndersizePkts,
				pPort_cntrs.etherStatsOversizePkts,
				pPort_cntrs.etherStatsPkts64Octets,
				pPort_cntrs.etherStatsPkts65to127Octets,
				pPort_cntrs.etherStatsPkts128to255Octets,
				pPort_cntrs.etherStatsPkts256to511Octets,
				pPort_cntrs.etherStatsPkts512to1023Octets,
				pPort_cntrs.etherStatsPkts1024toMaxOctets,
				pPort_cntrs.etherStatsMcastPkts,
				pPort_cntrs.etherStatsBcastPkts,	
				pPort_cntrs.ifOutOctets,
				pPort_cntrs.dot3StatsSingleCollisionFrames,
				pPort_cntrs.dot3StatsMultipleCollisionFrames,
				pPort_cntrs.dot3StatsDeferredTransmissions,
				pPort_cntrs.dot3StatsLateCollisions,
				pPort_cntrs.etherStatsCollisions,
				pPort_cntrs.dot3StatsExcessiveCollisions,
				pPort_cntrs.dot3OutPauseFrames,
				pPort_cntrs.dot1dBasePortDelayExceededDiscards,
				pPort_cntrs.dot1dTpPortInDiscards,
				pPort_cntrs.ifOutUcastPkts,
				pPort_cntrs.ifOutMulticastPkts,
				pPort_cntrs.ifOutBrocastPkts,
				pPort_cntrs.outOampduPkts,
				pPort_cntrs.inOampduPkts,
				pPort_cntrs.pktgenPkts
			);
		}
		else
			printk("rtk_stat_port_getAll() return %d\n", retVal);
		
		break;

	case 6:
		copy_from_user(&port_user, (int __user *)arg, sizeof(int));
		port = port_user;
		printk("rtk_stat_port_getAll(%d...)\n", port);
		retVal = rtk_stat_port_getAll(port, &pPort_cntrs);

		if (retVal == RT_ERR_OK)
		{
			printk("ifInOctets: %lld \ndot3StatsFCSErrors: %d \ndot3StatsSymbolErrors: %d \ndot3InPauseFrames: %d \ndot3ControlInUnknownOpcodes: %d \netherStatsFragments: %d \netherStatsJabbers: %d \nifInUcastPkts: %d \netherStatsDropEvents: %d \netherStatsOctets: %lld \netherStatsUndersizePkts: %d \netherStatsOversizePkts: %d \netherStatsPkts64Octets: %d \netherStatsPkts65to127Octets: %d \netherStatsPkts128to255Octets: %d \netherStatsPkts256to511Octets: %d \netherStatsPkts512to1023Octets: %d \netherStatsPkts1024toMaxOctets: %d \netherStatsMcastPkts: %d \netherStatsBcastPkts: %d \nifOutOctets: %lld \ndot3StatsSingleCollisionFrames: %d \ndot3StatsMultipleCollisionFrames: %d \ndot3StatsDeferredTransmissions: %d \ndot3StatsLateCollisions: %d \netherStatsCollisions: %d \ndot3StatsExcessiveCollisions: %d \ndot3OutPauseFrames: %d \ndot1dBasePortDelayExceededDiscards: %d \ndot1dTpPortInDiscards: %d \nifOutUcastPkts: %d \nifOutMulticastPkts: %d \nifOutBrocastPkts: %d \noutOampduPkts: %d \ninOampduPkts: %d \npktgenPkts: %d\n\n",
				pPort_cntrs.ifInOctets,
				pPort_cntrs.dot3StatsFCSErrors,
				pPort_cntrs.dot3StatsSymbolErrors,
				pPort_cntrs.dot3InPauseFrames,
				pPort_cntrs.dot3ControlInUnknownOpcodes,
				pPort_cntrs.etherStatsFragments,
				pPort_cntrs.etherStatsJabbers,
				pPort_cntrs.ifInUcastPkts,
				pPort_cntrs.etherStatsDropEvents,
				pPort_cntrs.etherStatsOctets,
				pPort_cntrs.etherStatsUndersizePkts,
				pPort_cntrs.etherStatsOversizePkts,
				pPort_cntrs.etherStatsPkts64Octets,
				pPort_cntrs.etherStatsPkts65to127Octets,
				pPort_cntrs.etherStatsPkts128to255Octets,
				pPort_cntrs.etherStatsPkts256to511Octets,
				pPort_cntrs.etherStatsPkts512to1023Octets,
				pPort_cntrs.etherStatsPkts1024toMaxOctets,
				pPort_cntrs.etherStatsMcastPkts,
				pPort_cntrs.etherStatsBcastPkts,	
				pPort_cntrs.ifOutOctets,
				pPort_cntrs.dot3StatsSingleCollisionFrames,
				pPort_cntrs.dot3StatsMultipleCollisionFrames,
				pPort_cntrs.dot3StatsDeferredTransmissions,
				pPort_cntrs.dot3StatsLateCollisions,
				pPort_cntrs.etherStatsCollisions,
				pPort_cntrs.dot3StatsExcessiveCollisions,
				pPort_cntrs.dot3OutPauseFrames,
				pPort_cntrs.dot1dBasePortDelayExceededDiscards,
				pPort_cntrs.dot1dTpPortInDiscards,
				pPort_cntrs.ifOutUcastPkts,
				pPort_cntrs.ifOutMulticastPkts,
				pPort_cntrs.ifOutBrocastPkts,
				pPort_cntrs.outOampduPkts,
				pPort_cntrs.inOampduPkts,
				pPort_cntrs.pktgenPkts
			);
		}
		else
			printk("rtk_stat_port_getAll() return %d\n", retVal);
		
		break;

	case RALINK_GPIO_SET_DIR:	// 0x01
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		if (info.idx < 0 || RALINK_GPIO_NUMBER <= info.idx)
			return -1;
/*
		gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
		printk("GPIOMODE: %x\n",  gpiomode);
*/
		if (info.value == GPIO_DIR_OUT)
		{
			if (info.idx <= 23) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
				tmp |= (1L << info.idx);
			}
			else if (info.idx <= 39) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
				tmp |= (1L << (info.idx-24));
			}
			else {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
				tmp |= (1L << (info.idx-40));
			}
		}
		else if (info.value == GPIO_DIR_IN)
		{		
			if (info.idx <= 23) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
				tmp &= ~(1L << info.idx);
			}
			else if (info.idx <= 39) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
				tmp &= ~(1L << (info.idx-24));
			}
			else {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
				tmp &= ~(1L << (info.idx-40));
			}
		}
		else
			return -1;
	
		if (info.idx <= 23) {
			*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
		}
		else if (info.idx <= 39) {
			*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
		}
		else {
			*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
		}
	
		return 0;

		break;

	case RALINK_GPIO_READ_BIT:	// 0x04
//		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		copy_from_user(&info.idx, (int __user *)arg, sizeof(int));

		if (info.idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (info.idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

		if (info.idx <= 23)
			tmp = (tmp >> info.idx) & 1L;
		else if (info.idx <= 39)
			tmp = (tmp >> (info.idx-24)) & 1L;
		else
			tmp = (tmp >> (info.idx-40)) & 1L;
//		return tmp;
//		put_user(tmp, (int __user *)&((*(asus_gpio_info *)arg).value));
		put_user(tmp, (int __user *)arg);

		break;

	case RALINK_GPIO_WRITE_BIT:	// 0x05
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		if (info.idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (info.value & 1L)
				tmp |= (1L << info.idx);
			else
				tmp &= ~(1L << info.idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (info.idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (info.value & 1L)
				tmp |= (1L << (info.idx-24));
			else
				tmp &= ~(1L << (info.idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (info.value & 1L)
				tmp |= (1L << (info.idx-40));
			else
				tmp &= ~(1L << (info.idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;

	case 7:
		printk("rtk_stat_port_reset()\n");
		if (wan_stb_g == 0)		// P4
		{				// default mode
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 1)	// P3,P4
		{
			rtk_stat_port_reset(LAN_PORT_4);
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 2)	// P2,P4
		{
			rtk_stat_port_reset(LAN_PORT_3);
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 3)	// P1,P4
		{
			rtk_stat_port_reset(LAN_PORT_2);
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 4)	// P0,P4
		{
			rtk_stat_port_reset(LAN_PORT_1);
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 5)	// P0,P1,P4
		{
			rtk_stat_port_reset(LAN_PORT_2);
			rtk_stat_port_reset(LAN_PORT_1);
			rtk_stat_port_reset(4);
		}
		else if (wan_stb_g == 6)	// P2,P3,P4
		{
			rtk_stat_port_reset(LAN_PORT_4);
			rtk_stat_port_reset(LAN_PORT_3);
			rtk_stat_port_reset(4);
		}
		rtk_stat_port_reset(8);
		rtk_stat_port_reset(9);

		break;

	case 8:
		copy_from_user(&wan_stb_x, (int __user *)arg, sizeof(int));
		if (wan_stb_x == 0)
		{
			printk("LAN: P0,P1,P2,P3 WAN: P4\n");
		}
		else if (wan_stb_x == 1)
		{
			printk("LAN: P0,P1,P2 WAN: P3,P4\n");
		}
		else if (wan_stb_x == 2)
		{
			printk("LAN: P0,P1,P3 WAN: P2,P4\n");
		}
		else if (wan_stb_x == 3)
		{
			printk("LAN: P0,P2,P3 WAN: P1,P4\n");
		}
		else if (wan_stb_x == 4)
		{
			printk("LAN: P1,P2,P3 WAN: P0,P4\n");
		}
		else if (wan_stb_x == 5)
		{
			printk("LAN: P2,P3 WAN: P0,P1,P4\n");
		}
		else if (wan_stb_x == 6)
		{
			printk("LAN: P0,P1 WAN: P2,P3,P4\n");
		}
		wan_stb_g = wan_stb_x;
		LANWANPartition_adv(wan_stb_x);

		break;

	case 9:
		copy_from_user(&txDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		printk("txDelay_user: %d\n", txDelay_user);

		printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user);
		retVal = rtk_port_rgmiiDelayExt0_set(txDelay_user, rxDelay_user);
		printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);
		retVal = rtk_port_rgmiiDelayExt1_set(txDelay_user, rxDelay_user);
		printk("rtk_port_rgmiiDelayExt1_set(): return %d\n", retVal);

		rtk_port_rgmiiDelayExt0_get(&txDelay_ro, &rxDelay_ro);
		printk("current Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);
		rtk_port_rgmiiDelayExt1_get(&txDelay_ro, &rxDelay_ro);
		printk("current Ext1 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);

		break;

	case 10:
		copy_from_user(&rxDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		printk("rxDelay_user: %d\n", rxDelay_user);

		printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user);
		retVal = rtk_port_rgmiiDelayExt0_set(txDelay_user, rxDelay_user);
		printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);
		retVal = rtk_port_rgmiiDelayExt1_set(txDelay_user, rxDelay_user);
		printk("rtk_port_rgmiiDelayExt1_set(): return %d\n", retVal);

		rtk_port_rgmiiDelayExt0_get(&txDelay_ro, &rxDelay_ro);
		printk("current Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);
		rtk_port_rgmiiDelayExt1_get(&txDelay_ro, &rxDelay_ro);
		printk("current Ext1 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);

		break;

	case 11:
		regData = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
		printk("GPIOMODE before: %x\n",  regData);
		regData &= ~RALINK_GPIOMODE_JTAG;
		regData |= RALINK_GPIOMODE_JTAG;
		printk("GPIOMODE writing: %x\n", regData);
		*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(regData);

		break;

	case 13:		// check WAN port phy speed
                port = 4;	// port 4 is WAN port
		retVal = rtk_port_phyStatus_get(port, &pLinkStatus, &pSpeed, &pDuplex);
//		printk("rtk_port_phyStatus_get() : %d\n", pSpeed);
		put_user(pSpeed, (unsigned int __user *)arg);

		break;

	case 14:		// power up LAN port(s)
		if (wan_stb_g == 0)		// P0,P1,P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 1)	// P0,P1,P2
		{
			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 2)	// P0,P1,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 3)	// P0,P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
 		else if (wan_stb_g == 4)	// P1,P2,P3
 		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 5)	// P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 6)	// P0,P1
		{
			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData &= ~CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		
		break;

	case 15:		// power down LAN port(s)		
		if (wan_stb_g == 0)		// P0,P1,P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 1)	// P0,P1,P2
		{
			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 2)	// P0,P1,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 3)	// P0,P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}
 		else if (wan_stb_g == 4)	// P1,P2,P3
 		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 5)	// P2,P3
		{
			rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);
		}
		else if (wan_stb_g == 6)	// P0,P1
		{
			rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

			rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
			pData |= CONTROL_REG_PORT_POWER_BIT;
			rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);
		}

		break;

	case 16:		// power up all ports
		printk("power up all ports\n");

		rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

		rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

		rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

		rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);

		rtk_port_phyReg_get(4, PHY_CONTROL_REG, &pData);
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(4, PHY_CONTROL_REG, pData);

		break;

	case 17:		// power down all ports
		printk("power down all ports\n");
                        
		rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);
                        
		rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);
                        
		rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);
                        
		rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);

		rtk_port_phyReg_get(4, PHY_CONTROL_REG, &pData);
		pData |= CONTROL_REG_PORT_POWER_BIT;
		rtk_port_phyReg_set(4, PHY_CONTROL_REG, pData);

		break;

	case 18:		// phy status for ATE command
		copy_from_user(&pS, (phyState __user *)arg, sizeof(pS));

		for (port = 0; port < 5; port++)
		{
                	retVal = rtk_port_phyStatus_get(port, &pLinkStatus, &pSpeed, &pDuplex);
			pS.link[port] = pLinkStatus;
			pS.speed[port] = pSpeed;
		}                

		copy_to_user((phyState __user *)arg, &pS, sizeof(pS));

		break;

        case 19:
                copy_from_user(&txDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
                printk("WAN port txDelay: %d\n", txDelay_user);

                printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user);
                retVal = rtk_port_rgmiiDelayExt0_set(txDelay_user, rxDelay_user);
                printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);

                rtk_port_rgmiiDelayExt0_get(&txDelay_ro, &rxDelay_ro);
                printk("current Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);

                break;

        case 20:
		copy_from_user(&rxDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		printk("WAN port rxDelay: %d\n", rxDelay_user);

		printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user);
		retVal = rtk_port_rgmiiDelayExt0_set(txDelay_user, rxDelay_user);
		printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);

		rtk_port_rgmiiDelayExt0_get(&txDelay_ro, &rxDelay_ro);
		printk("current Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);

		break;

	case 21:
		printk("reset strom control\n");

		rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(0, STORM_GROUP_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(0, STORM_GROUP_BROADCAST, 1048568, 1, 0);
	
		rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_BROADCAST, 1048568, 1, 0);
	
		rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_BROADCAST, 1048568, 1, 0);
	
		rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_BROADCAST, 1048568, 1, 0);
	
		rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_MULTICAST, 1048568, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_BROADCAST, 1048568, 1, 0);

		break;

        case 22:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set unknown unicast strom control as: %d\n", control_rate);
		rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		
		break;

        case 23:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set unknown multicast strom control as: %d\n", control_rate);
		rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		
		break;

        case 24:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set multicast strom control as: %d\n", control_rate);
		rtk_storm_controlRate_set(0, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		
		break;

        case 25:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set broadcast strom control as: %d\n", control_rate);
		rtk_storm_controlRate_set(0, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(1, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(2, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(3, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		rtk_storm_controlRate_set(4, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		
		break;

	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}

int rtl8367m_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
//	MOD_INC_USE_COUNT;
#else
//	try_module_get(THIS_MODULE);
#endif
	return 0;
}

int rtl8367m_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
//	MOD_DEC_USE_COUNT;
#else
//	module_put(THIS_MODULE);
#endif
	return 0;
}

struct file_operations rtl8367m_fops =
{
	owner:		THIS_MODULE,
	ioctl:		rtl8367m_ioctl,
	open:		rtl8367m_open,
	release:	rtl8367m_release,
};

int __init rtl8367m_init(void)
{
	int r = 0;
	r = register_chrdev(rtl8367m_major, RTL8367M_DEVNAME,
			&rtl8367m_fops);
	if (r < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return r;
	}
	if (rtl8367m_major == 0) {
		rtl8367m_major = r;
		printk(KERN_DEBUG NAME ": got dynamic major %d\n", r);
	}

	printk("software reset RTL8367M...\n");
	rtl8370_setAsicReg(0x1322, 1);	// software reset
	msleep(1000);

	int retVal;
	retVal = rtk_switch_init();
	printk("rtk_switch_init() return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;

	rtk_port_mac_ability_t mac_cfg;
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= 1;
	mac_cfg.nway		= 0;
	mac_cfg.rxpause		= 1;
	mac_cfg.txpause		= 1;

	retVal = rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);	// WAN port
	printk("rtk_port_macForceLinkExt0_set(): return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;
	retVal = rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);	// LAN ports
	printk("rtk_port_macForceLinkExt1_set(): return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;

	/* power down all ports */
	rtk_port_phy_data_t pData;
	printk("power down all ports\n");

	rtk_port_phyReg_get(LAN_PORT_4, PHY_CONTROL_REG, &pData);
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(LAN_PORT_4, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(LAN_PORT_3, PHY_CONTROL_REG, &pData);
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(LAN_PORT_3, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(LAN_PORT_2, PHY_CONTROL_REG, &pData);
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(LAN_PORT_2, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(LAN_PORT_1, PHY_CONTROL_REG, &pData);
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(LAN_PORT_1, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(4, PHY_CONTROL_REG, &pData);
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(4, PHY_CONTROL_REG, pData);

	rtk_data_t txDelay_ro, rxDelay_ro;	
	rtk_port_rgmiiDelayExt0_get(&txDelay_ro, &rxDelay_ro);
	printk("org Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);
	rtk_port_rgmiiDelayExt1_get(&txDelay_ro, &rxDelay_ro);
	printk("org Ext1 txDelay: %d, rxDelay: %d\n", txDelay_ro, rxDelay_ro);

	rtk_data_t txDelay_ext0 = 1;
#ifdef SR3
	rtk_data_t rxDelay_ext0 = 0;
#else
	rtk_data_t rxDelay_ext0 = 3;
#endif
	printk("new Ext0 txDelay: %d, rxDelay: %d\n", txDelay_ext0, rxDelay_ext0);
	retVal = rtk_port_rgmiiDelayExt0_set(txDelay_ext0, rxDelay_ext0);
	printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;

	rtk_data_t txDelay_ext1 = 1;
#ifdef SR3
	rtk_data_t rxDelay_ext1 = 0;
#else
	rtk_data_t rxDelay_ext1 = 3;
#endif
	printk("new Ext1 txDelay: %d, rxDelay: %d\n", txDelay_ext1, rxDelay_ext1);
	retVal = rtk_port_rgmiiDelayExt1_set(txDelay_ext1, rxDelay_ext1);
	printk("rtk_port_rgmiiDelayExt1_set(): return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;

	rtk_portmask_t portmask;
	portmask.bits[0] = 0x1F;

	retVal = rtk_led_enable_set(LED_GROUP_0, portmask);
	printk("rtk_led_enable_set(LED_GROUP_0...): return %d\n", retVal);
	retVal = rtk_led_enable_set(LED_GROUP_1, portmask);
	printk("rtk_led_enable_set(LED_GROUP_1...): return %d\n", retVal);

	retVal = rtk_led_operation_set(LED_OP_PARALLEL);
	printk("rtk_led_operation_set(): return %d\n", retVal);
/*
	retVal = rtk_led_mode_set(LED_MODE_1);
	printk("rtk_led_mode_set(): return %d\n", retVal);

	retVal = rtk_led_blinkRate_set(LED_BLINKRATE_256MS);
	printk("rtk_led_blinkRate_set(): return %d\n", retVal);
*/
	retVal = rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);
	printk("rtk_led_groupConfig_set(LED_GROUP_0...): return %d\n", retVal);
	retVal = rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);
	printk("rtk_led_groupConfig_set(LED_GROUP_1...): return %d\n", retVal);

	rtk_data_t BlinkRate;
	rtk_led_blinkRate_get(&BlinkRate);
	printk("current led blinkRate: %d\n", BlinkRate);

	rtk_data_t pLen;
	retVal = rtk_switch_maxPktLen_get(&pLen);
	printk("rtk_switch_maxPktLen_get(): return %d\n", retVal);
	printk("current rtk_switch_maxPktLen: %d\n", pLen);
	retVal = rtk_switch_maxPktLen_set(MAXPKTLEN_16000B);
	printk("rtk_switch_maxPktLen_set(): return %d\n", retVal);

	rtk_data_t pEnable;
	retVal = rtk_switch_greenEthernet_get(&pEnable);
	printk("rtk_switch_greenEthernet_get(): return %d\n", retVal);
	printk("current rtk_switch_greenEthernet state: %d\n", pEnable);
	retVal = rtk_switch_greenEthernet_set(ENABLED);
	printk("rtk_switch_greenEthernet_set(): return %d\n", retVal);

	/* Enable 802.3az Energy Efficient Ethernet */
#if 0
	retVal = rtk_eee_init();
	printk("rtk_eee_init(): return %d\n", retVal);
	retVal = rtk_eee_portEnable_set(0, ENABLED);
	printk("rtk_eee_portEnable_set(0, ENABLED): return %d\n", retVal);
	retVal = rtk_eee_portEnable_set(1, ENABLED);
	printk("rtk_eee_portEnable_set(1, ENABLED): return %d\n", retVal);
	retVal = rtk_eee_portEnable_set(2, ENABLED);
	printk("rtk_eee_portEnable_set(2, ENABLED): return %d\n", retVal);
	retVal = rtk_eee_portEnable_set(3, ENABLED);
	printk("rtk_eee_portEnable_set(3, ENABLED): return %d\n", retVal);
	retVal = rtk_eee_portEnable_set(4, ENABLED);
	printk("rtk_eee_portEnable_set(4, ENABLED): return %d\n", retVal);
#endif

	/* Storm Filtering Control */
#if 0
	rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_UNICAST, 16384, 1, 0);
	rtk_storm_controlRate_set(0, STORM_GROUP_UNKNOWN_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(0, STORM_GROUP_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(0, STORM_GROUP_BROADCAST, 16384, 1, 0);

	rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_UNICAST, 16384, 1, 0);
	rtk_storm_controlRate_set(1, STORM_GROUP_UNKNOWN_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(1, STORM_GROUP_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(1, STORM_GROUP_BROADCAST, 16384, 1, 0);

	rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_UNICAST, 16384, 1, 0);
	rtk_storm_controlRate_set(2, STORM_GROUP_UNKNOWN_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(2, STORM_GROUP_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(2, STORM_GROUP_BROADCAST, 16384, 1, 0);

	rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_UNICAST, 16384, 1, 0);
	rtk_storm_controlRate_set(3, STORM_GROUP_UNKNOWN_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(3, STORM_GROUP_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(3, STORM_GROUP_BROADCAST, 16384, 1, 0);

	rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_UNICAST, 16384, 1, 0);
	rtk_storm_controlRate_set(4, STORM_GROUP_UNKNOWN_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(4, STORM_GROUP_MULTICAST, 20000, 1, 0);
	rtk_storm_controlRate_set(4, STORM_GROUP_BROADCAST, 16384, 1, 0);
#endif
	LANWANPartition();

	printk("RTL8367M driver initialized\n");

	return 0;
}

void __exit rtl8367m_exit(void)
{
	unregister_chrdev(rtl8367m_major, RTL8367M_DEVNAME);

	printk("RTL8367M driver exited\n");
}

module_init(rtl8367m_init);
module_exit(rtl8367m_exit);
