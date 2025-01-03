/*
 * Copyright (c) 2005-2021 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* sg_pt_linux version 1.51 20210102 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>      /* to define 'major' */
#ifndef major
#include <sys/types.h>
#endif


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <linux/major.h>

#include "sg_pt.h"
#include "sg_lib.h"
#include "sg_linux_inc.h"
#include "sg_pt_linux.h"
#include "sg_pr2serr.h"


#ifdef major
#define SG_DEV_MAJOR major
#else
#ifdef HAVE_LINUX_KDEV_T_H
#include <linux/kdev_t.h>
#endif
#define SG_DEV_MAJOR MAJOR  /* MAJOR() macro faulty if > 255 minors */
#endif

#ifndef BLOCK_EXT_MAJOR
#define BLOCK_EXT_MAJOR 259
#endif

#define DEF_TIMEOUT 60000       /* 60,000 millisecs (60 seconds) */

/* sg driver displayed format: [x]xyyzz --> [x]x.[y]y.zz */
#define SG_LINUX_SG_VER_V4_BASE 40000   /* lowest sg driver version with
                                         * v4 interface */
#define SG_LINUX_SG_VER_V4_FULL 40030   /* lowest version with full v4
                                         * interface */

static const char * linux_host_bytes[] = {
    "DID_OK", "DID_NO_CONNECT", "DID_BUS_BUSY", "DID_TIME_OUT",
    "DID_BAD_TARGET", "DID_ABORT", "DID_PARITY", "DID_ERROR",
    "DID_RESET", "DID_BAD_INTR", "DID_PASSTHROUGH", "DID_SOFT_ERROR",
    "DID_IMM_RETRY", "DID_REQUEUE" /* 0xd */,
    "DID_TRANSPORT_DISRUPTED", "DID_TRANSPORT_FAILFAST",
    "DID_TARGET_FAILURE" /* 0x10 */,
    "DID_NEXUS_FAILURE (reservation conflict)",
    "DID_ALLOC_FAILURE",
    "DID_MEDIUM_ERROR",
};

static const char * linux_driver_bytes[] = {
    "DRIVER_OK", "DRIVER_BUSY", "DRIVER_SOFT", "DRIVER_MEDIA",
    "DRIVER_ERROR", "DRIVER_INVALID", "DRIVER_TIMEOUT", "DRIVER_HARD",
    "DRIVER_SENSE"
};

#if 0

static const char * linux_driver_suggests[] = {
    "SUGGEST_OK", "SUGGEST_RETRY", "SUGGEST_ABORT", "SUGGEST_REMAP",
    "SUGGEST_DIE", "UNKNOWN","UNKNOWN","UNKNOWN",
    "SUGGEST_SENSE"
};

#endif

/*
 * These defines are for constants that should be visible in the
 * /usr/include/scsi directory (brought in by sg_linux_inc.h).
 * Redefined and aliased here to decouple this code from
 * sg_io_linux.h  N.B. the SUGGEST_* constants are no longer used.
 */
#ifndef DRIVER_MASK
#define DRIVER_MASK 0x0f
#endif
#ifndef SUGGEST_MASK
#define SUGGEST_MASK 0xf0       /* Suggest mask is obsolete */
#endif
#ifndef DRIVER_SENSE
#define DRIVER_SENSE 0x08
#endif
#define SG_LIB_DRIVER_MASK      DRIVER_MASK
#define SG_LIB_SUGGEST_MASK     SUGGEST_MASK
#define SG_LIB_DRIVER_SENSE    DRIVER_SENSE

bool sg_bsg_nvme_char_major_checked = false;
int sg_bsg_major = 0;
volatile int sg_nvme_char_major = 0;

bool sg_checked_version_num = false;
int sg_driver_version_num = 0;
bool sg_duration_set_nano = false;

long sg_lin_page_size = 4096;   /* default, overridden with correct value */


/* This function only needs to be called once (unless a NVMe controller
 * can be hot-plugged into system in which case it should be called
 * (again) after that event). */
void
sg_find_bsg_nvme_char_major(int verbose)
{
    bool got_one = false;
    int n;
    const char * proc_devices = "/proc/devices";
    char * cp;
    FILE *fp;
    char a[128];
    char b[128];

    sg_lin_page_size = sysconf(_SC_PAGESIZE);
    if (NULL == (fp = fopen(proc_devices, "r"))) {
        if (verbose)
            pr2ws("fopen %s failed: %s\n", proc_devices, strerror(errno));
        return;
    }
    while ((cp = fgets(b, sizeof(b), fp))) {
        if ((1 == sscanf(b, "%126s", a)) &&
            (0 == memcmp(a, "Character", 9)))
            break;
    }
    while (cp && (cp = fgets(b, sizeof(b), fp))) {
        if (2 == sscanf(b, "%d %126s", &n, a)) {
            if (0 == strcmp("bsg", a)) {
                sg_bsg_major = n;
                if (got_one)
                    break;
                got_one = true;
            } else if (0 == strcmp("nvme", a)) {
                sg_nvme_char_major = n;
                if (got_one)
                    break;
                got_one = true;
            }
        } else
            break;
    }
    if (verbose > 3) {
        if (cp) {
            if (sg_bsg_major > 0)
                pr2ws("found sg_bsg_major=%d\n", sg_bsg_major);
            if (sg_nvme_char_major > 0)
                pr2ws("found sg_nvme_char_major=%d\n", sg_nvme_char_major);
        } else
            pr2ws("found no bsg not nvme char device in %s\n", proc_devices);
    }
    fclose(fp);
}

/* Assumes that sg_find_bsg_nvme_char_major() has already been called. Returns
 * true if dev_fd is a scsi generic pass-through device. If yields
 * *is_nvme_p = true with *nsid_p = 0 then dev_fd is a NVMe char device.
 * If yields *nsid_p > 0 then dev_fd is a NVMe block device. */
static bool
check_file_type(int dev_fd, struct stat * dev_statp, bool * is_bsg_p,
                bool * is_nvme_p, uint32_t * nsid_p, int * os_err_p,
                int verbose)
{
    bool is_nvme = false;
    bool is_sg = false;
    bool is_bsg = false;
    bool is_block = false;
    int os_err = 0;
    int major_num;
    uint32_t nsid = 0;          /* invalid NSID */

    if (dev_fd >= 0) {
        if (fstat(dev_fd, dev_statp) < 0) {
            os_err = errno;
            if (verbose)
                pr2ws("%s: fstat() failed: %s (errno=%d)\n", __func__,
                      safe_strerror(os_err), os_err);
            goto skip_out;
        }
        major_num = (int)SG_DEV_MAJOR(dev_statp->st_rdev);
        if (S_ISCHR(dev_statp->st_mode)) {
            if (SCSI_GENERIC_MAJOR == major_num)
                is_sg = true;
            else if (sg_bsg_major == major_num)
                is_bsg = true;
            else if (sg_nvme_char_major == major_num)
                is_nvme = true;
        } else if (S_ISBLK(dev_statp->st_mode)) {
            is_block = true;
            if (BLOCK_EXT_MAJOR == major_num) {
                is_nvme = true;
                nsid = ioctl(dev_fd, NVME_IOCTL_ID, NULL);
                if (SG_NVME_BROADCAST_NSID == nsid) {  /* means ioctl error */
                    os_err = errno;
                    if (verbose)
                        pr2ws("%s: ioctl(NVME_IOCTL_ID) failed: %s "
                              "(errno=%d)\n", __func__, safe_strerror(os_err),
                              os_err);
                } else
                    os_err = 0;
            }
        }
    } else {
        os_err = EBADF;
        if (verbose)
            pr2ws("%s: invalid file descriptor (%d)\n", __func__, dev_fd);
    }
skip_out:
    if (verbose > 3) {
        pr2ws("%s: file descriptor is ", __func__);
        if (is_sg)
            pr2ws("sg device\n");
        else if (is_bsg)
            pr2ws("bsg device\n");
        else if (is_nvme && (0 == nsid))
            pr2ws("NVMe char device\n");
        else if (is_nvme)
            pr2ws("NVMe block device, nsid=%lld\n",
                  ((uint32_t)-1 == nsid) ? -1LL : (long long)nsid);
        else if (is_block)
            pr2ws("block device\n");
        else
            pr2ws("undetermined device, could be regular file\n");
    }
    if (is_bsg_p)
        *is_bsg_p = is_bsg;
    if (is_nvme_p)
        *is_nvme_p = is_nvme;
    if (nsid_p)
        *nsid_p = nsid;
    if (os_err_p)
        *os_err_p = os_err;
    return is_sg;
}

/* Assumes dev_fd is an "open" file handle associated with device_name. If
 * the implementation (possibly for one OS) cannot determine from dev_fd if
 * a SCSI or NVMe pass-through is referenced, then it might guess based on
 * device_name. Returns 1 if SCSI generic pass-though device, returns 2 if
 * secondary SCSI pass-through device (in Linux a bsg device); returns 3 is
 * char NVMe device (i.e. no NSID); returns 4 if block NVMe device (includes
 * NSID), or 0 if something else (e.g. ATA block device) or dev_fd < 0.
 * If error, returns negated errno (operating system) value. */
int
check_pt_file_handle(int dev_fd, const char * device_name, int verbose)
{
    if (verbose > 4)
        pr2ws("%s: dev_fd=%d, device_name: %s\n", __func__, dev_fd,
              device_name);
    /* Linux doesn't need device_name to determine which pass-through */
    if (! sg_bsg_nvme_char_major_checked) {
        sg_bsg_nvme_char_major_checked = true;
        sg_find_bsg_nvme_char_major(verbose);
    }
    if (dev_fd >= 0) {
        bool is_sg, is_bsg, is_nvme;
        int err;
        uint32_t nsid;
        struct stat a_stat;

        is_sg = check_file_type(dev_fd, &a_stat, &is_bsg, &is_nvme, &nsid,
                                &err, verbose);
        if (err)
            return -err;
        else if (is_sg)
            return 1;
        else if (is_bsg)
            return 2;
        else if (is_nvme && (0 == nsid))
            return 3;
        else if (is_nvme)
            return 4;
        else
            return 0;
    } else
        return 0;
}

/*
 * We make a runtime decision whether to use the sg v3 interface or the sg
 * v4 interface (currently exclusively used by the bsg driver). If all the
 * following are true we use sg v4 which is only currently supported on bsg
 * device nodes:
 *   a) there is a bsg entry in the /proc/devices file
 *   b) the device node given to scsi_pt_open() is a char device
 *   c) the char major number of the device node given to scsi_pt_open()
 *      matches the char major number of the bsg entry in /proc/devices
 * Otherwise the sg v3 interface is used.
 *
 * Note that in either case we prepare the data in a sg v4 structure. If
 * the runtime tests indicate that the v3 interface is needed then
 * do_scsi_pt_v3() transfers the input data into a v3 structure and
 * then the output data is transferred back into a sg v4 structure.
 * That implementation detail could change in the future.
 *
 * [20120806] Only use MAJOR() macro in kdev_t.h if that header file is
 * available and major() macro [N.B. lower case] is not available.
 */


#ifdef major
#define SG_DEV_MAJOR major
#else
#ifdef HAVE_LINUX_KDEV_T_H
#include <linux/kdev_t.h>
#endif
#define SG_DEV_MAJOR MAJOR  /* MAJOR() macro faulty if > 255 minors */
#endif


/* Similar to scsi_pt_open_device() but takes Unix style open flags OR-ed */
/* together. The 'flags' argument is advisory and may be ignored. */
/* Returns >= 0 if successful, otherwise returns negated errno. */
int
scsi_pt_open_flags(const char * device_name, int flags, int verbose)
{
    int fd;

    if (! sg_bsg_nvme_char_major_checked) {
        sg_bsg_nvme_char_major_checked = true;
        sg_find_bsg_nvme_char_major(verbose);
    }
    if (verbose > 1) {
        pr2ws("open %s with flags=0x%x\n", device_name, flags);
    }
    fd = open(device_name, flags);
    if (fd < 0) {
        fd = -errno;
        if (verbose > 1)
            pr2ws("%s: open(%s, 0x%x) failed: %s\n", __func__, device_name,
                  flags, safe_strerror(-fd));
    }
    return fd;
}

/* Returns >= 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_open_device(const char * device_name, bool read_only, int verbose)
{
    int oflags = O_NONBLOCK;

    oflags |= (read_only ? O_RDONLY : O_RDWR);
    return scsi_pt_open_flags(device_name, oflags, verbose);
}

/* Returns 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_close_device(int device_fd)
{
    int res;

    res = close(device_fd);
    if (res < 0)
        res = -errno;
    return res;
}

#if (HAVE_NVME && (! IGNORE_NVME))
static bool checked_ev_dsense = false;
static bool ev_dsense = false;
#endif


/* Caller should additionally call get_scsi_pt_os_err() after this call */
struct sg_pt_base *
construct_scsi_pt_obj_with_fd(int dev_fd, int verbose)
{
    int err;
    struct sg_pt_linux_scsi * ptp;

    ptp = (struct sg_pt_linux_scsi *)
          calloc(1, sizeof(struct sg_pt_linux_scsi));
    if (ptp) {
#if (HAVE_NVME && (! IGNORE_NVME))
        sntl_init_dev_stat(&ptp->dev_stat);
        if (! checked_ev_dsense) {
            ev_dsense = sg_get_initial_dsense();
            checked_ev_dsense = true;
        }
        ptp->dev_stat.scsi_dsense = ev_dsense;
#endif
        err = set_pt_file_handle((struct sg_pt_base *)ptp, dev_fd, verbose);
        if ((0 == err) && (! ptp->is_nvme)) {
            ptp->io_hdr.guard = 'Q';
#ifdef BSG_PROTOCOL_SCSI
            ptp->io_hdr.protocol = BSG_PROTOCOL_SCSI;
#endif
#ifdef BSG_SUB_PROTOCOL_SCSI_CMD
            ptp->io_hdr.subprotocol = BSG_SUB_PROTOCOL_SCSI_CMD;
#endif
        }
    } else if (verbose)
        pr2ws("%s: calloc() failed, out of memory?\n", __func__);

    return (struct sg_pt_base *)ptp;
}

struct sg_pt_base *
construct_scsi_pt_obj()
{
    return construct_scsi_pt_obj_with_fd(-1 /* dev_fd */, 0 /* verbose */);
}

void
destruct_scsi_pt_obj(struct sg_pt_base * vp)
{

    if (NULL == vp)
        pr2ws(">>>>>>> Warning: %s called with NULL pointer\n", __func__);
    else {
        struct sg_pt_linux_scsi * ptp = &vp->impl;

        if (ptp->free_nvme_id_ctlp) {
            free(ptp->free_nvme_id_ctlp);
            ptp->free_nvme_id_ctlp = NULL;
            ptp->nvme_id_ctlp = NULL;
        }
        if (ptp)
            free(ptp);
    }
}

/* Remembers previous device file descriptor */
void
clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    bool is_sg, is_bsg, is_nvme;
    int fd;
    uint32_t nvme_nsid;
    struct sg_sntl_dev_state_t dev_stat;
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (ptp) {
        fd = ptp->dev_fd;
        is_sg = ptp->is_sg;
        is_bsg = ptp->is_bsg;
        is_nvme = ptp->is_nvme;
        nvme_nsid = ptp->nvme_nsid;
        dev_stat = ptp->dev_stat;
        if (ptp->free_nvme_id_ctlp)
            free(ptp->free_nvme_id_ctlp);
        memset(ptp, 0, sizeof(struct sg_pt_linux_scsi));
        ptp->io_hdr.guard = 'Q';
#ifdef BSG_PROTOCOL_SCSI
        ptp->io_hdr.protocol = BSG_PROTOCOL_SCSI;
#endif
#ifdef BSG_SUB_PROTOCOL_SCSI_CMD
        ptp->io_hdr.subprotocol = BSG_SUB_PROTOCOL_SCSI_CMD;
#endif
        ptp->dev_fd = fd;
        ptp->is_sg = is_sg;
        ptp->is_bsg = is_bsg;
        ptp->is_nvme = is_nvme;
        ptp->nvme_our_sntl = false;
        ptp->nvme_nsid = nvme_nsid;
        ptp->dev_stat = dev_stat;
    }
}

void
partial_clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (NULL == ptp)
        return;
    ptp->in_err = 0;
    ptp->os_err = 0;
    ptp->io_hdr.device_status = 0;
    ptp->io_hdr.transport_status = 0;
    ptp->io_hdr.driver_status = 0;
    ptp->io_hdr.din_xferp = 0;
    ptp->io_hdr.din_xfer_len = 0;
    ptp->io_hdr.dout_xferp = 0;
    ptp->io_hdr.dout_xfer_len = 0;
    ptp->nvme_result = 0;
}

#ifndef SG_SET_GET_EXTENDED

/* If both sei_wr_mask and sei_rd_mask are 0, this ioctl does nothing */
struct sg_extended_info {
    uint32_t   sei_wr_mask;    /* OR-ed SG_SEIM_* user->driver values */
    uint32_t   sei_rd_mask;    /* OR-ed SG_SEIM_* driver->user values */
    uint32_t   ctl_flags_wr_mask;      /* OR-ed SG_CTL_FLAGM_* values */
    uint32_t   ctl_flags_rd_mask;      /* OR-ed SG_CTL_FLAGM_* values */
    uint32_t   ctl_flags;      /* bit values OR-ed, see SG_CTL_FLAGM_* */
    uint32_t   read_value;     /* write SG_SEIRV_*, read back related */

    uint32_t   reserved_sz;    /* data/sgl size of pre-allocated request */
    uint32_t   tot_fd_thresh;  /* total data/sgat for this fd, 0: no limit */
    uint32_t   minor_index;    /* rd: kernel's sg device minor number */
    uint32_t   share_fd;       /* SHARE_FD and CHG_SHARE_FD use this */
    uint32_t   sgat_elem_sz;   /* sgat element size (must be power of 2) */
    uint8_t    pad_to_96[52];  /* pad so struct is 96 bytes long */
};

#define SG_IOCTL_MAGIC_NUM 0x22

#define SG_SET_GET_EXTENDED _IOWR(SG_IOCTL_MAGIC_NUM, 0x51,     \
                                  struct sg_extended_info)

#define SG_SEIM_CTL_FLAGS       0x1

#define SG_CTL_FLAGM_TIME_IN_NS 0x1

#endif

/* Forget any previous dev_fd and install the one given. May attempt to
 * find file type (e.g. if pass-though) from OS so there could be an error.
 * Returns 0 for success or the same value as get_scsi_pt_os_err()
 * will return. dev_fd should be >= 0 for a valid file handle or -1 . */
int
set_pt_file_handle(struct sg_pt_base * vp, int dev_fd, int verbose)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;
    struct stat a_stat;

    if (! sg_bsg_nvme_char_major_checked) {
        sg_bsg_nvme_char_major_checked = true;
        sg_find_bsg_nvme_char_major(verbose);
    }
    ptp->dev_fd = dev_fd;
    if (dev_fd >= 0) {
        ptp->is_sg = check_file_type(dev_fd, &a_stat, &ptp->is_bsg,
                                     &ptp->is_nvme, &ptp->nvme_nsid,
                                     &ptp->os_err, verbose);
        if (ptp->is_sg && (! sg_checked_version_num)) {
            if (ioctl(dev_fd, SG_GET_VERSION_NUM, &ptp->sg_version) < 0) {
                ptp->sg_version = 0;
                if (verbose > 3)
                    pr2ws("%s: ioctl(SG_GET_VERSION_NUM) failed: errno: %d "
                          "[%s]\n", __func__, errno, safe_strerror(errno));
            } else {    /* got version number */
                sg_driver_version_num = ptp->sg_version;
                sg_checked_version_num = true;
            }
            if (verbose > 4) {
                int ver = ptp->sg_version;

                if (ptp->sg_version >= SG_LINUX_SG_VER_V4_BASE) {
#ifdef IGNORE_LINUX_SGV4
                    pr2ws("%s: sg driver version %d.%02d.%02d but config "
                          "override back to v3\n", __func__, ver / 10000,
                          (ver / 100) % 100, ver % 100);
#else
                    pr2ws("%s: sg driver version %d.%02d.%02d so choose v4\n",
                          __func__, ver / 10000, (ver / 100) % 100,
                          ver % 100);
#endif
                } else if (verbose > 5)
                    pr2ws("%s: sg driver version %d.%02d.%02d so choose v3\n",
                          __func__, ver / 10000, (ver / 100) % 100,
                          ver % 100);
            }
        } else if (ptp->is_sg)
            ptp->sg_version = sg_driver_version_num;

        if (ptp->is_sg && (ptp->sg_version >= SG_LINUX_SG_VER_V4_FULL) &&
            getenv("SG3_UTILS_LINUX_NANO")) {
            struct sg_extended_info sei;
            struct sg_extended_info * seip = &sei;

            memset(seip, 0, sizeof(*seip));
            /* try to override default of milliseconds */
            seip->sei_wr_mask |= SG_SEIM_CTL_FLAGS;
            seip->ctl_flags_wr_mask |= SG_CTL_FLAGM_TIME_IN_NS;
            seip->ctl_flags |= SG_CTL_FLAGM_TIME_IN_NS;
            if (ioctl(dev_fd, SG_SET_GET_EXTENDED, seip) < 0) {
                if (verbose > 2)
                    pr2ws("%s: unable to override milli --> nanoseconds: "
                          "%s\n", __func__, safe_strerror(errno));
            } else {
                if (! sg_duration_set_nano)
                    sg_duration_set_nano = true;
                if (verbose > 5)
                    pr2ws("%s: dev_fd=%d, succeeding in setting durations "
                          "to nanoseconds\n", __func__, dev_fd);
            }
        } else if (ptp->is_sg && (ptp->sg_version >= SG_LINUX_SG_VER_V4_BASE)
                   && getenv("SG3_UTILS_LINUX_NANO")) {
            if (verbose > 2)
                pr2ws("%s: dev_fd=%d, ignored SG3_UTILS_LINUX_NANO\nbecause "
                      "base version sg version 4 driver\n", __func__, dev_fd);
        }
    } else {
        ptp->is_sg = false;
        ptp->is_bsg = false;
        ptp->is_nvme = false;
        ptp->nvme_our_sntl = false;
        ptp->nvme_nsid = 0;
        ptp->os_err = 0;
    }
    return ptp->os_err;
}

int
sg_linux_get_sg_version(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->sg_version;
}

/* Valid file handles (which is the return value) are >= 0 . Returns -1
 * if there is no valid file handle. */
int
get_pt_file_handle(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->dev_fd;
}

void
set_scsi_pt_cdb(struct sg_pt_base * vp, const uint8_t * cdb,
                int cdb_len)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.request = (__u64)(sg_uintptr_t)cdb;
    ptp->io_hdr.request_len = cdb_len;
}

int
get_scsi_pt_cdb_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->io_hdr.request_len;
}

uint8_t *
get_scsi_pt_cdb_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return (uint8_t *)(sg_uintptr_t)ptp->io_hdr.request;
}

void
set_scsi_pt_sense(struct sg_pt_base * vp, uint8_t * sense,
                  int max_sense_len)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (sense) {
        if (max_sense_len > 0)
            memset(sense, 0, max_sense_len);
    }
    ptp->io_hdr.response = (__u64)(sg_uintptr_t)sense;
    ptp->io_hdr.max_response_len = max_sense_len;
}

/* Setup for data transfer from device */
void
set_scsi_pt_data_in(struct sg_pt_base * vp, uint8_t * dxferp,
                    int dxfer_ilen)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (ptp->io_hdr.din_xferp)
        ++ptp->in_err;
    if (dxfer_ilen > 0) {
        ptp->io_hdr.din_xferp = (__u64)(sg_uintptr_t)dxferp;
        ptp->io_hdr.din_xfer_len = dxfer_ilen;
    }
}

/* Setup for data transfer toward device */
void
set_scsi_pt_data_out(struct sg_pt_base * vp, const uint8_t * dxferp,
                     int dxfer_olen)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (ptp->io_hdr.dout_xferp)
        ++ptp->in_err;
    if (dxfer_olen > 0) {
        ptp->io_hdr.dout_xferp = (__u64)(sg_uintptr_t)dxferp;
        ptp->io_hdr.dout_xfer_len = dxfer_olen;
    }
}

void
set_pt_metadata_xfer(struct sg_pt_base * vp, uint8_t * dxferp,
                     uint32_t dxfer_len, bool out_true)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (dxfer_len > 0) {
        ptp->mdxferp = dxferp;
        ptp->mdxfer_len = dxfer_len;
        ptp->mdxfer_out = out_true;
    }
}

void
set_scsi_pt_packet_id(struct sg_pt_base * vp, int pack_id)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.request_extra = pack_id;        /* was placed in spare_in */
}

void
set_scsi_pt_tag(struct sg_pt_base * vp, uint64_t tag)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.request_tag = tag;
}

/* Note that task management function codes are transport specific */
void
set_scsi_pt_task_management(struct sg_pt_base * vp, int tmf_code)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.subprotocol = 1;        /* SCSI task management function */
    ptp->tmf_request[0] = (uint8_t)tmf_code;      /* assume it fits */
    ptp->io_hdr.request = (__u64)(sg_uintptr_t)(&(ptp->tmf_request[0]));
    ptp->io_hdr.request_len = 1;
}

void
set_scsi_pt_task_attr(struct sg_pt_base * vp, int attribute, int priority)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.request_attr = attribute;
    ptp->io_hdr.request_priority = priority;
}

#ifndef BSG_FLAG_Q_AT_TAIL
#define BSG_FLAG_Q_AT_TAIL 0x10
#endif
#ifndef BSG_FLAG_Q_AT_HEAD
#define BSG_FLAG_Q_AT_HEAD 0x20
#endif

/* Need this later if translated to v3 interface */
#ifndef SG_FLAG_Q_AT_TAIL
#define SG_FLAG_Q_AT_TAIL 0x10
#endif
#ifndef SG_FLAG_Q_AT_HEAD
#define SG_FLAG_Q_AT_HEAD 0x20
#endif

void
set_scsi_pt_flags(struct sg_pt_base * vp, int flags)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    /* default action of bsg driver (sg v4) is QUEUE_AT_HEAD */
    /* default action of block layer SG_IO ioctl is QUEUE_AT_TAIL */
    if (SCSI_PT_FLAGS_QUEUE_AT_HEAD & flags) {  /* favour AT_HEAD */
        ptp->io_hdr.flags |= BSG_FLAG_Q_AT_HEAD;
        ptp->io_hdr.flags &= ~BSG_FLAG_Q_AT_TAIL;
    } else if (SCSI_PT_FLAGS_QUEUE_AT_TAIL & flags) {
        ptp->io_hdr.flags |= BSG_FLAG_Q_AT_TAIL;
        ptp->io_hdr.flags &= ~BSG_FLAG_Q_AT_HEAD;
    }
}

/* If supported it is the number of bytes requested to transfer less the
 * number actually transferred. This it typically important for data-in
 * transfers. For data-out (only) transfers, the 'dout_req_len -
 * dout_act_len' is returned. For bidi transfer the "din" residual is
 * returned. */
/* N.B. Returns din_resid and ignores dout_resid */
int
get_scsi_pt_resid(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    if ((NULL == ptp) || (ptp->is_nvme && ! ptp->nvme_our_sntl))
        return 0;
    else if ((ptp->io_hdr.din_xfer_len > 0) &&
             (ptp->io_hdr.dout_xfer_len > 0))
        return ptp->io_hdr.din_resid;
    else if (ptp->io_hdr.dout_xfer_len > 0)
        return ptp->io_hdr.dout_resid;
    return ptp->io_hdr.din_resid;
}

void
get_pt_req_lengths(const struct sg_pt_base * vp, int * req_dinp,
                   int * req_doutp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (req_dinp) {
        if (ptp->io_hdr.din_xfer_len > 0)
            *req_dinp = ptp->io_hdr.din_xfer_len;
        else
            *req_dinp = 0;
    }
    if (req_doutp) {
        if (ptp->io_hdr.dout_xfer_len > 0)
            *req_doutp = ptp->io_hdr.dout_xfer_len;
        else
            *req_doutp = 0;
    }
}

void
get_pt_actual_lengths(const struct sg_pt_base * vp, int * act_dinp,
                      int * act_doutp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (act_dinp) {
        if (ptp->io_hdr.din_xfer_len > 0) {
            int res = ptp->io_hdr.din_xfer_len - ptp->io_hdr.din_resid;

            *act_dinp = (res > 0) ? res : 0;
        } else
            *act_dinp = 0;
    }
    if (act_doutp) {
        if (ptp->io_hdr.dout_xfer_len > 0)
            *act_doutp = ptp->io_hdr.dout_xfer_len - ptp->io_hdr.dout_resid;
        else
            *act_doutp = 0;
    }
}

int
get_scsi_pt_status_response(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (NULL == ptp)
        return 0;
    return (int)((ptp->is_nvme && ! ptp->nvme_our_sntl) ?
                         ptp->nvme_status : ptp->io_hdr.device_status);
}

uint32_t
get_pt_result(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    if (NULL == ptp)
        return 0;
    return (ptp->is_nvme && ! ptp->nvme_our_sntl) ?
                        ptp->nvme_result : ptp->io_hdr.device_status;
}

int
get_scsi_pt_sense_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->io_hdr.response_len;
}

uint8_t *
get_scsi_pt_sense_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;
}

int
get_scsi_pt_duration_ms(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return sg_duration_set_nano ? (ptp->io_hdr.duration / 1000) :
                                  ptp->io_hdr.duration;
}

/* If not available return 0 otherwise return number of nanoseconds that the
 * lower layers (and hardware) took to execute the command just completed. */
uint64_t
get_pt_duration_ns(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return sg_duration_set_nano ? (uint32_t)ptp->io_hdr.duration : 0;
}

int
get_scsi_pt_transport_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->io_hdr.transport_status;
}

void
set_scsi_pt_transport_err(struct sg_pt_base * vp, int err)
{
    struct sg_pt_linux_scsi * ptp = &vp->impl;

    ptp->io_hdr.transport_status = err;
}

/* Returns b which will contain a null char terminated string (if
 * max_b_len > 0). Combined driver and transport (called "host" in Linux
 * kernel) statuses */
char *
get_scsi_pt_transport_err_str(const struct sg_pt_base * vp, int max_b_len,
                              char * b)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;
    int ds = ptp->io_hdr.driver_status;
    int hs = ptp->io_hdr.transport_status;
    int n, m;
    char * cp = b;
    int driv;
    const char * driv_cp = "invalid";

    if (max_b_len < 1)
        return b;
    m = max_b_len;
    n = 0;
    if (hs) {
        if ((hs < 0) || (hs >= (int)SG_ARRAY_SIZE(linux_host_bytes)))
            n = snprintf(cp, m, "Host_status=0x%02x is invalid\n", hs);
        else
            n = snprintf(cp, m, "Host_status=0x%02x [%s]\n", hs,
                         linux_host_bytes[hs]);
    }
    m -= n;
    if (m < 1) {
        b[max_b_len - 1] = '\0';
        return b;
    }
    cp += n;
    driv = ds & SG_LIB_DRIVER_MASK;
    if (driv < (int)SG_ARRAY_SIZE(linux_driver_bytes))
        driv_cp = linux_driver_bytes[driv];
#if 0
    sugg = (ds & SG_LIB_SUGGEST_MASK) >> 4;
    if (sugg < SG_ARRAY_SIZE(linux_driver_suggests)
        sugg_cp = linux_driver_suggests[sugg];
#endif
    n = snprintf(cp, m, "Driver_status=0x%02x [%s]\n", ds, driv_cp);
    m -= n;
    if (m < 1)
        b[max_b_len - 1] = '\0';
    return b;
}

int
get_scsi_pt_result_category(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;
    int dr_st = ptp->io_hdr.driver_status & SG_LIB_DRIVER_MASK;
    int scsi_st = ptp->io_hdr.device_status & 0x7e;

    if (ptp->os_err)
        return SCSI_PT_RESULT_OS_ERR;
    else if (ptp->io_hdr.transport_status)
        return SCSI_PT_RESULT_TRANSPORT_ERR;
    else if (dr_st && (SG_LIB_DRIVER_SENSE != dr_st))
        return SCSI_PT_RESULT_TRANSPORT_ERR;
    else if ((SG_LIB_DRIVER_SENSE == dr_st) ||
             (SAM_STAT_CHECK_CONDITION == scsi_st) ||
             (SAM_STAT_COMMAND_TERMINATED == scsi_st))
        return SCSI_PT_RESULT_SENSE;
    else if (scsi_st)
        return SCSI_PT_RESULT_STATUS;
    else
        return SCSI_PT_RESULT_GOOD;
}

int
get_scsi_pt_os_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->os_err;
}

char *
get_scsi_pt_os_err_str(const struct sg_pt_base * vp, int max_b_len, char * b)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;
    const char * cp;

    cp = safe_strerror(ptp->os_err);
    strncpy(b, cp, max_b_len);
    if ((int)strlen(cp) >= max_b_len)
        b[max_b_len - 1] = '\0';
    return b;
}

bool
pt_device_is_nvme(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->is_nvme;
}

/* If a NVMe block device (which includes the NSID) handle is associated
 * with 'vp', then its NSID is returned (values range from 0x1 to
 * 0xffffffe). Otherwise 0 is returned. */
uint32_t
get_pt_nvme_nsid(const struct sg_pt_base * vp)
{
    const struct sg_pt_linux_scsi * ptp = &vp->impl;

    return ptp->nvme_nsid;
}

/* Executes SCSI command using sg v3 interface */
static int
do_scsi_pt_v3(struct sg_pt_linux_scsi * ptp, int fd, int time_secs,
              int verbose)
{
    struct sg_io_hdr v3_hdr;

    memset(&v3_hdr, 0, sizeof(v3_hdr));
    /* convert v4 to v3 header */
    v3_hdr.interface_id = 'S';
    v3_hdr.dxfer_direction = SG_DXFER_NONE;
    v3_hdr.cmdp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.request;
    v3_hdr.cmd_len = (uint8_t)ptp->io_hdr.request_len;
    if (ptp->io_hdr.din_xfer_len > 0) {
        if (ptp->io_hdr.dout_xfer_len > 0) {
            if (verbose)
                pr2ws("sgv3 doesn't support bidi\n");
            return SCSI_PT_DO_BAD_PARAMS;
        }
        v3_hdr.dxferp = (void *)(long)ptp->io_hdr.din_xferp;
        v3_hdr.dxfer_len = (unsigned int)ptp->io_hdr.din_xfer_len;
        v3_hdr.dxfer_direction =  SG_DXFER_FROM_DEV;
    } else if (ptp->io_hdr.dout_xfer_len > 0) {
        v3_hdr.dxferp = (void *)(long)ptp->io_hdr.dout_xferp;
        v3_hdr.dxfer_len = (unsigned int)ptp->io_hdr.dout_xfer_len;
        v3_hdr.dxfer_direction =  SG_DXFER_TO_DEV;
    }
    if (ptp->io_hdr.response && (ptp->io_hdr.max_response_len > 0)) {
        v3_hdr.sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;
        v3_hdr.mx_sb_len = (uint8_t)ptp->io_hdr.max_response_len;
    }
    v3_hdr.pack_id = (int)ptp->io_hdr.request_extra;
    if (BSG_FLAG_Q_AT_HEAD & ptp->io_hdr.flags)
        v3_hdr.flags |= SG_FLAG_Q_AT_HEAD;      /* favour AT_HEAD */
    else if (BSG_FLAG_Q_AT_TAIL & ptp->io_hdr.flags)
        v3_hdr.flags |= SG_FLAG_Q_AT_TAIL;

    if (NULL == v3_hdr.cmdp) {
        if (verbose)
            pr2ws("No SCSI command (cdb) given [v3]\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    /* io_hdr.timeout is in milliseconds, if greater than zero */
    v3_hdr.timeout = ((time_secs > 0) ? (time_secs * 1000) : DEF_TIMEOUT);
    /* Finally do the v3 SG_IO ioctl */
    if (ioctl(fd, SG_IO, &v3_hdr) < 0) {
        ptp->os_err = errno;
        if (verbose > 1)
            pr2ws("ioctl(SG_IO v3) failed: %s (errno=%d)\n",
                  safe_strerror(ptp->os_err), ptp->os_err);
        return -ptp->os_err;
    }
    ptp->io_hdr.device_status = (__u32)v3_hdr.status;
    ptp->io_hdr.driver_status = (__u32)v3_hdr.driver_status;
    ptp->io_hdr.transport_status = (__u32)v3_hdr.host_status;
    ptp->io_hdr.response_len = (__u32)v3_hdr.sb_len_wr;
    ptp->io_hdr.duration = (__u32)v3_hdr.duration;
    ptp->io_hdr.din_resid = (__s32)v3_hdr.resid;
    /* v3_hdr.info not passed back since no mapping defined (yet) */
    return 0;
}

/* Executes SCSI command using sg v4 interface */
static int
do_scsi_pt_v4(struct sg_pt_linux_scsi * ptp, int fd, int time_secs,
              int verbose)
{
    if (0 == ptp->io_hdr.request) {
        if (verbose)
            pr2ws("No SCSI command (cdb) given [v4]\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    /* io_hdr.timeout is in milliseconds, if greater than zero */
    ptp->io_hdr.timeout = ((time_secs > 0) ? (time_secs * 1000) : DEF_TIMEOUT);
    if (ioctl(fd, SG_IO, &ptp->io_hdr) < 0) {
        ptp->os_err = errno;
        if (verbose > 1)
            pr2ws("ioctl(SG_IO v4) failed: %s (errno=%d)\n",
                  safe_strerror(ptp->os_err), ptp->os_err);
        return -ptp->os_err;
    }
    return 0;
}

/* Executes SCSI command (or at least forwards it to lower layers).
 * Returns 0 for success, negative numbers are negated 'errno' values from
 * OS system calls. Positive return values are errors from this package. */
int
do_scsi_pt(struct sg_pt_base * vp, int fd, int time_secs, int verbose)
{
    int err;
    struct sg_pt_linux_scsi * ptp = &vp->impl;
    bool have_checked_for_type = (ptp->dev_fd >= 0);

    if (! sg_bsg_nvme_char_major_checked) {
        sg_bsg_nvme_char_major_checked = true;
        sg_find_bsg_nvme_char_major(verbose);
    }
    if (ptp->in_err) {
        if (verbose)
            pr2ws("Replicated or unused set_scsi_pt... functions\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (fd >= 0) {
        if ((ptp->dev_fd >= 0) && (fd != ptp->dev_fd)) {
            if (verbose)
                pr2ws("%s: file descriptor given to create() and here "
                      "differ\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        ptp->dev_fd = fd;
    } else if (ptp->dev_fd < 0) {
        if (verbose)
            pr2ws("%s: invalid file descriptors\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    } else
        fd = ptp->dev_fd;
    if (! have_checked_for_type) {
        err = set_pt_file_handle(vp, ptp->dev_fd, verbose);
        if (err)
            return -ptp->os_err;
    }
    if (ptp->os_err)
        return -ptp->os_err;
    if (verbose > 5)
        pr2ws("%s:  is_nvme=%d, is_sg=%d, is_bsg=%d\n", __func__,
              (int)ptp->is_nvme, (int)ptp->is_sg, (int)ptp->is_bsg);
    if (ptp->is_nvme)
        return sg_do_nvme_pt(vp, -1, time_secs, verbose);
    else if (ptp->is_sg) {
#ifdef IGNORE_LINUX_SGV4
        return do_scsi_pt_v3(ptp, fd, time_secs, verbose);
#else
        if (ptp->sg_version >= SG_LINUX_SG_VER_V4_BASE)
            return do_scsi_pt_v4(ptp, fd, time_secs, verbose);
        else
            return do_scsi_pt_v3(ptp, fd, time_secs, verbose);
#endif
    } else if (sg_bsg_major <= 0)
        return do_scsi_pt_v3(ptp, fd, time_secs, verbose);
    else if (ptp->is_bsg)
        return do_scsi_pt_v4(ptp, fd, time_secs, verbose);
    else
        return do_scsi_pt_v3(ptp, fd, time_secs, verbose);

    pr2ws("%s: Should never reach this point\n", __func__);
    return 0;
}
