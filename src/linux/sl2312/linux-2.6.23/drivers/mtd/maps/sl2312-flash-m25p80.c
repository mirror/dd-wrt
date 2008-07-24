/*
 * $Id: sl2312-flash-m25p80.c,v 1.2 2006/06/02 08:46:02 middle Exp $
 *
 * Flash and EPROM on Hitachi Solution Engine and similar boards.
 *
 * (C) 2001 Red Hat, Inc.
 *
 * GPL'd
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/hardware.h>

#include <asm/arch/sl2312.h>
#include <asm/arch/flash.h>
#include <linux/init.h> //add
#define  g_chipen     SERIAL_FLASH_CHIP0_EN   //ST

//static int m25p80_page_program(__u32 address, __u8 data, __u32 schip_en);
static void m25p80_write_cmd(__u8 cmd, __u32 schip_en);
extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);


static __u32 read_flash_ctrl_reg(__u32 ofs)
{
    __u32 *base;

    base = (__u32 *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE + ofs));
    return __raw_readl(base);
}

static void write_flash_ctrl_reg(__u32 ofs,__u32 data)
{
    __u32 *base;

    base = (__u32 *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE + ofs));
    __raw_writel(data, base);
}

static void m25p80_read(__u32 address, __u8 *data, __u32 schip_en)
{
      __u32 opcode,status;
      __u32 value;

      //opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | M25P80_READ;
      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS_DATA | M25P80_READ;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);

      	opcode|=g_chipen;

      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      status=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(status&0x80000000)
      {
          status=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }

      value=read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      *data = value & 0xff;
}

static int m25p80_page_program(__u32 address, __u8 *data, __u32 schip_en)
{
      __u32 opcode;
      __u32  status;
	  __u32 tmp;
	  int res = FLASH_ERR_OK;
	  //volatile FLASH_DATA_T* data_ptr = (volatile FLASH_DATA_T*) data;
	  opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | M25P80_READ_STATUS;

      	      opcode|=g_chipen;

          write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
          tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
          //middle delay_ms(130);
          status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
          if((status&0x02)==0x02)
      	  {
      	       //middle delay_ms(100);
               m25p80_write_cmd(M25P80_WRITE_DISABLE, schip_en);
          }


      m25p80_write_cmd(M25P80_WRITE_ENABLE, schip_en);
      ////middle delay_ms(10);
      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS_DATA | M25P80_PAGE_PROGRAM;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_WRITE_DATA_OFFSET, *data);

      //status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      //while(status!=data)
      //{
      //    status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      //    //middle delay_ms(10);
      //}

      	opcode|=g_chipen;

      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
      //opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | M25P80_READ_STATUS;

      	opcode|=g_chipen;


      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
      status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      //while(status&0xfd)
      while(status&0x01)
      {
      	  //if((status&0x9c)!=0)
      	  //	printf("  m25p80_page_program	Protect Status = %x\n",status);
      	  write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      	  tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
          status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
          flash_delay();
          schedule();
          //middle delay_ms(50);
      }
      //printf("status = %x, data = %x\n",status,data);
      if((status&0x02)==0x02)
      {
	  //middle delay_ms(100);
          m25p80_write_cmd(M25P80_WRITE_DISABLE, schip_en);
      }
    //};//while (len > 0)
    return res;
}

void m25p80_copy_from(struct map_info *map, void *buf, unsigned long ofs, ssize_t len)
{
//     __u32 size;
     __u8  *buffer;
     __u32 length;//i, j,

	length = len;
     buffer = (__u8 *)buf;
     while(len)
     {
        m25p80_read(ofs, buffer, g_chipen);
        buffer++;
        ofs++;
        len --;
     }	;

}

__u32 m25p80_read32(struct map_info *map, unsigned long ofs)
{

      return read_flash_ctrl_reg(ofs);


}

void m25p80_write32(struct map_info *map, __u32 d, unsigned long ofs)
{

      write_flash_ctrl_reg(ofs, d);

}

void m25p80_copy_to(struct map_info *map, unsigned long ofs, void *buf, ssize_t len)
{
     __u32 size, i, ret;

     while(len > 0)
     {
        if(len >= M25P80_PAGE_SIZE)
			size = M25P80_PAGE_SIZE;
		else
			size = len;

        for(i=0;i<size;i++)
	    {
	    	ret = m25p80_page_program( (ofs+i),  (buf+i),  g_chipen);
	    }
        buf+=M25P80_PAGE_SIZE;
        ofs+=M25P80_PAGE_SIZE;
		len-=M25P80_PAGE_SIZE;

    };


}

static struct mtd_info *serial_mtd;

static struct mtd_partition *parsed_parts;

static struct map_info m25p80_map = {

	.name = "SL2312 serial flash m25p80",
	.size = 1048576, //0x100000,
		//buswidth: 4,
	.bankwidth = 4,
	.phys =		 SL2312_FLASH_BASE,
#ifdef CONFIG_MTD_COMPLEX_MAPPINGS
	.copy_from = m25p80_copy_from,
	.read = m25p80_read32,
	.write = m25p80_write32,
	.copy_to = m25p80_copy_to
#endif
};



static struct mtd_partition m25p80_partitions[] = {

	/* boot code */
	{ .name = "bootloader", .offset = 0x00000000, .size = 0x20000, },
	/* kernel image */
	{ .name = "kerel image", .offset = 0x000020000, .size = 0xC0000 },
	/* All else is writable (e.g. JFFS) */
	{ .name = "user data", .offset = 0x000E0000, .size = 0x00010000, },


};

void flash_delay()
{
	int i,j;
	for(i=0;i<0x100;i++)
		j=i*3+5;
}

int m25p80_sector_erase(__u32 address, __u32 schip_en)
{
      __u32 opcode;
      __u32  status;
      __u32 tmp;
      int res = FLASH_ERR_OK;
	//printf("\n-->m25p80_sector_erase");
	if(address >= FLASH_START)
		address-=FLASH_START;

      m25p80_write_cmd(M25P80_WRITE_ENABLE, schip_en);
      //printf("\n     m25p80_sector_erase : after we-en");
      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | M25P80_SECTOR_ERASE;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      #ifdef MIDWAY_DIAG
      	opcode|=schip_en;
      #endif
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | M25P80_READ_STATUS;
      #ifdef MIDWAY_DIAG
      	opcode|=schip_en;
      #endif

      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
      status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      //while(status&0xfd)
      while(status&0x01)
      {
      	  //if((status&0x9c)!=0)
      	  //	printf("  m25p80_sector_erase	Protect Status = %x\n",status);
      	  write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      	  tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
          status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
          flash_delay();
          schedule();
          //middle delay_ms(50);
      }
      if((status&0x02)==0x02)
      {
      	  //middle delay_ms(100);
          m25p80_write_cmd(M25P80_WRITE_DISABLE, schip_en);
      }
      //printf("\n<--m25p80_sector_erase");
      return res;
}

static void m25p80_write_cmd(__u8 cmd, __u32 schip_en)
{
      __u32 opcode,tmp;
      __u32  status;




      opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE | cmd;

      	opcode|=g_chipen;

      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(tmp&0x80000000)
      {
          tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      //////
      opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | M25P80_READ_STATUS;

      	opcode|=g_chipen;

      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(tmp&0x80000000)
      {
          tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      //middle delay_ms(130);
      status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      //printf("\ncmd =%x  status = %x",cmd,status);
      if(cmd==M25P80_WRITE_ENABLE)
      {
      	//printf("\n**-->enable**  status = %x",status);
      	//middle delay_ms(100);
      	   while((status&0x03) != 2)
      	   {
      	   	//if((status&0x9c)!=0)
      	  	//    printf("	M25P80_WRITE_ENABLE   Protect Status = %x\n",status);

      	   	  write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      	   	  tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    //flash_delay();
      			}
      	       status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      	       //printf("\n**enable**  status = %x",status);
      	       flash_delay();
      	       schedule();
      	       //middle delay_ms(100);
      	   }
      }
      else if(cmd==M25P80_WRITE_DISABLE)
      {
      	   //while((status&0x03) == 2)
      	   //   printf("\n**disable**  status = %x",status);
      	   //middle delay_ms(100);
      	   while((status&0x03) != 0)
      	   {
	       //m25p80_write_status((status&0xfd),schip_en);
      	       write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      	       tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      		while(tmp&0x80000000)
      		{
      		    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      		    flash_delay();
      		    schedule();
      		}
      	       status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      	       //printf("\n**disable**  status = %x",status);
      	       flash_delay();
      	       schedule();
      	       //middle delay_ms(50);
      	   }
      }
      else
      {
      	   //while((status&0x01) !=0)
      	   while((status&0x01) !=0)
      	   {
      	   	  write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      	   	  tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			while(tmp&0x80000000)
      			{
      			    tmp=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      			    flash_delay();
      			    schedule();
      			}
      	       status = read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      	       flash_delay();
      	       schedule();
      	       //middle delay_ms(50);
      	   }
      }
      //////

      //printf("\n<--  status = %x",status);
}

static int __init init_sl2312_m25p80(void)
{
	int nr_parts = 0;
	struct mtd_partition *parts;

	serial_mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!serial_mtd)
		return NULL;

	memset(serial_mtd, 0, sizeof(struct mtd_info));
	m25p80_map.virt = (unsigned long)ioremap(SL2312_FLASH_BASE, SFLASH_SIZE);//(unsigned long)ioremap(FLASH_START, SFLASH_SIZE);
    if (!m25p80_map.virt) {
		printk(" failed to ioremap \n");
		return -EIO;
	}
	serial_mtd = do_map_probe("map_serial", &m25p80_map);
	if (serial_mtd) {
		serial_mtd->owner = THIS_MODULE;

	}

#ifdef CONFIG_MTD_REDBOOT_PARTS
	nr_parts = parse_redboot_partitions(serial_mtd, &parsed_parts);
	if (nr_parts > 0)
		printk(KERN_NOTICE "Found RedBoot partition table.\n");
	else if (nr_parts < 0)
		printk(KERN_NOTICE "Error looking for RedBoot partitions.\n");
#else
	parsed_parts = m25p80_partitions;
	parts = m25p80_partitions;
	nr_parts = sizeof(m25p80_partitions)/sizeof(*parts);
	nr_parts = sizeof(m25p80_partitions)/sizeof(*parsed_parts);
#endif /* CONFIG_MTD_REDBOOT_PARTS */

	if (nr_parts > 0)
	    add_mtd_partitions(serial_mtd, parsed_parts, nr_parts);
	else
	    add_mtd_device(serial_mtd);

	return 0;
}

static void __exit cleanup_sl2312_m25p80(void)
{
	if (parsed_parts)
	    del_mtd_partitions(serial_mtd);
	else
	    del_mtd_device(serial_mtd);

	map_destroy(serial_mtd);


}

module_init(init_sl2312_m25p80);
module_exit(cleanup_sl2312_m25p80);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Plus Chen <plus@storlink.com.tw>");
MODULE_DESCRIPTION("MTD map driver for Storlink Sword boards");

