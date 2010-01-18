/****************************************************************************
* Copyright 2006 Storlink Corp.  All rights reserved.
*----------------------------------------------------------------------------
* Name			: sl351x_proc.c
* Description	:
*		Handle Proc Routines for Storlink SL351x Platform
*
* History
*
*	Date		Writer		Description
*----------------------------------------------------------------------------
*	04/13/2006	Gary Chen	Create and implement
*
*
****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/completion.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/semaphore.h>
#include <asm/arch/irqs.h>
#include <asm/arch/it8712.h>
#include <linux/mtd/kvctl.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/ppp_defs.h>
#ifdef CONFIG_NETFILTER
// #include <linux/netfilter_ipv4/ip_conntrack.h>
#endif
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/percpu.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif

#define	 MIDWAY
#define	 SL_LEPUS

// #define PROC_DEBUG_MSG	1

#include <asm/arch/sl2312.h>
#include <asm/arch/sl351x_gmac.h>
#include <asm/arch/sl351x_hash_cfg.h>
#include <asm/arch/sl351x_nat_cfg.h>
#include <asm/arch/sl351x_toe.h>

#ifdef CONFIG_PROC_FS
/*----------------------------------------------------------------------
* Definition
*----------------------------------------------------------------------*/
#define	proc_printf					printk
#define SL351x_GMAC_PROC_NAME		"sl351x_gmac"
#define SL351x_NAT_PROC_NAME		"sl351x_nat"
#define SL351x_TOE_PROC_NAME		"sl351x_toe"

/*----------------------------------------------------------------------
* Function Definition
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static int nat_ct_open(struct inode *inode, struct file *file);
static void *nat_ct_seq_start(struct seq_file *s, loff_t *pos);
static void nat_ct_seq_stop(struct seq_file *s, void *v);
static void *nat_ct_seq_next(struct seq_file *s, void *v, loff_t *pos);
static int nat_ct_seq_show(struct seq_file *s, void *v);
#endif

#ifdef CONFIG_SL351x_RXTOE
static int toe_ct_open(struct inode *inode, struct file *file);
static void *toe_ct_seq_start(struct seq_file *s, loff_t *pos);
static void toe_ct_seq_stop(struct seq_file *s, void *v);
static void *toe_ct_seq_next(struct seq_file *s, void *v, loff_t *pos);
static int toe_ct_seq_show(struct seq_file *s, void *v);
extern int sl351x_get_toe_conn_flag(int index);
extern struct toe_conn * sl351x_get_toe_conn_info(int index);
#endif

static int gmac_ct_open(struct inode *inode, struct file *file);
static void *gmac_ct_seq_start(struct seq_file *s, loff_t *pos);
static void gmac_ct_seq_stop(struct seq_file *s, void *v);
static void *gmac_ct_seq_next(struct seq_file *s, void *v, loff_t *pos);
static int gmac_ct_seq_show(struct seq_file *s, void *v);


/*----------------------------------------------------------------------
* Data
*----------------------------------------------------------------------*/
#ifdef CONFIG_SYSCTL
// static struct ctl_table_header *nat_ct_sysctl_header;
#endif

#ifdef CONFIG_SL351x_NAT
static struct seq_operations nat_ct_seq_ops = {
	.start = nat_ct_seq_start,
	.next  = nat_ct_seq_next,
	.stop  = nat_ct_seq_stop,
	.show  = nat_ct_seq_show
};

static struct file_operations nat_file_ops= {
	.owner   = THIS_MODULE,
	.open    = nat_ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif // CONFIG_SL351x_NAT

#ifdef CONFIG_SL351x_RXTOE
static struct seq_operations toe_ct_seq_ops = {
	.start = toe_ct_seq_start,
	.next  = toe_ct_seq_next,
	.stop  = toe_ct_seq_stop,
	.show  = toe_ct_seq_show
};

static struct file_operations toe_file_ops= {
	.owner   = THIS_MODULE,
	.open    = toe_ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
#endif

static struct seq_operations gmac_ct_seq_ops = {
	.start = gmac_ct_seq_start,
	.next  = gmac_ct_seq_next,
	.stop  = gmac_ct_seq_stop,
	.show  = gmac_ct_seq_show
};

static struct file_operations gmac_file_ops= {
	.owner   = THIS_MODULE,
	.open    = gmac_ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};

#ifdef SL351x_GMAC_WORKAROUND
extern u32 gmac_workaround_cnt[4];
extern u32 gmac_short_frame_workaround_cnt[2];
#ifdef CONFIG_SL351x_NAT
	extern u32 sl351x_nat_workaround_cnt;
#endif
#endif
/*----------------------------------------------------------------------
* nat_ct_open
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static int nat_ct_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &nat_ct_seq_ops);
}
#endif // CONFIG_SL351x_NAT
/*----------------------------------------------------------------------
* nat_ct_seq_start
* find the first
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static void *nat_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	int i;

	// proc_printf("%s: *pos=%d\n", __func__, (int)*pos);
	for (i=*pos; i<HASH_TOTAL_ENTRIES; i++)
	{
		if (hash_get_nat_owner_flag(i))
		{
			*pos = i;
			return (void *)(i+1);
		}
	}
	return NULL;
}
#endif // CONFIG_SL351x_NAT
/*----------------------------------------------------------------------
* nat_ct_seq_stop
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static void nat_ct_seq_stop(struct seq_file *s, void *v)
{
}
#endif // CONFIG_SL351x_NAT
/*----------------------------------------------------------------------
* nat_ct_seq_next
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static void *nat_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	int i;

	// proc_printf("%s: *pos=%d\n", __func__, (int)*pos);
	(*pos)++;
	for (i=*pos; i<HASH_TOTAL_ENTRIES; i++)
	{
		if (hash_get_nat_owner_flag(i))
		{
			*pos = i;
			return (void *)(i+1);
		}
	}
	return NULL;
}
#endif // CONFIG_SL351x_NAT
/*----------------------------------------------------------------------
* nat_ct_seq_show
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static int nat_ct_seq_show(struct seq_file *s, void *v)
{
	int 				idx;
	NAT_HASH_ENTRY_T	*nat_entry;
	GRE_HASH_ENTRY_T	*gre_entry;

	idx = (int)v;
	if (idx<=0 || idx >HASH_TOTAL_ENTRIES)
		return -ENOSPC;

	idx--;
	nat_entry = (NAT_HASH_ENTRY_T *)&hash_tables[idx];
	gre_entry = (GRE_HASH_ENTRY_T *)nat_entry;
	if (nat_entry->key.ip_protocol == IPPROTO_GRE)
	{
		if (seq_printf(s, "%4d: KEY MAC-%d [%d] %u.%u.%u.%u [%u]-->%u.%u.%u.%u\n",
					idx, gre_entry->key.port_id, gre_entry->key.ip_protocol,
					HIPQUAD(gre_entry->key.sip), ntohs(gre_entry->key.call_id),
					HIPQUAD(gre_entry->key.dip)))
			return -ENOSPC;
		if (seq_printf(s, "      PARAMETER: %u.%u.%u.%u -->%u.%u.%u.%u [%u] Timeout:%ds\n",
					HIPQUAD(gre_entry->param.Sip),
					HIPQUAD(gre_entry->param.Dip), gre_entry->param.Dport,
					gre_entry->tmo.counter))
			return -ENOSPC;
	}
	else
	{
		if (seq_printf(s, "%4d: KEY MAC-%d [%d] %u.%u.%u.%u [%u]-->%u.%u.%u.%u [%u]\n",
					idx, nat_entry->key.port_id, nat_entry->key.ip_protocol,
					HIPQUAD(nat_entry->key.sip), ntohs(nat_entry->key.sport),
					HIPQUAD(nat_entry->key.dip), ntohs(nat_entry->key.dport)))
			return -ENOSPC;
		if (seq_printf(s, "      PARAMETER: %u.%u.%u.%u [%u]-->%u.%u.%u.%u [%u] Timeout:%ds\n",
					HIPQUAD(nat_entry->param.Sip), nat_entry->param.Sport,
					HIPQUAD(nat_entry->param.Dip), nat_entry->param.Dport,
					nat_entry->tmo.counter))
			return -ENOSPC;
	}
	return 0;
}
#endif // CONFIG_SL351x_NAT

/*----------------------------------------------------------------------
* toe_ct_open
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static int toe_ct_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &toe_ct_seq_ops);
}
#endif
/*----------------------------------------------------------------------
* toe_ct_seq_start
* find the first
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static void *toe_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	int i;

	// proc_printf("%s: *pos=%d\n", __func__, (int)*pos);
	for (i=*pos; i<TOE_TOE_QUEUE_NUM; i++)
	{
		if (sl351x_get_toe_conn_flag(i))
		{
			*pos = i;
			return (void *)(i+1);
		}
	}
	return NULL;
}
#endif
/*----------------------------------------------------------------------
* toe_ct_seq_stop
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static void toe_ct_seq_stop(struct seq_file *s, void *v)
{
}
#endif
/*----------------------------------------------------------------------
* toe_ct_seq_next
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static void *toe_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	int i;

	// proc_printf("%s: *pos=%d\n", __func__, (int)*pos);
	(*pos)++;
	for (i=*pos; i<TOE_TOE_QUEUE_NUM; i++)
	{
		if (sl351x_get_toe_conn_flag(i))
		{
			*pos = i;
			return (void *)(i+1);
		}
	}
	return NULL;
}
#endif
/*----------------------------------------------------------------------
* toe_ct_seq_show
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static int toe_ct_seq_show(struct seq_file *s, void *v)
{
	int 				idx;
	struct toe_conn		*toe_entry;

	idx = (int)v;
	if (idx<=0 || idx >TOE_TOE_QUEUE_NUM)
		return -ENOSPC;

	idx--;
	toe_entry = (struct toe_conn *)sl351x_get_toe_conn_info(idx);
	if (!toe_entry)
		return -ENOSPC;

	if (seq_printf(s, "%4d: Qid %d MAC-%d TCP %u.%u.%u.%u [%u]-->%u.%u.%u.%u [%u]\n",
				idx, toe_entry->qid, toe_entry->gmac->port_id,
				NIPQUAD(toe_entry->saddr[0]), ntohs(toe_entry->source),
				NIPQUAD(toe_entry->daddr[0]), ntohs(toe_entry->dest)))
			return -ENOSPC;
	return 0;
}
#endif
/*----------------------------------------------------------------------
* gmac_ct_open
*----------------------------------------------------------------------*/
static int gmac_ct_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &gmac_ct_seq_ops);
}

/*----------------------------------------------------------------------
* gmac_ct_seq_start
* find the first
*----------------------------------------------------------------------*/
static void *gmac_ct_seq_start(struct seq_file *s, loff_t *pos)
{
	int i;
	i = (int)*pos + 1;;

	if (i > 9)
		return NULL;
	else
		return (void *)i;
}

/*----------------------------------------------------------------------
* gmac_ct_seq_stop
*----------------------------------------------------------------------*/
static void gmac_ct_seq_stop(struct seq_file *s, void *v)
{
}

/*----------------------------------------------------------------------
* gmac_ct_seq_next
*----------------------------------------------------------------------*/
static void *gmac_ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	int i;

	// proc_printf("%s: *pos=%d\n", __func__, (int)*pos);

	(*pos)++;
	i = (int)*pos + 1;;

	if (i > 9)
		return NULL;
	else
		return (void *)i;
}

/*----------------------------------------------------------------------
* seq_dm_long
*----------------------------------------------------------------------*/
static void seq_dm_long(struct seq_file *s, u32 location, int length)
{
	u32		*start_p, *curr_p, *end_p;
	u32		*datap, data;
	int		i;

	//if (length > 1024)
	//	length = 1024;

	start_p = (u32 *)location;
	end_p = (u32 *)location + length;
	curr_p = (u32 *)((u32)location & 0xfffffff0);
	datap = (u32 *)location;
	while (curr_p < end_p)
	{
		cond_resched();
		seq_printf(s, "0x%08x: ",(u32)curr_p & 0xfffffff0);
		for (i=0; i<4; i++)
		{
			if (curr_p < start_p || curr_p >= end_p)
               seq_printf(s, "         ");
			else
			{
				data = *datap;
				seq_printf(s, "%08X ", data);
			}
			if (i==1)
              seq_printf(s, "- ");

			curr_p++;
			datap++;
		}
        seq_printf(s, "\n");
	}
}

/*----------------------------------------------------------------------
* gmac_ct_seq_show
*----------------------------------------------------------------------*/
static int gmac_ct_seq_show(struct seq_file *s, void *v)
{
	switch ((int)v)
	{
		case 1:
			seq_printf(s, "\nGMAC Global Registers\n");
			seq_dm_long(s, TOE_GLOBAL_BASE, 32);
			break;
		case 2:
			seq_printf(s, "\nGMAC Non-TOE Queue Header\n");
			seq_dm_long(s, TOE_NONTOE_QUE_HDR_BASE, 12);
			break;
		case 3:
			seq_printf(s, "\nGMAC TOE Queue Header\n");
			seq_dm_long(s, TOE_TOE_QUE_HDR_BASE, 12);
			break;
		case 4:
			seq_printf(s, "\nGMAC-0 DMA Registers\n");
			seq_dm_long(s, TOE_GMAC0_DMA_BASE, 52);
			break;
		case 5:
			seq_printf(s, "\nGMAC-0 Registers\n");
			seq_dm_long(s, TOE_GMAC0_BASE, 32);
			break;
		case 6:
			seq_printf(s, "\nGMAC-1 DMA Registers\n");
			seq_dm_long(s, TOE_GMAC1_DMA_BASE, 52);
			break;
		case 7:
			seq_printf(s, "\nGMAC-1 Registers\n");
			seq_dm_long(s, TOE_GMAC1_BASE, 32);
			break;
		case 8:
			seq_printf(s, "\nGLOBAL Registers\n");
			seq_dm_long(s, GMAC_GLOBAL_BASE_ADDR, 16);
			break;
		case 9:
#ifdef SL351x_GMAC_WORKAROUND
			seq_printf(s, "\nGMAC-0 Rx/Tx/Short Workaround: %u, %u, %u\n", gmac_workaround_cnt[0], gmac_workaround_cnt[1], gmac_short_frame_workaround_cnt[0]);
			seq_printf(s, "GMAC-1 Rx/Tx/Short Workaround: %u, %u, %u\n", gmac_workaround_cnt[2], gmac_workaround_cnt[3], gmac_short_frame_workaround_cnt[1]);
#ifdef CONFIG_SL351x_NAT
			seq_printf(s, "NAT Workaround: %u\n", sl351x_nat_workaround_cnt);
#endif
#endif
			break;
		default:
			return -ENOSPC;
	}
	return 0;
}

/*----------------------------------------------------------------------
* init
*----------------------------------------------------------------------*/
static int __init init(void)
{
	struct proc_dir_entry *proc_gmac=NULL;

#ifdef CONFIG_SL351x_NAT
	struct proc_dir_entry *proc_nat=NULL;
#endif

#ifdef CONFIG_SL351x_RXTOE
	struct proc_dir_entry *proc_toe=NULL;
#endif

#ifdef CONFIG_SYSCTL
	// nat_ct_sysctl_header = NULL;
#endif
	proc_gmac = proc_net_fops_create(SL351x_GMAC_PROC_NAME, 0440, &gmac_file_ops);
	if (!proc_gmac) goto init_bad;

#ifdef CONFIG_SL351x_NAT
	proc_nat = proc_net_fops_create(SL351x_NAT_PROC_NAME, 0440, &nat_file_ops);
	if (!proc_nat) goto init_bad;
#endif // CONFIG_SL351x_NAT

#ifdef CONFIG_SL351x_RXTOE
	proc_toe = proc_net_fops_create(SL351x_TOE_PROC_NAME, 0440, &toe_file_ops);
	if (!proc_toe) goto init_bad;
#endif

#ifdef CONFIG_SYSCTL
	// nat_ct_sysctl_header = register_sysctl_table(nat_ct_net_table, 0);
	// if (!nat_ct_sysctl_header) goto init_bad;
#endif

	return 0;

init_bad:
	if (proc_gmac) proc_net_remove(SL351x_GMAC_PROC_NAME);

#ifdef CONFIG_SL351x_NAT
	if (proc_nat) proc_net_remove(SL351x_NAT_PROC_NAME);
#endif

#ifdef CONFIG_SL351x_RXTOE
	if (proc_toe) proc_net_remove(SL351x_NAT_PROC_NAME);
#endif

#ifdef CONFIG_SYSCTL
	// if (nat_ct_sysctl_header) unregister_sysctl_table(nat_ct_sysctl_header);
#endif
	proc_printf("SL351x NAT Proc: can't create proc or register sysctl.\n");
	return -ENOMEM;
}

/*----------------------------------------------------------------------
* fini
*----------------------------------------------------------------------*/
static void __exit fini(void)
{
	proc_net_remove(SL351x_GMAC_PROC_NAME);

#ifdef CONFIG_SL351x_NAT
	proc_net_remove(SL351x_NAT_PROC_NAME);
#endif

#ifdef CONFIG_SL351x_RXTOE
	proc_net_remove(SL351x_TOE_PROC_NAME);
#endif

#ifdef CONFIG_SYSCTL
	// unregister_sysctl_table(nat_ct_sysctl_header);
#endif
}

/*----------------------------------------------------------------------
* module
*----------------------------------------------------------------------*/
module_init(init);
module_exit(fini);

#endif	// CONFIG_PROC_FS
