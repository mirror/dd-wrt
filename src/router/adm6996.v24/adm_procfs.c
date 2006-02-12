#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/init.h>

#include "adm6996.h"

#define BIT(x) (1<<(x))
#ifdef CONFIG_PROC_FS


static inline char* yesno (unsigned int v)
{
	return (v ? "yes" : "no");
}

static int adm6996_procfs_read (char *page, char **start,
		off_t offset, int count,
		int *eof, void *data)
{
	int len, i;
	adm_info_t* info = (adm_info_t*) data; 
	struct adm_port_stat aps[6];
	struct adm_gen_config conf;

	if (offset > 0)
		return 0;

	len = sprintf (page, "ADM6996L version %s\n", ADM6996_VERSION);

	/* EEPROM config */
	len += sprintf (page+len, "--- Stats ---\n");
	adm_get_ports_stats (6, aps);
	for (i=0; i<6; ++i)
	{
		len += sprintf (page+len, "Port %d: ", i);
		len += sprintf (page+len, " %s", aps[i].link? "UP":"DOWN");
		len += sprintf (page+len, " %s", aps[i].speed? "100M": "10M");
		len += sprintf (page+len, " %s",aps[i].duplex? "FD":"HD");
		if (aps[i].flow) 
			len += sprintf (page+len, aps[i].duplex ? " 802.3X" : " BP");
		if (aps[i].cable_broken) len += sprintf (page+len, " NOCABLE");
		len += sprintf (page+len, "\n");

		len += sprintf (page+len, "RX packets:%u%s bytes:%u%s ", 
				aps[i].rx_packets, aps[i].rxp_overflow ? " (overflow)":"",
				aps[i].rx_bytes, aps[i].rxb_overflow ? " (overflow)":"");
		
		len += sprintf (page+len, "TX packets:%u%s bytes:%u%s\n", 
				aps[i].tx_packets, aps[i].txp_overflow ? " (overflow)":"",
				aps[i].tx_bytes, aps[i].txb_overflow ? " (overflow)":"");
		
		len += sprintf (page+len, "errors:%u%s collisions:%u%s "
				"cable length:%u\n", 
				aps[i].errors, aps[i].err_overflow ? " (overflow)":"",
				aps[i].collisions, aps[i].col_overflow ? " (overflow)":"",
				aps[i].cable_len);
		len += sprintf (page+len, "\n");
	}
		
	return len;
}

static int adm6996_procfs_write (struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	int retval = -EINVAL;
	return retval;
}

int __init adm6996_register_procfs (adm_info_t *info)
{
	static struct proc_dir_entry *adm6996_file;
	char filename[9];

	/* create using convenience function */
	sprintf(filename, "adm6996_%d", 0/*info->nr*/);
	adm6996_file = create_proc_entry (filename, S_IFREG | S_IRUGO | S_IWUSR, proc_root_driver);
	if (adm6996_file == NULL)
		return -ENOMEM;

	adm6996_file->data = info;
	adm6996_file->read_proc = adm6996_procfs_read;
	adm6996_file->write_proc = adm6996_procfs_write;
	adm6996_file->size = 0;
	adm6996_file->owner = THIS_MODULE;

	/* single card backward compatibility */
	if (0/*info->nr*/ == 0)
		proc_symlink ("adm6996", proc_root_driver, "adm6996_0");
	// PRINTK_INFO ("procfs file registered for adm6996_%d\n", 0/*info->nr*/);
	return 0;
}

void __exit adm6996_unregister_procfs (adm_info_t* info)
{
	char filename[9];
	sprintf(filename, "adm6996_%d", 0/*info->nr*/);
	remove_proc_entry (filename, proc_root_driver);
	/* single card backward compatibility */
	if (0/*info->nr*/ == 0)
		remove_proc_entry ("adm6996", proc_root_driver);
	// PRINTK_INFO ("procfs file unregistered for adm6996_%d\n", 0/*info->nr*/);
}

// static struct adm_gen_config cfg;
static int drop_on_collisions, replace_vid_pvid, ipg_bits, trunk,
	         far_end_fault, xcrc, aging,
					 polarity_error, reg_10_bit_3, reg_2c_bit_11;
static int storming[2];
static int drop[4];
static int mac_clone_11, vlan_mode;
static int mac_clone_30, mii_speed, speed_led, port_led;
static int tag_shift, fwd_management_mac1, fwd_management_mac2,
					 fwd_management_mac3, fwd_management_mac4, smart_squelch;
static int vlan_groups[16][6], vlan_prio[8], tos_prio[8];

static struct _port_cfg {
	int flow, autoneg, speed, duplex, tagging, enabled, use_tos,
	    port_prio, pvid, fx, crossover, mac_lock, bandwidth, count_recv,
			vlan, use_prio;
} *port_cfg;

static int 
adm_sysctl_handler (ctl_table *table, int write, struct file *filp,
                    void *buffer, size_t *lenp)
{
	int ret, i, port, ctl_name;
	uint16 val;
	uint32 val32;
	static const int port_map[] = {0x01, 0x03, 0x05, 0x07, 0x08, 0x09};

	ret = proc_dointvec(table, write, filp, buffer, lenp);
	if (ret || ! write) return ret;

	port = table->ctl_name / 100 - 1;
	ctl_name = table->ctl_name;
	if (ctl_name >= 100) ctl_name = ctl_name%100+100;

	switch (ctl_name)
	{
		default:
			printk ("ADM6996: unknown sysctl request: %s [%d]\n",
					table->procname, table->ctl_name);
			return -1;

		case 1: /* Discard mode */
			for (i=0; i<4; ++i) drop[i] &= 3;
		case 9: /* storming */
			storming[1] &= 3;
		case 2: /* CRC */
		case 3: /* Aging */
		case 25: /* reg 10, bit 3 */
			val = (storming[1] & 3) |
				    ((storming[0] ? 1 : 0) << 2) |
						((reg_10_bit_3 ? 1 : 0) << 3) |
						((xcrc ? 0 : 1) << 4) |
						((aging ? 0 : 1) << 7) |
						(drop[0] << 8) |
						(drop[1] << 10) |
						(drop[2] << 12) |
						(drop[3] << 14)						
						;
			adm_wreg (einfo, 0x10, val);
			break;
			
		case 7: /* Replaced packet VID 0,1 with PVID */
			val = ((replace_vid_pvid ? 1 : 0) << 9);
			adm_wreg (einfo, 0x0a, val);
			break;

		case 5: /* Trunking */
		case 6: /* IPG bits */
		case 4: /* Far end fault detection */
			val = ((ipg_bits==92 ? 1 : 0) << 6) |
				    ((trunk ? 1 : 0) << 7) |
						((far_end_fault ? 0 : 1) << 15);
			adm_wreg (einfo, 0x0b, val);
			break;

		case 10: /* vlan mode */
		case 11: /* mac clone 11reg */
		case 21: /* smart squelch */
			val = ((mac_clone_11 ? 1 : 0) << 4) |
				    ((vlan_mode ? 1 : 0) << 5) |
						((smart_squelch ? 1 : 0) << 10);
			adm_wreg (einfo, 0x11, val);
			break;

		case 22: /* vlan priority map */
			val = 0;
			for (i=0; i<8; ++i)
				val |= ((vlan_prio[i]&=3)<<(i*2));
			adm_wreg (einfo, 0x0e, val);
			break;

		case 23: /* tos priority map */
			val = 0;
			for (i=0; i<8; ++i)
				val |= ((tos_prio[i]&=3)<<(i*2));
			adm_wreg (einfo, 0x0f, val);
			break;


		case 12: /* mac clone 30 reg */
		case 13: /* mii speed double */
		case 14: /* dual speed led */
		case 15: /* port led mode */
			/* this register has R/W reserved bits, so we had to 
				 preserve its values */
			adm_rreg(einfo, 0, 0x30, &val32);
			val = val32 & (0xFFFF & ~(BIT(5)&BIT(6)&BIT(9)&BIT(12)));
			
			val |= ((mac_clone_30 ? 1 : 0) << 5) |
				     ((mii_speed ? 1 : 0) << 6) |
						 ((speed_led ? 1 : 0) << 9) |
						 ((port_led ? 1 : 0) << 12);
			adm_wreg (einfo, 0x30, val);
			break;

		case 8: /* drop on collisions */
		/* port settings */
		case 112: /* mac lock */
			val = ((port_cfg[0].mac_lock?1:0)<<0) |
				    ((port_cfg[1].mac_lock?1:0)<<2) |
				    ((port_cfg[2].mac_lock?1:0)<<4) |
				    ((port_cfg[3].mac_lock?1:0)<<6) |
				    ((port_cfg[4].mac_lock?1:0)<<7) |
				    ((port_cfg[5].mac_lock?1:0)<<8) |
						((drop_on_collisions?1:0)<<15);
			adm_wreg (einfo, 0x12, val);
			break;

		/* vlan-groups */
		case 70: case 71: case 72: case 73:
		case 74: case 75: case 76: case 77:
		case 78: case 79: case 80: case 81:
		case 82: case 83: case 84: case 85:
			for (i=0; i<16; ++i)
			{
				static const int bit_map[] = { BIT(0), BIT(2), BIT(4),
																			 BIT(6), BIT(7), BIT(8) };

				int j;
				for (val=0, j=0; j<6; ++j) 
					if (vlan_groups[i][j]) val |= bit_map[j];
				adm_wreg (einfo, 0x13+i, val);
			}
			break;
	
		
		case 101: /* speed */
			if (port_cfg[port].speed != 100 && 
					port_cfg[port].speed != 10)
				port_cfg[port].speed = 100;

		case 100: /* enable */
		case 104: /* tagging */
		case 105: /* autoneg */
		case 106: /* duplex */
		case 107: /* port prio */
		case 108: /* vlan-mask */
		case 109: /* crossover */
		case 110: /* tos */
		case 111: /* fx */
		case 113: /* flow */
			printk ("ADM6996: %d request; port %d; id %d\n", 
					ctl_name, port, table->ctl_name);
			
			val = ((port_cfg[port].flow?1:0) << 0) |
				    ((port_cfg[port].autoneg?1:0) << 1) |
						((port_cfg[port].speed==100?1:0) << 2) |
						((port_cfg[port].duplex?1:0) << 3) |
						((port_cfg[port].tagging?1:0)<<4) |
						((port_cfg[port].enabled?0:1)<<5) |
						((port_cfg[port].use_tos?1:0)<<6) |
						((port_cfg[port].use_prio?1:0)<<7) |
						((port_cfg[port].port_prio&=3)<<8) |
						((port_cfg[port].fx?1:0) << 14) |
						((port_cfg[port].crossover?1:0)<<15);
			val |= ((port_cfg[port].pvid&0x0f)<<10);
			adm_wreg (einfo, port_map[port], val);

			if (ctl_name != 108) break;

			port_cfg[port].vlan = (port_cfg[port].pvid>>tag_shift) & 0xf;

		
			if (port <= 2)
			{
				val = ((port_cfg[port].pvid & 0xff0)>>4);
				adm_wreg (einfo, 0x28+port, val);
				break;
			} 
			else if (port <= 4)
			{
				val = ((port_cfg[3].pvid & 0xff0)>>4) |
					    ((port_cfg[4].pvid & 0xff0)<<4);
				adm_wreg (einfo, 0x2b, val);
				break;
			}

			
			/* fallback */
		case 16:
			if (ctl_name == 16)
			{
				for (i=0; i<6; ++i)
					port_cfg[i].vlan = (port_cfg[i].pvid>>tag_shift) & 0xf;
			}
		case 17: case 18: case 19: case 20: case 26:
			val = ((port_cfg[3].pvid & 0xff0)>>4) |
				    ((tag_shift&7)<<8) |
				    ((reg_2c_bit_11?1:0)<<11) |
						((fwd_management_mac1?1:0)<<12) |
						((fwd_management_mac2?1:0)<<13) |
						((fwd_management_mac3?1:0)<<14) |
						((fwd_management_mac4?1:0)<<15);
			adm_wreg (einfo, 0x2c, val);
			break;
			
	}

	return 0;
}

static char* bandwidth_map[] = { "256k", "512k", "1m", "2m", "5m",
	                               "10m", "20m", "50m", "full" };


static int 
adm_sysctl_bandwidth (ctl_table *table, int write, struct file *filp,
                      void *buffer, size_t *lenp)
{
	int ret, port, val, index;
	ctl_table fake_table;
	int ctl_name;
	unsigned char buf[5];
	
	port = (table->ctl_name / 100) - 1;
	// printk ("ADM6996: port %d detected, ctlname %d\n", port, table->ctl_name);
	ctl_name = table->ctl_name % 100 + 100;

	index = port_cfg[port].bandwidth;
	if (index == -1) index = 8;
	
	strcpy (buf, bandwidth_map[index]);
	fake_table.data = buf;
	fake_table.maxlen = sizeof(buf);

	ret = proc_dostring(&fake_table, write, filp, buffer, lenp);

	if (ret == 0 && write)
	{
		int i;
		for (i=0; i<sizeof (bandwidth_map)/sizeof(char*); ++i)
			if (! strcmp(buf, bandwidth_map[i])) break;
		
		if (i>8) return -EINVAL;
		if (i>7) i=-1;
		
		port_cfg[port].bandwidth = i;

		if (port <= 3)
		{
			val = ((port_cfg[0].bandwidth & 7) << 0) |
						((port_cfg[0].count_recv ? 0 : 1) << 3) |
			      ((port_cfg[1].bandwidth & 7) << 4) |
						((port_cfg[1].count_recv ? 0 : 1) << 7) |
			      ((port_cfg[2].bandwidth & 7) << 8) |
						((port_cfg[2].count_recv ? 0 : 1) << 11) |
			      ((port_cfg[3].bandwidth & 7) << 12) |
						((port_cfg[3].count_recv ? 0 : 1) << 15);
			adm_wreg (einfo, 0x31, val);
		}
		else
		{
			val = ((port_cfg[4].bandwidth & 7) << 0) |
						((port_cfg[4].count_recv ? 0 : 1) << 3) |
			      ((port_cfg[5].bandwidth & 7) << 4) |
						((port_cfg[5].count_recv ? 0 : 1) << 7);
			adm_wreg (einfo, 0x32, val);
		}

		val = ((port_cfg[0].bandwidth==-1?0:1)<<0) |
				  ((port_cfg[1].bandwidth==-1?0:1)<<2) |
				  ((port_cfg[2].bandwidth==-1?0:1)<<4) |
				  ((port_cfg[3].bandwidth==-1?0:1)<<6) |
				  ((port_cfg[4].bandwidth==-1?0:1)<<7) |
				  ((port_cfg[5].bandwidth==-1?0:1)<<8);
		adm_wreg (einfo, 0x33, val);
	}

	return 0;
}

static ctl_table vlan_table[] = {
	{ 70, "0", vlan_groups[0], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 71, "1", vlan_groups[1], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 72, "2", vlan_groups[2], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 73, "3", vlan_groups[3], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 74, "4", vlan_groups[4], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 75, "5", vlan_groups[5], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 76, "6", vlan_groups[6], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 77, "7", vlan_groups[7], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 78, "8", vlan_groups[8], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 79, "9", vlan_groups[9], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 80, "10", vlan_groups[10], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 81, "11", vlan_groups[11], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 82, "12", vlan_groups[12], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 83, "13", vlan_groups[13], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 84, "14", vlan_groups[14], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 85, "15", vlan_groups[15], 6*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 0, }
};

static ctl_table port0_table[] = {
	{ 100, "enable", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 101, "speed", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 102, "bandwidth", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_bandwidth, NULL, },
	{ 103, "count-recv", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 104, "tagging", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 105, "auto-negotiating", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 106, "duplex", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 107, "port-prio", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 108, "vlan-group-mask", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 109, "crossover", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 110, "tos-over-vlan-prio", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 111, "fx-mode", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 112, "mac-lock", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 113, "flow", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	{ 114, "vlan-group", NULL, sizeof(int), 0444, NULL,
		&proc_dointvec, NULL, },
	{ 115, "port-prio-enable", NULL, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },
	
	{ 0, }
};

static ctl_table port1_table[sizeof(port0_table) / sizeof(ctl_table)];
static ctl_table port2_table[sizeof(port0_table) / sizeof(ctl_table)];
static ctl_table port3_table[sizeof(port0_table) / sizeof(ctl_table)];
static ctl_table port4_table[sizeof(port0_table) / sizeof(ctl_table)];
static ctl_table port5_table[sizeof(port0_table) / sizeof(ctl_table)];
		
static ctl_table adm_table[] = {
	{ 50, "vlan-groups", NULL, 0, 0555, vlan_table, },

	{ 51, "port0", NULL, 0, 0555, port0_table, },
	{ 52, "port1", NULL, 0, 0555, port1_table, },
	{ 53, "port2", NULL, 0, 0555, port2_table, },
	{ 54, "port3", NULL, 0, 0555, port3_table, },
	{ 55, "port4", NULL, 0, 0555, port4_table, },
	{ 56, "port5", NULL, 0, 0555, port5_table, },

	{ 1, "drop-scheme", &drop, 4*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 2, "crc", &xcrc, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 3, "aging", &aging, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 4, "far-end-fault", &far_end_fault, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 5, "trunk", &trunk, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 6, "ipg-bits", &ipg_bits, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 7, "replace-vid01-with-pvid", &replace_vid_pvid, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 8, "drop-on-collisions", &drop_on_collisions, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 9, "storming", &storming, 2*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 10, "vlan-mode", &vlan_mode, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 11, "mac-clone-reg11h", &mac_clone_11, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 12, "mac-clone-reg30h", &mac_clone_30, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 13, "mii-speed-double", &mii_speed, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 14, "speed-led", &speed_led, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 15, "port-led", &port_led, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 16, "tag-shift", &tag_shift, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 17, "fwd-management-mac-1", &fwd_management_mac1, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 18, "fwd-management-mac-2", &fwd_management_mac2, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 19, "fwd-management-mac-3", &fwd_management_mac3, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 20, "fwd-management-mac-4", &fwd_management_mac4, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 21, "smart-squelch", &smart_squelch, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 22, "vlan-prio", &vlan_prio, 8*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 23, "tos-prio", &tos_prio, 8*sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 24, "polarity-error", &polarity_error, sizeof(int), 0444, NULL,
		&adm_sysctl_handler, NULL, },

	{ 25, "reg_10_bit_3", &reg_10_bit_3, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 26, "reg_2c_bit_11", &reg_2c_bit_11, sizeof(int), 0644, NULL,
		&adm_sysctl_handler, NULL, },

	{ 0, }
};

static ctl_table adm_root[] = {
	{ 1, "adm6996", NULL, 0, 0555, adm_table, },
	{ 0, }
};

static ctl_table dev_root[] = {
	{ CTL_DEV, "dev", NULL, 0, 0555, adm_root, },
	{ 0, }
};

static struct ctl_table_header *sysctl_header;

int __init init_sysctl(void)
{
	int i, k;
	ctl_table* tbl[6]	= { port0_table, port1_table, port2_table,
		                    port3_table, port4_table, port5_table };
	static struct adm_gen_config cfg;

	port_cfg = kmalloc(6 * sizeof(struct _port_cfg), GFP_KERNEL);

	for (k=1; k<6; ++k)
	{
		for (i=0; i<sizeof(port0_table) / sizeof(ctl_table); ++i)
		{
			tbl[k][i] = port0_table[i];
		}
	}

	for (k=0; k<6; ++k)
	{
		for (i=0; i<(sizeof port0_table/sizeof(ctl_table))-1; ++i)
			tbl[k][i].ctl_name = (k+1)*100+i;

		tbl[k][0].data = & port_cfg[k].enabled;
		tbl[k][1].data = & port_cfg[k].speed;
		tbl[k][2].data = & port_cfg[k].bandwidth;
		tbl[k][3].data = & port_cfg[k].count_recv;
		tbl[k][4].data = & port_cfg[k].tagging;
		tbl[k][5].data = & port_cfg[k].autoneg;
		tbl[k][6].data = & port_cfg[k].duplex;
		tbl[k][7].data = & port_cfg[k].port_prio;
		tbl[k][8].data = & port_cfg[k].pvid;
		tbl[k][9].data = & port_cfg[k].crossover;
		tbl[k][10].data = & port_cfg[k].use_tos;
		tbl[k][11].data = & port_cfg[k].fx;
		tbl[k][12].data = & port_cfg[k].mac_lock;
		tbl[k][13].data = & port_cfg[k].flow;
		tbl[k][14].data = & port_cfg[k].vlan;
		tbl[k][15].data = & port_cfg[k].use_prio;
	}
	
	adm_get_gen_config (&cfg);

	for (k=0; k<6; ++k)
	{
		port_cfg[k].count_recv = cfg.port[k].count_recv ? 0 : 1;
		port_cfg[k].enabled = cfg.port[k].disabled ? 0 : 1;
		port_cfg[k].autoneg = cfg.port[k].autoneg ? 1 : 0;
		port_cfg[k].speed = cfg.port[k].speed ? 100 : 10;
		port_cfg[k].bandwidth = 
			(cfg.port[k].thrs_ena ? cfg.port[k].threshold : -1);
		port_cfg[k].tagging = cfg.port[k].tagging ? 1 : 0;
		port_cfg[k].duplex = cfg.port[k].duplex;
		port_cfg[k].fx = cfg.port[k].fx;
		port_cfg[k].crossover = cfg.port[k].crossover;
		port_cfg[k].flow = cfg.port[k].flow;
		port_cfg[k].use_tos = cfg.port[k].tos;
		port_cfg[k].port_prio = cfg.port[k].port_prio;
		port_cfg[k].use_prio = cfg.port[k].use_prio;
		port_cfg[k].mac_lock = cfg.port[k].mac_lock;
		port_cfg[k].pvid = cfg.port[k].pvid_0_3 | (cfg.port[1].pvid_4_11<<4);
		port_cfg[k].vlan = (port_cfg[k].pvid>>cfg.tag_shift) & 0xf;
		for (i=0; i<16; ++i)
		{
			int j;
			for (j=0; j<6; ++j)
				vlan_groups[i][j] = (cfg.vlan_groups[i] & BIT(j)) ? 1 : 0;
		}
	}

	drop_on_collisions = cfg.drop_on_collisions;
	replace_vid_pvid = cfg.replace_vid_pvid;
	ipg_bits = cfg.ipg92 ? 92 : 96;
	trunk = cfg.trunk;
	far_end_fault = cfg.far_end_fault;
	xcrc = cfg.xcrc;
	aging = cfg.aging;
	polarity_error = cfg.polarity_error;
	reg_10_bit_3 = cfg.reg_10_bit_3;
	reg_2c_bit_11 = cfg.reg_2c_bit_11;
	drop[0] = cfg.drop_q0;
	drop[1] = cfg.drop_q1;
	drop[2] = cfg.drop_q2;
	drop[3] = cfg.drop_q3;
	storming[0] = cfg.storming;
	storming[1] = cfg.storm_threshold;
	mac_clone_11 = cfg.mac_clone_11;
	vlan_mode = cfg.vlan_mode;
	mac_clone_30 = cfg.mac_clone_30;
	mii_speed = cfg.mii_speed;
	speed_led = cfg.speed_led;
	port_led = cfg.port_led;
	tag_shift = cfg.tag_shift;
	fwd_management_mac1 = cfg.fwd_management_mac1;
	fwd_management_mac2 = cfg.fwd_management_mac2;
	fwd_management_mac3 = cfg.fwd_management_mac3;
	fwd_management_mac4 = cfg.fwd_management_mac4;
	smart_squelch = cfg.smart_squelch;
	for (i=0; i<8; ++i)
	{
		vlan_prio[i] = (cfg.vlan_prio>>(i*2)) & 3;
		tos_prio[i] = (cfg.tos_prio>>(i*2)) & 3;
	}
	
	sysctl_header = register_sysctl_table(dev_root, 0);
	return 0;
}

void __exit cleanup_sysctl(void)
{
	unregister_sysctl_table(sysctl_header);
	kfree (port_cfg);
}

#endif /* CONFIG_PROC_FS */

