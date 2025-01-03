#ifndef SG_IO_LINUX_H
#define SG_IO_LINUX_H

/*
 * Copyright (c) 2004-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * Version 1.08 [20201102]
 */

/*
 * This header file contains Linux specific information related to the SCSI
 * command pass through in the SCSI generic (sg) driver and the Linux
 * block layer.
 */

#include "sg_lib.h"
#include "sg_linux_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* host_bytes: DID_* are Linux SCSI result (a 32 bit variable) bits 16:23 */
#ifndef DID_OK
#define DID_OK 0x00
#endif
#ifndef DID_NO_CONNECT
#define DID_NO_CONNECT 0x01     /* Unable to connect before timeout */
#define DID_BUS_BUSY 0x02       /* Bus remain busy until timeout */
#define DID_TIME_OUT 0x03       /* Timed out for some other reason */
#define DID_BAD_TARGET 0x04     /* Bad target (id?) */
#define DID_ABORT 0x05          /* Told to abort for some other reason */
#define DID_PARITY 0x06         /* Parity error (on SCSI bus) */
#define DID_ERROR 0x07          /* Internal error */
#define DID_RESET 0x08          /* Reset by somebody */
#define DID_BAD_INTR 0x09       /* Received an unexpected interrupt */
#define DID_PASSTHROUGH 0x0a    /* Force command past mid-level */
#define DID_SOFT_ERROR 0x0b     /* The low-level driver wants a retry */
#endif
#ifndef DID_IMM_RETRY
#define DID_IMM_RETRY 0x0c      /* Retry without decrementing retry count  */
#endif
#ifndef DID_REQUEUE
#define DID_REQUEUE 0x0d        /* Requeue command (no immediate retry) also
                                 * without decrementing the retry count    */
#endif
#ifndef DID_TRANSPORT_DISRUPTED
#define DID_TRANSPORT_DISRUPTED 0xe
#endif
#ifndef DID_TRANSPORT_FAILFAST
#define DID_TRANSPORT_FAILFAST 0xf
#endif
#ifndef DID_TARGET_FAILURE
#define DID_TARGET_FAILURE 0x10
#endif
#ifndef DID_NEXUS_FAILURE
#define DID_NEXUS_FAILURE 0x11
#endif

/* These defines are to isolate applications from kernel define changes */
#define SG_LIB_DID_OK           DID_OK
#define SG_LIB_DID_NO_CONNECT   DID_NO_CONNECT
#define SG_LIB_DID_BUS_BUSY     DID_BUS_BUSY
#define SG_LIB_DID_TIME_OUT     DID_TIME_OUT
#define SG_LIB_DID_BAD_TARGET   DID_BAD_TARGET
#define SG_LIB_DID_ABORT        DID_ABORT
#define SG_LIB_DID_PARITY       DID_PARITY
#define SG_LIB_DID_ERROR        DID_ERROR
#define SG_LIB_DID_RESET        DID_RESET
#define SG_LIB_DID_BAD_INTR     DID_BAD_INTR
#define SG_LIB_DID_PASSTHROUGH  DID_PASSTHROUGH
#define SG_LIB_DID_SOFT_ERROR   DID_SOFT_ERROR
#define SG_LIB_DID_IMM_RETRY    DID_IMM_RETRY
#define SG_LIB_DID_REQUEUE      DID_REQUEUE
#define SG_LIB_TRANSPORT_DISRUPTED      DID_TRANSPORT_DISRUPTED
#define SG_LIB_DID_TRANSPORT_FAILFAST   DID_TRANSPORT_FAILFAST
#define SG_LIB_DID_TARGET_FAILURE       DID_TARGET_FAILURE
#define SG_LIB_DID_NEXUS_FAILURE        DID_NEXUS_FAILURE

/* DRIVER_* are Linux SCSI result (a 32 bit variable) bits 24:27 */
#ifndef DRIVER_OK
#define DRIVER_OK 0x00
#endif
#ifndef DRIVER_BUSY
#define DRIVER_BUSY 0x01
#define DRIVER_SOFT 0x02
#define DRIVER_MEDIA 0x03
#define DRIVER_ERROR 0x04
#define DRIVER_INVALID 0x05
#define DRIVER_TIMEOUT 0x06
#define DRIVER_HARD 0x07
#define DRIVER_SENSE 0x08       /* Sense_buffer has been set */

/* SUGGEST_* are Linux SCSI result (a 32 bit variable) bits 28:31 */
/* N.B. the SUGGEST_* codes are no longer used in Linux and are only kept
 * to stop compilation breakages.
 * Following "suggests" are "or-ed" with one of previous 8 entries */
#define SUGGEST_RETRY 0x10
#define SUGGEST_ABORT 0x20
#define SUGGEST_REMAP 0x30
#define SUGGEST_DIE 0x40
#define SUGGEST_SENSE 0x80
#define SUGGEST_IS_OK 0xff
#endif

#ifndef DRIVER_MASK
#define DRIVER_MASK 0x0f
#endif
#ifndef SUGGEST_MASK
#define SUGGEST_MASK 0xf0
#endif

/* These defines are to isolate applications from kernel define changes */
#define SG_LIB_DRIVER_OK        DRIVER_OK
#define SG_LIB_DRIVER_BUSY      DRIVER_BUSY
#define SG_LIB_DRIVER_SOFT      DRIVER_SOFT
#define SG_LIB_DRIVER_MEDIA     DRIVER_MEDIA
#define SG_LIB_DRIVER_ERROR     DRIVER_ERROR
#define SG_LIB_DRIVER_INVALID   DRIVER_INVALID
#define SG_LIB_DRIVER_TIMEOUT   DRIVER_TIMEOUT
#define SG_LIB_DRIVER_HARD      DRIVER_HARD
#define SG_LIB_DRIVER_SENSE     DRIVER_SENSE


/* N.B. the SUGGEST_* codes are no longer used in Linux and are only kept
 * to stop compilation breakages. */
#define SG_LIB_SUGGEST_RETRY    SUGGEST_RETRY
#define SG_LIB_SUGGEST_ABORT    SUGGEST_ABORT
#define SG_LIB_SUGGEST_REMAP    SUGGEST_REMAP
#define SG_LIB_SUGGEST_DIE      SUGGEST_DIE
#define SG_LIB_SUGGEST_SENSE    SUGGEST_SENSE
#define SG_LIB_SUGGEST_IS_OK    SUGGEST_IS_OK
#define SG_LIB_DRIVER_MASK      DRIVER_MASK
#define SG_LIB_SUGGEST_MASK     SUGGEST_MASK

void sg_print_masked_status(int masked_status);
void sg_print_host_status(int host_status);
void sg_print_driver_status(int driver_status);

/* sg_chk_n_print() returns 1 quietly if there are no errors/warnings
 * else it prints errors/warnings (prefixed by 'leadin') to
 * 'sg_warnings_fd' and returns 0. raw_sinfo indicates whether the
 * raw sense buffer (in ASCII hex) should be printed. */
int sg_chk_n_print(const char * leadin, int masked_status, int host_status,
                   int driver_status, const uint8_t * sense_buffer,
                   int sb_len, bool raw_sinfo);

/* The following function declaration is for the sg version 3 driver. */
struct sg_io_hdr;

/* sg_chk_n_print3() returns 1 quietly if there are no errors/warnings;
 * else it prints errors/warnings (prefixed by 'leadin') to
 * 'sg_warnings_fd' and returns 0. For sg_io_v4 interface use
 * sg_linux_sense_print() instead. */
int sg_chk_n_print3(const char * leadin, struct sg_io_hdr * hp,
                    bool raw_sinfo);

/* Returns 1 if no errors found and thus nothing printed; otherwise
 * prints error/warning (prefix by 'leadin') to stderr (pr2ws) and
 * returns 0. */
int sg_linux_sense_print(const char * leadin, int scsi_status,
                         int host_status, int driver_status,
                         const uint8_t * sense_buffer, int sb_len,
                         bool raw_sinfo);

/* Calls sg_scsi_normalize_sense() after obtaining the sense buffer and
 * its length from the struct sg_io_hdr pointer. If these cannot be
 * obtained, false is returned. For sg_io_v4 interface use
 * sg_scsi_normalize_sense() function instead [see sg_lib.h].  */
bool sg_normalize_sense(const struct sg_io_hdr * hp,
                        struct sg_scsi_sense_hdr * sshp);

/* Returns SG_LIB_CAT_* value. */
int sg_err_category(int masked_status, int host_status, int driver_status,
                    const uint8_t * sense_buffer, int sb_len);

/* Returns SG_LIB_CAT_* value. */
int sg_err_category_new(int scsi_status, int host_status, int driver_status,
                        const uint8_t * sense_buffer, int sb_len);

/* The following function declaration is for the sg version 3 driver. for
 * sg_io_v4 interface use sg_err_category_new() function instead */
int sg_err_category3(struct sg_io_hdr * hp);


/* Note about SCSI status codes found in older versions of Linux.
 * Linux has traditionally used a 1 bit right shifted and masked
 * version of SCSI standard status codes. Now CHECK_CONDITION
 * and friends (in <scsi/scsi.h>) are deprecated. */

#ifdef __cplusplus
}
#endif

#endif
