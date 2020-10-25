/*
 * Copyright (c) 2008-2019 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_cmds_basic.h"
#include "sg_cmds_mmc.h"
#include "sg_pt.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"


#define SENSE_BUFF_LEN 64       /* Arbitrary, could be larger */

#define DEF_PT_TIMEOUT 60       /* 60 seconds */

#define GET_CONFIG_CMD 0x46
#define GET_CONFIG_CMD_LEN 10
#define GET_PERFORMANCE_CMD 0xac
#define GET_PERFORMANCE_CMD_LEN 12
#define SET_CD_SPEED_CMD 0xbb
#define SET_CD_SPEED_CMDLEN 12
#define SET_STREAMING_CMD 0xb6
#define SET_STREAMING_CMDLEN 12


static struct sg_pt_base *
create_pt_obj(const char * cname)
{
    struct sg_pt_base * ptvp = construct_scsi_pt_obj();
    if (NULL == ptvp)
        pr2ws("%s: out of memory\n", cname);
    return ptvp;
}

/* Invokes a SCSI SET CD SPEED command (MMC).
 * Return of 0 -> success, SG_LIB_CAT_INVALID_OP -> command not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int
sg_ll_set_cd_speed(int sg_fd, int rot_control, int drv_read_speed,
                   int drv_write_speed, bool noisy, int verbose)
{
    static const char * const cdb_s = "set cd speed";
    int res, ret, k, sense_cat;
    uint8_t scsCmdBlk[SET_CD_SPEED_CMDLEN] = {SET_CD_SPEED_CMD, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    scsCmdBlk[1] |= (rot_control & 0x3);
    sg_put_unaligned_be16((uint16_t)drv_read_speed, scsCmdBlk + 2);
    sg_put_unaligned_be16((uint16_t)drv_write_speed, scsCmdBlk + 4);

    if (verbose) {
        pr2ws("    %s cdb: ", cdb_s);
        for (k = 0; k < SET_CD_SPEED_CMDLEN; ++k)
            pr2ws("%02x ", scsCmdBlk[k]);
        pr2ws("\n");
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, scsCmdBlk, sizeof(scsCmdBlk));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_NOT_READY:
        case SG_LIB_CAT_UNIT_ATTENTION:
        case SG_LIB_CAT_INVALID_OP:
        case SG_LIB_CAT_ILLEGAL_REQ:
        case SG_LIB_CAT_ABORTED_COMMAND:
            ret = sense_cat;
            break;
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = -1;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI GET CONFIGURATION command (MMC-3,4,5).
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP if command not
 * supported, SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int
sg_ll_get_config(int sg_fd, int rt, int starting, void * resp,
                 int mx_resp_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "get configuration";
    int res, k, ret, sense_cat;
    uint8_t gcCmdBlk[GET_CONFIG_CMD_LEN] = {GET_CONFIG_CMD, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if ((rt < 0) || (rt > 3)) {
        pr2ws("Bad rt value: %d\n", rt);
        return -1;
    }
    gcCmdBlk[1] = (rt & 0x3);
    if ((starting < 0) || (starting > 0xffff)) {
        pr2ws("Bad starting field number: 0x%x\n", starting);
        return -1;
    }
    sg_put_unaligned_be16((uint16_t)starting, gcCmdBlk + 2);
    if ((mx_resp_len < 0) || (mx_resp_len > 0xffff)) {
        pr2ws("Bad mx_resp_len: 0x%x\n", starting);
        return -1;
    }
    sg_put_unaligned_be16((uint16_t)mx_resp_len, gcCmdBlk + 7);

    if (verbose) {
        pr2ws("    %s cdb: ", cdb_s);
        for (k = 0; k < GET_CONFIG_CMD_LEN; ++k)
            pr2ws("%02x ", gcCmdBlk[k]);
        pr2ws("\n");
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, gcCmdBlk, sizeof(gcCmdBlk));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_INVALID_OP:
        case SG_LIB_CAT_ILLEGAL_REQ:
        case SG_LIB_CAT_UNIT_ATTENTION:
        case SG_LIB_CAT_ABORTED_COMMAND:
            ret = sense_cat;
            break;
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = -1;
            break;
        }
    } else {
        if ((verbose > 2) && (ret > 3)) {
            uint8_t * bp;
            int len;

            bp = (uint8_t *)resp;
            len = sg_get_unaligned_be32(bp + 0);
            if (len < 0)
                len = 0;
            len = (ret < len) ? ret : len;
            pr2ws("    %s: response:\n", cdb_s);
            if (3 == verbose) {
                pr2ws("%s:\n", (len > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (len > 256 ? 256 : len),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, len, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI GET PERFORMANCE command (MMC-3...6).
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP if command not
 * supported, SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int
sg_ll_get_performance(int sg_fd, int data_type, unsigned int starting_lba,
                      int max_num_desc, int ttype, void * resp,
                      int mx_resp_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "get performance";
    int res, k, ret, sense_cat;
    uint8_t gpCmdBlk[GET_PERFORMANCE_CMD_LEN] = {GET_PERFORMANCE_CMD, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if ((data_type < 0) || (data_type > 0x1f)) {
        pr2ws("Bad data_type value: %d\n", data_type);
        return -1;
    }
    gpCmdBlk[1] = (data_type & 0x1f);
    sg_put_unaligned_be32((uint32_t)starting_lba, gpCmdBlk + 2);
    if ((max_num_desc < 0) || (max_num_desc > 0xffff)) {
        pr2ws("Bad max_num_desc: 0x%x\n", max_num_desc);
        return -1;
    }
    sg_put_unaligned_be16((uint16_t)max_num_desc, gpCmdBlk + 8);
    if ((ttype < 0) || (ttype > 0xff)) {
        pr2ws("Bad type: 0x%x\n", ttype);
        return -1;
    }
    gpCmdBlk[10] = (uint8_t)ttype;

    if (verbose) {
        pr2ws("    %s cdb: ", cdb_s);
        for (k = 0; k < GET_PERFORMANCE_CMD_LEN; ++k)
            pr2ws("%02x ", gpCmdBlk[k]);
        pr2ws("\n");
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, gpCmdBlk, sizeof(gpCmdBlk));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_INVALID_OP:
        case SG_LIB_CAT_ILLEGAL_REQ:
        case SG_LIB_CAT_UNIT_ATTENTION:
        case SG_LIB_CAT_ABORTED_COMMAND:
            ret = sense_cat;
            break;
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = -1;
            break;
        }
    } else {
        if ((verbose > 2) && (ret > 3)) {
            uint8_t * bp;
            int len;

            bp = (uint8_t *)resp;
            len = sg_get_unaligned_be32(bp + 0);
            if (len < 0)
                len = 0;
            len = (ret < len) ? ret : len;
            pr2ws("    %s: response", cdb_s);
            if (3 == verbose) {
                pr2ws("%s:\n", (len > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (len > 256 ? 256 : len),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, len, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI SET STREAMING command (MMC). Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Set Streaming not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_NOT_READY -> device not ready,
 * -1 -> other failure */
int
sg_ll_set_streaming(int sg_fd, int type, void * paramp, int param_len,
                    bool noisy, int verbose)
{
    static const char * const cdb_s = "set streaming";
    int k, res, ret, sense_cat;
    uint8_t ssCmdBlk[SET_STREAMING_CMDLEN] =
                 {SET_STREAMING_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    ssCmdBlk[8] = type;
    sg_put_unaligned_be16((uint16_t)param_len, ssCmdBlk + 9);
    if (verbose) {
        pr2ws("    %s cdb: ", cdb_s);
        for (k = 0; k < SET_STREAMING_CMDLEN; ++k)
            pr2ws("%02x ", ssCmdBlk[k]);
        pr2ws("\n");
        if ((verbose > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, ssCmdBlk, sizeof(ssCmdBlk));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_NOT_READY:
        case SG_LIB_CAT_INVALID_OP:
        case SG_LIB_CAT_ILLEGAL_REQ:
        case SG_LIB_CAT_UNIT_ATTENTION:
        case SG_LIB_CAT_ABORTED_COMMAND:
            ret = sense_cat;
            break;
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = -1;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}
