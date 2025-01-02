/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * This file contains defines for the LAP interface
 */
#ifndef __CVMX_LAP_H__
#define __CVMX_LAP_H__

#define CVMX_LAP_MAX_LAPS (2)
#define CVMX_LAP_QUEUES_PER_LAP 3
#define CVMX_LAP_MAX_LAB_SIZE 128
#define CVMX_LAP_MIN_LAB_SIZE 64

#define CVMX_LAPX_DID(x) (0x0C+x)

typedef enum {
    CVMX_LAP_SEND = 8,
    CVMX_LAP_RTN,
} cvmx_lap_cmd_e_t;

typedef enum {
    CVMX_LAP_NONE = 0x0,
    CVMX_LAP_TERMINATE = 0x9,
    CVMX_LAP_EOP_SKIP = 0xC,
    CVMX_LAP_PARITY = 0xD,
    CVMX_LAP_LAB_OUT = 0x21,
    CVMX_LAP_LAB_INVALID = 0x22,
    CVMX_LAP_RESP_TIME = 0x23,
    CVMX_LAP_MISMATCH = 0x24,
    CVMX_LAP_XID_BAD = 0x25,
    CVMX_LAP_TOO_BIG = 0x26
} cvmx_lap_err_e_t;

#define CVMX_LAP_MAX_EXCEPTION_REGS (16)
typedef struct  {
    int lab_size;
    int ooo;
    int max_labs[CVMX_LAP_QUEUES_PER_LAP];
    cvmx_lapx_xid_pos_t xid_pos;
    uint64_t exp_valid[CVMX_LAP_MAX_EXCEPTION_REGS];
    uint64_t exp_data[CVMX_LAP_MAX_EXCEPTION_REGS];
} cvmx_lap_config_t;

typedef union {
    uint64_t u64;
    struct {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t scraddr:8;
        uint64_t rtnlen:8;
        uint64_t did:8;
        uint64_t node:4;
        uint64_t cmd:4;
        uint64_t df:1;
        uint64_t ds:1;
        uint64_t eq:1;
        uint64_t chan:1;
        uint64_t ito:1;
        uint64_t rsvd:27;
#else
        uint64_t rsvd:27;
        uint64_t ito:1;
        uint64_t chan:1;
        uint64_t eq:1;
        uint64_t ds:1;
        uint64_t df:1;
        uint64_t cmd:4;
        uint64_t node:4;
        uint64_t did:8;
        uint64_t rtnlen:8;
        uint64_t scraddr:8;
#endif
    } s;
} cvmx_lap_send_lmtdma_t;


typedef union {
    uint64_t u64;
    struct {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t scraddr:8;
        uint64_t rtnlen:8;
        uint64_t did:8;
        uint64_t node:4;
        uint64_t cmd:4;
        uint64_t df:1;
        uint64_t rsvd1:3;
        uint64_t ito:1;
        uint64_t rsvd2:3;
        uint64_t lab:8;
        uint64_t rsvd3:9;
        uint64_t offset:4;
        uint64_t rsvd4:3;
#else
        uint64_t rsvd4:3;
        uint64_t offset:4;
        uint64_t rsvd3:9;
        uint64_t lab:8;
        uint64_t rsvd2:3;
        uint64_t ito:1;
        uint64_t rsvd1:3;
        uint64_t df:1;
        uint64_t cmd:4;
        uint64_t node:4;
        uint64_t did:8;
        uint64_t rtnlen:8;
        uint64_t scraddr:8;
#endif
    } s;
} cvmx_lap_rd_iobdma_t;


typedef union {
    uint64_t u64;
    struct {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t rsvd1:7;
        uint64_t as0:15;
        uint64_t rsvd2:2;
        uint64_t proto:1;
        uint64_t as1:6;
        uint64_t chan:1;
        uint64_t as2:8;
        uint64_t rsvd3:24;
#else
        uint64_t rsvd3:24;
        uint64_t as2:8;
        uint64_t chan:1;
        uint64_t as1:6;
        uint64_t proto:1;
        uint64_t rsvd2:2;
        uint64_t as0:15;
        uint64_t rsvd1:7;
#endif
    } s;
} cvmx_lap_ctl_req_t;

typedef union {
    uint64_t u64;
    struct {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t rsvd1:2;
        uint64_t length:5;
        uint64_t as0:15;
        uint64_t rsvd2:2;
        uint64_t proto:1;
        uint64_t as1:6;
        uint64_t chan:1;
        uint64_t as2:8;
        uint64_t timeout:1;
        uint64_t eqne:1;
        uint64_t err:6;
        uint64_t rsvd3:8;
        uint64_t lab:8;
#else
        uint64_t lab:8;
        uint64_t rsvd3:8;
        uint64_t err:6;
        uint64_t eqne:1;
        uint64_t timeout:1;
        uint64_t as2:8;
        uint64_t chan:1;
        uint64_t as1:6;
        uint64_t proto:1;
        uint64_t rsvd2:2;
        uint64_t as0:15;
        uint64_t length:5;
        uint64_t rsvd1:2;
#endif
    } s;
} cvmx_lap_ctl_rtn_t;


int cvmx_lap_soft_reset(int lap_num);
int cvmx_lap_init(int lap_num, cvmx_lap_config_t *lap_config);
int cvmx_lap_fill_eq(int lap_num, int num_bufs);
int  cvmx_lap_mgr_get_lab(int lap_num);
void cvmx_lap_mgr_put_lab(int lap_num);
int cvmx_lap_send_lmtdma(int lap_num,  int channel, void * req_ptr, int req_len, 
			 void * rsp_ptr, int rsp_size);
int cvmx_lap_send_lmtdma_ito(int lap_num, int channel, void * req_ptr, int req_len);
int cvmx_lap_read_iobdma(int lap_num, int lab_id, int offset, void * rsp_ptr, int rsp_size);
#endif /* __CVMX_LAP_H__ */
