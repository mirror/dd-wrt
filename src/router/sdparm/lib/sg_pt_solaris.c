/*
 * Copyright (c) 2007-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* sg_pt_solaris version 1.14 20200724 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>

/* Solaris headers */
#include <sys/scsi/generic/commands.h>
#include <sys/scsi/generic/status.h>
#include <sys/scsi/impl/types.h>
#include <sys/scsi/impl/uscsi.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_pt.h"
#include "sg_lib.h"


#define DEF_TIMEOUT 60       /* 60 seconds */

struct sg_pt_solaris_scsi {
    struct uscsi_cmd uscsi;
    int max_sense_len;
    int in_err;
    int os_err;
    bool is_nvme;
    int dev_fd;
};

struct sg_pt_base {
    struct sg_pt_solaris_scsi impl;
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
 * together. The 'flags' argument is ignored in Solaris.
 * Returns >= 0 if successful, otherwise returns negated errno. */
int
scsi_pt_open_flags(const char * device_name, int flags_arg, int verbose)
{
    int oflags = O_NONBLOCK | O_RDWR;
    int fd;

    flags_arg = flags_arg;  /* ignore flags argument, suppress warning */
    if (verbose > 1) {
        fprintf(sg_warnings_strm ? sg_warnings_strm : stderr,
                "open %s with flags=0x%x\n", device_name, oflags);
    }
    fd = open(device_name, oflags);
    if (fd < 0)
        fd = -errno;
    return fd;
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

struct sg_pt_base *
construct_scsi_pt_obj_with_fd(int dev_fd, int verbose)
{
    struct sg_pt_solaris_scsi * ptp;

    ptp = (struct sg_pt_solaris_scsi *)
          calloc(1, sizeof(struct sg_pt_solaris_scsi));
    if (ptp) {
        ptp->dev_fd = (dev_fd < 0) ? -1 : dev_fd;
        ptp->is_nvme = false;
        ptp->uscsi.uscsi_timeout = DEF_TIMEOUT;
        /* Comment in Illumos suggest USCSI_ISOLATE and USCSI_DIAGNOSE (both)
         * seem to mean "don't retry" which is what we want. */
        ptp->uscsi.uscsi_flags = USCSI_ISOLATE | USCSI_DIAGNOSE |
                                 USCSI_RQENABLE;
    } else if (verbose)
        fprintf(sg_warnings_strm ? sg_warnings_strm : stderr,
                "%s: calloc() out of memory\n", __func__);
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
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (ptp)
        free(ptp);
}

void
clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    bool is_nvme;
    int dev_fd;
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (ptp) {
        is_nvme = ptp->is_nvme;
        dev_fd = ptp->dev_fd;
        memset(ptp, 0, sizeof(struct sg_pt_solaris_scsi));
        ptp->dev_fd = dev_fd;
        ptp->is_nvme = is_nvme;
        ptp->uscsi.uscsi_timeout = DEF_TIMEOUT;
        ptp->uscsi.uscsi_flags = USCSI_ISOLATE | USCSI_DIAGNOSE |
                                 USCSI_RQENABLE;
    }
}

void
partial_clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (ptp) {
        ptp->in_err = 0;
        ptp->os_err = 0;
        ptp->uscsi.uscsi_status = 0;
        ptp->uscsi.uscsi_bufaddr = NULL;
        ptp->uscsi.uscsi_buflen = 0;
        ptp->uscsi.uscsi_flags = USCSI_ISOLATE | USCSI_DIAGNOSE |
                                 USCSI_RQENABLE;
    }
}

void
set_scsi_pt_cdb(struct sg_pt_base * vp, const uint8_t * cdb,
                int cdb_len)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    ptp->uscsi.uscsi_cdb = (char *)cdb;
    ptp->uscsi.uscsi_cdblen = cdb_len;
}

int
get_scsi_pt_cdb_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return ptp->uscsi.uscsi_cdblen;
}

uint8_t *
get_scsi_pt_cdb_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return (uint8_t *)ptp->uscsi.uscsi_cdb;
}

void
set_scsi_pt_sense(struct sg_pt_base * vp, uint8_t * sense,
                  int max_sense_len)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (sense && (max_sense_len > 0))
        memset(sense, 0, max_sense_len);
    ptp->uscsi.uscsi_rqbuf = (char *)sense;
    ptp->uscsi.uscsi_rqlen = max_sense_len;
    ptp->max_sense_len = max_sense_len;
}

/* from device */
void
set_scsi_pt_data_in(struct sg_pt_base * vp, uint8_t * dxferp,
                    int dxfer_len)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (ptp->uscsi.uscsi_bufaddr)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->uscsi.uscsi_bufaddr = (char *)dxferp;
        ptp->uscsi.uscsi_buflen = dxfer_len;
        ptp->uscsi.uscsi_flags = USCSI_READ | USCSI_ISOLATE | USCSI_DIAGNOSE |
                                 USCSI_RQENABLE;
    }
}

/* to device */
void
set_scsi_pt_data_out(struct sg_pt_base * vp, const uint8_t * dxferp,
                     int dxfer_len)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (ptp->uscsi.uscsi_bufaddr)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->uscsi.uscsi_bufaddr = (char *)dxferp;
        ptp->uscsi.uscsi_buflen = dxfer_len;
        ptp->uscsi.uscsi_flags = USCSI_WRITE | USCSI_ISOLATE | USCSI_DIAGNOSE |
                                 USCSI_RQENABLE;
    }
}

void
set_scsi_pt_packet_id(struct sg_pt_base * vp, int pack_id)
{
    // struct sg_pt_solaris_scsi * ptp = &vp->impl;

    vp = vp;                    /* ignore and suppress warning */
    pack_id = pack_id;          /* ignore and suppress warning */
}

void
set_scsi_pt_tag(struct sg_pt_base * vp, uint64_t tag)
{
    // struct sg_pt_solaris_scsi * ptp = &vp->impl;

    vp = vp;                    /* ignore and suppress warning */
    tag = tag;                  /* ignore and suppress warning */
}

/* Note that task management function codes are transport specific */
void
set_scsi_pt_task_management(struct sg_pt_base * vp, int tmf_code)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    ++ptp->in_err;
    tmf_code = tmf_code;        /* dummy to silence compiler */
}

void
set_scsi_pt_task_attr(struct sg_pt_base * vp, int attribute, int priority)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;

    ++ptp->in_err;
    attribute = attribute;      /* dummy to silence compiler */
    priority = priority;        /* dummy to silence compiler */
}

void
set_scsi_pt_flags(struct sg_pt_base * objp, int flags)
{
    /* do nothing, suppress warnings */
    objp = objp;
    flags = flags;
}

/* Executes SCSI command (or at least forwards it to lower layers).
 * Clears os_err field prior to active call (whose result may set it
 * again). */
int
do_scsi_pt(struct sg_pt_base * vp, int fd, int time_secs, int verbose)
{
    struct sg_pt_solaris_scsi * ptp = &vp->impl;
    FILE * ferr = sg_warnings_strm ? sg_warnings_strm : stderr;

    ptp->os_err = 0;
    if (ptp->in_err) {
        if (verbose)
            fprintf(ferr, "Replicated or unused set_scsi_pt... functions\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (fd < 0) {
        if (ptp->dev_fd < 0) {
            if (verbose)
                fprintf(ferr, "%s: No device file descriptor given\n",
                        __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
    } else {
        if (ptp->dev_fd >= 0) {
            if (fd != ptp->dev_fd) {
                if (verbose)
                    fprintf(ferr, "%s: file descriptor given to create and "
                            "this differ\n", __func__);
                return SCSI_PT_DO_BAD_PARAMS;
            }
        } else
            ptp->dev_fd = fd;
    }
    if (NULL == ptp->uscsi.uscsi_cdb) {
        if (verbose)
            fprintf(ferr, "%s: No SCSI command (cdb) given\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (time_secs > 0)
        ptp->uscsi.uscsi_timeout = time_secs;

    if (ioctl(ptp->dev_fd, USCSICMD, &ptp->uscsi)) {
        ptp->os_err = errno;
        if ((EIO == ptp->os_err) && ptp->uscsi.uscsi_status) {
            ptp->os_err = 0;
            return 0;
        }
        if (verbose)
            fprintf(ferr, "%s: ioctl(USCSICMD) failed with os_err (errno) "
                    "= %d\n", __func__, ptp->os_err);
        return -ptp->os_err;
    }
    return 0;
}

int
get_scsi_pt_result_category(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;
    int scsi_st = ptp->uscsi.uscsi_status;

    if (ptp->os_err)
        return SCSI_PT_RESULT_OS_ERR;
    else if ((SAM_STAT_CHECK_CONDITION == scsi_st) ||
             (SAM_STAT_COMMAND_TERMINATED == scsi_st))
        return SCSI_PT_RESULT_SENSE;
    else if (scsi_st)
        return SCSI_PT_RESULT_STATUS;
    else
        return SCSI_PT_RESULT_GOOD;
}

uint32_t
get_pt_result(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return (uint32_t)ptp->uscsi.uscsi_status;
}

int
get_scsi_pt_resid(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return ptp->uscsi.uscsi_resid;
}

void
get_pt_req_lengths(const struct sg_pt_base * vp, int * req_dinp,
                   int * req_doutp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;
    int dxfer_len = ptp->uscsi.uscsi_buflen;
    int flags = ptp->uscsi.uscsi_flags;

    if (req_dinp) {
        if ((dxfer_len > 0) && (USCSI_READ & flags))
            *req_dinp = dxfer_len;
        else
            *req_dinp = 0;
    }
    if (req_doutp) {
        if ((dxfer_len > 0) && (USCSI_WRITE & flags))
            *req_doutp = dxfer_len;
        else
            *req_doutp = 0;
    }
}

void
get_pt_actual_lengths(const struct sg_pt_base * vp, int * act_dinp,
                      int * act_doutp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;
    int dxfer_len = ptp->uscsi.uscsi_buflen;
    int flags = ptp->uscsi.uscsi_flags;

    if (act_dinp) {
        if ((dxfer_len > 0) && (USCSI_READ & flags))
            *act_dinp = dxfer_len - ptp->uscsi.uscsi_resid;
        else
            *act_dinp = 0;
    }
    if (act_doutp) {
        if ((dxfer_len > 0) && (USCSI_WRITE & flags))
            *act_doutp = dxfer_len - ptp->uscsi.uscsi_resid;
        else
            *act_doutp = 0;
    }
}

int
get_scsi_pt_status_response(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return ptp->uscsi.uscsi_status;
}

int
get_scsi_pt_sense_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;
    int res;

    if (ptp->max_sense_len > 0) {
        res = ptp->max_sense_len - ptp->uscsi.uscsi_rqresid;
        return (res > 0) ? res : 0;
    }
    return 0;
}

uint8_t *
get_scsi_pt_sense_buf(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return (uint8_t *)ptp->uscsi.uscsi_rqbuf;
}

int
get_scsi_pt_duration_ms(const struct sg_pt_base * vp)
{
    // const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    vp = vp;            /* ignore and suppress warning */
    return -1;          /* not available */
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
    // const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (vp) { ; }            /* ignore and suppress warning */
    return 0;
}

void
set_scsi_pt_transport_err(struct sg_pt_base * vp, int err)
{
    // const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    if (vp) { ; }            /* ignore and suppress warning */
    if (err) { ; }           /* ignore and suppress warning */
}

int
get_scsi_pt_os_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return ptp->os_err;
}

bool
pt_device_is_nvme(const struct sg_pt_base * vp)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    return ptp ? ptp->is_nvme : false;
}

char *
get_scsi_pt_transport_err_str(const struct sg_pt_base * vp, int max_b_len,
                              char * b)
{
    // const struct sg_pt_solaris_scsi * ptp = &vp->impl;

    vp = vp;            /* ignore and suppress warning */
    if (max_b_len > 0)
        b[0] = '\0';

    return b;
}

char *
get_scsi_pt_os_err_str(const struct sg_pt_base * vp, int max_b_len, char * b)
{
    const struct sg_pt_solaris_scsi * ptp = &vp->impl;
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
