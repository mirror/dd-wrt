/*
 * Copyright (c) 2017-2021 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * The code to use the NVMe Management Interface (MI) SES pass-through
 * was provided by WDC in November 2017.
 */

/*
 * Copyright 2017, Western Digital Corporation
 *
 * Written by Berck Nash
 *
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * Based on the NVM-Express command line utility, which bore the following
 * notice:
 *
 * Copyright (c) 2014-2015, Intel Corporation.
 *
 * Written by Keith Busch <keith.busch@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *                   MA 02110-1301, USA.
 */

/* sg_pt_linux_nvme version 1.15 20210102 */

/* This file contains a small "SPC-only" SNTL to support the SES pass-through
 * of SEND DIAGNOSTIC and RECEIVE DIAGNOSTIC RESULTS through NVME-MI
 * SES Send and SES Receive. */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>      /* to define 'major' */
#ifndef major
#include <sys/types.h>
#endif


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <linux/major.h>

#include "sg_pt.h"
#include "sg_lib.h"
#include "sg_linux_inc.h"
#include "sg_pt_linux.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"

#define SCSI_INQUIRY_OPC     0x12
#define SCSI_REPORT_LUNS_OPC 0xa0
#define SCSI_TEST_UNIT_READY_OPC  0x0
#define SCSI_REQUEST_SENSE_OPC  0x3
#define SCSI_SEND_DIAGNOSTIC_OPC  0x1d
#define SCSI_RECEIVE_DIAGNOSTIC_OPC  0x1c
#define SCSI_MAINT_IN_OPC  0xa3
#define SCSI_READ10_OPC 0x28
#define SCSI_READ16_OPC 0x88
#define SCSI_REP_SUP_OPCS_OPC  0xc
#define SCSI_REP_SUP_TMFS_OPC  0xd
#define SCSI_MODE_SENSE10_OPC  0x5a
#define SCSI_MODE_SELECT10_OPC  0x55
#define SCSI_READ_CAPACITY10_OPC  0x25
#define SCSI_START_STOP_OPC 0x1b
#define SCSI_SYNC_CACHE10_OPC  0x35
#define SCSI_SYNC_CACHE16_OPC  0x91
#define SCSI_VERIFY10_OPC 0x2f
#define SCSI_VERIFY16_OPC 0x8f
#define SCSI_WRITE10_OPC 0x2a
#define SCSI_WRITE16_OPC 0x8a
#define SCSI_WRITE_SAME10_OPC 0x41
#define SCSI_WRITE_SAME16_OPC 0x93
#define SCSI_SERVICE_ACT_IN_OPC  0x9e
#define SCSI_READ_CAPACITY16_SA  0x10
#define SCSI_SA_MSK  0x1f

/* Additional Sense Code (ASC) */
#define NO_ADDITIONAL_SENSE 0x0
#define LOGICAL_UNIT_NOT_READY 0x4
#define LOGICAL_UNIT_COMMUNICATION_FAILURE 0x8
#define UNRECOVERED_READ_ERR 0x11
#define PARAMETER_LIST_LENGTH_ERR 0x1a
#define INVALID_OPCODE 0x20
#define LBA_OUT_OF_RANGE 0x21
#define INVALID_FIELD_IN_CDB 0x24
#define INVALID_FIELD_IN_PARAM_LIST 0x26
#define UA_RESET_ASC 0x29
#define UA_CHANGED_ASC 0x2a
#define TARGET_CHANGED_ASC 0x3f
#define LUNS_CHANGED_ASCQ 0x0e
#define INSUFF_RES_ASC 0x55
#define INSUFF_RES_ASCQ 0x3
#define LOW_POWER_COND_ON_ASC  0x5e     /* ASCQ=0 */
#define POWER_ON_RESET_ASCQ 0x0
#define BUS_RESET_ASCQ 0x2      /* scsi bus reset occurred */
#define MODE_CHANGED_ASCQ 0x1   /* mode parameters changed */
#define CAPACITY_CHANGED_ASCQ 0x9
#define SAVING_PARAMS_UNSUP 0x39
#define TRANSPORT_PROBLEM 0x4b
#define THRESHOLD_EXCEEDED 0x5d
#define LOW_POWER_COND_ON 0x5e
#define MISCOMPARE_VERIFY_ASC 0x1d
#define MICROCODE_CHANGED_ASCQ 0x1      /* with TARGET_CHANGED_ASC */
#define MICROCODE_CHANGED_WO_RESET_ASCQ 0x16
#define PCIE_ERR_ASC 0x4b
#define PCIE_UNSUPP_REQ_ASCQ 0x13

/* NVMe Admin commands */
#define SG_NVME_AD_GET_FEATURE 0xa
#define SG_NVME_AD_SET_FEATURE 0x9
#define SG_NVME_AD_IDENTIFY 0x6         /* similar to SCSI INQUIRY */
#define SG_NVME_AD_MI_RECEIVE 0x1e      /* MI: Management Interface */
#define SG_NVME_AD_MI_SEND 0x1d         /* hmmm, same opcode as SEND DIAG */

/* NVMe NVM (Non-Volatile Memory) commands */
#define SG_NVME_NVM_FLUSH 0x0           /* SCSI SYNCHRONIZE CACHE */
#define SG_NVME_NVM_COMPARE 0x5         /* SCSI VERIFY(BYTCHK=1) */
#define SG_NVME_NVM_READ 0x2
#define SG_NVME_NVM_VERIFY 0xc          /* SCSI VERIFY(BYTCHK=0) */
#define SG_NVME_NVM_WRITE 0x1
#define SG_NVME_NVM_WRITE_ZEROES 0x8    /* SCSI WRITE SAME */

#define SG_NVME_NVM_CDW12_FUA (1 << 30) /* Force Unit Access bit */


#if (HAVE_NVME && (! IGNORE_NVME))

/* This trims given NVMe block device name in Linux (e.g. /dev/nvme0n1p5)
 * to the name of its associated char device (e.g. /dev/nvme0). If this
 * occurs true is returned and the char device name is placed in 'b' (as
 * long as b_len is sufficient). Otherwise false is returned. */
bool
sg_get_nvme_char_devname(const char * nvme_block_devname, uint32_t b_len,
                         char * b)
{
    uint32_t n, tlen;
    const char * cp;
    char buff[8];

    if ((NULL == b) || (b_len < 5))
        return false;   /* degenerate cases */
    cp = strstr(nvme_block_devname, "nvme");
    if (NULL == cp)
        return false;   /* expected to find "nvme" in given name */
    if (1 != sscanf(cp, "nvme%u", &n))
        return false;   /* didn't find valid "nvme<number>" */
    snprintf(buff, sizeof(buff), "%u", n);
    tlen = (cp - nvme_block_devname) + 4 + strlen(buff);
    if ((tlen + 1) > b_len)
        return false;           /* b isn't long enough to fit output */
    memcpy(b, nvme_block_devname, tlen);
    b[tlen] = '\0';
    return true;
}

static void
mk_sense_asc_ascq(struct sg_pt_linux_scsi * ptp, int sk, int asc, int ascq,
                  int vb)
{
    bool dsense = !! ptp->dev_stat.scsi_dsense;
    int n;
    uint8_t * sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;

    ptp->io_hdr.device_status = SAM_STAT_CHECK_CONDITION;
    n = ptp->io_hdr.max_response_len;
    if ((n < 8) || ((! dsense) && (n < 14))) {
        if (vb)
            pr2ws("%s: max_response_len=%d too short, want 14 or more\n",
                  __func__, n);
        return;
    } else
        ptp->io_hdr.response_len = dsense ? n : ((n < 18) ? n : 18);
    memset(sbp, 0, n);
    sg_build_sense_buffer(dsense, sbp, sk, asc, ascq);
    if (vb > 3)
        pr2ws("%s:  [sense_key,asc,ascq]: [0x%x,0x%x,0x%x]\n", __func__, sk,
              asc, ascq);
}

static void
mk_sense_from_nvme_status(struct sg_pt_linux_scsi * ptp, int vb)
{
    bool ok;
    bool dsense = !! ptp->dev_stat.scsi_dsense;
    int n;
    uint8_t sstatus, sk, asc, ascq;
    uint8_t * sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;

    ok = sg_nvme_status2scsi(ptp->nvme_status, &sstatus, &sk, &asc, &ascq);
    if (! ok) { /* can't find a mapping to a SCSI error, so ... */
        sstatus = SAM_STAT_CHECK_CONDITION;
        sk = SPC_SK_ILLEGAL_REQUEST;
        asc = 0xb;
        ascq = 0x0;     /* asc: "WARNING" purposely vague */
    }

    ptp->io_hdr.device_status = sstatus;
    n = ptp->io_hdr.max_response_len;
    if ((n < 8) || ((! dsense) && (n < 14))) {
        pr2ws("%s: sense_len=%d too short, want 14 or more\n", __func__, n);
        return;
    } else
        ptp->io_hdr.response_len = dsense ? n : ((n < 18) ? n : 18);
    memset(sbp, 0, n);
    sg_build_sense_buffer(dsense, sbp, sk, asc, ascq);
    if (dsense && (ptp->nvme_status > 0))
        sg_nvme_desc2sense(sbp, ptp->nvme_stat_dnr, ptp->nvme_stat_more,
                           ptp->nvme_status);
    if (vb > 3)
        pr2ws("%s: [status, sense_key,asc,ascq]: [0x%x, 0x%x,0x%x,0x%x]\n",
              __func__, sstatus, sk, asc, ascq);
}

/* Set in_bit to -1 to indicate no bit position of invalid field */
static void
mk_sense_invalid_fld(struct sg_pt_linux_scsi * ptp, bool in_cdb, int in_byte,
                     int in_bit, int vb)
{
    bool dsense = !! ptp->dev_stat.scsi_dsense;
    int sl, asc, n;
    uint8_t * sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;
    uint8_t sks[4];

    ptp->io_hdr.device_status = SAM_STAT_CHECK_CONDITION;
    asc = in_cdb ? INVALID_FIELD_IN_CDB : INVALID_FIELD_IN_PARAM_LIST;
    n = ptp->io_hdr.max_response_len;
    if ((n < 8) || ((! dsense) && (n < 14))) {
        if (vb)
            pr2ws("%s: max_response_len=%d too short, want 14 or more\n",
                  __func__, n);
        return;
    } else
        ptp->io_hdr.response_len = dsense ? n : ((n < 18) ? n : 18);

    memset(sbp, 0, n);
    sg_build_sense_buffer(dsense, sbp, SPC_SK_ILLEGAL_REQUEST, asc, 0);
    memset(sks, 0, sizeof(sks));
    sks[0] = 0x80;
    if (in_cdb)
        sks[0] |= 0x40;
    if (in_bit >= 0) {
        sks[0] |= 0x8;
        sks[0] |= (0x7 & in_bit);
    }
    sg_put_unaligned_be16(in_byte, sks + 1);
    if (dsense) {
        sl = sbp[7] + 8;
        sbp[7] = sl;
        sbp[sl] = 0x2;
        sbp[sl + 1] = 0x6;
        memcpy(sbp + sl + 4, sks, 3);
    } else
        memcpy(sbp + 15, sks, 3);
    if (vb > 3)
        pr2ws("%s:  [sense_key,asc,ascq]: [0x5,0x%x,0x0] %c byte=%d, bit=%d\n",
              __func__, asc, in_cdb ? 'C' : 'D', in_byte,
              ((in_bit > 0) ? (0x7 & in_bit) : 0));
}

/* Returns 0 for success. Returns SG_LIB_NVME_STATUS if there is non-zero
 * NVMe status (from the completion queue) with the value placed in
 * ptp->nvme_status. If Unix error from ioctl then return negated value
 * (equivalent -errno from basic Unix system functions like open()).
 * CDW0 from the completion queue is placed in ptp->nvme_result in the
 * absence of a Unix error. If time_secs is negative it is treated as
 * a timeout in milliseconds (of abs(time_secs) ). */
static int
sg_nvme_admin_cmd(struct sg_pt_linux_scsi * ptp,
                  struct sg_nvme_passthru_cmd *cmdp, void * dp, bool is_read,
                  int time_secs, int vb)
{
    const uint32_t cmd_len = sizeof(struct sg_nvme_passthru_cmd);
    int res;
    uint32_t n;
    uint16_t sct_sc;
    const uint8_t * up = ((const uint8_t *)cmdp) + SG_NVME_PT_OPCODE;
    char nam[64];

    if (vb)
        sg_get_nvme_opcode_name(*up, true /* ADMIN */, sizeof(nam), nam);
    else
        nam[0] = '\0';
    cmdp->timeout_ms = (time_secs < 0) ? (-time_secs) : (1000 * time_secs);
    ptp->os_err = 0;
    if (vb > 2) {
        pr2ws("NVMe Admin command: %s\n", nam);
        hex2stderr((const uint8_t *)cmdp, cmd_len, 1);
        if ((vb > 3) && (! is_read) && dp) {
            uint32_t len = sg_get_unaligned_le32(up + SG_NVME_PT_DATA_LEN);

            if (len > 0) {
                n = len;
                if ((len < 512) || (vb > 5))
                    pr2ws("\nData-out buffer (%u bytes):\n", n);
                else {
                    pr2ws("\nData-out buffer (first 512 of %u bytes):\n", n);
                    n = 512;
                }
                hex2stderr((const uint8_t *)dp, n, 0);
            }
        }
    }
    res = ioctl(ptp->dev_fd, NVME_IOCTL_ADMIN_CMD, cmdp);
    if (res < 0) {  /* OS error (errno negated) */
        ptp->os_err = -res;
        if (vb > 1) {
            pr2ws("%s: ioctl for %s [0x%x] failed: %s "
                  "(errno=%d)\n", __func__, nam, *up, strerror(-res), -res);
        }
        return res;
    }

    /* Now res contains NVMe completion queue CDW3 31:17 (15 bits) */
    ptp->nvme_result = cmdp->result;
    if ((! ptp->nvme_our_sntl) && ptp->io_hdr.response &&
        (ptp->io_hdr.max_response_len > 3)) {
        /* build 32 byte "sense" buffer */
        uint8_t * sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;
        uint16_t st = (uint16_t)res;

        n = ptp->io_hdr.max_response_len;
        n = (n < 32) ? n : 32;
        memset(sbp, 0 , n);
        ptp->io_hdr.response_len = n;
        sg_put_unaligned_le32(cmdp->result,
                              sbp + SG_NVME_PT_CQ_RESULT);
        if (n > 15) /* LSBit will be 0 (Phase bit) after (st << 1) */
            sg_put_unaligned_le16(st << 1, sbp + SG_NVME_PT_CQ_STATUS_P);
    }
    /* clear upper bits (DNR and More) leaving ((SCT << 8) | SC) */
    sct_sc = 0x7ff & res;       /* 11 bits */
    ptp->nvme_status = sct_sc;
    ptp->nvme_stat_dnr = !!(0x4000 & res);
    ptp->nvme_stat_more = !!(0x2000 & res);
    if (sct_sc) {  /* when non-zero, treat as command error */
        if (vb > 1) {
            char b[80];

            pr2ws("%s: ioctl for %s [0x%x] failed, status: %s [0x%x]\n",
                   __func__, nam, *up,
                  sg_get_nvme_cmd_status_str(sct_sc, sizeof(b), b), sct_sc);
        }
        return SG_LIB_NVME_STATUS;      /* == SCSI_PT_DO_NVME_STATUS */
    }
    if ((vb > 3) && is_read && dp) {
        uint32_t len = sg_get_unaligned_le32(up + SG_NVME_PT_DATA_LEN);

        if (len > 0) {
            n = len;
            if ((len < 1024) || (vb > 5))
                pr2ws("\nData-in buffer (%u bytes):\n", n);
            else {
                pr2ws("\nData-in buffer (first 1024 of %u bytes):\n", n);
                n = 1024;
            }
            hex2stderr((const uint8_t *)dp, n, 0);
        }
    }
    return 0;
}

/* see NVME MI document, NVMSR is NVM Subsystem Report */
static void
sntl_check_enclosure_override(struct sg_pt_linux_scsi * ptp, int vb)
{
    uint8_t * up = ptp->nvme_id_ctlp;
    uint8_t nvmsr;

    if (NULL == up)
        return;
    nvmsr = up[253];
    if (vb > 3)
        pr2ws("%s: enter, nvmsr=%u\n", __func__, nvmsr);
    ptp->dev_stat.id_ctl253 = nvmsr;
    switch (ptp->dev_stat.enclosure_override) {
    case 0x0:       /* no override */
        if (0x3 & nvmsr) {
            ptp->dev_stat.pdt = PDT_DISK;
            ptp->dev_stat.enc_serv = 1;
        } else if (0x2 & nvmsr) {
            ptp->dev_stat.pdt = PDT_SES;
            ptp->dev_stat.enc_serv = 1;
        } else if (0x1 & nvmsr) {
            ptp->dev_stat.pdt = PDT_DISK;
            ptp->dev_stat.enc_serv = 0;
        } else {
            uint32_t nn = sg_get_unaligned_le32(up + 516);

            ptp->dev_stat.pdt = nn ? PDT_DISK : PDT_UNKNOWN;
            ptp->dev_stat.enc_serv = 0;
        }
        break;
    case 0x1:       /* override to SES device */
        ptp->dev_stat.pdt = PDT_SES;
        ptp->dev_stat.enc_serv = 1;
        break;
    case 0x2:       /* override to disk with attached SES device */
        ptp->dev_stat.pdt = PDT_DISK;
        ptp->dev_stat.enc_serv = 1;
        break;
    case 0x3:       /* override to SAFTE device (PDT_PROCESSOR) */
        ptp->dev_stat.pdt = PDT_PROCESSOR;
        ptp->dev_stat.enc_serv = 1;
        break;
    case 0xff:      /* override to normal disk */
        ptp->dev_stat.pdt = PDT_DISK;
        ptp->dev_stat.enc_serv = 0;
        break;
    default:
        pr2ws("%s: unknown enclosure_override value: %d\n", __func__,
              ptp->dev_stat.enclosure_override);
        break;
    }
}

static int
sntl_do_identify(struct sg_pt_linux_scsi * ptp, int cns, int nsid,
                 int time_secs, int u_len, uint8_t * up, int vb)
{
    struct sg_nvme_passthru_cmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = SG_NVME_AD_IDENTIFY;
    cmd.nsid = nsid;
    cmd.cdw10 = cns;
    cmd.addr = (uint64_t)(sg_uintptr_t)up;
    cmd.data_len = u_len;
    return sg_nvme_admin_cmd(ptp, &cmd, up, true, time_secs, vb);
}

/* Currently only caches associated identify controller response (4096 bytes).
 * Returns 0 on success; otherwise a positive value is returned */
static int
sntl_cache_identify(struct sg_pt_linux_scsi * ptp, int time_secs, int vb)
{
    int ret;
    uint32_t pg_sz = sg_get_page_size();
    uint8_t * up;

    up = sg_memalign(pg_sz, pg_sz, &ptp->free_nvme_id_ctlp, false);
    ptp->nvme_id_ctlp = up;
    if (NULL == up) {
        pr2ws("%s: sg_memalign() failed to get memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    ret = sntl_do_identify(ptp, 0x1 /* CNS */, 0 /* nsid */, time_secs,
                           pg_sz, up, vb);
    if (0 == ret)
        sntl_check_enclosure_override(ptp, vb);
    return (ret < 0) ? sg_convert_errno(-ret) : ret;
}

/* If nsid==0 then set cmdp->nsid to SG_NVME_BROADCAST_NSID. */
static int
sntl_get_features(struct sg_pt_linux_scsi * ptp, int feature_id, int select,
                  uint32_t nsid, uint64_t din_addr, int time_secs, int vb)
{
    int res;
    struct sg_nvme_passthru_cmd cmd;
    struct sg_nvme_passthru_cmd * cmdp = &cmd;

    if (vb > 4)
        pr2ws("%s: feature_id=0x%x, select=%d\n", __func__, feature_id,
              select);
    memset(cmdp, 0, sizeof(*cmdp));
    cmdp->opcode = SG_NVME_AD_GET_FEATURE;
    cmdp->nsid = nsid ? nsid : SG_NVME_BROADCAST_NSID;
    select &= 0x7;
    feature_id &= 0xff;
    cmdp->cdw10 = (select << 8) | feature_id;
    if (din_addr)
        cmdp->addr = din_addr;
    cmdp->timeout_ms = (time_secs < 0) ? 0 : (1000 * time_secs);
    res = sg_nvme_admin_cmd(ptp, cmdp, NULL, false, time_secs, vb);
    if (res)
        return res;
    ptp->os_err = 0;
    ptp->nvme_status = 0;
    return 0;
}

static const char * nvme_scsi_vendor_str = "NVMe    ";
static const uint16_t inq_resp_len = 36;

static int
sntl_inq(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp, int time_secs,
         int vb)
{
    bool evpd;
    bool cp_id_ctl = false;
    int res;
    uint16_t n, alloc_len, pg_cd;
    uint32_t pg_sz = sg_get_page_size();
    uint8_t * nvme_id_ns = NULL;
    uint8_t * free_nvme_id_ns = NULL;
    uint8_t inq_dout[256];

    if (vb > 3)
        pr2ws("%s: time_secs=%d\n", __func__, time_secs);

    if (0x2 & cdbp[1]) {        /* Reject CmdDt=1 */
        mk_sense_invalid_fld(ptp, true, 1, 1, vb);
        return 0;
    }
    if (NULL == ptp->nvme_id_ctlp) {
        res = sntl_cache_identify(ptp, time_secs, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else if (res) /* should be negative errno */
            return res;
    }
    memset(inq_dout, 0, sizeof(inq_dout));
    alloc_len = sg_get_unaligned_be16(cdbp + 3);
    evpd = !!(0x1 & cdbp[1]);
    pg_cd = cdbp[2];
    if (evpd) {         /* VPD page responses */
        switch (pg_cd) {
        case 0:
            /* inq_dout[0] = (PQ=0)<<5 | (PDT=0); prefer pdt=0xd --> SES */
            inq_dout[1] = pg_cd;
            n = 11;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[4] = 0x0;
            inq_dout[5] = 0x80;
            inq_dout[6] = 0x83;
            inq_dout[7] = 0x86;
            inq_dout[8] = 0x87;
            inq_dout[9] = 0x92;
            inq_dout[n - 1] = SG_NVME_VPD_NICR;     /* last VPD number */
            break;
        case 0x80:
            /* inq_dout[0] = (PQ=0)<<5 | (PDT=0); prefer pdt=0xd --> SES */
            inq_dout[1] = pg_cd;
            n = 24;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            memcpy(inq_dout + 4, ptp->nvme_id_ctlp + 4, 20);    /* SN */
            break;
        case 0x83:
            if ((ptp->nvme_nsid > 0) &&
                (ptp->nvme_nsid < SG_NVME_BROADCAST_NSID)) {
                nvme_id_ns = sg_memalign(pg_sz, pg_sz, &free_nvme_id_ns,
                                         false);
                if (nvme_id_ns) {
                    /* CNS=0x0 Identify namespace */
                    res = sntl_do_identify(ptp, 0x0, ptp->nvme_nsid,
                                           time_secs, pg_sz, nvme_id_ns, vb);
                    if (res) {
                        free(free_nvme_id_ns);
                        free_nvme_id_ns = NULL;
                        nvme_id_ns = NULL;
                    }
                }
            }
            n = sg_make_vpd_devid_for_nvme(ptp->nvme_id_ctlp, nvme_id_ns,
                                           0 /* pdt */, -1 /*tproto */,
                                           inq_dout, sizeof(inq_dout));
            if (n > 3)
                sg_put_unaligned_be16(n - 4, inq_dout + 2);
            if (free_nvme_id_ns) {
                free(free_nvme_id_ns);
                free_nvme_id_ns = NULL;
                nvme_id_ns = NULL;
            }
            break;
        case 0x86:      /* Extended INQUIRY (per SFS SPC Discovery 2016) */
            inq_dout[1] = pg_cd;
            n = 64;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[5] = 0x1;          /* SIMPSUP=1 */
            inq_dout[7] = 0x1;          /* LUICLR=1 */
            inq_dout[13] = 0x40;        /* max supported sense data length */
            break;
        case 0x87:      /* Mode page policy (per SFS SPC Discovery 2016) */
            inq_dout[1] = pg_cd;
            n = 8;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[4] = 0x3f;         /* all mode pages */
            inq_dout[5] = 0xff;         /*     and their sub-pages */
            inq_dout[6] = 0x80;         /* MLUS=1, policy=shared */
            break;
        case 0x92:      /* SCSI Feature set: only SPC Discovery 2016 */
            inq_dout[1] = pg_cd;
            n = 10;
            sg_put_unaligned_be16(n - 4, inq_dout + 2);
            inq_dout[9] = 0x1;  /* SFS SPC Discovery 2016 */
            break;
        case SG_NVME_VPD_NICR:  /* 0xde (vendor (sg3_utils) specific) */
            inq_dout[1] = pg_cd;
            sg_put_unaligned_be16((16 + 4096) - 4, inq_dout + 2);
            n = 16 + 4096;
            cp_id_ctl = true;
            break;
        default:        /* Point to page_code field in cdb */
            mk_sense_invalid_fld(ptp, true, 2, 7, vb);
            return 0;
        }
        if (alloc_len > 0) {
            n = (alloc_len < n) ? alloc_len : n;
            n = (n < ptp->io_hdr.din_xfer_len) ? n : ptp->io_hdr.din_xfer_len;
            ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - n;
            if (n > 0) {
                uint8_t * dp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp;

                if (cp_id_ctl) {
                    memcpy(dp, inq_dout, (n < 16 ? n : 16));
                    if (n > 16)
                        memcpy(dp + 16, ptp->nvme_id_ctlp, n - 16);
                } else
                    memcpy(dp, inq_dout, n);
            }
        }
    } else {            /* Standard INQUIRY response */
        /* pdt=0 --> disk; pdt=0xd --> SES; pdt=3 --> processor (safte) */
        inq_dout[0] = (0x1f & ptp->dev_stat.pdt);  /* (PQ=0)<<5 */
        /* inq_dout[1] = (RMD=0)<<7 | (LU_CONG=0)<<6 | (HOT_PLUG=0)<<4; */
        inq_dout[2] = 6;   /* version: SPC-4 */
        inq_dout[3] = 2;   /* NORMACA=0, HISUP=0, response data format: 2 */
        inq_dout[4] = 31;  /* so response length is (or could be) 36 bytes */
        inq_dout[6] = ptp->dev_stat.enc_serv ? 0x40 : 0;
        inq_dout[7] = 0x2;    /* CMDQUE=1 */
        memcpy(inq_dout + 8, nvme_scsi_vendor_str, 8);  /* NVMe not Intel */
        memcpy(inq_dout + 16, ptp->nvme_id_ctlp + 24, 16); /* Prod <-- MN */
        memcpy(inq_dout + 32, ptp->nvme_id_ctlp + 64, 4);  /* Rev <-- FR */
        if (alloc_len > 0) {
            n = (alloc_len < inq_resp_len) ? alloc_len : inq_resp_len;
            n = (n < ptp->io_hdr.din_xfer_len) ? n : ptp->io_hdr.din_xfer_len;
            ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - n;
            if (n > 0)
                memcpy((uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp,
                       inq_dout, n);
        }
    }
    return 0;
}

static int
sntl_rluns(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp, int time_secs,
           int vb)
{
    int res;
    uint16_t sel_report;
    uint32_t alloc_len, k, n, num, max_nsid;
    uint8_t * rl_doutp;
    uint8_t * up;

    if (vb > 3)
        pr2ws("%s: time_secs=%d\n", __func__, time_secs);

    sel_report = cdbp[2];
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (NULL == ptp->nvme_id_ctlp) {
        res = sntl_cache_identify(ptp, time_secs, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else if (res)
            return res;
    }
    max_nsid = sg_get_unaligned_le32(ptp->nvme_id_ctlp + 516);
    switch (sel_report) {
    case 0:
    case 2:
        num = max_nsid;
        break;
    case 1:
    case 0x10:
    case 0x12:
        num = 0;
        break;
    case 0x11:
        num = (1 == ptp->nvme_nsid) ? max_nsid :  0;
        break;
    default:
        if (vb > 1)
            pr2ws("%s: bad select_report value: 0x%x\n", __func__,
                  sel_report);
        mk_sense_invalid_fld(ptp, true, 2, 7, vb);
        return 0;
    }
    rl_doutp = (uint8_t *)calloc(num + 1, 8);
    if (NULL == rl_doutp) {
        pr2ws("%s: calloc() failed to get memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    for (k = 0, up = rl_doutp + 8; k < num; ++k, up += 8)
        sg_put_unaligned_be16(k, up);
    n = num * 8;
    sg_put_unaligned_be32(n, rl_doutp);
    n+= 8;
    if (alloc_len > 0) {
        n = (alloc_len < n) ? alloc_len : n;
        n = (n < ptp->io_hdr.din_xfer_len) ? n : ptp->io_hdr.din_xfer_len;
        ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - n;
        if (n > 0)
            memcpy((uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp, rl_doutp,
                   n);
    }
    res = 0;
    free(rl_doutp);
    return res;
}

static int
sntl_tur(struct sg_pt_linux_scsi * ptp, int time_secs, int vb)
{
    int res;
    uint32_t pow_state;

    if (vb > 4)
        pr2ws("%s: start\n", __func__);
    if (NULL == ptp->nvme_id_ctlp) {
        res = sntl_cache_identify(ptp, time_secs, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else if (res)
            return res;
    }
    res = sntl_get_features(ptp, 2 /* Power Management */, 0 /* current */,
                            0, 0, time_secs, vb);
    if (0 != res) {
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else
            return res;
    }
    pow_state = (0x1f & ptp->nvme_result);
    if (vb > 3)
        pr2ws("%s: pow_state=%u\n", __func__, pow_state);
#if 0   /* pow_state bounces around too much on laptop */
    if (pow_state)
        mk_sense_asc_ascq(ptp, SPC_SK_NOT_READY, LOW_POWER_COND_ON_ASC, 0,
                          vb);
#endif
    return 0;
}

static int
sntl_req_sense(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
               int time_secs, int vb)
{
    bool desc;
    int res;
    uint32_t pow_state, alloc_len, n;
    uint8_t rs_dout[64];

    if (vb > 3)
        pr2ws("%s: time_secs=%d\n", __func__, time_secs);
    if (NULL == ptp->nvme_id_ctlp) {
        res = sntl_cache_identify(ptp, time_secs, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else if (res)
            return res;
    }
    desc = !!(0x1 & cdbp[1]);
    alloc_len = cdbp[4];
    res = sntl_get_features(ptp, 0x2 /* Power Management */, 0 /* current */,
                            0, 0, time_secs, vb);
    if (0 != res) {
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else
            return res;
    }
    ptp->io_hdr.response_len = 0;
    pow_state = (0x1f & ptp->nvme_result);
    if (vb > 3)
        pr2ws("%s: pow_state=%u\n", __func__, pow_state);
    memset(rs_dout, 0, sizeof(rs_dout));
    if (pow_state)
        sg_build_sense_buffer(desc, rs_dout, SPC_SK_NO_SENSE,
                              LOW_POWER_COND_ON_ASC, 0);
    else
        sg_build_sense_buffer(desc, rs_dout, SPC_SK_NO_SENSE,
                              NO_ADDITIONAL_SENSE, 0);
    n = desc ? 8 : 18;
    n = (n < alloc_len) ? n : alloc_len;
    n = (n < ptp->io_hdr.din_xfer_len) ? n : ptp->io_hdr.din_xfer_len;
    ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - n;
    if (n > 0)
        memcpy((uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp, rs_dout, n);
    return 0;
}

static uint8_t pc_t10_2_select[] = {0, 3, 1, 2};

/* For MODE SENSE(10) and MODE SELECT(10). 6 byte variants not supported */
static int
sntl_mode_ss(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
             int time_secs, int vb)
{
    bool is_msense = (SCSI_MODE_SENSE10_OPC == cdbp[0]);
    int res, n, len;
    uint8_t * bp;
    struct sg_sntl_result_t sntl_result;

    if (vb > 3)
        pr2ws("%s: mode se%s\n", __func__, (is_msense ? "nse" : "lect"));
    if (NULL == ptp->nvme_id_ctlp) {
        res = sntl_cache_identify(ptp, time_secs, vb);
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else if (res)
            return res;
    }
    if (is_msense) {    /* MODE SENSE(10) */
        uint8_t pc_t10 = (cdbp[2] >> 6) & 0x3;
        int mp_t10 = (cdbp[2] & 0x3f);

        if ((0x3f == mp_t10) || (0x8 /* caching mpage */ == mp_t10)) {
            /* 0x6 is "Volatile write cache" feature id */
            res = sntl_get_features(ptp, 0x6, pc_t10_2_select[pc_t10], 0,
                                    0, time_secs, vb);
            if (0 != res) {
                if (SG_LIB_NVME_STATUS == res) {
                    mk_sense_from_nvme_status(ptp, vb);
                    return 0;
                } else
                    return res;
            }
            ptp->dev_stat.wce = !!(0x1 & ptp->nvme_result);
        }
        len = ptp->io_hdr.din_xfer_len;
        bp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp;
        n = sntl_resp_mode_sense10(&ptp->dev_stat, cdbp, bp, len,
                                   &sntl_result);
        ptp->io_hdr.din_resid = (n >= 0) ? len - n : len;
    } else {            /* MODE SELECT(10) */
        bool sp = !!(0x1 & cdbp[1]);    /* Save Page indication */
        uint8_t pre_enc_ov = ptp->dev_stat.enclosure_override;

        len = ptp->io_hdr.dout_xfer_len;
        bp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
        ptp->dev_stat.wce_changed = false;
        n = sntl_resp_mode_select10(&ptp->dev_stat, cdbp, bp, len,
                                    &sntl_result);
        if (ptp->dev_stat.wce_changed) {
            uint32_t nsid = ptp->nvme_nsid;
            struct sg_nvme_passthru_cmd cmd;
            struct sg_nvme_passthru_cmd * cmdp = &cmd;

            ptp->dev_stat.wce_changed = false;
            memset(cmdp, 0, sizeof(*cmdp));
            cmdp->opcode = SG_NVME_AD_SET_FEATURE;
            cmdp->nsid = nsid ? nsid : SG_NVME_BROADCAST_NSID;
            cmdp->cdw10 = 0x6; /* "Volatile write cache" feature id */
            if (sp)
                cmdp->cdw10 |= (1 << 31);
            cmdp->cdw11 = (uint32_t)ptp->dev_stat.wce;
            cmdp->timeout_ms = (time_secs < 0) ? 0 : (1000 * time_secs);
            res = sg_nvme_admin_cmd(ptp, cmdp, NULL, false, time_secs, vb);
            if (0 != res) {
                if (SG_LIB_NVME_STATUS == res) {
                    mk_sense_from_nvme_status(ptp, vb);
                    return 0;
                } else
                    return res;
            }
            ptp->os_err = 0;
            ptp->nvme_status = 0;
        }
        if (pre_enc_ov != ptp->dev_stat.enclosure_override)
            sntl_check_enclosure_override(ptp, vb);  /* ENC_OV has changed */
    }
    if (n < 0) {
        int in_bit = (255 == sntl_result.in_bit) ? (int)sntl_result.in_bit :
                                                   -1;
        if ((SAM_STAT_CHECK_CONDITION == sntl_result.sstatus) &&
            (SPC_SK_ILLEGAL_REQUEST == sntl_result.sk)) {
            if (INVALID_FIELD_IN_CDB == sntl_result.asc)
                mk_sense_invalid_fld(ptp, true, sntl_result.in_byte, in_bit,
                                     vb);
            else if (INVALID_FIELD_IN_PARAM_LIST == sntl_result.asc)
                mk_sense_invalid_fld(ptp, false, sntl_result.in_byte, in_bit,
                                     vb);
            else
                mk_sense_asc_ascq(ptp, sntl_result.sk, sntl_result.asc,
                                  sntl_result.ascq, vb);
        } else
            pr2ws("%s: error but no sense?? n=%d\n", __func__, n);
    }
    return 0;
}

/* This is not really a SNTL. For SCSI SEND DIAGNOSTIC(PF=1) NVMe-MI
 * has a special command (SES Send) to tunnel through pages to an
 * enclosure. The NVMe enclosure is meant to understand the SES
 * (SCSI Enclosure Services) use of diagnostics pages that are
 * related to SES. */
static int
sntl_senddiag(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
              int time_secs, int vb)
{
    bool pf, self_test;
    int res;
    uint8_t st_cd, dpg_cd;
    uint32_t alloc_len, n, dout_len, dpg_len, nvme_dst;
    const uint32_t pg_sz = sg_get_page_size();
    uint8_t * dop;
    struct sg_nvme_passthru_cmd cmd;
    uint8_t * cmd_up = (uint8_t *)&cmd;

    st_cd = 0x7 & (cdbp[1] >> 5);
    self_test = !! (0x4 & cdbp[1]);
    pf = !! (0x10 & cdbp[1]);
    if (vb > 3)
        pr2ws("%s: pf=%d, self_test=%d (st_code=%d)\n", __func__, (int)pf,
              (int)self_test, (int)st_cd);
    if (self_test || st_cd) {
        memset(cmd_up, 0, sizeof(cmd));
        cmd_up[SG_NVME_PT_OPCODE] = 0x14;   /* Device self-test */
        /* just this namespace (if there is one) and controller */
        sg_put_unaligned_le32(ptp->nvme_nsid, cmd_up + SG_NVME_PT_NSID);
        switch (st_cd) {
        case 0: /* Here if self_test is set, do short self-test */
        case 1: /* Background short */
        case 5: /* Foreground short */
            nvme_dst = 1;
            break;
        case 2: /* Background extended */
        case 6: /* Foreground extended */
            nvme_dst = 2;
            break;
        case 4: /* Abort self-test */
            nvme_dst = 0xf;
            break;
        default:
            pr2ws("%s: bad self-test code [0x%x]\n", __func__, st_cd);
            mk_sense_invalid_fld(ptp, true, 1, 7, vb);
            return 0;
        }
        sg_put_unaligned_le32(nvme_dst, cmd_up + SG_NVME_PT_CDW10);
        res = sg_nvme_admin_cmd(ptp, &cmd, NULL, false, time_secs, vb);
        if (0 != res) {
            if (SG_LIB_NVME_STATUS == res) {
                mk_sense_from_nvme_status(ptp, vb);
                return 0;
            } else
                return res;
        }
    }
    alloc_len = sg_get_unaligned_be16(cdbp + 3); /* parameter list length */
    dout_len = ptp->io_hdr.dout_xfer_len;
    if (pf) {
        if (0 == alloc_len) {
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: PF bit set bit param_list_len=0\n", __func__);
            return 0;
        }
    } else {    /* PF bit clear */
        if (alloc_len) {
            mk_sense_invalid_fld(ptp, true, 3, 7, vb);
            if (vb)
                pr2ws("%s: param_list_len>0 but PF clear\n", __func__);
            return 0;
        } else
            return 0;     /* nothing to do */
    }
    if (dout_len < 4) {
        if (vb)
            pr2ws("%s: dout length (%u bytes) too short\n", __func__,
                  dout_len);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    n = dout_len;
    n = (n < alloc_len) ? n : alloc_len;
    dop = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
    if (! sg_is_aligned(dop, pg_sz)) {  /* is dop page aligned ? */
        if (vb)
            pr2ws("%s: dout [0x%" PRIx64 "] not page aligned\n", __func__,
                  (uint64_t)ptp->io_hdr.dout_xferp);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    dpg_cd = dop[0];
    dpg_len = sg_get_unaligned_be16(dop + 2) + 4;
    /* should we allow for more than one D_PG is dout ?? */
    n = (n < dpg_len) ? n : dpg_len;    /* not yet ... */

    if (vb)
        pr2ws("%s: passing through d_pg=0x%x, len=%u to NVME_MI SES send\n",
              __func__, dpg_cd, dpg_len);
    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = SG_NVME_AD_MI_SEND;
    cmd.addr = (uint64_t)(sg_uintptr_t)dop;
    cmd.data_len = 0x1000;   /* NVMe 4k page size. Maybe determine this? */
                             /* dout_len > 0x1000, is this a problem?? */
    cmd.cdw10 = 0x0804;      /* NVMe Message Header */
    cmd.cdw11 = 0x9;         /* nvme_mi_ses_send; (0x8 -> mi_ses_recv) */
    cmd.cdw13 = n;
    res = sg_nvme_admin_cmd(ptp, &cmd, dop, false, time_secs, vb);
    if (0 != res) {
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        }
    }
    return res;
}

/* This is not really a SNTL. For SCSI RECEIVE DIAGNOSTIC RESULTS(PCV=1)
 * NVMe-MI has a special command (SES Receive) to read pages through a
 * tunnel from an enclosure. The NVMe enclosure is meant to understand the
 * SES (SCSI Enclosure Services) use of diagnostics pages that are
 * related to SES. */
static int
sntl_recvdiag(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
              int time_secs, int vb)
{
    bool pcv;
    int res;
    uint8_t dpg_cd;
    uint32_t alloc_len, n, din_len;
    uint32_t pg_sz = sg_get_page_size();
    uint8_t * dip;
    struct sg_nvme_passthru_cmd cmd;

    pcv = !! (0x1 & cdbp[1]);
    dpg_cd = cdbp[2];
    alloc_len = sg_get_unaligned_be16(cdbp + 3); /* parameter list length */
    if (vb > 3)
        pr2ws("%s: dpg_cd=0x%x, pcv=%d, alloc_len=0x%x\n", __func__,
              dpg_cd, (int)pcv, alloc_len);
    din_len = ptp->io_hdr.din_xfer_len;
    n = din_len;
    n = (n < alloc_len) ? n : alloc_len;
    dip = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp;
    if (! sg_is_aligned(dip, pg_sz)) {
        if (vb)
            pr2ws("%s: din [0x%" PRIx64 "] not page aligned\n", __func__,
                  (uint64_t)ptp->io_hdr.din_xferp);
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if (vb)
        pr2ws("%s: expecting d_pg=0x%x from NVME_MI SES receive\n", __func__,
              dpg_cd);
    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = SG_NVME_AD_MI_RECEIVE;
    cmd.addr = (uint64_t)(sg_uintptr_t)dip;
    cmd.data_len = 0x1000;   /* NVMe 4k page size. Maybe determine this? */
                             /* din_len > 0x1000, is this a problem?? */
    cmd.cdw10 = 0x0804;      /* NVMe Message Header */
    cmd.cdw11 = 0x8;         /* nvme_mi_ses_receive */
    cmd.cdw12 = dpg_cd;
    cmd.cdw13 = n;
    res = sg_nvme_admin_cmd(ptp, &cmd, dip, true, time_secs, vb);
    if (0 != res) {
        if (SG_LIB_NVME_STATUS == res) {
            mk_sense_from_nvme_status(ptp, vb);
            return 0;
        } else
            return res;
    }
    ptp->io_hdr.din_resid = din_len - n;
    return res;
}

#define F_SA_LOW                0x80    /* cdb byte 1, bits 4 to 0 */
#define F_SA_HIGH               0x100   /* as used by variable length cdbs */
#define FF_SA (F_SA_HIGH | F_SA_LOW)
#define F_INV_OP                0x200

static int
sntl_rep_opcodes(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
                 int time_secs, int vb)
{
    bool rctd;
    uint8_t reporting_opts, req_opcode, supp;
    uint16_t req_sa, u;
    uint32_t alloc_len, offset, a_len;
    uint32_t pg_sz = sg_get_page_size();
    int k, len, count, bump;
    const struct sg_opcode_info_t *oip;
    uint8_t *arr;
    uint8_t *free_arr;

    if (vb > 3)
        pr2ws("%s: time_secs=%d\n", __func__, time_secs);
    rctd = !!(cdbp[2] & 0x80);      /* report command timeout desc. */
    reporting_opts = cdbp[2] & 0x7;
    req_opcode = cdbp[3];
    req_sa = sg_get_unaligned_be16(cdbp + 4);
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (alloc_len < 4 || alloc_len > 0xffff) {
        mk_sense_invalid_fld(ptp, true, 6, -1, vb);
        return 0;
    }
    a_len = pg_sz - 72;
    arr = sg_memalign(pg_sz, pg_sz, &free_arr, false);
    if (NULL == arr) {
        pr2ws("%s: calloc() failed to get memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    switch (reporting_opts) {
    case 0: /* all commands */
        count = 0;
        bump = rctd ? 20 : 8;
        for (offset = 4, oip = sg_get_opcode_translation();
             (oip->flags != 0xffff) && (offset < a_len); ++oip) {
            if (F_INV_OP & oip->flags)
                continue;
            ++count;
            arr[offset] = oip->opcode;
            sg_put_unaligned_be16(oip->sa, arr + offset + 2);
            if (rctd)
                arr[offset + 5] |= 0x2;
            if (FF_SA & oip->flags)
                arr[offset + 5] |= 0x1;
            sg_put_unaligned_be16(oip->len_mask[0], arr + offset + 6);
            if (rctd)
                sg_put_unaligned_be16(0xa, arr + offset + 8);
            offset += bump;
        }
        sg_put_unaligned_be32(count * bump, arr + 0);
        break;
    case 1: /* one command: opcode only */
    case 2: /* one command: opcode plus service action */
    case 3: /* one command: if sa==0 then opcode only else opcode+sa */
        for (oip = sg_get_opcode_translation(); oip->flags != 0xffff; ++oip) {
            if ((req_opcode == oip->opcode) && (req_sa == oip->sa))
                break;
        }
        if ((0xffff == oip->flags) || (F_INV_OP & oip->flags)) {
            supp = 1;
            offset = 4;
        } else {
            if (1 == reporting_opts) {
                if (FF_SA & oip->flags) {
                    mk_sense_invalid_fld(ptp, true, 2, 2, vb);
                    free(free_arr);
                    return 0;
                }
                req_sa = 0;
            } else if ((2 == reporting_opts) && 0 == (FF_SA & oip->flags)) {
                mk_sense_invalid_fld(ptp, true, 4, -1, vb);
                free(free_arr);
                return 0;
            }
            if ((0 == (FF_SA & oip->flags)) && (req_opcode == oip->opcode))
                supp = 3;
            else if (0 == (FF_SA & oip->flags))
                supp = 1;
            else if (req_sa != oip->sa)
                supp = 1;
            else
                supp = 3;
            if (3 == supp) {
                u = oip->len_mask[0];
                sg_put_unaligned_be16(u, arr + 2);
                arr[4] = oip->opcode;
                for (k = 1; k < u; ++k)
                    arr[4 + k] = (k < 16) ?
                oip->len_mask[k] : 0xff;
                offset = 4 + u;
            } else
                offset = 4;
        }
        arr[1] = (rctd ? 0x80 : 0) | supp;
        if (rctd) {
            sg_put_unaligned_be16(0xa, arr + offset);
            offset += 12;
        }
        break;
    default:
        mk_sense_invalid_fld(ptp, true, 2, 2, vb);
        free(free_arr);
        return 0;
    }
    offset = (offset < a_len) ? offset : a_len;
    len = (offset < alloc_len) ? offset : alloc_len;
    ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - len;
    if (len > 0)
        memcpy((uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp, arr, len);
    free(free_arr);
    return 0;
}

static int
sntl_rep_tmfs(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
              int time_secs, int vb)
{
    bool repd;
    uint32_t alloc_len, len;
    uint8_t arr[16];

    if (vb > 3)
        pr2ws("%s: time_secs=%d\n", __func__, time_secs);
    memset(arr, 0, sizeof(arr));
    repd = !!(cdbp[2] & 0x80);
    alloc_len = sg_get_unaligned_be32(cdbp + 6);
    if (alloc_len < 4) {
        mk_sense_invalid_fld(ptp, true, 6, -1, vb);
        return 0;
    }
    arr[0] = 0xc8;          /* ATS | ATSS | LURS */
    arr[1] = 0x1;           /* ITNRS */
    if (repd) {
        arr[3] = 0xc;
        len = 16;
    } else
        len = 4;

    len = (len < alloc_len) ? len : alloc_len;
    ptp->io_hdr.din_resid = ptp->io_hdr.din_xfer_len - len;
    if (len > 0)
        memcpy((uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp, arr, len);
    return 0;
}

/* Note that the "Returned logical block address" (RLBA) field in the SCSI
 * READ CAPACITY (10+16) command's response provides the address of the _last_
 * LBA (counting origin 0) which will be one less that the "size" in the
 * NVMe Identify command response's NSZE field. One problem is that in
 * some situations NSZE can be zero: temporarily set RLBA field to 0
 * (implying a 1 LB logical units size) pending further research. The LBLIB
 * is the "Logical Block Length In Bytes" field in the RCAP response. */
static int
sntl_readcap(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
             int time_secs, int vb)
{
    bool is_rcap10 = (SCSI_READ_CAPACITY10_OPC == cdbp[0]);
    int res, n, len, alloc_len, dps;
    uint8_t flbas, index, lbads;  /* NVMe: 2**LBADS --> Logical Block size */
    uint32_t lbafx;     /* NVME: LBAF0...LBAF15, each 16 bytes */
    uint32_t pg_sz = sg_get_page_size();
    uint64_t nsze;
    uint8_t * bp;
    uint8_t * up;
    uint8_t * free_up = NULL;
    uint8_t resp[32];

    if (vb > 3)
        pr2ws("%s: RCAP%d, time_secs=%d\n", __func__,
              (is_rcap10 ? 10 : 16), time_secs);
    up = sg_memalign(pg_sz, pg_sz, &free_up, false);
    if (NULL == up) {
        pr2ws("%s: sg_memalign() failed to get memory\n", __func__);
        return sg_convert_errno(ENOMEM);
    }
    res = sntl_do_identify(ptp, 0x0 /* CNS */, ptp->nvme_nsid, time_secs,
                           pg_sz, up, vb);
    if (res < 0) {
        res = sg_convert_errno(-res);
        goto fini;
    }
    memset(resp, 0, sizeof(resp));
    nsze = sg_get_unaligned_le64(up + 0);
    flbas = up[26];     /* NVME FLBAS field from Identify, want LBAF[flbas] */
    index = 128 + (4 * (flbas & 0xf));
    lbafx = sg_get_unaligned_le32(up + index);
    lbads = (lbafx >> 16) & 0xff;       /* bits 16 to 23 inclusive, pow2 */
    if (is_rcap10) {
        alloc_len = 8;  /* implicit, not in cdb */
        if (nsze > 0xffffffff)
            sg_put_unaligned_be32(0xffffffff, resp + 0);
        else if (0 == nsze)     /* no good answer here */
            sg_put_unaligned_be32(0, resp + 0);         /* SCSI RLBA field */
        else
            sg_put_unaligned_be32((uint32_t)(nsze - 1), resp + 0);
        sg_put_unaligned_be32(1 << lbads, resp + 4);    /* SCSI LBLIB field */
    } else {
        alloc_len = sg_get_unaligned_be32(cdbp + 10);
        dps = up[29];
        if (0x7 & dps) {
            resp[12] = 0x1;
            n = (0x7 & dps) - 1;
            if (n > 0)
                resp[12] |= (n + n);
        }
        if (0 == nsze)  /* no good answer here */
            sg_put_unaligned_be64(0, resp + 0);
        else
            sg_put_unaligned_be64(nsze - 1, resp + 0);
        sg_put_unaligned_be32(1 << lbads, resp + 8);    /* SCSI LBLIB field */
    }
    len = ptp->io_hdr.din_xfer_len;
    bp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.din_xferp;
    n = 32;
    n = (n < alloc_len) ? n : alloc_len;
    n = (n < len) ? n : len;
    ptp->io_hdr.din_resid = len - n;
    if (n > 0)
        memcpy(bp, resp, n);
fini:
    if (free_up)
        free(free_up);
    return res;
}

static int
do_nvm_pt_low(struct sg_pt_linux_scsi * ptp,
              struct sg_nvme_passthru_cmd *cmdp, void * dp, int dlen,
              bool is_read, int time_secs, int vb)
{
    const uint32_t cmd_len = sizeof(struct sg_nvme_passthru_cmd);
    int res;
    uint32_t n;
    uint16_t sct_sc;
    const uint8_t * up = ((const uint8_t *)cmdp) + SG_NVME_PT_OPCODE;
    char nam[64];

    if (vb)
        sg_get_nvme_opcode_name(*up, false /* NVM */ , sizeof(nam), nam);
    else
        nam[0] = '\0';
    cmdp->timeout_ms = (time_secs < 0) ? (-time_secs) : (1000 * time_secs);
    ptp->os_err = 0;
    if (vb > 2) {
        pr2ws("NVMe NVM command: %s\n", nam);
        hex2stderr((const uint8_t *)cmdp, cmd_len, 1);
        if ((vb > 3) && (! is_read) && dp) {
            if (dlen > 0) {
                n = dlen;
                if ((dlen < 512) || (vb > 5))
                    pr2ws("\nData-out buffer (%u bytes):\n", n);
                else {
                    pr2ws("\nData-out buffer (first 512 of %u bytes):\n", n);
                    n = 512;
                }
                hex2stderr((const uint8_t *)dp, n, 0);
            }
        }
    }
    res = ioctl(ptp->dev_fd, NVME_IOCTL_IO_CMD, cmdp);
    if (res < 0) {  /* OS error (errno negated) */
        ptp->os_err = -res;
        if (vb > 1) {
            pr2ws("%s: ioctl for %s [0x%x] failed: %s "
                  "(errno=%d)\n", __func__, nam, *up, strerror(-res), -res);
        }
        return res;
    }

    /* Now res contains NVMe completion queue CDW3 31:17 (15 bits) */
    ptp->nvme_result = cmdp->result;
    if ((! ptp->nvme_our_sntl) && ptp->io_hdr.response &&
        (ptp->io_hdr.max_response_len > 3)) {
        /* build 32 byte "sense" buffer */
        uint8_t * sbp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.response;
        uint16_t st = (uint16_t)res;

        n = ptp->io_hdr.max_response_len;
        n = (n < 32) ? n : 32;
        memset(sbp, 0 , n);
        ptp->io_hdr.response_len = n;
        sg_put_unaligned_le32(cmdp->result,
                              sbp + SG_NVME_PT_CQ_RESULT);
        if (n > 15) /* LSBit will be 0 (Phase bit) after (st << 1) */
            sg_put_unaligned_le16(st << 1, sbp + SG_NVME_PT_CQ_STATUS_P);
    }
    /* clear upper bits (DNR and More) leaving ((SCT << 8) | SC) */
    sct_sc = 0x7ff & res;       /* 11 bits */
    ptp->nvme_status = sct_sc;
    ptp->nvme_stat_dnr = !!(0x4000 & res);
    ptp->nvme_stat_more = !!(0x2000 & res);
    if (sct_sc) {  /* when non-zero, treat as command error */
        if (vb > 1) {
            char b[80];

            pr2ws("%s: ioctl for %s [0x%x] failed, status: %s [0x%x]\n",
                   __func__, nam, *up,
                  sg_get_nvme_cmd_status_str(sct_sc, sizeof(b), b), sct_sc);
        }
        return SG_LIB_NVME_STATUS;      /* == SCSI_PT_DO_NVME_STATUS */
    }
    if ((vb > 3) && is_read && dp) {
        if (dlen > 0) {
            n = dlen;
            if ((dlen < 1024) || (vb > 5))
                pr2ws("\nData-in buffer (%u bytes):\n", n);
            else {
                pr2ws("\nData-in buffer (first 1024 of %u bytes):\n", n);
                n = 1024;
            }
            hex2stderr((const uint8_t *)dp, n, 0);
        }
    }
    return 0;
}

/* Since ptp can be a char device (e.g. /dev/nvme0) or a blocks device
 * (e.g. /dev/nvme0n1 or /dev/nvme0n1p3) use NVME_IOCTL_IO_CMD which is
 * common to both (and takes a timeout). The difficult is that
 * NVME_IOCTL_IO_CMD takes a nvme_passthru_cmd object point. */
static int
sntl_do_nvm_cmd(struct sg_pt_linux_scsi * ptp, struct sg_nvme_user_io * iop,
                uint32_t dlen, bool is_read, int time_secs, int vb)
{

    struct sg_nvme_passthru_cmd nvme_pt_cmd;
    struct sg_nvme_passthru_cmd *cmdp = &nvme_pt_cmd;
    void * dp = (void *)(sg_uintptr_t)iop->addr;

    memset(cmdp, 0, sizeof(*cmdp));
    cmdp->opcode = iop->opcode;
    cmdp->flags = iop->flags;
    cmdp->nsid = ptp->nvme_nsid;
    cmdp->addr = iop->addr;
    cmdp->data_len = dlen;
    cmdp->cdw10 = iop->slba & 0xffffffff;
    cmdp->cdw11 = (iop->slba >> 32) & 0xffffffff;
    cmdp->cdw12 = iop->nblocks; /* lower 16 bits already "0's based" count */

    return do_nvm_pt_low(ptp, cmdp, dp, dlen, is_read, time_secs, vb);
}

static int
sntl_read(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
          int time_secs, int vb)
{
    bool is_read10 = (SCSI_READ10_OPC == cdbp[0]);
    bool have_fua = !!(cdbp[1] & 0x8);
    int res;
    int nblks_t10 = 0;
    struct sg_nvme_user_io io;
    struct sg_nvme_user_io * iop = &io;

    if (vb > 3)
        pr2ws("%s: fua=%d, time_secs=%d\n", __func__, (int)have_fua,
              time_secs);
    memset(iop, 0, sizeof(*iop));
    iop->opcode = SG_NVME_NVM_READ;
    if (is_read10) {
        iop->slba = sg_get_unaligned_be32(cdbp + 2);
        nblks_t10 = sg_get_unaligned_be16(cdbp + 7);
    } else {
        uint32_t num = sg_get_unaligned_be32(cdbp + 10);

        iop->slba = sg_get_unaligned_be64(cdbp + 2);
        if (num > (UINT16_MAX + 1)) {
            mk_sense_invalid_fld(ptp, true, 11, -1, vb);
            return 0;
        } else
            nblks_t10 = num;
    }
    if (0 == nblks_t10) {         /* NOP in SCSI */
        if (vb > 4)
            pr2ws("%s: nblks_t10 is 0, a NOP in SCSI, can't map to NVMe\n",
                  __func__);
        return 0;
    }
    iop->nblocks = nblks_t10 - 1;       /* crazy "0's based" */
    if (have_fua)
        iop->nblocks |= SG_NVME_NVM_CDW12_FUA;
    iop->addr = (uint64_t)ptp->io_hdr.din_xferp;
    res = sntl_do_nvm_cmd(ptp, iop, ptp->io_hdr.din_xfer_len,
                          true /* is_read */, time_secs, vb);
    if (SG_LIB_NVME_STATUS == res) {
        mk_sense_from_nvme_status(ptp, vb);
        return 0;
    }
    return res;
}

static int
sntl_write(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
           int time_secs, int vb)
{
    bool is_write10 = (SCSI_WRITE10_OPC == cdbp[0]);
    bool have_fua = !!(cdbp[1] & 0x8);
    int res;
    int nblks_t10 = 0;
    struct sg_nvme_user_io io;
    struct sg_nvme_user_io * iop = &io;

    if (vb > 3)
        pr2ws("%s: fua=%d, time_secs=%d\n", __func__, (int)have_fua,
              time_secs);
    memset(iop, 0, sizeof(*iop));
    iop->opcode = SG_NVME_NVM_WRITE;
    if (is_write10) {
        iop->slba = sg_get_unaligned_be32(cdbp + 2);
        nblks_t10 = sg_get_unaligned_be16(cdbp + 7);
    } else {
        uint32_t num = sg_get_unaligned_be32(cdbp + 10);

        iop->slba = sg_get_unaligned_be64(cdbp + 2);
        if (num > (UINT16_MAX + 1)) {
            mk_sense_invalid_fld(ptp, true, 11, -1, vb);
            return 0;
        } else
            nblks_t10 = num;
    }
    if (0 == nblks_t10) { /* NOP in SCSI */
        if (vb > 4)
            pr2ws("%s: nblks_t10 is 0, a NOP in SCSI, can't map to NVMe\n",
                  __func__);
        return 0;
    }
    iop->nblocks = nblks_t10 - 1;
    if (have_fua)
        iop->nblocks |= SG_NVME_NVM_CDW12_FUA;
    iop->addr = (uint64_t)ptp->io_hdr.dout_xferp;
    res = sntl_do_nvm_cmd(ptp, iop, ptp->io_hdr.dout_xfer_len, false,
                          time_secs, vb);
    if (SG_LIB_NVME_STATUS == res) {
        mk_sense_from_nvme_status(ptp, vb);
        return 0;
    }
    return res;
}

static int
sntl_verify(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
           int time_secs, int vb)
{
    bool is_verify10 = (SCSI_VERIFY10_OPC == cdbp[0]);
    uint8_t bytchk = (cdbp[1] >> 1) & 0x3;
    uint32_t dlen = 0;
    int res;
    int nblks_t10 = 0;
    struct sg_nvme_user_io io;
    struct sg_nvme_user_io * iop = &io;

    if (vb > 3)
        pr2ws("%s: bytchk=%d, time_secs=%d\n", __func__, bytchk, time_secs);
    if (bytchk > 1) {
        mk_sense_invalid_fld(ptp, true, 1, 2, vb);
        return 0;
    }
    memset(iop, 0, sizeof(*iop));
    iop->opcode = bytchk ? SG_NVME_NVM_COMPARE : SG_NVME_NVM_WRITE;
    if (is_verify10) {
        iop->slba = sg_get_unaligned_be32(cdbp + 2);
        nblks_t10 = sg_get_unaligned_be16(cdbp + 7);
    } else {
        uint32_t num = sg_get_unaligned_be32(cdbp + 10);

        iop->slba = sg_get_unaligned_be64(cdbp + 2);
        if (num > (UINT16_MAX + 1)) {
            mk_sense_invalid_fld(ptp, true, 11, -1, vb);
            return 0;
        } else
            nblks_t10 = num;
    }
    if (0 == nblks_t10) { /* NOP in SCSI */
        if (vb > 4)
            pr2ws("%s: nblks_t10 is 0, a NOP in SCSI, can't map to NVMe\n",
                  __func__);
        return 0;
    }
    iop->nblocks = nblks_t10 - 1;
    if (bytchk) {
        iop->addr = (uint64_t)ptp->io_hdr.dout_xferp;
        dlen = ptp->io_hdr.dout_xfer_len;
    }
    res = sntl_do_nvm_cmd(ptp, iop, dlen, false, time_secs, vb);
    if (SG_LIB_NVME_STATUS == res) {
        mk_sense_from_nvme_status(ptp, vb);
        return 0;
    }
    return res;
}

static int
sntl_write_same(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
                int time_secs, int vb)
{
    bool is_ws10 = (SCSI_WRITE_SAME10_OPC == cdbp[0]);
    bool ndob = is_ws10 ? false : !!(0x1 & cdbp[1]);
    int res;
    int nblks_t10 = 0;
    struct sg_nvme_user_io io;
    struct sg_nvme_user_io * iop = &io;

    if (vb > 3)
        pr2ws("%s: ndob=%d, time_secs=%d\n", __func__, (int)ndob, time_secs);
    if (! ndob) {
        int flbas, index, lbafx, lbads, lbsize;
        uint8_t * up;
        uint8_t * dp;

        dp = (uint8_t *)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
        if (dp == NULL)
            return sg_convert_errno(ENOMEM);
        if (NULL == ptp->nvme_id_ctlp) {
            res = sntl_cache_identify(ptp, time_secs, vb);
            if (SG_LIB_NVME_STATUS == res) {
                mk_sense_from_nvme_status(ptp, vb);
                return 0;
            } else if (res)
                return res;
        }
        up = ptp->nvme_id_ctlp;
        flbas = up[26];     /* NVME FLBAS field from Identify */
        index = 128 + (4 * (flbas & 0xf));
        lbafx = sg_get_unaligned_le32(up + index);
        lbads = (lbafx >> 16) & 0xff;  /* bits 16 to 23 inclusive, pow2 */
        lbsize = 1 << lbads;
        if (! sg_all_zeros(dp, lbsize)) {
            mk_sense_asc_ascq(ptp, SPC_SK_ILLEGAL_REQUEST, PCIE_ERR_ASC,
                              PCIE_UNSUPP_REQ_ASCQ, vb);
            return 0;
        }
        /* so given single LB full of zeros, can translate .... */
    }
    memset(iop, 0, sizeof(*iop));
    iop->opcode =  SG_NVME_NVM_WRITE_ZEROES;
    if (is_ws10) {
        iop->slba = sg_get_unaligned_be32(cdbp + 2);
        nblks_t10 = sg_get_unaligned_be16(cdbp + 7);
    } else {
        uint32_t num = sg_get_unaligned_be32(cdbp + 10);

        iop->slba = sg_get_unaligned_be64(cdbp + 2);
        if (num > (UINT16_MAX + 1)) {
            mk_sense_invalid_fld(ptp, true, 11, -1, vb);
            return 0;
        } else
            nblks_t10 = num;
    }
    if (0 == nblks_t10) { /* NOP in SCSI */
        if (vb > 4)
            pr2ws("%s: nblks_t10 is 0, a NOP in SCSI, can't map to NVMe\n",
                  __func__);
        return 0;
    }
    iop->nblocks = nblks_t10 - 1;
    res = sntl_do_nvm_cmd(ptp, iop, 0, false, time_secs, vb);
    if (SG_LIB_NVME_STATUS == res) {
        mk_sense_from_nvme_status(ptp, vb);
        return 0;
    }
    return res;
}

static int
sntl_sync_cache(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
                int time_secs, int vb)
{
    bool immed = !!(0x2 & cdbp[1]);
    struct sg_nvme_user_io io;
    struct sg_nvme_user_io * iop = &io;
    int res;

    if (vb > 3)
        pr2ws("%s: immed=%d, time_secs=%d\n", __func__, (int)immed,
              time_secs);
    memset(iop, 0, sizeof(*iop));
    iop->opcode =  SG_NVME_NVM_FLUSH;
    if (vb > 4)
        pr2ws("%s: immed bit, lba and num_lbs fields ignored\n", __func__);
    res = sntl_do_nvm_cmd(ptp, iop, 0, false, time_secs, vb);
    if (SG_LIB_NVME_STATUS == res) {
        mk_sense_from_nvme_status(ptp, vb);
        return 0;
    }
    return res;
}

static int
sntl_start_stop(struct sg_pt_linux_scsi * ptp, const uint8_t * cdbp,
                int time_secs, int vb)
{
    bool immed = !!(0x1 & cdbp[1]);

    if (vb > 3)
        pr2ws("%s: immed=%d, time_secs=%d, ignore\n", __func__, (int)immed,
              time_secs);
    if (ptp) { }        /* suppress warning */
    return 0;
}

/* Executes NVMe Admin command (or at least forwards it to lower layers).
 * Returns 0 for success, negative numbers are negated 'errno' values from
 * OS system calls. Positive return values are errors from this package.
 * When time_secs is 0 the Linux NVMe Admin command default of 60 seconds
 * is used. */
int
sg_do_nvme_pt(struct sg_pt_base * vp, int fd, int time_secs, int vb)
{
    bool scsi_cdb;
    bool is_read = false;
    int n, len, hold_dev_fd;
    uint16_t sa;
    struct sg_pt_linux_scsi * ptp = &vp->impl;
    struct sg_nvme_passthru_cmd cmd;
    const uint8_t * cdbp;
    void * dp = NULL;

    if (! ptp->io_hdr.request) {
        if (vb)
            pr2ws("No NVMe command given (set_scsi_pt_cdb())\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    hold_dev_fd = ptp->dev_fd;
    if (fd >= 0) {
        if ((ptp->dev_fd >= 0) && (fd != ptp->dev_fd)) {
            if (vb)
                pr2ws("%s: file descriptor given to create() and here "
                      "differ\n", __func__);
            return SCSI_PT_DO_BAD_PARAMS;
        }
        ptp->dev_fd = fd;
    } else if (ptp->dev_fd < 0) {
        if (vb)
            pr2ws("%s: invalid file descriptors\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    n = ptp->io_hdr.request_len;
    cdbp = (const uint8_t *)(sg_uintptr_t)ptp->io_hdr.request;
    if (vb > 3)
        pr2ws("%s: opcode=0x%x, fd=%d (dev_fd=%d), time_secs=%d\n", __func__,
              cdbp[0], fd, hold_dev_fd, time_secs);
    scsi_cdb = sg_is_scsi_cdb(cdbp, n);
    /* direct NVMe command (i.e. 64 bytes long) or SNTL */
    ptp->nvme_our_sntl = scsi_cdb;
    if (scsi_cdb) {
        switch (cdbp[0]) {
        case SCSI_INQUIRY_OPC:
            return sntl_inq(ptp, cdbp, time_secs, vb);
        case SCSI_REPORT_LUNS_OPC:
            return sntl_rluns(ptp, cdbp, time_secs, vb);
        case SCSI_TEST_UNIT_READY_OPC:
            return sntl_tur(ptp, time_secs, vb);
        case SCSI_REQUEST_SENSE_OPC:
            return sntl_req_sense(ptp, cdbp, time_secs, vb);
        case SCSI_READ10_OPC:
        case SCSI_READ16_OPC:
            return sntl_read(ptp, cdbp, time_secs, vb);
        case SCSI_WRITE10_OPC:
        case SCSI_WRITE16_OPC:
            return sntl_write(ptp, cdbp, time_secs, vb);
        case SCSI_START_STOP_OPC:
            return sntl_start_stop(ptp, cdbp, time_secs, vb);
        case SCSI_SEND_DIAGNOSTIC_OPC:
            return sntl_senddiag(ptp, cdbp, time_secs, vb);
        case SCSI_RECEIVE_DIAGNOSTIC_OPC:
            return sntl_recvdiag(ptp, cdbp, time_secs, vb);
        case SCSI_MODE_SENSE10_OPC:
        case SCSI_MODE_SELECT10_OPC:
            return sntl_mode_ss(ptp, cdbp, time_secs, vb);
        case SCSI_READ_CAPACITY10_OPC:
            return sntl_readcap(ptp, cdbp, time_secs, vb);
        case SCSI_VERIFY10_OPC:
        case SCSI_VERIFY16_OPC:
            return sntl_verify(ptp, cdbp, time_secs, vb);
        case SCSI_WRITE_SAME10_OPC:
        case SCSI_WRITE_SAME16_OPC:
            return sntl_write_same(ptp, cdbp, time_secs, vb);
        case SCSI_SYNC_CACHE10_OPC:
        case SCSI_SYNC_CACHE16_OPC:
            return sntl_sync_cache(ptp, cdbp, time_secs, vb);
        case SCSI_SERVICE_ACT_IN_OPC:
            if (SCSI_READ_CAPACITY16_SA == (cdbp[1] & SCSI_SA_MSK))
                return sntl_readcap(ptp, cdbp, time_secs, vb);
            goto fini;
        case SCSI_MAINT_IN_OPC:
            sa = SCSI_SA_MSK & cdbp[1];        /* service action */
            if (SCSI_REP_SUP_OPCS_OPC == sa)
                return sntl_rep_opcodes(ptp, cdbp, time_secs, vb);
            else if (SCSI_REP_SUP_TMFS_OPC == sa)
                return sntl_rep_tmfs(ptp, cdbp, time_secs, vb);
            /* fall through */
        default:
fini:
            if (vb > 2) {
                char b[64];

                sg_get_command_name(cdbp, -1, sizeof(b), b);
                pr2ws("%s: no translation to NVMe for SCSI %s command\n",
                      __func__, b);
            }
            mk_sense_asc_ascq(ptp, SPC_SK_ILLEGAL_REQUEST, INVALID_OPCODE,
                              0, vb);
            return 0;
        }
    }
    len = (int)sizeof(cmd);
    n = (n < len) ? n : len;
    if (n < 64) {
        if (vb)
            pr2ws("%s: command length of %d bytes is too short\n", __func__,
                  n);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    memcpy(&cmd, (const uint8_t *)(sg_uintptr_t)ptp->io_hdr.request, n);
    if (n < len)        /* zero out rest of 'cmd' */
        memset((uint8_t *)&cmd + n, 0, len - n);
    if (ptp->io_hdr.din_xfer_len > 0) {
        cmd.data_len = ptp->io_hdr.din_xfer_len;
        dp = (void *)(sg_uintptr_t)ptp->io_hdr.din_xferp;
        cmd.addr = (uint64_t)(sg_uintptr_t)ptp->io_hdr.din_xferp;
        is_read = true;
    } else if (ptp->io_hdr.dout_xfer_len > 0) {
        cmd.data_len = ptp->io_hdr.dout_xfer_len;
        dp = (void *)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
        cmd.addr = (uint64_t)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
        is_read = false;
    }
    return sg_nvme_admin_cmd(ptp, &cmd, dp, is_read, time_secs, vb);
}

#else           /* (HAVE_NVME && (! IGNORE_NVME)) [around line 140] */

int
sg_do_nvme_pt(struct sg_pt_base * vp, int fd, int time_secs, int vb)
{
    if (vb) {
        pr2ws("%s: not supported, ", __func__);
#ifdef HAVE_NVME
        pr2ws("HAVE_NVME, ");
#else
        pr2ws("don't HAVE_NVME, ");
#endif

#ifdef IGNORE_NVME
        pr2ws("IGNORE_NVME");
#else
        pr2ws("don't IGNORE_NVME");
#endif
        pr2ws("\n");
     }
    if (vp) { ; }               /* suppress warning */
    if (fd) { ; }               /* suppress warning */
    if (time_secs) { ; }        /* suppress warning */
    return -ENOTTY;             /* inappropriate ioctl error */
}

#endif          /* (HAVE_NVME && (! IGNORE_NVME)) */

#if (HAVE_NVME && (! IGNORE_NVME))

int
do_nvm_pt(struct sg_pt_base * vp, int submq, int timeout_secs, int vb)
{
    bool is_read = false;
    int dlen;
    struct sg_pt_linux_scsi * ptp = &vp->impl;
    struct sg_nvme_passthru_cmd cmd;
    uint8_t * cmdp = (uint8_t *)&cmd;
    void * dp = NULL;

    if (vb && (submq != 0))
        pr2ws("%s: warning, uses submit queue 0\n", __func__);
    if (ptp->dev_fd < 0) {
        if (vb > 1)
            pr2ws("%s: no NVMe file descriptor given\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (! ptp->is_nvme) {
        if (vb > 1)
            pr2ws("%s: file descriptor is not NVMe device\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if ((! ptp->io_hdr.request) || (64 != ptp->io_hdr.request_len)) {
        if (vb > 1)
            pr2ws("%s: no NVMe 64 byte command present\n", __func__);
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (sizeof(cmd) > 64)
        memset(cmdp + 64, 0, sizeof(cmd) - 64);
    memcpy(cmdp, (uint8_t *)(sg_uintptr_t)ptp->io_hdr.request, 64);
    ptp->nvme_our_sntl = false;

    dlen = ptp->io_hdr.din_xfer_len;
    if (dlen > 0) {
        is_read = true;
        dp = (void *)(sg_uintptr_t)ptp->io_hdr.din_xferp;
    } else {
        dlen = ptp->io_hdr.dout_xfer_len;
        if (dlen > 0)
            dp = (void *)(sg_uintptr_t)ptp->io_hdr.dout_xferp;
    }
    return do_nvm_pt_low(ptp, &cmd, dp, dlen, is_read, timeout_secs, vb);
}

#else           /* (HAVE_NVME && (! IGNORE_NVME)) */

int
do_nvm_pt(struct sg_pt_base * vp, int submq, int timeout_secs, int verbose)
{
    if (vb) {
        pr2ws("%s: not supported, ", __func__);
#ifdef HAVE_NVME
        pr2ws("HAVE_NVME, ");
#else
        pr2ws("don't HAVE_NVME, ");
#endif

#ifdef IGNORE_NVME
        pr2ws("IGNORE_NVME");
#else
        pr2ws("don't IGNORE_NVME");
#endif
    if (vp) { }
    if (submq) { }
    if (timeout_secs) { }
    return SCSI_PT_DO_NOT_SUPPORTED;
}

#endif          /* (HAVE_NVME && (! IGNORE_NVME)) */
