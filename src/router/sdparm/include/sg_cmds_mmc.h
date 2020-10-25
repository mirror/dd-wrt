#ifndef SG_CMDS_MMC_H
#define SG_CMDS_MMC_H

/*
 * Copyright (c) 2008-2017 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifdef __cplusplus
extern "C" {
#endif


/* Invokes a SCSI GET CONFIGURATION command (MMC-3...6).
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP if command not
 * supported, SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int sg_ll_get_config(int sg_fd, int rt, int starting, void * resp,
                     int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI GET PERFORMANCE command (MMC-3...6).
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP if command not
 * supported, SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int sg_ll_get_performance(int sg_fd, int data_type, unsigned int starting_lba,
                          int max_num_desc, int type, void * resp,
                          int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI SET CD SPEED command (MMC).
 * Return of 0 -> success, SG_LIB_CAT_INVALID_OP -> command not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_set_cd_speed(int sg_fd, int rot_control, int drv_read_speed,
                       int drv_write_speed, bool noisy, int verbose);

/* Invokes a SCSI SET STREAMING command (MMC). Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Set Streaming not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_NOT_READY -> device not ready,
 * -1 -> other failure */
int sg_ll_set_streaming(int sg_fd, int type, void * paramp, int param_len,
                        bool noisy, int verbose);


#ifdef __cplusplus
}
#endif

#endif
