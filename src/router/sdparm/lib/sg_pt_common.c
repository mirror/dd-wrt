/*
 * Copyright (c) 2009-2020 Douglas Gilbert.
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
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_pt.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"
#include "sg_pr2serr.h"

#if (HAVE_NVME && (! IGNORE_NVME))
#include "sg_pt_nvme.h"
#endif

static const char * scsi_pt_version_str = "3.16 20200722";


const char *
scsi_pt_version()
{
    return scsi_pt_version_str;
}

const char *
sg_pt_version()
{
    return scsi_pt_version_str;
}


#if (HAVE_NVME && (! IGNORE_NVME))
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

#define SAVING_PARAMS_UNSUP 0x39
#define INVALID_FIELD_IN_CDB 0x24
#define INVALID_FIELD_IN_PARAM_LIST 0x26
#define PARAMETER_LIST_LENGTH_ERR 0x1a

static const char * nvme_scsi_vendor_str = "NVMe    ";


#define F_SA_LOW                0x80    /* cdb byte 1, bits 4 to 0 */
#define F_SA_HIGH               0x100   /* as used by variable length cdbs */
#define FF_SA (F_SA_HIGH | F_SA_LOW)
#define F_INV_OP                0x200

/* Table of SCSI operation code (opcodes) supported by SNTL */
static struct sg_opcode_info_t sg_opcode_info_arr[] =
{
    {0x0, 0, 0, {6,              /* TEST UNIT READY */
      0, 0, 0, 0, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x3, 0, 0, {6,             /* REQUEST SENSE */
      0xe1, 0, 0, 0xff, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x12, 0, 0, {6,            /* INQUIRY */
      0xe3, 0xff, 0xff, 0xff, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x1b, 0, 0, {6,            /* START STOP UNIT */
      0x1, 0, 0xf, 0xf7, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x1c, 0, 0, {6,            /* RECEIVE DIAGNOSTIC RESULTS */
      0x1, 0xff, 0xff, 0xff, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x1d, 0, 0, {6,            /* SEND DIAGNOSTIC */
      0xf7, 0x0, 0xff, 0xff, 0xc7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
    {0x25, 0, 0, {10,            /* READ CAPACITY(10) */
      0x1, 0xff, 0xff, 0xff, 0xff, 0, 0, 0x1, 0xc7, 0, 0, 0, 0, 0, 0} },
    {0x28, 0, 0, {10,            /* READ(10) */
      0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc7, 0, 0, 0, 0,
      0, 0} },
    {0x2a, 0, 0, {10,            /* WRITE(10) */
      0xfb, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc7, 0, 0, 0, 0,
      0, 0} },
    {0x2f, 0, 0, {10,            /* VERIFY(10) */
      0xf6, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc7, 0, 0, 0, 0,
      0, 0} },
    {0x35, 0, 0, {10,            /* SYNCHRONIZE CACHE(10) */
      0x7, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc7, 0, 0, 0, 0,
      0, 0} },
    {0x41, 0, 0, {10,            /* WRITE SAME(10) */
      0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xc7, 0, 0, 0, 0,
      0, 0} },
    {0x55, 0, 0, {10,           /* MODE SELECT(10) */
      0x13, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xc7, 0, 0, 0, 0, 0, 0} },
    {0x5a, 0, 0, {10,           /* MODE SENSE(10) */
      0x18, 0xff, 0xff, 0x0, 0x0, 0x0, 0xff, 0xff, 0xc7, 0, 0, 0, 0, 0, 0} },
    {0x88, 0, 0, {16,            /* READ(16) */
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xc7} },
    {0x8a, 0, 0, {16,            /* WRITE(16) */
      0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xc7} },
    {0x8f, 0, 0, {16,            /* VERIFY(16) */
      0xf6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x3f, 0xc7} },
    {0x91, 0, 0, {16,            /* SYNCHRONIZE CACHE(16) */
      0x7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x3f, 0xc7} },
    {0x93, 0, 0, {16,            /* WRITE SAME(16) */
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x3f, 0xc7} },
    {0x9e, 0x10, F_SA_LOW, {16,  /* READ CAPACITY(16) [service action in] */
      0x10, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x1, 0xc7} },
    {0xa0, 0, 0, {12,           /* REPORT LUNS */
      0xe3, 0xff, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0, 0xc7, 0, 0, 0, 0} },
    {0xa3, 0xc, F_SA_LOW, {12,  /* REPORT SUPPORTED OPERATION CODES */
      0xc, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0xc7, 0, 0, 0,
      0} },
    {0xa3, 0xd, F_SA_LOW, {12,  /* REPORT SUPPORTED TASK MAN. FUNCTIONS */
      0xd, 0x80, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0, 0xc7, 0, 0, 0, 0} },

    {0xff, 0xffff, 0xffff, {0,  /* Sentinel, keep as last element */
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} },
};

/* Returns pointer to array of struct sg_opcode_info_t of SCSI commands
 * translated to NVMe. */
const struct sg_opcode_info_t *
sg_get_opcode_translation(void)
{
    return sg_opcode_info_arr;
}

/* Given the NVMe Identify controller response and optionally the NVMe
 * Identify namespace response (NULL otherwise), generate the SCSI VPD
 * page 0x83 (device identification) descriptor(s) in dop. Return the
 * number of bytes written which will not exceed max_do_len. Probably use
 * Peripheral Device Type (pdt) of 0 (disk) for don't know. Transport
 * protocol (tproto) should be -1 if not known, else SCSI value.
 * N.B. Does not write total VPD page length into dop[2:3] . */
int
sg_make_vpd_devid_for_nvme(const uint8_t * nvme_id_ctl_p,
                           const uint8_t * nvme_id_ns_p, int pdt,
                           int tproto, uint8_t * dop, int max_do_len)
{
    bool have_nguid, have_eui64;
    int k, n;
    char b[4];

    if ((NULL == nvme_id_ctl_p) || (NULL == dop) || (max_do_len < 56))
        return 0;

    memset(dop, 0, max_do_len);
    dop[0] = 0x1f & pdt;  /* (PQ=0)<<5 | (PDT=pdt); 0 or 0xd (SES) */
    dop[1] = 0x83;      /* Device Identification VPD page number */
    /* Build a T10 Vendor ID based designator (desig_id=1) for controller */
    if (tproto >= 0) {
        dop[4] = ((0xf & tproto) << 4) | 0x2;
        dop[5] = 0xa1; /* PIV=1, ASSOC=2 (target device), desig_id=1 */
    } else {
        dop[4] = 0x2;  /* Prococol id=0, code_set=2 (ASCII) */
        dop[5] = 0x21; /* PIV=0, ASSOC=2 (target device), desig_id=1 */
    }
    memcpy(dop + 8, nvme_scsi_vendor_str, 8); /* N.B. this is "NVMe    " */
    memcpy(dop + 16, nvme_id_ctl_p + 24, 40);  /* MN */
    for (k = 40; k > 0; --k) {
        if (' ' == dop[15 + k])
            dop[15 + k] = '_'; /* convert trailing spaces */
        else
            break;
    }
    if (40 == k)
        --k;
    n = 16 + 1 + k;
    if (max_do_len < (n + 20))
        return 0;
    memcpy(dop + n, nvme_id_ctl_p + 4, 20); /* SN */
    for (k = 20; k > 0; --k) {  /* trim trailing spaces */
        if (' ' == dop[n + k - 1])
            dop[n + k - 1] = '\0';
        else
            break;
    }
    n += k;
    if (0 != (n % 4))
        n = ((n / 4) + 1) * 4;  /* round up to next modulo 4 */
    dop[7] = n - 8;
    if (NULL == nvme_id_ns_p)
        return n;

    /* Look for NGUID (16 byte identifier) or EUI64 (8 byte) fields in
     * NVME Identify for namespace. If found form a EUI and a SCSI string
     * descriptor for non-zero NGUID or EUI64 (prefer NGUID if both). */
    have_nguid = ! sg_all_zeros(nvme_id_ns_p + 104, 16);
    have_eui64 = ! sg_all_zeros(nvme_id_ns_p + 120, 8);
    if ((! have_nguid) && (! have_eui64))
        return n;
    if (have_nguid) {
        if (max_do_len < (n + 20))
            return n;
        dop[n + 0] = 0x1;  /* Prococol id=0, code_set=1 (binary) */
        dop[n + 1] = 0x02; /* PIV=0, ASSOC=0 (lu), desig_id=2 (eui) */
        dop[n + 3] = 16;
        memcpy(dop + n + 4, nvme_id_ns_p + 104, 16);
        n += 20;
        if (max_do_len < (n + 40))
            return n;
        dop[n + 0] = 0x3;  /* Prococol id=0, code_set=3 (utf8) */
        dop[n + 1] = 0x08; /* PIV=0, ASSOC=0 (lu), desig_id=8 (scsi string) */
        dop[n + 3] = 36;
        memcpy(dop + n + 4, "eui.", 4);
        for (k = 0; k < 16; ++k) {
            snprintf(b, sizeof(b), "%02X", nvme_id_ns_p[104 + k]);
            memcpy(dop + n + 8 + (2 * k), b, 2);
        }
        return n + 40;
    } else {    /* have_eui64 is true, 8 byte identifier */
        if (max_do_len < (n + 12))
            return n;
        dop[n + 0] = 0x1;  /* Prococol id=0, code_set=1 (binary) */
        dop[n + 1] = 0x02; /* PIV=0, ASSOC=0 (lu), desig_id=2 (eui) */
        dop[n + 3] = 8;
        memcpy(dop + n + 4, nvme_id_ns_p + 120, 8);
        n += 12;
        if (max_do_len < (n + 24))
            return n;
        dop[n + 0] = 0x3;  /* Prococol id=0, code_set=3 (utf8) */
        dop[n + 1] = 0x08; /* PIV=0, ASSOC=0 (lu), desig_id=8 (scsi string) */
        dop[n + 3] = 20;
        memcpy(dop + n + 4, "eui.", 4);
        for (k = 0; k < 8; ++k) {
            snprintf(b, sizeof(b), "%02X", nvme_id_ns_p[120 + k]);
            memcpy(dop + n + 8 + (2 * k), b, 2);
        }
        return n + 24;
    }
}

/* Disconnect-Reconnect page for mode_sense */
static int
resp_disconnect_pg(uint8_t * p, int pcontrol)
{
    uint8_t disconnect_pg[] = {0x2, 0xe, 128, 128, 0, 10, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0};

    memcpy(p, disconnect_pg, sizeof(disconnect_pg));
    if (1 == pcontrol)
        memset(p + 2, 0, sizeof(disconnect_pg) - 2);
    return sizeof(disconnect_pg);
}

static uint8_t caching_m_pg[] = {0x8, 18, 0x14, 0, 0xff, 0xff, 0, 0,
                                 0xff, 0xff, 0xff, 0xff, 0x80, 0x14, 0, 0,
                                 0, 0, 0, 0};

/* Control mode page (SBC) for mode_sense */
static int
resp_caching_m_pg(unsigned char *p, int pcontrol, bool wce)
{       /* Caching page for mode_sense */
        uint8_t ch_caching_m_pg[] = {/* 0x8, 18, */ 0x4, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t d_caching_m_pg[] = {0x8, 18, 0x14, 0, 0xff, 0xff, 0, 0,
                0xff, 0xff, 0xff, 0xff, 0x80, 0x14, 0, 0,     0, 0, 0, 0};

        // if (SDEBUG_OPT_N_WCE & sdebug_opts)
                caching_m_pg[2] &= ~0x4;  /* set WCE=0 (default WCE=1) */
        if ((0 == pcontrol) || (3 == pcontrol)) {
            if (wce)
                caching_m_pg[2] |= 0x4;
            else
                caching_m_pg[2] &= ~0x4;
        }
        memcpy(p, caching_m_pg, sizeof(caching_m_pg));
        if (1 == pcontrol) {
            if (wce)
                ch_caching_m_pg[2] |= 0x4;
            else
                ch_caching_m_pg[2] &= ~0x4;
            memcpy(p + 2, ch_caching_m_pg, sizeof(ch_caching_m_pg));
        }
        else if (2 == pcontrol) {
            if (wce)
                d_caching_m_pg[2] |= 0x4;
            else
                d_caching_m_pg[2] &= ~0x4;
            memcpy(p, d_caching_m_pg, sizeof(d_caching_m_pg));
        }
        return sizeof(caching_m_pg);
}

static uint8_t ctrl_m_pg[] = {0xa, 10, 2, 0, 0, 0, 0, 0,
                              0, 0, 0x2, 0x4b};

/* Control mode page for mode_sense */
static int
resp_ctrl_m_pg(uint8_t *p, int pcontrol)
{
    uint8_t ch_ctrl_m_pg[] = {/* 0xa, 10, */ 0x6, 0, 0, 0, 0, 0,
                              0, 0, 0, 0};
    uint8_t d_ctrl_m_pg[] = {0xa, 10, 2, 0, 0, 0, 0, 0,
                             0, 0, 0x2, 0x4b};

    memcpy(p, ctrl_m_pg, sizeof(ctrl_m_pg));
    if (1 == pcontrol)
        memcpy(p + 2, ch_ctrl_m_pg, sizeof(ch_ctrl_m_pg));
    else if (2 == pcontrol)
        memcpy(p, d_ctrl_m_pg, sizeof(d_ctrl_m_pg));
    return sizeof(ctrl_m_pg);
}

static uint8_t ctrl_ext_m_pg[] = {0x4a, 0x1, 0, 0x1c,  0, 0, 0x40, 0,
                                  0, 0, 0, 0,  0, 0, 0, 0,
                                  0, 0, 0, 0,  0, 0, 0, 0,
                                  0, 0, 0, 0,  0, 0, 0, 0, };

/* Control Extension mode page [0xa,0x1] for mode_sense */
static int
resp_ctrl_ext_m_pg(uint8_t *p, int pcontrol)
{
    uint8_t ch_ctrl_ext_m_pg[] = {/* 0x4a, 0x1, 0, 0x1c, */ 0, 0, 0, 0,
                         0, 0, 0, 0,  0, 0, 0, 0,
                         0, 0, 0, 0,  0, 0, 0, 0,
                         0, 0, 0, 0,  0, 0, 0, 0, };
    uint8_t d_ctrl_ext_m_pg[] = {0x4a, 0x1, 0, 0x1c,  0, 0, 0x40, 0,
                         0, 0, 0, 0,  0, 0, 0, 0,
                         0, 0, 0, 0,  0, 0, 0, 0,
                         0, 0, 0, 0,  0, 0, 0, 0, };

    memcpy(p, ctrl_ext_m_pg, sizeof(ctrl_ext_m_pg));
    if (1 == pcontrol)
        memcpy(p + 4, ch_ctrl_ext_m_pg, sizeof(ch_ctrl_ext_m_pg));
    else if (2 == pcontrol)
        memcpy(p, d_ctrl_ext_m_pg, sizeof(d_ctrl_ext_m_pg));
    return sizeof(ctrl_ext_m_pg);
}

static uint8_t iec_m_pg[] = {0x1c, 0xa, 0x08, 0, 0, 0, 0, 0, 0, 0, 0x0, 0x0};

/* Informational Exceptions control mode page for mode_sense */
static int
resp_iec_m_pg(uint8_t *p, int pcontrol)
{
    uint8_t ch_iec_m_pg[] = {/* 0x1c, 0xa, */ 0x4, 0xf, 0, 0, 0, 0, 0, 0,
                             0x0, 0x0};
    uint8_t d_iec_m_pg[] = {0x1c, 0xa, 0x08, 0, 0, 0, 0, 0, 0, 0, 0x0, 0x0};

    memcpy(p, iec_m_pg, sizeof(iec_m_pg));
    if (1 == pcontrol)
        memcpy(p + 2, ch_iec_m_pg, sizeof(ch_iec_m_pg));
    else if (2 == pcontrol)
        memcpy(p, d_iec_m_pg, sizeof(d_iec_m_pg));
    return sizeof(iec_m_pg);
}

static uint8_t vs_ua_m_pg[] = {0x0, 0xe, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0};

/* Vendor specific Unit Attention mode page for mode_sense */
static int
resp_vs_ua_m_pg(uint8_t *p, int pcontrol)
{
    uint8_t ch_vs_ua_m_pg[] = {/* 0x0, 0xe, */ 0xff, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t d_vs_ua_m_pg[] = {0x0, 0xe, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0};

    memcpy(p, vs_ua_m_pg, sizeof(vs_ua_m_pg));
    if (1 == pcontrol)
        memcpy(p + 2, ch_vs_ua_m_pg, sizeof(ch_vs_ua_m_pg));
    else if (2 == pcontrol)
        memcpy(p, d_vs_ua_m_pg, sizeof(d_vs_ua_m_pg));
    return sizeof(vs_ua_m_pg);
}

void
sntl_init_dev_stat(struct sg_sntl_dev_state_t * dsp)
{
    if (dsp) {
        dsp->scsi_dsense = !! (0x4 & ctrl_m_pg[2]);
        dsp->enclosure_override = vs_ua_m_pg[2];
    }
}


#define SDEBUG_MAX_MSENSE_SZ 256

/* Only support MODE SENSE(10). Returns the number of bytes written to dip,
 * or -1 if error info placed in resp. */
int
sntl_resp_mode_sense10(const struct sg_sntl_dev_state_t * dsp,
                       const uint8_t * cdbp, uint8_t * dip, int mx_di_len,
                       struct sg_sntl_result_t * resp)
{
    bool dbd, llbaa, is_disk, bad_pcode;
    int pcontrol, pcode, subpcode, bd_len, alloc_len, offset, len;
    const uint32_t num_blocks = 0x100000;       /* made up */
    const uint32_t lb_size = 512;               /* guess */
    uint8_t dev_spec;
    uint8_t * ap;
    uint8_t arr[SDEBUG_MAX_MSENSE_SZ];

    memset(resp, 0, sizeof(*resp));
    dbd = !! (cdbp[1] & 0x8);         /* disable block descriptors */
    pcontrol = (cdbp[2] & 0xc0) >> 6;
    pcode = cdbp[2] & 0x3f;
    subpcode = cdbp[3];
    llbaa = !!(cdbp[1] & 0x10);
    is_disk = ((dsp->pdt == PDT_DISK) || (dsp->pdt == PDT_ZBC));
    if (is_disk && !dbd)
        bd_len = llbaa ? 16 : 8;
    else
        bd_len = 0;
    alloc_len = sg_get_unaligned_be16(cdbp + 7);
    memset(arr, 0, SDEBUG_MAX_MSENSE_SZ);
    if (0x3 == pcontrol) {  /* Saving values not supported */
        resp->asc = SAVING_PARAMS_UNSUP;
        goto err_out;
    }
    /* for disks set DPOFUA bit and clear write protect (WP) bit */
    if (is_disk)
        dev_spec = 0x10;        /* =0x90 if WP=1 implies read-only */
    else
        dev_spec = 0x0;
    arr[3] = dev_spec;
    if (16 == bd_len)
        arr[4] = 0x1;   /* set LONGLBA bit */
    arr[7] = bd_len;        /* assume 255 or less */
    offset = 8;
    ap = arr + offset;

    if (8 == bd_len) {
        sg_put_unaligned_be32(num_blocks, ap + 0);
        sg_put_unaligned_be16((uint16_t)lb_size, ap + 6);
        offset += bd_len;
        ap = arr + offset;
    } else if (16 == bd_len) {
        sg_put_unaligned_be64(num_blocks, ap + 0);
        sg_put_unaligned_be32(lb_size, ap + 12);
        offset += bd_len;
        ap = arr + offset;
    }
    bad_pcode = false;

    switch (pcode) {
    case 0x2:       /* Disconnect-Reconnect page, all devices */
        if (0x0 == subpcode)
            len = resp_disconnect_pg(ap, pcontrol);
        else {
            len = 0;
            bad_pcode = true;
        }
        offset += len;
        break;
    case 0x8:       /* Caching Mode page, disk (like) devices */
        if (! is_disk) {
            len = 0;
            bad_pcode = true;
        } else if (0x0 == subpcode)
            len = resp_caching_m_pg(ap, pcontrol, dsp->wce);
        else {
            len = 0;
            bad_pcode = true;
        }
        offset += len;
        break;
    case 0xa:       /* Control Mode page, all devices */
        if (0x0 == subpcode)
            len = resp_ctrl_m_pg(ap, pcontrol);
        else if (0x1 == subpcode)
            len = resp_ctrl_ext_m_pg(ap, pcontrol);
        else {
            len = 0;
            bad_pcode = true;
        }
        offset += len;
        break;
    case 0x1c:      /* Informational Exceptions Mode page, all devices */
        if (0x0 == subpcode)
            len = resp_iec_m_pg(ap, pcontrol);
        else {
            len = 0;
            bad_pcode = true;
        }
        offset += len;
        break;
    case 0x3f:      /* Read all Mode pages */
        if ((0 == subpcode) || (0xff == subpcode)) {
            len = 0;
            len = resp_disconnect_pg(ap + len, pcontrol);
            if (is_disk)
                len += resp_caching_m_pg(ap + len, pcontrol, dsp->wce);
            len += resp_ctrl_m_pg(ap + len, pcontrol);
            if (0xff == subpcode)
                len += resp_ctrl_ext_m_pg(ap + len, pcontrol);
            len += resp_iec_m_pg(ap + len, pcontrol);
            len += resp_vs_ua_m_pg(ap + len, pcontrol);
            offset += len;
        } else {
            resp->asc = INVALID_FIELD_IN_CDB;
            resp->in_byte = 3;
            resp->in_bit = 255;
            goto err_out;
        }
        break;
    case 0x0:       /* Vendor specific "Unit Attention" mode page */
        /* all sub-page codes ?? */
        len = resp_vs_ua_m_pg(ap, pcontrol);
        offset += len;
        break;      /* vendor is "NVMe    " (from INQUIRY field) */
    default:
        bad_pcode = true;
        break;
    }
    if (bad_pcode) {
        resp->asc = INVALID_FIELD_IN_CDB;
        resp->in_byte = 2;
        resp->in_bit = 5;
        goto err_out;
    }
    sg_put_unaligned_be16(offset - 2, arr + 0);
    len = (alloc_len < offset) ? alloc_len : offset;
    len = (len < mx_di_len) ? len : mx_di_len;
    memcpy(dip, arr, len);
    return len;

err_out:
    resp->sstatus = SAM_STAT_CHECK_CONDITION;
    resp->sk = SPC_SK_ILLEGAL_REQUEST;
    return -1;
}

#define SDEBUG_MAX_MSELECT_SZ 512

/* Only support MODE SELECT(10). Returns number of bytes used from dop,
 * else -1 on error with sense code placed in resp. */
int
sntl_resp_mode_select10(struct sg_sntl_dev_state_t * dsp,
                        const uint8_t * cdbp, const uint8_t * dop, int do_len,
                        struct sg_sntl_result_t * resp)
{
    int pf, sp, ps, md_len, bd_len, off, spf, pg_len, rlen, param_len, mpage;
    int sub_mpage;
    uint8_t arr[SDEBUG_MAX_MSELECT_SZ];

    memset(resp, 0, sizeof(*resp));
    memset(arr, 0, sizeof(arr));
    pf = cdbp[1] & 0x10;
    sp = cdbp[1] & 0x1;
    param_len = sg_get_unaligned_be16(cdbp + 7);
    if ((0 == pf) || sp || (param_len > SDEBUG_MAX_MSELECT_SZ)) {
        resp->asc = INVALID_FIELD_IN_CDB;
        resp->in_byte = 1;
        if (sp)
            resp->in_bit = 0;
        else if (0 == pf)
            resp->in_bit = 4;
        else {
            resp->in_byte = 7;
            resp->in_bit = 255;
        }
        goto err_out;
    }
    rlen = (do_len < param_len) ? do_len : param_len;
    memcpy(arr, dop, rlen);
    md_len = sg_get_unaligned_be16(arr + 0) + 2;
    bd_len = sg_get_unaligned_be16(arr + 6);
    if (md_len > 2) {
        resp->asc = INVALID_FIELD_IN_PARAM_LIST;
        resp->in_byte = 0;
        resp->in_bit = 255;
        goto err_out;
    }
    off = bd_len + 8;
    mpage = arr[off] & 0x3f;
    ps = !!(arr[off] & 0x80);
    if (ps) {
        resp->asc = INVALID_FIELD_IN_PARAM_LIST;
        resp->in_byte = off;
        resp->in_bit = 7;
        goto err_out;
    }
    spf = !!(arr[off] & 0x40);
    pg_len = spf ? (sg_get_unaligned_be16(arr + off + 2) + 4) :
                   (arr[off + 1] + 2);
    sub_mpage = spf ? arr[off + 1] : 0;
    if ((pg_len + off) > param_len) {
        resp->asc = PARAMETER_LIST_LENGTH_ERR;
        goto err_out;
    }
    switch (mpage) {
    case 0x8:      /* Caching Mode page */
        if (0x0 == sub_mpage) {
            if (caching_m_pg[1] == arr[off + 1]) {
                memcpy(caching_m_pg + 2, arr + off + 2,
                       sizeof(caching_m_pg) - 2);
                dsp->wce = !!(caching_m_pg[2] & 0x4);
                dsp->wce_changed = true;
                break;
            }
        }
        goto def_case;
    case 0xa:      /* Control Mode page */
        if (0x0 == sub_mpage) {
            if (ctrl_m_pg[1] == arr[off + 1]) {
                memcpy(ctrl_m_pg + 2, arr + off + 2,
                       sizeof(ctrl_m_pg) - 2);
                dsp->scsi_dsense = !!(ctrl_m_pg[2] & 0x4);
                break;
            }
        }
        goto def_case;
    case 0x1c:      /* Informational Exceptions Mode page (SBC) */
        if (0x0 == sub_mpage) {
            if (iec_m_pg[1] == arr[off + 1]) {
                memcpy(iec_m_pg + 2, arr + off + 2,
                       sizeof(iec_m_pg) - 2);
                break;
            }
        }
        goto def_case;
    case 0x0:       /* Vendor specific "Unit Attention" mode page */
        if (vs_ua_m_pg[1] == arr[off + 1]) {
            memcpy(vs_ua_m_pg + 2, arr + off + 2,
                   sizeof(vs_ua_m_pg) - 2);
            dsp->enclosure_override = vs_ua_m_pg[2];
        }
        break;
    default:
def_case:
        resp->asc = INVALID_FIELD_IN_PARAM_LIST;
        resp->in_byte = off;
        resp->in_bit = 5;
        goto err_out;
    }
    return rlen;

err_out:
    resp->sk = SPC_SK_ILLEGAL_REQUEST;
    resp->sstatus = SAM_STAT_CHECK_CONDITION;
    return -1;
}

#endif          /* (HAVE_NVME && (! IGNORE_NVME)) [near line 140] */
