#ifndef SDPARM_H
#define SDPARM_H

/*
 * sdparm is a utility program for getting and setting parameters on devices
 * that use one of the SCSI command sets. In some cases commands can be sent
 * to the device (e.g. eject removable media).
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_MODE_6_RESP_LEN 252
#define DEF_MODE_RESP_LEN 512
#define DEF_INQ_RESP_LEN 252
#define VPD_ATA_INFO_RESP_LEN 572
#define VPD_XCOPY_RESP_LEN 572
#define VPD_LARGE_RESP_LEN 1020

#define MP_DESC_DNC (-1)        /* Do not care indicator */

/* Mode page numbers */
#define UNIT_ATTENTION_MP 0
#define RW_ERR_RECOVERY_MP 1
#define DISCONNECT_MP 2
#define FORMAT_MP 3
#define MRW_MP 3
#define RIGID_DISK_MP 4
#define FLEX_DISK_MP 5
#define WRITE_PARAM_MP 5
#define RBC_DEV_PARAM_MP 6
#define V_ERR_RECOVERY_MP 7
#define CACHING_MP 8
#define CONTROL_MP 0xa
#define NOTCH_MP 0xc    /* Notch and partition (obsolete in sbc4r14) */
#define POWER_OLD_MP 0xd
/* #define CD_DEV_PARAMS 0xd */
#define ADC_MP 0xe
#define DATA_COMPR_MP 0xf
#define DEV_CONF_MP 0x10
#define XOR_MP 0x10
#define MED_PART_MP 0x11
#define ES_MAN_MP 0x14
#define EXTENDED_MP 0x15        /* SUBPG > 0, for all device type */
#define PROT_SPEC_LU_MP 0x18
#define PROT_SPEC_PORT_MP 0x19
#define POWER_MP 0x1a
#define IEC_MP 0x1c
#define MED_CONF_MP 0x1d
#define TIMEOUT_PROT_MP 0x1d
#define ELE_ADDR_ASS_MP 0x1d
#define TRANS_GEO_PAR_MP 0x1e
#define DEV_CAP_MP 0x1f
#define MMCMS_MP 0x2a
#define ALL_MPAGES 0x3f

/* Mode subpage numbers (when SPF bit is set in bit 6, byte 0 of response) */
#define NO_MSP 0                /* SPF is clear (0) in this case */
#define MSP_SPC_CE 1            /* control extension */
#define MSP_SPI_MC 1
#define MSP_SPI_STC 2
#define MSP_SPI_NS 3
#define MSP_SPI_RTC 4
#define MSP_SAS_PCD 1
#define MSP_SAS_SPC 2
#define MSP_SAS_E_PHY 3
#define MSP_SAS_OOB_M_C 4       /* OOB Management Control */
#define MSP_BACK_CTL 1
#define MSP_SAT_AFC 0xf2        /* SAT ATA Feature Control [a,f2] */
#define MSP_SAT_PATA 0xf1       /* SAT PATA Control [a,f1] */
#define MSP_SAT_POWER 0xf1      /* SAT ATA Power condition [1a,f1] */
#define MSP_DEV_CONF_EXT 1      /* device conf extension (ssc) */
#define MSP_EXT_DEV_CAP 0x41    /* extended device capabilities (smc) */
#define MSP_ADC_TGT_DEV 0x1
#define MSP_ADC_DT_DPP 0x2
#define MSP_ADC_LU 0x3
#define MSP_ADC_TD_SN 0x4
#define MSP_SBC_LB_PROV 0x2
#define MSP_SSC_CDP 0xf0
#define MSP_SBC_APP_TAG 0x2     /* changed from 0xf0 to 0x2 sbc3r28 */
#define MSP_SPC_PS 0x1          /* power consumption */
#define MSP_SPC_CDLA 0x3
#define MSP_SPC_CDLB 0x4
#define MSP_SPC_CDLT2A 0x7      /* spc6r01 */
#define MSP_SPC_CDLT2B 0x8      /* spc6r01 */
#define MSP_SBC_IO_ADVI 0x5
#define MSP_SBC_BACK_OP 0x6
#define MSP_ZB_D_CTL 0xf        /* zbc2r04a */

#define MODE_DATA_OVERHEAD 128
#define EBUFF_SZ 256
#define MAX_MP_IT_VAL 128       /* maximum number of items that can be */
                                /* changed in one invocation */
#define MAX_MODE_DATA_LEN 2048

/* VPD pages (fetched by INQUIRY command) */
#define VPD_SUPPORTED_VPDS 0x0
#define VPD_UNIT_SERIAL_NUM 0x80
#define VPD_IMP_OP_DEF 0x81     /* obsolete in SPC-2 */
#define VPD_ASCII_OP_DEF 0x82   /* obsolete in SPC-2 */
#define VPD_DEVICE_ID 0x83
#define VPD_SOFTW_INF_ID 0x84
#define VPD_MAN_NET_ADDR 0x85
#define VPD_EXT_INQ 0x86                /* Extended Inquiry */
#define VPD_MODE_PG_POLICY 0x87
#define VPD_SCSI_PORTS 0x88
#define VPD_ATA_INFO 0x89
#define VPD_POWER_CONDITION 0x8a
#define VPD_DEVICE_CONSTITUENTS 0x8b
#define VPD_CFA_PROFILE_INFO 0x8c
#define VPD_POWER_CONSUMPTION 0x8d
#define VPD_3PARTY_COPY 0x8f
#define VPD_PROTO_LU 0x90
#define VPD_PROTO_PORT 0x91
#define VPD_SCSI_FEATURE_SETS 0x92      /* spc5r11 */
#define VPD_BLOCK_LIMITS 0xb0           /* SBC-3 */
#define VPD_SA_DEV_CAP 0xb0             /* SSC-3 */
#define VPD_OSD_INFO 0xb0               /* OSD */
#define VPD_BLOCK_DEV_CHARS 0xb1        /* SBC-3 */
#define VPD_MAN_ASS_SN 0xb1             /* SSC-3, ADC-2 */
#define VPD_SECURITY_TOKEN 0xb1         /* OSD */
#define VPD_TA_SUPPORTED 0xb2           /* SSC-3 */
#define VPD_LB_PROVISIONING 0xb2        /* SBC-3 */
#define VPD_REFERRALS 0xb3              /* SBC-3 */
#define VPD_AUTOMATION_DEV_SN 0xb3      /* SSC-3 */
#define VPD_SUP_BLOCK_LENS 0xb4         /* sbc4r01 */
#define VPD_DTDE_ADDRESS 0xb4           /* SSC-4 */
#define VPD_BLOCK_DEV_C_EXTENS 0xb5     /* sbc4r02 */
#define VPD_LB_PROTECTION 0xb5          /* SSC-5 */
#define VPD_ZBC_DEV_CHARS 0xb6          /* zbc-r01b */
#define VPD_BLOCK_LIMITS_EXT 0xb7       /* sbc4r08 */
#define VPD_FORMAT_PRESETS 0xb8         /* sbc4r18 */
#define VPD_CON_POS_RANGE 0xb9          /* 20-089r2 */
#define VPD_NOT_STD_INQ -2      /* request for standard inquiry */

#define VPD_ASSOC_LU 0
#define VPD_ASSOC_TPORT 1
#define VPD_ASSOC_TDEVICE 2

/* values are 2**vpd_assoc */
#define VPD_DI_SEL_LU 1
#define VPD_DI_SEL_TPORT 2
#define VPD_DI_SEL_TARGET 4
#define VPD_DI_SEL_AS_IS 32

#ifndef SG_NVME_VPD_NICR
#define SG_NVME_VPD_NICR 0xde   /* NVME Identify Controller Response */
#endif

#define DEF_TRANSPORT_PROTOCOL TPROTO_SAS

/* Vendor identifiers, only for mode pages, not VPD pages */
#define VENDOR_SEAGATE 0x0
#define VENDOR_HITACHI 0x1
#define VENDOR_MAXTOR 0x2
#define VENDOR_FUJITSU 0x3
#define VENDOR_NONE 0x4
#define VENDOR_LTO5 0x5
#define VENDOR_LTO6 0x6
#define VENDOR_NVME 0x7
#define VENDOR_SG 0x8

/* bit flag settings for sdparm_mode_page_item::flags */
#define MF_COMMON 0x1   /* output in summary mode */
#define MF_HEX 0x2
#define MF_CLASH_OK 0x4 /* know this overlaps safely with generic */
#define MF_TWOS_COMP 0x8   /* integer is two's complement */
#define MF_ALL_1S 0x10     /* even with MF_HEX output all ones as -1 */
#define MF_SAVE_PGS 0x20   /* advise/require --save option */
#define MF_DESC_ID_B0 0x100      /* mpage descriptor ID, bit 0 */
#define MF_DESC_ID_B1 0x200
#define MF_DESC_ID_B2 0x400
#define MF_DESC_ID_B3 0x800
#define MF_DESC_ID_MASK 0xf00
#define MF_DESC_ID_SHIFT 8

/* Output (bit) mask values */
#define MP_OM_CUR 0x1
#define MP_OM_CHA 0x2
#define MP_OM_DEF 0x4
#define MP_OM_SAV 0x8
#define MP_OM_ALL 0xf

/* enumerations for commands */
#define CMD_READY 1
#define CMD_START 2
#define CMD_STOP 3
#define CMD_LOAD 4
#define CMD_EJECT 5
#define CMD_UNLOCK 6
#define CMD_SENSE 7
#define CMD_SYNC 8
#define CMD_CAPACITY 9
#define CMD_SPEED 10
#define CMD_PROFILE 11

/* squeeze two PDTs into one field, must not use PDT_DISK as upper */
#define PDT_DISK_ZBC (PDT_DISK | (PDT_ZBC << 8))


/* Mainly command line options */
struct sdparm_opt_coll {
    bool dbd;
    bool dummy;
    bool examine;
    bool flexible;
    bool inquiry;
    bool mode_6;        /* false (default) for Mode Sense or Select(10) */
    bool num_desc;      /* report number of descriptors */
    bool do_raw;        /* -R (usually '-r' but already used) */
    bool read_only;
    bool save;
    bool verbose_given;
    bool version_given;
    int defaults;       /* set mode page to its default values, or when set
                         * twice set RTD bit to set defaults on all pages */
    int do_all;         /* -iaa outputs all VPD pages found in the Supported
                         * VPD Pages VPD page (0x0) */
    int do_enum;
    int do_hex;
    int do_long;
    int out_mask;       /* OR-ed MP_OM_* values, default: MP_OM_ALL (0xf) */
    int pdt;
    int do_quiet;
    int transport;      /* -1 means not transport specific (def) */
    int vendor_id;      /* -1 means not vendor specific (def) */
    int verbose;
    const char * inhex_fn;
};

/* Instances and arrays of the following templates are mainly found in the
 * sdparm_data.c file. */

/* Template for mode pages that use descriptor format; forms in use:
 *                         Fixed descriptor Length
 *                         =======================
 *    num_descs_off      >=0                  >=0
 *    num_descs_inc      >=0                   -1
 *    desc_len            >0                   >0
 *    desc_len_off        -1                   -1
 *    have_desc_id       false                false
 *    ---------------------------------------------------
 *    Notes            number of descs    length of descs
 *                     in mpage           in mpage
 *                         ^                    ^
 *                         |                    |
 *    Examples:          pcd (SAS/SPL)        lbp, atag (SBC)
 *
 *
 *                      Variable descriptor Length
 *                      ==========================
 *    num_descs_off      >=0                   -1
 *    num_descs_inc      >=0                    0
 *    desc_len            -1                   -1
 *    desc_len_off       >=0                  >=0
 *    have_desc_id       false                true
 *    ---------------------------------------------------
 *    Notes         number of descs and   only length of descs
 *                  and length in mpage   in mpage
 *                         ^                    ^
 *                         |                    |
 *    Examples:          sep (SAS/SPL)        oobm (SAS/SPL)
 *
 * The last one (i.e. have_desc_id=true) is the most complex. It can have
 * many descriptors, one or more of one desc_id and one or more of another
 * desc_id. And descriptors with different desc_id_s can have different
 * lengths and different fields (items).                    */
struct sdparm_mode_descriptor_t {
    int num_descs_off;    /* byte offset of start of num_descriptors */
    int num_descs_bytes;  /* number of bytes in num_descriptors field */
    int num_descs_inc;    /* number to add to num_descriptors */
                          /* if negative then value in num_descriptors */
                          /* is byte count, so divide by desc_len */
    int first_desc_off;
    int desc_len;         /* -1 for unknown otherwise fixed per desc */
    int desc_len_off;     /* if (-1 == desc_len) then this is offset */
    int desc_len_bytes;   /* ... after start of descriptor */
    /* Hence: <desc_len> = deref(base + d_len_off, d_len_bytes) + */
    /*                     d_len_off + d_len_bytes */
    bool have_desc_id;    /* descriptor has 4 bit ID, byte 0, bits 3 to 0 */
    const char * name;
};

/* Template for each mode page, array populated in sdparm_data.c for generic
 * and transport mpages. Vendor mode pages found in sdparm_data_vendor.c . */
struct sdparm_mode_page_t {
    int page;
    int subpage;
    int pdt_s;       /* compound peripheral device type id, -1 is the default
                      * for fields defined in SPC (common to all PDTs).
                      * Compound pdt_s may hold two PDTs. The most common
                      * example is:
                      *    PDT_DISK | (PDT_ZBC << 8)    */
    int ro;          /* read-only */
    const char * acron;
    const char * name;
    const struct sdparm_mode_descriptor_t * mp_desc;
                    /* non-NULL when mpage has descriptor format */
};

/* Template for each VPD page, array populated in sdparm_data.c */
struct sdparm_vpd_page_t {
    int vpd_num;
    int subvalue;
    int pdt_s;       /* see pdt_s explanation above */
    const char * acron;
    const char * name;
};

/* Template for each mode/VPD page vendor, array populated in
 * sdparm_data_vendor.c */
struct sdparm_vendor_name_t {
    int vendor_id;
    const char * acron;
    const char * name;
};

/* Template for each mode page field (item), arrays for generic and each
 * transport populated in sdparm_data.c . Arrays for vendors populated
 * in sdparm_data_vendor.c */
struct sdparm_mode_page_item {
    const char * acron;
    int pg_num;
    int subpg_num;
    int pdt_s;       /* see pdt_s explanation above */
    int start_byte;
    int start_bit;
    int num_bits;
    int flags;       /* bit settings or-ed, see MF_* */
    const char * description;
    const char * extra;
};

/* Command line arguments to --clear, --get= and --set= are parsed into
 * one or more instances of this structure. On the command line each
 * has the form: <mitem>[.<d_num>][=<val>]
 * struct sdparm_mode_page_settings contains an array of these. */
struct sdparm_mode_page_it_val {
    struct sdparm_mode_page_item mpi;   /* holds <mitem> in above form */
    int64_t val;        /* holds <val> in above form. Defaults to 1 for
                         * for --set=, and to 0 for --clear= and --get= .
                         * The <val> for --get= is for output format. */
    int64_t orig_val;   /* what Mode sense indicates is currently in that
                         * <mitem>[.<d_num>] . If same, nothing to do */
    int descriptor_num; /* holds <d_num> in the above form. Defaults to 0
                         * which indicates the first descriptor. 1 corresponds
                         * to second descriptor, etc. */
};

/* Instance holds the argument to --clear=, --get= or --set= plus the
 * mode page number and sub-page number. That argument could be empty, a
 * single <mitem>[.<d_num>][=<val>] instance or a comma separated list of
 * them. */
struct sdparm_mode_page_settings {
    int pg_num;
    int subpg_num;
    struct sdparm_mode_page_it_val it_vals[MAX_MP_IT_VAL];
    int num_it_vals;    /* number of active elements in it_vals[] */
};

/* Template for a transport's mode pages and fields. Array of these in
 * sdparm_data.c . Index of array corresponds to SCSI transport protocol id
 * (which is a number from 0 to 15). Undefined or unsupported entries
 * contain NULL, NULL. */
struct sdparm_transport_pair {
    struct sdparm_mode_page_t * mpage;          /* array of transport specific
                                                   mode pages */
    struct sdparm_mode_page_item * mitem;       /* array of transport specific
                                                   mode page fields (items) */
};

/* Template for a vendor's mode pages and fields. Array of these found in
 * sdparm_data_vendor.c . */
struct sdparm_vendor_pair {
    struct sdparm_mode_page_t * mpage;
    struct sdparm_mode_page_item * mitem;
};

/* Template for a simple SCSI command supported by sdparm. Array of them
 * found in sdparm_data.c */
struct sdparm_command_t {
    int cmd_num;
    const char * name;
    const char * min_abbrev;
    const char * extra_arg;
};

/* Simple value and description pair */
struct sdparm_val_desc_t {
        int val;
        const char * desc;
};

extern struct sdparm_mode_page_t sdparm_gen_mode_pg[];
extern struct sdparm_vpd_page_t sdparm_vpd_pg[];
extern struct sdparm_val_desc_t sdparm_transport_id[];
extern struct sdparm_val_desc_t sdparm_add_transport_acron[];
extern struct sdparm_transport_pair sdparm_transport_mp[];
extern struct sdparm_vendor_name_t sdparm_vendor_id[];
extern struct sdparm_vendor_pair sdparm_vendor_mp[];
extern const int sdparm_vendor_mp_len;
extern struct sdparm_mode_page_item sdparm_mitem_arr[];
extern struct sdparm_command_t sdparm_command_arr[];
extern struct sdparm_val_desc_t sdparm_profile_arr[];

extern const char * sdparm_ansi_version_arr[];
extern const char * sdparm_network_service_type_arr[];
extern const char * sdparm_mode_page_policy_arr[];


int sdp_mpage_len(const uint8_t * mp);    /* page, not MS response */
const struct sdparm_mode_page_t * sdp_get_mpage_t(int page_num,
                int subpage_num, int pdt, int transp_proto, int vendor_num);
const struct sdparm_mode_page_t * sdp_get_mpt_with_str(int page_num,
                int subpage_num, int pdt, int transp_proto, int vendor_num,
                bool plus_acron, bool hex, int max_b_len, char * bp);
const struct sdparm_mode_page_t * sdp_find_mpt_by_acron(const char * ap,
                int transp_proto, int vendor_num);
const struct sdparm_vpd_page_t * sdp_get_vpd_detail(int page_num,
                int subvalue, int pdt);
const struct sdparm_vpd_page_t * sdp_find_vpd_by_acron(const char * ap);
char * sdp_get_transport_name(int proto_num, int b_len, char * b);
/* sdp_find_transport_id_by_acron() returns -1 for not found */
int sdp_find_transport_id_by_acron(const char * ap);
const char * sdp_get_vendor_name(int vendor_num);
const struct sdparm_vendor_name_t * sdp_find_vendor_by_acron(const char * ap);
const struct sdparm_vendor_pair * sdp_get_vendor_pair(int vendor_num);
const struct sdparm_mode_page_item * sdp_find_mitem_by_acron(const char * ap,
                int * from, int transp_proto, int vendor_num);
uint64_t sdp_mitem_get_value(const struct sdparm_mode_page_item *mpi,
                             const uint8_t * mp);
uint64_t sdp_mitem_get_value_check(const struct sdparm_mode_page_item *mpi,
                                   const uint8_t * mp, bool * all_setp);
void sdp_print_signed_decimal(uint64_t u, int num_bits, bool leading_zeros);
void sdp_mitem_set_value(uint64_t val, const struct sdparm_mode_page_item *mpi,
                         uint8_t * mp);
char * sdp_get_ansi_version_str(int version, int buff_len, char * buff);
int sdp_get_desc_id(int flags);
int sdp_strcase_eq(const char * s1p, const char * s2p);
int sdp_strcase_eq_upto(const char * s1p, const char * s2p, int n);

/*
 * Declarations for functions found in sdparm_vpd.c
 */

int sdp_process_vpd_page(int sg_fd, int pn, int spn,
                         const struct sdparm_opt_coll * op, int req_pdt,
                         bool protect, const uint8_t * ihbp, int ihb_len,
                         uint8_t * alt_buf, int off);

/*
 * Declarations for functions found in sdparm_cmd.c
 */

const struct sdparm_command_t * sdp_build_cmd(const char * cmd_str,
                                              bool * rwp, int * argp);
void sdp_enumerate_commands();
int sdp_process_cmd(int sg_fd, const struct sdparm_command_t * scmdp,
                    int cmd_arg, int pdt, const struct sdparm_opt_coll * opts);

/* Must not have PDT_DISK as upper byte of mask */
#define PDT_LOWER_MASK 0xff
#define PDT_UPPER_MASK (~PDT_LOWER_MASK)

/* Returns true if left argument is "equal" to the right argument. l_pdt_s
 * is a compound PDT (SCSI Peripheral Device Type) or a negative number
 * which represents a wildcard (i.e. match anything). r_pdt_s has a similar
 * form. PDT values are 5 bits long (0 to 31) and a compound pdt_s is
 * formed by shifting the second (upper) PDT by eight bits to the left and
 * OR-ing it with the first PDT. The pdt_s values must be defined so
 * PDT_DISK (0) is _not_ the upper value in a compound pdt_s. */
bool pdt_s_eq(int l_pdt_s, int r_pdt_s);

/*
 * Declarations for functions that are port dependent
 */

#ifdef SG_LIB_WIN32

int sg_do_wscan(char letter, int do_scan, int verbose);

#endif

#ifdef __cplusplus
}
#endif

#endif
