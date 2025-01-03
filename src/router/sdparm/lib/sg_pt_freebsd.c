/*
 * Copyright (c) 2005-2021 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* sg_pt_freebsd version 1.39 20210225 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h>     /* for basename */
#include <fcntl.h>
#include <errno.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>   /* from PRIx macros */
#include <err.h>
#include <camlib.h>
#include <cam/scsi/scsi_message.h>
// #include <sys/ata.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <fcntl.h>
#include <stddef.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_pt.h"
#include "sg_lib.h"
#include "sg_unaligned.h"
#include "sg_pt_nvme.h"
#include "sg_pr2serr.h"

#if (HAVE_NVME && (! IGNORE_NVME))
#include "freebsd_nvme_ioctl.h"
#else
#define NVME_CTRLR_PREFIX       "/dev/nvme"
#define NVME_NS_PREFIX          "ns"
#endif


#define FREEBSD_MAXDEV 64
#define FREEBSD_FDOFFSET 16;


struct freebsd_dev_channel {
    int unitnum;                  // the SCSI unit number
    bool is_nvme;       /* OS device type, if false ignore nvme_our_sntl */
    bool nvme_our_sntl; /* true: our SNTL; false: received NVMe command */
    bool is_char;
    uint32_t nsid;
    uint32_t nv_ctrlid;
    int dev_fd;                   // for NVMe, use -1 to indicate not provided
    uint32_t nvme_result;         // cdw0 from completion
    uint16_t nvme_status;         // from completion: ((sct << 8) | sc)
    char* devname;                // the device name
    struct cam_device* cam_dev;
    uint8_t * nvme_id_ctlp;
    uint8_t * free_nvme_id_ctlp;
    uint8_t cq_dw0_3[16];
    struct sg_sntl_dev_state_t dev_stat;    // owner
};

// Private table of open devices: guaranteed zero on startup since
// part of static data.
static struct freebsd_dev_channel *devicetable[FREEBSD_MAXDEV];

#define DEF_TIMEOUT 60000       /* 60,000 milliseconds (60 seconds) */

struct sg_pt_freebsd_scsi {
    struct cam_device* cam_dev; // copy held for error processing
    union ccb *ccb;
    uint8_t * cdb;
    int cdb_len;
    uint8_t * sense;
    int sense_len;
    uint8_t * dxferp;
    int dxfer_len;
    int dxfer_dir;      /* CAM_DIR_NONE, _IN, _OUT and _BOTH */
    uint8_t * dxferip;
    uint8_t * dxferop;
    uint8_t * mdxferp;
    uint32_t dxfer_ilen;
    uint32_t dxfer_olen;
    uint32_t mdxfer_len;
    bool mdxfer_out;
    int timeout_ms;
    int scsi_status;
    int resid;
    int sense_resid;
    int in_err;
    int os_err;
    int transport_err;
    int dev_han;                // should be >= FREEBSD_FDOFFSET then
                                // (dev_han - FREEBSD_FDOFFSET) is the
                                // index into devicetable[]
    bool is_nvme;               // copy of same field in fdc object
    bool nvme_our_sntl;         // copy of same field in fdc object
    struct sg_sntl_dev_state_t * dev_statp;     // points to associated fdc
};

struct sg_pt_base {
    struct sg_pt_freebsd_scsi impl;
};

static const uint32_t broadcast_nsid = SG_NVME_BROADCAST_NSID;

static int sg_do_nvme_pt(struct sg_pt_base * vp, int fd, int vb);



static struct freebsd_dev_channel *
get_fdc_p(struct sg_pt_freebsd_scsi * ptp)
{
    int han = ptp->dev_han - FREEBSD_FDOFFSET;

    if ((han < 0) || (han >= FREEBSD_MAXDEV))
        return NULL;
    return devicetable[han];
}

static const struct freebsd_dev_channel *
get_fdc_cp(const struct sg_pt_freebsd_scsi * ptp)
{
    int han = ptp->dev_han - FREEBSD_FDOFFSET;

    if ((han < 0) || (han >= FREEBSD_MAXDEV))
        return NULL;
    return devicetable[han];
}

/* Returns >= 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_open_device(const char * device_name, bool read_only, int vb)
{
    int oflags = 0 /* O_NONBLOCK*/ ;

    oflags |= (read_only ? O_RDONLY : O_RDWR);
    return scsi_pt_open_flags(device_name, oflags, vb);
}

/* Similar to scsi_pt_open_device() but takes Unix style open flags OR-ed
 * together. The 'oflags' is only used on NVMe devices. It is ignored on
 * SCSI and ATA devices in FreeBSD.
 * Returns >= 0 if successful, otherwise returns negated errno. */
int
scsi_pt_open_flags(const char * device_name, int oflags, int vb)
{
    bool is_char, is_block, possible_nvme;
    char tmp, first_ch;
    int k, err, dev_fd, ret;
    uint32_t nsid, nv_ctrlid;
    ssize_t s;
    struct freebsd_dev_channel *fdc_p = NULL;
    struct cam_device* cam_dev;
    struct stat a_stat;
    char dev_nm[PATH_MAX];

    // Search table for a free entry
    for (k = 0; k < FREEBSD_MAXDEV; k++)
        if (! devicetable[k])
            break;

    // If no free entry found, return error.  We have max allowed number
    // of "file descriptors" already allocated.
    if (k == FREEBSD_MAXDEV) {
        if (vb)
            pr2ws("too many open file descriptors (%d)\n", FREEBSD_MAXDEV);
        ret = -EMFILE;
        goto err_out;
    }
    first_ch = device_name[0];
    if (('/' != first_ch) && ('.' != first_ch)) {
        char b[PATH_MAX];

        /* Step 1: if device_name is symlink, follow it */
        s = readlink(device_name,  b, sizeof(b));
        if (s <= 0) {
            strncpy(b, device_name, PATH_MAX - 1);
            b[PATH_MAX - 1] = '\0';
        }
        /* Step 2: if no leading '/' nor '.' given, prepend '/dev/' */
        first_ch = b[0];
        if (('/' != first_ch) && ('.' != first_ch))
            snprintf(dev_nm, PATH_MAX, "%s%s", "/dev/", b);
        else
            strcpy(dev_nm, b);
    } else
        strcpy(dev_nm, device_name);
    if (stat(dev_nm, &a_stat) < 0) {
        err = errno;
        pr2ws("%s: unable to stat(%s): %s\n", __func__, dev_nm,
              strerror(err));
        ret = -err;
        goto err_out;
    }
    is_block = S_ISBLK(a_stat.st_mode);
    is_char = S_ISCHR(a_stat.st_mode);
    if (! (is_block || is_char)) {
        if (vb)
            pr2ws("%s: %s is not char nor block device\n", __func__,
                            dev_nm);
        ret = -ENODEV;
        goto err_out;
    }

    /* Some code borrowed from smartmontools, Christian Franke */
    nsid = broadcast_nsid;
    nv_ctrlid = broadcast_nsid;
    possible_nvme = false;
    while (true) {      /* dummy loop, so can 'break' out */
        if(sscanf(dev_nm, NVME_CTRLR_PREFIX "%u%c", &nv_ctrlid, &tmp) == 1) {
            if(nv_ctrlid == broadcast_nsid)
                break;
        } else if (sscanf(dev_nm, NVME_CTRLR_PREFIX "%d" NVME_NS_PREFIX
                          "%d%c", &nv_ctrlid, &nsid, &tmp) == 2) {
            if((nv_ctrlid == broadcast_nsid) || (nsid == broadcast_nsid))
                break;
        } else
            break;
        possible_nvme = true;
        break;
    }

    fdc_p = (struct freebsd_dev_channel *)
                calloc(1,sizeof(struct freebsd_dev_channel));
    if (fdc_p == NULL) {
        // errno already set by call to calloc()
        ret = -ENOMEM;
        goto err_out;
    }
    fdc_p->dev_fd = -1;
#if (HAVE_NVME && (! IGNORE_NVME))
    sntl_init_dev_stat(&fdc_p->dev_stat);
#endif
    if (! (fdc_p->devname = (char *)calloc(1, DEV_IDLEN+1))) {
         ret = -ENOMEM;
         goto err_out;
    }

    if (possible_nvme) {
        // we should always open controller, not namespace device
        snprintf(fdc_p->devname, DEV_IDLEN, NVME_CTRLR_PREFIX"%d",
                 nv_ctrlid);
        dev_fd = open(fdc_p->devname, oflags);
        if (dev_fd < 0) {
            err = errno;
            if (vb)
                pr2ws("%s: open(%s) failed: %s (errno=%d), try SCSI/ATA\n",
                      __func__, fdc_p->devname, strerror(err), err);
            goto scsi_ata_try;
        }
        fdc_p->is_nvme = true;
        fdc_p->nvme_our_sntl = true;    /* guess at this stage */
        fdc_p->is_char = is_char;
        fdc_p->nsid = (broadcast_nsid == nsid) ? 0 : nsid;
        fdc_p->nv_ctrlid = nv_ctrlid;
        fdc_p->dev_fd = dev_fd;
        devicetable[k] = fdc_p;
        return k + FREEBSD_FDOFFSET;
    }

scsi_ata_try:
    fdc_p->is_char = is_char;
    if (cam_get_device(dev_nm, fdc_p->devname, DEV_IDLEN,
                       &(fdc_p->unitnum)) == -1) {
        if (vb)
            pr2ws("bad device name structure\n");
        errno = EINVAL;
        ret = -errno;
        goto err_out;
    }
    if (vb > 4)
        pr2ws("%s: cam_get_device, f->devname: %s, f->unitnum=%d\n", __func__,
              fdc_p->devname, fdc_p->unitnum);

    if (! (cam_dev = cam_open_spec_device(fdc_p->devname,
                                          fdc_p->unitnum, O_RDWR, NULL))) {
        if (vb)
            pr2ws("cam_open_spec_device: %s\n", cam_errbuf);
        errno = EPERM; /* permissions or not CAM device (NVMe ?) */
        ret = -errno;
        goto err_out;
    }
    fdc_p->cam_dev = cam_dev;
    // return pointer to "file descriptor" table entry, properly offset.
    devicetable[k] = fdc_p;
    return k + FREEBSD_FDOFFSET;

err_out:                /* ret should be negative value (negated errno) */
    if (fdc_p) {
        if (fdc_p->devname)
            free(fdc_p->devname);
        free(fdc_p);
        fdc_p = NULL;
    }
    return ret;
}

/* Returns 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_close_device(int device_han)
{
    struct freebsd_dev_channel *fdc_p;
    int han = device_han - FREEBSD_FDOFFSET;

    if ((han < 0) || (han >= FREEBSD_MAXDEV)) {
        errno = ENODEV;
        return -errno;
    }
    fdc_p = devicetable[han];
    if (NULL == fdc_p) {
        errno = ENODEV;
        return -errno;
    }
    if (fdc_p->devname)
        free(fdc_p->devname);
    if (fdc_p->cam_dev)
        cam_close_device(fdc_p->cam_dev);
    if (fdc_p->is_nvme) {
        if (fdc_p->dev_fd >= 0)
            close(fdc_p->dev_fd);
        if (fdc_p->free_nvme_id_ctlp) {
            free(fdc_p->free_nvme_id_ctlp);
            fdc_p->nvme_id_ctlp = NULL;
            fdc_p->free_nvme_id_ctlp = NULL;
        }
    }
    free(fdc_p);
    devicetable[han] = NULL;
    errno = 0;
    return 0;
}

/* Assumes device_han is an "open" file handle associated with some device.
 * Returns 1 if SCSI generic pass-though device, returns 2 if secondary
 * SCSI pass-through device (in Linux a bsg device); returns 3 is char
 * NVMe device (i.e. no NSID); returns 4 if block NVMe device (includes
 * NSID), or 0 if something else (e.g. ATA block device) or device_han < 0.
 * If error, returns negated errno (operating system) value. */
int
check_pt_file_handle(int device_han, const char * device_name, int vb)
{
    struct freebsd_dev_channel *fdc_p;
    int han = device_han - FREEBSD_FDOFFSET;

    if ((han < 0) || (han >= FREEBSD_MAXDEV))
        return -ENODEV;
    fdc_p = devicetable[han];
    if (NULL == fdc_p)
        return -ENODEV;
    if (fdc_p->is_nvme)
        return 4 - (int)fdc_p->is_char;
    else if (fdc_p->cam_dev)
        return 2 - (int)fdc_p->is_char;
    else {
        if (vb)
            pr2ws("%s: neither SCSI nor NVMe ... hmm, dvice name: %s\n",
                  __func__, device_name);
        return 0;
    }
}

#if (HAVE_NVME && (! IGNORE_NVME))
static bool checked_ev_dsense = false;
static bool ev_dsense = false;
#endif

struct sg_pt_base *
construct_scsi_pt_obj_with_fd(int dev_han, int vb)
{
    struct sg_pt_freebsd_scsi * ptp;

    ptp = (struct sg_pt_freebsd_scsi *)
                calloc(1, sizeof(struct sg_pt_freebsd_scsi));
    if (ptp) {
        ptp->dxfer_dir = CAM_DIR_NONE;
        ptp->dev_han = (dev_han < 0) ? -1 : dev_han;
        if (ptp->dev_han >= 0) {
            struct freebsd_dev_channel *fdc_p;

            fdc_p = get_fdc_p(ptp);
            if (fdc_p) {
                ptp->is_nvme = fdc_p->is_nvme;
                ptp->cam_dev = fdc_p->cam_dev;
                ptp->dev_statp = &fdc_p->dev_stat;
#if (HAVE_NVME && (! IGNORE_NVME))
                sntl_init_dev_stat(ptp->dev_statp);
                if (! checked_ev_dsense) {
                    ev_dsense = sg_get_initial_dsense();
                    checked_ev_dsense = true;
                }
                fdc_p->dev_stat.scsi_dsense = ev_dsense;
#endif
            } else if (vb)
                pr2ws("%s: bad dev_han=%d\n", __func__, dev_han);
        }
    } else if (vb)
        pr2ws("%s: calloc() out of memory\n", __func__);
    return (struct sg_pt_base *)ptp;
}


struct sg_pt_base *
construct_scsi_pt_obj()
{
    return construct_scsi_pt_obj_with_fd(-1, 0);
}

void
destruct_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_freebsd_scsi * ptp;

    if (NULL == vp) {
        pr2ws(">>>> %s: given NULL pointer\n", __func__);
        return;
    }
    if ((ptp = &vp->impl)) {
        if (ptp->ccb)
            cam_freeccb(ptp->ccb);
        free(ptp);
    }
}

void
clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    bool is_nvme;
    int dev_han;
    struct sg_pt_freebsd_scsi * ptp;
    struct cam_device* cam_dev;
    struct sg_sntl_dev_state_t * dsp;

    if (NULL == vp) {
        pr2ws(">>>>> %s: NULL pointer given\n", __func__);
        return;
    }
    if ((ptp = &vp->impl)) {
        if (ptp->ccb)
            cam_freeccb(ptp->ccb);
        is_nvme = ptp->is_nvme;
        dev_han = ptp->dev_han;
        cam_dev = ptp->cam_dev;
        dsp = ptp->dev_statp;
        memset(ptp, 0, sizeof(struct sg_pt_freebsd_scsi));
        ptp->dxfer_dir = CAM_DIR_NONE;
        ptp->dev_han = dev_han;
        ptp->is_nvme = is_nvme;
        ptp->cam_dev = cam_dev;
        ptp->dev_statp = dsp;
    }
}

void
partial_clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    struct freebsd_dev_channel *fdc_p;

    if (NULL == ptp)
        return;
    ptp->in_err = 0;
    ptp->os_err = 0;
    ptp->transport_err = 0;
    ptp->scsi_status = 0;
    ptp->dxfer_dir = CAM_DIR_NONE;
    ptp->dxferip = NULL;
    ptp->dxfer_ilen = 0;
    ptp->dxferop = NULL;
    ptp->dxfer_olen = 0;
    fdc_p = get_fdc_p(ptp);
    if (fdc_p)
        fdc_p->nvme_result = 0;
}

/* Forget any previous dev_han and install the one given. May attempt to
 * find file type (e.g. if pass-though) from OS so there could be an error.
 * Returns 0 for success or the same value as get_scsi_pt_os_err()
 * will return. dev_han should be >= 0 for a valid file handle or -1 . */
int
set_pt_file_handle(struct sg_pt_base * vp, int dev_han, int vb)
{
    struct sg_pt_freebsd_scsi * ptp;

    if (NULL == vp) {
        if (vb)
            pr2ws(">>>> %s: pointer to object is NULL\n", __func__);
        return EINVAL;
    }
    if ((ptp = &vp->impl)) {
        struct freebsd_dev_channel *fdc_p;

        if (dev_han < 0) {
            ptp->dev_han = -1;
            ptp->dxfer_dir = CAM_DIR_NONE;
            ptp->is_nvme = false;
            ptp->cam_dev = NULL;
            return 0;
        }
        fdc_p = get_fdc_p(ptp);
        if (NULL == fdc_p) {
            if (vb)
                pr2ws("%s: dev_han (%d) is invalid\n", __func__, dev_han);
            ptp->os_err = EINVAL;
            return ptp->os_err;
        }
        ptp->os_err = 0;
        ptp->transport_err = 0;
        ptp->in_err = 0;
        ptp->scsi_status = 0;
        ptp->dev_han = dev_han;
        ptp->dxfer_dir = CAM_DIR_NONE;
        ptp->is_nvme = fdc_p->is_nvme;
        ptp->cam_dev = fdc_p->cam_dev;
        ptp->dev_statp = &fdc_p->dev_stat;
    }
    return 0;
}

/* Valid file handles (which is the return value) are >= 0 . Returns -1
 * if there is no valid file handle. */
int
get_pt_file_handle(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp ? ptp->dev_han : -1;
}

void
set_scsi_pt_cdb(struct sg_pt_base * vp, const uint8_t * cdb, int cdb_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ptp->cdb = (uint8_t *)cdb;
    ptp->cdb_len = cdb_len;
}

int
get_scsi_pt_cdb_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->cdb_len;
}

uint8_t *
get_scsi_pt_cdb_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->cdb;
}

void
set_scsi_pt_sense(struct sg_pt_base * vp, uint8_t * sense,
                  int max_sense_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (sense) {
        if (max_sense_len > 0)
            memset(sense, 0, max_sense_len);
    }
    ptp->sense = sense;
    ptp->sense_len = max_sense_len;
}

/* Setup for data transfer from device */
void
set_scsi_pt_data_in(struct sg_pt_base * vp, uint8_t * dxferp,
                    int dxfer_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->dxferip)
        ++ptp->in_err;
    ptp->dxferip = dxferp;
    ptp->dxfer_ilen = dxfer_len;
    if (dxfer_len > 0) {
        ptp->dxferp = dxferp;
        ptp->dxfer_len = dxfer_len;
        if (ptp->dxfer_dir == CAM_DIR_OUT)
            ptp->dxfer_dir = CAM_DIR_BOTH;
        else
            ptp->dxfer_dir = CAM_DIR_IN;
    }
}

/* Setup for data transfer toward device */
void
set_scsi_pt_data_out(struct sg_pt_base * vp, const uint8_t * dxferp,
                     int dxfer_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->dxferop)
        ++ptp->in_err;
    ptp->dxferop = (uint8_t *)dxferp;
    ptp->dxfer_olen = dxfer_len;
    if (dxfer_len > 0) {
        ptp->dxferp = (uint8_t *)dxferp;
        ptp->dxfer_len = dxfer_len;
        if (ptp->dxfer_dir == CAM_DIR_IN)
            ptp->dxfer_dir = CAM_DIR_BOTH;
        else
            ptp->dxfer_dir = CAM_DIR_OUT;
    }
}

void
set_pt_metadata_xfer(struct sg_pt_base * vp, uint8_t * mdxferp,
                     uint32_t mdxfer_len, bool out_true)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->mdxferp)
        ++ptp->in_err;
    ptp->mdxferp = mdxferp;
    ptp->mdxfer_len = mdxfer_len;
    if (mdxfer_len > 0)
        ptp->mdxfer_out = out_true;
}

void
set_scsi_pt_packet_id(struct sg_pt_base * vp __attribute__ ((unused)),
                      int pack_id __attribute__ ((unused)))
{
}

void
set_scsi_pt_tag(struct sg_pt_base * vp, uint64_t tag __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_management(struct sg_pt_base * vp,
                            int tmf_code __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_attr(struct sg_pt_base * vp,
                      int attrib __attribute__ ((unused)),
                      int priority __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_flags(struct sg_pt_base * objp, int flags)
{
    if (objp) { ; }     /* unused, suppress warning */
    if (flags) { ; }     /* unused, suppress warning */
}

/* Executes SCSI command (or at least forwards it to lower layers).
 * Clears os_err field prior to active call (whose result may set it
 * again). */
int
do_scsi_pt(struct sg_pt_base * vp, int dev_han, int time_secs, int vb)
{
    int len;
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    struct freebsd_dev_channel *fdc_p;
    union ccb *ccb;

    ptp->os_err = 0;
    if (ptp->in_err) {
        if (vb)
            pr2ws("Replicated or unused set_scsi_pt...\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (dev_han < 0) {
        if (ptp->dev_han < 0) {
            if (vb)
                pr2ws("%s: No device file handle given\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        dev_han = ptp->dev_han;
    } else {
        if (ptp->dev_han >= 0) {
            if (dev_han != ptp->dev_han) {
                if (vb)
                    pr2ws("%s: file handle given to create and this "
                          "differ\n", __func__);
                return SCSI_PT_DO_BAD_PARAMS;
            }
        } else
            ptp->dev_han = dev_han;
    }

    if (NULL == ptp->cdb) {
        if (vb)
            pr2ws("No command (cdb) given\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (ptp->is_nvme)
        return sg_do_nvme_pt(vp, -1, vb);

    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        if (vb)
            pr2ws("File descriptor bad or closed??\n");
        ptp->os_err = ENODEV;
        return -ptp->os_err;
    }
    ptp->is_nvme = fdc_p->is_nvme;
    ptp->dev_statp = &fdc_p->dev_stat;
    if (fdc_p->is_nvme)
        return sg_do_nvme_pt(vp, -1, vb);

    if (NULL == fdc_p->cam_dev) {
        if (vb)
            pr2ws("No open CAM device\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if (NULL == ptp->ccb) {     /* re-use if we have one already */
        if (! (ccb = cam_getccb(fdc_p->cam_dev))) {
            if (vb)
                pr2ws("cam_getccb: failed\n");
            ptp->os_err = ENOMEM;
            return -ptp->os_err;
        }
        ptp->ccb = ccb;
    } else
        ccb = ptp->ccb;

    // clear out structure, except for header that was filled in for us
    bzero(&(&ccb->ccb_h)[1],
            sizeof(struct ccb_scsiio) - sizeof(struct ccb_hdr));

    ptp->timeout_ms = (time_secs > 0) ? (time_secs * 1000) : DEF_TIMEOUT;
    cam_fill_csio(&ccb->csio,
                  /* retries */ 1,
                  /* cbfcnp */ NULL,
                  /* flags */ ptp->dxfer_dir,
                  /* tagaction */ MSG_SIMPLE_Q_TAG,
                  /* dataptr */ ptp->dxferp,
                  /* datalen */ ptp->dxfer_len,
                  /* senselen */ ptp->sense_len,
                  /* cdblen */ ptp->cdb_len,
                  /* timeout (millisecs) */ ptp->timeout_ms);
    memcpy(ccb->csio.cdb_io.cdb_bytes, ptp->cdb, ptp->cdb_len);

    if (cam_send_ccb(fdc_p->cam_dev, ccb) < 0) {
        if (vb) {
            warn("error sending SCSI ccb");
 #if __FreeBSD_version > 500000
            cam_error_print(fdc_p->cam_dev, ccb, CAM_ESF_ALL,
                            CAM_EPF_ALL, stderr);
 #endif
        }
        cam_freeccb(ptp->ccb);
        ptp->ccb = NULL;
        ptp->os_err = EIO;
        return -ptp->os_err;
    }

    if (((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP) ||
        ((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR)) {
        ptp->scsi_status = ccb->csio.scsi_status;
        ptp->resid = ccb->csio.resid;
        ptp->sense_resid = ccb->csio.sense_resid;

        if ((SAM_STAT_CHECK_CONDITION == ptp->scsi_status) ||
            (SAM_STAT_COMMAND_TERMINATED == ptp->scsi_status)) {
            if (ptp->sense_resid > ptp->sense_len)
                len = ptp->sense_len;   /* crazy; ignore sense_resid */
            else
                len = ptp->sense_len - ptp->sense_resid;
            if (len > 0)
                memcpy(ptp->sense, &(ccb->csio.sense_data), len);
        }
    } else
        ptp->transport_err = 1;

    ptp->cam_dev = fdc_p->cam_dev;     // for error processing
    return 0;
}

int
get_scsi_pt_result_category(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->os_err)
        return SCSI_PT_RESULT_OS_ERR;
    else if (ptp->transport_err)
        return SCSI_PT_RESULT_TRANSPORT_ERR;
    else if ((SAM_STAT_CHECK_CONDITION == ptp->scsi_status) ||
             (SAM_STAT_COMMAND_TERMINATED == ptp->scsi_status))
        return SCSI_PT_RESULT_SENSE;
    else if (ptp->scsi_status)
        return SCSI_PT_RESULT_STATUS;
    else
        return SCSI_PT_RESULT_GOOD;
}

int
get_scsi_pt_resid(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ((NULL == ptp) || (ptp->is_nvme && ! ptp->nvme_our_sntl)) ?
                 0 : ptp->resid;
}

void
get_pt_req_lengths(const struct sg_pt_base * vp, int * req_dinp,
                   int * req_doutp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    bool bidi = (ptp->dxfer_dir == CAM_DIR_BOTH);

    if (req_dinp) {
        if (ptp->dxfer_ilen > 0)
            *req_dinp = ptp->dxfer_ilen;
        else
            *req_dinp = 0;
    }
    if (req_doutp) {
        if ((!bidi) && (ptp->dxfer_olen > 0))
            *req_doutp = ptp->dxfer_olen;
        else
            *req_doutp = 0;
    }
}

void
get_pt_actual_lengths(const struct sg_pt_base * vp, int * act_dinp,
                      int * act_doutp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    bool bidi = (ptp->dxfer_dir == CAM_DIR_BOTH);

    if (act_dinp) {
        if (ptp->dxfer_ilen > 0)
            *act_dinp = ptp->dxfer_ilen - ptp->resid;
        else
            *act_dinp = 0;
    }
    if (act_doutp) {
        if ((!bidi) && (ptp->dxfer_olen > 0))
            *act_doutp = ptp->dxfer_olen - ptp->resid;
        else
            *act_doutp = 0;
    }
}

/* Returns SCSI status value (from device that received the command). If an
 * NVMe command was issued directly (i.e. through do_scsi_pt() then return
 * NVMe status (i.e. ((SCT << 8) | SC)). If problem returns -1. */
int
get_scsi_pt_status_response(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp) {
        if (ptp->is_nvme && ! ptp->nvme_our_sntl) {
            const struct freebsd_dev_channel *fdc_p;

            fdc_p = get_fdc_cp(ptp);
            if (NULL == fdc_p)
                return -1;
            return (int)fdc_p->nvme_status;
        } else
            return ptp->scsi_status;
    }
    return -1;
}

/* For NVMe command: CDW0 from completion (32 bits); for SCSI: the status */
uint32_t
get_pt_result(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp) {
        if (ptp->is_nvme && ! ptp->nvme_our_sntl) {
            const struct freebsd_dev_channel *fdc_p;

            fdc_p = get_fdc_cp(ptp);
            if (NULL == fdc_p)
                return -1;
            return fdc_p->nvme_result;
        } else
            return (uint32_t)ptp->scsi_status;
    }
    return 0xffffffff;
}

int
get_scsi_pt_sense_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->sense_resid > ptp->sense_len)
        return ptp->sense_len;  /* strange; ignore ptp->sense_resid */
    else
        return ptp->sense_len - ptp->sense_resid;
}

uint8_t *
get_scsi_pt_sense_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->sense;
}

/* Not implemented so return -1 . */
int
get_scsi_pt_duration_ms(const struct sg_pt_base * vp __attribute__ ((unused)))
{
    // const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return -1;
}

/* If not available return 0 otherwise return number of nanoseconds that the
 * lower layers (and hardware) took to execute the command just completed. */
uint64_t
get_pt_duration_ns(const struct sg_pt_base * vp __attribute__ ((unused)))
{
    return 0;
}

int
get_scsi_pt_transport_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->transport_err;
}

void
set_scsi_pt_transport_err(struct sg_pt_base * vp, int err)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ptp->transport_err = err;
}

int
get_scsi_pt_os_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->os_err;
}

char *
get_scsi_pt_transport_err_str(const struct sg_pt_base * vp, int max_b_len,
                              char * b)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (0 == ptp->transport_err) {
        strncpy(b, "no transport error available", max_b_len);
        b[max_b_len - 1] = '\0';
        return b;
    }
    if (ptp->is_nvme) {
        snprintf(b, max_b_len, "NVMe has no transport errors at present "
                 "but tranport_err=%d ??\n", ptp->transport_err);
        return b;
    }
#if __FreeBSD_version > 500000
    if (ptp->cam_dev)
        cam_error_string(ptp->cam_dev, ptp->ccb, b, max_b_len, CAM_ESF_ALL,
                         CAM_EPF_ALL);
    else {
        strncpy(b, "no transport error available", max_b_len);
        b[max_b_len - 1] = '\0';
   }
#else
    strncpy(b, "no transport error available", max_b_len);
    b[max_b_len - 1] = '\0';
#endif
    return b;
}

bool
pt_device_is_nvme(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp && (ptp->dev_han >= 0)) {
        const struct freebsd_dev_channel *fdc_p;

        fdc_p = get_fdc_cp(ptp);
        if (NULL == fdc_p) {
            pr2ws("%s: unable to find fdc_p\n", __func__);
            errno = ENODEV;
            return false;
        }
        /* if unequal, cast away const and drive fdc_p value into ptp */
        if (ptp->is_nvme != fdc_p->is_nvme)     /* indicates logic error */
            ((struct sg_pt_freebsd_scsi *)ptp)->is_nvme = fdc_p->is_nvme;
        return fdc_p->is_nvme;
    }
    return false;
}

/* If a NVMe block device (which includes the NSID) handle is associated
 * with 'objp', then its NSID is returned (values range from 0x1 to
 * 0xffffffe). Otherwise 0 is returned. */
uint32_t
get_pt_nvme_nsid(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp && (ptp->dev_han >= 0)) {
        const struct freebsd_dev_channel *fdc_p;

        fdc_p = get_fdc_cp(ptp);
        if (NULL == fdc_p)
            return 0;
        return fdc_p->nsid;
    }
    return 0;
}

char *
get_scsi_pt_os_err_str(const struct sg_pt_base * vp, int max_b_len, char * b)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    const char * cp;

    cp = safe_strerror(ptp->os_err);
    strncpy(b, cp, max_b_len);
    if ((int)strlen(cp) >= max_b_len)
        b[max_b_len - 1] = '\0';
    return b;
}


#define SCSI_INQUIRY_OPC     0x12
#define SCSI_MAINT_IN_OPC  0xa3
#define SCSI_MODE_SENSE10_OPC  0x5a
#define SCSI_MODE_SELECT10_OPC  0x55
#define SCSI_READ_CAPACITY10_OPC  0x25
#define SCSI_RECEIVE_DIAGNOSTIC_OPC  0x1c
#define SCSI_REP_SUP_OPCS_OPC  0xc
#define SCSI_REP_SUP_TMFS_OPC  0xd
#define SCSI_REPORT_LUNS_OPC 0xa0
#define SCSI_REQUEST_SENSE_OPC  0x3
#define SCSI_SEND_DIAGNOSTIC_OPC  0x1d
#define SCSI_TEST_UNIT_READY_OPC  0x0
#define SCSI_SERVICE_ACT_IN_OPC  0x9e
#define SCSI_READ_CAPACITY16_SA  0x10
#define SCSI_SA_MSK  0x1f

/* Additional Sense Code (ASC) */
#define NO_ADDITIONAL_SENSE 0x0
#define LOGICAL_UNIT_NOT_READY 0x4
#define LOGICAL_UNIT_COMMUNICATION_FAILURE 0x8
#define UNRECOVERED_READ_ERR 0x11
#define PARAMETER_LIST_LENGTH_ERR 0x1a
#define INVALID_OPCODE 0x20
#define LBA_OUT_OF_RANGE 0x21
#define INVALID_FIELD_IN_CDB 0x24
#define INVALID_FIELD_IN_PARAM_LIST 0x26
#define UA_RESET_ASC 0x29
#define UA_CHANGED_ASC 0x2a
#define TARGET_CHANGED_ASC 0x3f
#define LUNS_CHANGED_ASCQ 0x0e
#define INSUFF_RES_ASC 0x55
#define INSUFF_RES_ASCQ 0x3
#define LOW_POWER_COND_ON_ASC  0x5e     /* ASCQ=0 */
#define POWER_ON_RESET_ASCQ 0x0
#define BUS_RESET_ASCQ 0x2      /* scsi bus reset occurred */
#define MODE_CHANGED_ASCQ 0x1   /* mode parameters changed */
#define CAPACITY_CHANGED_ASCQ 0x9
#define SAVING_PARAMS_UNSUP 0x39
#define TRANSPORT_PROBLEM 0x4b
#define THRESHOLD_EXCEEDED 0x5d
#define LOW_POWER_COND_ON 0x5e
#define MISCOMPARE_VERIFY_ASC 0x1d
#define MICROCODE_CHANGED_ASCQ 0x1      /* with TARGET_CHANGED_ASC */
#define MICROCODE_CHANGED_WO_RESET_ASCQ 0x16

#if (HAVE_NVME && (! IGNORE_NVME))

static void
mk_sense_asc_ascq(struct sg_pt_freebsd_scsi * ptp, int sk, int asc, int ascq,
                  int vb)
{
    bool dsense = ptp->dev_statp->scsi_dsense;
    int n;
    uint8_t * sbp = ptp->sense;

    ptp->scsi_status = SAM_STAT_CHECK_CONDITION;
    n = ptp->sense_len;
    if ((n < 8) || ((! dsense) && (n < 14))) {
        pr2ws("%s: sense_len=%d too short, want 14 or more\n", __func__, n);
        return;
    } else
        ptp->sense_resid = ptp->sense_len -
                           (dsense ? 8 : ((n < 18) ? n : 18));
    memset(sbp, 0, n);
    sg_build_sense_buffer(dsense, sbp, sk, asc, ascq);
    if (vb > 3)
        pr2ws("%s:  [sense_key,asc,ascq]: [0x%x,0x%x,0x%x]\n", __func__,
              sk, asc, ascq);
}

static void
mk_sense_from_nvme_status(struct sg_pt_freebsd_scsi * ptp, uint16_t sct_sc,
                          int vb)
{
    bool ok;
    bool dsense = ptp->dev_statp->scsi_dsense;
    int n;
    uint8_t sstatus, sk, asc, ascq;
    uint8_t * sbp = ptp->sense;

    ok = sg_nvme_status2scsi(sct_sc, &sstatus, &sk, &asc, &ascq);
    if (! ok) { /* can't find a mapping to a SCSI error, so ... */
        sstatus = SAM_STAT_CHECK_CONDITION;
        sk = SPC_SK_ILLEGAL_REQUEST;
        asc = 0xb;
        ascq = 0x0;     /* asc: "WARNING" purposely vague */
    }

    ptp->scsi_status = sstatus;
    n = ptp->sense_len;
    if ((n < 8) || ((! dsense) && (n < 14))) {
        pr2ws("%s: sense_len=%d too short, want 14 or more\n", __func__, n);
        return;
    } else
        ptp->sense_resid = ptp->sense_len -
                           (dsense ? 8 : ((n < 18) ? n : 18));
    memset(sbp, 0, n);
    sg_build_sense_buffer(dsense, sbp, sk, asc, ascq);
    if (vb > 3)
        pr2ws("%s:  [sense_key,asc,ascq]: [0x%x,0x%x,0x%x]\n", __func__,
              sk, asc, ascq);
    if (dsense && (sct_sc > 0) && (ptp->sense_resid > 7)) {
        sg_nvme_desc2sense(sbp, 0x4000 & sct_sc /* dnr */,
                           0x2000 & sct_sc /* more */, 0x7ff & sct_sc);
        ptp->sense_resid -= 8;
    }
}

/* Set in_bit to -1 to indicate no bit position of invalid field */
static void
mk_sense_invalid_fld(struct sg_pt_freebsd_scsi * ptp, bool in_cdb,
                     int in_byte, int in_bit, int vb)
{
    bool ds = ptp->dev_statp->scsi_dsense;
    int sl, asc, n;
    uint8_t * sbp = (uint8_t *)ptp->sense;
    uint8_t sks[4];

    ptp->scsi_status = SAM_STAT_CHECK_CONDITION;
    asc = in_cdb ? INVALID_FIELD_IN_CDB : INVALID_FIELD_IN_PARAM_LIST;
    n = ptp->sense_len;
    if ((n < 8) || ((! ds) && (n < 14))) {
        pr2ws("%s: max_response_len=%d too short, want 14 or more\n",
              __func__, n);
        return;
    } else
        ptp->sense_resid = ptp->sense_len - (ds ? 8 : ((n < 18) ? n : 18));
    memset(sbp, 0, n);
    sg_build_sense_buffer(ds, sbp, SPC_SK_ILLEGAL_REQUEST, asc, 0);
    memset(sks, 0, sizeof(sks));
    sks[0] = 0x80;
    if (in_cdb)
        sks[0] |= 0x40;
    if (in_bit >= 0) {
        sks[0] |= 0x8;
        sks[0] |= (0x7 & in_bit);
    }
    sg_put_unaligned_be16(in_byte, sks + 1);
    if (ds) {
        sl = sbp[7] + 8;
        sbp[7] = sl;
        sbp[sl] = 0x2;
        sbp[sl + 1] = 0x6;
        memcpy(sbp + sl + 4, sks, 3);
    } else
        memcpy(sbp + 15, sks, 3);
    if (vb > 3)
        pr2ws("%s:  [sense_key,asc,ascq]: [0x5,0x%x,0x0] %c byte=%d, bit=%d\n",
              __func__, asc, in_cdb ? 'C' : 'D', in_byte,
              ((in_bit > 0) ? (0x7 & in_bit) : 0));
}

/* Does actual ioctl(NVME_PASSTHROUGH_CMD). Returns 0 on success; negative
 * values are Unix negated errno values; positive values are NVMe status
 * (i.e. ((SCT << 8) | SC) ). */
static int
nvme_pt_low(struct freebsd_dev_channel *fdc_p, void * dxferp, uint32_t len,
            bool is_read, struct nvme_pt_command * npcp, int vb)
{
    int err;
    uint16_t sct_sc;
    uint8_t opcode;
    char b[80];

    if (fdc_p->dev_fd < 0) {
        if (vb)
            pr2ws("%s: is_nvme is true but dev_fd<0, inconsistent\n",
                  __func__);
        return -EINVAL;
    }
    npcp->buf = dxferp;
    npcp->len = len;
    npcp->is_read = (uint32_t)is_read;
    opcode = npcp->cmd.opc;
    err = ioctl(fdc_p->dev_fd, NVME_PASSTHROUGH_CMD, npcp);
    if (err < 0)
        return -errno;  /* Assume Unix error in normal place */
    sct_sc = (NVME_STATUS_GET_SCT(npcp->cpl.status) << 8) |
             NVME_STATUS_GET_SC(npcp->cpl.status);
    fdc_p->nvme_result = npcp->cpl.cdw0;
    sg_put_unaligned_le32(npcp->cpl.cdw0,
                          fdc_p->cq_dw0_3 + SG_NVME_PT_CQ_RESULT);
    sg_put_unaligned_le32(npcp->cpl.rsvd1, fdc_p->cq_dw0_3 + 4);
    sg_put_unaligned_le16(npcp->cpl.sqhd, fdc_p->cq_dw0_3 + 8);
    sg_put_unaligned_le16(npcp->cpl.sqid, fdc_p->cq_dw0_3 + 10);
    sg_put_unaligned_le16(npcp->cpl.cid, fdc_p->cq_dw0_3 + 12);
    sg_put_unaligned_le16(*((const uint16_t *)&(npcp->cpl.status)),
                          fdc_p->cq_dw0_3 + SG_NVME_PT_CQ_STATUS_P);
    if (sct_sc && (vb > 1)) {
        char nam[64];

        sg_get_nvme_opcode_name(opcode, true, sizeof(nam), nam);
        pr2ws("%s: %s [0x%x], status: %s\n", __func__, nam, opcode,
              sg_get_nvme_cmd_status_str(sct_sc, sizeof(b), b));
    }
    return sct_sc;
}

static void
sntl_check_enclosure_override(struct freebsd_dev_channel * fdc_p, int vb)
{
    uint8_t * up = fdc_p->nvme_id_ctlp;
    uint8_t nvmsr;

    if (NULL == up)
        return;
    nvmsr = up[253];
    if (vb > 3)
        pr2ws("%s: enter, nvmsr=%u\n", __func__, nvmsr);
    fdc_p->dev_stat.id_ctl253 = nvmsr;
    switch (fdc_p->dev_stat.enclosure_override) {
    case 0x0:       /* no override */
        if (0x3 & nvmsr) {
            fdc_p->dev_stat.pdt = PDT_DISK;
            fdc_p->dev_stat.enc_serv = 1;
        } else if (0x2 & nvmsr) {
            fdc_p->dev_stat.pdt = PDT_SES;
            fdc_p->dev_stat.enc_serv = 1;
        } else if (0x1 & nvmsr) {
            fdc_p->dev_stat.pdt = PDT_DISK;
            fdc_p->dev_stat.enc_serv = 0;
        } else {
            uint32_t nn = sg_get_unaligned_le32(up + 516);

            fdc_p->dev_stat.pdt = nn ? PDT_DISK : PDT_UNKNOWN;
            fdc_p->dev_stat.enc_serv = 0;
        }
        break;
    case 0x1:       /* override to SES device */
        fdc_p->dev_stat.pdt = PDT_SES;
        fdc_p->dev_stat.enc_serv = 1;
        break;
    case 0x2:       /* override to disk with attached SES device */
        fdc_p->dev_stat.pdt = PDT_DISK;
        fdc_p->dev_stat.enc_serv = 1;
        break;
    case 0x3:       /* override to SAFTE device (PDT_PROCESSOR) */
        fdc_p->dev_stat.pdt = PDT_PROCESSOR;
        fdc_p->dev_stat.enc_serv = 1;
        break;
    case 0xff:      /* override to normal disk */
        fdc_p->dev_stat.pdt = PDT_DISK;
        fdc_p->dev_stat.enc_serv = 0;
        break;
    default:
        pr2ws("%s: unknown enclosure_override value: %d\n", __func__,
              fdc_p->dev_stat.enclosure_override);
        break;
    }
}

static int
sntl_do_identify(struct freebsd_dev_channel * fdc_p, int cns, int nsid,
                 int u_len, uint8_t * up, int vb)
{
    int err;
    struct nvme_pt_command npc;
    uint8_t * npc_up = (uint8_t *)&npc;

    memset(npc_up, 0, sizeof(npc));
    npc_up[SG_NVME_PT_OPCODE] = 0x6;   /* Identify */
    sg_put_unaligned_le32(nsid, npc_up + SG_NVME_PT_NSID);
    /* CNS=0x1 Identify: controller */
    sg_put_unaligned_le32(cns, npc_up + SG_NVME_PT_CDW10);
    sg_put_unaligned_le64((sg_uintptr_t)up, npc_up + SG_NVME_PT_ADDR);
    sg_put_unaligned_le32(u_len, npc_up + SG_NVME_PT_DATA_LEN);
    err = nvme_pt_low(fdc_p, up, u_len, true, &npc, vb);
    if (err) {
        if (err < 0) {
            if (vb > 1)
                pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n", __func__,
                      strerror(-err), -err);
            return err;
        } else {        /* non-zero NVMe command status */
            fdc_p->nvme_status = err;
            return SG_LIB_NVME_STATUS;
        }
    }
    return 0;
}

/* Currently only caches associated controller response (4096 bytes) */
static int
sntl_cache_identity(struct freebsd_dev_channel * fdc_p, int vb)
{
    int ret;
    uint32_t pg_sz = sg_get_page_size();

    fdc_p->nvme_id_ctlp = sg_memalign(pg_sz, pg_sz,
                                      &fdc_p->free_nvme_id_ctlp, vb > 3);
    if (NULL == fdc_p->nvme_id_ctlp) {
        pr2ws("%s: sg_memalign() failed to get memory\n", __func__);
        return -ENOMEM;
    }
    ret = sntl_do_identify(fdc_p, 0x1 /* CNS */, 0 /* nsid */, pg_sz,
                           fdc_p->nvme_id_ctlp, vb);
    if (0 == ret)
        sntl_check_enclosure_override(fdc_p, vb);
    return (ret < 0) ? sg_convert_errno(-ret) : ret;
}

static const char * nvme_scsi_vendor_str = "NVMe    ";
static const uint16_t inq_resp_len = 36;

static int
sntl_inq(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool evpd;
    bool cp_id_ctl = false;
    int res;
    uint16_t n, alloc_len, pg_cd;
    uint32_t pg_sz = sg_get_page_size();
    struct freebsd_dev_channel * fdc_p;
    uint8_t * nvme_id_ns = NULL;
    uint8_t * free_nvme_id_ns = NULL;
    uint8_t inq_dout[256];

    if (vb > 3)
        pr2ws("%s: starting\n", __func__);

    if (0x2 & cdbp[1]) {        /* Reject CmdDt=1 */
        mk_sense_invalid_fld(ptp, true, 1, 1, vb);
        return 0;
    }
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    if (NULL == fdc_p->nvme_id_ctlp) {
        res = sntl_cache_identity(fdc_p, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, fdc_p->nvme_status, vb);
            return 0;
        } else if (res)         /* should be negative errno */
            return res;
    }
    memset(inq_dout, 0, sizeof(inq_dout));
    alloc_len = sg_get_unaligned_be16(cdbp + 3);
    evpd = !!(0x1 & cdbp[1]);
    pg_cd = cdbp[2];
    if (evpd) {         /* VPD page responses */
        switch (pg_cd) {
        case 0:         /* Supported VPD pages VPD page */
            /* inq_dout[0] = (PQ=0)<<5 | (PDT=0); prefer pdt=0xd --> SES */
            inq_dout[1] = pg_cd;
            n = 11;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[4] = 0x0;
            inq_dout[5] = 0x80;
            inq_dout[6] = 0x83;
            inq_dout[7] = 0x86;
            inq_dout[8] = 0x87;
            inq_dout[9] = 0x92;
            inq_dout[n - 1] = SG_NVME_VPD_NICR;     /* last VPD number */
            break;
        case 0x80:      /* Serial number VPD page */
            /* inq_dout[0] = (PQ=0)<<5 | (PDT=0); prefer pdt=0xd --> SES */
            inq_dout[1] = pg_cd;
            n = 24;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            memcpy(inq_dout + 4, fdc_p->nvme_id_ctlp + 4, 20);    /* SN */
            break;
        case 0x83:      /* Device identification VPD page */
            if ((fdc_p->nsid > 0) && (fdc_p->nsid < SG_NVME_BROADCAST_NSID)) {
                nvme_id_ns = sg_memalign(pg_sz, pg_sz, &free_nvme_id_ns,
                                         vb > 3);
                if (nvme_id_ns) {
                    struct nvme_pt_command npc;
                    uint8_t * npc_up = (uint8_t *)&npc;

                    memset(npc_up, 0, sizeof(npc));
                    npc_up[SG_NVME_PT_OPCODE] = 0x6;   /* Identify */
                    sg_put_unaligned_le32(fdc_p->nsid,
                                          npc_up + SG_NVME_PT_NSID);
                    /* CNS=0x0 Identify: namespace */
                    sg_put_unaligned_le32(0x0, npc_up + SG_NVME_PT_CDW10);
                    sg_put_unaligned_le64((sg_uintptr_t)nvme_id_ns,
                                          npc_up + SG_NVME_PT_ADDR);
                    sg_put_unaligned_le32(pg_sz,
                                          npc_up + SG_NVME_PT_DATA_LEN);
                    res = nvme_pt_low(fdc_p, nvme_id_ns, pg_sz, true, &npc,
                                      vb > 3);
                    if (res) {
                        free(free_nvme_id_ns);
                        free_nvme_id_ns = NULL;
                        nvme_id_ns = NULL;
                    }
                }
            }
            n = sg_make_vpd_devid_for_nvme(fdc_p->nvme_id_ctlp, nvme_id_ns, 0,
                                           -1, inq_dout, sizeof(inq_dout));
            if (n > 3)
                sg_put_unaligned_be16(n - 4, inq_dout + 2);
            if (free_nvme_id_ns) {
                free(free_nvme_id_ns);
                free_nvme_id_ns = NULL;
                nvme_id_ns = NULL;
            }
            break;
        case 0x86:      /* Extended INQUIRY (per SFS SPC Discovery 2016) */
            inq_dout[1] = pg_cd;
            n = 64;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[5] = 0x1;          /* SIMPSUP=1 */
            inq_dout[7] = 0x1;          /* LUICLR=1 */
            inq_dout[13] = 0x40;        /* max supported sense data length */
            break;
        case 0x87:      /* Mode page policy (per SFS SPC Discovery 2016) */
            inq_dout[1] = pg_cd;
            n = 8;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[4] = 0x3f;         /* all mode pages */
            inq_dout[5] = 0xff;         /*     and their sub-pages */
            inq_dout[6] = 0x80;         /* MLUS=1, policy=shared */
            break;
        case 0x92:      /* SCSI Feature set: only SPC Discovery 2016 */
            inq_dout[1] = pg_cd;
            n = 10;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[9] = 0x1;  /* SFS SPC Discovery 2016 */
            break;
        case SG_NVME_VPD_NICR:  /* 0xde */
            inq_dout[1] = pg_cd;
            sg_put_unaligned_be16((16 + 4096) - 4, inq_dout + 2);
            n = 16 + 4096;
            cp_id_ctl = true;
            break;
        default:        /* Point to page_code field in cdb */
            mk_sense_invalid_fld(ptp, true, 2, 7, vb);
            return 0;
        }
        if (alloc_len > 0) {
            n = (alloc_len < n) ? alloc_len : n;
            n = (n < ptp->dxfer_len) ? n : ptp->dxfer_len;
            ptp->resid = ptp->dxfer_len - n;
            if (n > 0) {
                if (cp_id_ctl) {
                    memcpy((uint8_t *)ptp->dxferp, inq_dout,
                           (n < 16 ? n : 16));
                    if (n > 16)
                        memcpy((uint8_t *)ptp->dxferp + 16,
                               fdc_p->nvme_id_ctlp, n - 16);
                } else
                    memcpy((uint8_t *)ptp->dxferp, inq_dout, n);
            }
        }
    } else {            /* Standard INQUIRY response */
        /* pdt=0 --> disk; pdt=0xd --> SES; pdt=3 --> processor (safte) */
        inq_dout[0] = (0x1f & fdc_p->dev_stat.pdt);  /* (PQ=0)<<5 */
        /* inq_dout[1] = (RMD=0)<<7 | (LU_CONG=0)<<6; rest reserved */
        inq_dout[2] = 6;   /* version: SPC-4 */
        inq_dout[3] = 2;   /* NORMACA=0, HISUP=0, response data format: 2 */
        inq_dout[4] = 31;  /* so response length is (or could be) 36 bytes */
        inq_dout[6] = fdc_p->dev_stat.enc_serv ? 0x40 : 0;
        inq_dout[7] = 0x2;    /* CMDQUE=1 */
        memcpy(inq_dout + 8, nvme_scsi_vendor_str, 8);  /* NVMe not Intel */
        memcpy(inq_dout + 16, fdc_p->nvme_id_ctlp + 24, 16);/* Prod <-- MN */
        memcpy(inq_dout + 32, fdc_p->nvme_id_ctlp + 64, 4); /* Rev <-- FR */
        if (alloc_len > 0) {
            n = (alloc_len < inq_resp_len) ? alloc_len : inq_resp_len;
            n = (n < ptp->dxfer_len) ? n : ptp->dxfer_len;
            if (n > 0)
                memcpy((uint8_t *)ptp->dxferp, inq_dout, n);
        }
    }
    return 0;
}

static int
sntl_rluns(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    int res;
    uint16_t sel_report;
    uint32_t alloc_len, k, n, num, max_nsid;
    struct freebsd_dev_channel * fdc_p;
    uint8_t * rl_doutp;
    uint8_t * up;

    if (vb > 3)
        pr2ws("%s: starting\n", __func__);
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    sel_report = cdbp[2];
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (NULL == fdc_p->nvme_id_ctlp) {
        res = sntl_cache_identity(fdc_p, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, fdc_p->nvme_status, vb);
            return 0;
        } else if (res)
            return res;
    }
    max_nsid = sg_get_unaligned_le32(fdc_p->nvme_id_ctlp + 516);
    switch (sel_report) {
    case 0:
    case 2:
        num = max_nsid;
        break;
    case 1:
    case 0x10:
    case 0x12:
        num = 0;
        break;
    case 0x11:
        num = (1 == fdc_p->nsid) ? max_nsid :  0;
        break;
    default:
        if (vb > 1)
            pr2ws("%s: bad select_report value: 0x%x\n", __func__,
                  sel_report);
        mk_sense_invalid_fld(ptp, true, 2, 7, vb);
        return 0;
    }
    rl_doutp = (uint8_t *)calloc(num + 1, 8);
    if (NULL == rl_doutp) {
        pr2ws("%s: calloc() failed to get memory\n", __func__);
        return -ENOMEM;
    }
    for (k = 0, up = rl_doutp + 8; k < num; ++k, up += 8)
        sg_put_unaligned_be16(k, up);
    n = num * 8;
    sg_put_unaligned_be32(n, rl_doutp);
    n+= 8;
    if (alloc_len > 0) {
        n = (alloc_len < n) ? alloc_len : n;
        n = (n < (uint32_t)ptp->dxfer_len) ? n : (uint32_t)ptp->dxfer_len;
        ptp->resid = ptp->dxfer_len - (int)n;
        if (n > 0)
            memcpy((uint8_t *)ptp->dxferp, rl_doutp, n);
    }
    res = 0;
    free(rl_doutp);
    return res;
}

static int
sntl_tur(struct sg_pt_freebsd_scsi * ptp, int vb)
{
    int res, err;
    uint32_t pow_state;
    struct nvme_pt_command npc;
    uint8_t * npc_up = (uint8_t *)&npc;
    struct freebsd_dev_channel * fdc_p;

    if (vb > 3)
        pr2ws("%s: starting\n", __func__);
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    if (NULL == fdc_p->nvme_id_ctlp) {
        res = sntl_cache_identity(fdc_p, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, fdc_p->nvme_status, vb);
            return 0;
        } else if (res)
            return res;
    }
    memset(npc_up, 0, sizeof(npc));
    npc_up[SG_NVME_PT_OPCODE] = 0xa;   /* Get feature */
    sg_put_unaligned_le32(SG_NVME_BROADCAST_NSID, npc_up + SG_NVME_PT_NSID);
    /* SEL=0 (current), Feature=2 Power Management */
    sg_put_unaligned_le32(0x2, npc_up + SG_NVME_PT_CDW10);
    err = nvme_pt_low(fdc_p, NULL, 0, false, &npc, vb);
    if (err) {
        if (err < 0) {
            if (vb > 1)
                pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n", __func__,
                      strerror(-err), -err);
            return err;
        } else {
            fdc_p->nvme_status = err;
            mk_sense_from_nvme_status(ptp, err, vb);
            return 0;
        }
    }
    pow_state = (0x1f & fdc_p->nvme_result);
    if (vb > 3)
        pr2ws("%s: pow_state=%u\n", __func__, pow_state);
#if 0   /* pow_state bounces around too much on laptop */
    if (pow_state)
        mk_sense_asc_ascq(ptp, SPC_SK_NOT_READY, LOW_POWER_COND_ON_ASC, 0,
                          vb);
#endif
    return 0;
}

static int
sntl_req_sense(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool desc;
    int res, err;
    uint32_t pow_state, alloc_len, n;
    struct nvme_pt_command npc;
    uint8_t * npc_up = (uint8_t *)&npc;
    struct freebsd_dev_channel * fdc_p;
    uint8_t rs_dout[64];

    if (vb > 3)
        pr2ws("%s: starting\n", __func__);
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    if (NULL == fdc_p->nvme_id_ctlp) {
        res = sntl_cache_identity(fdc_p, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, fdc_p->nvme_status, vb);
            return 0;
        } else if (res)
            return res;
    }
    desc = !!(0x1 & cdbp[1]);
    alloc_len = cdbp[4];
    memset(npc_up, 0, sizeof(npc));
    npc_up[SG_NVME_PT_OPCODE] = 0xa;   /* Get feature */
    sg_put_unaligned_le32(SG_NVME_BROADCAST_NSID, npc_up + SG_NVME_PT_NSID);
    /* SEL=0 (current), Feature=2 Power Management */
    sg_put_unaligned_le32(0x2, npc_up + SG_NVME_PT_CDW10);
    err = nvme_pt_low(fdc_p, NULL, 0, false, &npc, vb);
    if (err) {
        if (err < 0) {
            if (vb > 1)
                pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n", __func__,
                      strerror(-err), -err);
            return err;
        } else {
            fdc_p->nvme_status = err;
            mk_sense_from_nvme_status(ptp, err, vb);
            return 0;
        }
    }
    pow_state = (0x1f & fdc_p->nvme_result);
    if (vb > 3)
        pr2ws("%s: pow_state=%u\n", __func__, pow_state);
    memset(rs_dout, 0, sizeof(rs_dout));
    if (pow_state)
            sg_build_sense_buffer(desc, rs_dout, SPC_SK_NO_SENSE,
                                  LOW_POWER_COND_ON_ASC, 0);
    else
            sg_build_sense_buffer(desc, rs_dout, SPC_SK_NO_SENSE,
                                  NO_ADDITIONAL_SENSE, 0);
    n = desc ? 8 : 18;
    n = (n < alloc_len) ? n : alloc_len;
        n = (n < (uint32_t)ptp->dxfer_len) ? n : (uint32_t)ptp->dxfer_len;
    ptp->resid = ptp->dxfer_len - (int)n;
    if (n > 0)
        memcpy((uint8_t *)ptp->dxferp, rs_dout, n);
    return 0;
}

static int
sntl_mode_ss(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool is_msense = (SCSI_MODE_SENSE10_OPC == cdbp[0]);
    int res, n, len;
    uint8_t * bp;
    struct freebsd_dev_channel * fdc_p;
    struct sg_sntl_result_t sntl_result;

    if (vb > 3)
        pr2ws("%s: mse%s\n", __func__, (is_msense ? "nse" : "lect"));
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    if (NULL == fdc_p->nvme_id_ctlp) {
        res = sntl_cache_identity(fdc_p, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, fdc_p->nvme_status, vb);
            return 0;
        } else if (res)
            return res;
    }
    if (is_msense) {    /* MODE SENSE(10) */
        len = ptp->dxfer_len;
        bp = ptp->dxferp;
        n = sntl_resp_mode_sense10(&fdc_p->dev_stat, cdbp, bp, len,
                                   &sntl_result);
        ptp->resid = (n >= 0) ? len - n : len;
    } else {            /* MODE SELECT(10) */
        uint8_t pre_enc_ov = fdc_p->dev_stat.enclosure_override;

        len = ptp->dxfer_len;
        bp = ptp->dxferp;
        n = sntl_resp_mode_select10(&fdc_p->dev_stat, cdbp, bp, len,
                                    &sntl_result);
        if (pre_enc_ov != fdc_p->dev_stat.enclosure_override)
            sntl_check_enclosure_override(fdc_p, vb);  /* ENC_OV has changed */
    }
    if (n < 0) {
        int in_bit = (255 == sntl_result.in_bit) ? (int)sntl_result.in_bit :
                                                   -1;
        if ((SAM_STAT_CHECK_CONDITION == sntl_result.sstatus) &&
            (SPC_SK_ILLEGAL_REQUEST == sntl_result.sk)) {
            if (INVALID_FIELD_IN_CDB == sntl_result.asc)
                mk_sense_invalid_fld(ptp, true, sntl_result.in_byte, in_bit,
                                     vb);
            else if (INVALID_FIELD_IN_PARAM_LIST == sntl_result.asc)
                mk_sense_invalid_fld(ptp, false, sntl_result.in_byte, in_bit,
                                     vb);
            else
                mk_sense_asc_ascq(ptp, sntl_result.sk, sntl_result.asc,
                                  sntl_result.ascq, vb);
        } else
            pr2ws("%s: error but no sense?? n=%d\n", __func__, n);
    }
    return 0;
}

/* This is not really a SNTL. For SCSI SEND DIAGNOSTIC(PF=1) NVMe-MI
 * has a special command (SES Send) to tunnel through pages to an
 * enclosure. The NVMe enclosure is meant to understand the SES
 * (SCSI Enclosure Services) use of diagnostics pages that are
 * related to SES. */
static int
sntl_senddiag(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool pf, self_test;
    int err;
    uint8_t st_cd, dpg_cd;
    uint32_t alloc_len, n, dout_len, dpg_len, nvme_dst;
    const uint8_t * dop;
    struct nvme_pt_command npc;
    uint8_t * npc_up = (uint8_t *)&npc;
    struct freebsd_dev_channel * fdc_p;

    st_cd = 0x7 & (cdbp[1] >> 5);
    pf = !! (0x4 & cdbp[1]);
    self_test = !! (0x10 & cdbp[1]);
    if (vb > 3)
        pr2ws("%s: pf=%d, self_test=%d, st_code=%d\n", __func__, (int)pf,
              (int)self_test, (int)st_cd);
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    if (self_test || st_cd) {
        memset(npc_up, 0, sizeof(npc));
        npc_up[SG_NVME_PT_OPCODE] = 0x14;   /* Device self-test */
        /* just this namespace (if there is one) and controller */
        sg_put_unaligned_le32(fdc_p->nsid, npc_up + SG_NVME_PT_NSID);
        switch (st_cd) {
        case 0: /* Here if self_test is set, do short self-test */
        case 1: /* Background short */
        case 5: /* Foreground short */
            nvme_dst = 1;
            break;
        case 2: /* Background extended */
        case 6: /* Foreground extended */
            nvme_dst = 2;
            break;
        case 4: /* Abort self-test */
            nvme_dst = 0xf;
            break;
        default:
            pr2ws("%s: bad self-test code [0x%x]\n", __func__, st_cd);
            mk_sense_invalid_fld(ptp, true, 1, 7, vb);
            return 0;
        }
        sg_put_unaligned_le32(nvme_dst, npc_up + SG_NVME_PT_CDW10);
        err = nvme_pt_low(fdc_p, NULL, 0x0, false, &npc, vb);
        goto do_low;
    }
    alloc_len = sg_get_unaligned_be16(cdbp + 3); /* parameter list length */
    dout_len = ptp->dxfer_len;
    if (pf) {
        if (0 == alloc_len) {
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: PF bit set bit param_list_len=0\n", __func__);
            return 0;
        }
    } else {    /* PF bit clear */
        if (alloc_len) {
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: param_list_len>0 but PF clear\n", __func__);
            return 0;
        } else
            return 0;     /* nothing to do */
        if (dout_len > 0) {
            if (vb)
                pr2ws("%s: dout given but PF clear\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
    }
    if (dout_len < 4) {
        if (vb)
            pr2ws("%s: dout length (%u bytes) too short\n", __func__,
                  dout_len);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    n = dout_len;
    n = (n < alloc_len) ? n : alloc_len;
    dop = (const uint8_t *)ptp->dxferp;
    if (! sg_is_aligned(dop, 0)) {
        if (vb)
            pr2ws("%s: dout [0x%" PRIx64 "] not page aligned\n", __func__,
                  (uint64_t)ptp->dxferp);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    dpg_cd = dop[0];
    dpg_len = sg_get_unaligned_be16(dop + 2) + 4;
    /* should we allow for more than one D_PG is dout ?? */
    n = (n < dpg_len) ? n : dpg_len;    /* not yet ... */

    if (vb)
        pr2ws("%s: passing through d_pg=0x%x, len=%u to NVME_MI SES send\n",
              __func__, dpg_cd, dpg_len);
    memset(npc_up, 0, sizeof(npc));
    npc_up[SG_NVME_PT_OPCODE] = 0x1d;  /* MI send; same opcode as SEND DIAG */
    sg_put_unaligned_le64((sg_uintptr_t)ptp->dxferp,
                          npc_up + SG_NVME_PT_ADDR);
    /* NVMe 4k page size. Maybe determine this? */
    /* dout_len > 0x1000, is this a problem?? */
    sg_put_unaligned_le32(0x1000, npc_up + SG_NVME_PT_DATA_LEN);
    /* NVMe Message Header */
    sg_put_unaligned_le32(0x0804, npc_up + SG_NVME_PT_CDW10);
    /* nvme_mi_ses_send; (0x8 -> mi_ses_recv) */
    sg_put_unaligned_le32(0x9, npc_up + SG_NVME_PT_CDW11);
    /* data-out length I hope */
    sg_put_unaligned_le32(n, npc_up + SG_NVME_PT_CDW13);
    err = nvme_pt_low(fdc_p, ptp->dxferp, 0x1000, false, &npc, vb);
do_low:
    if (err) {
        if (err < 0) {
            if (vb > 1)
                pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n",
                      __func__, strerror(-err), -err);
            return err;
        } else {
            fdc_p->nvme_status = err;
            mk_sense_from_nvme_status(ptp, err, vb);
            return 0;
        }
    }
    return 0;
}

/* This is not really a SNTL. For SCSI RECEIVE DIAGNOSTIC RESULTS(PCV=1)
 * NVMe-MI has a special command (SES Receive) to read pages through a
 * tunnel from an enclosure. The NVMe enclosure is meant to understand the
 * SES (SCSI Enclosure Services) use of diagnostics pages that are
 * related to SES. */
static int
sntl_recvdiag(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool pcv;
    int err;
    uint8_t dpg_cd;
    uint32_t alloc_len, n, din_len;
    const uint8_t * dip;
    struct nvme_pt_command npc;
    uint8_t * npc_up = (uint8_t *)&npc;
    struct freebsd_dev_channel * fdc_p;

    pcv = !! (0x1 & cdbp[1]);
    dpg_cd = cdbp[2];
    alloc_len = sg_get_unaligned_be16(cdbp + 3); /* parameter list length */
    if (vb > 3)
        pr2ws("%s: dpg_cd=0x%x, pcv=%d, alloc_len=0x%x\n", __func__,
              dpg_cd, (int)pcv, alloc_len);
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    din_len = ptp->dxfer_len;
    if (pcv) {
        if (0 == alloc_len) {
            /* T10 says not an error, hmmm */
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: PCV bit set bit but alloc_len=0\n", __func__);
            return 0;
        }
    } else {    /* PCV bit clear */
        if (alloc_len) {
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: alloc_len>0 but PCV clear\n", __func__);
            return 0;
        } else
            return 0;     /* nothing to do */
        if (din_len > 0) {
            if (vb)
                pr2ws("%s: din given but PCV clear\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
    }
    n = din_len;
    n = (n < alloc_len) ? n : alloc_len;
    dip = (const uint8_t *)ptp->dxferp;
    if (! sg_is_aligned(dip, 0)) {
        if (vb)
            pr2ws("%s: din [0x%" PRIx64 "] not page aligned\n", __func__,
                  (uint64_t)ptp->dxferp);
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if (vb)
        pr2ws("%s: expecting d_pg=0x%x from NVME_MI SES receive\n", __func__,
              dpg_cd);
    memset(npc_up, 0, sizeof(npc));
    npc_up[SG_NVME_PT_OPCODE] = 0x1e;  /* MI receive */
    sg_put_unaligned_le64((sg_uintptr_t)ptp->dxferp,
                          npc_up + SG_NVME_PT_ADDR);
    /* NVMe 4k page size. Maybe determine this? */
    /* dout_len > 0x1000, is this a problem?? */
    sg_put_unaligned_le32(0x1000, npc_up + SG_NVME_PT_DATA_LEN);
    /* NVMe Message Header */
    sg_put_unaligned_le32(0x0804, npc_up + SG_NVME_PT_CDW10);
    /* nvme_mi_ses_receive */
    sg_put_unaligned_le32(0x8, npc_up + SG_NVME_PT_CDW11);
    sg_put_unaligned_le32(dpg_cd, npc_up + SG_NVME_PT_CDW12);
    /* data-in length I hope */
    sg_put_unaligned_le32(n, npc_up + SG_NVME_PT_CDW13);
    err = nvme_pt_low(fdc_p, ptp->dxferp, 0x1000, true, &npc, vb);
    if (err) {
        if (err < 0) {
            if (vb > 1)
                pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n",
                      __func__, strerror(-err), -err);
            return err;
        } else {
            fdc_p->nvme_status = err;
            mk_sense_from_nvme_status(ptp, err, vb);
            return 0;
        }
    }
    ptp->resid = din_len - n;
    return 0;
}

#define F_SA_LOW                0x80    /* cdb byte 1, bits 4 to 0 */
#define F_SA_HIGH               0x100   /* as used by variable length cdbs */
#define FF_SA (F_SA_HIGH | F_SA_LOW)
#define F_INV_OP                0x200

static int
sntl_rep_opcodes(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp,
                 int vb)
{
    bool rctd;
    uint8_t reporting_opts, req_opcode, supp;
    uint16_t req_sa, u;
    uint32_t alloc_len, offset, a_len;
    uint32_t pg_sz = sg_get_page_size();
    int k, len, count, bump;
    const struct sg_opcode_info_t *oip;
    uint8_t *arr;
    uint8_t *free_arr;

    if (vb > 3)
        pr2ws("%s: start\n", __func__);
    rctd = !!(cdbp[2] & 0x80);      /* report command timeout desc. */
    reporting_opts = cdbp[2] & 0x7;
    req_opcode = cdbp[3];
    req_sa = sg_get_unaligned_be16(cdbp + 4);
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (alloc_len < 4 || alloc_len > 0xffff) {
        mk_sense_invalid_fld(ptp, true, 6, -1, vb);
        return 0;
    }
    a_len = pg_sz - 72;
    arr = sg_memalign(pg_sz, pg_sz, &free_arr, vb > 3);
    if (NULL == arr) {
        pr2ws("%s: calloc() failed to get memory\n", __func__);
        return -ENOMEM;
    }
    switch (reporting_opts) {
    case 0: /* all commands */
        count = 0;
        bump = rctd ? 20 : 8;
        for (offset = 4, oip = sg_get_opcode_translation();
             (oip->flags != 0xffff) && (offset < a_len); ++oip) {
            if (F_INV_OP & oip->flags)
                continue;
            ++count;
            arr[offset] = oip->opcode;
            sg_put_unaligned_be16(oip->sa, arr + offset + 2);
            if (rctd)
                arr[offset + 5] |= 0x2;
            if (FF_SA & oip->flags)
                arr[offset + 5] |= 0x1;
            sg_put_unaligned_be16(oip->len_mask[0], arr + offset + 6);
            if (rctd)
                sg_put_unaligned_be16(0xa, arr + offset + 8);
            offset += bump;
        }
        sg_put_unaligned_be32(count * bump, arr + 0);
        break;
    case 1: /* one command: opcode only */
    case 2: /* one command: opcode plus service action */
    case 3: /* one command: if sa==0 then opcode only else opcode+sa */
        for (oip = sg_get_opcode_translation(); oip->flags != 0xffff; ++oip) {
            if ((req_opcode == oip->opcode) && (req_sa == oip->sa))
                break;
        }
        if ((0xffff == oip->flags) || (F_INV_OP & oip->flags)) {
            supp = 1;
            offset = 4;
        } else {
            if (1 == reporting_opts) {
                if (FF_SA & oip->flags) {
                    mk_sense_invalid_fld(ptp, true, 2, 2, vb);
                    free(free_arr);
                    return 0;
                }
                req_sa = 0;
            } else if ((2 == reporting_opts) && 0 == (FF_SA & oip->flags)) {
                mk_sense_invalid_fld(ptp, true, 4, -1, vb);
                free(free_arr);
                return 0;
            }
            if ((0 == (FF_SA & oip->flags)) && (req_opcode == oip->opcode))
                supp = 3;
            else if (0 == (FF_SA & oip->flags))
                supp = 1;
            else if (req_sa != oip->sa)
                supp = 1;
            else
                supp = 3;
            if (3 == supp) {
                u = oip->len_mask[0];
                sg_put_unaligned_be16(u, arr + 2);
                arr[4] = oip->opcode;
                for (k = 1; k < u; ++k)
                    arr[4 + k] = (k < 16) ?
                oip->len_mask[k] : 0xff;
                offset = 4 + u;
            } else
                offset = 4;
        }
        arr[1] = (rctd ? 0x80 : 0) | supp;
        if (rctd) {
            sg_put_unaligned_be16(0xa, arr + offset);
            offset += 12;
        }
        break;
    default:
        mk_sense_invalid_fld(ptp, true, 2, 2, vb);
        free(free_arr);
        return 0;
    }
    offset = (offset < a_len) ? offset : a_len;
    len = (offset < alloc_len) ? offset : alloc_len;
    ptp->resid = ptp->dxfer_len - (int)len;
    if (len > 0)
        memcpy((uint8_t *)ptp->dxferp, arr, len);
    free(free_arr);
    return 0;
}

static int
sntl_rep_tmfs(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool repd;
    uint32_t alloc_len, len;
    uint8_t arr[16];

    if (vb > 3)
        pr2ws("%s: start\n", __func__);
    memset(arr, 0, sizeof(arr));
    repd = !!(cdbp[2] & 0x80);
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (alloc_len < 4) {
        mk_sense_invalid_fld(ptp, true, 6, -1, vb);
        return 0;
    }
    arr[0] = 0xc8;          /* ATS | ATSS | LURS */
    arr[1] = 0x1;           /* ITNRS */
    if (repd) {
        arr[3] = 0xc;
        len = 16;
    } else
        len = 4;

    len = (len < alloc_len) ? len : alloc_len;
    ptp->resid = ptp->dxfer_len - (int)len;
    if (len > 0)
        memcpy((uint8_t *)ptp->dxferp, arr, len);
    return 0;
}

/* Note that the "Returned logical block address" (RLBA) field in the SCSI
 * READ CAPACITY (10+16) command's response provides the address of the _last_
 * LBA (counting origin 0) which will be one less that the "size" in the
 * NVMe Identify command response's NSZE field. One problem is that in
 * some situations NSZE can be zero: temporarily set RLBA field to 0
 * (implying a 1 LB logical units size) pending further research. The LBLIB
 * is the "Logical Block Length In Bytes" field in the RCAP response. */
static int
sntl_readcap(struct sg_pt_freebsd_scsi * ptp, const uint8_t * cdbp, int vb)
{
    bool is_rcap10 = (SCSI_READ_CAPACITY10_OPC == cdbp[0]);
    int res, n, len, alloc_len, dps;
    uint8_t flbas, index, lbads;  /* NVMe: 2**LBADS --> Logical Block size */
    uint32_t lbafx;     /* NVME: LBAF0...LBAF15, each 16 bytes */
    uint32_t pg_sz = sg_get_page_size();
    uint64_t nsze;
    uint8_t * bp;
    uint8_t * up;
    uint8_t * free_up = NULL;
    struct freebsd_dev_channel * fdc_p;
    uint8_t resp[32];

    if (vb > 3)
        pr2ws("%s: RCAP%d\n", __func__, (is_rcap10 ? 10 : 16));
    fdc_p = get_fdc_p(ptp);
    if (NULL == fdc_p) {
        pr2ws("%s: get_fdc_p() failed, no file descriptor ?\n", __func__);
        return -EINVAL;
    }
    up = sg_memalign(pg_sz, pg_sz, &free_up, false);
    if (NULL == up) {
        pr2ws("%s: sg_memalign() failed to get memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    res = sntl_do_identify(fdc_p, 0x0 /* CNS */, fdc_p->nsid, pg_sz, up,
                           vb);
    if (res < 0) {
        res = sg_convert_errno(-res);
        goto fini;
    }
    memset(resp, 0, sizeof(resp));
    nsze = sg_get_unaligned_le64(up + 0);
    flbas = up[26];     /* NVME FLBAS field from Identify, want LBAF[flbas] */
    index = 128 + (4 * (flbas & 0xf));
    lbafx = sg_get_unaligned_le32(up + index);
    lbads = (lbafx >> 16) & 0xff;       /* bits 16 to 23 inclusive, pow2 */
    if (is_rcap10) {
        alloc_len = 8;  /* implicit, not in cdb */
        if (nsze > 0xffffffff)
            sg_put_unaligned_be32(0xffffffff, resp + 0);
        else if (0 == nsze)     /* no good answer here */
            sg_put_unaligned_be32(0, resp + 0);         /* SCSI RLBA field */
        else
            sg_put_unaligned_be32((uint32_t)(nsze - 1), resp + 0);
        sg_put_unaligned_be32(1 << lbads, resp + 4);    /* SCSI LBLIB field */
    } else {
        alloc_len = sg_get_unaligned_be32(cdbp + 10);
        dps = up[29];
        if (0x7 & dps) {
            resp[12] = 0x1;
            n = (0x7 & dps) - 1;
            if (n > 0)
                resp[12] |= (n + n);
        }
        if (0 == nsze)  /* no good answer here */
            sg_put_unaligned_be64(0, resp + 0);
        else
            sg_put_unaligned_be64(nsze - 1, resp + 0);
        sg_put_unaligned_be32(1 << lbads, resp + 8);    /* SCSI LBLIB field */
    }
    len = ptp->dxfer_len;
    bp = ptp->dxferp;
    n = 32;
    n = (n < alloc_len) ? n : alloc_len;
    n = (n < len) ? n : len;
    ptp->resid = len - n;
    if (n > 0)
        memcpy(bp, resp, n);
fini:
    if (free_up)
        free(free_up);
    return res;
}

/* Executes NVMe Admin command (or at least forwards it to lower layers).
 * Returns 0 for success, negative numbers are negated 'errno' values from
 * OS system calls. Positive return values are errors from this package. */
static int
sg_do_nvme_pt(struct sg_pt_base * vp, int fd, int vb)
{
    bool scsi_cdb, in_xfer;
    int n, err, len, io_len;
    uint16_t sct_sc, sa;
    uint8_t * dxferp;
    uint8_t * npc_up;
    struct freebsd_dev_channel * fdc_p;
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    const uint8_t * cdbp;
    struct nvme_pt_command npc;

    npc_up = (uint8_t *)&npc;
    if (vb > 3)
        pr2ws("%s: fd=%d\n", __func__, fd);
    if (! ptp->cdb) {
        if (vb)
            pr2ws("%s: No NVMe command given (set_scsi_pt_cdb())\n",
                  __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    fdc_p = get_fdc_p(ptp);
    if (fd < 0) {
        if (NULL == fdc_p) {
            pr2ws("%s: no device handle in object or fd ?\n", __func__);
            return -EINVAL;
        }
    } else {
        int han = fd - FREEBSD_FDOFFSET;

        if ((han < 0) || (han >= FREEBSD_MAXDEV)) {
            pr2ws("%s: argument 'fd' is bad\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        if (NULL == devicetable[han]) {
            pr2ws("%s: argument 'fd' is bad (2)\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        if (fdc_p && (fdc_p != devicetable[han])) {
            pr2ws("%s: different device handle in object and fd ?\n",
                  __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        if (NULL == fdc_p) {
            ptp->dev_han = fd;
            fdc_p = devicetable[han];
        }
    }

    n = ptp->cdb_len;
    cdbp = (const uint8_t *)ptp->cdb;
    if (vb > 3)
        pr2ws("%s: opcode=0x%x, fd=%d\n", __func__, cdbp[0], fd);
    scsi_cdb = sg_is_scsi_cdb(cdbp, n);
    /* nvme_our_sntl is false when NVMe command (64 byte) has been given */
    ptp->nvme_our_sntl = scsi_cdb;
    fdc_p->nvme_our_sntl = ptp->nvme_our_sntl;
    if (scsi_cdb) {
        switch (cdbp[0]) {
        case SCSI_INQUIRY_OPC:
            return sntl_inq(ptp, cdbp, vb);
        case SCSI_REPORT_LUNS_OPC:
            return sntl_rluns(ptp, cdbp, vb);
        case SCSI_TEST_UNIT_READY_OPC:
            return sntl_tur(ptp, vb);
        case SCSI_REQUEST_SENSE_OPC:
            return sntl_req_sense(ptp, cdbp, vb);
        case SCSI_SEND_DIAGNOSTIC_OPC:
            return sntl_senddiag(ptp, cdbp, vb);
        case SCSI_RECEIVE_DIAGNOSTIC_OPC:
            return sntl_recvdiag(ptp, cdbp, vb);
        case SCSI_MODE_SENSE10_OPC:
        case SCSI_MODE_SELECT10_OPC:
            return sntl_mode_ss(ptp, cdbp, vb);
        case SCSI_READ_CAPACITY10_OPC:
            return sntl_readcap(ptp, cdbp, vb);
        case SCSI_SERVICE_ACT_IN_OPC:
            if (SCSI_READ_CAPACITY16_SA == (cdbp[1] & SCSI_SA_MSK))
                return sntl_readcap(ptp, cdbp, vb);
            goto fini;
        case SCSI_MAINT_IN_OPC:
            sa = 0x1f & cdbp[1];        /* service action */
            if (SCSI_REP_SUP_OPCS_OPC == sa)
                return sntl_rep_opcodes(ptp, cdbp, vb);
            else if (SCSI_REP_SUP_TMFS_OPC == sa)
                return sntl_rep_tmfs(ptp, cdbp, vb);
            /* fall through */
        default:
fini:
            if (vb > 2) {
                char b[64];

                sg_get_command_name(cdbp, -1, sizeof(b), b);
                pr2ws("%s: no translation to NVMe for SCSI %s command\n",
                      __func__, b);
            }
            mk_sense_asc_ascq(ptp, SPC_SK_ILLEGAL_REQUEST, INVALID_OPCODE,
                              0, vb);
            return 0;
        }
    }
    /* NVMe command given to pass-through */
    len = (int)sizeof(npc.cmd);
    n = (n < len) ? n : len;
    if (n < 64) {
        if (vb)
            pr2ws("%s: command length of %d bytes is too short\n", __func__,
                  n);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    memcpy(npc_up, (const uint8_t *)ptp->cdb, n);
    if (n < len)        /* zero out rest of 'npc' */
        memset(npc_up + n, 0, len - n);
    in_xfer = false;
    io_len = 0;
    dxferp = NULL;
    if (ptp->dxfer_ilen > 0) {
        in_xfer = true;
        io_len = ptp->dxfer_ilen;
        dxferp = ptp->dxferip;
        sg_put_unaligned_le32(ptp->dxfer_ilen, npc_up + SG_NVME_PT_DATA_LEN);
        sg_put_unaligned_le64((sg_uintptr_t)ptp->dxferip,
                              npc_up + SG_NVME_PT_ADDR);
    } else if (ptp->dxfer_olen > 0) {
        in_xfer = false;
        io_len = ptp->dxfer_olen;
        dxferp = ptp->dxferop;
        sg_put_unaligned_le32(ptp->dxfer_olen, npc_up + SG_NVME_PT_DATA_LEN);
        sg_put_unaligned_le64((sg_uintptr_t)ptp->dxferop,
                              npc_up + SG_NVME_PT_ADDR);
    }
    err = nvme_pt_low(fdc_p, dxferp, io_len, in_xfer, &npc, vb);
    if (err < 0) {
        if (vb > 1)
            pr2ws("%s: do_nvme_pt() failed: %s (errno=%d)\n",
                  __func__, strerror(-err), -err);
        return err;
    }
    sct_sc = err;       /* ((SCT << 8) | SC) which may be 0 */
    fdc_p->nvme_status = sct_sc;
    if (ptp->sense && (ptp->sense_len > 0)) {
        uint32_t k = sizeof(fdc_p->cq_dw0_3);

        if ((int)k < ptp->sense_len)
            ptp->sense_resid = ptp->sense_len - (int)k;
        else {
            k = ptp->sense_len;
            ptp->sense_resid = 0;
        }
        memcpy(ptp->sense, fdc_p->cq_dw0_3, k);
    }
    if (in_xfer)
        ptp->resid = 0; /* Just hoping ... */
    return sct_sc ? SG_LIB_NVME_STATUS : 0;
}

#else           /* if not(HAVE_NVME && (! IGNORE_NVME)) */

static int
sg_do_nvme_pt(struct sg_pt_base * vp, int fd, int vb)
{
    if (vb) {
        pr2ws("%s: not supported, ", __func__);
#ifdef HAVE_NVME
        pr2ws("HAVE_NVME, ");
#else
        pr2ws("don't HAVE_NVME, ");
#endif

#ifdef IGNORE_NVME
        pr2ws("IGNORE_NVME");
#else
        pr2ws("don't IGNORE_NVME");
#endif
        pr2ws("\n");
        if (NULL == vp)
            pr2ws("%s: object pointer NULL; fd=%d\n", __func__, fd);
    }
    return -ENOTTY;             /* inappropriate ioctl error */
}

#endif          /* (HAVE_NVME && (! IGNORE_NVME)) */

#if (HAVE_NVME && (! IGNORE_NVME))

int
do_nvm_pt(struct sg_pt_base * vp, int submq, int timeout_secs, int vb)
{
    if (vb)
        pr2ws("%s: not supported, ", __func__);
    if (vp) { }
    if (submq) { }
    if (timeout_secs) { }
    return SCSI_PT_DO_NOT_SUPPORTED;
}

#else           /* (HAVE_NVME && (! IGNORE_NVME)) */

int
do_nvm_pt(struct sg_pt_base * vp, int submq, int timeout_secs, int verbose)
{
    if (vb) {
        pr2ws("%s: not supported, ", __func__);
#ifdef HAVE_NVME
        pr2ws("HAVE_NVME, ");
#else
        pr2ws("don't HAVE_NVME, ");
#endif

#ifdef IGNORE_NVME
        pr2ws("IGNORE_NVME");
#else
        pr2ws("don't IGNORE_NVME");
#endif
    }
    if (vp) { }
    if (submq) { }
    if (timeout_secs) { }
    return SCSI_PT_DO_NOT_SUPPORTED;
}

#endif          /* (HAVE_NVME && (! IGNORE_NVME)) */
