/*
 * Copyright (c) 1999-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SG_LIB_LINUX

#include "sg_io_linux.h"
#include "sg_pr2serr.h"


/* Version 1.11 20200401 */


void
sg_print_masked_status(int masked_status)
{
    int scsi_status = (masked_status << 1) & 0x7e;

    sg_print_scsi_status(scsi_status);
}

/* host_bytes: DID_* are Linux SCSI result (a 32 bit variable) bits 16:23 */

static const char * linux_host_bytes[] = {
    "DID_OK", "DID_NO_CONNECT", "DID_BUS_BUSY", "DID_TIME_OUT",
    "DID_BAD_TARGET", "DID_ABORT", "DID_PARITY", "DID_ERROR",
    "DID_RESET", "DID_BAD_INTR", "DID_PASSTHROUGH", "DID_SOFT_ERROR",
    "DID_IMM_RETRY", "DID_REQUEUE", "DID_TRANSPORT_DISRUPTED",
    "DID_TRANSPORT_FAILFAST", "DID_TARGET_FAILURE", "DID_NEXUS_FAILURE",
    "DID_ALLOC_FAILURE", "DID_MEDIUM_ERROR",
};

void
sg_print_host_status(int host_status)
{
    pr2ws("Host_status=0x%02x ", host_status);
    if ((host_status < 0) ||
        (host_status >= (int)SG_ARRAY_SIZE(linux_host_bytes)))
        pr2ws("is invalid ");
    else
        pr2ws("[%s] ", linux_host_bytes[host_status]);
}

/* DRIVER_* are Linux SCSI result (a 32 bit variable) bits 24:27 */

static const char * linux_driver_bytes[] = {
    "DRIVER_OK", "DRIVER_BUSY", "DRIVER_SOFT", "DRIVER_MEDIA",
    "DRIVER_ERROR", "DRIVER_INVALID", "DRIVER_TIMEOUT", "DRIVER_HARD",
    "DRIVER_SENSE",
};

#if 0

/* SUGGEST_* are Linux SCSI result (a 32 bit variable) bits 28:31 */

static const char * linux_driver_suggests[] = {
    "SUGGEST_OK", "SUGGEST_RETRY", "SUGGEST_ABORT", "SUGGEST_REMAP",
    "SUGGEST_DIE", "UNKNOWN","UNKNOWN","UNKNOWN",
    "SUGGEST_SENSE",
};
#endif


void
sg_print_driver_status(int driver_status)
{
    int driv;
    const char * driv_cp = "invalid";

    driv = driver_status & SG_LIB_DRIVER_MASK;
    if (driv < (int)SG_ARRAY_SIZE(linux_driver_bytes))
        driv_cp = linux_driver_bytes[driv];
#if 0
    sugg = (driver_status & SG_LIB_SUGGEST_MASK) >> 4;
    if (sugg < (int)SG_ARRAY_SIZE(linux_driver_suggests))
        sugg_cp = linux_driver_suggests[sugg];
#endif
    pr2ws("Driver_status=0x%02x", driver_status);
    pr2ws(" [%s] ", driv_cp);
}

/* Returns 1 if no errors found and thus nothing printed; otherwise
 * prints error/warning (prefix by 'leadin') to stderr (pr2ws) and
 * returns 0. */
int
sg_linux_sense_print(const char * leadin, int scsi_status, int host_status,
                     int driver_status, const uint8_t * sense_buffer,
                     int sb_len, bool raw_sinfo)
{
    bool done_leadin = false;
    bool done_sense = false;

    scsi_status &= 0x7e; /*sanity */
    if ((0 == scsi_status) && (0 == host_status) && (0 == driver_status))
        return 1;       /* No problems */
    if (0 != scsi_status) {
        if (leadin)
            pr2ws("%s: ", leadin);
        done_leadin = true;
        pr2ws("SCSI status: ");
        sg_print_scsi_status(scsi_status);
        pr2ws("\n");
        if (sense_buffer && ((scsi_status == SAM_STAT_CHECK_CONDITION) ||
                             (scsi_status == SAM_STAT_COMMAND_TERMINATED))) {
            /* SAM_STAT_COMMAND_TERMINATED is obsolete */
            sg_print_sense(0, sense_buffer, sb_len, raw_sinfo);
            done_sense = true;
        }
    }
    if (0 != host_status) {
        if (leadin && (! done_leadin))
            pr2ws("%s: ", leadin);
        if (done_leadin)
            pr2ws("plus...: ");
        else
            done_leadin = true;
        sg_print_host_status(host_status);
        pr2ws("\n");
    }
    if (0 != driver_status) {
        if (done_sense &&
            (SG_LIB_DRIVER_SENSE == (SG_LIB_DRIVER_MASK & driver_status)))
            return 0;
        if (leadin && (! done_leadin))
            pr2ws("%s: ", leadin);
        if (done_leadin)
            pr2ws("plus...: ");
        sg_print_driver_status(driver_status);
        pr2ws("\n");
        if (sense_buffer && (! done_sense) &&
            (SG_LIB_DRIVER_SENSE == (SG_LIB_DRIVER_MASK & driver_status)))
            sg_print_sense(0, sense_buffer, sb_len, raw_sinfo);
    }
    return 0;
}

#ifdef SG_IO

bool
sg_normalize_sense(const struct sg_io_hdr * hp,
                   struct sg_scsi_sense_hdr * sshp)
{
    if ((NULL == hp) || (0 == hp->sb_len_wr)) {
        if (sshp)
            memset(sshp, 0, sizeof(struct sg_scsi_sense_hdr));
        return 0;
    }
    return sg_scsi_normalize_sense(hp->sbp, hp->sb_len_wr, sshp);
}

/* Returns 1 if no errors found and thus nothing printed; otherwise
   returns 0. */
int
sg_chk_n_print3(const char * leadin, struct sg_io_hdr * hp,
                bool raw_sinfo)
{
    return sg_linux_sense_print(leadin, hp->status, hp->host_status,
                                hp->driver_status, hp->sbp, hp->sb_len_wr,
                                raw_sinfo);
}
#endif

/* Returns 1 if no errors found and thus nothing printed; otherwise
   returns 0. */
int
sg_chk_n_print(const char * leadin, int masked_status, int host_status,
               int driver_status, const uint8_t * sense_buffer,
               int sb_len, bool raw_sinfo)
{
    int scsi_status = (masked_status << 1) & 0x7e;

    return sg_linux_sense_print(leadin, scsi_status, host_status,
                                driver_status, sense_buffer, sb_len,
                                raw_sinfo);
}

#ifdef SG_IO
int
sg_err_category3(struct sg_io_hdr * hp)
{
    return sg_err_category_new(hp->status, hp->host_status,
                               hp->driver_status, hp->sbp, hp->sb_len_wr);
}
#endif

int
sg_err_category(int masked_status, int host_status, int driver_status,
                const uint8_t * sense_buffer, int sb_len)
{
    int scsi_status = (masked_status << 1) & 0x7e;

    return sg_err_category_new(scsi_status, host_status, driver_status,
                               sense_buffer, sb_len);
}

int
sg_err_category_new(int scsi_status, int host_status, int driver_status,
                    const uint8_t * sense_buffer, int sb_len)
{
    int masked_driver_status = (SG_LIB_DRIVER_MASK & driver_status);

    scsi_status &= 0x7e;
    if ((0 == scsi_status) && (0 == host_status) &&
        (0 == masked_driver_status))
        return SG_LIB_CAT_CLEAN;
    if ((SAM_STAT_CHECK_CONDITION == scsi_status) ||
        (SAM_STAT_COMMAND_TERMINATED == scsi_status) ||
        (SG_LIB_DRIVER_SENSE == masked_driver_status))
        return sg_err_category_sense(sense_buffer, sb_len);
    if (0 != host_status) {
        if ((SG_LIB_DID_NO_CONNECT == host_status) ||
            (SG_LIB_DID_BUS_BUSY == host_status) ||
            (SG_LIB_DID_TIME_OUT == host_status))
            return SG_LIB_CAT_TIMEOUT;
        if (SG_LIB_DID_NEXUS_FAILURE == host_status)
            return SG_LIB_CAT_RES_CONFLICT;
    }
    if (SG_LIB_DRIVER_TIMEOUT == masked_driver_status)
        return SG_LIB_CAT_TIMEOUT;
    return SG_LIB_CAT_OTHER;
}

#endif  /* if SG_LIB_LINUX defined */
