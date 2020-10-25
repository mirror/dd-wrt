#ifndef SG_CMDS_EXTRA_H
#define SG_CMDS_EXTRA_H

/*
 * Copyright (c) 2004-2018 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note: all functions that have an 'int timeout_secs' argument will use
 * that value if it is > 0. Otherwise they will set an internal default
 * which is currently 60 seconds. This timeout is typically applied in the
 * SCSI stack above the initiator. If it goes off then the SCSI command is
 * aborted and there can be other unwelcome side effects. Note that some
 * commands (e.g. FORMAT UNIT and the Third Party copy commands) can take
 * a lot longer than the default timeout. */

/* Functions with the "_pt" suffix ^^^ take a pointer to an object (derived
 * from) sg_pt_base rather than an open file descriptor as their first
 * argument. That object is assumed to be constructed and have a device file
 * descriptor * associated with it. Caller is responsible for lifetime of
 * ptp.
 *    ^^^ apart from sg_ll_ata_pt() as 'pass-through' is part of its name. */

struct sg_pt_base;


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
int sg_ll_ata_pt(int sg_fd, const uint8_t * cdbp, int cdb_len,
                 int timeout_secs,  void * dinp, void * doutp, int dlen,
                 uint8_t * sensep, int max_sense_len, uint8_t * ata_return_dp,
                 int max_ata_return_len, int * residp, int verbose);

/* Invokes a FORMAT UNIT (SBC-3) command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Format unit not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure. Note that sg_ll_format_unit2() and
 * sg_ll_format_unit_v2() are the same, both add the ffmt argument. */
int sg_ll_format_unit(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                      bool cmplist, int dlist_format, int timeout_secs,
                      void * paramp, int param_len, bool noisy, int verbose);
int sg_ll_format_unit2(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                       bool cmplist, int dlist_format, int ffmt,
                       int timeout_secs, void * paramp, int param_len,
                       bool noisy, int verbose);
int sg_ll_format_unit_v2(int sg_fd, int fmtpinfo, bool longlist, bool fmtdata,
                         bool cmplist, int dlist_format, int ffmt,
                         int timeout_secs, void * paramp, int param_len,
                         bool noisy, int verbose);

/* Invokes a SCSI GET LBA STATUS(16) or GET LBA STATUS(32) command (SBC).
 * Returns 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> GET LBA STATUS(16 or 32) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_NOT_READY -> device not ready, -1 -> other failure.
 * sg_ll_get_lba_status() calls the 16 byte variant with rt=0 . */
int sg_ll_get_lba_status(int sg_fd, uint64_t start_llba, void * resp,
                         int alloc_len, bool noisy, int verbose);
int sg_ll_get_lba_status16(int sg_fd, uint64_t start_llba, uint8_t rt,
                           void * resp, int alloc_len, bool noisy,
                           int verbose);
int sg_ll_get_lba_status32(int sg_fd, uint64_t start_llba, uint32_t scan_len,
                           uint32_t element_id, uint8_t rt,
                           void * resp, int alloc_len, bool noisy,
                           int verbose);

/* Invokes a SCSI PERSISTENT RESERVE IN command (SPC). Returns 0
 * when successful, SG_LIB_CAT_INVALID_OP if command not supported,
 * SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int sg_ll_persistent_reserve_in(int sg_fd, int rq_servact, void * resp,
                                int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI PERSISTENT RESERVE OUT command (SPC). Returns 0
 * when successful, SG_LIB_CAT_INVALID_OP if command not supported,
 * SG_LIB_CAT_ILLEGAL_REQ if field in cdb not supported,
 * SG_LIB_CAT_UNIT_ATTENTION, SG_LIB_CAT_ABORTED_COMMAND, else -1 */
int sg_ll_persistent_reserve_out(int sg_fd, int rq_servact, int rq_scope,
                                 unsigned int rq_type, void * paramp,
                                 int param_len, bool noisy, int verbose);

/* Invokes a SCSI READ BLOCK LIMITS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> READ BLOCK LIMITS not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_NOT_READY (shouldn't happen), -1 -> other failure */
int sg_ll_read_block_limits(int sg_fd, void * resp, int mx_resp_len,
                            bool noisy, int verbose);

/* Invokes a SCSI READ BUFFER command (SPC). Return of 0 ->
 * success, SG_LIB_CAT_INVALID_OP -> invalid opcode,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_read_buffer(int sg_fd, int mode, int buffer_id, int buffer_offset,
                      void * resp, int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI READ DEFECT DATA (10) command (SBC). Return of 0 ->
 * success, SG_LIB_CAT_INVALID_OP -> invalid opcode,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_read_defect10(int sg_fd, bool req_plist, bool req_glist,
                        int dl_format, void * resp, int mx_resp_len,
                        bool noisy, int verbose);

/* Invokes a SCSI READ LONG (10) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> READ LONG(10) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO -> bad field in cdb, with info
 * field written to 'offsetp', SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_read_long10(int sg_fd, bool pblock, bool correct, unsigned int lba,
                      void * resp, int xfer_len, int * offsetp, bool noisy,
                      int verbose);

/* Invokes a SCSI READ LONG (16) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> READ LONG(16) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO -> bad field in cdb, with info
 * field written to 'offsetp', SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 *  -1 -> other failure */
int sg_ll_read_long16(int sg_fd, bool pblock, bool correct, uint64_t llba,
                      void * resp, int xfer_len, int * offsetp, bool noisy,
                      int verbose);

/* Invokes a SCSI READ MEDIA SERIAL NUMBER command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Read media serial number not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_read_media_serial_num(int sg_fd, void * resp, int mx_resp_len,
                                bool noisy, int verbose);

/* Invokes a SCSI REASSIGN BLOCKS command.  Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> invalid opcode, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_NOT_READY -> device not ready, -1 -> other failure */
int sg_ll_reassign_blocks(int sg_fd, bool longlba, bool longlist,
                          void * paramp, int param_len, bool noisy,
                          int verbose);

/* Invokes a SCSI RECEIVE DIAGNOSTIC RESULTS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Receive diagnostic results not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_receive_diag(int sg_fd, bool pcv, int pg_code, void * resp,
                       int mx_resp_len, bool noisy, int verbose);

/* Same as sg_ll_receive_diag() but with added timeout_secs and residp
 * arguments. Adds the ability to set the command abort timeout
 * and the ability to report the residual count. If timeout_secs is zero
 * or less the default command abort timeout (60 seconds) is used.
 * If residp is non-NULL then the residual value is written where residp
 * points. A residual value of 0 implies mx_resp_len bytes have be written
 * where resp points. If the residual value equals mx_resp_len then no
 * bytes have been written. */
int sg_ll_receive_diag_v2(int sg_fd, bool pcv, int pg_code, void * resp,
                          int mx_resp_len, int timeout_secs, int * residp,
                          bool noisy, int verbose);

int sg_ll_receive_diag_pt(struct sg_pt_base * ptp, bool pcv, int pg_code,
                          void * resp, int mx_resp_len, int timeout_secs,
                          int * residp, bool noisy, int verbose);

/* Invokes a SCSI REPORT IDENTIFYING INFORMATION command. This command was
 * called REPORT DEVICE IDENTIFIER prior to spc4r07. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Report identifying information not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_report_id_info(int sg_fd, int itype, void * resp, int max_resp_len,
                         bool noisy, int verbose);

/* Invokes a SCSI REPORT TARGET PORT GROUPS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Report Target Port Groups not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, -1 -> other failure */
int sg_ll_report_tgt_prt_grp(int sg_fd, void * resp, int mx_resp_len,
                             bool noisy, int verbose);
int sg_ll_report_tgt_prt_grp2(int sg_fd, void * resp, int mx_resp_len,
                              bool extended, bool noisy, int verbose);

/* Invokes a SCSI SET TARGET PORT GROUPS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Report Target Port Groups not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, -1 -> other failure */
int sg_ll_set_tgt_prt_grp(int sg_fd, void * paramp, int param_len, bool noisy,
                          int verbose);

/* Invokes a SCSI REPORT REFERRALS command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Report Referrals not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, -1 -> other failure */
int sg_ll_report_referrals(int sg_fd, uint64_t start_llba, bool one_seg,
                           void * resp, int mx_resp_len, bool noisy,
                           int verbose);

/* Invokes a SCSI SEND DIAGNOSTIC command. Foreground, extended self tests can
 * take a long time, if so set long_duration flag in which case the timeout
 * is set to 7200 seconds; if the value of long_duration is > 7200 then that
 * value is taken as the timeout value in seconds. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Send diagnostic not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_send_diag(int sg_fd, int st_code, bool pf_bit, bool st_bit,
                    bool devofl_bit, bool unitofl_bit, int long_duration,
                    void * paramp, int param_len, bool noisy, int verbose);

int sg_ll_send_diag_pt(struct sg_pt_base * ptp, int st_code, bool pf_bit,
                       bool st_bit, bool devofl_bit, bool unitofl_bit,
                       int long_duration, void * paramp, int param_len,
                       bool noisy, int verbose);

/* Invokes a SCSI SET IDENTIFYING INFORMATION command. This command was
 * called SET DEVICE IDENTIFIER prior to spc4r07. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Set identifying information not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_set_id_info(int sg_fd, int itype, void * paramp, int param_len,
                      bool noisy, int verbose);

/* Invokes a SCSI UNMAP (SBC-3) command. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> command not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_UNIT_ATTENTION, -1 -> other failure */
int sg_ll_unmap(int sg_fd, int group_num, int timeout_secs, void * paramp,
                int param_len, bool noisy, int verbose);
/* Invokes a SCSI UNMAP (SBC-3) command. Version 2 adds anchor field
 * (sbc3r22). Otherwise same as sg_ll_unmap() . */
int sg_ll_unmap_v2(int sg_fd, bool anchor, int group_num, int timeout_secs,
                   void * paramp, int param_len, bool noisy, int verbose);

/* Invokes a SCSI VERIFY (10) command (SBC and MMC).
 * Note that 'veri_len' is in blocks while 'data_out_len' is in bytes.
 * Returns of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Verify(10) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_MEDIUM_HARD -> medium or hardware error, no valid info,
 * SG_LIB_CAT_MEDIUM_HARD_WITH_INFO -> as previous, with valid info,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_MISCOMPARE, -1 -> other failure */
int sg_ll_verify10(int sg_fd, int vrprotect, bool dpo, int bytechk,
                   unsigned int lba, int veri_len, void * data_out,
                   int data_out_len, unsigned int * infop, bool noisy,
                   int verbose);

/* Invokes a SCSI VERIFY (16) command (SBC).
 * Note that 'veri_len' is in blocks while 'data_out_len' is in bytes.
 * Returns of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Verify(16) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_MEDIUM_HARD -> medium or hardware error, no valid info,
 * SG_LIB_CAT_MEDIUM_HARD_WITH_INFO -> as previous, with valid info,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * SG_LIB_CAT_MISCOMPARE, -1 -> other failure */
int sg_ll_verify16(int sg_fd, int vrprotect, bool dpo, int bytechk,
                   uint64_t llba, int veri_len, int group_num,
                   void * data_out, int data_out_len, uint64_t * infop,
                   bool noisy, int verbose);

/* Invokes a SCSI WRITE BUFFER command (SPC). Return of 0 ->
 * success, SG_LIB_CAT_INVALID_OP -> invalid opcode,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_write_buffer(int sg_fd, int mode, int buffer_id, int buffer_offset,
                       void * paramp, int param_len, bool noisy, int verbose);

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
                      int verbose);

/* Invokes a SCSI WRITE LONG (10) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> WRITE LONG(10) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO -> bad field in cdb, with info
 * field written to 'offsetp', SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_write_long10(int sg_fd, bool cor_dis, bool wr_uncor, bool pblock,
                       unsigned int lba, void * data_out, int xfer_len,
                       int * offsetp, bool noisy, int verbose);

/* Invokes a SCSI WRITE LONG (16) command (SBC). Note that 'xfer_len'
 * is in bytes. Returns 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> WRITE LONG(16) not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb,
 * SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO -> bad field in cdb, with info
 * field written to 'offsetp', SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_write_long16(int sg_fd, bool cor_dis, bool wr_uncor, bool pblock,
                       uint64_t llba, void * data_out, int xfer_len,
                       int * offsetp, bool noisy, int verbose);

/* Invokes a SPC-3 SCSI RECEIVE COPY RESULTS command. In SPC-4 this function
 * supports all service action variants of the THIRD-PARTY COPY IN opcode.
 * SG_LIB_CAT_INVALID_OP -> Receive copy results not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_receive_copy_results(int sg_fd, int sa, int list_id, void * resp,
                               int mx_resp_len, bool noisy, int verbose);

/* Invokes a SCSI EXTENDED COPY(LID1) command. For EXTENDED COPY(LID4)
 * including POPULATE TOKEN and WRITE USING TOKEN use
 * sg_ll_3party_copy_out().  Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> Extended copy not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_extended_copy(int sg_fd, void * paramp, int param_len, bool noisy,
                        int verbose);

/* Handles various service actions associated with opcode 0x83 which is
 * called THIRD PARTY COPY OUT. These include the EXTENDED COPY(LID4),
 * POPULATE TOKEN and WRITE USING TOKEN commands. Return of 0 -> success,
 * SG_LIB_CAT_INVALID_OP -> opcode 0x83 not supported,
 * SG_LIB_CAT_ILLEGAL_REQ -> bad field in cdb, SG_LIB_CAT_UNIT_ATTENTION,
 * SG_LIB_CAT_NOT_READY -> device not ready, SG_LIB_CAT_ABORTED_COMMAND,
 * -1 -> other failure */
int sg_ll_3party_copy_out(int sg_fd, int sa, unsigned int list_id,
                          int group_num, int timeout_secs, void * paramp,
                          int param_len, bool noisy, int verbose);

/* Invokes a SCSI PRE-FETCH(10), PRE-FETCH(16) or SEEK(10) command (SBC).
 * Returns 0 -> success, 25 (SG_LIB_CAT_CONDITION_MET), various SG_LIB_CAT_*
 * positive values or -1 -> other errors. Note that CONDITION MET status
 * is returned when immed=true and num_blocks can fit in device's cache,
 * somewaht strangely, GOOD status (return 0) is returned if num_blocks
 * cannot fit in device's cache. If do_seek10==true then does a SEEK(10)
 * command with given lba, if that LBA is < 2**32 . Unclear what SEEK(10)
 * does, assume it is like PRE-FETCH. If timeout_secs is 0 (or less) then
 * use DEF_PT_TIMEOUT (60 seconds) as command timeout. */
int sg_ll_pre_fetch_x(int sg_fd, bool do_seek10, bool cdb16, bool immed,
                      uint64_t lba, uint32_t num_blocks, int group_num,
                      int timeout_secs, bool noisy, int verbose);

#ifdef __cplusplus
}
#endif

#endif
