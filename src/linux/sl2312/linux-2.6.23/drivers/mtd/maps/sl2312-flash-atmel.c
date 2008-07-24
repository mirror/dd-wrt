/*
 * $Id: sl2312-flash-atmel.c,v 1.2 2006/06/05 02:35:57 middle Exp $
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


#define  g_page_addr  AT45DB321_PAGE_SHIFT    //321 : shift 10  ; 642 : shift 11
#define  g_chipen     SERIAL_FLASH_CHIP0_EN   //atmel

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);

void address_to_page(__u32 address, __u16 *page, __u16 *offset)
{
    *page = address / SPAGE_SIZE;
    *offset = address % SPAGE_SIZE;
}

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

void atmel_read_status(__u8 cmd, __u8 *data)
{
      __u32 opcode;
      __u32 value;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_OPCODE_DATA | cmd | g_chipen;
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }

      value=read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      *data = value & 0xff;
}

void main_memory_page_read(__u8 cmd, __u16 page, __u16 offset, __u8 *data)
{
      __u32 opcode;
      __u32 address;
      __u32 value;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS_4X_DATA | cmd | g_chipen;
      address = (page << g_page_addr) + offset;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }

      value=read_flash_ctrl_reg(FLASH_READ_DATA_OFFSET);
      *data = value & 0xff;
}

void buffer_to_main_memory(__u8 cmd, __u16 page)
{
      __u32 opcode;
      __u32 address;
      __u8  status;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | cmd | g_chipen;
      address = (page << g_page_addr);
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      atmel_read_status(READ_STATUS_SPI, &status);
      while(!(status&0x80))
      {
          atmel_read_status(READ_STATUS_SPI, &status);
          flash_delay();
          schedule();
      }

}


void atmel_flash_read_page(__u32 address, __u8 *buffer, __u32 len)
{
    __u8  byte;
    __u16 page, offset;
    __u16 i;

    address_to_page(address, &page, &offset);

     for(i=0; i<len; i++,offset++)
    {
        main_memory_page_read(MAIN_MEMORY_PAGE_READ_SPI , page, offset, &byte);
        buffer [i]= byte;
    }
}

void atmel_flash_program_page(__u32 address, __u8 *buffer, __u32 len)
{
    __u8  pattern;
    __u16 page, offset;
    __u32 i;

    address_to_page(address, &page, &offset);
 //   printk("atmel_flash_program_page: offset %x len %x page %x \n", offset, len, page);

    if(offset)
 	    main_memory_to_buffer(MAIN_MEMORY_TO_BUFFER1,page);

    for(i=0; i<len; i++,offset++)
    {
        pattern = buffer[i];
        atmel_buffer_write(BUFFER1_WRITE,offset,pattern);
    }

  //  printk("atmel_flash_program_page: offset %x \n", offset);
    buffer_to_main_memory(BUFFER1_TO_MAIN_MEMORY, page);
  //  printk("atmel_flash_program_page: buffer_to_main_memory %x page\n", page);

}


void main_memory_to_buffer(__u8 cmd, __u16 page)
{
      __u32 opcode;
      __u32 address;
      __u8  status;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | cmd | g_chipen;
      address = (page << g_page_addr);
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      atmel_read_status(READ_STATUS_SPI, &status);
      while(!(status&0x80))
      {
          atmel_read_status(READ_STATUS_SPI, &status);
          flash_delay();
          schedule();
      }

}

void main_memory_page_program(__u8 cmd, __u16 page, __u16 offset, __u8 data)
{
      __u32 opcode;
      __u32 address;
      __u8  status;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS_DATA | cmd | g_chipen;
      address = (page << g_page_addr) + offset;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_WRITE_DATA_OFFSET, data);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      atmel_read_status(READ_STATUS_SPI, &status);
      while(!(status&0x80))
      {
          atmel_read_status(READ_STATUS_SPI, &status);
          flash_delay();
          schedule();
      }
}

void atmel_buffer_write(__u8 cmd, __u16 offset, __u8 data)
{
      __u32 opcode;
      __u32 address;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS_DATA | cmd  | g_chipen;
      address = offset;
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_WRITE_DATA_OFFSET, data);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }

}

void atmel_erase_page(__u8 cmd, __u16 page)
{
      __u32 opcode;
      __u32 address;
      __u8  status;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | cmd | g_chipen;
      address = (page << g_page_addr);
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      atmel_read_status(READ_STATUS_SPI, &status);
      while(!(status&0x80))
      {
          atmel_read_status(READ_STATUS_SPI, &status);
          flash_delay();
          schedule();
      }

}

void atmel_erase_block(__u8 cmd, __u16 block)
{
      __u32 opcode;
      __u32 address;
      __u8  status;

      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | cmd | g_chipen;
      address = (block << 13);
      write_flash_ctrl_reg(FLASH_ADDRESS_OFFSET, address);
      write_flash_ctrl_reg(FLASH_ACCESS_OFFSET, opcode);
      opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode=read_flash_ctrl_reg(FLASH_ACCESS_OFFSET);
          flash_delay();
          schedule();
      }
      atmel_read_status(READ_STATUS_SPI, &status);
      while(!(status&0x80))
      {
          atmel_read_status(READ_STATUS_SPI, &status);
          flash_delay();
          schedule();
      }

}

void flash_delay(void)
{
      int i;

      for(i=0; i<50; i++)
           i=i;
}




__u32 sl2312_read32(struct map_info *map, unsigned long ofs)
{

#if 0
    __u16 page, offset;
    __u32 pattern;
    __u8  byte, i;

     pattern = 0;
     address_to_page(ofs, &page, &offset);
     for(i=0; i<4; i++, offset++)
    {
        pattern = pattern << 8;
        main_memory_page_read(MAIN_MEMORY_PAGE_READ_SPI , page, offset, &byte);
//printk("sl2312_read32:: address = %08x  data = %c \n",ofs,byte);
        pattern += byte;
    }
    return pattern;
#else
      return read_flash_ctrl_reg(ofs);
#endif

}

__u8 sl2312_read8(struct map_info *map, unsigned long ofs)
{
    __u16 page, offset;
    __u8  byte;

     address_to_page(ofs, &page, &offset);
     main_memory_page_read(MAIN_MEMORY_PAGE_READ_SPI , page, offset, &byte);
	 //printk("sl2312_read8:: address = %08x  data = %c \n",ofs,byte);
     return byte;

}

void sl2312_write32(struct map_info *map, __u32 d, unsigned long ofs)
{
#if 0
    __u16 page, offset;
    __u8  byte, i;

     address_to_page(ofs, &page, &offset);
     for(i=0; i<4; i++, offset++)
    {
    	byte = d & 0xff;
        main_memory_page_program(MAIN_MEMORY_PROGRAM_BUFFER1, page, offset, byte);
        d = d >> 8;
//printk("sl2312_write32:: address = %08x  data = %c \n",ofs,byte);
    }
#else
      write_flash_ctrl_reg(ofs, d);
#endif
}

void sl2312_write8(struct map_info *map, __u8 d, unsigned long ofs)
{
     __u16 page, offset;

     address_to_page(ofs, &page, &offset);
     main_memory_page_program(MAIN_MEMORY_PROGRAM_BUFFER1, page, offset, d);
//printk("sl2312_write8:: address = %08x  data = %c \n",ofs,d);

}

void sl2312_copy_from(struct map_info *map, void *buf, unsigned long ofs, ssize_t len)
{
     __u32 size;
     __u8  *buffer;
     __u32 length;//i, j,

     //printk("sl2312_copy_from:: address = %08x  datalen = %d \n",ofs,len);

     length = len;
     buffer = (__u8 *)buf;
     while(len)
     {
        size = SPAGE_SIZE - (ofs%SPAGE_SIZE);
        if(size > len)
            size = len;
        atmel_flash_read_page(ofs, buffer, size);
        buffer+=size;
        ofs+=size;
        len -= size;
     }

#if 0
        buffer = (__u8 *)buf;
        for(i=0; i<length; i+=16)
       {
          for(j=0; j<16; j++,buffer++)
         {
            if((i*16+j)<length)
              printk("%x  ",(int)*buffer);
	 }
          printk("\n");
       }

       printk("\n");
#endif

}


void sl2312_copy_to(struct map_info *map, unsigned long ofs, void *buf, ssize_t len)
{
     __u32 size;
     __u8  *buffer;

     buffer = (__u8 *)buf;
     //printk("sl2312_copy_to:offset %x len %x \n", ofs, len);
//     printk("sl2312_copy_to:buf is %x \n", (int)buf);

     while(len)
     {
        size = SPAGE_SIZE - (ofs%SPAGE_SIZE);
        if(size > len)
            size = len;
        atmel_flash_program_page(ofs, buffer, size);
        buffer+=size;
        ofs+=size;
	len-=size;
    }


}


static struct mtd_info *serial_mtd;

static struct mtd_partition *parsed_parts;

static struct map_info sl2312_serial_map = {
//	name: "SL2312 serial flash",
//	size: 4194304, //0x400000,
//		//buswidth: 4,
//	bankwidth: 4,
//	phys:		 SL2312_FLASH_BASE,
//#ifdef CONFIG_MTD_COMPLEX_MAPPINGS
//	//read32: sl2312_read32,
//	//read8: sl2312_read8,
//	copy_from: sl2312_copy_from,
//	//write8: sl2312_write8,
//	//write32: sl2312_write32,
//	read: sl2312_read32,
//	write: sl2312_write32,
//	copy_to: sl2312_copy_to
//#endif
	.name = "SL2312 serial flash",
	.size = 4194304, //0x400000,
		//buswidth: 4,
	.bankwidth = 4,
	.phys =		 SL2312_FLASH_BASE,
#ifdef CONFIG_MTD_COMPLEX_MAPPINGS
	//read32: sl2312_read32,
	//read8: sl2312_read8,
	.copy_from = sl2312_copy_from,
	//write8: sl2312_write8,
	//write32: sl2312_write32,
	.read = sl2312_read32,
	.write = sl2312_write32,
	.copy_to = sl2312_copy_to
#endif
};



static struct mtd_partition sl2312_partitions[] = {


	///* boot code */
	//{ name: "bootloader", offset: 0x00000000, size: 0x20000, },
	///* kernel image */
	//{ name: "kerel image", offset: 0x000020000, size: 0x2E0000 },
	///* All else is writable (e.g. JFFS) */
	//{ name: "user data", offset: 0x00300000, size: 0x00100000, },
	/* boot code */
	{ .name = "bootloader", .offset = 0x00000000, .size = 0x20000, },
	/* kernel image */
	{ .name = "kerel image", .offset = 0x000020000, .size = 0xE0000 },
	/* All else is writable (e.g. JFFS) */
	{ .name = "user data", .offset = 0x00100000, .size = 0x00300000, },


};



static int __init init_sl2312_maps(void)
{
	int nr_parts = 0;
	struct mtd_partition *parts;

	serial_mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!serial_mtd)
		return NULL;

	memset(serial_mtd, 0, sizeof(struct mtd_info));
	//sl2312flash_map.virt = (unsigned long)ioremap(SL2312_FLASH_BASE, FLASH_SIZE);
    //sl2312_serial_map.map_priv_1 = (unsigned long)ioremap(SL2312_FLASH_BASE, SFLASH_SIZE);//(unsigned long)FLASH_VBASE;
    sl2312_serial_map.virt = (unsigned long)ioremap(SL2312_FLASH_BASE, SFLASH_SIZE);//(unsigned long)ioremap(FLASH_START, SFLASH_SIZE);
    if (!sl2312_serial_map.virt) {
		printk(" failed to ioremap \n");
		return -EIO;
	}
	serial_mtd = do_map_probe("map_serial", &sl2312_serial_map);
	if (serial_mtd) {
		//serial_mtd->module = THIS_MODULE;
		serial_mtd->owner = THIS_MODULE;

	}

#ifdef CONFIG_MTD_REDBOOT_PARTS
	nr_parts = parse_redboot_partitions(serial_mtd, &parsed_parts);
	if (nr_parts > 0)
		printk(KERN_NOTICE "Found RedBoot partition table.\n");
	else if (nr_parts < 0)
		printk(KERN_NOTICE "Error looking for RedBoot partitions.\n");
#else
	parsed_parts = sl2312_partitions;
	parts = sl2312_partitions;
	nr_parts = sizeof(sl2312_partitions)/sizeof(*parts);
	nr_parts = sizeof(sl2312_partitions)/sizeof(*parsed_parts);
#endif /* CONFIG_MTD_REDBOOT_PARTS */

	if (nr_parts > 0)
	    add_mtd_partitions(serial_mtd, parsed_parts, nr_parts);
	else
	    add_mtd_device(serial_mtd);

	return 0;
}

static void __exit cleanup_sl2312_maps(void)
{
	if (parsed_parts)
	    del_mtd_partitions(serial_mtd);
	else
	    del_mtd_device(serial_mtd);

	map_destroy(serial_mtd);


}

module_init(init_sl2312_maps);
module_exit(cleanup_sl2312_maps);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Plus Chen <plus@storlink.com.tw>");
MODULE_DESCRIPTION("MTD map driver for Storlink Sword boards");

