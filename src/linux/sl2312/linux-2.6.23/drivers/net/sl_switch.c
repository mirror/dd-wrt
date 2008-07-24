#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/hardware.h>
#include <asm/io.h>

#define GMAC_GLOBAL_BASE_ADDR       (IO_ADDRESS(SL2312_GLOBAL_BASE))
#define GPIO_BASE_ADDR1  (IO_ADDRESS(SL2312_GPIO_BASE1))
enum GPIO_REG
{
    GPIO_DATA_OUT   = 0x00,
    GPIO_DATA_IN    = 0x04,
    GPIO_PIN_DIR    = 0x08,
    GPIO_BY_PASS    = 0x0c,
    GPIO_DATA_SET   = 0x10,
    GPIO_DATA_CLEAR = 0x14,
};

#define GMAC_SPEED_10			0
#define GMAC_SPEED_100			1
#define GMAC_SPEED_1000			2

enum phy_state
{
    LINK_DOWN   = 0,
    LINK_UP     = 1
};

#ifndef BIT
#define BIT(x)						(1 << (x))
#endif

//int Get_Set_port_status();
unsigned int SPI_read_bit(void);
void SPI_write_bit(char bit_EEDO);
void SPI_write(unsigned char block,unsigned char subblock,unsigned char addr,unsigned int value);
unsigned int SPI_read(unsigned char block,unsigned char subblock,unsigned char addr);
int SPI_default(void);
void SPI_CS_enable(unsigned char enable);
unsigned int SPI_get_identifier(void);
void phy_write(unsigned char port_no,unsigned char reg,unsigned int val);
unsigned int phy_read(unsigned char port_no,unsigned char reg);
void phy_write_masked(unsigned char port_no,unsigned char reg,unsigned int val,unsigned int mask);
void init_seq_7385(unsigned char port_no) ;
void phy_receiver_init (unsigned char port_no);

#define PORT_NO		4
int switch_pre_speed[PORT_NO]={0,0,0,0};
int switch_pre_link[PORT_NO]={0,0,0,0};





/*				NOTES
 *   The Protocol of the SPI are as follows:
 *
 *     		   Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
 *	byte0     |   Block id  | r/w | sub-block        |
 *	byte1     |		Address			 |
 *	byte2	  |		Data			 |
 *	byte3	  |		Data			 |
 *	byte4	  |		Data			 |
 *	byte5	  |		Data			 |
 */




/***************************************/
/* define GPIO module base address     */
/***************************************/
#define GPIO_EECS	     0x80000000		/*   EECS: GPIO[22]   */
#define GPIO_MOSI	     0x20000000         /*   EEDO: GPIO[29]   send to 6996*/
#define GPIO_MISO	     0x40000000         /*   EEDI: GPIO[30]   receive from 6996*/
#define GPIO_EECK	     0x10000000         /*   EECK: GPIO[31]   */

/*************************************************************
* SPI protocol for ADM6996 control
**************************************************************/
#define SPI_OP_LEN	     0x08		// the length of start bit and opcode
#define SPI_OPWRITE	     0X05		// write
#define SPI_OPREAD	     0X06		// read
#define SPI_OPERASE	     0X07		// erase
#define SPI_OPWTEN	     0X04		// write enable
#define SPI_OPWTDIS	     0X04		// write disable
#define SPI_OPERSALL	     0X04		// erase all
#define SPI_OPWTALL	     0X04		// write all

#define SPI_ADD_LEN	     8			// bits of Address
#define SPI_DAT_LEN	     32			// bits of Data


/****************************************/
/*	Function Declare		*/
/****************************************/

//unsigned int SPI_read_bit(void);
//void SPI_write_bit(char bit_EEDO);
//unsigned int SPI_read_bit(void);
/******************************************
* SPI_write
* addr -> Write Address
* value -> value to be write
***************************************** */
void phy_receiver_init (unsigned char port_no)
{
    phy_write(port_no,31,0x2a30);
    phy_write_masked(port_no, 12, 0x0200, 0x0300);
    phy_write(port_no,31,0);
}

void phy_write(unsigned char port_no,unsigned char reg,unsigned int val)
{
	unsigned int cmd;

	cmd = (port_no<<21)|(reg<<16)|val;
	SPI_write(3,0,1,cmd);
}

unsigned int phy_read(unsigned char port_no,unsigned char reg)
{
	unsigned int cmd,reg_val;

	cmd = BIT(26)|(port_no<<21)|(reg<<16);
	SPI_write(3,0,1,cmd);
	msleep(2);
	reg_val = SPI_read(3,0,2);
	return reg_val;
}

void phy_write_masked(unsigned char port_no,unsigned char reg,unsigned int val,unsigned int mask)
{
	unsigned int cmd,reg_val;

	cmd = BIT(26)|(port_no<<21)|(reg<<16);	// Read reg_val
	SPI_write(3,0,1,cmd);
	mdelay(2);
	reg_val = SPI_read(3,0,2);
	reg_val &= ~mask;			// Clear masked bit
	reg_val |= (val&mask) ;			// set masked bit ,if true
	cmd = (port_no<<21)|(reg<<16)|reg_val;
	SPI_write(3,0,1,cmd);
}

void init_seq_7385(unsigned char port_no)
{
	unsigned char rev;

	phy_write(port_no, 31, 0x2a30);
	phy_write_masked(port_no, 8, 0x0200, 0x0200);
	phy_write(port_no, 31, 0x52b5);
	phy_write(port_no, 16, 0xb68a);
	phy_write_masked(port_no, 18, 0x0003, 0xff07);
	phy_write_masked(port_no, 17, 0x00a2, 0x00ff);
	phy_write(port_no, 16, 0x968a);
	phy_write(port_no, 31, 0x2a30);
	phy_write_masked(port_no, 8, 0x0000, 0x0200);
	phy_write(port_no, 31, 0x0000); /* Read revision */
	rev = phy_read(port_no, 3) & 0x000f;
	if (rev == 0)
	{
		phy_write(port_no, 31, 0x2a30);
		phy_write_masked(port_no, 8, 0x0200, 0x0200);
		phy_write(port_no, 31, 0x52b5);
		phy_write(port_no, 18, 0x0000);
		phy_write(port_no, 17, 0x0689);
		phy_write(port_no, 16, 0x8f92);
		phy_write(port_no, 31, 0x52B5);
		phy_write(port_no, 18, 0x0000);
		phy_write(port_no, 17, 0x0E35);
		phy_write(port_no, 16, 0x9786);
		phy_write(port_no, 31, 0x2a30);
		phy_write_masked(port_no, 8, 0x0000, 0x0200);
		phy_write(port_no, 23, 0xFF80);
		phy_write(port_no, 23, 0x0000);
	}
	phy_write(port_no, 31, 0x0000);
	phy_write(port_no, 18, 0x0048);
	if (rev == 0)
	{
		phy_write(port_no, 31, 0x2a30);
		phy_write(port_no, 20, 0x6600);
		phy_write(port_no, 31, 0x0000);
		phy_write(port_no, 24, 0xa24e);
	}
	else
	{
		phy_write(port_no, 31, 0x2a30);
		phy_write_masked(port_no, 22, 0x0240, 0x0fc0);
		phy_write_masked(port_no, 20, 0x4000, 0x6000);
		phy_write(port_no, 31, 1);
		phy_write_masked(port_no, 20, 0x6000, 0xe000);
		phy_write(port_no, 31, 0x0000);
	}
}

int Get_Set_port_status()
{
	unsigned int    reg_val,ability,rcv_mask,mac_config;
	int is_link=0;
	int i;

 	rcv_mask = SPI_read(2,0,0x10);			// Receive mask

	for(i=0;i<4;i++){
		reg_val = phy_read(i,1);
		if ((reg_val & 0x0024) == 0x0024) /* link is established and auto_negotiate process completed */
		{
			is_link=1;
			if(switch_pre_link[i]==LINK_DOWN){		// Link Down ==> Link up

				rcv_mask |= BIT(i);			// Enable receive

				reg_val = phy_read(i,10);
				if(reg_val & 0x0c00){
					printk("Port%d:Giga mode\n",i);
//					SPI_write(1,i,0x00,0x300701B1);
					mac_config = 0x00060004|(6<<6);

					SPI_write(1,i,0x00,((mac_config & 0xfffffff8) | 1) | 0x20000030);	// reset port
					mac_config |= (( BIT(i) << 19) | 0x08000000);
					SPI_write(1,i,0x00,mac_config);
					SPI_write(1,i,0x04,0x000300ff);		// flow control

					reg_val = SPI_read(5,0,0x12);
					reg_val &= ~BIT(i);
					SPI_write(5,0,0x12,reg_val);

					reg_val = SPI_read(1,i,0x00);
					reg_val |= 0x10010000;
					SPI_write(1,i,0x00,reg_val);
//					SPI_write(1,i,0x00,0x10070181);
					switch_pre_link[i]=LINK_UP;
					switch_pre_speed[i]=GMAC_SPEED_1000;
				}
				else{
					reg_val = phy_read(i,5);
					ability = (reg_val&0x5e0) >>5;
					if ((ability & 0x0C)) /* 100M */
					{
//						SPI_write(1,i,0x00,0x30050472);
						if((ability&0x08)==0) 		// Half
							mac_config = 0x00040004 |(17<<6);
						else				// Full
							mac_config = 0x00040004 |(17<<6);

						SPI_write(1,i,0x00,((mac_config & 0xfffffff8) | 1) | 0x20000030);	// reset port
						mac_config |= (( BIT(i) << 19) | 0x08000000);
						SPI_write(1,i,0x00,mac_config);
						SPI_write(1,i,0x04,0x000300ff);		// flow control

						reg_val = SPI_read(5,0,0x12);
						reg_val &= ~BIT(i);
						SPI_write(5,0,0x12,reg_val);

						reg_val = SPI_read(1,i,0x00);
						reg_val &= ~0x08000000;
						reg_val |= 0x10010000;
						SPI_write(1,i,0x00,reg_val);
//						SPI_write(1,i,0x00,0x10050442);
						printk("Port%d:100M\n",i);
						switch_pre_link[i]=LINK_UP;
						switch_pre_speed[i]=GMAC_SPEED_100;
					}
					else if((ability & 0x03)) /* 10M */
					{
//						SPI_write(1,i,0x00,0x30050473);
						if((ability&0x2)==0)		// Half
							mac_config = 0x00040004 |(17<<6);
						else				// Full
							mac_config = 0x00040004 |(17<<6);

						SPI_write(1,i,0x00,((mac_config & 0xfffffff8) | 1) | 0x20000030);	// reset port
						mac_config |= (( BIT(i) << 19) | 0x08000000);
						SPI_write(1,i,0x00,mac_config);
						SPI_write(1,i,0x04,0x000300ff);		// flow control

						reg_val = SPI_read(5,0,0x12);
						reg_val &= ~BIT(i);
						SPI_write(5,0,0x12,reg_val);

						reg_val = SPI_read(1,i,0x00);
						reg_val &= ~0x08000000;
						reg_val |= 0x10010000;
						SPI_write(1,i,0x00,reg_val);
//						SPI_write(1,i,0x00,0x10050443);
						printk("Port%d:10M\n",i);
						switch_pre_link[i]=LINK_UP;
						switch_pre_speed[i]=GMAC_SPEED_10;
					}
					else{
						SPI_write(1,i,0x00,0x20000030);
						printk("Port%d:Unknown mode\n",i);
						switch_pre_link[i]=LINK_DOWN;
						switch_pre_speed[i]=GMAC_SPEED_10;
					}
				}
			}
			else{						// Link up ==> Link UP

			}
		}
		else{							// Link Down
			if(switch_pre_link[i]==LINK_UP){
				printk("Port%d:Link Down\n",i);
				//phy_receiver_init(i);
				reg_val = SPI_read(1,i,0);
				reg_val &= ~BIT(16);
				SPI_write(1,i,0x00,reg_val);			// disable RX
				SPI_write(5,0,0x0E,BIT(i));			// dicard packet
				while((SPI_read(5,0,0x0C)&BIT(i))==0)		// wait to be empty
					msleep(1);
				SPI_write(1,i,0x00,0x20000030);			// PORT_RST
				SPI_write(5,0,0x0E,SPI_read(5,0,0x0E) & ~BIT(i));// accept packet

				reg_val = SPI_read(5,0,0x12);
				reg_val |= BIT(i);
				SPI_write(5,0,0x12,reg_val);
			}
			switch_pre_link[i]=LINK_DOWN;
			rcv_mask &= ~BIT(i);			// disable receive
		}
	}

	SPI_write(2,0,0x10,rcv_mask);			// Receive mask
	return is_link;

}
EXPORT_SYMBOL(Get_Set_port_status);

void SPI_write(unsigned char block,unsigned char subblock,unsigned char addr,unsigned int value)
{
	int     i;
	char    bit;
	unsigned int data;

	SPI_CS_enable(1);

	data = (block<<5) | 0x10 | subblock;

	//send write command
	for(i=SPI_OP_LEN-1;i>=0;i--)
	{
		bit = (data>>i)& 0x01;
		SPI_write_bit(bit);
	}

	// send 8 bits address (MSB first, LSB last)
	for(i=SPI_ADD_LEN-1;i>=0;i--)
	{
		bit = (addr>>i)& 0x01;
		SPI_write_bit(bit);
	}
	// send 32 bits data (MSB first, LSB last)
	for(i=SPI_DAT_LEN-1;i>=0;i--)
	{
		bit = (value>>i)& 0x01;
		SPI_write_bit(bit);
	}

	SPI_CS_enable(0);	// CS low

}


/************************************
* SPI_write_bit
* bit_EEDO -> 1 or 0 to be written
************************************/
void SPI_write_bit(char bit_EEDO)
{
	unsigned int addr;
	unsigned int value;

	addr = (GPIO_BASE_ADDR1 + GPIO_PIN_DIR);
	value = readl(addr) |GPIO_EECK |GPIO_MOSI ;   /* set EECK/MISO Pin to output */
	writel(value,addr);
	if(bit_EEDO)
	{
		addr = (GPIO_BASE_ADDR1 + GPIO_DATA_SET);
		writel(GPIO_MOSI,addr); /* set MISO to 1 */

	}
	else
	{
		addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
		writel(GPIO_MOSI,addr); /* set MISO to 0 */
	}
	addr = (GPIO_BASE_ADDR1 + GPIO_DATA_SET);
	writel(GPIO_EECK,addr); /* set EECK to 1 */
	addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
	writel(GPIO_EECK,addr); /* set EECK to 0 */

	//return ;
}

/**********************************************************************
* read a bit from ADM6996 register
***********************************************************************/
unsigned int SPI_read_bit(void) // read data from
{
	unsigned int addr;
	unsigned int value;

	addr = (GPIO_BASE_ADDR1 + GPIO_PIN_DIR);
	value = readl(addr) & (~GPIO_MISO);   // set EECK to output and MISO to input
	writel(value,addr);

	addr =(GPIO_BASE_ADDR1 + GPIO_DATA_SET);
	writel(GPIO_EECK,addr); // set EECK to 1


	addr = (GPIO_BASE_ADDR1 + GPIO_DATA_IN);
	value = readl(addr) ;

	addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
	writel(GPIO_EECK,addr); // set EECK to 0


	value = value >> 30;
	return value ;
}

/******************************************
* SPI_default
* EEPROM content default value
*******************************************/
int SPI_default(void)
{
	int i;
	unsigned reg_val,cmd;

#if 0
	SPI_write(7,0,0x1C,0x01);				// map code space to 0

	reg_val = SPI_read(7,0,0x10);
	reg_val |= 0x0146;
	reg_val &= ~0x0001;
	SPI_write(7,0,0x10,reg_val);				// reset iCPU and enable ext_access
	SPI_write(7,0,0x11,0x0000);				// start address
	for(i=0;i<sizeof(vts_img);i++){
		SPI_write(7,0,0x12,vts_img[i]);			// fill in ROM data
	}
	reg_val |= BIT(0)|BIT(3);
	SPI_write(7,0,0x10,reg_val);				// release iCPU
	SPI_write(7,0,0x10,SPI_read(7,0,0x10)&~BIT(7));				// release iCPU
	return ;
#endif


	for(i=0;i<15;i++){
		if(i!=6 && i!=7)
			SPI_write(3,2,0,0x1010400+i);		// Initial memory
		mdelay(1);
	}

	mdelay(30);

	SPI_write(2,0,0xB0,0x05);			// Clear MAC table
	SPI_write(2,0,0xD0,0x03);			// Clear VLAN

	//for(i=0;i<5;i++)
	SPI_write(1,6,0x19,0x2C);			// Double Data rate

	for(i=0;i<4;i++){
		SPI_write(1,i,0x00,0x30050472);		// MAC configure
		SPI_write(1,i,0x00,0x10050442);		// MAC configure
		SPI_write(1,i,0x10,0x5F4);		// Max length
		SPI_write(1,i,0x04,0x00030000);		// Flow control
		SPI_write(1,i,0xDF,0x00000001);		// Flow control
		SPI_write(1,i,0x08,0x000050c2);		// Flow control mac high
		SPI_write(1,i,0x0C,0x002b00f1);		// Flow control mac low
		SPI_write(1,i,0x6E,BIT(3));		// forward pause frame
	}
	SPI_write(1,i,0x00,0x20000030);			// set port 4 as reset

	SPI_write(1,6,0x00,0x300701B1);			// MAC configure
	SPI_write(1,6,0x00,0x10070181);			// MAC configure
	SPI_write(1,6,0x10,0x5F4);			// Max length
	SPI_write(1,6,0x04,0x00030000);		// Flow control
	SPI_write(1,6,0xDF,0x00000002);		// Flow control
	SPI_write(1,6,0x08,0x000050c2);		// Flow control mac high
	SPI_write(1,6,0x0C,0x002b00f1);		// Flow control mac low
	SPI_write(1,6,0x6E,BIT(3));		// forward pause frame


	//SPI_write(7,0,0x05,0x31);			// MII delay for loader
	//SPI_write(7,0,0x05,0x01);			// MII delay for kernel
	SPI_write(7,0,0x05,0x33);

	SPI_write(2,0,0x10,0x4F);			// Receive mask

	mdelay(50);

	SPI_write(7,0,0x14,0x02);			// Release Reset

	mdelay(3);

	for(i=0;i<4;i++){
		init_seq_7385(i);
		phy_receiver_init(i);
		cmd = BIT(26)|(i<<21)|(0x1B<<16);	// Config LED
		SPI_write(3,0,1,cmd);
		mdelay(10);
		reg_val = SPI_read(3,0,2);
		reg_val &= 0xFF00;
		reg_val |= 0x61;
		cmd = (i<<21)|(0x1B<<16)|reg_val;
		SPI_write(3,0,1,cmd);

		cmd = BIT(26)|(i<<21)|(0x04<<16);	// Pause enable
		SPI_write(3,0,1,cmd);
		mdelay(10);
		reg_val = SPI_read(3,0,2);
		reg_val |= BIT(10)|BIT(11);
		cmd = (i<<21)|(0x04<<16)|reg_val;
		SPI_write(3,0,1,cmd);

		cmd = BIT(26)|(i<<21)|(0x0<<16);	// collision test and re-negotiation
		SPI_write(3,0,1,cmd);
		mdelay(10);
		reg_val = SPI_read(3,0,2);
		reg_val |= BIT(7)|BIT(8)|BIT(9);
		cmd = (i<<21)|(0x0<<16)|reg_val;
		SPI_write(3,0,1,cmd);
	}
	init_seq_7385(i);
	writel(0x5787a7f0,GMAC_GLOBAL_BASE_ADDR+0x1c);//For switch timing
	return 4;		// return port_no
}
EXPORT_SYMBOL(SPI_default);

/***********************************************************
* SPI_CS_enable
* before access ,you have to enable Chip Select. (pull high)
* When fisish, you should pull low !!
*************************************************************/
void SPI_CS_enable(unsigned char enable)
{

	unsigned int addr,value;

	addr = (GPIO_BASE_ADDR1 + GPIO_PIN_DIR);
	value = readl(addr) |GPIO_EECS |GPIO_EECK;   /* set EECS/EECK Pin to output */
	writel(value,addr);

	if(enable)
	{
		addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
		writel(GPIO_EECK,addr); /* set EECK to 0 */	// pull low clk first
		addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
		writel(GPIO_EECS,addr); /* set EECS to 0 */

	}
	else
	{
		addr = (GPIO_BASE_ADDR1 + GPIO_DATA_SET);
		writel(GPIO_EECK,addr); /* set EECK to 1 */	// pull high clk before disable
		writel(GPIO_EECS,addr); /* set EECS to 1 */
	}
}


/************************************************
* SPI_read
* table -> which table to be read: 1/count  0/EEPROM
* addr  -> Address to be read
* return : Value of the register
*************************************************/
unsigned int SPI_read(unsigned char block,unsigned char subblock,unsigned char addr)
{
	int     i;
	char    bit;
	unsigned int data,value=0;

	SPI_CS_enable(1);

	data = (block<<5) | subblock;

	//send write command
	for(i=SPI_OP_LEN-1;i>=0;i--)
	{
		bit = (data>>i)& 0x01;
		SPI_write_bit(bit);
	}

	// send 8 bits address (MSB first, LSB last)
	for(i=SPI_ADD_LEN-1;i>=0;i--)
	{
		bit = (addr>>i)& 0x01;
		SPI_write_bit(bit);
	}

	// dummy read for chip ready
	for(i=0;i<8;i++)
		SPI_read_bit();


	// read 32 bits data (MSB first, LSB last)
	for(i=SPI_DAT_LEN-1;i>=0;i--)
	{
		bit = SPI_read_bit();
		value |= bit<<i;
	}

	SPI_CS_enable(0);	// CS low
	return(value);

}

void pull_low_gpio(unsigned int val)
{

	unsigned int addr,value;

	addr = (GPIO_BASE_ADDR1 + GPIO_DATA_CLEAR);
	writel(val,addr); /* set pin low to save power*/

	addr = (GPIO_BASE_ADDR1 + GPIO_PIN_DIR);
	value = readl(addr) & ~ val;   /* set Pin to input */
	writel(value,addr);

//	value = readl(GMAC_GLOBAL_BASE_ADDR+0x0C);	// reset GPIO1 module(self clear)
//	value |= BIT(21);
//	writel(value,GMAC_GLOBAL_BASE_ADDR+0x0C);
}

unsigned int SPI_get_identifier(void)
{
	unsigned int flag=0;

	SPI_write(7,0,0x01,0x01);
	flag = SPI_read(7,0,0x18);  // chip id
	if((flag & 0x0ffff000)==0x07385000){
		printk("Get VSC-switch ID 0x%08x\n",flag);
		//Giga_switch = 1;;
		return 1;
	}
	else{
		printk("VSC-switch not found\n");
		//Giga_switch = 0;
		pull_low_gpio(GPIO_EECK|GPIO_MOSI|GPIO_MISO|GPIO_EECS); // reduce power consume
		return 0;
	}
}
EXPORT_SYMBOL(SPI_get_identifier);

