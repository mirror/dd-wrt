/*
 * raid.c
 *
 * Copyright (C) 2018 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_RAID
#include <stdlib.h>
#include <ddnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <fcntl.h>

void fscheck_main(int argc, char *argv[])
{
	int i = 0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		if (!*raid)
			break;
		char *type = nvram_nget("raidtype%d", i);
		char *poolname = nvram_nget("raidname%d", i);
		if (!strcmp(type, "zfs")) {
			dd_loginfo("raid", "start ZFS Scrubbing");
			eval("zpool", "scrub", poolname);
		}
		if (!strcmp(type, "md")) {
			dd_loginfo("raid", "start checking MD Raid");
			sysprintf("echo check > /sys/block/md%d/md/sync_action", i);
		}
		i++;
	}
}

int try_mount(const char *fs, const char *dev, const char *opt, const char *mountpoint)
{
	int ret = 0;
	int count = 5;
	if (count-- > 0) {
		if (opt)
			ret = eval("mount", "-t", fs, dev, "-o", opt, mountpoint);
		else
			ret = eval("mount", "-t", fs, dev, mountpoint);
		if (!ret)
			return 0;
		sleep(1);
	}
	return ret;
}
void stop_raid(void)
{
	// cannot be unloaded
}

void start_raid_internal(void)
{
	static int raid_running = 0;
	if (raid_running)
		return;
	nvram_set("raid_interrupt", "0");
	raid_running = 1;
again:;
	sleep(5);
	if (nvram_match("raid_interrupt","1") {
		nvram_set("raid_interrupt", "0");
		goto again;
	}
	int i = 0;
	int zfs = 0;
	int md = 0;
	int btrfs = 0;
	int xfs = 0;
	int ext2 = 0;
	int ext4 = 0;
	int apfs = 0;
	int exfat = 0;
	int fat32 = 0;
	int ntfs = 0;
	int todo = 0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *type = nvram_nget("raidtype%d", i);
		char *fs = nvram_nget("raidfs%d", i);
		char *done = nvram_nget("raiddone%d", i);
		if (!strcmp(type, "md")) {
			md = 1;
			if (!strcmp(fs, "btrfs"))
				btrfs = 1;
			if (!strcmp(fs, "exfat"))
				exfat = 1;
			if (!strcmp(fs, "fat32"))
				fat32 = 1;
			if (!strcmp(fs, "xfs"))
				xfs = 1;
			if (!strcmp(fs, "apfs"))
				apfs = 1;
			if (!strcmp(fs, "ntfs"))
				ntfs = 1;
			if (!strcmp(fs, "zfs"))
				zfs = 1;
			if (!strcmp(fs, "ext2"))
				ext2 = 1;
			else if (!strncmp(fs, "ext", 3))
				ext4 = 1;
		}
		if (!strcmp(type, "zfs"))
			zfs = 1;
		if (!strcmp(type, "btrfs"))
			btrfs = 1;
		if (!*raid)
			break;
		todo = 1;
		i++;
	}
	if (i == 0) {
		raid_running = 1;
		return;
	}
	writeprocsys("vm/min_free_kbytes", nvram_default_get("vm.min_free_kbytes", "65536"));
	writeprocsys("vm/vfs_cache_pressure", nvram_default_get("vm.vfs_cache_pressure", "10000"));
	writeprocsys("vm/dirty_expire_centisecs", nvram_default_get("vm.dirty_expire_centisecs", "100"));
	writeprocsys("vm/dirty_writeback_centisecs", nvram_default_get("vm.dirty_writeback_centisecs", "100"));

	int cpucount = getlogicalcores();
	if (todo) {
		eval("service", "cron", "stop");
		eval("service", "samba3", "stop");
		eval("service", "nfs", "stop");
		eval("service", "rsync", "stop");
		eval("service", "dlna", "stop");
		eval("service", "ftpsrv", "stop");
#ifdef HAVE_WEBSERVER
		eval("service", "lighttpd", "stop");
#endif
#ifdef HAVE_TRANSMISSION
		eval("service", "transmission", "stop");
#endif
#ifdef HAVE_PLEX
		eval("service", "plex", "stop");
#endif
		eval("service", "run_rc_usb", "stop");
	}
	if (md) {
		insmod("libcrc32c crc32c_generic crc32_generic");
		insmod("dm-mod");
		insmod("async_tx");
		insmod("async_memcpy");
		insmod("xor-neon"); // for arm only
		insmod("xor");
		insmod("async_xor");
		insmod("raid6_pq");
		insmod("async_pq");
		insmod("async_raid6_recov");
		insmod("md-mod");
		insmod("raid456");
		insmod("dm-raid");
		insmod("raid0");
		insmod("raid1");
		insmod("raid10");
		dd_loginfo("raid", "MD raid modules successfully loaded");
	}
	if (zfs) {
		insmod("spl");
		char zfs_threads[32];
		sprintf(zfs_threads, "zvol_threads=%d", cpucount);
		eval("insmod", "zfs", zfs_threads);
		dd_loginfo("raid", "ZFS modules successfully loaded (%d threads)", cpucount);
	}
	if (btrfs) {
		insmod("libcrc32c crc32c_generic crc32_generic lzo_compress lzo_decompress xxhash zstd_common zstd_compress zstd_decompress raid6_pq xor-neon xor btrfs");
		dd_loginfo("raid", "BTRFS modules successfully loaded");
	}
	if (ntfs) {
#ifdef HAVE_LEGACY_KERNEL
		insmod("fuse");
		dd_loginfo("raid", "NTFS / FUSE modules successfully loaded");
#else
		insmod("antfs");
		insmod("ntfs3");
		insmod("ntfsplus");
		dd_loginfo("raid", "NTFS modules successfully loaded");
#endif
	}
	if (xfs) {
		insmod("xfs");
		dd_loginfo("raid", "XFS modules successfully loaded");
	}
	if (apfs) {
		insmod("libcrc32c apfs");
		dd_loginfo("raid", "APFS modules successfully loaded");
	}
	if (ext4 || ext2) {
		insmod("crc16 mbcache ext2 jbd jbd2 ext3 ext4");
		dd_loginfo("raid", "EXT4 modules successfully loaded");
	}
	if (ext2) {
		insmod("mbcache ext2");
		dd_loginfo("raid", "EXT2 modules successfully loaded");
	}
	if (exfat) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("exfat");
		dd_loginfo("raid", "EXFAT modules successfully loaded");
	}
	if (fat32) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("fat");
		insmod("vfat");
		insmod("msdos");
		dd_loginfo("raid", "FAT32 modules successfully loaded");
	}

	i = 0;
	char *raid = nvram_nget("raid%d", i);
	const char *next;
	char drive[64];
	foreach(drive, raid, next) {
		eval("hdparm", "-S", "242", drive);
		eval("blockdev", "--setra", nvram_safe_get("drive_ra"), drive);
	}
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		if (!*raid)
			break;
		char *level = nvram_nget("raidlevel%d", i);
		char *done = nvram_nget("raiddone%d", i);
		char *type = nvram_nget("raidtype%d", i);
		char *poolname = nvram_nget("raidname%d", i);
		char fname[32];
		char pname[128];
		sprintf(fname, "/dev/md%d", i);
		sprintf(pname, "/tmp/mnt/%s", poolname);
		if (strcmp(done, "1")) {
			int drives = 0;
			foreach(drive, raid, next) {
				drives++;
				umount(drive);
			}
			umount(fname);
			eval("mdadm", "--stop", fname);
			eval("zpool", "destroy", pname);
			if (!strcmp(type, "md")) {
				dd_loginfo("raid", "creating MD Raid %s", fname);
				sysprintf("mdadm --create --assume-clean %s --level=%s --raid-devices=%d --run %s", fname, level,
					  drives, raid);
				wait_file_exists(fname, 5, 0);
				if (nvram_nmatch("ext4", "raidfs%d", i)) {
					eval("mkfs.ext4", "-F", "-E", "lazy_itable_init=1", "-L", poolname, fname);
				}
				if (nvram_nmatch("ext2", "raidfs%d", i)) {
					eval("mkfs.ext2", "-F", "-E", "lazy_itable_init=1", "-L", poolname, fname);
				}
				if (nvram_nmatch("ext3", "raidfs%d", i)) {
					eval("mkfs.ext3", "-F", "-E", "lazy_itable_init=1", "-L", poolname, fname);
				}
				if (nvram_nmatch("xfs", "raidfs%d", i)) {
					eval("mkfs.xfs", "-f", "-L", poolname, fname);
				}
				if (nvram_nmatch("apfs", "raidfs%d", i)) {
					eval("mkfs.apfs", "-s", "-L", poolname, fname);
				}
				if (nvram_nmatch("btrfs", "raidfs%d", i)) {
					eval("mkfs.btrfs", "-f", "-L", poolname, fname);
				}
				if (nvram_nmatch("exfat", "raidfs%d", i)) {
					eval("mkfs.exfat", "-n", poolname, fname);
				}
				if (nvram_nmatch("fat32", "raidfs%d", i)) {
					eval("mkfs.fat", "-F", "32", "-n", poolname, fname);
				}
				if (nvram_nmatch("ntfs", "raidfs%d", i)) {
					eval("mkfs.ntfs", "-Q", "-F", "-L", poolname, fname);
				}
				if (nvram_nmatch("zfs", "raidfs%d", i)) {
					sysprintf("zpool create -f -m \"%s\" \"%s\" %s", pname, poolname, fname);
				}
			}
			if (!strcmp(type, "btrfs")) {
				if (!strcmp(level, "0"))
					sysprintf("mkfs.btrfs -f -d raid0 %s", raid);
				if (!strcmp(level, "1"))
					sysprintf("mkfs.btrfs -f -d raid1 %s", raid);
				if (!strcmp(level, "5"))
					sysprintf("mkfs.btrfs -f -d raid5 %s", raid);
				if (!strcmp(level, "6"))
					sysprintf("mkfs.btrfs -f -d raid6 %s", raid);
				if (!strcmp(level, "10"))
					sysprintf("mkfs.btrfs -f -d raid10 %s", raid);
			}
			if (!strcmp(type, "zfs")) {
				dd_loginfo("raid", "creating ZFS Pool %s", poolname);
				eval("mkdir", "-p", pname);
				if (!strcmp(level, "1"))
					sysprintf("zpool create -f -m \"%s\" \"%s\" mirror %s", pname, poolname, raid);
				if (!strcmp(level, "5"))
					sysprintf("zpool create -f -m \"%s\" \"%s\" raidz1 %s", pname, poolname, raid);
				if (!strcmp(level, "6"))
					sysprintf("zpool create -f -m \"%s\" \"%s\" raidz2 %s", pname, poolname, raid);
				if (!strcmp(level, "z3"))
					sysprintf("zpool create -f -m \"%s\" \"%s\" raidz3 %s", pname, poolname, raid);
				if (!strcmp(level, "0"))
					sysprintf("zpool create -f -m \"%s\" \"%s\" %s", pname, poolname, raid);
			}
			/* reread partition table */
			foreach(drive, raid, next) {
				int fd = open(drive, O_RDONLY | O_NONBLOCK);
				ioctl(fd, BLKRRPART, NULL);
				close(fd);
			}
		}
		if (!strcmp(type, "zfs")) {
			eval("mkdir", "-p", pname);
			eval("zpool", "import", "-a", "-d", "/dev");
			eval("zpool", "upgrade", poolname);
			eval("zfs", "set", "checksum=blake3", poolname);
			eval("zfs", "mount", poolname);
			if (nvram_nmatch("zle", "raidlz%d", i))
				eval("zfs", "set", "compression=zle", poolname);
			else if (nvram_nmatch("lz4", "raidlz%d", i))
				eval("zfs", "set", "compression=lz4", poolname);
			else if (nvram_nmatch("gzip", "raidlz%d", i)) {
				if (nvram_nmatch("0", "raidlzlevel%d", i))
					eval("zfs", "set", "compression=gzip", poolname);
				else
					sysprintf("zfs set compression=gzip-%s %s", nvram_nget("raidlzlevel%d", i), poolname);
			} else if (nvram_nmatch("lzjb", "raidlz%d", i))
				eval("zfs", "set", "compression=lzjb", poolname);
			else if (nvram_nmatch("zstd", "raidlz%d", i)) {
				if (nvram_nmatch("0", "raidlzlevel%d", i))
					eval("zfs", "set", "compression=zstd", poolname);
				else
					sysprintf("zfs set compression=zstd-%s %s", nvram_nget("raidlzlevel%d", i), poolname);
			} else
				eval("zfs", "set", "compression=off", poolname);
			if (nvram_nmatch("1", "raiddedup%d", i))
				eval("zfs", "set", "dedup=on", poolname);
			else
				eval("zfs", "set", "dedup=off", poolname);

			nvram_set("usb_reason", "zfs_pool_add");
			nvram_set("usb_dev", raid);
			eval("service", "run_rc_usb", "start");
		}
		if (!strcmp(type, "md")) {
			wait_file_exists(fname, 1, 0);
			// disable NCQ
			foreach(drive, raid, next) {
				wait_file_exists(drive, 1, 0);
				char *tmp = strrchr(drive, '/');
				if (!tmp)
					tmp = drive;
				else
					tmp++;
				sysprintf("echo 1 > /sys/block/%s/device/queue_depth", tmp);
			}
			sysprintf("mdadm --assemble %s %s", fname, raid);
			wait_file_exists(fname, 5, 0);
			sysprintf("echo 32768 > /sys/block/md%d/md/stripe_cache_size", i);
			writeprocsys("dev/raid/speed_limit_max", nvram_default_get("dev.raid.speed_limit_max", "10000000"));
			eval("mkdir", "-p", pname);
			if (nvram_nmatch("ext4", "raidfs%d", i)) {
				eval("fsck.ext4", "-p", fname);
				try_mount("ext4", fname, "init_itable=0,nobarrier,noatime,nobh,nodiratime,barrier=0", pname);
			}
			if (nvram_nmatch("ext2", "raidfs%d", i)) {
				eval("fsck.ext2", "-p", fname);
				try_mount("ext2", fname, "nobarrier,noatime,nobh,nodiratime,barrier=0", pname);
			}
			if (nvram_nmatch("ext3", "raidfs%d", i)) {
				eval("fsck.ext3", "-p", fname);
				try_mount("ext3", fname, "nobarrier,noatime,nobh,nodiratime,barrier=0", pname);
			}
			if (nvram_nmatch("xfs", "raidfs%d", i)) {
				try_mount("xfs", fname, NULL, pname);
			}
			if (nvram_nmatch("btrfs", "raidfs%d", i)) {
				try_mount("btrfs", fname, NULL, pname);
			}
			if (nvram_nmatch("exfat", "raidfs%d", i)) {
				eval("fsck.exfat", "-p", fname);
				try_mount("exfat", fname, "iocharset=utf8", pname);
			}
			if (nvram_nmatch("apfs", "raidfs%d", i)) {
				try_mount("apfs", fname, NULL, pname);
			}
			if (nvram_nmatch("fat32", "raidfs%d", i)) {
				eval("fsck.vfat", "-p", fname);
				try_mount("vfat", fname, "iocharset=utf8", pname);
			}
			if (nvram_nmatch("ntfs", "raidfs%d", i)) {
#ifdef HAVE_LEGACY_KERNEL
				sysprintf("ntfs-3g -o compression,direct_io,big_writes %s \"%s\"", fname, pname);
#else
#ifdef HAVE_NTFS3
				if (try_mount("ntfsplus", fname, "nls=utf8,noatime", pname))
					try_mount("ntfs3", fname, "nls=utf8,noatime", pname);
#else
				try_mount("antfs", fname, "utf8", pname);
#endif
#endif
			}
			if (nvram_nmatch("zfs", "raidfs%d", i)) {
				eval("zpool", "import", "-a", "-d", "/dev");
				eval("zfs", "mount", poolname);
			}
			nvram_set("usb_reason", "md_raid_add");
			nvram_set("usb_dev", poolname);
			eval("service", "run_rc_usb", "start");
		}
		if (!strcmp(type, "btrfs")) {
			char *r = strdup(raid);
			strcpy(r, raid);
			strstrtok(r, ' ');
			sysprintf("mkdir -p \"%s\"", pname);

			if (nvram_nmatch("lzo", "raidlz%d", i))
				sysprintf("mount -t btrfs -o compression=lzo %s \"%s\"", r, pname);
			else if (nvram_nmatch("zstd", "raidlz%d", i))
				sysprintf("mount -t btrfs -o compression=zstd %s \"%s\"", r, pname);
			else if (nvram_nmatch("gzip", "raidlz%d", i)) {
				if (nvram_nmatch("0", "raidlzlevel%d", i))
					sysprintf("mount -t btrfs -o compression=gzip %s \"%s\"", r, pname);
				else
					sysprintf("mount -t btrfs -o compression=gzip:%s %s \"%s\"", nvram_nget("raidlzlevel%d", i),
						  r, pname);
			} else
				sysprintf("mount -t btrfs %s \"%s\"", r, pname);
			free(r);
			nvram_set("usb_reason", "btrfs_raid_add");
			nvram_set("usb_dev", poolname);
			eval("service", "run_rc_usb", "start");
		}

		if (!strcmp(done, "0")) {
			nvram_nseti(1, "raiddone%d", i);
			nvram_async_commit();
		}

		i++;
	}
	if (todo) {
		eval("service", "cron", "start");
		eval("service", "samba3", "start");
		eval("service", "nfs", "start");
		eval("service", "rsync", "start");
		eval("service", "ftpsrv", "start");
		eval("service", "dlna", "start");
#ifdef HAVE_WEBSERVER
		eval("service", "lighttpd", "start");
#endif
#ifdef HAVE_TRANSMISSION
		eval("service", "transmission", "start");
#endif
#ifdef HAVE_PLEX
		eval("service", "plex", "start");
#endif
	}
	raid_running = 0;
}

void start_raid(void)
{
	sysprintf("service raid_internal start&");
}

#endif
