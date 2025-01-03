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
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_cmds_basic.h"
#include "sg_pt.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"

/* Needs to be after config.h */
#ifdef SG_LIB_LINUX
#include <errno.h>
#endif


static const char * const version_str = "1.97 20200722";


#define SENSE_BUFF_LEN 64       /* Arbitrary, could be larger */
#define EBUFF_SZ 256

#define DEF_PT_TIMEOUT 60       /* 60 seconds */
#define START_PT_TIMEOUT 120    /* 120 seconds == 2 minutes */
#define LONG_PT_TIMEOUT 7200    /* 7,200 seconds == 120 minutes */

#define INQUIRY_CMD     0x12
#define INQUIRY_CMDLEN  6
#define REQUEST_SENSE_CMD 0x3
#define REQUEST_SENSE_CMDLEN 6
#define REPORT_LUNS_CMD 0xa0
#define REPORT_LUNS_CMDLEN 12
#define TUR_CMD  0x0
#define TUR_CMDLEN  6

#define SAFE_STD_INQ_RESP_LEN 36 /* other lengths lock up some devices */


const char *
sg_cmds_version()
{
    return version_str;
}

/* Returns file descriptor >= 0 if successful. If error in Unix returns
   negated errno. */
int
sg_cmds_open_device(const char * device_name, bool read_only, int verbose)
{
    return scsi_pt_open_device(device_name, read_only, verbose);
}

/* Returns file descriptor >= 0 if successful. If error in Unix returns
   negated errno. */
int
sg_cmds_open_flags(const char * device_name, int flags, int verbose)
{
    return scsi_pt_open_flags(device_name, flags, verbose);
}

/* Returns 0 if successful. If error in Unix returns negated errno. */
int
sg_cmds_close_device(int device_fd)
{
    return scsi_pt_close_device(device_fd);
}

static const char * const pass_through_s = "pass-through";

static void
sg_cmds_resid_print(const char * leadin, bool is_din, int num_req,
                    int num_got)
{
    pr2ws("    %s: %s requested %d bytes (data-%s  got %d "
          "bytes%s\n", leadin, pass_through_s,num_req,
          (is_din ? "in), got" : "out) but reported"), num_got,
          (is_din ? "" : " sent"));
}

static int
sg_cmds_process_helper(const char * leadin, int req_din_x, int act_din_x,
                       int req_dout_x, int act_dout_x, const uint8_t * sbp,
                       int slen, bool noisy, int verbose, int * o_sense_cat)
{
    int scat;
    bool n = false;
    bool check_data_in = false;
    char b[512];

    scat = sg_err_category_sense(sbp, slen);
    switch (scat) {
    case SG_LIB_CAT_NOT_READY:
    case SG_LIB_CAT_INVALID_OP:
    case SG_LIB_CAT_ILLEGAL_REQ:
    case SG_LIB_LBA_OUT_OF_RANGE:
    case SG_LIB_CAT_ABORTED_COMMAND:
    case SG_LIB_CAT_COPY_ABORTED:
    case SG_LIB_CAT_DATA_PROTECT:
    case SG_LIB_CAT_PROTECTION:
    case SG_LIB_CAT_NO_SENSE:
    case SG_LIB_CAT_MISCOMPARE:
        n = false;
        break;
    case SG_LIB_CAT_RECOVERED:
    case SG_LIB_CAT_MEDIUM_HARD:
        check_data_in = true;
#if defined(__GNUC__)
#if (__GNUC__ >= 7)
        __attribute__((fallthrough));
        /* FALL THROUGH */
#endif
#endif
    case SG_LIB_CAT_UNIT_ATTENTION:
    case SG_LIB_CAT_SENSE:
    default:
        n = noisy;
        break;
    }
    if (verbose || n) {
        if (leadin && (strlen(leadin) > 0))
            pr2ws("%s:\n", leadin);
        sg_get_sense_str(NULL, sbp, slen, (verbose > 1),
                         sizeof(b), b);
        pr2ws("%s", b);
        if (req_din_x > 0) {
            if (act_din_x != req_din_x) {
                if ((verbose > 2) || check_data_in || (act_din_x > 0))
                    sg_cmds_resid_print(leadin, true, req_din_x, act_din_x);
                if (act_din_x < 0) {
                    if (verbose)
                        pr2ws("    %s: %s can't get negative bytes, say it "
                              "got none\n", leadin, pass_through_s);
                }
            }
        }
        if (req_dout_x > 0) {
            if (act_dout_x != req_dout_x) {
                if ((verbose > 1) && (act_dout_x > 0))
                    sg_cmds_resid_print(leadin, false, req_dout_x, act_dout_x);
                if (act_dout_x < 0) {
                    if (verbose)
                        pr2ws("    %s: %s can't send negative bytes, say it "
                              "sent none\n", leadin, pass_through_s);
                }
            }
        }
    }
    if (o_sense_cat)
        *o_sense_cat = scat;
    return -2;
}

/* This is a helper function used by sg_cmds_* implementations after the
 * call to the pass-through. pt_res is returned from do_scsi_pt(). If valid
 * sense data is found it is decoded and output to sg_warnings_strm (def:
 * stderr); depending on the 'noisy' and 'verbose' settings. Returns -2 for
 * o_sense_cat (sense category) written which may not be fatal. Returns
 * -1 for other types of failure. Returns 0, or a positive number. If data-in
 * type command (or bidi) then returns actual number of bytes read
 * (din_len - resid); otherwise returns 0. Note that several sense categories
 * also have data in bytes received; -2 is still returned. */
int
sg_cmds_process_resp(struct sg_pt_base * ptvp, const char * leadin,
                     int pt_res, bool noisy, int verbose, int * o_sense_cat)
{
    bool favour_sense;
    int cat, slen, resp_code, sstat, req_din_x, req_dout_x;
    int act_din_x, act_dout_x;
    const uint8_t * sbp;
    char b[1024];

    if (NULL == leadin)
        leadin = "";
    if (pt_res < 0) {
#ifdef SG_LIB_LINUX
        if (verbose)
            pr2ws("%s: %s os error: %s\n", leadin, pass_through_s,
                  safe_strerror(-pt_res));
        if ((-ENXIO == pt_res) && o_sense_cat) {
            if (verbose > 2)
                pr2ws("map ENXIO to SG_LIB_CAT_NOT_READY\n");
            *o_sense_cat = SG_LIB_CAT_NOT_READY;
            return -2;
        } else if (noisy && (0 == verbose))
            pr2ws("%s: %s os error: %s\n", leadin, pass_through_s,
                  safe_strerror(-pt_res));
#else
        if (noisy || verbose)
            pr2ws("%s: %s os error: %s\n", leadin, pass_through_s,
                  safe_strerror(-pt_res));
#endif
        return -1;
    } else if (SCSI_PT_DO_BAD_PARAMS == pt_res) {
        pr2ws("%s: bad %s setup\n", leadin, pass_through_s);
        return -1;
    } else if (SCSI_PT_DO_TIMEOUT == pt_res) {
        pr2ws("%s: %s timeout\n", leadin, pass_through_s);
        return -1;
    }
    if (verbose > 2) {
        uint64_t duration = get_pt_duration_ns(ptvp);

        if (duration > 0)
            pr2ws("      duration=%" PRIu64 " ns\n", duration);
        else {
            int d = get_scsi_pt_duration_ms(ptvp);

            if (d != -1)
                pr2ws("      duration=%u ms\n", (uint32_t)d);
        }
    }
    get_pt_req_lengths(ptvp, &req_din_x, &req_dout_x);
    get_pt_actual_lengths(ptvp, &act_din_x, &act_dout_x);
    slen = get_scsi_pt_sense_len(ptvp);
    sbp = get_scsi_pt_sense_buf(ptvp);
    switch ((cat = get_scsi_pt_result_category(ptvp))) {
    case SCSI_PT_RESULT_GOOD:
        if (sbp && (slen > 7)) {
            resp_code = sbp[0] & 0x7f;
            /* SBC referrals can have status=GOOD and sense_key=COMPLETED */
            if (resp_code >= 0x70) {
                if (resp_code < 0x72) {
                    if (SPC_SK_NO_SENSE != (0xf & sbp[2]))
                        sg_err_category_sense(sbp, slen);
                } else if (resp_code < 0x74) {
                    if (SPC_SK_NO_SENSE != (0xf & sbp[1]))
                        sg_err_category_sense(sbp, slen);
                }
            }
        }
        if (req_din_x > 0) {
            if (act_din_x != req_din_x) {
                if ((verbose > 1) && (act_din_x >= 0))
                    sg_cmds_resid_print(leadin, true, req_din_x, act_din_x);
                if (act_din_x < 0) {
                    if (verbose)
                        pr2ws("    %s: %s can't get negative bytes, say it "
                              "got none\n", leadin, pass_through_s);
                    act_din_x = 0;
                }
            }
        }
        if (req_dout_x > 0) {
            if (act_dout_x != req_dout_x) {
                if ((verbose > 1) && (act_dout_x >= 0))
                    sg_cmds_resid_print(leadin, false, req_dout_x, act_dout_x);
                if (act_dout_x < 0) {
                    if (verbose)
                        pr2ws("    %s: %s can't send negative bytes, say it "
                              "sent none\n", leadin, pass_through_s);
                    act_dout_x = 0;
                }
            }
        }
        return act_din_x;
    case SCSI_PT_RESULT_STATUS: /* other than GOOD and CHECK CONDITION */
        sstat = get_scsi_pt_status_response(ptvp);
        if (o_sense_cat) {
            switch (sstat) {
            case SAM_STAT_RESERVATION_CONFLICT:
                *o_sense_cat = SG_LIB_CAT_RES_CONFLICT;
                return -2;
            case SAM_STAT_CONDITION_MET:
                *o_sense_cat = SG_LIB_CAT_CONDITION_MET;
                return -2;
            case SAM_STAT_BUSY:
                *o_sense_cat = SG_LIB_CAT_BUSY;
                return -2;
            case SAM_STAT_TASK_SET_FULL:
                *o_sense_cat = SG_LIB_CAT_TS_FULL;
                return -2;
            case SAM_STAT_ACA_ACTIVE:
                *o_sense_cat = SG_LIB_CAT_ACA_ACTIVE;
                return -2;
            case SAM_STAT_TASK_ABORTED:
                *o_sense_cat = SG_LIB_CAT_TASK_ABORTED;
                return -2;
            default:
                break;
            }
        }
        if (verbose || noisy) {
            sg_get_scsi_status_str(sstat, sizeof(b), b);
            pr2ws("%s: scsi status: %s\n", leadin, b);
        }
        return -1;
    case SCSI_PT_RESULT_SENSE:
        return sg_cmds_process_helper(leadin, req_din_x, act_din_x,
                                      req_dout_x, act_dout_x, sbp, slen,
                                      noisy, verbose, o_sense_cat);
    case SCSI_PT_RESULT_TRANSPORT_ERR:
        if (verbose || noisy) {
            get_scsi_pt_transport_err_str(ptvp, sizeof(b), b);
            pr2ws("%s: transport: %s\n", leadin, b);
        }
        /* Shall we favour sense data over a transport error (given both) */
#ifdef SG_LIB_LINUX
        favour_sense = false; /* DRIVER_SENSE is not passed through */
#else
        favour_sense = ((SAM_STAT_CHECK_CONDITION ==
                            get_scsi_pt_status_response(ptvp)) && (slen > 0));
#endif
        if (favour_sense)
            return sg_cmds_process_helper(leadin, req_din_x, act_din_x,
                                          req_dout_x, act_dout_x, sbp, slen,
                                          noisy, verbose, o_sense_cat);
        else
            return -1;
    case SCSI_PT_RESULT_OS_ERR:
        if (verbose || noisy) {
            get_scsi_pt_os_err_str(ptvp, sizeof(b), b);
            pr2ws("%s: os: %s\n", leadin, b);
        }
        return -1;
    default:
        pr2ws("%s: unknown %s result category (%d)\n", leadin, pass_through_s,
               cat);
        return -1;
    }
}

bool
sg_cmds_is_nvme(const struct sg_pt_base * ptvp)
{
    return pt_device_is_nvme(ptvp);
}

static struct sg_pt_base *
create_pt_obj(const char * cname)
{
    struct sg_pt_base * ptvp = construct_scsi_pt_obj();
    if (NULL == ptvp)
        pr2ws("%s: out of memory\n", cname);
    return ptvp;
}

static const char * const inquiry_s = "inquiry";


/* Returns 0 on success, while positive values are SG_LIB_CAT_* errors
 * (e.g. SG_LIB_CAT_MALFORMED). If OS error, returns negated errno or -1. */
static int
sg_ll_inquiry_com(struct sg_pt_base * ptvp, int sg_fd, bool cmddt, bool evpd,
                  int pg_op, void * resp, int mx_resp_len, int timeout_secs,
                  int * residp, bool noisy, int verbose)
{
    bool ptvp_given = false;
    bool local_sense = true;
    bool local_cdb = true;
    int res, ret, sense_cat, resid;
    uint8_t inq_cdb[INQUIRY_CMDLEN] = {INQUIRY_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];
    uint8_t * up;

    if (resp == NULL) {
        if (verbose)
            pr2ws("Got NULL `resp` pointer");
        return SG_LIB_CAT_MALFORMED;
    }
    if (cmddt)
        inq_cdb[1] |= 0x2;
    if (evpd)
        inq_cdb[1] |= 0x1;
    inq_cdb[2] = (uint8_t)pg_op;
    /* 16 bit allocation length (was 8, increased in spc3r09, 200209) */
    sg_put_unaligned_be16((uint16_t)mx_resp_len, inq_cdb + 3);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", inquiry_s,
              sg_get_command_str(inq_cdb, INQUIRY_CMDLEN, false, sizeof(b),
                                 b));
    }
    if (resp && (mx_resp_len > 0)) {
        up = (uint8_t *)resp;
        up[0] = 0x7f;   /* defensive prefill */
        if (mx_resp_len > 4)
            up[4] = 0;
    }
    if (timeout_secs <= 0)
        timeout_secs = DEF_PT_TIMEOUT;
    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false; /* N.B. Ignores locally built cdb */
        else
            set_scsi_pt_cdb(ptvp, inq_cdb, sizeof(inq_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, verbose);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, inq_cdb, sizeof(inq_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, -1, timeout_secs, verbose);
    ret = sg_cmds_process_resp(ptvp, inquiry_s, res, noisy, verbose,
                               &sense_cat);
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
    } else if (ret < 4) {
        if (verbose)
            pr2ws("%s: got too few bytes (%d)\n", __func__, ret);
        ret = SG_LIB_CAT_MALFORMED;
    } else
        ret = 0;

    if (resid > 0) {
        if (resid > mx_resp_len) {
            pr2ws("%s resid (%d) should never exceed requested "
                    "len=%d\n", inquiry_s, resid, mx_resp_len);
            if (0 == ret)
                ret = SG_LIB_CAT_MALFORMED;
            goto fini;
        }
        /* zero unfilled section of response buffer, based on resid */
        memset((uint8_t *)resp + (mx_resp_len - resid), 0, resid);
    }
fini:
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

/* Invokes a SCSI INQUIRY command and yields the response. Returns 0 when
 * successful, various SG_LIB_CAT_* positive values, negated errno or
 * -1 -> other errors. The CMDDT field is obsolete in the INQUIRY cdb. */
int
sg_ll_inquiry(int sg_fd, bool cmddt, bool evpd, int pg_op, void * resp,
              int mx_resp_len, bool noisy, int verbose)
{
    return sg_ll_inquiry_com(NULL, sg_fd, cmddt, evpd, pg_op, resp,
                             mx_resp_len, 0 /* timeout_sec */, NULL, noisy,
                             verbose);
}

/* Invokes a SCSI INQUIRY command and yields the response. Returns 0 when
 * successful, various SG_LIB_CAT_* positive values or -1 -> other errors.
 * The CMDDT field is obsolete in the INQUIRY cdb (since spc3r16 in 2003) so
 * an argument to set it has been removed (use the REPORT SUPPORTED OPERATION
 * CODES command instead). Adds the ability to set the command abort timeout
 * and the ability to report the residual count. If timeout_secs is zero
 * or less the default command abort timeout (60 seconds) is used.
 * If residp is non-NULL then the residual value is written where residp
 * points. A residual value of 0 implies mx_resp_len bytes have be written
 * where resp points. If the residual value equals mx_resp_len then no
 * bytes have been written. */
int
sg_ll_inquiry_v2(int sg_fd, bool evpd, int pg_op, void * resp,
                 int mx_resp_len, int timeout_secs, int * residp,
                 bool noisy, int verbose)
{
    return sg_ll_inquiry_com(NULL, sg_fd, false, evpd, pg_op, resp,
                             mx_resp_len, timeout_secs, residp, noisy,
                             verbose);
}

/* Similar to _v2 but takes a pointer to an object (derived from) sg_pt_base.
 * That object is assumed to be constructed and have a device file descriptor
 * associated with it. Caller is responsible for lifetime of ptp. */
int
sg_ll_inquiry_pt(struct sg_pt_base * ptvp, bool evpd, int pg_op, void * resp,
                 int mx_resp_len, int timeout_secs, int * residp, bool noisy,
                 int verbose)
{
    return sg_ll_inquiry_com(ptvp, -1, false, evpd, pg_op, resp, mx_resp_len,
                             timeout_secs, residp, noisy, verbose);
}

/* Yields most of first 36 bytes of a standard INQUIRY (evpd==0) response.
 * Returns 0 when successful, various SG_LIB_CAT_* positive values, negated
 * errno or -1 -> other errors */
int
sg_simple_inquiry(int sg_fd, struct sg_simple_inquiry_resp * inq_data,
                  bool noisy, int verbose)
{
    int ret;
    uint8_t * inq_resp = NULL;
    uint8_t * free_irp = NULL;

    if (inq_data) {
        memset(inq_data, 0, sizeof(* inq_data));
        inq_data->peripheral_qualifier = 0x3;
        inq_data->peripheral_type = 0x1f;
    }
    inq_resp = sg_memalign(SAFE_STD_INQ_RESP_LEN, 0, &free_irp, false);
    if (NULL == inq_resp) {
        pr2ws("%s: out of memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    ret = sg_ll_inquiry_com(NULL, sg_fd, false, false, 0, inq_resp,
                            SAFE_STD_INQ_RESP_LEN, 0, NULL, noisy, verbose);

    if (inq_data && (0 == ret)) {
        inq_data->peripheral_qualifier = (inq_resp[0] >> 5) & 0x7;
        inq_data->peripheral_type = inq_resp[0] & 0x1f;
        inq_data->byte_1 = inq_resp[1];
        inq_data->version = inq_resp[2];
        inq_data->byte_3 = inq_resp[3];
        inq_data->byte_5 = inq_resp[5];
        inq_data->byte_6 = inq_resp[6];
        inq_data->byte_7 = inq_resp[7];
        memcpy(inq_data->vendor, inq_resp + 8, 8);
        memcpy(inq_data->product, inq_resp + 16, 16);
        memcpy(inq_data->revision, inq_resp + 32, 4);
    }
    if (free_irp)
        free(free_irp);
    return ret;
}

/* Similar to sg_simple_inquiry() but takes pointer to pt object rather
 * than device file descriptor. */
int
sg_simple_inquiry_pt(struct sg_pt_base * ptvp,
                     struct sg_simple_inquiry_resp * inq_data,
                     bool noisy, int verbose)
{
    int ret;
    uint8_t * inq_resp = NULL;
    uint8_t * free_irp = NULL;

    if (inq_data) {
        memset(inq_data, 0, sizeof(* inq_data));
        inq_data->peripheral_qualifier = 0x3;
        inq_data->peripheral_type = 0x1f;
    }
    inq_resp = sg_memalign(SAFE_STD_INQ_RESP_LEN, 0, &free_irp, false);
    if (NULL == inq_resp) {
        pr2ws("%s: out of memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    ret = sg_ll_inquiry_com(ptvp, -1, false, false, 0, inq_resp,
                            SAFE_STD_INQ_RESP_LEN, 0, NULL, noisy, verbose);

    if (inq_data && (0 == ret)) {
        inq_data->peripheral_qualifier = (inq_resp[0] >> 5) & 0x7;
        inq_data->peripheral_type = inq_resp[0] & 0x1f;
        inq_data->byte_1 = inq_resp[1];
        inq_data->version = inq_resp[2];
        inq_data->byte_3 = inq_resp[3];
        inq_data->byte_5 = inq_resp[5];
        inq_data->byte_6 = inq_resp[6];
        inq_data->byte_7 = inq_resp[7];
        memcpy(inq_data->vendor, inq_resp + 8, 8);
        memcpy(inq_data->product, inq_resp + 16, 16);
        memcpy(inq_data->revision, inq_resp + 32, 4);
    }
    if (free_irp)
        free(free_irp);
    return ret;
}

/* Invokes a SCSI TEST UNIT READY command.
 * N.B. To access the sense buffer outside this routine then one be
 * provided by the caller.
 * 'pack_id' is just for diagnostics, safe to set to 0.
 * Looks for progress indicator if 'progress' non-NULL;
 * if found writes value [0..65535] else write -1.
 * Returns 0 when successful, various SG_LIB_CAT_* positive values or
 * -1 -> other errors */
static int
sg_ll_test_unit_ready_com(struct sg_pt_base * ptvp, int sg_fd, int pack_id,
                          int * progress, bool noisy, int verbose)
{
    static const char * const tur_s = "test unit ready";
    bool ptvp_given = false;
    bool local_sense = true;
    bool local_cdb = true;
    int res, ret, sense_cat;
    uint8_t tur_cdb[TUR_CMDLEN] = {TUR_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", tur_s,
              sg_get_command_str(tur_cdb, TUR_CMDLEN, false, sizeof(b), b));
    }
    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false; /* N.B. Ignores locally built cdb */
        else
            set_scsi_pt_cdb(ptvp, tur_cdb, sizeof(tur_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, verbose);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, tur_cdb, sizeof(tur_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_packet_id(ptvp, pack_id);
    res = do_scsi_pt(ptvp, -1, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, tur_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret)
        ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    else if (-2 == ret) {
        if (progress) {
            int slen = get_scsi_pt_sense_len(ptvp);

            if (! sg_get_sense_progress_fld(sense_b, slen, progress))
                *progress = -1;
        }
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
sg_ll_test_unit_ready_progress_pt(struct sg_pt_base * ptvp, int pack_id,
                                  int * progress, bool noisy, int verbose)
{
    return sg_ll_test_unit_ready_com(ptvp, -1, pack_id, progress, noisy,
                                     verbose);
}

int
sg_ll_test_unit_ready_progress(int sg_fd, int pack_id, int * progress,
                               bool noisy, int verbose)
{
    return sg_ll_test_unit_ready_com(NULL, sg_fd, pack_id, progress, noisy,
                                     verbose);
}

/* Invokes a SCSI TEST UNIT READY command.
 * 'pack_id' is just for diagnostics, safe to set to 0.
 * Returns 0 when successful, various SG_LIB_CAT_* positive values or
 * -1 -> other errors */
int
sg_ll_test_unit_ready(int sg_fd, int pack_id, bool noisy, int verbose)
{
    return sg_ll_test_unit_ready_com(NULL, sg_fd, pack_id, NULL, noisy,
                                     verbose);
}

int
sg_ll_test_unit_ready_pt(struct sg_pt_base * ptvp, int pack_id, bool noisy,
                         int verbose)
{
    return sg_ll_test_unit_ready_com(ptvp, -1, pack_id, NULL, noisy, verbose);
}

/* Invokes a SCSI REQUEST SENSE command. Returns 0 when successful, various
 * SG_LIB_CAT_* positive values or -1 -> other errors */
static int
sg_ll_request_sense_com(struct sg_pt_base * ptvp, int sg_fd, bool desc,
                        void * resp, int mx_resp_len, bool noisy, int verbose)
{
    bool ptvp_given = false;
    bool local_cdb = true;
    bool local_sense = true;
    int ret, res, sense_cat;
    static const char * const rq_s = "request sense";
    uint8_t rs_cdb[REQUEST_SENSE_CMDLEN] =
        {REQUEST_SENSE_CMD, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    if (desc)
        rs_cdb[1] |= 0x1;
    if (mx_resp_len > 0xff) {
        pr2ws("mx_resp_len cannot exceed 255\n");
        return -1;
    }
    rs_cdb[4] = mx_resp_len & 0xff;
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", rq_s,
              sg_get_command_str(rs_cdb, REQUEST_SENSE_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (ptvp) {
        ptvp_given = true;
        if (get_scsi_pt_sense_buf(ptvp))
            local_cdb = false;
        else
            set_scsi_pt_cdb(ptvp, rs_cdb, sizeof(rs_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, verbose);
        if (NULL == ptvp)
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, rs_cdb, sizeof(rs_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, -1, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, rq_s, res, noisy, verbose, &sense_cat);
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
        if ((mx_resp_len >= 8) && (ret < 8)) {
            if (verbose)
                pr2ws("    %s: got %d bytes in response, too short\n", rq_s,
                      ret);
            ret = -1;
        } else
            ret = 0;
    }
    if (ptvp_given) {
        if (local_sense)        /* stop caller accessing local sense */
        set_scsi_pt_sense(ptvp, NULL, 0);
        if (local_cdb)  /* stop caller accessing local sense */
        set_scsi_pt_cdb(ptvp, NULL, 0);
    } else if (ptvp)
        destruct_scsi_pt_obj(ptvp);
    return ret;
}

int
sg_ll_request_sense(int sg_fd, bool desc, void * resp, int mx_resp_len,
                    bool noisy, int verbose)
{
    return sg_ll_request_sense_com(NULL, sg_fd, desc, resp, mx_resp_len,
                                   noisy, verbose);
}

int
sg_ll_request_sense_pt(struct sg_pt_base * ptvp, bool desc, void * resp,
                       int mx_resp_len, bool noisy, int verbose)
{
    return sg_ll_request_sense_com(ptvp, -1, desc, resp, mx_resp_len,
                                   noisy, verbose);
}

/* Invokes a SCSI REPORT LUNS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors */
static int
sg_ll_report_luns_com(struct sg_pt_base * ptvp, int sg_fd, int select_report,
                      void * resp, int mx_resp_len, bool noisy, int verbose)
{
    static const char * const report_luns_s = "report luns";
    bool ptvp_given = false;
    bool local_cdb = true;
    bool local_sense = true;
    int ret, res, sense_cat;
    uint8_t rl_cdb[REPORT_LUNS_CMDLEN] =
                         {REPORT_LUNS_CMD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN];

    rl_cdb[2] = select_report & 0xff;
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rl_cdb + 6);
    if (verbose) {
        char b[128];

        pr2ws("    %s cdb: %s\n", report_luns_s,
              sg_get_command_str(rl_cdb, REPORT_LUNS_CMDLEN, false,
                                 sizeof(b), b));
    }
    if (ptvp) {
        ptvp_given = true;
        partial_clear_scsi_pt_obj(ptvp);
        if (get_scsi_pt_cdb_buf(ptvp))
            local_cdb = false;
        else
            set_scsi_pt_cdb(ptvp, rl_cdb, sizeof(rl_cdb));
        if (get_scsi_pt_sense_buf(ptvp))
            local_sense = false;
        else
            set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    } else {
        if (NULL == ((ptvp = create_pt_obj(report_luns_s))))
            return sg_convert_errno(ENOMEM);
        set_scsi_pt_cdb(ptvp, rl_cdb, sizeof(rl_cdb));
        set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    }
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, sg_fd, DEF_PT_TIMEOUT, verbose);
    ret = sg_cmds_process_resp(ptvp, report_luns_s, res, noisy, verbose,
                               &sense_cat);
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
        if (local_sense)        /* stop caller accessing local sense */
            set_scsi_pt_sense(ptvp, NULL, 0);
        if (local_cdb)
            set_scsi_pt_cdb(ptvp, NULL, 0);
    } else {
        if (ptvp)
            destruct_scsi_pt_obj(ptvp);
    }
    return ret;
}

/* Invokes a SCSI REPORT LUNS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors,
 * Expects sg_fd to be >= 0 representing an open device fd. */
int
sg_ll_report_luns(int sg_fd, int select_report, void * resp, int mx_resp_len,
                  bool noisy, int verbose)
{
    return sg_ll_report_luns_com(NULL, sg_fd, select_report, resp,
                                 mx_resp_len, noisy, verbose);
}


/* Invokes a SCSI REPORT LUNS command. Return of 0 -> success,
 * various SG_LIB_CAT_* positive values or -1 -> other errors.
 * Expects a non-NULL ptvp containing an open device fd. */
int
sg_ll_report_luns_pt(struct sg_pt_base * ptvp, int select_report,
                     void * resp, int mx_resp_len, bool noisy, int verbose)
{
    return sg_ll_report_luns_com(ptvp, -1, select_report, resp,
                                 mx_resp_len, noisy, verbose);
}
