/*
 * usb_hotplug_adv.c
 *
 * Copyright (C) 2013 Richard Schneidt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <typedefs.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>
#include <md5.h>

void run_opt(void);
static int usb_process_path(char *path, int host, char *part, char *devpath);
static void usb_unmount(char *dev);
static int usb_add_ufd(char *link, int host, char *devpath, int mode);

#define DUMPFILE "/tmp/disktype.dump"
#define PARTFILE "/tmp/part.dump"
#define MOUNTSTAT "/tmp/mounting"

static void run_on_mount(char *p)
{
	struct stat tmp_stat;
	char path[128];
	md5_ctx_t MD;

	if (!nvram_match("usb_runonmount", "")) {
		snprintf(path, sizeof(path), "%s %s", nvram_safe_get("usb_runonmount"), p);
		if (stat(path, &tmp_stat) == 0) //file exists
		{
			setenv("PATH",
			       "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin",
			       1);
			setenv("LD_LIBRARY_PATH",
			       "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib", 1);

			system(path);
		}
	}
	snprintf(path, sizeof(path), "%s/nvrambak.bin", p);
	FILE *fp = fopen(path, "rb");
	if (fp) {
		unsigned char hash[32];
		unsigned char shash[32 * 2];
		size_t size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);
		char *mem = malloc(size);
		fread(mem, size, 1, fp);
		fclose(fp);
		dd_md5_begin(&MD);
		dd_md5_hash(mem, size, &MD);
		dd_md5_end((unsigned char *)hash, &MD);
		int i;
		for (i = 0; i < size; i++)
			sprintf(&shash[2 * i], "%02x", hash[i]);

		if (!nvram_match("backup_hash", shash)) {
			nvram_restore(path, 0);
			nvram_set("backup_hash", shash);
			nvram_commit();
			sys_reboot();
		}
	}

	return;
}

/* TODO improvement: use procfs to identify pids that have openfiles on externel discs and then stop them before umount*/
static bool usb_stopservices()
{
	eval("stopservice", "cron");
	eval("stopservice", "samba3");
	eval("stopservice", "nfs");
	eval("stopservice", "rsync");
	eval("stopservice", "dlna");
	eval("stopservice", "ftpsrv");
#ifdef HAVE_WEBSERVER
	eval("stopservice", "lighttpd");
#endif
#ifdef HAVE_TRANSMISSION
	eval("stopservice", "transmission");
#endif
#ifdef HAVE_PLEX
	eval("stopservice", "plex");
#endif
	eval("stopservice", "run_rc_usb");
	return 0;
}

/* when adding external media some services should be restarted, e.g. minidlna in order to scan for media files*/
static bool usb_startservices(void)
{
	eval("startservice", "cron", "-f");
	eval("startservice", "samba3", "-f");
	eval("startservice", "nfs", "-f");
	eval("startservice", "rsync", "-f");
	eval("startservice", "dlna", "-f");
	eval("startservice", "ftpsrv", "-f");
#ifdef HAVE_WEBSERVER
	eval("startservice", "lighttpd", "-f");
#endif
#ifdef HAVE_TRANSMISSION
	eval("startservice", "transmission", "-f");
#endif
#ifdef HAVE_PLEX
	eval("startservice", "plex", "-f");
#endif
	char *next;
	char *services = nvram_safe_get("custom_configs");
	char service[32];
	foreach(service, services, next)
	{
		eval("service", service, "stop");
		eval("service", service, "start");
	}
	return 0;
}

//Kernel 2.6.x
void start_hotplug_usb(void)
{
	char *device, *interface, *devpath;
	char *action;
	char path[256];
	char link[128];
	int i, host;
	struct stat tmp_stat;

	int class, subclass, protocol;

	if (!(action = getenv("ACTION")) || !(device = getenv("TYPE")))
		return;

	if (!(nvram_matchi("usb_automnt", 1)))
		return;

	sscanf(device, "%d/%d/%d", &class, &subclass, &protocol);

	if (class == 0) {
		if (!(interface = getenv("INTERFACE")))
			return;
		sscanf(interface, "%d/%d/%d", &class, &subclass, &protocol);
	}

	/* 
	 * If a new USB device is added and it is of storage class 
	 */
	if (class == 8 && subclass == 6) {
		devpath = getenv("DEVPATH");

		/* to avoid problems with multiple discs sleep until the first disc is done */
		if (stat(MOUNTSTAT, &tmp_stat) == 0) {
			for (i = 0; i <= 10; i++)
				sleep(1);
		} else {
			eval("touch", MOUNTSTAT);
		}

		if (!strcmp(action,
			    "add")) { //let the disc settle before we mount
			for (i = 0; i <= 10; i++)
				sleep(1);
		}

		/* loop through sysfs to find out the correct lun0 */
		for (host = 0; host < 10; host++) {
			sprintf(path, "/sys/%s/host%d", devpath, host);
			if (stat(path, &tmp_stat) == 0) {
				sprintf(link, "/dev/scsi/host%d/bus0/target0/lun0", host);
				if (!strcmp(action, "add")) {
					usb_add_ufd(link, host, devpath, 0);
					unlink(MOUNTSTAT);
				}
				break;
			}
		}

		if (!strcmp(action, "remove"))
			usb_unmount(devpath);
	}

	/*	if (class == 0 && subclass == 0) {
		if (!strcmp(action, "add"))
			usb_add_ufd(NULL);
		if (!strcmp(action, "remove"))
			usb_unmount(NULL);
	}*/

	return;
}

/* Optimize performance */
#define READ_AHEAD_KB_BUF "1024"
#define READ_AHEAD_CONF "/sys/block/%s/queue/read_ahead_kb"

static void optimize_block_device(char *devname)
{
	char blkdev[8] = { 0 };
	char read_ahead_conf[64] = { 0 };
	int err;

	bzero(blkdev, sizeof(blkdev));
	strncpy(blkdev, devname, 3);
	sprintf(read_ahead_conf, READ_AHEAD_CONF, blkdev);
	writestr(read_ahead_conf, READ_AHEAD_KB_BUF);
}

//Kernel 3.x

void stop_hotplug_block(void)
{
}

void start_hotplug_block(void)
{
	char *devpath;
	char *action;
	int i;
	int c;
	char part[128];
	char devname[32];
	if (!(devpath = getenv("DEVPATH")))
		return;
	if (!(action = getenv("ACTION")))
		return;
	if (!(nvram_matchi("usb_automnt", 1)))
		return;

	// e.g. /devices/pci0000:00/0000:00:04.1/usb1/1-1/1-1.2/1-1.2:1.0/host1/target1:0:0/1:0:0:0/block/sda/sda1
	//sysprintf("echo hotplug_block_block action %s devpath %s >> /tmp/hotplugs", action, devpath);
	int len;
	if (!devpath)
		return;
	if (!*devpath)
		return;
	len = strlen(devpath);
	char *devp = strrchr(devpath, '/');
	if (!devp)
		return;
	if (!*devp)
		return;
	strncpy(part, devp + 1, sizeof(part) - 1);

	optimize_block_device(devp + 1);

	if (strncmp(part, "sd", 2) && strncmp(part, "mmc", 3) && strncmp(part, "hd", 2) && strncmp(part, "sr", 2) &&
	    strncmp(part, "md", 2) && strncmp(part, "nvme", 4))
		return;

	if (!strcmp(action, "add")) {
		usb_stopservices();
	}

	sprintf(devname, "/dev/%s", part);
	sysprintf("/usr/sbin/disktype %s", devname);
	eval("hdparm", "-S", "242", devname);
	eval("blockdev", "--setra", nvram_safe_get("drive_ra"), devname);
	if (strlen(part) != 4 && strlen(part) != 7) {
		sysprintf("echo  \"<b>Partition %s:</b>\" >> /tmp/disk/%s", part, part);
		sysprintf("/usr/sbin/disktype %s >> /tmp/disk/%s", devname, part);
	}
	if (!strcmp(action, "add"))
		usb_add_ufd(NULL, 0, devname, 1);
	if (!strcmp(action, "remove"))
		usb_unmount(devname);
	if (!strcmp(action, "add")) {
		//finally start services again after mounting all partitions for this drive
		usb_startservices();
	}

	return;
}

static bool usb_load_modules(char *fs)
{
#ifdef HAVE_NTFS3G
	if (!strcmp(fs, "ntfs")) {
#ifdef HAVE_LEGACY_KERNEL
		insmod("fuse");
#else
		insmod("antfs");
		insmod("ntfs3");
#endif
	}
#endif
	if (!strcmp(fs, "ext2")) {
		insmod("mbcache ext2");
	}
#ifdef HAVE_USB_ADVANCED
	if (!strcmp(fs, "ext3") || !strcmp(fs, "ext4")) {
		insmod("crc16 crc32c_generic crc32_generic mbcache ext2 jbd jbd2 ext3 ext4");
	}
	if (!strcmp(fs, "btrfs")) {
		insmod("libcrc32c crc32c_generic crc32_generic lzo_compress lzo_decompress xxhash zstd_common zstd_compress zstd_decompress raid6_pq xor-neon xor btrfs");
	}
	if (!strcmp(fs, "hfs")) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("hfs");
	}
	if (!strcmp(fs, "hfsplus")) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("hfsplus");
	}
#endif
	if (!strcmp(fs, "vfat")) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("fat");
		insmod("vfat");
		insmod("msdos");
	}
	if (!strcmp(fs, "exfat")) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("exfat");
	}
	if (!strcmp(fs, "xfs")) {
		insmod("xfs");
	}
	if (!strcmp(fs, "apfs")) {
		insmod("libcrc32c apfs");
	}
	if (!strcmp(fs, "f2fs")) {
		insmod("crc32 libcrc32c crc32c_generic crc32_generic f2fs");
	}
	if (!strcmp(fs, "udf")) {
		insmod("crc-itu-t udf");
	}
	if (!strcmp(fs, "iso9660")) {
		insmod("isofs");
	}
	return 0;
}

static void do_mount(char *fs, char *path, char *mount_point, char *dev)
{
	int ret = 0;
	int first = 0;
	/* lets start mounting */
#ifdef HAVE_NTFS3G
	if (!strcmp(fs, "ntfs")) {
repeat:;
#ifdef HAVE_LEGACY_KERNEL
		ret = eval("ntfs-3g", "-o", "compression,direct_io,big_writes", path, mount_point);
#else
#ifdef HAVE_NTFS3
		ret = eval("/bin/mount", "-t", "ntfs3", "-o", "nls=utf8,noatime", path, mount_point);
#else
		ret = eval("/bin/mount", "-t", "antfs", "-o", "utf8", path, mount_point);
#endif
#endif
		if (!first && ret) {
			first = 1;
			eval("ntfsfix", "-d", path);
			goto repeat;
		}

	} else
#endif
	if (!strcmp(fs, "vfat")) {
		eval("fsck.vfat","-p",path);
		ret = eval("/bin/mount", "-t", fs, "-o", "iocharset=utf8", path, mount_point);
	} else if (!strcmp(fs, "exfat")) {
		eval("fsck.exfat","-p",path);
		ret = eval("/bin/mount", "-t", fs, "-o", "iocharset=utf8", path, mount_point);
	} else if (!strcmp(fs, "ext4")) 
		eval("fsck.ext4","-p",path);
		ret = eval("/bin/mount", "-t", fs, "-o", "init_itable=0,nobarrier,noatime,nobh,nodiratime,barrier=0", path,
			   mount_point);
	} else if (!strcmp(fs, "ext2")) {
		eval("fsck.ext2","-p",path);
		ret = eval("/bin/mount", "-t", fs, "-o", "nobarrier,noatime,nobh,nodiratime,barrier=0", path, mount_point);
	} else if (!strcmp(fs, "ext3")) {
		eval("fsck.ext3","-p",path);
		ret = eval("/bin/mount", "-t", fs, "-o", "nobarrier,noatime,nobh,nodiratime,barrier=0", path, mount_point);
	} else {
		ret = eval("/bin/mount", "-t", fs, path, mount_point);
	}

	if (ret != 0) { //give it another try
		first = 0;
again:;
#ifdef HAVE_NTFS3G
		if (!strcmp(fs, "ntfs")) {
#ifdef HAVE_LEGACY_KERNEL
			ret = eval("ntfs-3g", "-o", "compression,direct_io,big_writes", path, mount_point);
#else
#ifdef HAVE_NTFS3
			ret = eval("/bin/mount", "-t", "ntfs3", "-o", "nls=utf8,noatime", path, mount_point);
#else
			ret = eval("/bin/mount", "-t", "antfs", "-o", "utf8", path, mount_point);
#endif
#endif
			if (!first && ret) {
				first = 1;
				eval("ntfsfix", "-d", path);
				goto again;
			}
		} else
#endif
			ret = eval("/bin/mount", path, mount_point); //guess fs
	}

	if (!strcmp(fs, "swap")) {
		sysprintf("echo \"<b>%s</b> mounted to <b>%s</b><hr>\"  >> /tmp/disk/%s", path, "swap", dev);
	} else {
		sysprintf("echo \"<b>%s</b> mounted to <b>%s</b><hr>\"  >> /tmp/disk/%s", path, mount_point, dev);
		nvram_set("usb_reason", "blockdev_add");
		nvram_set("usb_dev", mount_point);
		eval("startservice", "run_rc_usb", "-f");
	}
	if (!ret)
		run_on_mount(path);
}

void start_raid(void);

/* 
  *   Mount partition 
  */
static int usb_process_path(char *path, int host, char *part, char *devpath)
{
	int ret = ENOENT;
	FILE *fp = NULL;
	char *fs;
	char line[256];
	char mount_point[32];
	char uuid[48];
	char dev_dir[128];
	char sym_link[256];
	char part_file[32];
	int len = strlen(path);
	int i;
	int found = 0;
	char *dev = &path[len - 4];
	for (i = 0; i < len; i++) { // seek for last occurence
		if (path[i] == '/')
			dev = &path[i + 1];
	}

	if (host == -1) {
		sprintf(part_file, "/tmp/disk/%s", dev);
	} else {
		sprintf(part_file, "/tmp/disk/disc%d-%s", host, part);
	}
	sysprintf("/usr/sbin/disktype %s > %s", path, &part_file);

	/* determine fs */
	fs = "";
	if ((fp = fopen(part_file, "r"))) {
		int linenr = 0;
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (linenr++ == 5)
				break;
			if (strstr(line, "Linux swap")) {
				fs = "swap";
				ret = eval("/sbin/swapon", path);
				sysprintf("echo \"<b>%s</b> mounted to <b>swap</b>\" >> %s", path, DUMPFILE);
			}

			if (strstr(line, "file system")) {
				if (strstr(line, "FAT")) {
					if (strstr(line, "exFAT")) {
						fs = "exfat";
						usb_load_modules(fs);
					} else {
						fs = "vfat";
						usb_load_modules(fs);
					}
				} else if (strstr(line, "Ext2")) {
					fs = "ext4";
					usb_load_modules(fs);
				} else if (strstr(line, "XFS")) {
					fs = "xfs";
					usb_load_modules(fs);
				} else if (strstr(line, "APFS")) {
					fs = "apfs";
					usb_load_modules(fs);
				} else if (strstr(line, "UDF")) {
					fs = "udf";
					usb_load_modules(fs);
				} else if (strstr(line, "ISO9660")) {
					fs = "iso9660";
					usb_load_modules(fs);
				} else if (strstr(line, "Ext3")) {
					fs = "ext4";
					usb_load_modules(fs);
				} else if (strstr(line, "Ext4")) {
					fs = "ext4";
					usb_load_modules(fs);
				} else if (strstr(line, "Btrfs")) {
					fs = "btrfs";
					usb_load_modules(fs);
				} else if (strstr(line, "F2FS")) {
					fs = "f2fs";
					usb_load_modules(fs);
				} else if (strstr(line, "HFS Plus")) {
					fs = "hfsplus";
					usb_load_modules(fs);
				} else if (strstr(line, "HFS")) {
					fs = "hfs";
					usb_load_modules(fs);
				}
#ifdef HAVE_NTFS3G
				else if (strstr(line, "NTFS")) {
					fs = "ntfs";
					usb_load_modules(fs);
				}
#endif
#ifdef HAVE_RAID
				else if (strstr(line, "ZFS")) {
					start_raid();
				} else if (strstr(line, "MD Raid")) {
					start_raid();
				}
#endif
			}
		}
		fclose(fp);
	}

	if (!strcmp(fs, "")) {
		sysprintf("echo \"<b>%s</b> not mounted <b>%s</b><hr>\"  >> /tmp/disk/%s", path, "Unsupported Filesystem", dev);
		return 1;
	}

	char *mntlabel = nvram_nget("%s_label", dev);
	if (strlen(mntlabel)) {
		sprintf(mount_point, "/tmp/mnt/%s", mntlabel);
	} else {
		/* strategy one: mount to default location */
		if (host == -1) //K3
		{
			sprintf(mount_point, "/tmp/mnt/%s", dev);

		} else { //K2.6
			sprintf(mount_point, "/tmp/mnt/disc%d-%s", host, part);
		}
	}

	/* strategy two: mount by partition label, overrides strategy one */
	if ((fp = fopen(part_file, "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, "Jffs") || strstr(line, "JFFS") || strstr(line, "\"jffs")) {
				found = 1;
				do_mount(fs, path, "/jffs", dev);
			}
			sprintf(uuid, "%s", nvram_safe_get("usb_mntjffs"));
			if (strlen(uuid) > 15 && strstr(line, uuid)) {
				found = 1;
				do_mount(fs, path, "/jffs", dev);
			}
			if (strstr(line, "Opt") || strstr(line, "OPT") || strstr(line, "\"opt")) {
				found = 1;
				do_mount(fs, path, "/opt", dev);
				run_opt();
			}
			sprintf(uuid, "%s", nvram_safe_get("usb_mntopt"));
			if (strlen(uuid) > 15 && strstr(line, uuid)) {
				found = 1;
				do_mount(fs, path, "/opt", dev);
				run_opt();
			}
		}
		fclose(fp);
	}

	if (strcmp(fs,
		   "swap")) //don't create dir as swap is not mounted to a dir
		eval("mkdir", "-p", mount_point);

	if (host != -1) {
		/* at the time the kernel (2.6) notifies us, the part entries are gone thus we map devicepath to mounted partitions so we can umount them later on */
		sprintf(dev_dir, "/tmp/%s", devpath);
		eval("mkdir", "-p", dev_dir);
		// e.g. /mnt /tmp/devices/pci0000:00/0000:00:04.1/usb1/1-1/1-1:1.0/part1
		sprintf(sym_link, "%s/%s", dev_dir, part);
		eval("ln", "-s", mount_point, sym_link);
	}
	if (found != 1)
		do_mount(fs, path, mount_point, dev);

	// now we will get a nice ordered dump of all partitions
	sysprintf("cat /tmp/disk/sd* > %s", DUMPFILE);

	/* avoid out of memory problems which could lead to broken wireless, so we limit the minimum free ram everything else can be used for fs cache */
#ifdef HAVE_80211AC
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "20480"));
#elif HAVE_MVEBU
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "65536"));
#elif HAVE_IPQ806X
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "65536"));
#elif HAVE_X86
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "65536"));
#else
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "4096"));
#endif
	writeprocsys("vm/vfs_cache_pressure", nvram_default_get("vm.vfs_cache_pressure", "10000"));
	writeprocsys("vm/dirty_expire_centisecs", nvram_default_get("vm.dirty_expire_centisecs", "100"));
	writeprocsys("vm/dirty_writeback_centisecs", nvram_default_get("vm.dirty_writeback_centisecs", "100"));
	writeprocsys("vm/overcommit_memory", nvram_default_get("vm.overcommit_memory", "2"));
	writeprocsys("vm/overcommit_ratio", nvram_default_get("vm.overcommit_ratio", "80"));

	//      writeprocsys("vm/pagecache_ratio","90");
	//      writeprocsys("vm/swappiness","90");
	//      writeprocsys("vm/overcommit_memory","2");
	//      writeprocsys("vm/overcommit_ratio","145");
	//prepare for optware
	//sysprintf("mkdir -p /jffs/lib/opkg");

	return ret;
}

/* 
 * umount device if user removes the drive by pulling the cable this is no clean umount only removes stale mountpoints
 */
static void usb_unmount(char *devpath)
{
	DIR *dir = NULL;
	char dev_dir[256];
	char sym_link[256];
	struct dirent *entry;

	usb_stopservices();

	writeprocsys("vm/drop_caches", "3"); // flush fs cache

	//K3 code
	eval("umount", devpath);

	//K2.6 code
	sprintf(dev_dir, "/tmp/%s", devpath);
	// /tmp/devices/pci0000:00/0000:00:04.1/usb1/1-1/1-1:1.0/
	dir = opendir(dev_dir);
	if (dir != NULL) {
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, ".", 1)) { //skip . and ..
				/* use the symlinks we created under /tmp to umount the devices partitions */
				sprintf(sym_link, "%s/%s", dev_dir, entry->d_name);
				eval("umount", sym_link);
			}
		}
	}

	usb_startservices();

	return;
}

/* 
* Handle hotplugging of UFD 
*/
static int usb_add_ufd(char *link, int host, char *devpath, int mode)
{
	DIR *dir = NULL;

	char part_link[128];

	struct dirent *entry;

	sysprintf("mkdir -p /tmp/disk/");

	//create directory to store disktype dumps

	if (mode == 1) { //K3
		usb_process_path(devpath, -1, NULL, NULL); //use -1 to signal K3
	} else { //K2.6
		usb_stopservices(); //K3 will start/stop only for a drive not partition
		dir = opendir(link);
		if (dir != NULL) {
			while ((entry = readdir(dir)) != NULL) {
				sprintf(part_link, "%s/%s", link, entry->d_name);
				if (!strncmp(entry->d_name, "disc", 4)) {
					sysprintf("echo  \"<b>Disk %d:</b>\" >> %s", host, DUMPFILE);
					sysprintf("/usr/sbin/disktype %s >> %s", part_link, DUMPFILE);
					sysprintf("echo  \"<b>Mountpoints:</b>\" >> %s", DUMPFILE);
				}

				if (strncmp(entry->d_name, "disc", 4) && strncmp(entry->d_name, ".",
										 1)) { //only get partitions
					usb_process_path(part_link, host, entry->d_name, devpath);
				}
			}
			closedir(dir);
		}
		usb_startservices();
	}

	return 0;
}
