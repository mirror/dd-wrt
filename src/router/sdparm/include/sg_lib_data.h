#ifndef SG_LIB_DATA_H
#define SG_LIB_DATA_H

/*
 * Copyright (c) 2007-2019 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * This header file contains some structure declarations and array name
 * declarations which are defined in the sg_lib_data.c .
 * Typically this header does not need to be exposed to users of the
 * sg_lib interface declared in sg_libs.h .
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Operation codes with associated service actions that change or qualify
 * the command name */
#define SG_EXTENDED_COPY 0x83 /* since spc4r34 became next entry */
#define SG_3PARTY_COPY_OUT 0x83 /* new in spc4r34: Third party copy out */
#define SG_RECEIVE_COPY 0x84  /* since spc4r34 became next entry */
#define SG_3PARTY_COPY_IN 0x84 /* new in spc4r34: Third party copy in */
#define SG_MAINTENANCE_IN 0xa3
#define SG_MAINTENANCE_OUT 0xa4
#define SG_PERSISTENT_RESERVE_IN 0x5e
#define SG_PERSISTENT_RESERVE_OUT 0x5f
#define SG_READ_ATTRIBUTE 0x8c
#define SG_READ_BUFFER 0x3c     /* now READ BUFFER(10) */
#define SG_READ_BUFFER_16 0x9b
#define SG_READ_POSITION 0x34   /* SSC command with service actions */
#define SG_SANITIZE 0x48
#define SG_SERVICE_ACTION_BIDI 0x9d
#define SG_SERVICE_ACTION_IN_12 0xab
#define SG_SERVICE_ACTION_IN_16 0x9e
#define SG_SERVICE_ACTION_OUT_12 0xa9
#define SG_SERVICE_ACTION_OUT_16 0x9f
#define SG_VARIABLE_LENGTH_CMD 0x7f
#define SG_WRITE_BUFFER 0x3b
#define SG_ZONING_OUT 0x94
#define SG_ZONING_IN 0x95



struct sg_lib_simple_value_name_t {
    int value;
    const char * name;
};

struct sg_lib_value_name_t {
    int value;
    int peri_dev_type; /* 0 -> SPC and/or PDT_DISK, >0 -> PDT */
    const char * name;
};

struct sg_value_2names_t {
    int value;
    const char * name;
    const char * name2;
};

struct sg_lib_asc_ascq_t {
    uint8_t asc;          /* additional sense code */
    uint8_t ascq;         /* additional sense code qualifier */
    const char * text;
};

struct sg_lib_asc_ascq_range_t {
    uint8_t asc;          /* additional sense code (ASC) */
    uint8_t ascq_min;     /* ASCQ minimum in range */
    uint8_t ascq_max;     /* ASCQ maximum in range */
    const char * text;
};

/* First use: SCSI status, sense_key, asc, ascq tuple */
struct sg_lib_4tuple_u8 {
    uint8_t t1;
    uint8_t t2;
    uint8_t t3;
    uint8_t t4;
};

struct sg_cmd_response_t {
    int din_len;
    int dout_len;
    int resid;
    int resid2;
    const uint8_t * sbp;
};


extern const char * sg_lib_version_str;

extern struct sg_lib_value_name_t sg_lib_normal_opcodes[];
extern struct sg_lib_value_name_t sg_lib_read_buff_arr[];
extern struct sg_lib_value_name_t sg_lib_write_buff_arr[];
extern struct sg_lib_value_name_t sg_lib_maint_in_arr[];
extern struct sg_lib_value_name_t sg_lib_maint_out_arr[];
extern struct sg_lib_value_name_t sg_lib_pr_in_arr[];
extern struct sg_lib_value_name_t sg_lib_pr_out_arr[];
extern struct sg_lib_value_name_t sg_lib_sanitize_sa_arr[];
extern struct sg_lib_value_name_t sg_lib_serv_in12_arr[];
extern struct sg_lib_value_name_t sg_lib_serv_out12_arr[];
extern struct sg_lib_value_name_t sg_lib_serv_in16_arr[];
extern struct sg_lib_value_name_t sg_lib_serv_out16_arr[];
extern struct sg_lib_value_name_t sg_lib_serv_bidi_arr[];
extern struct sg_lib_value_name_t sg_lib_xcopy_sa_arr[];
extern struct sg_lib_value_name_t sg_lib_rec_copy_sa_arr[];
extern struct sg_lib_value_name_t sg_lib_variable_length_arr[];
extern struct sg_lib_value_name_t sg_lib_zoning_out_arr[];
extern struct sg_lib_value_name_t sg_lib_zoning_in_arr[];
extern struct sg_lib_value_name_t sg_lib_read_attr_arr[];
extern struct sg_lib_value_name_t sg_lib_read_pos_arr[];
extern struct sg_lib_asc_ascq_range_t sg_lib_asc_ascq_range[];
extern struct sg_lib_asc_ascq_t sg_lib_asc_ascq[];
extern struct sg_lib_value_name_t sg_lib_scsi_feature_sets[];
extern const char * sg_lib_sense_key_desc[];
extern const char * sg_lib_pdt_strs[];
extern const char * sg_lib_transport_proto_strs[];
extern int sg_lib_pdt_decay_arr[];

extern struct sg_lib_simple_value_name_t sg_lib_nvme_admin_cmd_arr[];
extern struct sg_lib_simple_value_name_t sg_lib_nvme_nvm_cmd_arr[];
extern struct sg_lib_value_name_t sg_lib_nvme_cmd_status_arr[];
extern struct sg_lib_4tuple_u8 sg_lib_scsi_status_sense_arr[];

extern struct sg_value_2names_t sg_exit_str_arr[];

#ifdef __cplusplus
}
#endif

#endif
