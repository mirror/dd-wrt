#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

enum device_type {
    TYPE_UNKNOWN,   /* type could not be determined */
    TYPE_BAD,       /* neither file nor block device */
    TYPE_FILE,      /* image file rather than device */
    TYPE_VIRTUAL,   /* block devices like LVM or RAID volumes */
    TYPE_REMOVABLE, /* removable disk device */
    TYPE_FIXED      /* fixed disk device */
};

struct device_info {
    enum device_type type;

    /*
     * partition number if detected
     * 0  = whole disk device (including unpartitioned image file)
     * -1 = could not be determined
     */
    int partition;

    /*
     * whether partitions or device mapper devices or any other kind of
     * children use this device
     *  1 = yes
     *  0 = no
     * -1 = could not be determined
     */
    int has_children;

    /*
     * detected geometry, or -1 if unknown
     */
    int geom_heads;
    int geom_sectors;
    long long geom_start;
    long long geom_size;

    /*
     * detected sector size or -1 if unknown
     */
    int sector_size;

    /*
     * size in bytes, or -1 if unknown
     */
    long long size;
};


extern int device_info_verbose;

int get_device_info(int fd, struct device_info *info);
int is_device_mounted(const char *path);

#endif
