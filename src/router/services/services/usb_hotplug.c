/* 
 * USB hotplug service    Copyright 2007, Broadcom Corporation  All Rights
 * Reserved.    THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO
 * WARRANTIES OF ANY  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR
 * OTHERWISE. BROADCOM  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT
 * CONCERNING THIS SOFTWARE.    $Id$  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <typedefs.h>
#include <shutils.h>
#include <bcmnvram.h>

static bool usb_ufd_connected(char *str);
static int usb_process_path(char *path, char *fs);
static void usb_unmount(void);
int usb_add_ufd(void);

#define DUMPFILE	"/tmp/disktype.dump"

void start_hotplug_usb(void)
{
	char *device, *interface;
	char *action;
	int class, subclass, protocol;

	if (!(nvram_match("usb_automnt", "1")))
		return;

	if (!(action = getenv("ACTION")) || !(device = getenv("TYPE")))
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
		if (!strcmp(action, "add"))
			usb_add_ufd();
		if (!strcmp(action, "remove"))
			usb_unmount();
	}

	return;
}

    /* 
     *   Check if the UFD is still connected because the links created in
     * /dev/discs are not removed when the UFD is  unplugged.  
     */
static bool usb_ufd_connected(char *str)
{
	uint host_no;
	char proc_file[128];
	FILE *fp;
	char line[256];
	int i;

	/* 
	 * Host no. assigned by scsi driver for this UFD 
	 */
	host_no = atoi(str);
	sprintf(proc_file, "/proc/scsi/usb-storage-%d/%d", host_no, host_no);

	if ((fp = fopen(proc_file, "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, "Attached: Yes")) {
				fclose(fp);
				return TRUE;
			}
		}
		fclose(fp);
	}
	//in 2.6 kernels its a little bit different; find 1st
	for (i = 0; i < 16; i++) {
		sprintf(proc_file, "/proc/scsi/usb-storage/%d", i);
		if (f_exists(proc_file)) {
			return TRUE;
		}
	}
	return FALSE;

}

    /* 
     *   Mount the path and look for the WCN configuration file.  If it
     * exists launch wcnparse to process the configuration.  
     */
static int usb_process_path(char *path, char *fs)
{
	int ret = ENOENT;
	char mount_point[32];

	sprintf(mount_point, "/%s", nvram_default_get("usb_mntpoint", "mnt"));

	ret = eval("/bin/mount", "-t", fs, path, mount_point);

	if (ret != 0)		//give it another try
		ret = eval("/bin/mount", path, mount_point);	//guess fs
	system("echo 4096 > /proc/sys/vm/min_free_kbytes"); // avoid out of memory problems which could lead to broken wireless, so we limit the minimum free ram to 4096. everything else can be used for fs cache
	return ret;
}

static void usb_unmount(void)
{
	char mount_point[32];

	system("echo 1 > /proc/sys/vm/drop_caches"); // flush fs cache
	sprintf(mount_point, "/%s", nvram_default_get("usb_mntpoint", "mnt"));
	eval("/bin/umount", mount_point);
	eval("rm", "-f", DUMPFILE);
	return;
}

    /* 
     * Handle hotplugging of UFD 
     */
int usb_add_ufd(void)
{
	DIR *dir;
	FILE *fp;
	char line[256];
	struct dirent *entry;
	char path[128];
	char *fs = NULL;
	int is_part = 0;
	int is_mounted = 0;
	char part[10], *partitions, *next;
	struct stat tmp_stat;
	int i, found = 0;

	for (i = 1; i < 16; i++) {	//it needs some time for disk to settle down and /dev/discs is created
		if ((dir = opendir("/dev/discs")) != NULL
		    || (fp = fopen("/dev/sda", "rb")) != NULL
		    || (fp = fopen("/dev/sdb", "rb")) != NULL
		    || (fp = fopen("/dev/sdc", "rb")) != NULL
		    || (fp = fopen("/dev/sdd", "rb")) != NULL) {
			break;
		} else {
			sleep(1);
		}
	}
	int new = 0;
	if (fp) {
		fclose(fp);
		new = 1;
		if (dir)
			closedir(dir);
		dir = opendir("/dev");
	}
	if (dir == NULL)	// i is 16 here and not 15 if timeout happens
		return EINVAL;

	/* 
	 * Scan through entries in the directories 
	 */

	for (i = 1; i < 16; i++) {	//it needs some time for disk to settle down and /dev/discs/discs%d is created
		while ((entry = readdir(dir)) != NULL) {
			if ((strncmp(entry->d_name, "disc", 4))
			    && (strncmp(entry->d_name, "sd", 2)))
				continue;
			else
				found = 1;

			/* 
			 * Files created when the UFD is inserted are not removed when
			 * it is removed. Verify the device  is still inserted.  Strip
			 * the "disc" and pass the rest of the string.  
			 */
			if (new) {
				//everything else would cause a memory fault
				if (usb_ufd_connected(entry->d_name) == FALSE)
					continue;
			} else {
				if (usb_ufd_connected(entry->d_name + 4) ==
				    FALSE)
					continue;
			}
			if (new) {
				if (strlen(entry->d_name) != 3)
					continue;
				sprintf(path, "/dev/%s", entry->d_name);
			} else {
				sprintf(path, "/dev/discs/%s/disc",
					entry->d_name);
			}
			sysprintf("/usr/sbin/disktype %s > %s", path, DUMPFILE);

			/* 
			 * Check if it has file system 
			 */
			if ((fp = fopen(DUMPFILE, "r"))) {
				while (fgets(line, sizeof(line), fp) != NULL) {
					if (strstr(line, "Partition"))
						is_part = 1;

					if (strstr(line, "file system")) {
						if (strstr(line, "FAT")) {
							fs = "vfat";
							break;
						} else if (strstr(line, "Ext2")) {
							fs = "ext2";
							break;
						} else if (strstr(line, "Ext3")) {
#ifdef HAVE_USB_ADVANCED
							fs = "ext3";
#else
							fs = "ext2";
#endif
							break;
						}
					}

				}
				fclose(fp);
			}

			if (fs) {
				/* 
				 * If it is partioned, mount first partition else raw disk 
				 */
				if (is_part && !new) {
					partitions =
					    "part1 part2 part3 part4 part5 part6";
					foreach(part, partitions, next) {
						sprintf(path,
							"/dev/discs/%s/%s",
							entry->d_name, part);
						if (stat(path, &tmp_stat))
							continue;
						if (usb_process_path(path, fs)
						    == 0) {
							is_mounted = 1;
							break;
						}
					}
				}
				if (is_part && new) {
					partitions = "1 2 3 4 5 6";
					foreach(part, partitions, next) {
						sprintf(path, "/dev/%s%s",
							entry->d_name, part);
						if (stat(path, &tmp_stat))
							continue;
						if (usb_process_path(path, fs)
						    == 0) {
							is_mounted = 1;
							break;
						}
					}
				} else {
					if (usb_process_path(path, fs) == 0)
						is_mounted = 1;
				}

			}

			if ((fp = fopen(DUMPFILE, "a"))) {
				if (fs && is_mounted)
					fprintf(fp,
						"Status: <b>Mounted on /%s</b>\n",
						nvram_safe_get("usb_mntpoint"));
				else if (fs)
					fprintf(fp,
						"Status: <b>Not mounted</b>\n");
				else
					fprintf(fp,
						"Status: <b>Not mounted - Unsupported file system or disk not formated</b>\n");
				fclose(fp);
			}

			if (is_mounted && !nvram_match("usb_runonmount", "")) {
				sprintf(path, "%s",
					nvram_safe_get("usb_runonmount"));
				if (stat(path, &tmp_stat) == 0)	//file exists
				{
					setenv("PATH",
					       "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin:/mmc/sbin:/mmc/bin:/mmc/usr/sbin:/mmc/usr/bin:/opt/bin:/opt/sbin:/opt/usr/bin:/opt/usr/sbin",
					       1);
					setenv("LD_LIBRARY_PATH",
					       "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:/opt/lib:/opt/usr/lib",
					       1);

					system(path);
				}
			}

			if (is_mounted) {	//temp. fix: only mount 1st mountable part, then exit
				closedir(dir);
				return 0;
			}
		}
		if (!found)
			sleep(1);
		else
			break;
	}
	closedir(dir);
	return 0;
}
