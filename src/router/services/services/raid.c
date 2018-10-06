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
	if (!nvram_matchi("raid_enable", 1))
		return;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *type = nvram_nget("raidtype%d", i);
		if (!strcmp(type, "md"))
			md = 1;
		if (!strcmp(type, "zfs"))
			zfs = 1;
		if (!strlen(raid))
			break;
		i++;
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
	i=0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		if (!strlen(raid))
			break;
		char *level = nvram_nget("raidlevel%d", i);
		char *done = nvram_nget("raiddone%d", i);
		char *type = nvram_nget("raidtype%d", i);
		if (strcmp(done, "1")) {
			char *next;
			char drive[64];
			int drives = 0;
			foreach(drive, raid, next) {
				drives++;
			}
			if (!strcmp(type, "md")) {
				dd_loginfo("raid", "creating MD Raid /dev/md%d", i);
				sysprintf("mdadm --stop /dev/md%d\n",i);
				sysprintf("mdadm --create /dev/md%d --level=%s --raid-devices=%d --run %s", i, level, drives, raid);
				if (nvram_nmatch("ext4", "raidfs%d", i))
					sysprintf("mkfs.ext4 /dev/md%d", i);
				if (nvram_nmatch("ext2", "raidfs%d", i))
					sysprintf("mkfs.ext2 /dev/md%d", i);
				if (nvram_nmatch("xfs", "raidfs%d", i))
					sysprintf("mkfs.xfs /dev/md%d", i);
				if (nvram_nmatch("btrfs", "raidfs%d", i))
					sysprintf("mkfs.btrfs /dev/md%d", i);
			}
			if (!strcmp(type, "btrfs")) {
				if (!strcmp(level, "0"))
				    sysprintf("mkfs.btrfs -d raid0 %s", raid);
				if (!strcmp(level, "1"))
				    sysprintf("mkfs.btrfs -d raid1 %s", raid);
				if (!strcmp(level, "5"))
				    sysprintf("mkfs.btrfs -d raid5 %s", raid);
				if (!strcmp(level, "6"))
				    sysprintf("mkfs.btrfs -d raid6 %s", raid);
				if (!strcmp(level, "10"))
				    sysprintf("mkfs.btrfs -d raid10 %s", raid);

			}
			if (!strcmp(type, "zfs")) {
				char *poolname = nvram_nget("raidname%d", i);
				dd_loginfo("raid", "creating ZFS Pool %s", poolname);
				sysprintf("zpool destroy %s", poolname);
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
			nvram_nset("1", "raiddone%d", i);
			nvram_commit();
		}
		if (!strcmp(type, "zfs")) {
			char *poolname = nvram_nget("raidname%d", i);
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

		i++;
	}

}

#endif
