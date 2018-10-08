/*
 * raid.c
 *
 * Copyright (C) 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_raid(void)
{
	// cannot be unloaded
}

void start_raid(void)
{
	int i = 0;
	int zfs = 0;
	int md = 0;
	int btrfs = 0;
	int xfs = 0;
	int ext2 = 0;
	int ext4 = 0;
	int exfat = 0;
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
			if (!strcmp(fs, "xfs"))
				xfs = 1;
			if (!strcmp(fs, "ntfs"))
				ntfs = 1;
			if (!strcmp(fs, "ext2"))
				ext2 = 1;
			else if (!strncmp(fs, "ext", 3))
				ext4 = 1;

		}
		if (!strcmp(type, "zfs"))
			zfs = 1;
		if (!strcmp(type, "btrfs"))
			btrfs = 1;
		if (!strlen(raid))
			break;
		if (!strcmp(done, "0"))
			todo = 1;
		i++;
	}
	if (i == 0)
		return;
	if (todo) {
		eval("stopservice", "cron", "-f");
		eval("stopservice", "samba3", "-f");
		eval("stopservice", "dlna", "-f");
		eval("stopservice", "ftpsrv", "-f");
#ifdef HAVE_WEBSERVER
		eval("stopservice", "lighttpd", "-f");
#endif
#ifdef HAVE_TRANSMISSION
		eval("stopservice", "transmission", "-f");
#endif
	}
	if (md) {
		insmod("libcrc32c crc32c_generic crc32_generic");
		insmod("dm-mod");
		insmod("async_tx");
		insmod("async_memcpy");
		insmod("xor-neon");	// for arm only
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
		dd_loginfo("raid", "MD raid modules successfully loaded\n");
	}
	if (zfs) {
		insmod("spl");
		insmod("icp");
		insmod("znvpair");
		insmod("zcommon");
		insmod("zunicode");
		insmod("zavl");
		insmod("zfs");
		dd_loginfo("raid", "ZFS modules successfully loaded\n");
	}
	if (btrfs) {
		insmod("libcrc32c crc32c_generic crc32_generic lzo_compress lzo_decompress xxhash zstd_compress zstd_decompress raid6_pq xor-neon xor btrfs");
		dd_loginfo("raid", "BTRFS modules successfully loaded\n");
	}
	if (ntfs) {
		insmod("fuse");
		dd_loginfo("raid", "NTFS / FUSE modules successfully loaded\n");
	}
	if (xfs) {
		insmod("xfs");
		dd_loginfo("raid", "XFS modules successfully loaded\n");
	}
	if (ext4 || ext2) {
		insmod("crc16 mbcache ext2 jbd jbd2 ext3 ext4");
		dd_loginfo("raid", "EXT4 modules successfully loaded\n");
	}
	if (ext2) {
		insmod("mbcache ext2");
		dd_loginfo("raid", "EXT2 modules successfully loaded\n");
	}
	if (exfat) {
		insmod("nls_base nls_cp932 nls_cp936 nls_cp950 nls_cp437 nls_iso8859-1 nls_iso8859-2 nls_utf8");
		insmod("exfat");
		dd_loginfo("raid", "EXFAT modules successfully loaded\n");
	}
	i = 0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		if (!strlen(raid))
			break;
		char *level = nvram_nget("raidlevel%d", i);
		char *done = nvram_nget("raiddone%d", i);
		char *type = nvram_nget("raidtype%d", i);
		char *poolname = nvram_nget("raidname%d", i);
		if (strcmp(done, "1")) {
			char *next;
			char drive[64];
			int drives = 0;
			foreach(drive, raid, next) {
				drives++;
				sysprintf("umount %s", drive);
			}
			sysprintf("umount /dev/md%d\n", i);
			sysprintf("mdadm --stop /dev/md%d\n", i);
			sysprintf("zpool destroy %s", poolname);
			if (!strcmp(type, "md")) {
				dd_loginfo("raid", "creating MD Raid /dev/md%d", i);
				sysprintf("mdadm --create /dev/md%d --level=%s --raid-devices=%d --run %s", i, level, drives, raid);
				if (nvram_nmatch("ext4", "raidfs%d", i)) {
					sysprintf("mkfs.ext4 -F -E lazy_itable_init=1 -L \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("ext2", "raidfs%d", i)) {
					sysprintf("mkfs.ext2 -F -E lazy_itable_init=1 -L \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("ext3", "raidfs%d", i)) {
					sysprintf("mkfs.ext3 -F -E lazy_itable_init=1 -L \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("xfs", "raidfs%d", i)) {
					sysprintf("mkfs.xfs -f -L \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("btrfs", "raidfs%d", i)) {
					sysprintf("mkfs.btrfs -f -L \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("exfat", "raidfs%d", i)) {
					sysprintf("mkfs.exfat -n \"%s\" /dev/md%d", poolname, i);
				}
				if (nvram_nmatch("ntfs", "raidfs%d", i)) {
					sysprintf("mkfs.ntfs -Q -F -L \"%s\" /dev/md%d", poolname, i);
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
				sysprintf("mkdir -p /tmp/mnt/%s", poolname);
				if (!strcmp(level, "1"))
					sysprintf("zpool create -f -m /tmp/mnt/%s %s mirror %s", poolname, poolname, raid);
				if (!strcmp(level, "5"))
					sysprintf("zpool create -f -m /tmp/mnt/%s %s raidz1 %s", poolname, poolname, raid);
				if (!strcmp(level, "6"))
					sysprintf("zpool create -f -m /tmp/mnt/%s %s raidz2 %s", poolname, poolname, raid);
				if (!strcmp(level, "z3"))
					sysprintf("zpool create -f -m /tmp/mnt/%s %s raidz3 %s", poolname, poolname, raid);
				if (!strcmp(level, "0"))
					sysprintf("zpool create -f -m /tmp/mnt/%s %s %s", poolname, poolname, raid);
			}
		}
		if (!strcmp(type, "zfs")) {
			if (nvram_nmatch("1", "raidlz%d", i))
				sysprintf("zfs set compression=lz4 %s", poolname);
			else
				sysprintf("zfs set compression=off %s", poolname);
			if (nvram_nmatch("1", "raiddedup%d", i))
				sysprintf("zfs set dedup=on %s", poolname);
			else
				sysprintf("zfs set dedup=off %s", poolname);
			sysprintf("mkdir -p /tmp/mnt/%s", poolname);
			sysprintf("zpool import -a -d /dev", poolname);
		}
		if (!strcmp(type, "md")) {
			sysprintf("mdadm --assemble /dev/md%d %s", i, raid);
			sysprintf("mkdir -p /tmp/mnt/%s", poolname);
			if (nvram_nmatch("ext4", "raidfs%d", i)) {
				sysprintf("mount -t ext4 /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("ext2", "raidfs%d", i)) {
				sysprintf("mount -t ext2 /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("ext3", "raidfs%d", i)) {
				sysprintf("mount -t ext3 /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("xfs", "raidfs%d", i)) {
				sysprintf("mount -t xfs /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("btrfs", "raidfs%d", i)) {
				sysprintf("mount -t btrfs /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("exfat", "raidfs%d", i)) {
				sysprintf("mount -t exfat /dev/md%d /tmp/mnt/%s", i, poolname);
			}
			if (nvram_nmatch("ntfs", "raidfs%d", i)) {
				sysprintf("ntfs-3g -o compression,direct_io,big_writes /dev/md%d /tmp/mnt/%s", i, poolname);
			}
		}
		if (!strcmp(type, "btrfs")) {
			char *r = malloc(strlen(raid) + 1);
			strcpy(r, raid);
			strstrtok(r, ' ');
			sysprintf("mkdir -p /tmp/mnt/%s", poolname);
			sysprintf("mount -t btrfs %s /tmp/mnt/%s", r, poolname);
			free(r);
		}

		if (!strcmp(done, "0")) {
			nvram_nset("1", "raiddone%d", i);
			nvram_commit();
		}
		i++;
	}
	if (todo) {
		eval("startservice_f", "cron", "-f");
		eval("startservice_f", "samba3", "-f");
		eval("startservice_f", "dlna", "-f");
		eval("startservice_f", "ftpsrv", "-f");
#ifdef HAVE_WEBSERVER
		eval("startservice_f", "lighttpd", "-f");
#endif
#ifdef HAVE_TRANSMISSION
		eval("startservice_f", "transmission", "-f");
#endif
	}

}

#endif
