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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_lib_data.h"
#include "sg_cmds_basic.h"
#include "sg_cmds_extra.h"
#include "sg_pt.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"


#define SENSE_BUFF_LEN 64       /* Arbitrary, could be larger */

#define DEF_PT_TIMEOUT 60       /* 60 seconds */
#define LONG_PT_TIMEOUT 7200    /* 7,200 seconds == 120 minutes */

#define SERVICE_ACTION_IN_16_CMD 0x9e
#define SERVICE_ACTION_IN_16_CMDLEN 16
#define SERVICE_ACTION_OUT_16_CMD 0x9f
#define SERVICE_ACTION_OUT_16_CMDLEN 16
#define MAINTENANCE_IN_CMD 0xa3
#define MAINTENANCE_IN_CMDLEN 12
#define MAINTENANCE_OUT_CMD 0xa4
#define MAINTENANCE_OUT_CMDLEN 12

#define ATA_PT_12_CMD 0xa1
#define ATA_PT_12_CMDLEN 12
#define ATA_PT_16_CMD 0x85
#define ATA_PT_16_CMDLEN 16
#define ATA_PT_32_SA 0x1ff0
#define ATA_PT_32_CMDLEN 32
#define FORMAT_UNIT_CMD 0x4
#define FORMAT_UNIT_CMDLEN 6
#define PERSISTENT_RESERVE_IN_CMD 0x5e
#define PERSISTENT_RESERVE_IN_CMDLEN 10
#define PERSISTENT_RESERVE_OUT_CMD 0x5f
#define PERSISTENT_RESERVE_OUT_CMDLEN 10
#define READ_BLOCK_LIMITS_CMD 0x5
#define READ_BLOCK_LIMITS_CMDLEN 6
#define READ_BUFFER_CMD 0x3c
#define READ_BUFFER_CMDLEN 10
#define READ_DEFECT10_CMD     0x37
#define READ_DEFECT10_CMDLEN    10
#define REASSIGN_BLKS_CMD     0x7
#define REASSIGN_BLKS_CMDLEN  6
#define RECEIVE_DIAGNOSTICS_CMD   0x1c
#define RECEIVE_DIAGNOSTICS_CMDLEN  6
#define THIRD_PARTY_COPY_OUT_CMD 0x83   /* was EXTENDED_COPY_CMD */
#define THIRD_PARTY_COPY_OUT_CMDLEN 16
#define THIRD_PARTY_COPY_IN_CMD 0x84     /* was RECEIVE_COPY_RESULTS_CMD */
#define THIRD_PARTY_COPY_IN_CMDLEN 16
#define SEND_DIAGNOSTIC_CMD   0x1d
#define SEND_DIAGNOSTIC_CMDLEN  6
#define SERVICE_ACTION_IN_12_CMD 0xab
#define SERVICE_ACTION_IN_12_CMDLEN 12
#define READ_LONG10_CMD 0x3e
#define READ_LONG10_CMDLEN 10
#define UNMAP_CMD 0x42
#define UNMAP_CMDLEN 10
#define VERIFY10_CMD 0x2f
#define VERIFY10_CMDLEN 10
#define VERIFY16_CMD 0x8f
#define VERIFY16_CMDLEN 16
#define WRITE_LONG10_CMD 0x3f
#define WRITE_LONG10_CMDLEN 10
#define WRITE_BUFFER_CMD 0x3b
#define WRITE_BUFFER_CMDLEN 10
#define PRE_FETCH10_CMD 0x34
#define PRE_FETCH10_CMDLEN 10
#define PRE_FETCH16_CMD 0x90
#define PRE_FETCH16_CMDLEN 16
#define SEEK10_CMD 0x2b
#define SEEK10_CMDLEN 10

#define GET_LBA_STATUS16_SA 0x12
#define GET_LBA_STATUS32_SA 0x12
#define READ_LONG_16_SA 0x11
#define READ_MEDIA_SERIAL_NUM_SA 0x1
#define REPORT_IDENTIFYING_INFORMATION_SA 0x5
#define REPORT_TGT_PRT_GRP_SA 0xa
#define SET_IDENTIFYING_INFORMATION_SA 0x6
#define SET_TGT_PRT_GRP_SA 0xa
#define WRITE_LONG_16_SA 0x11
#define REPORT_REFERRALS_SA 0x13
#define EXTENDED_COPY_LID1_SA 0x0


static struct sg_pt_base *
create_pt_obj(const char * cname)
{
    struct sg_pt_base * ptvp = construct_scsi_pt_obj();
    if (NULL == ptvp)
        pr2ws("%s: out of memory\n", cname);
    return ptvp;
}


/* Invokes a SCSI GET LBA STATUS(16) command (SBC). Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_get_lba_status16(int sg_fd, uint64_t start_llba, uint8_t rt,
                      void * resp, int alloc_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Get LBA status(16)";
    int res, s_cat, ret;
    uint8_t getLbaStatCmd[SERVICE_ACTION_IN_16_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(getLbaStatCmd, 0, sizeof(getLbaStatCmd));
    getLbaStatCmd[0] = SERVICE_ACTION_IN_16_CMD;
    getLbaStatCmd[1] = GET_LBA_STATUS16_SA;

    sg_put_unaligned_be64(start_llba, getLbaStatCmd + 2);
    sg_put_unaligned_be32((uint32_t)alloc_len, getLbaStatCmd + 10);
    getLbaStatCmd[14] = rt;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(getLbaStatCmd, SERVICE_ACTION_IN_16_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, getLbaStatCmd, sizeof(getLbaStatCmd));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, alloc_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response\n", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

int
sg_ll_get_lba_status(int sg_fd, uint64_t start_llba, void * resp,
                     int alloc_len, bool noisy, int vb)
{
    return sg_ll_get_lba_status16(sg_fd, start_llba, /* rt = */ 0x0, resp,
                                  alloc_len, noisy, vb);
}

#define GLS32_CMD_LEN 32

int
sg_ll_get_lba_status32(int sg_fd, uint64_t start_llba, uint32_t scan_len,
                       uint32_t element_id, uint8_t rt,
                       void * resp, int alloc_len, bool noisy,
                       int vb)
{
    static const char * const cdb_s = "Get LBA status(32)";
    int res, s_cat, ret;
    uint8_t gls32_cmd[GLS32_CMD_LEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(gls32_cmd, 0, sizeof(gls32_cmd));
    gls32_cmd[0] = SG_VARIABLE_LENGTH_CMD;
    gls32_cmd[7] = GLS32_CMD_LEN - 8;
    sg_put_unaligned_be16((uint16_t)GET_LBA_STATUS32_SA, gls32_cmd + 8);
    gls32_cmd[10] = rt;
    sg_put_unaligned_be64(start_llba, gls32_cmd + 12);
    sg_put_unaligned_be32(scan_len, gls32_cmd + 20);
    sg_put_unaligned_be32(element_id, gls32_cmd + 24);
    sg_put_unaligned_be32((uint32_t)alloc_len, gls32_cmd + 28);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(gls32_cmd, GLS32_CMD_LEN, false, sizeof(b),
                                 b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, gls32_cmd, sizeof(gls32_cmd));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, alloc_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response\n", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

int
sg_ll_report_tgt_prt_grp(int sg_fd, void * resp, int mx_resp_len,
                         bool noisy, int vb)
{
    return sg_ll_report_tgt_prt_grp2(sg_fd, resp, mx_resp_len, false, noisy,
                                     vb);
}

/* Invokes a SCSI REPORT TARGET PORT GROUPS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_report_tgt_prt_grp2(int sg_fd, void * resp, int mx_resp_len,
                          bool extended, bool noisy, int vb)
{
    static const char * const cdb_s = "Report target port groups";
    int res, ret, s_cat;
    uint8_t rtpg_cdb[MAINTENANCE_IN_CMDLEN] =
                         {MAINTENANCE_IN_CMD, REPORT_TGT_PRT_GRP_SA,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (extended)
        rtpg_cdb[1] |= 0x20;
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rtpg_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rtpg_cdb, MAINTENANCE_IN_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, rtpg_cdb, sizeof(rtpg_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI SET TARGET PORT GROUPS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_set_tgt_prt_grp(int sg_fd, void * paramp, int param_len, bool noisy,
                      int vb)
{
    static const char * const cdb_s = "Set target port groups";
    int res, ret, s_cat;
    uint8_t stpg_cdb[MAINTENANCE_OUT_CMDLEN] =
                         {MAINTENANCE_OUT_CMD, SET_TGT_PRT_GRP_SA,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    sg_put_unaligned_be32((uint32_t)param_len, stpg_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(stpg_cdb, MAINTENANCE_OUT_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, stpg_cdb, sizeof(stpg_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI REPORT REFERRALS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_report_referrals(int sg_fd, uint64_t start_llba, bool one_seg,
                       void * resp, int mx_resp_len, bool noisy,
                       int vb)
{
    static const char * const cdb_s = "Report referrals";
    int res, ret, s_cat;
    uint8_t repRef_cdb[SERVICE_ACTION_IN_16_CMDLEN] =
                         {SERVICE_ACTION_IN_16_CMD, REPORT_REFERRALS_SA,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    sg_put_unaligned_be64(start_llba, repRef_cdb + 2);
    sg_put_unaligned_be32((uint32_t)mx_resp_len, repRef_cdb + 10);
    if (one_seg)
        repRef_cdb[14] = 0x1;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(repRef_cdb, SERVICE_ACTION_IN_16_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, repRef_cdb, sizeof(repRef_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI SEND DIAGNOSTIC command. Foreground, extended self tests can
 * take a long time, if so set long_duration flag in which case the timeout
 * is set to 7200 seconds; if the value of long_duration is > 7200 then that
 * value is taken as the timeout value in seconds. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_send_diag_com(struct sg_pt_base * ptvp, int sg_fd, int st_code,
                    bool pf_bit, bool st_bit, bool devofl_bit,
                    bool unitofl_bit, int long_duration, void * paramp,
                    int param_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Send diagnostic";
    bool ptvp_given = false;
    bool local_sense = true;
    bool local_cdb = true;
    int res, ret, s_cat, tmout;
    uint8_t senddiag_cdb[SEND_DIAGNOSTIC_CMDLEN] =
        {SEND_DIAGNOSTIC_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    senddiag_cdb[1] = (uint8_t)(st_code << 5);
    if (pf_bit)
        senddiag_cdb[1] |= 0x10;
    if (st_bit)
        senddiag_cdb[1] |= 0x4;
    if (devofl_bit)
        senddiag_cdb[1] |= 0x2;
    if (unitofl_bit)
        senddiag_cdb[1] |= 0x1;
    sg_put_unaligned_be16((uint16_t)param_len, senddiag_cdb + 3);
    if (long_duration > LONG_PT_TIMEOUT)
        tmout = long_duration;
    else
        tmout = long_duration ? LONG_PT_TIMEOUT : DEF_PT_TIMEOUT;

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(senddiag_cdb, SEND_DIAGNOSTIC_CMDLEN,
                                 false, sizeof(b), b));
        if (vb > 1) {
            if (paramp && param_len) {
                pr2ws("    %s parameter list:\n", cdb_s);
                hex2stderr((const uint8_t *)paramp, param_len, -1);
            }
            pr2ws("    %s timeout: %d seconds\n", cdb_s, tmout);
        }
    }
    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false; /* N.B. Ignores locally built cdb */
        else
            set_scsi_pt_cdb(ptvp, senddiag_cdb, sizeof(senddiag_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, vb);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, senddiag_cdb, sizeof(senddiag_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, -1, tmout, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    if (ptvp_given) {
        if (local_sense)    /* stop caller trying to access local sense */
            set_scsi_pt_sense(ptvp, NULL, 0);
        if (local_cdb)
            set_scsi_pt_cdb(ptvp, NULL, 0);
    } else {
        if (ptvp)
            destruct_scsi_pt_obj(ptvp);
    }
    return ret;
}

int
sg_ll_send_diag_pt(struct sg_pt_base * ptvp, int st_code, bool pf_bit,
                   bool st_bit, bool devofl_bit, bool unitofl_bit,
                   int long_duration, void * paramp, int param_len,
                   bool noisy, int vb)
{
    return sg_ll_send_diag_com(ptvp, -1, st_code, pf_bit, st_bit, devofl_bit,
                               unitofl_bit, long_duration, paramp,
                               param_len, noisy, vb);
}

int
sg_ll_send_diag(int sg_fd, int st_code, bool pf_bit, bool st_bit,
                bool devofl_bit, bool unitofl_bit, int long_duration,
                void * paramp, int param_len, bool noisy, int vb)
{
    return sg_ll_send_diag_com(NULL, sg_fd, st_code, pf_bit, st_bit,
                               devofl_bit, unitofl_bit, long_duration, paramp,
                               param_len, noisy, vb);
}

/* Invokes a SCSI RECEIVE DIAGNOSTIC RESULTS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
static int
sg_ll_receive_diag_com(struct sg_pt_base * ptvp, int sg_fd, bool pcv,
                       int pg_code, void * resp, int mx_resp_len,
                       int timeout_secs, int * residp, bool noisy, int vb)
{
    bool ptvp_given = false;
    bool local_sense = true;
    bool local_cdb = true;
    int resid = 0;
    int res, ret, s_cat;
    static const char * const cdb_s = "Receive diagnostic results";
    uint8_t rcvdiag_cdb[RECEIVE_DIAGNOSTICS_CMDLEN] =
        {RECEIVE_DIAGNOSTICS_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    if (pcv)
        rcvdiag_cdb[1] = 0x1;
    rcvdiag_cdb[2] = (uint8_t)(pg_code);
    sg_put_unaligned_be16((uint16_t)mx_resp_len, rcvdiag_cdb + 3);

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rcvdiag_cdb, RECEIVE_DIAGNOSTICS_CMDLEN,
                                 false, sizeof(b), b));
    }
    if (timeout_secs <= 0)
        timeout_secs = DEF_PT_TIMEOUT;

    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false; /* N.B. Ignores locally built cdb */
        else
            set_scsi_pt_cdb(ptvp, rcvdiag_cdb, sizeof(rcvdiag_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, vb);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, rcvdiag_cdb, sizeof(rcvdiag_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, -1, timeout_secs, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    resid = get_scsi_pt_resid(ptvp);
    if (residp)
        *residp = resid;
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                            -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }

    if (ptvp_given) {
        if (local_sense)    /* stop caller trying to access local sense */
            set_scsi_pt_sense(ptvp, NULL, 0);
        if (local_cdb)
            set_scsi_pt_cdb(ptvp, NULL, 0);
    } else {
        if (ptvp)
            destruct_scsi_pt_obj(ptvp);
    }
    return ret;
}

int
sg_ll_receive_diag_pt(struct sg_pt_base * ptvp, bool pcv, int pg_code,
                      void * resp, int mx_resp_len, int timeout_secs,
                      int * residp, bool noisy, int vb)
{
    return sg_ll_receive_diag_com(ptvp, -1, pcv, pg_code, resp, mx_resp_len,
                                  timeout_secs, residp, noisy, vb);
}

/* Invokes a SCSI RECEIVE DIAGNOSTIC RESULTS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_receive_diag(int sg_fd, bool pcv, int pg_code, void * resp,
                   int mx_resp_len, bool noisy, int vb)
{
    return sg_ll_receive_diag_com(NULL, sg_fd, pcv, pg_code, resp,
                                  mx_resp_len, 0, NULL, noisy, vb);
}

int
sg_ll_receive_diag_v2(int sg_fd, bool pcv, int pg_code, void * resp,
                      int mx_resp_len, int timeout_secs, int * residp,
                      bool noisy, int vb)
{
    return sg_ll_receive_diag_com(NULL, sg_fd, pcv, pg_code, resp,
                                  mx_resp_len, timeout_secs, residp, noisy,
                                  vb);
}

/* Invokes a SCSI READ DEFECT DATA (10) command (SBC). Return of 0 -> success
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_defect10(int sg_fd, bool req_plist, bool req_glist, int dl_format,
                    void * resp, int mx_resp_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Read defect(10)";
    int res, ret, s_cat;
    uint8_t rdef_cdb[READ_DEFECT10_CMDLEN] =
        {READ_DEFECT10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    rdef_cdb[2] = (dl_format & 0x7);
    if (req_plist)
        rdef_cdb[2] |= 0x10;
    if (req_glist)
        rdef_cdb[2] |= 0x8;
    sg_put_unaligned_be16((uint16_t)mx_resp_len, rdef_cdb + 7);
    if (mx_resp_len > 0xffff) {
        pr2ws("mx_resp_len too big\n");
        return -1;
    }
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rdef_cdb, READ_DEFECT10_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, rdef_cdb, sizeof(rdef_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response\n", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ MEDIA SERIAL NUMBER command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_media_serial_num(int sg_fd, void * resp, int mx_resp_len,
                            bool noisy, int vb)
{
    static const char * const cdb_s = "Read media serial number";
    int res, ret, s_cat;
    uint8_t rmsn_cdb[SERVICE_ACTION_IN_12_CMDLEN] =
                         {SERVICE_ACTION_IN_12_CMD, READ_MEDIA_SERIAL_NUM_SA,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    sg_put_unaligned_be32((uint32_t)mx_resp_len, rmsn_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rmsn_cdb, SERVICE_ACTION_IN_12_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, rmsn_cdb, sizeof(rmsn_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI REPORT IDENTIFYING INFORMATION command. This command was
 * called REPORT DEVICE IDENTIFIER prior to spc4r07. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_report_id_info(int sg_fd, int itype, void * resp, int max_resp_len,
                     bool noisy, int vb)
{
    static const char * const cdb_s = "Report identifying information";
    int res, ret, s_cat;
    uint8_t rii_cdb[MAINTENANCE_IN_CMDLEN] = {MAINTENANCE_IN_CMD,
                        REPORT_IDENTIFYING_INFORMATION_SA,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    sg_put_unaligned_be32((uint32_t)max_resp_len, rii_cdb + 6);
    rii_cdb[10] |= (itype << 1) & 0xfe;

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rii_cdb, MAINTENANCE_IN_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, rii_cdb, sizeof(rii_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, max_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI SET IDENTIFYING INFORMATION command. This command was
 * called SET DEVICE IDENTIFIER prior to spc4r07. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_set_id_info(int sg_fd, int itype, void * paramp, int param_len,
                  bool noisy, int vb)
{
    static const char * const cdb_s = "Set identifying information";
    int res, ret, s_cat;
    uint8_t sii_cdb[MAINTENANCE_OUT_CMDLEN] = {MAINTENANCE_OUT_CMD,
                         SET_IDENTIFYING_INFORMATION_SA,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    sg_put_unaligned_be32((uint32_t)param_len, sii_cdb + 6);
    sii_cdb[10] |= (itype << 1) & 0xfe;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(sii_cdb, MAINTENANCE_OUT_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, sii_cdb, sizeof(sii_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a FORMAT UNIT (SBC-3) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_format_unit(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                  bool cmplst, int dlist_format, int timeout_secs,
                  void * paramp, int param_len, bool noisy, int vb)
{
    return sg_ll_format_unit_v2(sg_fd, fmtpinfo, longlist, fmtdata, cmplst,
                                dlist_format, 0, timeout_secs, paramp,
                                param_len, noisy, vb);
}

/* Invokes a FORMAT UNIT (SBC-3) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_format_unit2(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                   bool cmplst, int dlist_format, int ffmt, int timeout_secs,
                   void * paramp, int param_len, bool noisy, int vb)
{
    return sg_ll_format_unit_v2(sg_fd, fmtpinfo, longlist, fmtdata, cmplst,
                                dlist_format, ffmt, timeout_secs, paramp,
                                param_len, noisy, vb);
}

/* Invokes a FORMAT UNIT (SBC-4) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors.
 * FFMT field added in sbc4r10 [20160121] */
int
sg_ll_format_unit_v2(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                     bool cmplst, int dlist_format, int ffmt,
                     int timeout_secs, void * paramp, int param_len,
                     bool noisy, int vb)
{
    static const char * const cdb_s = "Format unit";
    int res, ret, s_cat, tmout;
    uint8_t fu_cdb[FORMAT_UNIT_CMDLEN] =
                {FORMAT_UNIT_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (fmtpinfo)
        fu_cdb[1] |= (fmtpinfo << 6);
    if (longlist)
        fu_cdb[1] |= 0x20;
    if (fmtdata)
        fu_cdb[1] |= 0x10;
    if (cmplst)
        fu_cdb[1] |= 0x8;
    if (dlist_format)
        fu_cdb[1] |= (dlist_format & 0x7);
    if (ffmt)
        fu_cdb[4] |= (ffmt & 0x3);
    tmout = (timeout_secs > 0) ? timeout_secs : DEF_PT_TIMEOUT;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(fu_cdb, 6, false, sizeof(b), b));
        if (vb > 1) {
            if (param_len > 0) {
                pr2ws("    %s parameter list:\n", cdb_s);
                hex2stderr((const uint8_t *)paramp, param_len, -1);
            }
            pr2ws("    %s timeout: %d seconds\n", cdb_s, tmout);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, fu_cdb, sizeof(fu_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, tmout, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI REASSIGN BLOCKS command.  Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_reassign_blocks(int sg_fd, bool longlba, bool longlist, void * paramp,
                      int param_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Reassign blocks";
    int res, ret, s_cat;
    uint8_t reass_cdb[REASSIGN_BLKS_CMDLEN] =
        {REASSIGN_BLKS_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (longlba)
        reass_cdb[1] = 0x2;
    if (longlist)
        reass_cdb[1] |= 0x1;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(reass_cdb, REASSIGN_BLKS_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (vb > 1) {
        pr2ws("    %s parameter list\n", cdb_s);
        hex2stderr((const uint8_t *)paramp, param_len, -1);
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, reass_cdb, sizeof(reass_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI PERSISTENT RESERVE IN command (SPC). Returns 0
 * when successful, various SG_LIB_CAT_* positive values or
 * -1 -> other errors */
int
sg_ll_persistent_reserve_in(int sg_fd, int rq_servact, void * resp,
                            int mx_resp_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Persistent reservation in";
    int res, ret, s_cat;
    uint8_t prin_cdb[PERSISTENT_RESERVE_IN_CMDLEN] =
                 {PERSISTENT_RESERVE_IN_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (rq_servact > 0)
        prin_cdb[1] = (uint8_t)(rq_servact & 0x1f);
    sg_put_unaligned_be16((uint16_t)mx_resp_len, prin_cdb + 7);

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(prin_cdb, PERSISTENT_RESERVE_IN_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, prin_cdb, sizeof(prin_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI PERSISTENT RESERVE OUT command (SPC). Returns 0
 * when successful, various SG_LIB_CAT_* positive values or
 * -1 -> other errors */
int
sg_ll_persistent_reserve_out(int sg_fd, int rq_servact, int rq_scope,
                             unsigned int rq_type, void * paramp,
                             int param_len, bool noisy, int vb)
{
    static const char * const cdb_s = "Persistent reservation out";
    int res, ret, s_cat;
    uint8_t prout_cdb[PERSISTENT_RESERVE_OUT_CMDLEN] =
                 {PERSISTENT_RESERVE_OUT_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (rq_servact > 0)
        prout_cdb[1] = (uint8_t)(rq_servact & 0x1f);
    prout_cdb[2] = (((rq_scope & 0xf) << 4) | (rq_type & 0xf));
    sg_put_unaligned_be16((uint16_t)param_len, prout_cdb + 7);

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(prout_cdb, PERSISTENT_RESERVE_OUT_CMDLEN,
                                 false, sizeof(b), b));
        if (vb > 1) {
            pr2ws("    %s parameters:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, 0);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, prout_cdb, sizeof(prout_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

static bool
has_blk_ili(uint8_t * sensep, int sb_len)
{
    int resp_code;
    const uint8_t * cup;

    if (sb_len < 8)
        return false;
    resp_code = (0x7f & sensep[0]);
    if (resp_code >= 0x72) { /* descriptor format */
        /* find block command descriptor */
        if ((cup = sg_scsi_sense_desc_find(sensep, sb_len, 0x5)))
            return (cup[3] & 0x20);
    } else /* fixed */
        return (sensep[2] & 0x20);
    return false;
}

/* Invokes a SCSI READ LONG (10) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_long10(int sg_fd, bool pblock, bool correct, unsigned int lba,
                  void * resp, int xfer_len, int * offsetp, bool noisy,
                  int vb)
{
    static const char * const cdb_s = "read long(10)";
    int res, s_cat, ret;
    uint8_t readLong_cdb[READ_LONG10_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(readLong_cdb, 0, READ_LONG10_CMDLEN);
    readLong_cdb[0] = READ_LONG10_CMD;
    if (pblock)
        readLong_cdb[1] |= 0x4;
    if (correct)
        readLong_cdb[1] |= 0x2;

    sg_put_unaligned_be32((uint32_t)lba, readLong_cdb + 2);
    sg_put_unaligned_be16((uint16_t)xfer_len, readLong_cdb + 7);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(readLong_cdb, READ_LONG10_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, readLong_cdb, sizeof(readLong_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, xfer_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_ILLEGAL_REQ:
            {
                bool valid, ili;
                int slen;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                ili = has_blk_ili(sense_b, slen);
                if (valid && ili) {
                    if (offsetp)
                        *offsetp = (int)(int64_t)ull;
                    ret = SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO;
                } else {
                    if (vb > 1)
                        pr2ws("  info field: 0x%" PRIx64 ",  valid: %d, "
                              "ili: %d\n", ull, valid, ili);
                    ret = SG_LIB_CAT_ILLEGAL_REQ;
                }
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ LONG (16) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_long16(int sg_fd, bool pblock, bool correct, uint64_t llba,
                  void * resp, int xfer_len, int * offsetp, bool noisy,
                  int vb)
{
    static const char * const cdb_s = "read long(16)";
    int res, s_cat, ret;
    uint8_t readLong_cdb[SERVICE_ACTION_IN_16_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(readLong_cdb, 0, sizeof(readLong_cdb));
    readLong_cdb[0] = SERVICE_ACTION_IN_16_CMD;
    readLong_cdb[1] = READ_LONG_16_SA;
    if (pblock)
        readLong_cdb[14] |= 0x2;
    if (correct)
        readLong_cdb[14] |= 0x1;

    sg_put_unaligned_be64(llba, readLong_cdb + 2);
    sg_put_unaligned_be16((uint16_t)xfer_len, readLong_cdb + 12);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(readLong_cdb, SERVICE_ACTION_IN_16_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, readLong_cdb, sizeof(readLong_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, xfer_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_ILLEGAL_REQ:
            {
                bool valid, ili;
                int slen;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                ili = has_blk_ili(sense_b, slen);
                if (valid && ili) {
                    if (offsetp)
                        *offsetp = (int)(int64_t)ull;
                    ret = SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO;
                } else {
                    if (vb > 1)
                        pr2ws("  info field: 0x%" PRIx64 ",  valid: %d, "
                              "ili: %d\n", ull, (int)valid, (int)ili);
                    ret = SG_LIB_CAT_ILLEGAL_REQ;
                }
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI WRITE LONG (10) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_write_long10(int sg_fd, bool cor_dis, bool wr_uncor, bool pblock,
                   unsigned int lba, void * data_out, int xfer_len,
                   int * offsetp, bool noisy, int vb)
{
    static const char * const cdb_s = "write long(10)";
    int res, s_cat, ret;
    uint8_t writeLong_cdb[WRITE_LONG10_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(writeLong_cdb, 0, WRITE_LONG10_CMDLEN);
    writeLong_cdb[0] = WRITE_LONG10_CMD;
    if (cor_dis)
        writeLong_cdb[1] |= 0x80;
    if (wr_uncor)
        writeLong_cdb[1] |= 0x40;
    if (pblock)
        writeLong_cdb[1] |= 0x20;

    sg_put_unaligned_be32((uint32_t)lba, writeLong_cdb + 2);
    sg_put_unaligned_be16((uint16_t)xfer_len, writeLong_cdb + 7);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(writeLong_cdb, (int)sizeof(writeLong_cdb),
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, writeLong_cdb, sizeof(writeLong_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)data_out, xfer_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_ILLEGAL_REQ:
            {
                int valid, slen, ili;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                ili = has_blk_ili(sense_b, slen);
                if (valid && ili) {
                    if (offsetp)
                        *offsetp = (int)(int64_t)ull;
                    ret = SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO;
                } else {
                    if (vb > 1)
                        pr2ws("  info field: 0x%" PRIx64 ",  valid: %d, "
                              "ili: %d\n", ull, (int)valid, (int)ili);
                    ret = SG_LIB_CAT_ILLEGAL_REQ;
                }
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI WRITE LONG (16) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_write_long16(int sg_fd, bool cor_dis, bool wr_uncor, bool pblock,
                   uint64_t llba, void * data_out, int xfer_len,
                   int * offsetp, bool noisy, int vb)
{
    static const char * const cdb_s = "write long(16)";
    int res, s_cat, ret;
    uint8_t writeLong_cdb[SERVICE_ACTION_OUT_16_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(writeLong_cdb, 0, sizeof(writeLong_cdb));
    writeLong_cdb[0] = SERVICE_ACTION_OUT_16_CMD;
    writeLong_cdb[1] = WRITE_LONG_16_SA;
    if (cor_dis)
        writeLong_cdb[1] |= 0x80;
    if (wr_uncor)
        writeLong_cdb[1] |= 0x40;
    if (pblock)
        writeLong_cdb[1] |= 0x20;

    sg_put_unaligned_be64(llba, writeLong_cdb + 2);
    sg_put_unaligned_be16((uint16_t)xfer_len, writeLong_cdb + 12);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(writeLong_cdb, SERVICE_ACTION_OUT_16_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, writeLong_cdb, sizeof(writeLong_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)data_out, xfer_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_ILLEGAL_REQ:
            {
                bool valid, ili;
                int slen;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                ili = has_blk_ili(sense_b, slen);
                if (valid && ili) {
                    if (offsetp)
                        *offsetp = (int)(int64_t)ull;
                    ret = SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO;
                } else {
                    if (vb > 1)
                        pr2ws("  info field: 0x%" PRIx64 ",  valid: %d, "
                              "ili: %d\n", ull, (int)valid, (int)ili);
                    ret = SG_LIB_CAT_ILLEGAL_REQ;
                }
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI VERIFY (10) command (SBC and MMC).
 * Note that 'veri_len' is in blocks while 'data_out_len' is in bytes.
 * Returns of 0 -> success, * various SG_LIB_CAT_* positive values or
 * -1 -> other errors */
int
sg_ll_verify10(int sg_fd, int vrprotect, bool dpo, int bytchk,
               unsigned int lba, int veri_len, void * data_out,
               int data_out_len, unsigned int * infop, bool noisy,
               int vb)
{
    static const char * const cdb_s = "verify(10)";
    int k, res, ret, s_cat, slen;
    uint8_t v_cdb[VERIFY10_CMDLEN] =
                {VERIFY10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    /* N.B. BYTCHK field expanded to 2 bits sbc3r34 */
    v_cdb[1] = (((vrprotect & 0x7) << 5) | ((bytchk & 0x3) << 1)) ;
    if (dpo)
        v_cdb[1] |= 0x10;
    sg_put_unaligned_be32((uint32_t)lba, v_cdb + 2);
    sg_put_unaligned_be16((uint16_t)veri_len, v_cdb + 7);
    if (vb > 1) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(v_cdb, VERIFY10_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 3) && bytchk && data_out && (data_out_len > 0)) {
            k = data_out_len > 4104 ? 4104 : data_out_len;
            pr2ws("    data_out buffer%s\n",
                  (data_out_len > 4104 ? ", first 4104 bytes" : ""));
            hex2stderr((const uint8_t *)data_out, k, vb < 5);
        }
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, v_cdb, sizeof(v_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    if (data_out_len > 0)
        set_scsi_pt_data_out(ptvp, (uint8_t *)data_out, data_out_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_MEDIUM_HARD:
            {
                bool valid;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                if (valid) {
                    if (infop)
                        *infop = (unsigned int)ull;
                    ret = SG_LIB_CAT_MEDIUM_HARD_WITH_INFO;
                } else
                    ret = SG_LIB_CAT_MEDIUM_HARD;
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI VERIFY (16) command (SBC and MMC).
 * Note that 'veri_len' is in blocks while 'data_out_len' is in bytes.
 * Returns of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_verify16(int sg_fd, int vrprotect, bool dpo, int bytchk, uint64_t llba,
               int veri_len, int group_num, void * data_out,
               int data_out_len, uint64_t * infop, bool noisy, int vb)
{
    static const char * const cdb_s = "verify(16)";
    int k, res, ret, s_cat, slen;
    uint8_t v_cdb[VERIFY16_CMDLEN] =
                {VERIFY16_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    /* N.B. BYTCHK field expanded to 2 bits sbc3r34 */
    v_cdb[1] = (((vrprotect & 0x7) << 5) | ((bytchk & 0x3) << 1)) ;
    if (dpo)
        v_cdb[1] |= 0x10;
    sg_put_unaligned_be64(llba, v_cdb + 2);
    sg_put_unaligned_be32((uint32_t)veri_len, v_cdb + 10);
    v_cdb[14] = group_num & 0x1f;
    if (vb > 1) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(v_cdb, VERIFY16_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 3) && bytchk && data_out && (data_out_len > 0)) {
            k = data_out_len > 4104 ? 4104 : data_out_len;
            pr2ws("    data_out buffer%s\n",
                  (data_out_len > 4104 ? ", first 4104 bytes" : ""));
            hex2stderr((const uint8_t *)data_out, k, vb < 5);
        }
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return sg_convert_errno(ENOMEM);
    set_scsi_pt_cdb(ptvp, v_cdb, sizeof(v_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    if (data_out_len > 0)
        set_scsi_pt_data_out(ptvp, (uint8_t *)data_out, data_out_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        case SG_LIB_CAT_MEDIUM_HARD:
            {
                bool valid;
                uint64_t ull = 0;

                slen = get_scsi_pt_sense_len(ptvp);
                valid = sg_get_sense_info_fld(sense_b, slen, &ull);
                if (valid) {
                    if (infop)
                        *infop = ull;
                    ret = SG_LIB_CAT_MEDIUM_HARD_WITH_INFO;
                } else
                    ret = SG_LIB_CAT_MEDIUM_HARD;
            }
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a ATA PASS-THROUGH (12, 16 or 32) SCSI command (SAT). This is
 * selected by the cdb_len argument that can take values of 12, 16 or 32
 * only (else -1 is returned). The byte at offset 0 (and bytes 0 to 9
 * inclusive for ATA PT(32)) pointed to be cdbp are ignored and apart from
 * the control byte, the rest is copied into an internal cdb which is then
 * sent to the device. The control byte is byte 11 for ATA PT(12), byte 15
 * for ATA PT(16) and byte 1 for ATA PT(32). If timeout_secs <= 0 then the
 * timeout is set to 60 seconds. For data in or out transfers set dinp or
 * doutp, and dlen to the number of bytes to transfer. If dlen is zero then
 * no data transfer is assumed. If sense buffer obtained then it is written
 * to sensep, else sensep[0] is set to 0x0. If ATA return descriptor is
 * obtained then written to ata_return_dp, else ata_return_dp[0] is set to
 * 0x0. Either sensep or ata_return_dp (or both) may be NULL pointers.
 * Returns SCSI status value (>= 0) or -1 if other error. Users are
 * expected to check the sense buffer themselves. If available the data in
 * resid is written to residp. Note in SAT-2 and later, fixed format sense
 * data may be placed in *sensep in which case sensep[0]==0x70, prior to
 * SAT-2 descriptor sense format was required (i.e. sensep[0]==0x72).
 */
int
sg_ll_ata_pt(int sg_fd, const uint8_t * cdbp, int cdb_len,
             int timeout_secs, void * dinp, void * doutp, int dlen,
             uint8_t * sensep, int max_sense_len,
             uint8_t * ata_return_dp, int max_ata_return_len,
             int * residp, int vb)
{
    int k, res, slen, duration;
    int ret = -1;
    uint8_t apt_cdb[ATA_PT_32_CMDLEN];
    uint8_t sense_b[SENSE_BUFF_LEN];
    uint8_t * sp;
    const uint8_t * bp;
    struct sg_pt_base * ptvp;
    const char * cnamep;
    char b[256];

    memset(apt_cdb, 0, sizeof(apt_cdb));
    b[0] = '\0';
    switch (cdb_len) {
    case 12:
        cnamep = "ATA pass-through(12)";
        apt_cdb[0] = ATA_PT_12_CMD;
        memcpy(apt_cdb + 1, cdbp + 1,  10);
        /* control byte at cdb[11] left at zero */
        break;
    case 16:
        cnamep = "ATA pass-through(16)";
        apt_cdb[0] = ATA_PT_16_CMD;
        memcpy(apt_cdb + 1, cdbp + 1,  14);
        /* control byte at cdb[15] left at zero */
        break;
    case 32:
        cnamep = "ATA pass-through(32)";
        apt_cdb[0] = SG_VARIABLE_LENGTH_CMD;
        /* control byte at cdb[1] left at zero */
        apt_cdb[7] = 0x18;    /* length starting at next byte */
        sg_put_unaligned_be16(ATA_PT_32_SA, apt_cdb + 8);
        memcpy(apt_cdb + 10, cdbp + 10,  32 - 10);
        break;
    default:
        pr2ws("cdb_len must be 12, 16 or 32\n");
        return -1;
    }
    if (NULL == cdbp) {
        if (vb)
            pr2ws("%s NULL cdb pointer\n", cnamep);
        return -1;
    }
    if (sensep && (max_sense_len >= (int)sizeof(sense_b))) {
        sp = sensep;
        slen = max_sense_len;
    } else {
        sp = sense_b;
        slen = sizeof(sense_b);
    }
    if (vb) {
        if (cdb_len < 32) {
            char d[128];

            pr2ws("    %s cdb: %s\n", cnamep,
                  sg_get_command_str(apt_cdb, cdb_len, false, sizeof(d), d));
        } else {
            pr2ws("    %s cdb:\n", cnamep);
            hex2stderr(apt_cdb, cdb_len, -1);
        }
    }
    if (NULL == ((ptvp = create_pt_obj(cnamep))))
        return -1;
    set_scsi_pt_cdb(ptvp, apt_cdb, cdb_len);
    set_scsi_pt_sense(ptvp, sp, slen);
    if (dlen > 0) {
        if (dinp)
            set_scsi_pt_data_in(ptvp, (uint8_t *)dinp, dlen);
        else if (doutp)
            set_scsi_pt_data_out(ptvp, (uint8_t *)doutp, dlen);
    }
    res = do_scsi_pt(ptvp, sg_fd,
                     ((timeout_secs > 0) ? timeout_secs : DEF_PT_TIMEOUT),
                     vb);
    if (SCSI_PT_DO_BAD_PARAMS == res) {
        if (vb)
            pr2ws("%s: bad parameters\n", cnamep);
        goto out;
    } else if (SCSI_PT_DO_TIMEOUT == res) {
        if (vb)
            pr2ws("%s: timeout\n", cnamep);
        goto out;
    } else if (res > 2) {
        if (vb)
            pr2ws("%s: do_scsi_pt: errno=%d\n", cnamep, -res);
    }

    if ((vb > 2) && ((duration = get_scsi_pt_duration_ms(ptvp)) >= 0))
        pr2ws("      duration=%d ms\n", duration);

    switch (get_scsi_pt_result_category(ptvp)) {
    case SCSI_PT_RESULT_GOOD:
        if ((sensep) && (max_sense_len > 0))
            *sensep = 0;
        if ((ata_return_dp) && (max_ata_return_len > 0))
            *ata_return_dp = 0;
        if (residp && (dlen > 0))
            *residp = get_scsi_pt_resid(ptvp);
        ret = 0;
        break;
    case SCSI_PT_RESULT_STATUS: /* other than GOOD + CHECK CONDITION */
        if ((sensep) && (max_sense_len > 0))
            *sensep = 0;
        if ((ata_return_dp) && (max_ata_return_len > 0))
            *ata_return_dp = 0;
        ret = get_scsi_pt_status_response(ptvp);
        break;
    case SCSI_PT_RESULT_SENSE:
        if (sensep && (sp != sensep)) {
            k = get_scsi_pt_sense_len(ptvp);
            k = (k > max_sense_len) ? max_sense_len : k;
            memcpy(sensep, sp, k);
        }
        if (ata_return_dp && (max_ata_return_len > 0))  {
            /* search for ATA return descriptor */
            bp = sg_scsi_sense_desc_find(sp, slen, 0x9);
            if (bp) {
                k = bp[1] + 2;
                k = (k > max_ata_return_len) ? max_ata_return_len : k;
                memcpy(ata_return_dp, bp, k);
            } else
                ata_return_dp[0] = 0x0;
        }
        if (residp && (dlen > 0))
            *residp = get_scsi_pt_resid(ptvp);
        ret = get_scsi_pt_status_response(ptvp);
        break;
    case SCSI_PT_RESULT_TRANSPORT_ERR:
        if (vb)
            pr2ws("%s: transport error: %s\n", cnamep,
                  get_scsi_pt_transport_err_str(ptvp, sizeof(b), b));
        break;
    case SCSI_PT_RESULT_OS_ERR:
        if (vb)
            pr2ws("%s: os error: %s\n", cnamep,
                  get_scsi_pt_os_err_str(ptvp, sizeof(b) , b));
        break;
    default:
        if (vb)
            pr2ws("%s: unknown pt_result_category=%d\n", cnamep,
                  get_scsi_pt_result_category(ptvp));
        break;
    }

out:
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ BUFFER(10) command (SPC). Return of 0 -> success
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_buffer(int sg_fd, int mode, int buffer_id, int buffer_offset,
                  void * resp, int mx_resp_len, bool noisy, int vb)
{
    static const char * const cdb_s = "read buffer(10)";
    int res, ret, s_cat;
    uint8_t rbuf_cdb[READ_BUFFER_CMDLEN] =
        {READ_BUFFER_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    rbuf_cdb[1] = (uint8_t)(mode & 0x1f);
    rbuf_cdb[2] = (uint8_t)(buffer_id & 0xff);
    sg_put_unaligned_be24((uint32_t)buffer_offset, rbuf_cdb + 3);
    sg_put_unaligned_be24((uint32_t)mx_resp_len, rbuf_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rbuf_cdb, READ_BUFFER_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, rbuf_cdb, sizeof(rbuf_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI WRITE BUFFER command (SPC). Return of 0 -> success
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_write_buffer(int sg_fd, int mode, int buffer_id, int buffer_offset,
                   void * paramp, int param_len, bool noisy, int vb)
{
    static const char * const cdb_s = "write buffer";
    int res, ret, s_cat;
    uint8_t wbuf_cdb[WRITE_BUFFER_CMDLEN] =
        {WRITE_BUFFER_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    wbuf_cdb[1] = (uint8_t)(mode & 0x1f);
    wbuf_cdb[2] = (uint8_t)(buffer_id & 0xff);
    sg_put_unaligned_be24((uint32_t)buffer_offset, wbuf_cdb + 3);
    sg_put_unaligned_be24((uint32_t)param_len, wbuf_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(wbuf_cdb, WRITE_BUFFER_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list", cdb_s);
            if (2 == vb) {
                pr2ws("%s:\n", (param_len > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)paramp,
                           (param_len > 256 ? 256 : param_len), -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)paramp, param_len, 0);
            }
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, wbuf_cdb, sizeof(wbuf_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI WRITE BUFFER command (SPC). Return of 0 ->
 * success, SG_LIB_CAT_INVALID_OP -> invalid opcode,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure. Adds mode specific field (spc4r32) and timeout
 *  to command abort to override default of 60 seconds. If timeout_secs is
 *  0 or less then the default timeout is used instead. */
int
sg_ll_write_buffer_v2(int sg_fd, int mode, int m_specific, int buffer_id,
                      uint32_t buffer_offset, void * paramp,
                      uint32_t param_len, int timeout_secs, bool noisy,
                      int vb)
{
    int res, ret, s_cat;
    uint8_t wbuf_cdb[WRITE_BUFFER_CMDLEN] =
        {WRITE_BUFFER_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (buffer_offset > 0xffffff) {
        pr2ws("%s: buffer_offset value too large for 24 bits\n", __func__);
        return -1;
    }
    if (param_len > 0xffffff) {
        pr2ws("%s: param_len value too large for 24 bits\n", __func__);
        return -1;
    }
    wbuf_cdb[1] = (uint8_t)(mode & 0x1f);
    wbuf_cdb[1] |= (uint8_t)((m_specific & 0x7) << 5);
    wbuf_cdb[2] = (uint8_t)(buffer_id & 0xff);
    sg_put_unaligned_be24(buffer_offset, wbuf_cdb + 3);
    sg_put_unaligned_be24(param_len, wbuf_cdb + 6);
    if (vb) {
        char b[128];

        pr2ws("    Write buffer cdb: %s\n",
              sg_get_command_str(wbuf_cdb, WRITE_BUFFER_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    Write buffer parameter list%s:\n",
                  ((param_len > 256) ? " (first 256 bytes)" : ""));
            hex2stderr((const uint8_t *)paramp,
                       ((param_len > 256) ? 256 : param_len), -1);
        }
    }
    if (timeout_secs <= 0)
        timeout_secs = DEF_PT_TIMEOUT;

    ptvp = construct_scsi_pt_obj();
    if (NULL == ptvp) {
        pr2ws("%s: out of memory\n", __func__);
        return -1;
    }
    set_scsi_pt_cdb(ptvp, wbuf_cdb, sizeof(wbuf_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, timeout_secs, vb);
    ret = sg_cmds_process_resp(ptvp, "Write buffer", res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI UNMAP command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_unmap(int sg_fd, int group_num, int timeout_secs, void * paramp,
            int param_len, bool noisy, int vb)
{
    return sg_ll_unmap_v2(sg_fd, false, group_num, timeout_secs, paramp,
                          param_len, noisy, vb);
}

/* Invokes a SCSI UNMAP (SBC-3) command. Version 2 adds anchor field
 * (sbc3r22). Otherwise same as sg_ll_unmap() . */
int
sg_ll_unmap_v2(int sg_fd, bool anchor, int group_num, int timeout_secs,
               void * paramp, int param_len, bool noisy, int vb)
{
    static const char * const cdb_s = "unmap";
    int res, ret, s_cat, tmout;
    uint8_t u_cdb[UNMAP_CMDLEN] =
                         {UNMAP_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (anchor)
        u_cdb[1] |= 0x1;
    tmout = (timeout_secs > 0) ? timeout_secs : DEF_PT_TIMEOUT;
    u_cdb[6] = group_num & 0x1f;
    sg_put_unaligned_be16((uint16_t)param_len, u_cdb + 7);
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(u_cdb, UNMAP_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, u_cdb, sizeof(u_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, tmout, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ BLOCK LIMITS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_read_block_limits(int sg_fd, void * resp, int mx_resp_len,
                        bool noisy, int vb)
{
    static const char * const cdb_s = "read block limits";
    int ret, res, s_cat;
    uint8_t rl_cdb[READ_BLOCK_LIMITS_CMDLEN] =
      {READ_BLOCK_LIMITS_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rl_cdb, READ_BLOCK_LIMITS_CMDLEN,
                                 false, sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, rl_cdb, sizeof(rl_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else {
        if ((vb > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == vb) {
                pr2ws("%s:\n", (ret > 256 ? ", first 256 bytes" : ""));
                hex2stderr((const uint8_t *)resp, (ret > 256 ? 256 : ret),
                           -1);
            } else {
                pr2ws(":\n");
                hex2stderr((const uint8_t *)resp, ret, 0);
            }
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI RECEIVE COPY RESULTS command. Actually cover all current
 * uses of opcode 0x84 (Third-party copy IN). Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_receive_copy_results(int sg_fd, int sa, int list_id, void * resp,
                           int mx_resp_len, bool noisy, int vb)
{
    int res, ret, s_cat;
    uint8_t rcvcopyres_cdb[THIRD_PARTY_COPY_IN_CMDLEN] =
      {THIRD_PARTY_COPY_IN_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;
    char b[64];

    sg_get_opcode_sa_name(THIRD_PARTY_COPY_IN_CMD, sa, 0, (int)sizeof(b), b);
    rcvcopyres_cdb[1] = (uint8_t)(sa & 0x1f);
    if (sa <= 4)        /* LID1 variants */
        rcvcopyres_cdb[2] = (uint8_t)(list_id);
    else if ((sa >= 5) && (sa <= 7))    /* LID4 variants */
        sg_put_unaligned_be32((uint32_t)list_id, rcvcopyres_cdb + 2);
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rcvcopyres_cdb + 10);

    if (vb) {
        char d[128];

        pr2ws("    %s cdb: %s\n", b,
              sg_get_command_str(rcvcopyres_cdb, THIRD_PARTY_COPY_IN_CMDLEN,
                                 false, sizeof(d), d));
    }

    if (NULL == ((ptvp = create_pt_obj(b))))
        return -1;
    set_scsi_pt_cdb(ptvp, rcvcopyres_cdb, sizeof(rcvcopyres_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, b, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}


/* SPC-4 rev 35 and later calls this opcode (0x83) "Third-party copy OUT"
 * The original EXTENDED COPY command (now called EXTENDED COPY (LID1))
 * is the only one supported by sg_ll_extended_copy(). See function
 * sg_ll_3party_copy_out() for the other service actions ( > 0 ). */

/* Invokes a SCSI EXTENDED COPY (LID1) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_extended_copy(int sg_fd, void * paramp, int param_len, bool noisy,
                    int vb)
{
    int res, ret, s_cat;
    uint8_t xcopy_cdb[THIRD_PARTY_COPY_OUT_CMDLEN] =
      {THIRD_PARTY_COPY_OUT_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;
    const char * cdb_s = "Extended copy (LID1)";

    xcopy_cdb[1] = (uint8_t)(EXTENDED_COPY_LID1_SA & 0x1f);
    sg_put_unaligned_be32((uint32_t)param_len, xcopy_cdb + 10);

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(xcopy_cdb, THIRD_PARTY_COPY_OUT_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cdb_s);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, xcopy_cdb, sizeof(xcopy_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, vb);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Handles various service actions associated with opcode 0x83 which is
 * called THIRD PARTY COPY OUT. These include the EXTENDED COPY(LID1 and
 * LID4), POPULATE TOKEN and WRITE USING TOKEN commands.
 * Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_3party_copy_out(int sg_fd, int sa, unsigned int list_id, int group_num,
                      int timeout_secs, void * paramp, int param_len,
                      bool noisy, int vb)
{
    int res, ret, s_cat, tmout;
    uint8_t xcopy_cdb[THIRD_PARTY_COPY_OUT_CMDLEN] =
      {THIRD_PARTY_COPY_OUT_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;
    char cname[80];

    sg_get_opcode_sa_name(THIRD_PARTY_COPY_OUT_CMD, sa, 0, sizeof(cname),
                          cname);
    xcopy_cdb[1] = (uint8_t)(sa & 0x1f);
    switch (sa) {
    case 0x0:   /* XCOPY(LID1) */
    case 0x1:   /* XCOPY(LID4) */
        sg_put_unaligned_be32((uint32_t)param_len, xcopy_cdb + 10);
        break;
    case 0x10:  /* POPULATE TOKEN (SBC-3) */
    case 0x11:  /* WRITE USING TOKEN (SBC-3) */
        sg_put_unaligned_be32((uint32_t)list_id, xcopy_cdb + 6);
        sg_put_unaligned_be32((uint32_t)param_len, xcopy_cdb + 10);
        xcopy_cdb[14] = (uint8_t)(group_num & 0x1f);
        break;
    case 0x1c:  /* COPY OPERATION ABORT */
        sg_put_unaligned_be32((uint32_t)list_id, xcopy_cdb + 2);
        break;
    default:
        pr2ws("%s: unknown service action 0x%x\n", __func__, sa);
        return -1;
    }
    tmout = (timeout_secs > 0) ? timeout_secs : DEF_PT_TIMEOUT;

    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cname,
              sg_get_command_str(xcopy_cdb, THIRD_PARTY_COPY_OUT_CMDLEN,
                                 false, sizeof(b), b));
        if ((vb > 1) && paramp && param_len) {
            pr2ws("    %s parameter list:\n", cname);
            hex2stderr((const uint8_t *)paramp, param_len, -1);
        }
    }

    if (NULL == ((ptvp = create_pt_obj(cname))))
        return -1;
    set_scsi_pt_cdb(ptvp, xcopy_cdb, sizeof(xcopy_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, tmout, vb);
    ret = sg_cmds_process_resp(ptvp, cname, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI PRE-FETCH(10), PRE-FETCH(16) or SEEK(10) command (SBC).
 * Returns 0 -> success, 25 (SG_LIB_CAT_CONDITION_MET), various SG_LIB_CAT_*
 * positive values or -1 -> other errors. Note that CONDITION MET status
 * is returned when immed=true and num_blocks can fit in device's cache,
 * somewaht strangely, GOOD status (return 0) is returned if num_blocks
 * cannot fit in device's cache. If do_seek10==true then does a SEEK(10)
 * command with given lba, if that LBA is < 2**32 . Unclear what SEEK(10)
 * does, assume it is like PRE-FETCH. If timeout_secs is 0 (or less) then
 * use DEF_PT_TIMEOUT (60 seconds) as command timeout. */
int
sg_ll_pre_fetch_x(int sg_fd, bool do_seek10, bool cdb16, bool immed,
                  uint64_t lba, uint32_t num_blocks, int group_num,
                  int timeout_secs, bool noisy, int vb)
{
    static const char * const cdb10_name_s = "Pre-fetch(10)";
    static const char * const cdb16_name_s = "Pre-fetch(16)";
    static const char * const cdb_seek_name_s = "Seek(10)";
    int res, s_cat, ret, cdb_len, tmout;
    const char *cdb_s;
    uint8_t preFetchCdb[PRE_FETCH16_CMDLEN]; /* all use longest cdb */
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    memset(preFetchCdb, 0, sizeof(preFetchCdb));
    if (do_seek10) {
        if (lba > UINT32_MAX) {
            if (vb)
                pr2ws("%s: LBA exceeds 2**32 in %s\n", __func__,
                      cdb_seek_name_s);
            return -1;
        }
        preFetchCdb[0] = SEEK10_CMD;
        cdb_len = SEEK10_CMDLEN;
        cdb_s = cdb_seek_name_s;
        sg_put_unaligned_be32((uint32_t)lba, preFetchCdb + 2);
    } else {
        if ((! cdb16) &&
            ((lba > UINT32_MAX) || (num_blocks > UINT16_MAX))) {
            cdb16 = true;
            if (noisy || vb)
                pr2ws("%s: do %s due to %s size\n", __func__, cdb16_name_s,
                      (lba > UINT32_MAX) ? "LBA" : "NUM_BLOCKS");
        }
        if (cdb16) {
            preFetchCdb[0] = PRE_FETCH16_CMD;
            cdb_len = PRE_FETCH16_CMDLEN;
            cdb_s = cdb16_name_s;
            if (immed)
                preFetchCdb[1] = 0x2;
            sg_put_unaligned_be64(lba, preFetchCdb + 2);
            sg_put_unaligned_be32(num_blocks, preFetchCdb + 10);
            preFetchCdb[14] = 0x3f & group_num;
        } else {
            preFetchCdb[0] = PRE_FETCH10_CMD;
            cdb_len = PRE_FETCH10_CMDLEN;
            cdb_s = cdb10_name_s;
            if (immed)
                preFetchCdb[1] = 0x2;
            sg_put_unaligned_be32((uint32_t)lba, preFetchCdb + 2);
            preFetchCdb[6] = 0x3f & group_num;
            sg_put_unaligned_be16((uint16_t)num_blocks, preFetchCdb + 7);
        }
    }
    tmout = (timeout_secs > 0) ? timeout_secs : DEF_PT_TIMEOUT;
    if (vb) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(preFetchCdb, cdb_len, false, sizeof(b), b));
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, preFetchCdb, cdb_len);
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    res = do_scsi_pt(ptvp, sg_fd, tmout, vb);
    if (0 == res) {
        int sstat = get_scsi_pt_status_response(ptvp);

        if (SG_LIB_CAT_CONDITION_MET == sstat) {
            ret = SG_LIB_CAT_CONDITION_MET;
            if (vb > 2)
                pr2ws("%s: returns SG_LIB_CAT_CONDITION_MET\n", __func__);
            goto fini;
        }
    }
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, vb, &s_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (s_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = s_cat;
            break;
        }
    } else
        ret = 0;
fini:
    destruct_scsi_pt_obj(ptvp);
    return ret;
}
