/*
 *  linux/drivers/spi/chips/ks8995m.c
 *
 *  Copyright (C) 2005 Barnabas Kalman <ba...@sednet.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Driver for Kendin 8995M chip
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-id.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <asm/uaccess.h>

#include "ks8995m.h"

#define DEVICE_NAME	"KS8995M"

#define REG_COUNT		0x80

#define CMD_READ		3
#define CMD_WRITE		2

#define REG_CHIP_ID0	0x00
#define REG_CHIP_ID1	0x01


struct ks8995m_data {
	struct spi_client client;
	struct proc_dir_entry *proc_entry;
};

static struct ks8995m_data *g_d;

static struct spi_driver ks8995m_driver;

static ssize_t 
ks8995m_read (struct file *file, char __user *buf, size_t count,
                            loff_t *offset)
{
	int ret = 0;

//pr_debug("%s: count=%d, offset=%d\n",__FUNCTION__,count,*offset);	

	if( *offset == 0 )
	{
		char *tmp;
		int siz,i;
		u8 rd_data[128];
		u8 rd_cmd[2] = { CMD_READ, REG_CHIP_ID0 };
		struct spi_msg rd[2] = {
			{ 0, SPI_M_WR, .len = 2,   .buf = rd_cmd },
			{ 0, SPI_M_RD, .len = 128, .buf = rd_data }
		};

	    if( count > 8192 )
			count = 8192;

		tmp = kmalloc(count,GFP_KERNEL);
	    if( tmp == NULL )
			return -ENOMEM;
		
		ret = spi_transfer(g_d->client.adapter, rd, 2);
		if (ret != 2) {
			ret = -EFAULT;
			goto done;
		}
	
		for( i = siz = 0; i < 128 && (siz+7) < count; i++ ) {
			siz += sprintf(tmp+siz,"R%02X=%02X\n",i,rd_data[i]);
		}
	
	    *offset = ret = siz;
		ret = copy_to_user(buf,tmp,siz) ? -EFAULT : ret;

done:		
	    kfree(tmp);
	}    
    return ret;
}

static inline u8 h2b( char c )
{
	c = toupper(c);
	return c - (c > '9' ? 55 : 48);
}
																						    
static ssize_t 
ks8995m_write (struct file *file, const char __user *buf,
			     size_t count, loff_t *offset)
{
    int ret, state;
    char *tmp, *p, *pe;
	u8 wr_data[128];
	u8 wr_cmd[2] = { CMD_WRITE, REG_CHIP_ID0 };
	struct spi_msg wr[2] = {
		{ 0, SPI_M_WR, .len = 2, .buf = wr_cmd },
		{ 0, SPI_M_WR, .len = 0, .buf = wr_data }
	};

	if (count > 8192)
		count = 8192;

	tmp = kmalloc(count,GFP_KERNEL);
    if (tmp==NULL)
		return -ENOMEM;
    
    if (copy_from_user(tmp,buf,count)) {
		kfree(tmp);
		return -EFAULT;
	}
	
	state = 0;
	for( p = tmp, pe = tmp+count; p < pe; p++ ) {

		if( *p == '\n' )
		{
			if( state == 3 || state == 4 ) {
pr_debug("WRITE: addr=%02X, count=%d\n",wr[0].buf[1],wr[1].len);
				ret = spi_transfer(g_d->client.adapter, wr, 2);
				wr[0].buf[1] = REG_CHIP_ID0;
				wr[1].len = 0;
			}
		
			state = 0;
			continue;
		}
	
		if( isspace(*p) )
			continue;
		
		switch( state )
		{
		case 0:
			if( *p == 'R' ) state = 1;
			break;

		case 1:
			if( isxdigit(*p) ) {
				wr[0].buf[1] = (wr[0].buf[1] <<4) | h2b(*p);
			}
			else if( *p == '=' ) state = 3;
			else state = 0;
			break;
			
		case 3:
			if( isxdigit(*p) ) {
				wr[1].buf[wr[1].len] = h2b(*p);
				state = 4;
			}
			else state = 0;
			break;

		case 4:
			if( wr[1].len < 128 )
			{
				if( isxdigit(*p) ) {
					wr[1].buf[wr[1].len] = (wr[1].buf[wr[1].len] << 4) | h2b(*p);
					wr[1].len++;
					state = 3;
				}
				else state = 0;
			}
			break;
		}
	}
	if( state == 3 || state == 4 ) {
pr_debug("END-WRITE: addr=%02X, count=%d\n",wr[0].buf[1],wr[1].len);
		ret = spi_transfer(g_d->client.adapter, wr, 2);
	}

    ret = count;
    
    kfree(tmp);
    
    return ret;
}
																									      
static int
ks8995m_ioctl(struct inode *inode, struct file *file,
                unsigned int cmd, unsigned long arg)
{
	switch( cmd ) {
	default:
		;
	}
    return 0;
}    

static int
ks8995m_open(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

    return 0;
}
																								
																								
static int
ks8995m_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return 0;
}

static struct file_operations ks8995m_fops = {
	.owner          = THIS_MODULE,
	.open           = ks8995m_open,
	.release        = ks8995m_release,
	.ioctl          = ks8995m_ioctl,
	.read           = ks8995m_read,
	.write          = ks8995m_write,
	.llseek         = no_llseek,
};

/*
static int
ks8995m_read_info(char *buffer, char **start, off_t offset,
                        int length, int *eof, void *data)
{
	struct ks8995m_data *var = (struct ks8995m_data *)data;

	if( offset == 0 ) {
		int ret;
		u8 rd_data[128];
		u8 rd_cmd[2] = { CMD_READ, REG_CHIP_ID0 };
		struct spi_msg ctrl_rd[2] = {
			{ 0, SPI_M_WR, .len = 2,   .buf = rd_cmd },
			{ 0, SPI_M_RD, .len = 128, .buf = rd_data }
		};

		ret = spi_transfer(var->client.adapter, ctrl_rd, 2);
		if (ret == 2) {
			int i,siz;
			for( i = siz = 0; i < 128 && (siz+7) < length; i++ ) {
				siz += sprintf(buffer+siz,"R%02X:%02X\n",i,rd_data[i]);
			}
			*eof = 1;
			return siz;
		}
	}

    *eof = 1;
	return 0;
}

static int
ks8995m_write_info(struct file *file, const char *buffer,
                         unsigned long length, void *data)
{
	struct ks8995m_data *var = (struct ks8995m_data *)data;

	if (length) {
		int ret;
		u8 wr_data[128];
		u8 wr_cmd[2] = { CMD_WRITE, REG_CHIP_ID0 };
		struct spi_msg ctrl_rd[2] = {
			{ 0, SPI_M_WR, .len = 2,   .buf = rd_cmd },
			{ 0, SPI_M_WR, .len = 128, .buf = rd_data }
		};
	}

	return (int) length;
}
*/

static void
delete_proc( struct ks8995m_data *d )
{
	if(d->proc_entry) {
		remove_proc_entry( DEVICE_NAME, proc_root_driver );
		d->proc_entry = NULL;
	}
}

static int
create_proc( struct ks8995m_data *d )
{
	int ret = 0;
	
	d->proc_entry = create_proc_entry( DEVICE_NAME, 0644, proc_root_driver );
	if( !d->proc_entry ) {
		ret = -EACCES;
		goto done;
	}

	d->proc_entry->owner	= THIS_MODULE;
	d->proc_entry->data		= d;
	d->proc_entry->proc_fops= &ks8995m_fops;
//	d->proc_entry->read_proc  = ks8995m_read_info;
//	d->proc_entry->write_proc = ks8995m_write_info;

	
done:
	if( ret )
		delete_proc( d );
		
	return ret;
}


static int ks8995m_attach(struct spi_adapter *adap)
{
	int ret;
	struct spi_client *new_client;
	struct ks8995m_data *d;
	
	u8 CHIP_ID[2] = {0,0};
	
	u8 rd_cmd[2] = { CMD_READ, REG_CHIP_ID0 };
	struct spi_msg ctrl_rd[2] = {
		{ 0, SPI_M_WR, .len = 2, .buf = rd_cmd },
		{ 0, SPI_M_RD, .len = 2, .buf = CHIP_ID }
	};

	d = kmalloc(sizeof(struct ks8995m_data), GFP_KERNEL);
	if (!d) {
		ret = -ENOMEM;
		goto done;
	}
	g_d = d;
	memset(d, 0, sizeof(struct ks8995m_data));
	new_client = &d->client;

	strlcpy(new_client->name, DEVICE_NAME, SPI_NAME_SIZE);
	spi_set_clientdata(new_client, d);
	new_client->flags = SPI_CLIENT_ALLOW_USE | SPI_DF_NOTIFY;
	new_client->addr = 0;
	new_client->adapter = adap;
	new_client->driver = &ks8995m_driver;

	ret = create_proc( d );
	if( ret )
		goto done;
	
	// read Family ID

	ret = spi_transfer(new_client->adapter, ctrl_rd, 2);
	if (ret != 2) {
		printk(KERN_INFO DEVICE_NAME " not fount! (No response)\n");
		ret = -ENODEV;
		goto done;
	}
	if( CHIP_ID[0] != 0x95 ) {
		printk(KERN_INFO DEVICE_NAME " not fount! (Family ID: %02X)\n",CHIP_ID[0]);
		ret = -ENODEV;
		goto done;
	}
	printk(KERN_INFO DEVICE_NAME ": Family ID: %02X, Chip ID: %02X, Revision ID: %02X\n",CHIP_ID[0],(CHIP_ID[1]>>4)&15,(CHIP_ID[1]>1)&3);

	
	ret = spi_attach_client(new_client);
	
	
done:
	if (ret) {
		delete_proc( d );
		kfree(d);
	}
	return ret;
}

static int ks8995m_detach(struct spi_client *client)
{
	struct ks8995m_data *d = spi_get_clientdata(client);

	spi_detach_client(client);
	
	delete_proc(d);
	
	kfree(d);

	return 0;
}

static int
ks8995m_command(struct spi_client *client, unsigned int cmd, void *arg)
{

	pr_debug("%s:cmd=%d\n",__FUNCTION__,cmd);
/*
	switch (cmd) {
	case RTC_GETDATETIME:
		return rtc8564_get_datetime(client, arg);

	case RTC_SETTIME:
		return rtc8564_set_datetime(client, arg, 0);

	case RTC_SETDATETIME:
		return rtc8564_set_datetime(client, arg, 1);

	case RTC_GETCTRL:
		return rtc8564_get_ctrl(client, arg);

	case RTC_SETCTRL:
		return rtc8564_set_ctrl(client, arg);

	case MEM_READ:
		return rtc8564_read_mem(client, arg);

	case MEM_WRITE:
		return rtc8564_write_mem(client, arg);

	default:
		return -EINVAL;
	}
*/	
    return -EINVAL;
}

static struct spi_driver ks8995m_driver = {
	.owner			= THIS_MODULE,
	.name			= DEVICE_NAME,
	.id				= SPI_DRIVERID_KS8995M,
	.flags			= SPI_DF_NOTIFY,
	.attach_adapter = ks8995m_attach,
	.detach_client	= ks8995m_detach,
	.command		= ks8995m_command
};

static __init int ks8995m_init(void)
{
	return spi_add_driver(&ks8995m_driver);
}

static __exit void ks8995m_exit(void)
{
	spi_del_driver(&ks8995m_driver);
}

MODULE_AUTHOR("Barnabas Kalman <ba...@sednet.hu>");
MODULE_DESCRIPTION("Kendin KS8995M Driver");
MODULE_LICENSE("GPL");

module_init(ks8995m_init);
module_exit(ks8995m_exit);
