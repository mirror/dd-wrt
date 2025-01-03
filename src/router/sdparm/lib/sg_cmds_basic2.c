/*
 * Copyright (c) 1999-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * CONTENTS
 *    Some SCSI commands are executed in many contexts and hence began
 *    to appear in several sg3_utils utilities. This files centralizes
 *    some of the low level command execution code. In most cases the
 *    interpretation of the command response is left to the each
 *    utility.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_cmds_basic.h"
#include "sg_pt.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"



#define SENSE_BUFF_LEN 64       /* Arbitrary, could be larger */
#define EBUFF_SZ 256

#define DEF_PT_TIMEOUT 60       /* 60 seconds */
#define START_PT_TIMEOUT 120    /* 120 seconds == 2 minutes */
#define LONG_PT_TIMEOUT 7200    /* 7,200 seconds == 120 minutes */

#define SYNCHRONIZE_CACHE_CMD     0x35
#define SYNCHRONIZE_CACHE_CMDLEN  10
#define SERVICE_ACTION_IN_16_CMD 0x9e
#define SERVICE_ACTION_IN_16_CMDLEN 16
#define READ_CAPACITY_16_SA 0x10
#define READ_CAPACITY_10_CMD 0x25
#define READ_CAPACITY_10_CMDLEN 10
#define MODE_SENSE6_CMD      0x1a
#define MODE_SENSE6_CMDLEN   6
#define MODE_SENSE10_CMD     0x5a
#define MODE_SENSE10_CMDLEN  10
#define MODE_SELECT6_CMD   0x15
#define MODE_SELECT6_CMDLEN   6
#define MODE_SELECT10_CMD   0x55
#define MODE_SELECT10_CMDLEN  10
#define LOG_SENSE_CMD     0x4d
#define LOG_SENSE_CMDLEN  10
#define LOG_SELECT_CMD     0x4c
#define LOG_SELECT_CMDLEN  10
#define START_STOP_CMD          0x1b
#define START_STOP_CMDLEN       6
#define PREVENT_ALLOW_CMD    0x1e
#define PREVENT_ALLOW_CMDLEN   6

#define MODE6_RESP_HDR_LEN 4
#define MODE10_RESP_HDR_LEN 8
#define MODE_RESP_ARB_LEN 1024

#define INQUIRY_RESP_INITIAL_LEN 36


static struct sg_pt_base *
create_pt_obj(const char * cname)
{
    struct sg_pt_base * ptvp = construct_scsi_pt_obj();
    if (NULL == ptvp)
        pr2ws("%s: out of memory\n", cname);
    return ptvp;
}

/* Invokes a SCSI SYNCHRONIZE CACHE (10) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_sync_cache_10(int sg_fd, bool sync_nv, bool immed, int group,
                    unsigned int lba, unsigned int count, bool noisy,
                    int verbose)
{
    static const char * const cdb_s = "synchronize cache(10)";
    int res, ret, sense_cat;
    uint8_t sc_cdb[SYNCHRONIZE_CACHE_CMDLEN] =
                {SYNCHRONIZE_CACHE_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (sync_nv)
        sc_cdb[1] |= 4;
    if (immed)
        sc_cdb[1] |= 2;
    sg_put_unaligned_be32((uint32_t)lba, sc_cdb + 2);
    sc_cdb[6] = group & 0x1f;
    if (count > 0xffff) {
        pr2ws("count too big\n");
        return -1;
    }
    sg_put_unaligned_be16((int16_t)count, sc_cdb + 7);

    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(sc_cdb, SYNCHRONIZE_CACHE_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, sc_cdb, sizeof(sc_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ CAPACITY (16) command. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_readcap_16(int sg_fd, bool pmi, uint64_t llba, void * resp,
                 int mx_resp_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "read capacity(16)";
    int ret, res, sense_cat;
    uint8_t rc_cdb[SERVICE_ACTION_IN_16_CMDLEN] =
                        {SERVICE_ACTION_IN_16_CMD, READ_CAPACITY_16_SA,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (pmi) { /* lbs only valid when pmi set */
        rc_cdb[14] |= 1;
        sg_put_unaligned_be64(llba, rc_cdb + 2);
    }
    /* Allocation length, no guidance in SBC-2 rev 15b */
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rc_cdb + 10);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rc_cdb, SERVICE_ACTION_IN_16_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, rc_cdb, sizeof(rc_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI READ CAPACITY (10) command. Returns 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_readcap_10(int sg_fd, bool pmi, unsigned int lba, void * resp,
                 int mx_resp_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "read capacity(10)";
    int ret, res, sense_cat;
    uint8_t rc_cdb[READ_CAPACITY_10_CMDLEN] =
                         {READ_CAPACITY_10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (pmi) { /* lbs only valid when pmi set */
        rc_cdb[8] |= 1;
        sg_put_unaligned_be32((uint32_t)lba, rc_cdb + 2);
    }
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(rc_cdb, READ_CAPACITY_10_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, rc_cdb, sizeof(rc_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI MODE SENSE (6) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_mode_sense6(int sg_fd, bool dbd, int pc, int pg_code, int sub_pg_code,
                  void * resp, int mx_resp_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "mode sense(6)";
    int res, ret, sense_cat, resid;
    uint8_t modes_cdb[MODE_SENSE6_CMDLEN] =
        {MODE_SENSE6_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    modes_cdb[1] = (uint8_t)(dbd ? 0x8 : 0);
    modes_cdb[2] = (uint8_t)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
    modes_cdb[3] = (uint8_t)(sub_pg_code & 0xff);
    modes_cdb[4] = (uint8_t)(mx_resp_len & 0xff);
    if (mx_resp_len > 0xff) {
        pr2ws("mx_resp_len too big\n");
        return -1;
    }
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(modes_cdb, MODE_SENSE6_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, modes_cdb, sizeof(modes_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    resid = get_scsi_pt_resid(ptvp);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else {
        if ((verbose > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == verbose) {
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

    if (resid > 0) {
        if (resid > mx_resp_len) {
            pr2ws("%s: resid (%d) should never exceed requested len=%d\n",
                  cdb_s, resid, mx_resp_len);
            return ret ? ret : SG_LIB_CAT_MALFORMED;
        }
        /* zero unfilled section of response buffer */
        memset((uint8_t *)resp + (mx_resp_len - resid), 0, resid);
    }
    return ret;
}

/* Invokes a SCSI MODE SENSE (10) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_mode_sense10(int sg_fd, bool llbaa, bool dbd, int pc, int pg_code,
                   int sub_pg_code, void * resp, int mx_resp_len,
                   bool noisy, int verbose)
{
    return sg_ll_mode_sense10_v2(sg_fd, llbaa, dbd, pc, pg_code, sub_pg_code,
                                 resp, mx_resp_len, 0, NULL, noisy, verbose);
}

/* Invokes a SCSI MODE SENSE (10) command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors.
 * Adds the ability to set the command abort timeout
 * and the ability to report the residual count. If timeout_secs is zero
 * or less the default command abort timeout (60 seconds) is used.
 * If residp is non-NULL then the residual value is written where residp
 * points. A residual value of 0 implies mx_resp_len bytes have be written
 * where resp points. If the residual value equals mx_resp_len then no
 * bytes have been written. */
int
sg_ll_mode_sense10_v2(int sg_fd, bool llbaa, bool dbd, int pc, int pg_code,
                      int sub_pg_code, void * resp, int mx_resp_len,
                      int timeout_secs, int * residp, bool noisy, int verbose)
{
    int res, ret, sense_cat, resid;
    static const char * const cdb_s = "mode sense(10)";
    struct sg_pt_base * ptvp;
    uint8_t modes_cdb[MODE_SENSE10_CMDLEN] =
        {MODE_SENSE10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    modes_cdb[1] = (uint8_t)((dbd ? 0x8 : 0) | (llbaa ? 0x10 : 0));
    modes_cdb[2] = (uint8_t)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
    modes_cdb[3] = (uint8_t)(sub_pg_code & 0xff);
    sg_put_unaligned_be16((int16_t)mx_resp_len, modes_cdb + 7);
    if (mx_resp_len > 0xffff) {
        pr2ws("mx_resp_len too big\n");
        goto gen_err;
    }
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(modes_cdb, MODE_SENSE10_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (timeout_secs <= 0)
        timeout_secs = DEF_PT_TIMEOUT;

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        goto gen_err;
    set_scsi_pt_cdb(ptvp, modes_cdb, sizeof(modes_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, timeout_secs, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    resid = get_scsi_pt_resid(ptvp);
    if (residp)
        *residp = resid;
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else {
        if ((verbose > 2) && (ret > 0)) {
            pr2ws("    %s: response", cdb_s);
            if (3 == verbose) {
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

    if (resid > 0) {
        if (resid > mx_resp_len) {
            pr2ws("%s: resid (%d) should never exceed requested len=%d\n",
                  cdb_s, resid, mx_resp_len);
            return ret ? ret : SG_LIB_CAT_MALFORMED;
        }
        /* zero unfilled section of response buffer */
        memset((uint8_t *)resp + (mx_resp_len - resid), 0, resid);
    }
    return ret;
gen_err:
    if (residp)
        *residp = 0;
    return -1;
}

/* Invokes a SCSI MODE SELECT (6) command.  Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_mode_select6_v2(int sg_fd, bool pf, bool rtd, bool sp, void * paramp,
                      int param_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "mode select(6)";
    int res, ret, sense_cat;
    uint8_t modes_cdb[MODE_SELECT6_CMDLEN] =
        {MODE_SELECT6_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    modes_cdb[1] = (uint8_t)((pf ? 0x10 : 0x0) | (sp ? 0x1 : 0x0));
    if (rtd)
        modes_cdb[1] |= 0x2;
    modes_cdb[4] = (uint8_t)(param_len & 0xff);
    if (param_len > 0xff) {
        pr2ws("%s: param_len too big\n", cdb_s);
        return -1;
    }
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(modes_cdb, MODE_SELECT6_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (verbose > 1) {
        pr2ws("    %s parameter list\n", cdb_s);
        hex2stderr((const uint8_t *)paramp, param_len, -1);
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, modes_cdb, sizeof(modes_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

int
sg_ll_mode_select6(int sg_fd, bool pf, bool sp, void * paramp, int param_len,
                   bool noisy, int verbose)
{
    return sg_ll_mode_select6_v2(sg_fd, pf, false, sp, paramp, param_len,
                                 noisy, verbose);
}

/* Invokes a SCSI MODE SELECT (10) command.  Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors,
 * v2 adds rtd (revert to defaults) bit (spc5r11).  */
int
sg_ll_mode_select10_v2(int sg_fd, bool pf, bool rtd, bool sp, void * paramp,
                       int param_len, bool noisy, int verbose)
{
    static const char * const cdb_s = "mode select(10)";
    int res, ret, sense_cat;
    uint8_t modes_cdb[MODE_SELECT10_CMDLEN] =
        {MODE_SELECT10_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    modes_cdb[1] = (uint8_t)((pf ? 0x10 : 0x0) | (sp ? 0x1 : 0x0));
    if (rtd)
        modes_cdb[1] |= 0x2;
    sg_put_unaligned_be16((int16_t)param_len, modes_cdb + 7);
    if (param_len > 0xffff) {
        pr2ws("%s: param_len too big\n", cdb_s);
        return -1;
    }
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(modes_cdb, MODE_SELECT10_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (verbose > 1) {
        pr2ws("    %s parameter list\n", cdb_s);
        hex2stderr((const uint8_t *)paramp, param_len, -1);
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, modes_cdb, sizeof(modes_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, (uint8_t *)paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

int
sg_ll_mode_select10(int sg_fd, bool pf, bool sp, void * paramp,
                       int param_len, bool noisy, int verbose)
{
    return sg_ll_mode_select10_v2(sg_fd, pf, false, sp, paramp, param_len,
                                  noisy, verbose);
}

/* MODE SENSE commands yield a response that has header then zero or more
 * block descriptors followed by mode pages. In most cases users are
 * interested in the first mode page. This function returns the (byte)
 * offset of the start of the first mode page. Set mode_sense_6 to true for
 * MODE SENSE (6) and false for MODE SENSE (10). Returns >= 0 is successful
 * or -1 if failure. If there is a failure a message is written to err_buff
 * if it is non-NULL and err_buff_len > 0. */
int
sg_mode_page_offset(const uint8_t * resp, int resp_len,
                    bool mode_sense_6, char * err_buff, int err_buff_len)
{
    int bd_len, calc_len, offset;
    bool err_buff_ok = ((err_buff_len > 0) && err_buff);

    if ((NULL == resp) || (resp_len < 4))
            goto too_short;
    if (mode_sense_6) {
        calc_len = resp[0] + 1;
        bd_len = resp[3];
        offset = bd_len + MODE6_RESP_HDR_LEN;
    } else {    /* Mode sense(10) */
        if (resp_len < 8)
            goto too_short;
        calc_len = sg_get_unaligned_be16(resp) + 2;
        bd_len = sg_get_unaligned_be16(resp + 6);
        /* LongLBA doesn't change this calculation */
        offset = bd_len + MODE10_RESP_HDR_LEN;
    }
    if ((offset + 2) > calc_len) {
        if (err_buff_ok)
            snprintf(err_buff, err_buff_len, "calculated response "
                     "length too small, offset=%d calc_len=%d bd_len=%d\n",
                     offset, calc_len, bd_len);
        offset = -1;
    }
    return offset;
too_short:
    if (err_buff_ok)
        snprintf(err_buff, err_buff_len, "given MS(%d) response length (%d) "
                 "too short\n", (mode_sense_6 ? 6 : 10), resp_len);
    return -1;
}

/* MODE SENSE commands yield a response that has header then zero or more
 * block descriptors followed by mode pages. This functions returns the
 * length (in bytes) of those three components. Note that the return value
 * can exceed resp_len in which case the MODE SENSE command should be
 * re-issued with a larger response buffer. If bd_lenp is non-NULL and if
 * successful the block descriptor length (in bytes) is written to *bd_lenp.
 * Set mode_sense_6 to true for MODE SENSE (6) and false for MODE SENSE (10)
 * responses. Returns -1 if there is an error (e.g. response too short). */
int
sg_msense_calc_length(const uint8_t * resp, int resp_len,
                      bool mode_sense_6, int * bd_lenp)
{
    int calc_len;

    if (NULL == resp)
        goto an_err;
    if (mode_sense_6) {
        if (resp_len < 4)
            goto an_err;
        calc_len = resp[0] + 1;
    } else {
        if (resp_len < 8)
            goto an_err;
        calc_len = sg_get_unaligned_be16(resp + 0) + 2;
    }
    if (bd_lenp)
        *bd_lenp = mode_sense_6 ? resp[3] : sg_get_unaligned_be16(resp + 6);
    return calc_len;
an_err:
    if (bd_lenp)
        *bd_lenp = 0;
    return -1;
}

/* Fetches current, changeable, default and/or saveable modes pages as
 * indicated by pcontrol_arr for given pg_code and sub_pg_code. If
 * mode6==false then use MODE SENSE (10) else use MODE SENSE (6). If
 * flexible set and mode data length seems wrong then try and
 * fix (compensating hack for bad device or driver). pcontrol_arr
 * should have 4 elements for output of current, changeable, default
 * and saved values respectively. Each element should be NULL or
 * at least mx_mpage_len bytes long.
 * Return of 0 -> overall success, various SG_LIB_CAT_* positive values or
 * -1 -> other errors.
 * If success_mask pointer is not NULL then first zeros it. Then set bits
 * 0, 1, 2 and/or 3 if the current, changeable, default and saved values
 * respectively have been fetched. If error on current page
 * then stops and returns that error; otherwise continues if an error is
 * detected but returns the first error encountered.  */
int
sg_get_mode_page_controls(int sg_fd, bool mode6, int pg_code, int sub_pg_code,
                          bool dbd, bool flexible, int mx_mpage_len,
                          int * success_mask, void * pcontrol_arr[],
                          int * reported_lenp, int verbose)
{
    bool resp_mode6;
    int k, n, res, offset, calc_len, xfer_len;
    int resid = 0;
    const int msense10_hlen = MODE10_RESP_HDR_LEN;
    uint8_t buff[MODE_RESP_ARB_LEN];
    char ebuff[EBUFF_SZ];
    int first_err = 0;

    if (success_mask)
        *success_mask = 0;
    if (reported_lenp)
        *reported_lenp = 0;
    if (mx_mpage_len < 4)
        return 0;
    memset(ebuff, 0, sizeof(ebuff));
    /* first try to find length of current page response */
    memset(buff, 0, msense10_hlen);
    if (mode6)  /* want first 8 bytes just in case */
        res = sg_ll_mode_sense6(sg_fd, dbd, 0 /* pc */, pg_code,
                                sub_pg_code, buff, msense10_hlen, true,
                                verbose);
    else        /* MODE SENSE(10) obviously */
        res = sg_ll_mode_sense10_v2(sg_fd, false /* llbaa */, dbd,
                                    0 /* pc */, pg_code, sub_pg_code, buff,
                                    msense10_hlen, 0, &resid, true, verbose);
    if (0 != res)
        return res;
    n = buff[0];
    if (reported_lenp) {
        int m;

        m = sg_msense_calc_length(buff, msense10_hlen, mode6, NULL) - resid;
        if (m < 0)      /* Grrr, this should not happen */
            m = 0;
        *reported_lenp = m;
    }
    resp_mode6 = mode6;
    if (flexible) {
        if (mode6 && (n < 3)) {
            resp_mode6 = false;
            if (verbose)
                pr2ws(">>> msense(6) but resp[0]=%d so try msense(10) "
                      "response processing\n", n);
        }
        if ((! mode6) && (n > 5)) {
            if ((n > 11) && (0 == (n % 2)) && (0 == buff[4]) &&
                (0 == buff[5]) && (0 == buff[6])) {
                buff[1] = n;
                buff[0] = 0;
                if (verbose)
                    pr2ws(">>> msense(10) but resp[0]=%d and not msense(6) "
                          "response so fix length\n", n);
            } else
                resp_mode6 = true;
        }
    }
    if (verbose && (resp_mode6 != mode6))
        pr2ws(">>> msense(%d) but resp[0]=%d so switch response "
              "processing\n", (mode6 ? 6 : 10), buff[0]);
    calc_len = sg_msense_calc_length(buff, msense10_hlen, resp_mode6, NULL);
    if (calc_len > MODE_RESP_ARB_LEN)
        calc_len = MODE_RESP_ARB_LEN;
    offset = sg_mode_page_offset(buff, calc_len, resp_mode6, ebuff, EBUFF_SZ);
    if (offset < 0) {
        if (('\0' != ebuff[0]) && (verbose > 0))
            pr2ws("%s: %s\n", __func__, ebuff);
        return SG_LIB_CAT_MALFORMED;
    }
    xfer_len = calc_len - offset;
    if (xfer_len > mx_mpage_len)
        xfer_len = mx_mpage_len;

    for (k = 0; k < 4; ++k) {
        if (NULL == pcontrol_arr[k])
            continue;
        memset(pcontrol_arr[k], 0, mx_mpage_len);
        resid = 0;
        if (mode6)
            res = sg_ll_mode_sense6(sg_fd, dbd, k /* pc */,
                                    pg_code, sub_pg_code, buff,
                                    calc_len, true, verbose);
        else
            res = sg_ll_mode_sense10_v2(sg_fd, false /* llbaa */, dbd,
                                        k /* pc */, pg_code, sub_pg_code,
                                        buff, calc_len, 0, &resid, true,
                                        verbose);
        if (res || resid) {
            if (0 == first_err) {
                if (res)
                    first_err = res;
                else {
                    first_err = -49;    /* unexpected resid != 0 */
                    if (verbose)
                        pr2ws("%s: unexpected resid=%d, page=0x%x, "
                              "pcontrol=%d\n", __func__, resid, pg_code, k);
                }
            }
            if (0 == k)
                break;  /* if problem on current page, it won't improve */
            else
                continue;
        }
        if (xfer_len > 0)
            memcpy(pcontrol_arr[k], buff + offset, xfer_len);
        if (success_mask)
            *success_mask |= (1 << k);
    }
    return first_err;
}

/* Invokes a SCSI LOG SENSE command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors. */
int
sg_ll_log_sense(int sg_fd, bool ppc, bool sp, int pc, int pg_code,
                int subpg_code, int paramp, uint8_t * resp,
                int mx_resp_len, bool noisy, int verbose)
{
    return sg_ll_log_sense_v2(sg_fd, ppc, sp, pc, pg_code, subpg_code,
                              paramp, resp, mx_resp_len, 0, NULL, noisy,
                              verbose);
}

/* Invokes a SCSI LOG SENSE command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors.
 * Adds the ability to set the command abort timeout
 * and the ability to report the residual count. If timeout_secs is zero
 * or less the default command abort timeout (60 seconds) is used.
 * If residp is non-NULL then the residual value is written where residp
 * points. A residual value of 0 implies mx_resp_len bytes have be written
 * where resp points. If the residual value equals mx_resp_len then no
 * bytes have been written. */
int
sg_ll_log_sense_v2(int sg_fd, bool ppc, bool sp, int pc, int pg_code,
                   int subpg_code, int paramp, uint8_t * resp,
                   int mx_resp_len, int timeout_secs, int * residp,
                   bool noisy, int verbose)
{
    static const char * const cdb_s = "log sense";
    int res, ret, sense_cat, resid;
    uint8_t logs_cdb[LOG_SENSE_CMDLEN] =
        {LOG_SENSE_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (mx_resp_len > 0xffff) {
        pr2ws("mx_resp_len too big\n");
        goto gen_err;
    }
    logs_cdb[1] = (uint8_t)((ppc ? 2 : 0) | (sp ? 1 : 0));
    logs_cdb[2] = (uint8_t)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
    logs_cdb[3] = (uint8_t)(subpg_code & 0xff);
    sg_put_unaligned_be16((int16_t)paramp, logs_cdb + 5);
    sg_put_unaligned_be16((int16_t)mx_resp_len, logs_cdb + 7);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(logs_cdb, LOG_SENSE_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (timeout_secs <= 0)
        timeout_secs = DEF_PT_TIMEOUT;

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        goto gen_err;
    set_scsi_pt_cdb(ptvp, logs_cdb, sizeof(logs_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, timeout_secs, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    resid = get_scsi_pt_resid(ptvp);
    if (residp)
        *residp = resid;
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else {
        if ((mx_resp_len > 3) && (ret < 4)) {
            /* resid indicates LOG SENSE response length bad, so zero it */
            resp[2] = 0;
            resp[3] = 0;
        }
        ret = 0;
    }
    destruct_scsi_pt_obj(ptvp);

    if (resid > 0) {
        if (resid > mx_resp_len) {
            pr2ws("%s: resid (%d) should never exceed requested len=%d\n",
                  cdb_s, resid, mx_resp_len);
            return ret ? ret : SG_LIB_CAT_MALFORMED;
        }
        /* zero unfilled section of response buffer */
        memset((uint8_t *)resp + (mx_resp_len - resid), 0, resid);
    }
    return ret;
gen_err:
    if (residp)
        *residp = 0;
    return -1;
}

/* Invokes a SCSI LOG SELECT command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_log_select(int sg_fd, bool pcr, bool sp, int pc, int pg_code,
                 int subpg_code, uint8_t * paramp, int param_len,
                 bool noisy, int verbose)
{
    static const char * const cdb_s = "log select";
    int res, ret, sense_cat;
    uint8_t logs_cdb[LOG_SELECT_CMDLEN] =
        {LOG_SELECT_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if (param_len > 0xffff) {
        pr2ws("%s: param_len too big\n", cdb_s);
        return -1;
    }
    logs_cdb[1] = (uint8_t)((pcr ? 2 : 0) | (sp ? 1 : 0));
    logs_cdb[2] = (uint8_t)(((pc << 6) & 0xc0) | (pg_code & 0x3f));
    logs_cdb[3] = (uint8_t)(subpg_code & 0xff);
    sg_put_unaligned_be16((int16_t)param_len, logs_cdb + 7);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(logs_cdb, LOG_SELECT_CMDLEN, false,
                                 sizeof(b), b));
    }
    if ((verbose > 1) && (param_len > 0)) {
        pr2ws("    %s parameter list\n", cdb_s);
        hex2stderr(paramp, param_len, -1);
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, logs_cdb, sizeof(logs_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_out(ptvp, paramp, param_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
        ret = 0;

    destruct_scsi_pt_obj(ptvp);
    return ret;
}

/* Invokes a SCSI START STOP UNIT command (SBC + MMC).
 * Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors.
 * SBC-3 and MMC partially overlap on the power_condition_modifier(sbc) and
 * format_layer_number(mmc) fields. They also overlap on the noflush(sbc)
 * and fl(mmc) one bit field. This is the cause of the awkardly named
 * pc_mod__fl_num and noflush__fl arguments to this function.
 *  */
static int
sg_ll_start_stop_unit_com(struct sg_pt_base * ptvp, int sg_fd, bool immed,
                          int pc_mod__fl_num, int power_cond, bool noflush__fl,
                          bool loej, bool start, bool noisy, int verbose)
{
    static const char * const cdb_s = "start stop unit";
    bool ptvp_given = false;
    bool local_sense = true;
    bool local_cdb = true;
    int res, ret, sense_cat;
    uint8_t ssuBlk[START_STOP_CMDLEN] = {START_STOP_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    if (immed)
        ssuBlk[1] = 0x1;
    ssuBlk[3] = pc_mod__fl_num & 0xf;  /* bits 2 and 3 are reserved in MMC */
    ssuBlk[4] = ((power_cond & 0xf) << 4);
    if (noflush__fl)
        ssuBlk[4] |= 0x4;
    if (loej)
        ssuBlk[4] |= 0x2;
    if (start)
        ssuBlk[4] |= 0x1;
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(ssuBlk, sizeof(ssuBlk), false,
                                 sizeof(b), b));
    }
    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false; /* N.B. Ignores locally built cdb */
        else
            set_scsi_pt_cdb(ptvp, ssuBlk, sizeof(ssuBlk));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, verbose);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, ssuBlk, sizeof(ssuBlk));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    res = do_scsi_pt(ptvp, -1, START_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
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
sg_ll_start_stop_unit(int sg_fd, bool immed, int pc_mod__fl_num,
                      int power_cond, bool noflush__fl, bool loej, bool start,
                      bool noisy, int verbose)
{
    return sg_ll_start_stop_unit_com(NULL, sg_fd, immed, pc_mod__fl_num,
                                     power_cond, noflush__fl, loej, start,
                                     noisy, verbose);
}

int
sg_ll_start_stop_unit_pt(struct sg_pt_base * ptvp, bool immed,
                         int pc_mod__fl_num, int power_cond, bool noflush__fl,
                         bool loej, bool start, bool noisy, int verbose)
{
    return sg_ll_start_stop_unit_com(ptvp, -1, immed, pc_mod__fl_num,
                                     power_cond, noflush__fl, loej, start,
                                     noisy, verbose);
}

/* Invokes a SCSI PREVENT ALLOW MEDIUM REMOVAL command
 * [was in SPC-3 but displaced from SPC-4 into SBC-3, MMC-5, SSC-3]
 * prevent==0 allows removal, prevent==1 prevents removal ...
 * Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
int
sg_ll_prevent_allow(int sg_fd, int prevent, bool noisy, int verbose)
{
    static const char * const cdb_s = "prevent allow medium removal";
    int res, ret, sense_cat;
    uint8_t p_cdb[PREVENT_ALLOW_CMDLEN] =
                {PREVENT_ALLOW_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    struct sg_pt_base * ptvp;

    if ((prevent < 0) || (prevent > 3)) {
        pr2ws("prevent argument should be 0, 1, 2 or 3\n");
        return -1;
    }
    p_cdb[4] |= (prevent & 0x3);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", cdb_s,
              sg_get_command_str(p_cdb, PREVENT_ALLOW_CMDLEN, false,
                                 sizeof(b), b));
    }

    if (NULL == ((ptvp = create_pt_obj(cdb_s))))
        return -1;
    set_scsi_pt_cdb(ptvp, p_cdb, sizeof(p_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, cdb_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else
            ret = 0;
    destruct_scsi_pt_obj(ptvp);
    return ret;
}
