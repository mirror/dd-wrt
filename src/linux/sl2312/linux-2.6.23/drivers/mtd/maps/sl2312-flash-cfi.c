/*======================================================================

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
======================================================================*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/string.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/sl2312.h>
#include <linux/mtd/kvctl.h>
#include "sl2312_flashmap.h"
#include <linux/squashfs_fs.h>


//extern int parse_afs_partitions(struct mtd_info *, struct mtd_partition **);

/* the base address of FLASH control register */
#define FLASH_CONTROL_BASE_ADDR	    (IO_ADDRESS(SL2312_FLASH_CTRL_BASE))
#define SL2312_GLOBAL_BASE_ADDR     (IO_ADDRESS(SL2312_GLOBAL_BASE))

/* define read/write register utility */
#define FLASH_READ_REG(offset)			(__raw_readl(offset+FLASH_CONTROL_BASE_ADDR))
#define FLASH_WRITE_REG(offset,val) 	(__raw_writel(val,offset+FLASH_CONTROL_BASE_ADDR))

/* the offset of FLASH control register */
enum EMAC_REGISTER {
	FLASH_ID     	= 0x0000,
	FLASH_STATUS 	= 0x0008,
	FLASH_TYPE   	= 0x000c,
	FLASH_ACCESS	= 0x0020,
	FLASH_ADDRESS   = 0x0024,
	FLASH_DATA		= 0x0028,
	FLASH_TIMING    = 0x002c,
};

//#define FLASH_BASE	FLASH_CONTROL_BASE_ADDR
//#define FLASH_SIZE	0x00800000 //INTEGRATOR_FLASH_SIZE

//#define FLASH_PART_SIZE 8388608

static unsigned int flash_indirect_access = 0;

#ifdef CONFIG_SL2312_SHARE_PIN
static unsigned int chip_en = 0x00000000;

void sl2312flash_enable_parallel_flash(void)
{
    unsigned int    reg_val;

    reg_val = readl(SL2312_GLOBAL_BASE_ADDR + 0x30);
    reg_val = reg_val & 0xfffffffd;
    writel(reg_val,SL2312_GLOBAL_BASE_ADDR + 0x30);
    return;
}

void sl2312flash_disable_parallel_flash(void)
{
    unsigned int    reg_val;

    reg_val = readl(SL2312_GLOBAL_BASE_ADDR + 0x30);
    reg_val = reg_val | 0x00000002;
    writel(reg_val,SL2312_GLOBAL_BASE_ADDR + 0x30);
    return;
}
#endif


static struct map_info sl2312flash_map =
{
	name:		"SL2312 CFI Flash",
	size:       FLASH_SIZE,
	bankwidth:   2,
	//bankwidth:   1, //for 8 bits width
    phys:       SL2312_FLASH_BASE,
};

static struct mtd_info *mtd;
#if 0
static struct mtd_partition sl2312_partitions[] = {
	/* boot code */
	{
		name: "bootloader",
		offset: 0x00000000,
		size: 0x20000,
//		mask_flags: MTD_WRITEABLE,
	},
	/* kernel image */
	{
		name: "kerel image",
		offset: 0x00020000,
		size: 0x2E0000
	},
	/* All else is writable (e.g. JFFS) */
	{
		name: "user data",
		offset: 0x00300000,
		size: 0x00100000,
	}
};
#endif



static int __init sl2312flash_init(void)
{
	struct mtd_partition *parts;
	int nr_parts = 0;
	int ret;
	char *buf;
	unsigned char *p;
	int offset=0;
#ifndef CONFIG_SL2312_SHARE_PIN
    unsigned int    reg_val;
#endif

    printk("SL2312 MTD Driver Init.......\n");

#ifndef CONFIG_SL2312_SHARE_PIN
	/* enable flash */
    reg_val = readl(SL2312_GLOBAL_BASE_ADDR + 0x30);
    reg_val = reg_val & 0xfffffffd;
    writel(reg_val,SL2312_GLOBAL_BASE_ADDR + 0x30);
#else
    sl2312flash_enable_parallel_flash();      /* enable Parallel FLASH */
#endif
    FLASH_WRITE_REG(FLASH_ACCESS,0x00004000); /* parallel flash direct access mode */
    ret = FLASH_READ_REG(FLASH_ACCESS);
    if (ret == 0x00004000)
    {
        flash_indirect_access = 0;  /* parallel flash direct access */
    }
    else
    {
        flash_indirect_access = 1;  /* parallel flash indirect access */
    }

	/*
	 * Also, the CFI layer automatically works out what size
	 * of chips we have, and does the necessary identification
	 * for us automatically.
	 */
#ifdef CONFIG_GEMINI_IPI
	sl2312flash_map.virt = FLASH_VBASE;//(unsigned int *)ioremap(SL2312_FLASH_BASE, FLASH_SIZE);
#else
	sl2312flash_map.virt = (unsigned int *)ioremap(SL2312_FLASH_BASE, FLASH_SIZE);
#endif
	//printk("sl2312flash_map.virt  = %08x\n",(unsigned int)sl2312flash_map.virt);

//	simple_map_init(&sl2312flash_map);

    printk("probe cfi\n");
	mtd = do_map_probe("cfi_probe", &sl2312flash_map);
	if (!mtd)
	{
#ifdef CONFIG_SL2312_SHARE_PIN
        sl2312flash_disable_parallel_flash();      /* disable Parallel FLASH */
#endif
		return -ENXIO;
	}
	mtd->owner = THIS_MODULE;
//    mtd->erase = flash_erase;
//    mtd->read = flash_read;
//    mtd->write = flash_write;

    printk("scan for squashfs\n");
       parts = sl2312_partitions;
	nr_parts = sizeof(sl2312_partitions)/sizeof(*parts);
	buf = sl2312flash_map.virt;
	int erasesize = mtd->erasesize;
	int filesyssize=0;
	int tmplen=0;
	while((offset+erasesize)<mtd->size)
	    {
	    printk(KERN_EMERG "[0x%08X]\n",offset);
	    if (*((__u32 *) buf) == SQUASHFS_MAGIC) 
		{
	        struct squashfs_super_block *sb = (struct squashfs_super_block *) buf;
		filesyssize = sb->bytes_used;
		tmplen = offset + filesyssize;
		tmplen +=  (erasesize - 1);
		tmplen &= ~(erasesize - 1);
		filesyssize = tmplen - offset;
		parts[2].size = filesyssize;
		parts[2].offset = offset;
		parts[3].offset = offset+filesyssize;
		parts[3].size = (parts[1].offset+parts[1].size)-parts[3].offset;
		break;
		}
	    offset+=erasesize;
	    buf+=erasesize;
	    }

	ret = add_mtd_partitions(mtd, parts, nr_parts);
	/*If we got an error, free all resources.*/
	if (ret < 0) {
		del_mtd_partitions(mtd);
		map_destroy(mtd);
	}
#ifdef CONFIG_SL2312_SHARE_PIN
    sl2312flash_disable_parallel_flash();      /* disable Parallel FLASH */
#endif
    printk("SL2312 MTD Driver Init Success ......\n");
	return ret;
}

static void __exit sl2312flash_exit(void)
{
	if (mtd) {
		del_mtd_partitions(mtd);
		map_destroy(mtd);
	}

	if (sl2312flash_map.virt) {
	    iounmap((void *)sl2312flash_map.virt);
	    sl2312flash_map.virt = 0;
	}
}

char chrtohex(char c)
{
  char val;
  if ((c >= '0') && (c <= '9'))
  {
    val = c - '0';
    return val;
  }
  else if ((c >= 'a') && (c <= 'f'))
  {
    val = 10 + (c - 'a');
    return val;
  }
  else if ((c >= 'A') && (c <= 'F'))
  {
    val = 10 + (c - 'A');
    return val;
  }
  printk("<1>Error number\n");
  return 0;
}


int get_vlaninfo(vlaninfo* vlan)
{
	vctl_mheader head;
	vctl_entry entry;
	struct mtd_info *mymtd=NULL;
	int i, j, loc = 0;
	char *payload=0, *tmp1, *tmp2, tmp3[9];
	size_t retlen;

	#ifdef CONFIG_SL2312_SHARE_PIN
	sl2312flash_enable_parallel_flash();
	#endif
	for(i=0;i<MAX_MTD_DEVICES;i++)
	{
		mymtd=get_mtd_device(NULL,i);
		//    printk("mymtd->name: %s\n", mymtd->name);
		if(mymtd && !strcmp(mymtd->name,"VCTL"))
		{
			//      printk("%s\n", mymtd->name);
			break;
		}
	}
	if( i >= MAX_MTD_DEVICES)
	{
		printk("Can't find version control\n");
		#ifdef CONFIG_SL2312_SHARE_PIN
		sl2312flash_disable_parallel_flash();
		#endif
		return 0;
	}

	if (!mymtd | !mymtd->read)
	{
		printk("<1>Can't read Version Configuration\n");
		#ifdef CONFIG_SL2312_SHARE_PIN
		sl2312flash_disable_parallel_flash();
		#endif
		return 0;
	}

	mymtd->read(mymtd, 0, VCTL_HEAD_SIZE, &retlen, (u_char*)&head);
	//  printk("entry header: %c%c%c%c\n", head.header[0], head.header[1], head.header[2], head.header[3]);
	//  printk("entry number: %x\n", head.entry_num);
	if ( strncmp(head.header, "FLFM", 4) )
	{
		printk("VCTL is a erase block\n");
		#ifdef CONFIG_SL2312_SHARE_PIN
		sl2312flash_disable_parallel_flash();
		#endif
		return 0;
	}
	loc += retlen;
	for (i = 0; i < head.entry_num; i++)
	{
		mymtd->read(mymtd, loc, VCTL_ENTRY_LEN, &retlen, (u_char*)&entry);
		//    printk("type: %x\n", entry.type);
		//    printk("size: %x\n", entry.size);
		strncpy(tmp3, entry.header, 4);
		if (entry.type == VCT_VLAN)
		{
			for (j = 0; j < 6 ; j++)
			{
				vlan[0].mac[j] = 0;
				vlan[1].mac[j] = 0;
			}
			vlan[0].vlanid = 1;
			vlan[1].vlanid = 2;
			vlan[0].vlanmap = 0x7F;
			vlan[1].vlanmap = 0x80;

			payload = (char *)kmalloc(entry.size - VCTL_ENTRY_LEN, GFP_KERNEL);
			loc += VCTL_ENTRY_LEN;
			mymtd->read(mymtd, loc, entry.size - VCTL_ENTRY_LEN, &retlen, payload);
			//      printk("%s\n", payload);
			tmp1 = strstr(payload, "MAC1:");
			tmp2 = strstr(payload, "MAC2:");
			if(!tmp1||!tmp2){
				kfree(payload);
				#ifdef CONFIG_SL2312_SHARE_PIN
				sl2312flash_disable_parallel_flash();
				#endif
				printk("Error VCTL format!!\n");
				return 0;
			}
			tmp1 += 7;
			tmp2 += 7;


			for (j = 0; j < 6; j++)
			{
				vlan[0].mac[j] = chrtohex(tmp1[2*j])*16 + chrtohex(tmp1[(2*j)+1]);
				vlan[1].mac[j] = chrtohex(tmp2[2*j])*16 + chrtohex(tmp2[(2*j)+1]);
			}
			tmp1 = strstr(payload, "ID1:");
			tmp2 = strstr(payload, "ID2:");
			tmp1 += 4;
			tmp2 += 4;
			vlan[0].vlanid = tmp1[0] - '0';
			vlan[1].vlanid = tmp2[0] - '0';
			tmp1 = strstr(payload, "MAP1:");
			tmp2 = strstr(payload, "MAP2:");
			tmp1 += 7;
			tmp2 += 7;
			vlan[0].vlanmap = chrtohex(tmp1[0]) * 16 + chrtohex(tmp1[1]);
			vlan[1].vlanmap = chrtohex(tmp2[0]) * 16 + chrtohex(tmp2[1]);
			//  printk("Vlan1 id:%x map:%02x mac:%x%x%x%x%x%x\n", vlan[0].vlanid, vlan[0].vlanmap, vlan[0].mac[0], vlan[0].mac[1], vlan[0].mac[2], vlan[0].mac[3], vlan[0].mac[4], vlan[0].mac[5]);
			//  printk("Vlan2 id:%x map:%02x mac:%x%x%x%x%x%x\n", vlan[1].vlanid, vlan[1].vlanmap, vlan[1].mac[0], vlan[1].mac[1], vlan[1].mac[2], vlan[1].mac[3], vlan[1].mac[4], vlan[1].mac[5]);
			break;
		}
		loc += entry.size;
	}
	if ( entry.type == VCT_VLAN )
	{
		#ifdef CONFIG_SL2312_SHARE_PIN
		sl2312flash_disable_parallel_flash();
		#endif
		kfree(payload);
		return 1;
	}
	if (i >= head.entry_num)
	printk("Can't find vlan information\n");
	#ifdef CONFIG_SL2312_SHARE_PIN
	sl2312flash_disable_parallel_flash();
	#endif
	return 0;
}

EXPORT_SYMBOL(get_vlaninfo);


module_init(sl2312flash_init);
module_exit(sl2312flash_exit);

MODULE_AUTHOR("Storlink Ltd");
MODULE_DESCRIPTION("CFI map driver");
MODULE_LICENSE("GPL");
