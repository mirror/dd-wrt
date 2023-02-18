#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <libmount/libmount.h>
#include <sys/sysmacros.h>

#include "xlog.h"
#include "conffile.h"

#ifndef MOUNTINFO_PATH
#define MOUNTINFO_PATH "/proc/self/mountinfo"
#endif

#define CONF_NAME "nfsrahead"
#define NFS_DEFAULT_READAHEAD 128

/* Device information from the system */
struct device_info {
	char *device_number;
	dev_t dev;
	char *mountpoint;
	char *fstype;
};

/* Convert a string in the format n:m to a device number */
static int fill_device_number(struct device_info *info)
{
	char *s = strdup(info->device_number), *p;
	char *maj_s, *min_s;
	unsigned int maj, min;
	int err = -EINVAL;

	maj_s = p = s;
	for ( ; *p != ':' && *p != '\0'; p++)
		;

	if (*p == '\0')
		goto out_free;

	err = 0;
	*p = '\0';
	min_s = p + 1;

	maj = strtol(maj_s, NULL, 10);
	min = strtol(min_s, NULL, 10);

	info->dev = makedev(maj, min);
out_free:
	free(s);
	return err;
}

#define sfree(ptr) if (ptr) free(ptr)

/* device_info maintenance */
static void init_device_info(struct device_info *di, const char *device_number)
{
	di->device_number = strdup(device_number);
	di->dev = 0;
	di->mountpoint = NULL;
	di->fstype = NULL;
}


static void free_device_info(struct device_info *di)
{
	sfree(di->mountpoint);
	sfree(di->fstype);
	sfree(di->device_number);
}

static int get_mountinfo(const char *device_number, struct device_info *device_info, const char *mountinfo_path)
{
	int ret = 0;
	struct libmnt_table *mnttbl;
	struct libmnt_fs *fs;
	char *target;

	init_device_info(device_info, device_number);
	if ((ret = fill_device_number(device_info)) < 0)
		goto out_free_device_info;

	mnttbl = mnt_new_table();

	if ((ret = mnt_table_parse_file(mnttbl, mountinfo_path)) < 0) {
		xlog(D_GENERAL, "Failed to parse %s\n", mountinfo_path);
		goto out_free_tbl;
	}

	if ((fs = mnt_table_find_devno(mnttbl, device_info->dev, MNT_ITER_FORWARD)) == NULL) {
		ret = ENOENT;
		goto out_free_tbl;
	}

	if ((target = (char *)mnt_fs_get_target(fs)) == NULL) {
		ret = ENOENT;
		goto out_free_fs;
	}

	device_info->mountpoint = strdup(target);
	target = (char *)mnt_fs_get_fstype(fs);
	if (target)
		device_info->fstype = strdup(target);

out_free_fs:
	mnt_free_fs(fs);
out_free_tbl:
	mnt_free_table(mnttbl);
out_free_device_info:
	free(device_info->device_number);
	device_info->device_number = NULL;
	return ret;
}

static int get_device_info(const char *device_number, struct device_info *device_info)
{
	int ret = ENOENT;
	for (int retry_count = 0; retry_count < 10 && ret != 0; retry_count++)
		ret = get_mountinfo(device_number, device_info, MOUNTINFO_PATH);

	return ret;
}

static int conf_get_readahead(const char *kind) {
	int readahead = 0;

	if((readahead = conf_get_num(CONF_NAME, kind, -1)) == -1)
		readahead = conf_get_num(CONF_NAME, "default", NFS_DEFAULT_READAHEAD);
	
	return readahead;
}

int main(int argc, char **argv)
{
	int ret = 0, retry, opt;
	struct device_info device;
	unsigned int readahead = 128, log_level, log_stderr = 0;


	log_level = D_ALL & ~D_GENERAL;
	while((opt = getopt(argc, argv, "dF")) != -1) {
		switch (opt) {
		case 'd':
			log_level = D_ALL;
			break;
		case 'F':
			log_stderr = 1;
			break;
		}
	}

	conf_init_file(NFS_CONFFILE);

	xlog_stderr(log_stderr);
	xlog_syslog(~log_stderr);
	xlog_config(log_level, 1);
	xlog_open(CONF_NAME);

	// xlog_err causes the system to exit
	if ((argc - optind) != 1)
		xlog_err("expected the device number of a BDI; is udev ok?");

	for (retry = 0; retry <= 10; retry++ )
		if ((ret = get_device_info(argv[optind], &device)) == 0)
			break;

	if (ret != 0) {
		xlog(D_GENERAL, "unable to find device %s\n", argv[optind]);
		goto out;
	}

	if (strncmp("nfs", device.fstype, 3) != 0) {
		xlog(D_GENERAL,
			"not setting readahead for non supported fstype %s on device %s\n",
			device.fstype, argv[optind]);
		ret = -EINVAL;
		goto out;
	}

	readahead = conf_get_readahead(device.fstype);

	xlog(D_FAC7, "setting %s readahead to %d\n", device.mountpoint, readahead);

	printf("%d\n", readahead);

out:
	free_device_info(&device);
	return ret;
}
