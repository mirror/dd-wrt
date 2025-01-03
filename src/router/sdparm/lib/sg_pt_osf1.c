/*
 * Copyright (c) 2005-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <io/common/devgetinfo.h>
#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/uagt.h>
#include <io/cam/rzdisk.h>
#include <io/cam/scsi_opcodes.h>
#include <io/cam/scsi_all.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "sg_pt.h"
#include "sg_lib.h"
#include "sg_pr2serr.h"

/* Version 2.03 20200722 */

#define OSF1_MAXDEV 64

#ifndef CAM_DIR_BOTH
#define CAM_DIR_BOTH 0x0        /* copy value from FreeBSD */
#endif

struct osf1_dev_channel {
    int bus;
    int tgt;
    int lun;
};

// Private table of open devices: guaranteed zero on startup since
// part of static data.
static struct osf1_dev_channel *devicetable[OSF1_MAXDEV] = {0};
static char *cam_dev = "/dev/cam";
static int camfd;
static int camopened = 0;

struct sg_pt_osf1_scsi {
    uint8_t * cdb;
    int cdb_len;
    uint8_t * sense;
    int sense_len;
    uint8_t * dxferp;
    int dxfer_len;
    int dxfer_dir;
    int scsi_status;
    int resid;
    int sense_resid;
    int in_err;
    int os_err;
    int transport_err;
    bool is_nvme;
    int dev_fd;
};

struct sg_pt_base {
    struct sg_pt_osf1_scsi impl;
};


/* Returns >= 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_open_device(const char * device_name, bool read_only, int verbose)
{
    int oflags = 0 /* O_NONBLOCK*/ ;

    oflags |= (read_only ? O_RDONLY : O_RDWR);
    return scsi_pt_open_flags(device_name, oflags, verbose);
}

/* Similar to scsi_pt_open_device() but takes Unix style open flags OR-ed
 * together. The 'flags' argument is ignored in OSF-1.
 * Returns >= 0 if successful, otherwise returns negated errno. */
int
scsi_pt_open_flags(const char * device_name, int flags, int verbose)
{
    struct osf1_dev_channel *fdchan;
    int fd, k;

    if (!camopened) {
        camfd = open(cam_dev, O_RDWR, 0);
        if (camfd < 0)
            return -1;
        camopened++;
    }

    // Search table for a free entry
    for (k = 0; k < OSF1_MAXDEV; k++)
        if (! devicetable[k])
            break;

    if (k == OSF1_MAXDEV) {
        if (verbose)
            pr2ws("too many open devices (%d)\n", OSF1_MAXDEV);
        errno=EMFILE;
        return -1;
    }

    fdchan = (struct osf1_dev_channel *)calloc(1,
                                sizeof(struct osf1_dev_channel));
    if (fdchan == NULL) {
        // errno already set by call to malloc()
        return -1;
    }

    fd = open(device_name, O_RDONLY|O_NONBLOCK);
    if (fd > 0) {
        device_info_t devinfo;
        bzero(&devinfo, sizeof(devinfo));
        if (ioctl(fd, DEVGETINFO, &devinfo) == 0) {
            fdchan->bus = devinfo.v1.businfo.bus.scsi.bus_num;
            fdchan->tgt = devinfo.v1.businfo.bus.scsi.tgt_id;
            fdchan->lun = devinfo.v1.businfo.bus.scsi.lun;
        }
        close (fd);
    } else {
        free(fdchan);
        return -1;
    }

    devicetable[k] = fdchan;
    return k;
}

/* Returns 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_close_device(int device_fd)
{
    struct osf1_dev_channel *fdchan;
    int i;

    if ((device_fd < 0) || (device_fd >= OSF1_MAXDEV)) {
        errno = ENODEV;
        return -1;
    }

    fdchan = devicetable[device_fd];
    if (NULL == fdchan) {
        errno = ENODEV;
        return -1;
    }

    free(fdchan);
    devicetable[device_fd] = NULL;

    for (i = 0; i < OSF1_MAXDEV; i++) {
        if (devicetable[i])
            break;
    }
    if (i == OSF1_MAXDEV) {
        close(camfd);
        camopened = 0;
    }
    return 0;
}

struct sg_pt_base *
construct_scsi_pt_obj_with_fd(int device_fd, int verbose)
{
    struct sg_pt_osf1_scsi * ptp;

    ptp = (struct sg_pt_osf1_scsi *)malloc(sizeof(struct sg_pt_osf1_scsi));
    if (ptp) {
        bzero(ptp, sizeof(struct sg_pt_osf1_scsi));
        ptp->dev_fd = (device_fd < 0) ? -1 : device_fd;
        ptp->is_nvme = false;
        ptp->dxfer_dir = CAM_DIR_NONE;
    } else if (verbose)
        pr2ws("%s: malloc() out of memory\n", __func__);
    return (struct sg_pt_base *)ptp;
}

struct sg_pt_base *
construct_scsi_pt_obj(void)
{
    return construct_scsi_pt_obj_with_fd(-1, 0);
}

void
destruct_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (ptp)
        free(ptp);
}

void
clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    bool is_nvme;
    int dev_fd;
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (ptp) {
        is_nvme = ptp->is_nvme;
        dev_fd = ptp->dev_fd;
        bzero(ptp, sizeof(struct sg_pt_osf1_scsi));
        ptp->dev_fd = dev_fd;
        ptp->is_nvme = is_nvme;
        ptp->dxfer_dir = CAM_DIR_NONE;
    }
}

void
partial_clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (NULL == ptp)
        return;
    ptp->in_err = 0;
    ptp->os_err = 0;
    ptp->transport_err = 0;
    ptp->scsi_status = 0;
    ptp->dxfer_dir = CAM_DIR_NONE;
    ptp->dxferp = NULL;
    ptp->dxfer_len = 0;
}

void
set_scsi_pt_cdb(struct sg_pt_base * vp, const uint8_t * cdb,
                int cdb_len)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    ptp->cdb = (uint8_t *)cdb;
    ptp->cdb_len = cdb_len;
}

int
get_scsi_pt_cdb_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->cdb_len;
}

uint8_t *
get_scsi_pt_cdb_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->cdb;
}

void
set_scsi_pt_sense(struct sg_pt_base * vp, uint8_t * sense,
                  int max_sense_len)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (sense) {
        if (max_sense_len > 0)
            bzero(sense, max_sense_len);
    }
    ptp->sense = sense;
    ptp->sense_len = max_sense_len;
}

/* from device */
void
set_scsi_pt_data_in(struct sg_pt_base * vp, uint8_t * dxferp,
                    int dxfer_len)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (ptp->dxferp)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->dxferp = dxferp;
        ptp->dxfer_len = dxfer_len;
        ptp->dxfer_dir = CAM_DIR_IN;
    }
}

/* to device */
void
set_scsi_pt_data_out(struct sg_pt_base * vp, const uint8_t * dxferp,
                     int dxfer_len)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (ptp->dxferp)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->dxferp = (uint8_t *)dxferp;
        ptp->dxfer_len = dxfer_len;
        ptp->dxfer_dir = CAM_DIR_OUT;
    }
}

void
set_scsi_pt_packet_id(struct sg_pt_base * vp, int pack_id)
{
}

void
set_scsi_pt_tag(struct sg_pt_base * vp, uint64_t tag)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_management(struct sg_pt_base * vp, int tmf_code)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_attr(struct sg_pt_base * vp, int attrib, int priority)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_flags(struct sg_pt_base * objp, int flags)
{
    /* do nothing, suppress warnings */
    objp = objp;
    flags = flags;
}

static int
release_sim(struct sg_pt_base *vp, int device_fd, int verbose) {
    struct sg_pt_osf1_scsi * ptp = &vp->impl;
    struct osf1_dev_channel *fdchan = devicetable[device_fd];
    UAGT_CAM_CCB uagt;
    CCB_RELSIM relsim;
    int retval;

    bzero(&uagt, sizeof(uagt));
    bzero(&relsim, sizeof(relsim));

    uagt.uagt_ccb = (CCB_HEADER *) &relsim;
    uagt.uagt_ccblen = sizeof(relsim);

    relsim.cam_ch.cam_ccb_len = sizeof(relsim);
    relsim.cam_ch.cam_func_code = XPT_REL_SIMQ;
    relsim.cam_ch.cam_flags = CAM_DIR_IN | CAM_DIS_CALLBACK;
    relsim.cam_ch.cam_path_id = fdchan->bus;
    relsim.cam_ch.cam_target_id = fdchan->tgt;
    relsim.cam_ch.cam_target_lun = fdchan->lun;

    retval = ioctl(camfd, UAGT_CAM_IO, &uagt);
    if (retval < 0) {
        if (verbose)
            pr2ws("CAM ioctl error (Release SIM Queue)\n");
    }
    return retval;
}

int
do_scsi_pt(struct sg_pt_base * vp, int device_fd, int time_secs, int verbose)
{
    struct sg_pt_osf1_scsi * ptp = &vp->impl;
    struct osf1_dev_channel *fdchan;
    int len, retval;
    CCB_SCSIIO ccb;
    UAGT_CAM_CCB uagt;
    uint8_t sensep[ADDL_SENSE_LENGTH];


    ptp->os_err = 0;
    if (ptp->in_err) {
        if (verbose)
            pr2ws("Replicated or unused set_scsi_pt...\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (device_fd < 0) {
        if (ptp->dev_fd < 0) {
            if (verbose)
                pr2ws("%s: No device file descriptor given\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
    } else {
        if (ptp->dev_fd >= 0) {
            if (device_fd != ptp->dev_fd) {
                if (verbose)
                    pr2ws("%s: file descriptor given to create and this "
                          "differ\n", __func__);
                return SCSI_PT_DO_BAD_PARAMS;
            }
        } else
            ptp->dev_fd = device_fd;
    }
    if (NULL == ptp->cdb) {
        if (verbose)
            pr2ws("No command (cdb) given\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if ((ptp->dev_fd < 0) || (ptp->dev_fd >= OSF1_MAXDEV)) {
        if (verbose)
            pr2ws("Bad file descriptor\n");
        ptp->os_err = ENODEV;
        return -ptp->os_err;
    }
    fdchan = devicetable[ptp->dev_fd];
    if (NULL == fdchan) {
        if (verbose)
            pr2ws("File descriptor closed??\n");
        ptp->os_err = ENODEV;
        return -ptp->os_err;
    }
    if (0 == camopened) {
        if (verbose)
            pr2ws("No open CAM device\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }

    bzero(&uagt, sizeof(uagt));
    bzero(&ccb, sizeof(ccb));

    uagt.uagt_ccb = (CCB_HEADER *) &ccb;
    uagt.uagt_ccblen = sizeof(ccb);
    uagt.uagt_snsbuf = ccb.cam_sense_ptr = ptp->sense ? ptp->sense : sensep;
    uagt.uagt_snslen = ccb.cam_sense_len = ptp->sense ? ptp->sense_len :
                                                        sizeof sensep;
    uagt.uagt_buffer = ccb.cam_data_ptr =  ptp->dxferp;
    uagt.uagt_buflen = ccb.cam_dxfer_len = ptp->dxfer_len;

    ccb.cam_timeout = time_secs;
    ccb.cam_ch.my_addr = (CCB_HEADER *) &ccb;
    ccb.cam_ch.cam_ccb_len = sizeof(ccb);
    ccb.cam_ch.cam_func_code = XPT_SCSI_IO;
    ccb.cam_ch.cam_flags = ptp->dxfer_dir;
    ccb.cam_cdb_len = ptp->cdb_len;
    memcpy(ccb.cam_cdb_io.cam_cdb_bytes, ptp->cdb, ptp->cdb_len);
    ccb.cam_ch.cam_path_id = fdchan->bus;
    ccb.cam_ch.cam_target_id = fdchan->tgt;
    ccb.cam_ch.cam_target_lun = fdchan->lun;

    if (ioctl(camfd, UAGT_CAM_IO, &uagt) < 0) {
        if (verbose)
            pr2ws("CAN I/O Error\n");
        ptp->os_err = EIO;
        return -ptp->os_err;
    }

    if (((ccb.cam_ch.cam_status & CAM_STATUS_MASK) == CAM_REQ_CMP) ||
            ((ccb.cam_ch.cam_status & CAM_STATUS_MASK) == CAM_REQ_CMP_ERR)) {
        ptp->scsi_status = ccb.cam_scsi_status;
        ptp->resid = ccb.cam_resid;
        if (ptp->sense)
            ptp->sense_resid = ccb.cam_sense_resid;
    } else {
        ptp->transport_err = 1;
    }

    /* If the SIM queue is frozen, release SIM queue. */
    if (ccb.cam_ch.cam_status & CAM_SIM_QFRZN)
        release_sim(vp, ptp->dev_fd, verbose);

    return 0;
}

int
get_scsi_pt_result_category(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

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
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->resid;
}

void
get_pt_req_lengths(const struct sg_pt_base * vp, int * req_dinp,
                   int * req_doutp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;
    bool bidi = (ptp->dxfer_dir == CAM_DIR_BOTH);

    if (req_dinp) {
        if (ptp->dxfer_len > 0)
            *req_dinp = ptp->dxfer_len;
        else
            *req_dinp = 0;
    }
    if (req_doutp) {
        if ((!bidi) && (ptp->dxfer_len > 0))
            *req_doutp = ptp->dxfer_len;
        else
            *req_doutp = 0;
    }
}

void
get_pt_actual_lengths(const struct sg_pt_base * vp, int * act_dinp,
                      int * act_doutp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;
    bool bidi = (ptp->dxfer_dir == CAM_DIR_BOTH);

    if (act_dinp) {
        if (ptp->dxfer_len > 0)
            *act_dinp = ptp->dxfer_len - ptp->resid;
        else
            *act_dinp = 0;
    }
    if (act_doutp) {
        if ((!bidi) && (ptp->dxfer_len > 0))
            *act_doutp = ptp->dxfer_len - ptp->resid;
        else
            *act_doutp = 0;
    }
}


int
get_scsi_pt_status_response(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->scsi_status;
}

int
get_scsi_pt_sense_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;
    int len;

    len = ptp->sense_len - ptp->sense_resid;
    return (len > 0) ? len : 0;
}

uint8_t *
get_scsi_pt_sense_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->sense;
}

int
get_scsi_pt_duration_ms(const struct sg_pt_base * vp)
{
    // const struct sg_pt_osf1_scsi * ptp = &vp->impl;

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
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->transport_err;
}

int
get_scsi_pt_os_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp->os_err;
}

bool
pt_device_is_nvme(const struct sg_pt_base * vp)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    return ptp ? ptp->is_nvme : false;
}

char *
get_scsi_pt_transport_err_str(const struct sg_pt_base * vp, int max_b_len,
                              char * b)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;

    if (0 == ptp->transport_err) {
        strncpy(b, "no transport error available", max_b_len);
        b[max_b_len - 1] = '\0';
        return b;
    }
    strncpy(b, "no transport error available", max_b_len);
    b[max_b_len - 1] = '\0';
    return b;
}

char *
get_scsi_pt_os_err_str(const struct sg_pt_base * vp, int max_b_len, char * b)
{
    const struct sg_pt_osf1_scsi * ptp = &vp->impl;
    const char * cp;

    cp = safe_strerror(ptp->os_err);
    strncpy(b, cp, max_b_len);
    if ((int)strlen(cp) >= max_b_len)
        b[max_b_len - 1] = '\0';
    return b;
}

int
do_nvm_pt(struct sg_pt_base * vp, int submq, int timeout_secs, int verbose)
{
    if (vp) { }
    if (submq) { }
    if (timeout_secs) { }
    if (verbose) { }
    return SCSI_PT_DO_NOT_SUPPORTED;
}
