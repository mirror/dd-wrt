/* vi: set sw=4 ts=4: */

#include <linux/init.h>
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>

#include "rt3052_access.h"

//#define DEBUG

#ifdef DEBUG
#define DPRINTK(fmt, args...)	printk("%s: " fmt,__func__, ## args)
#else
#define DPRINTK(fmt, args...)
#endif

/*******************************************************************/

#define NUMBER_OF_PORTS	7
#define NUMBER_OF_VLANS	16

static char name[128];

static inline const char * getname_port_vlan_en(int i)
{
	sprintf(name, "vlan_en_port_%d", i);
	return name;
}
static inline const char * getname_port_vid(int i)
{
	sprintf(name, "vid_port_%d", i);
	return name;
}
static inline const char * getname_port_untag(int i)
{
	sprintf(name, "untag_port_%d", i);
	return name;
}
static inline const char * getname_vlan_id(int i)
{
	sprintf(name, "vid_vlan_%d", i);
	return name;
}
static inline const char * getname_vlan_mem(int i)
{
	sprintf(name, "mem_vlan_%d", i);
	return name;
}

/* ---------------------------------------------------------------- */

static int read_port_vid(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	unsigned int port = (unsigned int)data;
	char * p = buf;
	uint32_t val;

	if (port < NUMBER_OF_PORTS)
	{
		val = sysRegRead(RTES_PVIDC(port));
		DPRINTK("reg[0x%08x] << 0x%08x, port(%d)\n",RTES_PVIDC(port),val,port);
		val = (0xfff & ((port&1) ? (val>>12) : val));
		p += sprintf(p, "%d\n", val);
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_port_vid(struct file * file, const char * buf, unsigned long count, void * data)
{
	unsigned int port = (unsigned int)data;
	uint32_t val, reg;

	if (port < NUMBER_OF_PORTS)
	{
		reg = sysRegRead(RTES_PVIDC(port));
		DPRINTK("reg[0x%08x] >> 0x%08x, port(%d)\n",RTES_PVIDC(port),reg,port);
		val = simple_strtoul(buf, NULL, 10);
		if (port & 1)	reg = (0xff000fff & reg) | ((0xfff & val) << 12);
		else			reg = (0xfffff000 & reg) | (0xfff & val);
		sysRegWrite(RTES_PVIDC(port), reg);
		DPRINTK("reg[0x%08x] << 0x%08x\n", RTES_PVIDC(port), reg);
	}
	return count;
}

static int read_vlan_id(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	unsigned int vlan = (unsigned int)data;
	char * p = buf;
	uint32_t val;

	if (vlan < NUMBER_OF_VLANS)
	{
		val = sysRegRead(RTES_VLANI(vlan));
		DPRINTK("reg[0x%08x] << 0x%08x, vlan(%d)\n",RTES_VLANI(vlan),val, vlan);
		val = (0xfff & ((vlan&1) ? (val>>12) : val));
		p += sprintf(p, "%d\n", val);
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_vlan_id(struct file * file, const char * buf, unsigned long count, void * data)
{
	unsigned int vlan = (unsigned int)data;
	uint32_t val, reg;

	if (vlan < NUMBER_OF_VLANS)
	{
		reg = sysRegRead(RTES_VLANI(vlan));
		DPRINTK("reg[0x%08x] >> 0x%08x, vlan(%d)\n",RTES_VLANI(vlan),reg,vlan);
		val = simple_strtoul(buf, NULL, 10);
		if (vlan & 1)	reg = (0xff000fff & reg) | ((0xfff & val) << 12);
		else			reg = (0xfffff000 & reg) | (0xfff & val);
		sysRegWrite(RTES_VLANI(vlan), reg);
		DPRINTK("reg[0x%08x] << 0x%08x\n",RTES_PVIDC(vlan),reg);
	}
	return count;
}

static int read_port_untag(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	unsigned int port = (unsigned int)data;
	char * p = buf;
	uint32_t val;

	if (port < NUMBER_OF_PORTS)
	{
		val = sysRegRead(RTES_POC2);
		DPRINTK("reg[0x%08x] << 0x%08x, port(%d)\n",RTES_POC2,val, port);
		p += sprintf(p, "%d\n", (val & (1<<port)) ? 1 : 0);
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_port_untag(struct file * file, const char * buf, unsigned long count, void * data)
{
	unsigned int port = (unsigned int)data;
	uint32_t reg, val;

	if (port < NUMBER_OF_PORTS)
	{
		reg = sysRegRead(RTES_POC2);
		DPRINTK("reg[0x%08x] >> 0x%08x, port(%d)\n",RTES_POC2,reg,port);
		val = simple_strtoul(buf, NULL, 10);
		if (val)	reg |= (1<<port);
		else		reg &= ~(1<<port);
		sysRegWrite(RTES_POC2, reg);
		DPRINTK("reg[0x%08x] << 0x%08x\n",RTES_POC2,reg);
	}
	return count;
}

static int read_vlan_mem(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	unsigned int vlan = (unsigned int)data;
	char * p = buf;
	uint32_t val;

	if (vlan < NUMBER_OF_VLANS)
	{
		val = sysRegRead(RTES_VMSC(vlan));
		DPRINTK("reg[0x%08x] << 0x%08x, vlan(%d)\n",RTES_VMSC(vlan),val,vlan);
		p += sprintf(p, "0x%02x\n", (0xff & (val >> ((vlan%4)*8))));
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_vlan_mem(struct file * file, const char * buf, unsigned long count, void * data)
{
	unsigned int vlan = (unsigned int)data;
	uint32_t val, reg;

	if (vlan < NUMBER_OF_VLANS)
	{
		reg = sysRegRead(RTES_VMSC(vlan));
		DPRINTK("reg[0x%08x] >> 0x%08x, vlan(%d)\n",RTES_VMSC(vlan),reg,vlan);
		val = simple_strtoul(buf, NULL, 16);
		reg &= ~(0xff << ((vlan%4)*8));
		reg |= ((0xff & val) << ((vlan%4)*8));
		sysRegWrite(RTES_VMSC(vlan), reg);
		DPRINTK("reg[0x%08x] << 0x%08x\n",RTES_VMSC(vlan),reg);
	}
	return count;
}

static int read_port_vlan_en(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	unsigned int port = (unsigned int)data;
	char * p = buf;
	uint32_t val;

	if (port < NUMBER_OF_PORTS)
	{
		val = sysRegRead(RTES_PFC1);
		DPRINTK("reg[0x%08x] >> 0x%08x, port(%d)\n",RTES_PFC1,val,port);
		p += sprintf(p, "%d\n", (val & (1<<(16+port))) ? 1:0);
	}
	else p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_port_vlan_en(struct file * file, const char * buf, unsigned long count, void * data)
{
	unsigned int port = (unsigned int)data;
	uint32_t val, reg;

	if (port < NUMBER_OF_PORTS)
	{
		reg = sysRegRead(RTES_PFC1);
		DPRINTK("reg[0x%08x] >> 0x%08x, port(%d)\n",RTES_PFC1,reg,port);
		val = simple_strtoul(buf, NULL, 10);
		if (val)	reg |= (1<<(16+port));
		else		reg &= ~(1<<(16+port));
		sysRegWrite(RTES_PFC1, reg);
		DPRINTK("reg[0x%08x] << 0x%08x\n",RTES_PFC1,reg);
	}
	return count;
}

static void __init vlan_proc_init(struct proc_dir_entry * root)
{
	unsigned int i;
	struct proc_dir_entry * entry;

	for (i=0; i<NUMBER_OF_PORTS; i++)
	{
		/* Port default VLAN ID setting */
		entry = create_proc_entry(getname_port_vid(i), 0644, root);
		if (entry)
		{
			entry->data = (void *)i;
			entry->read_proc = read_port_vid;
			entry->write_proc = write_port_vid;
//			entry->owner = THIS_MODULE;
		}
		else printk("%s: unable to create '%s'\n",__func__,getname_port_vid(i));

		/* Port tagging/untagging setting */
		entry = create_proc_entry(getname_port_untag(i), 0644, root);
		if (entry)
		{
			entry->data = (void *)i;
			entry->read_proc = read_port_untag;
			entry->write_proc = write_port_untag;
//			entry->owner = THIS_MODULE;
		}
		else printk("%s: unable to create '%s'\n",__func__,getname_port_vid(i));

		/* Port VLAN enable */
		entry = create_proc_entry(getname_port_vlan_en(i), 0644, root);
		if (entry)
		{
			entry->data = (void *)i;
			entry->read_proc = read_port_vlan_en;
			entry->write_proc = write_port_vlan_en;
//			entry->owner = THIS_MODULE;
		}
		else printk("%s: unable to create '%s'\n",__func__,getname_port_vlan_en(i));
	}

	/* rt3052 support 16 VLAN */
	for (i=0; i<NUMBER_OF_VLANS; i++)
	{
		/* VLAN ID setting */
		entry = create_proc_entry(getname_vlan_id(i), 0644, root);
		if (entry)
		{
			entry->data = (void *)i;
			entry->read_proc = read_vlan_id;
			entry->write_proc = write_vlan_id;
//			entry->owner = THIS_MODULE;
		}
		else printk("%s: uanble to create '%s'\n",__func__,getname_vlan_id(i));

		/* VLAN member port setting */
		entry = create_proc_entry(getname_vlan_mem(i), 0644, root);
		if (entry)
		{
			entry->data = (void *)i;
			entry->read_proc = read_vlan_mem;
			entry->write_proc = write_vlan_mem;
//			entry->owner = THIS_MODULE;
		}
		else printk("%s: uanble to create '%s'\n",__func__,getname_vlan_id(i));
	}
}

static void vlan_proc_exit(struct proc_dir_entry * root)
{
	int i;

	for (i=0; i<NUMBER_OF_PORTS; i++)
	{
		remove_proc_entry(getname_port_vid(i), root);
		remove_proc_entry(getname_port_untag(i), root);
		remove_proc_entry(getname_port_vlan_en(i), root);
	}

	/* rt3052 support 16 VLAN */
	for (i=0; i<NUMBER_OF_VLANS; i++)
	{
		remove_proc_entry(getname_vlan_id(i), root);
		remove_proc_entry(getname_vlan_mem(i), root);
	}
}

/*******************************************************************/

extern u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

static int mii_read = 0;
static int mii_phy = 0;
static int mii_reg = 0;
static unsigned short mii_data = 0;

static int read_mii_ctrl(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	char * p = buf;
	p += sprintf(p, "%s phy:%d reg:%d\n", mii_read ? "read":"write", mii_phy, mii_reg);
	*eof = 1;
	return p - buf;
}
static int write_mii_ctrl(struct file * file, const char * buf, unsigned long count, void * data)
{
	char * ptr = (char *)buf;
	u32 val;

	DPRINTK("[%s]\n", ptr);

	if		(strncmp(ptr, "write", 5)==0)	{ mii_read = 0; ptr+=5; }
	else if	(strncmp(ptr, "read", 4)==0)	{ mii_read = 1; ptr+=4; }
	else
	{
		printk("rt3052_mii: unknown command [%s]\n",buf);
		return count;
	}

	while (*ptr && (*ptr==' ' || *ptr=='\t')) ptr++;
	mii_phy = simple_strtoul(ptr, &ptr, 10);
	while (*ptr && (*ptr==' ' || *ptr=='\t')) ptr++;
	mii_reg = simple_strtoul(ptr, &ptr, 10);
	if (mii_read == 0)
	{
		while (*ptr && (*ptr==' ' || *ptr=='\t')) ptr++;
		mii_data = simple_strtoul(ptr, &ptr, 16);
		val = mii_data;
		mii_mgr_write(mii_phy, mii_reg, val);
		DPRINTK("mii_read=%d, mii_phy=%d, mii_reg=%d, mii_data=0x%04x\n",mii_read,mii_phy,mii_reg,mii_data);
	}
	else
	{
		mii_mgr_read(mii_phy, mii_reg, &val);
		mii_data = (unsigned short)val;
		DPRINTK("mii_read=%d, mii_phy=%d, mii_reg=%d\n",mii_read,mii_phy,mii_reg);
	}

	return count;
}
static int read_mii_data(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	char * p = buf;
	p += sprintf(p, "0x%04x\n", mii_data);
	*eof = 1;
	return p - buf;
}
static int write_mii_data(struct file * file, const char * buf, unsigned long count, void * data)
{
	mii_data = (unsigned short)simple_strtoul(buf, NULL, 16);
	DPRINTK("mii_data = 0x%04x\n", mii_data);
	return count;
}


static void __init mii_proc_init(struct proc_dir_entry * root)
{
	struct proc_dir_entry * entry;

	/* MII ctrl */
	entry = create_proc_entry("ctrl", 0644, root);
	if (entry)
	{
		entry->read_proc = read_mii_ctrl;
		entry->write_proc = write_mii_ctrl;
//		entry->owner = THIS_MODULE;
	}

	/* MII data */
	entry = create_proc_entry("data", 0644, root);
	if (entry)
	{
		entry->read_proc = read_mii_data;
		entry->write_proc = write_mii_data;
//		entry->owner = THIS_MODULE;
	}
}


static void mii_proc_exit(struct proc_dir_entry * root)
{
	remove_proc_entry("ctrl", root);
	remove_proc_entry("data", root);
}

/*******************************************************************/

static int read_system_reset(char * buf, char **start, off_t offset, int len, int * eof, void * data)
{
	char * p = buf;
	p += sprintf(p, "n/a\n");
	*eof = 1;
	return p - buf;
}

static int write_system_reset(struct file * file, const char * buf, unsigned long count, void * data)
{
	printk("<<<< SYSTEM RESETTING >>>>\n");
	sysRegWrite(RT_RSTCTRL, BIT(0));
	return count;
}

/*******************************************************************/

static struct proc_dir_entry * g_proc_root = NULL;
static struct proc_dir_entry * g_proc_vlan = NULL;
static struct proc_dir_entry * g_proc_mii = NULL;
static struct proc_dir_entry * g_system_reset = NULL;

void rt3052_access_exit(void)
{
	if (g_proc_root)
	{
		if (g_proc_vlan)
		{
			vlan_proc_exit(g_proc_vlan);
			remove_proc_entry("vlan", g_proc_root);
			g_proc_vlan = NULL;
		}
		if (g_proc_mii)
		{
			mii_proc_exit(g_proc_mii);
			remove_proc_entry("mii", g_proc_root);
			g_proc_mii = NULL;
		}
		remove_proc_entry("rt3052", NULL);
		g_proc_root = NULL;
	}
	if (g_system_reset)
	{
		remove_proc_entry("system_reset", NULL);
		g_system_reset = NULL;
	}
}

int __init rt3052_access_init(void)
{
	int ret;

	printk("rt3052 access driver initialization.\n");

	do
	{
		/* create system reset */
		g_system_reset = create_proc_entry("system_reset", 0644, NULL);
		if (g_system_reset)
		{
			g_system_reset->data = 0;
			g_system_reset->read_proc = read_system_reset;
			g_system_reset->write_proc = write_system_reset;
//			g_system_reset->owner = THIS_MODULE;
		}
		else
		{
			printk("%s: Unable to create system_reset entry!!\n",__func__);
			ret = -ENOMEM;
			break;
		}

		/* create the root directory */
		g_proc_root = proc_mkdir("rt3052", NULL);
		if (g_proc_root == NULL)
		{
			printk("%s: proc_mkdir return NULL!\n",__func__);
			ret = -ENOMEM;
			break;
		}
		ret = 0;

		/* create directory for VLAN */
		g_proc_vlan = proc_mkdir("vlan", g_proc_root);
		if (g_proc_vlan) vlan_proc_init(g_proc_vlan);

		/* create directory for MII */
		g_proc_mii = proc_mkdir("mii", g_proc_root);
		if (g_proc_mii) mii_proc_init(g_proc_mii);

	} while (0);

	if (ret != 0) rt3052_access_exit();
	return ret;
}

