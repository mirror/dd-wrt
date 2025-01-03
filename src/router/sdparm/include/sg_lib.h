#ifndef SG_LIB_H
#define SG_LIB_H

/*
 * Copyright (c) 2004-2021 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 *
 * On 5th October 2004 a FreeBSD license was added to this file.
 * The intention is to keep this file and the related sg_lib.c file
 * as open source and encourage their unencumbered use.
 *
 * Current version number of this library is in the sg_lib_data.c file and
 * can be accessed with the sg_lib_version() function.
 */


/*
 * This header file contains defines and function declarations that may
 * be useful to applications that communicate with devices that use a
 * SCSI command set. These command sets have names like SPC-4, SBC-3,
 * SSC-3, SES-2 and draft standards defining them can be found at
 * http://www.t10.org . Virtually all devices in the Linux SCSI subsystem
 * utilize SCSI command sets. Many devices in other Linux device subsystems
 * utilize SCSI command sets either natively or via emulation (e.g. a
 * SATA disk in a USB enclosure).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SCSI Peripheral Device Types (PDT) [5 bit field] */
#define PDT_DISK 0x0    /* direct access block device (disk) */
#define PDT_TAPE 0x1    /* sequential access device (magnetic tape) */
#define PDT_PRINTER 0x2 /* printer device (see SSC-1) */
#define PDT_PROCESSOR 0x3       /* processor device (e.g. SAFTE device) */
#define PDT_WO 0x4      /* write once device (some optical disks) */
#define PDT_MMC 0x5     /* CD/DVD/BD (multi-media) */
#define PDT_SCANNER 0x6 /* obsolete */
#define PDT_OPTICAL 0x7 /* optical memory device (some optical disks) */
#define PDT_MCHANGER 0x8        /* media changer device (e.g. tape robot) */
#define PDT_COMMS 0x9   /* communications device (obsolete) */
#define PDT_SAC 0xc     /* storage array controller device */
#define PDT_SES 0xd     /* SCSI Enclosure Services (SES) device */
#define PDT_RBC 0xe     /* Reduced Block Commands (simplified PDT_DISK) */
#define PDT_OCRW 0xf    /* optical card read/write device */
#define PDT_BCC 0x10    /* bridge controller commands */
#define PDT_OSD 0x11    /* Object Storage Device (OSD) */
#define PDT_ADC 0x12    /* Automation/drive commands (ADC) */
#define PDT_SMD 0x13    /* Security Manager Device (SMD) */
#define PDT_ZBC 0x14    /* Zoned Block Commands (ZBC) */
#define PDT_WLUN 0x1e   /* Well known logical unit (WLUN) */
#define PDT_UNKNOWN 0x1f        /* Unknown or no device type */

#ifndef SAM_STAT_GOOD
/* The SCSI status codes as found in SAM-4 at www.t10.org */
#define SAM_STAT_GOOD 0x0
#define SAM_STAT_CHECK_CONDITION 0x2
#define SAM_STAT_CONDITION_MET 0x4
#define SAM_STAT_BUSY 0x8
#define SAM_STAT_INTERMEDIATE 0x10                /* obsolete in SAM-4 */
#define SAM_STAT_INTERMEDIATE_CONDITION_MET 0x14  /* obsolete in SAM-4 */
#define SAM_STAT_RESERVATION_CONFLICT 0x18
#define SAM_STAT_COMMAND_TERMINATED 0x22          /* obsolete in SAM-3 */
#define SAM_STAT_TASK_SET_FULL 0x28
#define SAM_STAT_ACA_ACTIVE 0x30
#define SAM_STAT_TASK_ABORTED 0x40
#endif

/* The SCSI sense key codes as found in SPC-4 at www.t10.org */
#define SPC_SK_NO_SENSE 0x0
#define SPC_SK_RECOVERED_ERROR 0x1
#define SPC_SK_NOT_READY 0x2
#define SPC_SK_MEDIUM_ERROR 0x3
#define SPC_SK_HARDWARE_ERROR 0x4
#define SPC_SK_ILLEGAL_REQUEST 0x5
#define SPC_SK_UNIT_ATTENTION 0x6
#define SPC_SK_DATA_PROTECT 0x7
#define SPC_SK_BLANK_CHECK 0x8
#define SPC_SK_VENDOR_SPECIFIC 0x9
#define SPC_SK_COPY_ABORTED 0xa
#define SPC_SK_ABORTED_COMMAND 0xb
#define SPC_SK_RESERVED 0xc
#define SPC_SK_VOLUME_OVERFLOW 0xd
#define SPC_SK_MISCOMPARE 0xe
#define SPC_SK_COMPLETED 0xf

/* Transport protocol identifiers or just Protocol identifiers */
#define TPROTO_FCP 0
#define TPROTO_SPI 1
#define TPROTO_SSA 2
#define TPROTO_1394 3
#define TPROTO_SRP 4            /* SCSI over RDMA */
#define TPROTO_ISCSI 5
#define TPROTO_SAS 6
#define TPROTO_ADT 7
#define TPROTO_ATA 8
#define TPROTO_UAS 9            /* USB attached SCSI */
#define TPROTO_SOP 0xa          /* SCSI over PCIe */
#define TPROTO_PCIE 0xb         /* includes NVMe */
#define TPROTO_NONE 0xf

/* SCSI Feature Sets (sfs) */
#define SCSI_FS_SPC_DISCOVERY_2016 0x1
#define SCSI_FS_SBC_BASE_2010 0x102
#define SCSI_FS_SBC_BASE_2016 0x101
#define SCSI_FS_SBC_BASIC_PROV_2016 0x103
#define SCSI_FS_SBC_DRIVE_MAINT_2016 0x104
#define SCSI_FS_ZBC_HOST_AWARE_2020 0x300
#define SCSI_FS_ZBC_HOST_MANAGED_2020 0x301
#define SCSI_FS_ZBC_DOMAINS_REALMS_2020 0x302

/* Often SCSI responses use the highest integer that can fit in a field
 * to indicate "unbounded" or limit does not apply. Sometimes represented
 * in output as "-1" for brevity */
#define SG_LIB_UNBOUNDED_16BIT 0xffff
#define SG_LIB_UNBOUNDED_32BIT 0xffffffffU
#define SG_LIB_UNBOUNDED_64BIT 0xffffffffffffffffULL

#if (__STDC_VERSION__ >= 199901L)  /* C99 or later */
    typedef uintptr_t sg_uintptr_t;
#else
    typedef unsigned long sg_uintptr_t;
#endif

/* Borrowed from Linux kernel; no check that 'arr' actually is one */
#define SG_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


/* The format of the version string is like this: "2.26 20170906" */
const char * sg_lib_version();

/* Returns length of SCSI command given the opcode (first byte).
 * Yields the wrong answer for variable length commands (opcode=0x7f)
 * and potentially some vendor specific commands. */
int sg_get_command_size(uint8_t cdb_byte0);

/* Command name given pointer to the cdb. Certain command names
 * depend on peripheral type (give 0 or -1 if unknown). Places command
 * name into buff and will write no more than buff_len bytes. */
void sg_get_command_name(const uint8_t * cdbp, int peri_type, int buff_len,
                         char * buff);

/* Command name given only the first byte (byte 0) of a cdb and
 * peripheral type (give 0 or -1 if unknown). */
void sg_get_opcode_name(uint8_t cdb_byte0, int peri_type, int buff_len,
                        char * buff);

/* Command name given opcode (byte 0), service action and peripheral type.
 * If no service action give 0, if unknown peripheral type give 0 or -1 . */
void sg_get_opcode_sa_name(uint8_t cdb_byte0, int service_action,
                           int peri_type, int buff_len, char * buff);

/* Fetch NVMe command name given first byte (byte offset 0 in 64 byte
 * command) of command. Gets Admin NVMe command name if 'admin' is true
 * (e.g. opcode=0x6 -> Identify), otherwise gets NVM command set name
 * (e.g. opcode=0 -> Flush). Returns 'buff'. */
char * sg_get_nvme_opcode_name(uint8_t cmd_byte0, bool admin, int buff_len,
                               char * buff);

/* Fetch scsi status string. */
void sg_get_scsi_status_str(int scsi_status, int buff_len, char * buff);

/* This is a slightly stretched SCSI sense "descriptor" format header.
 * The addition is to allow the 0x70 and 0x71 response codes. The idea
 * is to place the salient data of both "fixed" and "descriptor" sense
 * format into one structure to ease application processing.
 * The original sense buffer should be kept around for those cases
 * in which more information is required (e.g. the LBA of a MEDIUM ERROR). */
struct sg_scsi_sense_hdr {
    uint8_t response_code; /* permit: 0x0, 0x70, 0x71, 0x72, 0x73 */
    uint8_t sense_key;
    uint8_t asc;
    uint8_t ascq;
    uint8_t byte4;      /* descriptor: SDAT_OVFL; fixed: lower three ... */
    uint8_t byte5;      /* ... bytes of INFO field */
    uint8_t byte6;
    uint8_t additional_length;  /* zero for fixed format sense data */
};

/* Maps the salient data from a sense buffer which is in either fixed or
 * descriptor format into a structure mimicking a descriptor format
 * header (i.e. the first 8 bytes of sense descriptor format).
 * If zero response code returns false. Otherwise returns true and if 'sshp'
 * is non-NULL then zero all fields and then set the appropriate fields in
 * that structure. sshp::additional_length is always 0 for response
 * codes 0x70 and 0x71 (fixed format). */
bool sg_scsi_normalize_sense(const uint8_t * sensep, int sense_len,
                             struct sg_scsi_sense_hdr * sshp);

/* Attempt to find the first SCSI sense data descriptor that matches the
 * given 'desc_type'. If found return pointer to start of sense data
 * descriptor; otherwise (including fixed format sense data) returns NULL. */
const uint8_t * sg_scsi_sense_desc_find(const uint8_t * sensep, int sense_len,
                                        int desc_type);

/* Get sense key from sense buffer. If successful returns a sense key value
 * between 0 and 15. If sense buffer cannot be decode, returns -1 . */
int sg_get_sense_key(const uint8_t * sensep, int sense_len);

/* Yield string associated with sense_key value. Returns 'buff'. */
char * sg_get_sense_key_str(int sense_key, int buff_len, char * buff);

/* Yield string associated with ASC/ASCQ values. Returns 'buff'. */
char * sg_get_asc_ascq_str(int asc, int ascq, int buff_len, char * buff);

/* Returns true if valid bit set, false if valid bit clear. Irrespective the
 * information field is written out via 'info_outp' (except when it is
 * NULL). Handles both fixed and descriptor sense formats. */
bool sg_get_sense_info_fld(const uint8_t * sensep, int sb_len,
                           uint64_t * info_outp);

/* Returns true if fixed format or command specific information descriptor
 * is found in the descriptor sense; else false. If available the command
 * specific information field (4 byte integer in fixed format, 8 byte
 * integer in descriptor format) is written out via 'cmd_spec_outp'.
 * Handles both fixed and descriptor sense formats. */
bool sg_get_sense_cmd_spec_fld(const uint8_t * sensep, int sb_len,
                               uint64_t * cmd_spec_outp);

/* Returns true if any of the 3 bits (i.e. FILEMARK, EOM or ILI) are set.
 * In descriptor format if the stream commands descriptor not found
 * then returns false. Writes true or false corresponding to these bits to
 * the last three arguments if they are non-NULL. */
bool sg_get_sense_filemark_eom_ili(const uint8_t * sensep, int sb_len,
                                   bool * filemark_p, bool * eom_p,
                                   bool * ili_p);

/* Returns true if SKSV is set and sense key is NO_SENSE or NOT_READY. Also
 * returns true if progress indication sense data descriptor found. Places
 * progress field from sense data where progress_outp points. If progress
 * field is not available returns false. Handles both fixed and descriptor
 * sense formats. N.B. App should multiply by 100 and divide by 65536
 * to get percentage completion from given value. */
bool sg_get_sense_progress_fld(const uint8_t * sensep, int sb_len,
                               int * progress_outp);

/* Closely related to sg_print_sense(). Puts decoded sense data in 'buff'.
 * Usually multiline with multiple '\n' including one trailing. If
 * 'raw_sinfo' set appends sense buffer in hex. 'leadin' is string prepended
 * to each line written to 'buff', NULL treated as "". Returns the number of
 * bytes written to 'buff' excluding the trailing '\0'.
 * N.B. prior to sg3_utils v 1.42 'leadin' was only prepended to the first
 * line output. Also this function returned type void. */
int sg_get_sense_str(const char * leadin, const uint8_t * sense_buffer,
                     int sb_len, bool raw_sinfo, int buff_len, char * buff);

/* Decode descriptor format sense descriptors (assumes sense buffer is
 * in descriptor format). 'leadin' is string prepended to each line written
 * to 'b', NULL treated as "". Returns the number of bytes written to 'b'
 * excluding the trailing '\0'. If problem, returns 0. */
int sg_get_sense_descriptors_str(const char * leadin,
                                 const uint8_t * sense_buffer,
                                 int sb_len, int blen, char * b);

/* Decodes a designation descriptor (e.g. as found in the Device
 * Identification VPD page (0x83)) into string 'b' whose maximum length is
 * blen. 'leadin' is string prepended to each line written to 'b', NULL
 * treated as "". Returns the number of bytes written to 'b' excluding the
 * trailing '\0'. */
int sg_get_designation_descriptor_str(const char * leadin,
                                      const uint8_t * ddp, int dd_len,
                                      bool print_assoc, bool do_long,
                                      int blen, char * b);

/* Expects a T10 UUID designator (as found in the Device Identification VPD
 * page) pointed to by 'dp'. To not produce an error string in 'b', c_set
 * should be 1 (binary) and dlen should be 18. Currently T10 only supports
 * locally assigned UUIDs. Writes output to string 'b' of no more than blen
 * bytes and returns the number of bytes actually written to 'b' but doesn't
 * count the trailing null character it always appends (if blen > 0). 'lip'
 * is lead-in string (on each line) than may be NULL. skip_prefix avoids
 * outputting: '   Locally assigned UUID: ' before the UUID. */
int sg_t10_uuid_desig2str(const uint8_t * dp, int dlen, int c_set,
                          bool do_long, bool skip_prefix,
                          const char * lip, int blen, char * b);

/* Yield string associated with peripheral device type (pdt). Returns
 * 'buff'. If 'pdt' out of range yields "bad pdt" string. */
char * sg_get_pdt_str(int pdt, int buff_len, char * buff);

/* Some lesser used PDTs share a lot in common with a more used PDT.
 * Examples are PDT_ADC decaying to PDT_TAPE and PDT_ZBC to PDT_DISK.
 * If such a lesser used 'pdt' is given to this function, then it will
 * return the more used PDT (i.e. "decays to"); otherwise 'pdt' is returned.
 * Valid for 'pdt' 0 to 31, for other values returns 0. */
int sg_lib_pdt_decay(int pdt);

/* Yield string associated with transport protocol identifier (tpi). Returns
 * 'buff'. If 'tpi' out of range yields "bad tpi" string. */
char * sg_get_trans_proto_str(int tpi, int buff_len, char * buff);

/* Decode TransportID pointed to by 'bp' of length 'bplen'. Place decoded
 * string output in 'buff' which is also the return value. Each new line
 * is prefixed by 'leadin'. If leadin NULL treat as "". */
char * sg_decode_transportid_str(const char * leadin, uint8_t * bp, int bplen,
                                 bool only_one, int buff_len, char * buff);

/* Returns a designator's type string given 'val' (0 to 15 inclusive),
 * otherwise returns NULL. */
const char * sg_get_desig_type_str(int val);

/* Returns a designator's code_set string given 'val' (0 to 15 inclusive),
 * otherwise returns NULL. */
const char * sg_get_desig_code_set_str(int val);

/* Returns a designator's association string given 'val' (0 to 3 inclusive),
 * otherwise returns NULL. */
const char * sg_get_desig_assoc_str(int val);

/* Yield SCSI Feature Set (sfs) string. When 'peri_type' is < -1 (or > 31)
 * returns pointer to string (same as 'buff') associated with 'sfs_code'.
 * When 'peri_type' is between -1 (for SPC) and 31 (inclusive) then a match
 * on both 'sfs_code' and 'peri_type' is required. If 'foundp' is not NULL
 * then where it points is set to true if a match is found else it is set to
 * false. If 'buff' is not NULL then in the case of a match a descriptive
 * string is written to 'buff' while if there is not a not then a string
 * ending in "Reserved" is written (and may be prefixed with SPC, SBC, SSC
 * or ZBC). Returns 'buff' (i.e. a pointer value) even if it is NULL.
 * Example:
 *    char b[64];
 *    ...
 *    printf("%s\n", sg_get_sfs_str(sfs_code, -2, sizeof(b), b, NULL, 0));
 */
const char * sg_get_sfs_str(uint16_t sfs_code, int peri_type, int buff_len,
                            char * buff, bool * foundp, int verbose);

/* This is a heuristic that takes into account the command bytes and length
 * to decide whether the presented unstructured sequence of bytes could be
 * a SCSI command. If so it returns true otherwise false. Vendor specific
 * SCSI commands (i.e. opcodes from 0xc0 to 0xff), if presented, are assumed
 * to follow SCSI conventions (i.e. length of 6, 10, 12 or 16 bytes). The
 * only SCSI commands considered above 16 bytes of length are the Variable
 * Length Commands (opcode 0x7f) and the XCDB wrapped commands (opcode 0x7e).
 * Both have an inbuilt length field which can be cross checked with clen.
 * No NVMe commands (64 bytes long plus some extra added by some OSes) have
 * opcodes 0x7e or 0x7f yet. ATA is register based but SATA has FIS
 * structures that are sent across the wire. The 'FIS register' structure is
 * used to move a command from a SATA host to device, but the ATA 'command'
 * is not the first byte. So it is harder to say what will happen if a
 * FIS structure is presented as a SCSI command, hopefully there is a low
 * probability this function will yield true in that case. */
bool sg_is_scsi_cdb(const uint8_t * cdbp, int clen);

/* Yield string associated with NVMe command status value in sct_sc. It
 * expects to decode DW3 bits 27:17 from the completion queue. Bits 27:25
 * are the Status Code Type (SCT) and bits 24:17 are the Status Code (SC).
 * Bit 17 in DW3 should be bit 0 in sct_sc. If no status string is found
 * a string of the form "Reserved [0x<sct_sc_in_hex>]" is generated.
 * Returns 'buff'. Does nothing if buff_len<=0 or if buff is NULL.*/
char * sg_get_nvme_cmd_status_str(uint16_t sct_sc, int buff_len, char * buff);

/* Attempts to map NVMe status value ((SCT << 8) | SC) n sct_sc to a SCSI
 * status, sense_key, asc and ascq tuple. If successful returns true and
 * writes to non-NULL pointer arguments; otherwise returns false. */
bool sg_nvme_status2scsi(uint16_t sct_sc, uint8_t * status_p, uint8_t * sk_p,
                         uint8_t * asc_p, uint8_t * ascq_p);

/* Add vendor (sg3_utils) specific sense descriptor for the NVMe Status
 * field. Assumes descriptor (i.e. not fixed) sense. Assume sbp has room. */
void sg_nvme_desc2sense(uint8_t * sbp, bool dnr, bool more, uint16_t sct_sc);

/* Build minimum sense buffer, either descriptor type (desc=true) or fixed
 * type (desc=false). Assume sbp has enough room (8 or 14 bytes
 * respectively). sbp should have room for 32 or 18 bytes respectively */
void sg_build_sense_buffer(bool desc, uint8_t *sbp, uint8_t skey,
                           uint8_t asc, uint8_t ascq);

extern FILE * sg_warnings_strm;

void sg_set_warnings_strm(FILE * warnings_strm);

/* Given a SCSI command pointed to by cdbp of sz bytes this function forms a
 * SCSI command in ASCII hex surrounded by square brackets in 'b'. 'b' is at
 * least blen bytes long. If cmd_name is true then the command is prefixed
 * by its SCSI command name (e.g.  "VERIFY(10) [2f ...]". The command is
 * shown as spaced separated pairs of hexadecimal digits (i.e. 0-9, a-f).
 * Each pair represents byte. The leftmost pair of digits is cdbp[0] . If
 * sz <= 0 then this function tries to guess the length of the command. */
char *
sg_get_command_str(const uint8_t * cdbp, int sz, bool cmd_name, int blen,
                   char * b);

/* The following "print" functions send ASCII to 'sg_warnings_strm' file
 * descriptor (default value is stderr). 'leadin' is string prepended to
 * each line printed out, NULL treated as "". */
void sg_print_command_len(const uint8_t * command, int len);
void sg_print_command(const uint8_t * command);
void sg_print_scsi_status(int scsi_status);

/* DSENSE is 'descriptor sense' as opposed to the older 'fixed sense'. Reads
 * environment variable SG3_UTILS_DSENSE. Only (currently) used in SNTL. */
bool sg_get_initial_dsense(void);

/* 'leadin' is string prepended to each line printed out, NULL treated as
 * "". N.B. prior to sg3_utils v 1.42 'leadin' was only prepended to the
 * first line printed. */
void sg_print_sense(const char * leadin, const uint8_t * sense_buffer,
                    int sb_len, bool raw_info);

/* This examines exit_status and if an error message is known it is output
 * to stdout/stderr and true is returned. If no error message is
 * available nothing is output and false is returned. If exit_status is
 * zero (no error) nothing is output and true is returned. If exit_status
 * is negative then nothing is output and false is returned. If leadin is
 * non-NULL then it is printed before the error message. All messages are
 * a single line with a trailing LF. */
bool sg_if_can2stdout(const char * leadin, int exit_status);
bool sg_if_can2stderr(const char * leadin, int exit_status);

/* This examines exit_status and if an error message is known it is output
 * as a string to 'b' and true is returned. If 'longer' is true and extra
 * information is available then it is added to the output. If no error
 * message is available a null character is output and false is returned.
 * If exit_status is zero (no error) and 'longer' is true then the string
 * 'No errors' is output; if 'longer' is false then a null character is
 * output; in both cases true is returned. If exit_status is negative then
 * a null character is output and false is returned. All messages are a
 * single line (less than 80 characters) with no trailing LF. The output
 * string including the trailing null character is no longer than b_len. */
bool sg_exit2str(int exit_status, bool longer, int b_len, char * b);

/* Utilities can use these exit status values for syntax errors and
 * file (device node) problems (e.g. not found or permissions). */
#define SG_LIB_SYNTAX_ERROR 1   /* command line syntax problem */

/* The sg_err_category_sense() function returns one of the following.
 * These may be used as exit status values (from a process). Notice that
 * some of the lower values correspond to SCSI sense key values. */
#define SG_LIB_CAT_CLEAN 0      /* No errors or other information */
#define SG_LIB_OK_TRUE SG_LIB_CAT_CLEAN  /* No error, reporting true */
/* Value 1 left unused for utilities to use SG_LIB_SYNTAX_ERROR */
#define SG_LIB_CAT_NOT_READY 2  /* sense key, unit stopped?
                                 *       [sk,asc,ascq: 0x2,*,*] */
#define SG_LIB_CAT_MEDIUM_HARD 3 /* medium or hardware error, blank check
                                  *       [sk,asc,ascq: 0x3/0x4/0x8,*,*] */
#define SG_LIB_CAT_ILLEGAL_REQ 5 /* Illegal request (other than invalid
                                  * opcode):   [sk,asc,ascq: 0x5,*,*] */
#define SG_LIB_CAT_UNIT_ATTENTION 6 /* sense key, device state changed
                                     *       [sk,asc,ascq: 0x6,*,*] */
        /* was SG_LIB_CAT_MEDIA_CHANGED earlier [sk,asc,ascq: 0x6,0x28,*] */
#define SG_LIB_CAT_DATA_PROTECT 7 /* sense key, media write protected?
                                   *       [sk,asc,ascq: 0x7,*,*] */
#define SG_LIB_CAT_INVALID_OP 9 /* (Illegal request,) Invalid opcode:
                                 *       [sk,asc,ascq: 0x5,0x20,0x0] */
#define SG_LIB_CAT_COPY_ABORTED 10 /* sense key, some data transferred
                                    *       [sk,asc,ascq: 0xa,*,*] */
#define SG_LIB_CAT_ABORTED_COMMAND 11 /* interpreted from sense buffer
                                       *       [sk,asc,ascq: 0xb,! 0x10,*] */
#define SG_LIB_CAT_MISCOMPARE 14 /* sense key, probably verify
                                  *       [sk,asc,ascq: 0xe,*,*] */
#define SG_LIB_FILE_ERROR 15    /* device or other file problem */
#define SG_LIB_CAT_NO_SENSE 20  /* sense data with key of "no sense"
                                 *       [sk,asc,ascq: 0x0,*,*] */
#define SG_LIB_CAT_RECOVERED 21 /* Successful command after recovered err
                                 *       [sk,asc,ascq: 0x1,*,*] */
#define SG_LIB_LBA_OUT_OF_RANGE 22 /* Illegal request, LBA Out Of Range
                                    *    [sk,asc,ascq: 0x5,0x21,0x0] */
#define SG_LIB_CAT_RES_CONFLICT SAM_STAT_RESERVATION_CONFLICT
                                /* 24: this is a SCSI status, not sense.
                                 * It indicates reservation by another
                                 * machine blocks this command */
#define SG_LIB_CAT_CONDITION_MET 25 /* SCSI status, not sense key.
                                     * Only from PRE-FETCH (SBC-4) */
#define SG_LIB_CAT_BUSY       26 /* SCSI status, not sense. Invites retry */
#define SG_LIB_CAT_TS_FULL    27 /* SCSI status, not sense. Wait then retry */
#define SG_LIB_CAT_ACA_ACTIVE 28 /* SCSI status; ACA seldom used */
#define SG_LIB_CAT_TASK_ABORTED 29 /* SCSI status, this command aborted by? */
#define SG_LIB_CONTRADICT 31    /* error involving two or more cl options */
#define SG_LIB_LOGIC_ERROR 32   /* unexpected situation in code */
#define SG_LIB_WINDOWS_ERR 34   /* Windows error number don't fit in 7 bits so
                                 * map to a single value for exit statuses */
#define SG_LIB_OK_FALSE 36      /* no error, reporting false (cf. no error,
                                 * reporting true is SG_LIB_OK_TRUE(0) ) */
#define SG_LIB_CAT_PROTECTION 40 /* subset of aborted command (for PI, DIF)
                                  *       [sk,asc,ascq: 0xb,0x10,*] */
/* 47: flock error used in ddpt utility */
#define SG_LIB_NVME_STATUS 48   /* NVMe Status Field (SF) other than 0 */
#define SG_LIB_WILD_RESID 49    /* Residual value for data-in transfer of a
                                 * SCSI command is nonsensical */
#define SG_LIB_OS_BASE_ERR 50   /* in Linux: values found in:
                                 * include/uapi/asm-generic/errno-base.h
                                 * Example: ENOMEM reported as 62 (=50+12)
                                 * if errno > 46 then use this value */
/* 51-->96 set aside for Unix errno values shifted by SG_LIB_OS_BASE_ERR */
#define SG_LIB_CAT_MALFORMED 97 /* Response to SCSI command malformed */
#define SG_LIB_CAT_SENSE 98     /* Something else is in the sense buffer */
#define SG_LIB_CAT_OTHER 99     /* Some other error/warning has occurred
                                 * (e.g. a transport or driver error) */
/* 100 to 120 (inclusive) used by ddpt utility */
#define SG_LIB_UNUSED_ABOVE 120  /* Put extra errors in holes below this */

/* Returns a SG_LIB_CAT_* value. If cannot decode sense_buffer or a less
 * common sense key then return SG_LIB_CAT_SENSE .*/
int sg_err_category_sense(const uint8_t * sense_buffer, int sb_len);

/* Here are some additional sense data categories that are not returned
 * by sg_err_category_sense() but are returned by some related functions. */
#define SG_LIB_CAT_ILLEGAL_REQ_WITH_INFO 17 /* Illegal request (other than */
                                /* invalid opcode) plus 'info' field: */
                                /*  [sk,asc,ascq: 0x5,*,*] */
#define SG_LIB_CAT_MEDIUM_HARD_WITH_INFO 18 /* medium or hardware error */
                                /* sense key plus 'info' field: */
                                /*       [sk,asc,ascq: 0x3/0x4,*,*] */
#define SG_LIB_CAT_TIMEOUT 33   /* SCSI command timeout */
#define SG_LIB_CAT_PROTECTION_WITH_INFO 41 /* aborted command sense key, */
                                /* protection plus 'info' field: */
                                /*  [sk,asc,ascq: 0xb,0x10,*] */

/* Yield string associated with sense category. Returns 'buff' (or pointer
 * to "Bad sense category" if 'buff' is NULL). If sense_cat unknown then
 * yield "Sense category: <sense_cat)val>" string. The original 'sense
 * category' concept has been expanded to most detected errors and is
 * returned by these utilities as their exit status value (an (unsigned)
 * 8 bit value where 0 means good (i.e. no errors)).  Uses the
 * sg_exit2str() function. */
const char * sg_get_category_sense_str(int sense_cat, int buff_len,
                                       char * buff, int verbose);


/* Iterates to next designation descriptor in the device identification
 * VPD page. The 'initial_desig_desc' should point to start of first
 * descriptor with 'page_len' being the number of valid bytes in that
 * and following descriptors. To start, 'off' should point to a negative
 * value, thereafter it should point to the value yielded by the previous
 * call. If 0 returned then 'initial_desig_desc + *off' should be a valid
 * descriptor; returns -1 if normal end condition and -2 for an abnormal
 * termination. Matches association, designator_type and/or code_set when
 * any of those values are greater than or equal to zero. */
int sg_vpd_dev_id_iter(const uint8_t * initial_desig_desc, int page_len,
                       int * off, int m_assoc, int m_desig_type,
                       int m_code_set);


/* <<< General purpose (i.e. not SCSI specific) utility functions >>> */

/* Always returns valid string even if errnum is wild (or library problem).
 * If errnum is negative, flip its sign. */
char * safe_strerror(int errnum);


/* Print (to stdout) 'str' of bytes in hex, 16 bytes per line optionally
 * followed at the right hand side of the line with an ASCII interpretation.
 * Each line is prefixed with an address, starting at 0 for str[0]..str[15].
 * All output numbers are in hex. 'no_ascii' allows for 3 output types:
 *     > 0     each line has address then up to 16 ASCII-hex bytes
 *     = 0     in addition, the bytes are listed in ASCII to the right
 *     < 0     only the ASCII-hex bytes are listed (i.e. without address)
*/
void dStrHex(const char * str, int len, int no_ascii);

/* Print (to sg_warnings_strm (stderr)) 'str' of bytes in hex, 16 bytes per
 * line optionally followed at right by its ASCII interpretation. Same
 * logic as dStrHex() with different output stream (i.e. stderr). */
void dStrHexErr(const char * str, int len, int no_ascii);

/* Read 'len' bytes from 'str' and output as ASCII-Hex bytes (space
 * separated) to 'b' not to exceed 'b_len' characters. Each line
 * starts with 'leadin' (NULL for no leadin) and there are 16 bytes
 * per line with an extra space between the 8th and 9th bytes. 'format'
 * is 0 for repeat in printable ASCII ('.' for non printable chars) to
 * right of each line; 1 don't (so just output ASCII hex). Returns
 * number of bytes written to 'b' excluding the trailing '\0'. */
int dStrHexStr(const char * str, int len, const char * leadin, int format,
               int cb_len, char * cbp);

/* The following 3 functions are equivalent to dStrHex(), dStrHexErr() and
 * dStrHexStr() respectively. The difference is the type of the first of
 * argument: uint8_t instead of char. The name of the argument is changed
 * to b_str to stress it is a pointer to the start of a binary string. */
void hex2stdout(const uint8_t * b_str, int len, int no_ascii);
void hex2stderr(const uint8_t * b_str, int len, int no_ascii);
int hex2str(const uint8_t * b_str, int len, const char * leadin, int format,
            int cb_len, char * cbp);

/* Read ASCII hex bytes or binary from fname (a file named '-' taken as
 * stdin). If reading ASCII hex then there should be either one entry per
 * line or a comma, space or tab separated list of bytes. If no_space is
 * set then a string of ACSII hex digits is expected, 2 per byte. Everything
 * from and including a '#' on a line is ignored. Returns 0 if ok, or an
 * error code. If the error code is SG_LIB_LBA_OUT_OF_RANGE then mp_arr
 * would be exceeded and both mp_arr and mp_arr_len are written to. */
int sg_f2hex_arr(const char * fname, bool as_binary, bool no_space,
                 uint8_t * mp_arr, int * mp_arr_len, int max_arr_len);

/* Returns true when executed on big endian machine; else returns false.
 * Useful for displaying ATA identify words (which need swapping on a
 * big endian machine). */
bool sg_is_big_endian();

/* Returns true if byte sequence starting at bp with a length of b_len is
 * all zeros (for sg_all_zeros()) or all 0xff_s (for sg_all_ffs());
 * otherwise returns false. If bp is NULL or b_len <= 0 returns false. */
bool sg_all_zeros(const uint8_t * bp, int b_len);
bool sg_all_ffs(const uint8_t * bp, int b_len);

/* Extract character sequence from ATA words as in the model string
 * in a IDENTIFY DEVICE response. Returns number of characters
 * written to 'ochars' before 0 character is found or 'num' words
 * are processed. */
int sg_ata_get_chars(const uint16_t * word_arr, int start_word,
                     int num_words, bool is_big_endian, char * ochars);

/* Print (to stdout) 16 bit 'words' in hex, 8 words per line optionally
 * followed at the right hand side of the line with an ASCII interpretation
 * (pairs of ASCII characters in big endian order (upper first)).
 * Each line is prefixed with an address, starting at 0.
 * All output numbers are in hex. 'no_ascii' allows for 3 output types:
 *     > 0     each line has address then up to 8 ASCII-hex words
 *     = 0     in addition, the words are listed in ASCII pairs to the right
 *     = -1    only the ASCII-hex words are listed (i.e. without address)
 *     = -2    only the ASCII-hex words, formatted for "hdparm --Istdin"
 *     < -2    same as -1
 * If 'swapb' is true then bytes in each word swapped. Needs to be set
 * for ATA IDENTIFY DEVICE response on big-endian machines.
*/
void dWordHex(const uint16_t * words, int num, int no_ascii, bool swapb);

/* If the number in 'buf' can not be decoded or the multiplier is unknown
 * then -1 is returned. Accepts a hex prefix (0x or 0X) or a decimal
 * multiplier suffix (as per GNU's dd (since 2002: SI and IEC 60027-2)).
 * Main (SI) multipliers supported: K, M, G. Ignore leading spaces and
 * tabs; accept comma, hyphen, space, tab and hash as terminator.
 * Handles zero and positive values up to 2**31-1 .
 * Experimental: left argument (must in with hexadecimal digit) added
 * to, or multiplied, by right argument. No embedded spaces.
 * Examples: '3+1k' (evaluates to 1027) and '0xf+0x3'. */
int sg_get_num(const char * buf);

/* If the number in 'buf' can not be decoded then -1 is returned. Accepts a
 * hex prefix (0x or 0X) or a 'h' (or 'H') suffix; otherwise decimal is
 * assumed. Does not accept multipliers. Accept a comma (","), hyphen ("-"),
 * a whitespace or newline as terminator. Only decimal numbers can represent
 * negative numbers and '-1' must be treated separately. */
int sg_get_num_nomult(const char * buf);

/* If the number in 'buf' can not be decoded or the multiplier is unknown
 * then -1LL is returned. Accepts a hex prefix (0x or 0X), hex suffix
 * (h or H), or a decimal multiplier suffix (as per GNU's dd (since 2002:
 * SI and IEC 60027-2)).  Main (SI) multipliers supported: K, M, G, T, P
 * and E. Ignore leading spaces and tabs; accept comma, hyphen, space, tab
 * and hash as terminator. Handles zero and positive values up to 2**63-1 .
 * Experimental: the left argument (must end in with hexadecimal digit)
 * added to, or multiplied by, the right argument. No embedded spaces.
 * Examples: '3+1k' (evaluates to 1027) and '0xf+0x3'. */
int64_t sg_get_llnum(const char * buf);

/* If the number in 'buf' can not be decoded then -1 is returned. Accepts a
 * hex prefix (0x or 0X) or a 'h' (or 'H') suffix; otherwise decimal is
 * assumed. Does not accept multipliers. Accept a comma (","), hyphen ("-"),
 * a whitespace or newline as terminator. Only decimal numbers can represent
 * negative numbers and '-1' must be treated separately. */
int64_t sg_get_llnum_nomult(const char * buf);

/* Returns pointer to heap (or NULL) that is aligned to a align_to byte
 * boundary. Sends back *buff_to_free pointer in third argument that may be
 * different from the return value. If it is different then the *buff_to_free
 * pointer should be freed (rather than the returned value) when the heap is
 * no longer needed. If align_to is 0 then aligns to OS's page size. Sets all
 * returned heap to zeros. If num_bytes is 0 then set to page size. */
uint8_t * sg_memalign(uint32_t num_bytes, uint32_t align_to,
                      uint8_t ** buff_to_free, bool vb);

/* Returns OS page size in bytes. If uncertain returns 4096. */
uint32_t sg_get_page_size(void);

/* If byte_count is 0 or less then the OS page size is used as denominator.
 * Returns true  if the remainder of ((unsigned)pointer % byte_count) is 0,
 * else returns false. */
bool sg_is_aligned(const void * pointer, int byte_count);

/* Does similar job to sg_get_unaligned_be*() but this function starts at
 * a given start_bit (i.e. within byte, so 7 is MSbit of byte and 0 is LSbit)
 * offset. Maximum number of num_bits is 64. For example, these two
 * invocations are equivalent (and should yield the same result);
 *       sg_get_big_endian(from_bp, 7, 16)
 *       sg_get_unaligned_be16(from_bp)  */
uint64_t sg_get_big_endian(const uint8_t * from_bp,
                           int start_bit /* 0 to 7 */,
                           int num_bits /* 1 to 64 */);

/* Does similar job to sg_put_unaligned_be*() but this function starts at
 * a given start_bit offset. Maximum number of num_bits is 64. Preserves
 * residual bits in partially written bytes. start_bit 7 is MSb. */
void sg_set_big_endian(uint64_t val, uint8_t * to, int start_bit /* 0 to 7 */,
                       int num_bits /* 1 to 64 */);

/* If os_err_num is within bounds then the returned value is 'os_err_num +
 * SG_LIB_OS_BASE_ERR' otherwise SG_LIB_OS_BASE_ERR is returned. If
 * os_err_num is 0 then 0 is returned. */
int sg_convert_errno(int os_err_num);


/* <<< Architectural support functions [is there a better place?] >>> */

/* Non Unix OSes distinguish between text and binary files.
 * Set text mode on fd. Does nothing in Unix. Returns negative number on
 * failure. */
int sg_set_text_mode(int fd);

/* Set binary mode on fd. Does nothing in Unix. Returns negative number on
 * failure. */
int sg_set_binary_mode(int fd);

#ifdef __cplusplus
}
#endif

#endif          /* SG_LIB_H */
