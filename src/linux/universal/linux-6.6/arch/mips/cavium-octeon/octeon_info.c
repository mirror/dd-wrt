/*
 * Simple /proc interface to Octeon Information
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2012 Cavium Inc.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/octeon/octeon.h>

extern struct cvmx_bootinfo *octeon_bootinfo;
#if defined(CONFIG_CAVIUM_RESERVE32) && CONFIG_CAVIUM_RESERVE32
extern uint64_t octeon_reserve32_memory;
#endif

/**
 * User is reading /proc/octeon_info
 *
 * @param m
 * @param v
 * @return
 */
static int octeon_info_show(struct seq_file *m, void *v)
{
	struct cvmx_sysinfo *sysinfo = cvmx_sysinfo_get();

	seq_printf(m, "processor_id:        0x%x\n", read_c0_prid());
	seq_printf(m, "boot_flags:          0x%x\n", octeon_bootinfo->flags);
	seq_printf(m, "dram_size:           %u\n", octeon_bootinfo->dram_size);
	seq_printf(m, "phy_mem_desc_addr:   0x%x\n", (uint32_t)sysinfo->phy_mem_desc_addr);
	seq_printf(m, "eclock_hz:           %u\n", octeon_bootinfo->eclock_hz);
	seq_printf(m, "io_clock_hz:         %llu\n", octeon_get_io_clock_rate());
	seq_printf(m, "dclock_hz:           %u\n", octeon_bootinfo->dclock_hz);
	seq_printf(m, "board_type:          %u\n", octeon_bootinfo->board_type);
	seq_printf(m, "board_rev_major:     %u\n",
		   octeon_bootinfo->board_rev_major);
	seq_printf(m, "board_rev_minor:     %u\n",
		   octeon_bootinfo->board_rev_minor);
	seq_printf(m, "board_serial_number: %s\n",
		   sysinfo->board_serial_number);
	seq_printf(m, "mac_addr_base:       %pM\n", sysinfo->mac_addr_base);
	seq_printf(m, "mac_addr_count:      %u\n",
		   sysinfo->mac_addr_count);
	if (octeon_bootinfo->minor_version >= 3 && octeon_bootinfo->fdt_addr) {
		seq_printf(m, "fdt_addr:            0x%lx\n",
			(long int) octeon_bootinfo->fdt_addr);
	}
#if defined(CONFIG_CAVIUM_RESERVE32) && CONFIG_CAVIUM_RESERVE32
	seq_printf(m, "32bit_shared_mem_base: 0x%lx\n",
		   (long int) octeon_reserve32_memory);
	seq_printf(m, "32bit_shared_mem_size: 0x%x\n",
		   (long int) octeon_reserve32_memory ? CONFIG_CAVIUM_RESERVE32 << 20 : 0);
#else
	seq_printf(m, "32bit_shared_mem_base: 0x%lx\n", 0ul);
	seq_printf(m, "32bit_shared_mem_size: 0x%x\n", 0);
#endif
	return 0;
}

/**
 * /proc/octeon_info was openned. Use the single_open iterator
 *
 * @param inode
 * @param file
 * @return
 */
static int octeon_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, octeon_info_show, NULL);
}

static const struct file_operations octeon_info_operations = {
	.open = octeon_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/**
 * Module initialization
 *
 * @return
 */
static int __init octeon_info_init(void)
{
	struct proc_dir_entry *entry;

	entry =	proc_create("octeon_info", S_IRUGO,
			    NULL, &octeon_info_operations);
	if (entry == NULL)
		return -ENODEV;

	return 0;
}

/**
 * Module cleanup
 *
 * @return
 */
static void __exit octeon_info_cleanup(void)
{
	remove_proc_entry("octeon_info", NULL);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Inc. <support@cavium.com>");
MODULE_DESCRIPTION("Cavium Inc. OCTEON information interface.");
module_init(octeon_info_init);
module_exit(octeon_info_cleanup);
