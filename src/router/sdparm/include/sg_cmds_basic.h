#ifndef SG_CMDS_BASIC_H
#define SG_CMDS_BASIC_H

/*
 * Copyright (c) 2004-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * Error, warning and verbose output is sent to the file pointed to by
 * sg_warnings_strm which is declared in sg_lib.h and can be set with
 * the sg_set_warnings_strm() function. If not given sg_warnings_strm
 * defaults to stderr.
 * If 'noisy' is false and 'verbose' is zero then following functions should
 * not output anything to sg_warnings_strm. If 'noisy' is true and 'verbose'
 * is zero then Unit Attention, Recovered, Medium and Hardware errors (sense
 * keys) send output to sg_warnings_strm. Increasing values of 'verbose'
 * send increasing amounts of (debug) output to sg_warnings_strm.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Functions with the "_pt" suffix take a pointer to an object (derived from)
 * sg_pt_base rather than an open file descriptor as their first argument.
 * That object is assumed to be constructed and have a device file descriptor
 * associated with it. clear_scsi_pt_obj() is called at the start of each
 * "_pt" function. Caller is responsible for lifetime of ptp.
 * If the sense buffer is accessed outside the "_pt" function then the caller
 * must invoke set_scsi_pt_sense() _prior_ to the "_pt" function. Otherwise
 * a sense buffer local to "_pt" function is used.
 * Usually the cdb pointer will be NULL going into the "_pt" functions but
 * could be given by the caller in which case it will used rather than a
 * locally generated one. */

struct sg_pt_base;


/* Invokes a SCSI INQUIRY command and yields the response
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP -> not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_ABORTED_COMMAND, a negated errno or -1 -> other errors */
int sg_ll_inquiry(int sg_fd, bool cmddt, bool evpd, int pg_op, void * resp,
                  int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI INQUIRY command and yields the response. Returns 0 when
 * successful, various SG_LIB_CAT_* positive values, negated error or -1
 * for other errors. The CMDDT field is obsolete in the INQUIRY cdb (since
 * spc3r16 in 2003) so * an argument to set it has been removed (use the
 * REPORT SUPPORTED OPERATION CODES command instead). Adds the ability to
 * set the command abort timeout and the ability to report the residual
 * count. If timeout_secs is zero or less the default command abort timeout
 * (60 seconds) is used. If residp is non-NULL then the residual value is
 * written where residp points. A residual value of 0 implies mx_resp_len
 * bytes have be written where resp points. If the residual value equals
 * mx_resp_len then no bytes have been written. */
int sg_ll_inquiry_v2(int sg_fd, bool evpd, int pg_op, void * resp,
                     int mx_resp_len, int timeout_secs, int * residp,
                     bool noisy, int verbose);

/* Similar to sg_ll_inquiry_v2(). See note above about "_pt" suffix. */
int sg_ll_inquiry_pt(struct sg_pt_base * ptp, bool evpd, int pg_op,
                     void * resp, int mx_resp_len, int timeout_secs,
                     int * residp, bool noisy, int verbose);

/* Invokes a SCSI LOG SELECT command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Log Select not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_ABORTED_COMMAND, * SG_LIB_CAT_NOT_READY -> device not ready,
 * -1 -> other failure */
int sg_ll_log_select(int sg_fd, bool pcr, bool sp, int pc, int pg_code,
                     int subpg_code, uint8_t * paramp, int param_len,
                     bool noisy, int verbose);

/* Invokes a SCSI LOG SENSE command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Log Sense not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_log_sense(int sg_fd, bool ppc, bool sp, int pc, int pg_code,
                    int subpg_code, int paramp, uint8_t * resp,
                    int mx_resp_len, bool noisy, int verbose);

/* Same as sg_ll_log_sense() apart from timeout_secs and residp. See
 * sg_ll_inquiry_v2() for their description */
int sg_ll_log_sense_v2(int sg_fd, bool ppc, bool sp, int pc, int pg_code,
                       int subpg_code, int paramp, uint8_t * resp,
                       int mx_resp_len, int timeout_secs, int * residp,
                       bool noisy, int verbose);

/* Invokes a SCSI MODE SELECT (6) command.  Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_ILLEGAL_REQ ->
 * bad field in cdb, * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_mode_select6(int sg_fd, bool pf, bool sp, void * paramp,
                        int param_len, bool noisy, int verbose);
/* v2 adds RTD (revert to defaults) bit, added in spc5r11 */
int sg_ll_mode_select6_v2(int sg_fd, bool pf, bool rtd, bool sp,
                          void * paramp, int param_len, bool noisy,
                          int verbose);

/* Invokes a SCSI MODE SELECT (10) command.  Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_ILLEGAL_REQ ->
 * bad field in cdb, * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_mode_select10(int sg_fd, bool pf, bool sp, void * paramp,
                        int param_len, bool noisy, int verbose);
/* v2 adds RTD (revert to defaults) bit, added in spc5r11 */
int sg_ll_mode_select10_v2(int sg_fd, bool pf, bool rtd, bool sp,
                           void * paramp, int param_len, bool noisy,
                           int verbose);

/* Invokes a SCSI MODE SENSE (6) command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_ILLEGAL_REQ ->
 * bad field in cdb, * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_mode_sense6(int sg_fd, bool dbd, int pc, int pg_code,
                      int sub_pg_code, void * resp, int mx_resp_len,
                      bool noisy, int verbose);

/* Invokes a SCSI MODE SENSE (10) command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_ILLEGAL_REQ ->
 * bad field in cdb, * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_mode_sense10(int sg_fd, bool llbaa, bool dbd, int pc, int pg_code,
                       int sub_pg_code, void * resp, int mx_resp_len,
                       bool noisy, int verbose);

/* Same as sg_ll_mode_sense10() apart from timeout_secs and residp. See
 * sg_ll_inquiry_v2() for their description */
int sg_ll_mode_sense10_v2(int sg_fd, bool llbaa, bool dbd, int pc,
                          int pg_code, int sub_pg_code, void * resp,
                          int mx_resp_len, int timeout_secs, int * residp,
                          bool noisy, int verbose);

/* Invokes a SCSI PREVENT ALLOW MEDIUM REMOVAL command (SPC-3)
 * prevent==0 allows removal, prevent==1 prevents removal ...
 * Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> command not supported
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_prevent_allow(int sg_fd, int prevent, bool noisy, int verbose);

/* Invokes a SCSI READ CAPACITY (10) command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_UNIT_ATTENTION
 * -> perhaps media changed, SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_readcap_10(int sg_fd, bool pmi, unsigned int lba, void * resp,
                     int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI READ CAPACITY (16) command. Returns 0 -> success,
 * SG_LIB_CAT_UNIT_ATTENTION -> media changed??, SG_LIB_CAT_INVALID_OP
 *  -> cdb not supported, SG_LIB_CAT_IlLEGAL_REQ -> bad field in cdb
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_readcap_16(int sg_fd, bool pmi, uint64_t llba, void * resp,
                     int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI REPORT LUNS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Report Luns not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_NOT_READY (shouldn't happen), -1 -> other failure */
int sg_ll_report_luns(int sg_fd, int select_report, void * resp,
                      int mx_resp_len, bool noisy, int verbose);

/* Similar to sg_ll_report_luns(). See note above about "_pt" suffix. */
int sg_ll_report_luns_pt(struct sg_pt_base * ptp, int select_report,
                         void * resp, int mx_resp_len, bool noisy,
                         int verbose);

/* Invokes a SCSI REQUEST SENSE command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Request Sense not supported??,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_request_sense(int sg_fd, bool desc, void * resp, int mx_resp_len,
                        bool noisy, int verbose);

/* Similar to sg_ll_request_sense(). See note above about "_pt" suffix. */
int sg_ll_request_sense_pt(struct sg_pt_base * ptp, bool desc, void * resp,
                           int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI START STOP UNIT command (SBC + MMC).
 * Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Start stop unit not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure
 * SBC-3 and MMC partially overlap on the power_condition_modifier(sbc) and
 * format_layer_number(mmc) fields. They also overlap on the noflush(sbc)
 * and fl(mmc) one bit field. This is the cause of the awkardly named
 * pc_mod__fl_num and noflush__fl arguments to this function.  */
int sg_ll_start_stop_unit(int sg_fd, bool immed, int pc_mod__fl_num,
                          int power_cond, bool noflush__fl, bool loej,
                          bool start, bool noisy, int verbose);

/* Similar to sg_ll_start_stop_unit(). See note above about "_pt" suffix. */
int sg_ll_start_stop_unit_pt(struct sg_pt_base * ptp, bool immed,
                             int pc_mod__fl_num, int power_cond,
                             bool noflush__fl, bool loej, bool start,
                             bool noisy, int verbose);

/* Invokes a SCSI SYNCHRONIZE CACHE (10) command. Return of 0 -> success,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_INVALID_OP -> cdb not supported,
 * SG_LIB_CAT_IlLEGAL_REQ -> bad field in cdb
 * SG_LIB_CAT_NOT_READY -> device not ready, -1 -> other failure */
int sg_ll_sync_cache_10(int sg_fd, bool sync_nv, bool immed, int group,
                        unsigned int lba, unsigned int count, bool noisy,
                        int verbose);

/* Invokes a SCSI TEST UNIT READY command.
 * 'pack_id' is just for diagnostics, safe to set to 0.
 * Return of 0 -> success, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_ABORTED_COMMAND, -1 -> other failure */
int sg_ll_test_unit_ready(int sg_fd, int pack_id, bool noisy, int verbose);

/* Similar to sg_ll_test_unit_ready(). See note above about "_pt" suffix. */
int sg_ll_test_unit_ready_pt(struct sg_pt_base * ptp, int pack_id,
                             bool noisy, int verbose);

/* Invokes a SCSI TEST UNIT READY command.
 * 'pack_id' is just for diagnostics, safe to set to 0.
 * Looks for progress indicator if 'progress' non-NULL;
 * if found writes value [0..65535] else write -1.
 * Return of 0 -> success, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_ABORTED_COMMAND, SG_LIB_CAT_NOT_READY ->
 * device not ready, -1 -> other failure */
int sg_ll_test_unit_ready_progress(int sg_fd, int pack_id, int * progress,
                                   bool noisy, int verbose);

/* Similar to sg_ll_test_unit_ready_progress(). See note above about "_pt"
 * suffix. */
int sg_ll_test_unit_ready_progress_pt(struct sg_pt_base * ptp, int pack_id,
                                     int * progress, bool noisy, int verbose);


struct sg_simple_inquiry_resp {
    uint8_t peripheral_qualifier;
    uint8_t peripheral_type;
    uint8_t byte_1;             /* was 'rmb' prior to version 1.39 */
                                /* now rmb == !!(0x80 & byte_1) */
    uint8_t version;            /* as per recent drafts: whole of byte 2 */
    uint8_t byte_3;
    uint8_t byte_5;
    uint8_t byte_6;
    uint8_t byte_7;
    char vendor[9];             /* T10 field is 8 bytes, NUL char appended */
    char product[17];
    char revision[5];
};

/* Yields most of first 36 bytes of a standard INQUIRY (evpd==0) response.
 * Returns 0 when successful, SG_LIB_CAT_INVALID_OP -> not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * a negated errno or -1 -> other errors */
int sg_simple_inquiry(int sg_fd, struct sg_simple_inquiry_resp * inq_data,
                      bool noisy, int verbose);

/* Similar to sg_simple_inquiry(). See note above about "_pt" suffix. */
int sg_simple_inquiry_pt(struct sg_pt_base * ptvp,
                         struct sg_simple_inquiry_resp * inq_data, bool noisy,
                         int verbose);

/* MODE SENSE commands yield a response that has header then zero or more
 * block descriptors followed by mode pages. In most cases users are
 * interested in the first mode page. This function returns the (byte)
 * offset of the start of the first mode page. Set mode_sense_6 to true for
 * MODE SENSE (6) and false for MODE SENSE (10). Returns >= 0 is successful
 * or -1 if failure. If there is a failure a message is written to err_buff
 * if it is non-NULL and err_buff_len > 0. */
int sg_mode_page_offset(const uint8_t * resp, int resp_len,
                        bool mode_sense_6, char * err_buff, int err_buff_len);

/* MODE SENSE commands yield a response that has header then zero or more
 * block descriptors followed by mode pages. This functions returns the
 * length (in bytes) of those three components. Note that the return value
 * can exceed resp_len in which case the MODE SENSE command should be
 * re-issued with a larger response buffer. If bd_lenp is non-NULL and if
 * successful the block descriptor length (in bytes) is written to *bd_lenp.
 * Set mode_sense_6 to true for MODE SENSE (6) and false for MODE SENSE (10)
 * responses. Returns -1 if there is an error (e.g. response too short). */
int sg_msense_calc_length(const uint8_t * resp, int resp_len,
                          bool mode_sense_6, int * bd_lenp);

/* Fetches current, changeable, default and/or saveable modes pages as
 * indicated by pcontrol_arr for given pg_code and sub_pg_code. If
 * mode6 is true then use MODE SENSE (6) else use MODE SENSE (10). If
 * flexible true and mode data length seems wrong then try and
 * fix (compensating hack for bad device or driver). pcontrol_arr
 * should have 4 elements for output of current, changeable, default
 * and saved values respectively. Each element should be NULL or
 * at least mx_mpage_len bytes long.
 * Return of 0 -> overall success, SG_LIB_CAT_INVALID_OP -> invalid opcode,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready,
 * SG_LIB_CAT_MALFORMED -> bad response, -1 -> other failure.
 * If success_mask pointer is not NULL then first zeros it. Then set bits
 * 0, 1, 2 and/or 3 if the current, changeable, default and saved values
 * respectively have been fetched. If error on current page
 * then stops and returns that error; otherwise continues if an error is
 * detected but returns the first error encountered.  */
int sg_get_mode_page_controls(int sg_fd, bool mode6, int pg_code,
                              int sub_pg_code, bool dbd, bool flexible,
                              int mx_mpage_len, int * success_mask,
                              void * pcontrol_arr[], int * reported_lenp,
                              int verbose);

/* Returns file descriptor >= 0 if successful. If error in Unix returns
   negated errno. Implementation calls scsi_pt_open_device(). */
int sg_cmds_open_device(const char * device_name, bool read_only, int verbose);

/* Returns file descriptor >= 0 if successful. If error in Unix returns
   negated errno. Implementation calls scsi_pt_open_flags(). */
int sg_cmds_open_flags(const char * device_name, int flags, int verbose);

/* Returns 0 if successful. If error in Unix returns negated errno.
   Implementation calls scsi_pt_close_device(). */
int sg_cmds_close_device(int device_fd);

const char * sg_cmds_version();

#define SG_NO_DATA_IN 0


/* This is a helper function used by sg_cmds_* implementations after the
 * call to the pass-through. pt_res is returned from do_scsi_pt(). If valid
 * sense data is found it is decoded and output to sg_warnings_strm (def:
 * stderr); depending on the 'noisy' and 'verbose' settings. Returns -2 for
 * sense data (may not be fatal), -1 for failed, 0, or a positive number. If
 * 'mx_di_len > 0' then asks pass-through for resid and returns
 * (mx_di_len - resid); otherwise returns 0. So for data-in it should return
 * the actual number of bytes received. For data-out (to device) or no data
 * call with 'mx_di_len' set to 0 or less. If -2 returned then sense category
 * output via 'o_sense_cat' pointer (if not NULL). Note that several sense
 * categories also have data in bytes received; -2 is still returned. */
int sg_cmds_process_resp(struct sg_pt_base * ptvp, const char * leadin,
                         int pt_res, bool noisy, int verbose,
                         int * o_sense_cat);

/* NVMe devices use a different command set. This function will return true
 * if the device associated with 'pvtp' is a NVME device, else it will
 * return false (e.g. for SCSI devices). */
bool sg_cmds_is_nvme(const struct sg_pt_base * ptvp);

#ifdef __cplusplus
}
#endif

#endif
