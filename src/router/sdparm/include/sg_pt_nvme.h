#ifndef SG_PT_NVME_H
#define SG_PT_NVME_H

/*
 * Copyright (c) 2017-2019 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* structures copied and slightly modified from <linux/nvme_ioctl.h> which
 * is Copyright (c) 2011-2014, Intel Corporation.  */


/* Note that the command input structure is in (packed) "cpu" format. That
 * means, for example, if the CPU is little endian (most are) then so is the
 * structure. However what comes out in the data-in buffer (e.g. for the
 * Admin Identify command response) is almost all little endian following ATA
 * (but no SCSI and IP which are big endian) and Intel's preference. There
 * are exceptions, for example the EUI-64 identifiers in the Admin Identify
 * response are big endian.
 *
 * Code online (e.g. nvme-cli at github.com) seems to favour packed
 * structures, while the author prefers byte offset plus a range of unaligned
 * integer builders such as those in sg_unaligned.h .
 */

#ifdef __GNUC__
#ifndef __clang__
  struct __attribute__((__packed__)) sg_nvme_user_io
#else
  struct sg_nvme_user_io
#endif
#else
struct sg_nvme_user_io
#endif
{
        uint8_t opcode;
        uint8_t flags;
        uint16_t control;
        uint16_t nblocks;
        uint16_t rsvd;
        uint64_t metadata;
        uint64_t addr;
        uint64_t slba;
        uint32_t dsmgmt;
        uint32_t reftag;
        uint16_t apptag;
        uint16_t appmask;
}
#ifdef SG_LIB_FREEBSD
__packed;
#else
;
#endif

/* Using byte offsets and unaligned be/le copies safer than packed
 * structures. These are for sg_nvme_user_io . */
#define SG_NVME_IO_OPCODE 0
#define SG_NVME_IO_FLAGS 1
#define SG_NVME_IO_CONTROL 2
#define SG_NVME_IO_NBLOCKS 4
#define SG_NVME_IO_RSVD 6
#define SG_NVME_IO_METADATA 8
#define SG_NVME_IO_ADDR 16
#define SG_NVME_IO_SLBA 24
#define SG_NVME_IO_DSMGMT 32
#define SG_NVME_IO_REFTAG 36
#define SG_NVME_IO_APPTAG 40
#define SG_NVME_IO_APPMASK 42

#ifdef __GNUC__
#ifndef __clang__
  struct __attribute__((__packed__)) sg_nvme_passthru_cmd
#else
  struct sg_nvme_passthru_cmd
#endif
#else
struct sg_nvme_passthru_cmd
#endif
{
        uint8_t opcode;
        uint8_t flags;
        uint16_t rsvd1;
        uint32_t nsid;
        uint32_t cdw2;
        uint32_t cdw3;
        uint64_t metadata;
        uint64_t addr;
        uint32_t metadata_len;
        uint32_t data_len;
        uint32_t cdw10;
        uint32_t cdw11;
        uint32_t cdw12;
        uint32_t cdw13;
        uint32_t cdw14;
        uint32_t cdw15;
#ifdef SG_LIB_LINUX
        uint32_t timeout_ms;
        uint32_t result;        /* out: DWord(0) from completion queue */
#endif
}
#ifdef SG_LIB_FREEBSD
__packed;
#else
;
#endif

struct sg_sntl_dev_state_t {
    uint8_t scsi_dsense;
    uint8_t enclosure_override; /* ENC_OV in sdparm */
    uint8_t pdt;        /* 6 bit value in INQUIRY response */
    uint8_t enc_serv;   /* single bit in INQUIRY response */
    uint8_t id_ctl253;  /* NVMSR field of Identify controller (byte 253) */
    bool wce;		/* Write Cache Enable (WCE) setting */
    bool wce_changed;	/* WCE setting has been changed */
};

struct sg_sntl_result_t {
    uint8_t sstatus;
    uint8_t sk;
    uint8_t asc;
    uint8_t ascq;
    uint8_t in_byte;
    uint8_t in_bit;     /* use 255 for 'no bit position given' */
};

struct sg_opcode_info_t {
    uint8_t opcode;
    uint16_t sa;            /* service action, 0 for none */
    uint32_t flags;         /* OR-ed set of F_* flags */
    uint8_t len_mask[16];   /* len=len_mask[0], then mask for cdb[1]... */
                            /* ignore cdb bytes after position 15 */
};

/* Using byte offsets and unaligned be/le copies safer than packed
 * structures. These are for sg_nvme_passthru_cmd . */
#define SG_NVME_PT_OPCODE 0             /* length: 1 byte */
#define SG_NVME_PT_FLAGS 1              /* length: 1 byte */
#define SG_NVME_PT_RSVD1 2              /* length: 2 bytes */
#define SG_NVME_PT_NSID 4               /* length: 4 bytes */
#define SG_NVME_PT_CDW2 8               /* length: 4 bytes */
#define SG_NVME_PT_CDW3 12              /* length: 4 bytes */
#define SG_NVME_PT_METADATA 16          /* length: 8 bytes */
#define SG_NVME_PT_ADDR 24              /* length: 8 bytes */
#define SG_NVME_PT_METADATA_LEN 32      /* length: 4 bytes */
#define SG_NVME_PT_DATA_LEN 36          /* length: 4 bytes */
#define SG_NVME_PT_CDW10 40             /* length: 4 bytes */
#define SG_NVME_PT_CDW11 44             /* length: 4 bytes */
#define SG_NVME_PT_CDW12 48             /* length: 4 bytes */
#define SG_NVME_PT_CDW13 52             /* length: 4 bytes */
#define SG_NVME_PT_CDW14 56             /* length: 4 bytes */
#define SG_NVME_PT_CDW15 60             /* length: 4 bytes */

#ifdef SG_LIB_LINUX
/* General references state that "all NVMe commands are 64 bytes long". If
 * so then the following are add-ons by Linux, go to the OS and not the
 * the NVMe device. */
#define SG_NVME_PT_TIMEOUT_MS 64        /* length: 4 bytes */
#define SG_NVME_PT_RESULT 68            /* length: 4 bytes */
#endif

/* Byte offset of Result and Status (plus phase bit) in CQ */
#define SG_NVME_PT_CQ_RESULT 0          /* CDW0, length: 4 bytes */
#define SG_NVME_PT_CQ_DW0 0             /* CDW0, length: 4 bytes */
#define SG_NVME_PT_CQ_DW1 4             /* CDW1, length: 4 bytes */
#define SG_NVME_PT_CQ_DW2 8             /* CDW2, length: 4 bytes */
#define SG_NVME_PT_CQ_DW3 12            /* CDW3, length: 4 bytes */
#define SG_NVME_PT_CQ_STATUS_P 14       /* CDW3 31:16, length: 2 bytes */


/* Valid namespace IDs (nsid_s) range from 1 to 0xfffffffe, leaving: */
#define SG_NVME_BROADCAST_NSID 0xffffffff       /* all namespaces */
#define SG_NVME_CTL_NSID 0x0            /* the "controller's" namespace */

/* Vendor specific (sg3_utils) VPD pages */
#define SG_NVME_VPD_NICR 0xde   /* NVME Identify controller response */


/* Given the NVMe Identify Controller response and optionally the NVMe
 * Identify Namespace response (NULL otherwise), generate the SCSI VPD
 * page 0x83 (device identification) descriptor(s) in dop. Return the
 * number of bytes written which will not exceed max_do_len. Probably use
 * Peripheral Device Type (pdt) of 0 (disk) for don't know. Transport
 * protocol (tproto) should be -1 if not known, else SCSI value.
 * N.B. Does not write total VPD page length into dop[2:3] . */
int sg_make_vpd_devid_for_nvme(const uint8_t * nvme_id_ctl_p,
                               const uint8_t * nvme_id_ns_p, int pdt,
                               int tproto, uint8_t * dop, int max_do_len);

/* Initialize dev_stat pointed to by dsp */
void sntl_init_dev_stat(struct sg_sntl_dev_state_t * dsp);

/* Internal function (common to all OSes) to support the SNTL SCSI MODE
 * SENSE(10) command. Has a vendor specific Unit Attention mpage which
 * has only one field currently: ENC_OV (enclosure override) */
int sntl_resp_mode_sense10(const struct sg_sntl_dev_state_t * dsp,
                           const uint8_t * cdbp, uint8_t * dip, int mx_di_len,
                           struct sg_sntl_result_t * resp);

/* Internal function (common to all OSes) to support the SNTL SCSI MODE
 * SELECT(10) command. */
int sntl_resp_mode_select10(struct sg_sntl_dev_state_t * dsp,
                            const uint8_t * cdbp, const uint8_t * dop,
                            int do_len, struct sg_sntl_result_t * resp);

/* Returns pointer to array of struct sg_opcode_info_t of SCSI commands
 * translated to NVMe. */
const struct sg_opcode_info_t * sg_get_opcode_translation(void);

#ifdef __cplusplus
}
#endif

#endif          /* SG_PT_NVME_H */
