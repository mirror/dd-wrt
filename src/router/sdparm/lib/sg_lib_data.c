/*
 * Copyright (c) 2007-2021 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define SG_SCSI_STRINGS 1
#endif

#include "sg_lib.h"
#include "sg_lib_data.h"


const char * sg_lib_version_str = "2.79 20210304";
/* spc6r05, sbc4r22, zbc2r09 */


/* indexed by pdt; those that map to own index do not decay */
int sg_lib_pdt_decay_arr[32] = {
    PDT_DISK, PDT_TAPE, PDT_TAPE /* printer */, PDT_PROCESSOR,
    PDT_DISK /* WO */, PDT_MMC, PDT_SCANNER, PDT_DISK /* optical */,
    PDT_MCHANGER, PDT_COMMS, 0xa, 0xb,
    PDT_SAC, PDT_SES, PDT_DISK /* rbc */, PDT_OCRW,
    PDT_BCC, PDT_OSD, PDT_TAPE /* adc */, PDT_SMD,
    PDT_DISK /* zbc */, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, PDT_WLUN, PDT_UNKNOWN
};

#ifdef SG_SCSI_STRINGS
struct sg_lib_value_name_t sg_lib_normal_opcodes[] = {
    {0, 0, "Test Unit Ready"},
    {0x1, 0, "Rezero Unit"},
    {0x1, PDT_TAPE, "Rewind"},
    {0x3, 0, "Request Sense"},
    {0x4, 0, "Format Unit"},
    {0x4, PDT_TAPE, "Format medium"},
    {0x4, PDT_PRINTER, "Format"},
    {0x5, 0, "Read Block Limits"},
    {0x7, 0, "Reassign Blocks"},
    {0x7, PDT_MCHANGER, "Initialize element status"},
    {0x8, 0, "Read(6)"},        /* obsolete in sbc3r30 */
    {0x8, PDT_PROCESSOR, "Receive"},
    {0xa, 0, "Write(6)"},       /* obsolete in sbc3r30 */
    {0xa, PDT_PRINTER, "Print"},
    {0xa, PDT_PROCESSOR, "Send"},
    {0xb, 0, "Seek(6)"},
    {0xb, PDT_TAPE, "Set capacity"},
    {0xb, PDT_PRINTER, "Slew and print"},
    {0xf, 0, "Read reverse(6)"},
    {0x10, 0, "Write filemarks(6)"},
    {0x10, PDT_PRINTER, "Synchronize buffer"},
    {0x11, 0, "Space(6)"},
    {0x12, 0, "Inquiry"},
    {0x13, 0, "Verify(6)"},  /* SSC */
    {0x14, 0, "Recover buffered data"},
    {0x15, 0, "Mode select(6)"}, /* SBC-3 r31 recommends Mode select(10) */
    {0x16, 0, "Reserve(6)"},    /* obsolete in SPC-4 r11 */
    {0x16, PDT_MCHANGER, "Reserve element(6)"},
    {0x17, 0, "Release(6)"},    /* obsolete in SPC-4 r11 */
    {0x17, PDT_MCHANGER, "Release element(6)"},
    {0x18, 0, "Copy"},          /* obsolete in SPC-4 r11 */
    {0x19, 0, "Erase(6)"},
    {0x1a, 0, "Mode sense(6)"}, /* SBC-3 r31 recommends Mode sense(10) */
    {0x1b, 0, "Start stop unit"},
    {0x1b, PDT_TAPE, "Load unload"},
    {0x1b, PDT_ADC, "Load unload"},
    {0x1b, PDT_PRINTER, "Stop print"},
    {0x1c, 0, "Receive diagnostic results"},
    {0x1d, 0, "Send diagnostic"},
    {0x1e, 0, "Prevent allow medium removal"},
    {0x23, 0, "Read Format capacities"},
    {0x24, 0, "Set window"},
    {0x25, 0, "Read capacity(10)"},
                        /* SBC-3 r31 recommends Read capacity(16) */
    {0x25, PDT_OCRW, "Read card capacity"},
    {0x28, 0, "Read(10)"},      /* SBC-3 r31 recommends Read(16) */
    {0x29, 0, "Read generation"},
    {0x2a, 0, "Write(10)"},     /* SBC-3 r31 recommends Write(16) */
    {0x2b, 0, "Seek(10)"},
    {0x2b, PDT_TAPE, "Locate(10)"},
    {0x2b, PDT_MCHANGER, "Position to element"},
    {0x2c, 0, "Erase(10)"},
    {0x2d, 0, "Read updated block"},
    {0x2e, 0, "Write and verify(10)"},
                        /* SBC-3 r31 recommends Write and verify(16) */
    {0x2f, 0, "Verify(10)"},    /* SBC-3 r31 recommends Verify(16) */
    {0x30, 0, "Search data high(10)"},
    {0x31, 0, "Search data equal(10)"},
    {0x32, 0, "Search data low(10)"},
    {0x33, 0, "Set limits(10)"},
    {0x34, 0, "Pre-fetch(10)"}, /* SBC-3 r31 recommends Pre-fetch(16) */
    {0x34, PDT_TAPE, "Read position"},
    {0x35, 0, "Synchronize cache(10)"},
                        /* SBC-3 r31 recommends Synchronize cache(16) */
    {0x36, 0, "Lock unlock cache(10)"},
    {0x37, 0, "Read defect data(10)"},
                        /* SBC-3 r31 recommends Read defect data(12) */
    {0x37, PDT_MCHANGER, "Initialize element status with range"},
    {0x38, 0, "Format with preset scan"},
    {0x38, PDT_OCRW, "Medium scan"},
    {0x39, 0, "Compare"},               /* obsolete in SPC-4 r11 */
    {0x3a, 0, "Copy and verify"},       /* obsolete in SPC-4 r11 */
    {0x3b, 0, "Write buffer"},
    {0x3c, 0, "Read buffer(10)"},
    {0x3d, 0, "Update block"},
    {0x3e, 0, "Read long(10)"},         /* obsolete in SBC-4 r7 */
    {0x3f, 0, "Write long(10)"}, /* SBC-3 r31 recommends Write long(16) */
    {0x40, 0, "Change definition"},     /* obsolete in SPC-4 r11 */
    {0x41, 0, "Write same(10)"}, /* SBC-3 r31 recommends Write same(16) */
    {0x42, 0, "Unmap"},                 /* added SPC-4 rev 18 */
    {0x42, PDT_MMC, "Read sub-channel"},
    {0x43, PDT_MMC, "Read TOC/PMA/ATIP"},
    {0x44, 0, "Report density support"},
    {0x45, PDT_MMC, "Play audio(10)"},
    {0x46, PDT_MMC, "Get configuration"},
    {0x47, PDT_MMC, "Play audio msf"},
    {0x48, 0, "Sanitize"},
    {0x4a, PDT_MMC, "Get event status notification"},
    {0x4b, PDT_MMC, "Pause/resume"},
    {0x4c, 0, "Log select"},
    {0x4d, 0, "Log sense"},
    {0x4e, 0, "Stop play/scan"},
    {0x50, 0, "Xdwrite(10)"},           /* obsolete in SBC-3 r31 */
    {0x51, 0, "Xpwrite(10)"},           /* obsolete in SBC-4 r15 */
    {0x51, PDT_MMC, "Read disk information"},
    {0x52, 0, "Xdread(10)"},            /* obsolete in SBC-3 r31 */
    {0x52, PDT_MMC, "Read track information"},
    {0x53, 0, "Xdwriteread(10)"},       /* obsolete in SBC-4 r15 */
    {0x54, 0, "Send OPC information"},
    {0x55, 0, "Mode select(10)"},
    {0x56, 0, "Reserve(10)"},           /* obsolete in SPC-4 r11 */
    {0x56, PDT_MCHANGER, "Reserve element(10)"},
    {0x57, 0, "Release(10)"},           /* obsolete in SPC-4 r11 */
    {0x57, PDT_MCHANGER, "Release element(10)"},
    {0x58, 0, "Repair track"},
    {0x5a, 0, "Mode sense(10)"},
    {0x5b, 0, "Close track/session"},
    {0x5c, 0, "Read buffer capacity"},
    {0x5d, 0, "Send cue sheet"},
    {0x5e, 0, "Persistent reserve in"},
    {0x5f, 0, "Persistent reserve out"},
    {0x7e, 0, "Extended cdb (XCBD)"},           /* added in SPC-4 r12 */
    {0x80, 0, "Xdwrite extended(16)"},          /* obsolete in SBC-4 r15 */
    {0x80, PDT_TAPE, "Write filemarks(16)"},
    {0x81, 0, "Rebuild(16)"},
    {0x81, PDT_TAPE, "Read reverse(16)"},
    {0x82, 0, "Regenerate(16)"},
    {0x83, 0, "Third party copy out"},  /* Extended copy, before spc4r34 */
        /* Following was "Receive copy results", before spc4r34 */
    {0x84, 0, "Third party copy in"},
    {0x85, 0, "ATA pass-through(16)"},  /* was 0x98 in spc3 rev21c */
    {0x86, 0, "Access control in"},
    {0x87, 0, "Access control out"},
    {0x88, 0, "Read(16)"},
    {0x89, 0, "Compare and write"},
    {0x8a, 0, "Write(16)"},
    {0x8b, 0, "Orwrite(16)"},
    {0x8c, 0, "Read attribute"},
    {0x8d, 0, "Write attribute"},
    {0x8e, 0, "Write and verify(16)"},
    {0x8f, 0, "Verify(16)"},
    {0x90, 0, "Pre-fetch(16)"},
    {0x91, 0, "Synchronize cache(16)"},
    {0x91, PDT_TAPE, "Space(16)"},
    {0x92, 0, "Lock unlock cache(16)"},
    {0x92, PDT_TAPE, "Locate(16)"},
    {0x93, 0, "Write same(16)"},
    {0x93, PDT_TAPE, "Erase(16)"},
    {0x94, PDT_ZBC, "ZBC out"},  /* new sbc4r04, has service actions */
    {0x95, PDT_ZBC, "ZBC in"},   /* new sbc4r04, has service actions */
    {0x9a, 0, "Write stream(16)"},      /* added sbc4r07 */
    {0x9b, 0, "Read buffer(16)"},       /* added spc5r02 */
    {0x9c, 0, "Write atomic(16)"},
    {0x9d, 0, "Service action bidirectional"},  /* added spc4r35 */
    {0x9e, 0, "Service action in(16)"},
    {0x9f, 0, "Service action out(16)"},
    {0xa0, 0, "Report luns"},
    {0xa1, 0, "ATA pass-through(12)"},
    {0xa1, PDT_MMC, "Blank"},
    {0xa2, 0, "Security protocol in"},
    {0xa3, 0, "Maintenance in"},
    {0xa3, PDT_MMC, "Send key"},
    {0xa4, 0, "Maintenance out"},
    {0xa4, PDT_MMC, "Report key"},
    {0xa5, 0, "Move medium"},
    {0xa5, PDT_MMC, "Play audio(12)"},
    {0xa6, 0, "Exchange medium"},
    {0xa6, PDT_MMC, "Load/unload medium"},
    {0xa7, 0, "Move medium attached"},
    {0xa7, PDT_MMC, "Set read ahead"},
    {0xa8, 0, "Read(12)"},      /* SBC-3 r31 recommends Read(16) */
    {0xa9, 0, "Service action out(12)"},
    {0xaa, 0, "Write(12)"},     /* SBC-3 r31 recommends Write(16) */
    {0xab, 0, "Service action in(12)"},
    {0xac, 0, "erase(12)"},
    {0xac, PDT_MMC, "Get performance"},
    {0xad, PDT_MMC, "Read DVD/BD structure"},
    {0xae, 0, "Write and verify(12)"},
                        /* SBC-3 r31 recommends Write and verify(16) */
    {0xaf, 0, "Verify(12)"},    /* SBC-3 r31 recommends Verify(16) */
    {0xb0, 0, "Search data high(12)"},
    {0xb1, 0, "Search data equal(12)"},
    {0xb1, PDT_MCHANGER, "Open/close import/export element"},
    {0xb2, 0, "Search data low(12)"},
    {0xb3, 0, "Set limits(12)"},
    {0xb4, 0, "Read element status attached"},
    {0xb5, 0, "Security protocol out"},
    {0xb5, PDT_MCHANGER, "Request volume element address"},
    {0xb6, 0, "Send volume tag"},
    {0xb6, PDT_MMC, "Set streaming"},
    {0xb7, 0, "Read defect data(12)"},
    {0xb8, 0, "Read element status"},
    {0xb9, 0, "Read CD msf"},
    {0xba, 0, "Redundancy group in"},
    {0xba, PDT_MMC, "Scan"},
    {0xbb, 0, "Redundancy group out"},
    {0xbb, PDT_MMC, "Set CD speed"},
    {0xbc, 0, "Spare in"},
    {0xbd, 0, "Spare out"},
    {0xbd, PDT_MMC, "Mechanism status"},
    {0xbe, 0, "Volume set in"},
    {0xbe, PDT_MMC, "Read CD"},
    {0xbf, 0, "Volume set out"},
    {0xbf, PDT_MMC, "Send DVD/BD structure"},
    {0xffff, 0, NULL},
};

/* Read buffer(10) [0x3c] and Read buffer(16) [0x9b] service actions (sa),
 * need prefix */
struct sg_lib_value_name_t sg_lib_read_buff_arr[] = {
    {0x0, 0, "combined header and data [or multiple modes]"},
    {0x2, 0, "data"},
    {0x3, 0, "descriptor"},
    {0xa, 0, "read data from echo buffer"},
    {0xb, 0, "echo buffer descriptor"},
    {0x1a, 0, "enable expander comms protocol and echo buffer"},
    {0x1c, 0, "error history"},
    {0xffff, 0, NULL},
};

/* Write buffer [0x3b] service actions, need prefix */
struct sg_lib_value_name_t sg_lib_write_buff_arr[] = {
    {0x0, 0, "combined header and data [or multiple modes]"},
    {0x2, 0, "data"},
    {0x4, 0, "download microcode and activate"},
    {0x5, 0, "download microcode, save, and activate"},
    {0x6, 0, "download microcode with offsets and activate"},
    {0x7, 0, "download microcode with offsets, save, and activate"},
    {0xa, 0, "write data to echo buffer"},
    {0xd, 0, "download microcode with offsets, select activation events, "
             "save and defer activate"},
    {0xe, 0, "download microcode with offsets, save and defer activate"},
    {0xf, 0, "activate deferred microcode"},
    {0x1a, 0, "enable expander comms protocol and echo buffer"},
    {0x1b, 0, "disable expander comms protocol"},
    {0x1c, 0, "download application client error history"},
    {0xffff, 0, NULL},
};

/* Read position (SSC) [0x34] service actions, need prefix */
struct sg_lib_value_name_t sg_lib_read_pos_arr[] = {
    {0x0, PDT_TAPE, "short form - block id"},
    {0x1, PDT_TAPE, "short form - vendor specific"},
    {0x6, PDT_TAPE, "long form"},
    {0x8, PDT_TAPE, "extended form"},
    {0xffff, 0, NULL},
};

/* Maintenance in [0xa3] service actions */
struct sg_lib_value_name_t sg_lib_maint_in_arr[] = {
    {0x0, PDT_SAC, "Report assigned/unassigned p_extent"},
    {0x0, PDT_ADC, "Report automation device attributes"},
    {0x1, PDT_SAC, "Report component device"},
    {0x2, PDT_SAC, "Report component device attachments"},
    {0x3, PDT_SAC, "Report peripheral device"},
    {0x4, PDT_SAC, "Report peripheral device associations"},
    {0x5, 0, "Report identifying information"},
                /* was "Report device identifier" prior to spc4r07 */
    {0x6, PDT_SAC, "Report states"},
    {0x7, PDT_SAC, "Report device identification"},
    {0x8, PDT_SAC, "Report unconfigured capacity"},
    {0x9, PDT_SAC, "Report supported configuration method"},
    {0xa, 0, "Report target port groups"},
    {0xb, 0, "Report aliases"},
    {0xc, 0, "Report supported operation codes"},
    {0xd, 0, "Report supported task management functions"},
    {0xe, 0, "Report priority"},
    {0xf, 0, "Report timestamp"},
    {0x10, 0, "Management protocol in"},
    {0x1d, PDT_DISK, "Report provisioning initialization pattern"},
        /* added in sbc4r07, shares sa 0x1d with ssc5r01 (tape) */
    {0x1d, PDT_TAPE, "Receive recommended access order"},
    {0x1e, PDT_TAPE, "Read dynamic runtime attribute"},
    {0x1e, PDT_ADC, "Report automation device attributes"},
    {0x1f, 0, "Maintenance in vendor specific"},
    {0xffff, 0, NULL},
};

/* Maintenance out [0xa4] service actions */
struct sg_lib_value_name_t sg_lib_maint_out_arr[] = {
    {0x0, PDT_SAC, "Add peripheral device / component device"},
    {0x0, PDT_ADC, "Set automation device attribute"},
    {0x1, PDT_SAC, "Attach to component device"},
    {0x2, PDT_SAC, "Exchange p_extent"},
    {0x3, PDT_SAC, "Exchange peripheral device / component device"},
    {0x4, PDT_SAC, "Instruct component device"},
    {0x5, PDT_SAC, "Remove peripheral device / component device"},
    {0x6, 0, "Set identifying information"},
                /* was "Set device identifier" prior to spc4r07 */
    {0x7, PDT_SAC, "Break peripheral device / component device"},
    {0xa, 0, "Set target port groups"},
    {0xb, 0, "Change aliases"},
    {0xc, 0, "Remove I_T nexus"},
    {0xe, 0, "Set priority"},
    {0xf, 0, "Set timestamp"},
    {0x10, 0, "Management protocol out"},
    {0x1d, PDT_TAPE, "Generate recommended access order"},
    {0x1e, PDT_TAPE, "write dynamic runtime attribute"},
    {0x1e, PDT_ADC, "Set automation device attributes"},
    {0x1f, 0, "Maintenance out vendor specific"},
    {0xffff, 0, NULL},
};

/* Sanitize [0x48] service actions, need prefix */
struct sg_lib_value_name_t sg_lib_sanitize_sa_arr[] = {
    {0x1, 0, "overwrite"},
    {0x2, 0, "block erase"},
    {0x3, 0, "cryptographic erase"},
    {0x1f, 0, "exit failure mode"},
    {0xffff, 0, NULL},
};

/* Service action in(12) [0xab] service actions */
struct sg_lib_value_name_t sg_lib_serv_in12_arr[] = {
    {0x1, 0, "Read media serial number"},
    {0xffff, 0, NULL},
};

/* Service action out(12) [0xa9] service actions */
struct sg_lib_value_name_t sg_lib_serv_out12_arr[] = {
    {0x1f, PDT_ADC, "Set medium attribute"},
    {0xff, 0, "Impossible command name"},
    {0xffff, 0, NULL},
};

/* Service action in(16) [0x9e] service actions */
struct sg_lib_value_name_t sg_lib_serv_in16_arr[] = {
    {0xf, 0, "Receive binding report"}, /* added spc5r11 */
    {0x10, 0, "Read capacity(16)"},
    {0x11, 0, "Read long(16)"},         /* obsolete in SBC-4 r7 */
    {0x12, 0, "Get LBA status(16)"},    /* 32 byte variant added in sbc4r14 */
    {0x13, 0, "Report referrals"},
    {0x14, 0, "Stream control"},
    {0x15, 0, "Background control"},
    {0x16, 0, "Get stream status"},
    {0x17, 0, "Get physical element status"},   /* added sbc4r13 */
    {0x18, 0, "Remove element and truncate"},   /* added sbc4r13 */
    {0x19, 0, "Restore elements and rebuild"},  /* added sbc4r19 */
    {0x1a, 0, "Remove element and modify zones"},   /* added zbc2r07 */
    {0xffff, 0, NULL},
};

/* Service action out(16) [0x9f] service actions */
struct sg_lib_value_name_t sg_lib_serv_out16_arr[] = {
    {0x0b, 0, "Test bind"},             /* added spc5r13 */
    {0x0c, 0, "Prepare bind report"},   /* added spc5r11 */
    {0x0d, 0, "Set affiliation"},
    {0x0e, 0, "Bind"},
    {0x0f, 0, "Unbind"},
    {0x11, 0, "Write long(16)"},
    {0x12, 0, "Write scattered(16)"},   /* added sbc4r11 */
    {0x14, PDT_ZBC, "Reset write pointer"},
    {0x1f, PDT_ADC, "Notify data transfer device(16)"},
    {0xffff, 0, NULL},
};

/* Service action bidirectional [0x9d] service actions */
struct sg_lib_value_name_t sg_lib_serv_bidi_arr[] = {
    {0xffff, 0, NULL},
};

/* Persistent reserve in [0x5e] service actions, need prefix */
struct sg_lib_value_name_t sg_lib_pr_in_arr[] = {
    {0x0, 0, "read keys"},
    {0x1, 0, "read reservation"},
    {0x2, 0, "report capabilities"},
    {0x3, 0, "read full status"},
    {0xffff, 0, NULL},
};

/* Persistent reserve out [0x5f] service actions, need prefix */
struct sg_lib_value_name_t sg_lib_pr_out_arr[] = {
    {0x0, 0, "register"},
    {0x1, 0, "reserve"},
    {0x2, 0, "release"},
    {0x3, 0, "clear"},
    {0x4, 0, "preempt"},
    {0x5, 0, "preempt and abort"},
    {0x6, 0, "register and ignore existing key"},
    {0x7, 0, "register and move"},
    {0x8, 0, "replace lost reservation"},
    {0xffff, 0, NULL},
};

/* Third party copy in [0x83] service actions
 * Opcode 'Receive copy results' was renamed 'Third party copy in' in spc4r34
 * LID1 is an abbreviation of List Identifier length of 1 byte. In SPC-5
 * LID1 discontinued (references back to SPC-4) and "(LID4)" suffix removed
 * as there is no need to differentiate. */
struct sg_lib_value_name_t sg_lib_xcopy_sa_arr[] = {    /* originating */
    {0x0, 0, "Extended copy(LID1)"},
    {0x1, 0, "Extended copy"},          /* was 'Extended copy(LID4)' */
    {0x10, 0, "Populate token"},
    {0x11, 0, "Write using token"},
    {0x16, 1, "Set tape stream mirroring"},     /* ADC-4 and SSC-5 */
    {0x1c, 0, "Copy operation abort"},
    {0xffff, 0, NULL},
};

/* Third party copy out [0x84] service actions
 * Opcode 'Extended copy' was renamed 'Third party copy out' in spc4r34
 * LID4 is an abbreviation of List Identifier length of 4 bytes */
struct sg_lib_value_name_t sg_lib_rec_copy_sa_arr[] = { /* retrieve */
    {0x0, 0, "Receive copy status(LID1)"},
    {0x1, 0, "Receive copy data(LID1)"},
    {0x3, 0, "Receive copy operating parameters"},
    {0x4, 0, "Receive copy failure details(LID1)"},
    {0x5, 0, "Receive copy status"},    /* was 'Receive copy status(LID4)' */
    {0x6, 0, "Receive copy data"},      /* was 'Receive copy data(LID4)' */
    {0x7, 0, "Receive ROD token information"},
    {0x8, 0, "Report all ROD tokens"},
    {0x16, 1, "Report tape stream mirroring"},  /* SSC-5 */
    {0xffff, 0, NULL},
};

/* Variable length cdb [0x7f] service actions (more than 16 bytes long) */
struct sg_lib_value_name_t sg_lib_variable_length_arr[] = {
    {0x1, 0, "Rebuild(32)"},
    {0x2, 0, "Regenerate(32)"},
    {0x3, 0, "Xdread(32)"},             /* obsolete in SBC-3 r31 */
    {0x4, 0, "Xdwrite(32)"},            /* obsolete in SBC-3 r31 */
    {0x5, 0, "Xdwrite extended(32)"},   /* obsolete in SBC-4 r15 */
    {0x6, 0, "Xpwrite(32)"},            /* obsolete in SBC-4 r15 */
    {0x7, 0, "Xdwriteread(32)"},        /* obsolete in SBC-4 r15 */
    {0x8, 0, "Xdwrite extended(64)"},   /* obsolete in SBC-4 r15 */
    {0x9, 0, "Read(32)"},
    {0xa, 0, "Verify(32)"},
    {0xb, 0, "Write(32)"},
    {0xc, 0, "Write and verify(32)"},
    {0xd, 0, "Write same(32)"},
    {0xe, 0, "Orwrite(32)"},            /* added sbc3r25 */
    {0xf, 0, "Atomic write(32)"},       /* added sbc4r02 */
    {0x10, 0, "Write stream(32)"},      /* added sbc4r07 */
    {0x11, 0, "Write scattered(32)"},   /* added sbc4r11 */
    {0x12, 0, "Get LBA status(32)"},    /* added sbc4r14 */
    {0x1800, 0, "Receive credential"},
    {0x1ff0, 0, "ATA pass-through(32)"},/* added sat4r05 */
    {0x8801, 0, "Format OSD (osd)"},
    {0x8802, 0, "Create (osd)"},
    {0x8803, 0, "List (osd)"},
    {0x8805, 0, "Read (osd)"},
    {0x8806, 0, "Write (osd)"},
    {0x8807, 0, "Append (osd)"},
    {0x8808, 0, "Flush (osd)"},
    {0x880a, 0, "Remove (osd)"},
    {0x880b, 0, "Create partition (osd)"},
    {0x880c, 0, "Remove partition (osd)"},
    {0x880e, 0, "Get attributes (osd)"},
    {0x880f, 0, "Set attributes (osd)"},
    {0x8812, 0, "Create and write (osd)"},
    {0x8815, 0, "Create collection (osd)"},
    {0x8816, 0, "Remove collection (osd)"},
    {0x8817, 0, "List collection (osd)"},
    {0x8818, 0, "Set key (osd)"},
    {0x8819, 0, "Set master key (osd)"},
    {0x881a, 0, "Flush collection (osd)"},
    {0x881b, 0, "Flush partition (osd)"},
    {0x881c, 0, "Flush OSD (osd)"},
    {0x8880, 0, "Object structure check (osd-2)"},
    {0x8881, 0, "Format OSD (osd-2)"},
    {0x8882, 0, "Create (osd-2)"},
    {0x8883, 0, "List (osd-2)"},
    {0x8884, 0, "Punch (osd-2)"},
    {0x8885, 0, "Read (osd-2)"},
    {0x8886, 0, "Write (osd-2)"},
    {0x8887, 0, "Append (osd-2)"},
    {0x8888, 0, "Flush (osd-2)"},
    {0x8889, 0, "Clear (osd-2)"},
    {0x888a, 0, "Remove (osd-2)"},
    {0x888b, 0, "Create partition (osd-2)"},
    {0x888c, 0, "Remove partition (osd-2)"},
    {0x888e, 0, "Get attributes (osd-2)"},
    {0x888f, 0, "Set attributes (osd-2)"},
    {0x8892, 0, "Create and write (osd-2)"},
    {0x8895, 0, "Create collection (osd-2)"},
    {0x8896, 0, "Remove collection (osd-2)"},
    {0x8897, 0, "List collection (osd-2)"},
    {0x8898, 0, "Set key (osd-2)"},
    {0x8899, 0, "Set master key (osd-2)"},
    {0x889a, 0, "Flush collection (osd-2)"},
    {0x889b, 0, "Flush partition (osd-2)"},
    {0x889c, 0, "Flush OSD (osd-2)"},
    {0x88a0, 0, "Query (osd-2)"},
    {0x88a1, 0, "Remove member objects (osd-2)"},
    {0x88a2, 0, "Get member attributes (osd-2)"},
    {0x88a3, 0, "Set member attributes (osd-2)"},
    {0x88b1, 0, "Read map (osd-2)"},
    {0x8f7c, 0, "Perform SCSI command (osd-2)"},
    {0x8f7d, 0, "Perform task management function (osd-2)"},
    {0x8f7e, 0, "Perform SCSI command (osd)"},
    {0x8f7f, 0, "Perform task management function (osd)"},
    {0xffff, 0, NULL},
};

/* Zoning out [0x94] service actions */
struct sg_lib_value_name_t sg_lib_zoning_out_arr[] = {
    {0x1, PDT_ZBC, "Close zone"},
    {0x2, PDT_ZBC, "Finish zone"},
    {0x3, PDT_ZBC, "Open zone"},
    {0x4, PDT_ZBC, "Reset write pointer"},
    {0x10, PDT_ZBC, "Sequentialize zone"},      /* zbc2r01b */
    {0xffff, 0, NULL},
};

/* Zoning in [0x95] service actions */
struct sg_lib_value_name_t sg_lib_zoning_in_arr[] = {
    {0x0, PDT_ZBC, "Report zones"},
    {0x6, PDT_ZBC, "Report realms"},            /* zbc2r04 */
    {0x7, PDT_ZBC, "Report zone domains"},      /* zbc2r04 */
    {0x8, PDT_ZBC, "Zone activate"},            /* zbc2r04 */
    {0x9, PDT_ZBC, "Zone query"},               /* zbc2r04 */
    {0xffff, 0, NULL},
};

/* Read attribute [0x8c] service actions */
struct sg_lib_value_name_t sg_lib_read_attr_arr[] = {
    {0x0, 0, "attribute values"},
    {0x1, 0, "attribute list"},
    {0x2, 0, "logical volume list"},
    {0x3, 0, "partition list"},
    {0x5, 0, "supported attributes"},
    {0xffff, 0, NULL},
};

#else   /* SG_SCSI_STRINGS */

struct sg_lib_value_name_t sg_lib_normal_opcodes[] = {
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_read_buff_arr[] = {  /* opcode 0x3c */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_write_buff_arr[] = {  /* opcode 0x3b */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_read_pos_arr[] = {  /* opcode 0x34 (SSC) */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_maint_in_arr[] = {  /* opcode 0xa3 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_maint_out_arr[] = {  /* opcode 0xa4 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_sanitize_sa_arr[] = {  /* opcode 0x94 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_serv_in12_arr[] = { /* opcode 0xab */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_serv_out12_arr[] = { /* opcode 0xa9 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_serv_in16_arr[] = { /* opcode 0x9e */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_serv_out16_arr[] = { /* opcode 0x9f */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_serv_bidi_arr[] = { /* opcode 0x9d */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_pr_in_arr[] = { /* opcode 0x5e */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_pr_out_arr[] = { /* opcode 0x5f */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_xcopy_sa_arr[] = { /* opcode 0x83 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_rec_copy_sa_arr[] = { /* opcode 0x84 */
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_variable_length_arr[] = {
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_zoning_out_arr[] = {
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_zoning_in_arr[] = {
    {0xffff, 0, NULL},
};

struct sg_lib_value_name_t sg_lib_read_attr_arr[] = {
    {0xffff, 0, NULL},
};

#endif  /* SG_SCSI_STRINGS */

/* A conveniently formatted list of SCSI ASC/ASCQ codes and their
 * corresponding text can be found at: www.t10.org/lists/asc-num.txt
 * The following should match asc-num.txt dated 20200817 */

#ifdef SG_SCSI_STRINGS
struct sg_lib_asc_ascq_range_t sg_lib_asc_ascq_range[] =
{
    {0x40,0x01,0x7f,"Ram failure [0x%x]"},
    {0x40,0x80,0xff,"Diagnostic failure on component [0x%x]"},
    {0x41,0x01,0xff,"Data path failure [0x%x]"},
    {0x42,0x01,0xff,"Power-on or self-test failure [0x%x]"},
    {0x4d,0x00,0xff,"Tagged overlapped commands [0x%x]"},
    {0x70,0x00,0xff,"Decompression exception short algorithm id of 0x%x"},
    {0, 0, 0, NULL}
};

struct sg_lib_asc_ascq_t sg_lib_asc_ascq[] =
{
    {0x00,0x00,"No additional sense information"},
    {0x00,0x01,"Filemark detected"},
    {0x00,0x02,"End-of-partition/medium detected"},
    {0x00,0x03,"Setmark detected"},
    {0x00,0x04,"Beginning-of-partition/medium detected"},
    {0x00,0x05,"End-of-data detected"},
    {0x00,0x06,"I/O process terminated"},
    {0x00,0x07,"Programmable early warning detected"},
    {0x00,0x11,"Audio play operation in progress"},
    {0x00,0x12,"Audio play operation paused"},
    {0x00,0x13,"Audio play operation successfully completed"},
    {0x00,0x14,"Audio play operation stopped due to error"},
    {0x00,0x15,"No current audio status to return"},
    {0x00,0x16,"operation in progress"},
    {0x00,0x17,"Cleaning requested"},
    {0x00,0x18,"Erase operation in progress"},
    {0x00,0x19,"Locate operation in progress"},
    {0x00,0x1a,"Rewind operation in progress"},
    {0x00,0x1b,"Set capacity operation in progress"},
    {0x00,0x1c,"Verify operation in progress"},
    {0x00,0x1d,"ATA pass through information available"},
    {0x00,0x1e,"Conflicting SA creation request"},
    {0x00,0x1f,"Logical unit transitioning to another power condition"},
    {0x00,0x20,"Extended copy information available"},
    {0x00,0x21,"Atomic command aborted due to ACA"},
    {0x00,0x22,"Deferred microcode is pending"},
    {0x01,0x00,"No index/sector signal"},
    {0x02,0x00,"No seek complete"},
    {0x03,0x00,"Peripheral device write fault"},
    {0x03,0x01,"No write current"},
    {0x03,0x02,"Excessive write errors"},
    {0x04,0x00,"Logical unit not ready, cause not reportable"},
    {0x04,0x01,"Logical unit is in process of becoming ready"},
    {0x04,0x02,"Logical unit not ready, "
                "initializing command required"},
    {0x04,0x03,"Logical unit not ready, "
                "manual intervention required"},
    {0x04,0x04,"Logical unit not ready, format in progress"},
    {0x04,0x05,"Logical unit not ready, rebuild in progress"},
    {0x04,0x06,"Logical unit not ready, recalculation in progress"},
    {0x04,0x07,"Logical unit not ready, operation in progress"},
    {0x04,0x08,"Logical unit not ready, long write in progress"},
    {0x04,0x09,"Logical unit not ready, self-test in progress"},
    {0x04,0x0a,"Logical unit "
                "not accessible, asymmetric access state transition"},
    {0x04,0x0b,"Logical unit "
                "not accessible, target port in standby state"},
    {0x04,0x0c,"Logical unit "
                "not accessible, target port in unavailable state"},
    {0x04,0x0d,"Logical unit not ready, structure check required"},
    {0x04,0x0e,"Logical unit not ready, security session in progress"},
    {0x04,0x10,"Logical unit not ready, "
                "auxiliary memory not accessible"},
    {0x04,0x11,"Logical unit not ready, "
                "notify (enable spinup) required"},
    {0x04,0x12,"Logical unit not ready, offline"},
    {0x04,0x13,"Logical unit not ready, SA creation in progress"},
    {0x04,0x14,"Logical unit not ready, space allocation in progress"},
    {0x04,0x15,"Logical unit not ready, robotics disabled"},
    {0x04,0x16,"Logical unit not ready, configuration required"},
    {0x04,0x17,"Logical unit not ready, calibration required"},
    {0x04,0x18,"Logical unit not ready, a door is open"},
    {0x04,0x19,"Logical unit not ready, operating in sequential mode"},
    {0x04,0x1a,"Logical unit not ready, start stop unit command in progress"},
    {0x04,0x1b,"Logical unit not ready, sanitize in progress"},
    {0x04,0x1c,"Logical unit not ready, additional power use not yet "
                "granted"},
    {0x04,0x1d,"Logical unit not ready, configuration in progress"},
    {0x04,0x1e,"Logical unit not ready, microcode activation required"},
    {0x04,0x1f,"Logical unit not ready, microcode download required"},
    {0x04,0x20,"Logical unit not ready, logical unit reset required"},
    {0x04,0x21,"Logical unit not ready, hard reset required"},
    {0x04,0x22,"Logical unit not ready, power cycle required"},
    {0x04,0x23,"Logical unit not ready, affiliation required"},
    {0x04,0x24,"Depopulation in progress"},             /* spc5r15 */
    {0x04,0x25,"Depopulation restoration in progress"}, /* spc6r02 */
    {0x05,0x00,"Logical unit does not respond to selection"},
    {0x06,0x00,"No reference position found"},
    {0x07,0x00,"Multiple peripheral devices selected"},
    {0x08,0x00,"Logical unit communication failure"},
    {0x08,0x01,"Logical unit communication time-out"},
    {0x08,0x02,"Logical unit communication parity error"},
    {0x08,0x03,"Logical unit communication CRC error (Ultra-DMA/32)"},
    {0x08,0x04,"Unreachable copy target"},
    {0x09,0x00,"Track following error"},
    {0x09,0x01,"Tracking servo failure"},
    {0x09,0x02,"Focus servo failure"},
    {0x09,0x03,"Spindle servo failure"},
    {0x09,0x04,"Head select fault"},
    {0x09,0x05,"Vibration induced tracking error"},
    {0x0A,0x00,"Error log overflow"},
    {0x0B,0x00,"Warning"},
    {0x0B,0x01,"Warning - specified temperature exceeded"},
    {0x0B,0x02,"Warning - enclosure degraded"},
    {0x0B,0x03,"Warning - background self-test failed"},
    {0x0B,0x04,"Warning - background pre-scan detected medium error"},
    {0x0B,0x05,"Warning - background medium scan detected medium error"},
    {0x0B,0x06,"Warning - non-volatile cache now volatile"},
    {0x0B,0x07,"Warning - degraded power to non-volatile cache"},
    {0x0B,0x08,"Warning - power loss expected"},
    {0x0B,0x09,"Warning - device statistics notification active"},
    {0x0B,0x0A,"Warning - high critical temperature limit exceeded"},
    {0x0B,0x0B,"Warning - low critical temperature limit exceeded"},
    {0x0B,0x0C,"Warning - high operating temperature limit exceeded"},
    {0x0B,0x0D,"Warning - low operating temperature limit exceeded"},
    {0x0B,0x0E,"Warning - high critical humidity limit exceeded"},
    {0x0B,0x0F,"Warning - low critical humidity limit exceeded"},
    {0x0B,0x10,"Warning - high operating humidity limit exceeded"},
    {0x0B,0x11,"Warning - low operating humidity limit exceeded"},
    {0x0B,0x12,"Warning - microcode security at risk"},
    {0x0B,0x13,"Warning - microcode digital signature validation failure"},
    {0x0B,0x14,"Warning - physical element status change"},     /* spc5r15 */
    {0x0C,0x00,"Write error"},
    {0x0C,0x01,"Write error - recovered with auto reallocation"},
    {0x0C,0x02,"Write error - auto reallocation failed"},
    {0x0C,0x03,"Write error - recommend reassignment"},
    {0x0C,0x04,"Compression check miscompare error"},
    {0x0C,0x05,"Data expansion occurred during compression"},
    {0x0C,0x06,"Block not compressible"},
    {0x0C,0x07,"Write error - recovery needed"},
    {0x0C,0x08,"Write error - recovery failed"},
    {0x0C,0x09,"Write error - loss of streaming"},
    {0x0C,0x0A,"Write error - padding blocks added"},
    {0x0C,0x0B,"Auxiliary memory write error"},
    {0x0C,0x0C,"Write error - unexpected unsolicited data"},
    {0x0C,0x0D,"Write error - not enough unsolicited data"},
    {0x0C,0x0E,"Multiple write errors"},
    {0x0C,0x0F,"Defects in error window"},
    {0x0C,0x10,"Incomplete multiple atomic write operations"},
    {0x0C,0x11,"Write error - recovery scan needed"},
    {0x0C,0x12,"Write error - insufficient zone resources"},
    {0x0D,0x00,"Error detected by third party temporary initiator"},
    {0x0D,0x01,"Third party device failure"},
    {0x0D,0x02,"Copy target device not reachable"},
    {0x0D,0x03,"Incorrect copy target device type"},
    {0x0D,0x04,"Copy target device data underrun"},
    {0x0D,0x05,"Copy target device data overrun"},
    {0x0E,0x00,"Invalid information unit"},
    {0x0E,0x01,"Information unit too short"},
    {0x0E,0x02,"Information unit too long"},
    {0x0E,0x03,"Invalid field in command information unit"},
    {0x10,0x00,"Id CRC or ECC error"},
    {0x10,0x01,"Logical block guard check failed"},
    {0x10,0x02,"Logical block application tag check failed"},
    {0x10,0x03,"Logical block reference tag check failed"},
    {0x10,0x04,"Logical block protection error on recover buffered data"},
    {0x10,0x05,"Logical block protection method error"},
    {0x11,0x00,"Unrecovered read error"},
    {0x11,0x01,"Read retries exhausted"},
    {0x11,0x02,"Error too long to correct"},
    {0x11,0x03,"Multiple read errors"},
    {0x11,0x04,"Unrecovered read error - auto reallocate failed"},
    {0x11,0x05,"L-EC uncorrectable error"},
    {0x11,0x06,"CIRC unrecovered error"},
    {0x11,0x07,"Data re-synchronization error"},
    {0x11,0x08,"Incomplete block read"},
    {0x11,0x09,"No gap found"},
    {0x11,0x0A,"Miscorrected error"},
    {0x11,0x0B,"Unrecovered read error - recommend reassignment"},
    {0x11,0x0C,"Unrecovered read error - recommend rewrite the data"},
    {0x11,0x0D,"De-compression CRC error"},
    {0x11,0x0E,"Cannot decompress using declared algorithm"},
    {0x11,0x0F,"Error reading UPC/EAN number"},
    {0x11,0x10,"Error reading ISRC number"},
    {0x11,0x11,"Read error - loss of streaming"},
    {0x11,0x12,"Auxiliary memory read error"},
    {0x11,0x13,"Read error - failed retransmission request"},
    {0x11,0x14,"Read error - LBA marked bad by application client"},
    {0x11,0x15,"Write after sanitize required"},
    {0x12,0x00,"Address mark not found for id field"},
    {0x13,0x00,"Address mark not found for data field"},
    {0x14,0x00,"Recorded entity not found"},
    {0x14,0x01,"Record not found"},
    {0x14,0x02,"Filemark or setmark not found"},
    {0x14,0x03,"End-of-data not found"},
    {0x14,0x04,"Block sequence error"},
    {0x14,0x05,"Record not found - recommend reassignment"},
    {0x14,0x06,"Record not found - data auto-reallocated"},
    {0x14,0x07,"Locate operation failure"},
    {0x15,0x00,"Random positioning error"},
    {0x15,0x01,"Mechanical positioning error"},
    {0x15,0x02,"Positioning error detected by read of medium"},
    {0x16,0x00,"Data synchronization mark error"},
    {0x16,0x01,"Data sync error - data rewritten"},
    {0x16,0x02,"Data sync error - recommend rewrite"},
    {0x16,0x03,"Data sync error - data auto-reallocated"},
    {0x16,0x04,"Data sync error - recommend reassignment"},
    {0x17,0x00,"Recovered data with no error correction applied"},
    {0x17,0x01,"Recovered data with retries"},
    {0x17,0x02,"Recovered data with positive head offset"},
    {0x17,0x03,"Recovered data with negative head offset"},
    {0x17,0x04,"Recovered data with retries and/or circ applied"},
    {0x17,0x05,"Recovered data using previous sector id"},
    {0x17,0x06,"Recovered data without ECC - data auto-reallocated"},
    {0x17,0x07,"Recovered data without ECC - recommend reassignment"},
    {0x17,0x08,"Recovered data without ECC - recommend rewrite"},
    {0x17,0x09,"Recovered data without ECC - data rewritten"},
    {0x18,0x00,"Recovered data with error correction applied"},
    {0x18,0x01,"Recovered data with error corr. & retries applied"},
    {0x18,0x02,"Recovered data - data auto-reallocated"},
    {0x18,0x03,"Recovered data with CIRC"},
    {0x18,0x04,"Recovered data with L-EC"},
    {0x18,0x05,"Recovered data - recommend reassignment"},
    {0x18,0x06,"Recovered data - recommend rewrite"},
    {0x18,0x07,"Recovered data with ECC - data rewritten"},
    {0x18,0x08,"Recovered data with linking"},
    {0x19,0x00,"Defect list error"},
    {0x19,0x01,"Defect list not available"},
    {0x19,0x02,"Defect list error in primary list"},
    {0x19,0x03,"Defect list error in grown list"},
    {0x1A,0x00,"Parameter list length error"},
    {0x1B,0x00,"Synchronous data transfer error"},
    {0x1C,0x00,"Defect list not found"},
    {0x1C,0x01,"Primary defect list not found"},
    {0x1C,0x02,"Grown defect list not found"},
    {0x1D,0x00,"Miscompare during verify operation"},
    {0x1D,0x01,"Miscompare verify of unmapped lba"},
    {0x1E,0x00,"Recovered id with ECC correction"},
    {0x1F,0x00,"Partial defect list transfer"},
    {0x20,0x00,"Invalid command operation code"},
    {0x20,0x01,"Access denied - initiator pending-enrolled"},
    {0x20,0x02,"Access denied - no access rights"},
    {0x20,0x03,"Access denied - invalid mgmt id key"},
    {0x20,0x04,"Illegal command while in write capable state"},
    {0x20,0x05,"Write type operation while in read capable state (obs)"},
    {0x20,0x06,"Illegal command while in explicit address mode"},
    {0x20,0x07,"Illegal command while in implicit address mode"},
    {0x20,0x08,"Access denied - enrollment conflict"},
    {0x20,0x09,"Access denied - invalid LU identifier"},
    {0x20,0x0A,"Access denied - invalid proxy token"},
    {0x20,0x0B,"Access denied - ACL LUN conflict"},
    {0x20,0x0C,"Illegal command when not in append-only mode"},
    {0x20,0x0D,"Not an administrative logical unit"},
    {0x20,0x0E,"Not a subsidiary logical unit"},
    {0x20,0x0F,"Not a conglomerate logical unit"},
    {0x21,0x00,"Logical block address out of range"},
    {0x21,0x01,"Invalid element address"},
    {0x21,0x02,"Invalid address for write"},
    {0x21,0x03,"Invalid write crossing layer jump"},
    {0x21,0x04,"Unaligned write command"},
    {0x21,0x05,"Write boundary violation"},
    {0x21,0x06,"Attempt to read invalid data"},
    {0x21,0x07,"Read boundary violation"},
    {0x21,0x08,"Misaligned write command"},
    {0x21,0x09,"Attempt to access gap zone"},
    {0x22,0x00,"Illegal function (use 20 00, 24 00, or 26 00)"},
    {0x23,0x00,"Invalid token operation, cause not reportable"},
    {0x23,0x01,"Invalid token operation, unsupported token type"},
    {0x23,0x02,"Invalid token operation, remote token usage not supported"},
    {0x23,0x03,"invalid token operation, remote rod token creation not "
               "supported"},
    {0x23,0x04,"Invalid token operation, token unknown"},
    {0x23,0x05,"Invalid token operation, token corrupt"},
    {0x23,0x06,"Invalid token operation, token revoked"},
    {0x23,0x07,"Invalid token operation, token expired"},
    {0x23,0x08,"Invalid token operation, token cancelled"},
    {0x23,0x09,"Invalid token operation, token deleted"},
    {0x23,0x0a,"Invalid token operation, invalid token length"},
    {0x24,0x00,"Invalid field in cdb"},
    {0x24,0x01,"CDB decryption error"},
    {0x24,0x02,"Invalid cdb field while in explicit block model (obs)"},
    {0x24,0x03,"Invalid cdb field while in implicit block model (obs)"},
    {0x24,0x04,"Security audit value frozen"},
    {0x24,0x05,"Security working key frozen"},
    {0x24,0x06,"Nonce not unique"},
    {0x24,0x07,"Nonce timestamp out of range"},
    {0x24,0x08,"Invalid xcdb"},
    {0x24,0x09,"Invalid fast format"},
    {0x25,0x00,"Logical unit not supported"},
    {0x26,0x00,"Invalid field in parameter list"},
    {0x26,0x01,"Parameter not supported"},
    {0x26,0x02,"Parameter value invalid"},
    {0x26,0x03,"Threshold parameters not supported"},
    {0x26,0x04,"Invalid release of persistent reservation"},
    {0x26,0x05,"Data decryption error"},
    {0x26,0x06,"Too many target descriptors"},
    {0x26,0x07,"Unsupported target descriptor type code"},
    {0x26,0x08,"Too many segment descriptors"},
    {0x26,0x09,"Unsupported segment descriptor type code"},
    {0x26,0x0A,"Unexpected inexact segment"},
    {0x26,0x0B,"Inline data length exceeded"},
    {0x26,0x0C,"Invalid operation for copy source or destination"},
    {0x26,0x0D,"Copy segment granularity violation"},
    {0x26,0x0E,"Invalid parameter while port is enabled"},
    {0x26,0x0F,"Invalid data-out buffer integrity check value"},
    {0x26,0x10,"Data decryption key fail limit reached"},
    {0x26,0x11,"Incomplete key-associated data set"},
    {0x26,0x12,"Vendor specific key reference not found"},
    {0x26,0x13,"Application tag mode page is invalid"},
    {0x26,0x14,"Tape stream mirroring prevented"},
    {0x26,0x15,"Copy source or copy destination not authorized"},
    {0x26,0x16,"Fast copy not possible"},
    {0x27,0x00,"Write protected"},
    {0x27,0x01,"Hardware write protected"},
    {0x27,0x02,"Logical unit software write protected"},
    {0x27,0x03,"Associated write protect"},
    {0x27,0x04,"Persistent write protect"},
    {0x27,0x05,"Permanent write protect"},
    {0x27,0x06,"Conditional write protect"},
    {0x27,0x07,"Space allocation failed write protect"},
    {0x27,0x08,"Zone is read only"},
    {0x28,0x00,"Not ready to ready change, medium may have changed"},
    {0x28,0x01,"Import or export element accessed"},
    {0x28,0x02,"Format-layer may have changed"},
    {0x28,0x03,"Import/export element accessed, medium changed"},
    {0x29,0x00,"Power on, reset, or bus device reset occurred"},
    {0x29,0x01,"Power on occurred"},
    {0x29,0x02,"SCSI bus reset occurred"},
    {0x29,0x03,"Bus device reset function occurred"},
    {0x29,0x04,"Device internal reset"},
    {0x29,0x05,"Transceiver mode changed to single-ended"},
    {0x29,0x06,"Transceiver mode changed to lvd"},
    {0x29,0x07,"I_T nexus loss occurred"},
    {0x2A,0x00,"Parameters changed"},
    {0x2A,0x01,"Mode parameters changed"},
    {0x2A,0x02,"Log parameters changed"},
    {0x2A,0x03,"Reservations preempted"},
    {0x2A,0x04,"Reservations released"},
    {0x2A,0x05,"Registrations preempted"},
    {0x2A,0x06,"Asymmetric access state changed"},
    {0x2A,0x07,"Implicit asymmetric access state transition failed"},
    {0x2A,0x08,"Priority changed"},
    {0x2A,0x09,"Capacity data has changed"},
    {0x2A,0x0c, "Error recovery attributes have changed"},
    {0x2A,0x0d, "Data encryption capabilities changed"},
    {0x2A,0x10,"Timestamp changed"},
    {0x2A,0x11,"Data encryption parameters changed by another i_t nexus"},
    {0x2A,0x12,"Data encryption parameters changed by vendor specific event"},
    {0x2A,0x13,"Data encryption key instance counter has changed"},
    {0x2A,0x0a,"Error history i_t nexus cleared"},
    {0x2A,0x0b,"Error history snapshot released"},
    {0x2A,0x14,"SA creation capabilities data has changed"},
    {0x2A,0x15,"Medium removal prevention preempted"},
    {0x2A,0x16,"Zone reset write pointer recommended"},
    {0x2B,0x00,"Copy cannot execute since host cannot disconnect"},
    {0x2C,0x00,"Command sequence error"},
    {0x2C,0x01,"Too many windows specified"},
    {0x2C,0x02,"Invalid combination of windows specified"},
    {0x2C,0x03,"Current program area is not empty"},
    {0x2C,0x04,"Current program area is empty"},
    {0x2C,0x05,"Illegal power condition request"},
    {0x2C,0x06,"Persistent prevent conflict"},
    {0x2C,0x07,"Previous busy status"},
    {0x2C,0x08,"Previous task set full status"},
    {0x2C,0x09,"Previous reservation conflict status"},
    {0x2C,0x0A,"Partition or collection contains user objects"},
    {0x2C,0x0B,"Not reserved"},
    {0x2C,0x0C,"ORWRITE generation does not match"},
    {0x2C,0x0D,"Reset write pointer not allowed"},
    {0x2C,0x0E,"Zone is offline"},
    {0x2C,0x0F,"Stream not open"},
    {0x2C,0x10,"Unwritten data in zone"},
    {0x2C,0x11,"Descriptor format sense data required"},
    {0x2C,0x12,"Zone is inactive"},
    {0x2C,0x13,"Well known logical unit access required"},      /* spc6r02 */
    {0x2D,0x00,"Overwrite error on update in place"},
    {0x2E,0x00,"Insufficient time for operation"},
    {0x2E,0x01,"Command timeout before processing"},
    {0x2E,0x02,"Command timeout during processing"},
    {0x2E,0x03,"Command timeout during processing due to error recovery"},
    {0x2F,0x00,"Commands cleared by another initiator"},
    {0x2F,0x01,"Commands cleared by power loss notification"},
    {0x2F,0x02,"Commands cleared by device server"},
    {0x2F,0x03,"Some commands cleared by queuing layer event"},
    {0x30,0x00,"Incompatible medium installed"},
    {0x30,0x01,"Cannot read medium - unknown format"},
    {0x30,0x02,"Cannot read medium - incompatible format"},
    {0x30,0x03,"Cleaning cartridge installed"},
    {0x30,0x04,"Cannot write medium - unknown format"},
    {0x30,0x05,"Cannot write medium - incompatible format"},
    {0x30,0x06,"Cannot format medium - incompatible medium"},
    {0x30,0x07,"Cleaning failure"},
    {0x30,0x08,"Cannot write - application code mismatch"},
    {0x30,0x09,"Current session not fixated for append"},
    {0x30,0x0A,"Cleaning request rejected"},
    {0x30,0x0B,"Cleaning tape expired"},
    {0x30,0x0C,"WORM medium - overwrite attempted"},
    {0x30,0x0D,"WORM medium - integrity check"},
    {0x30,0x10,"Medium not formatted"},
    {0x30,0x11,"Incompatible volume type"},
    {0x30,0x12,"Incompatible volume qualifier"},
    {0x30,0x13,"Cleaning volume expired"},
    {0x31,0x00,"Medium format corrupted"},
    {0x31,0x01,"Format command failed"},
    {0x31,0x02,"Zoned formatting failed due to spare linking"},
    {0x31,0x03,"Sanitize command failed"},
    {0x31,0x04,"Depopulation failed"},               /* spc5r15 */
    {0x31,0x05,"Depopulation restoration failed"},   /* spc6r02 */
    {0x32,0x00,"No defect spare location available"},
    {0x32,0x01,"Defect list update failure"},
    {0x33,0x00,"Tape length error"},
    {0x34,0x00,"Enclosure failure"},
    {0x35,0x00,"Enclosure services failure"},
    {0x35,0x01,"Unsupported enclosure function"},
    {0x35,0x02,"Enclosure services unavailable"},
    {0x35,0x03,"Enclosure services transfer failure"},
    {0x35,0x04,"Enclosure services transfer refused"},
    {0x35,0x05,"Enclosure services checksum error"},
    {0x36,0x00,"Ribbon, ink, or toner failure"},
    {0x37,0x00,"Rounded parameter"},
    {0x38,0x00,"Event status notification"},
    {0x38,0x02,"Esn - power management class event"},
    {0x38,0x04,"Esn - media class event"},
    {0x38,0x06,"Esn - device busy class event"},
    {0x38,0x07,"Thin provisioning soft threshold reached"},
    {0x38,0x08,"Depopulation interrupted"},     /* spc6r03 */
    {0x39,0x00,"Saving parameters not supported"},
    {0x3A,0x00,"Medium not present"},
    {0x3A,0x01,"Medium not present - tray closed"},
    {0x3A,0x02,"Medium not present - tray open"},
    {0x3A,0x03,"Medium not present - loadable"},
    {0x3A,0x04,"Medium not present - medium auxiliary memory accessible"},
    {0x3B,0x00,"Sequential positioning error"},
    {0x3B,0x01,"Tape position error at beginning-of-medium"},
    {0x3B,0x02,"Tape position error at end-of-medium"},
    {0x3B,0x03,"Tape or electronic vertical forms unit not ready"},
    {0x3B,0x04,"Slew failure"},
    {0x3B,0x05,"Paper jam"},
    {0x3B,0x06,"Failed to sense top-of-form"},
    {0x3B,0x07,"Failed to sense bottom-of-form"},
    {0x3B,0x08,"Reposition error"},
    {0x3B,0x09,"Read past end of medium"},
    {0x3B,0x0A,"Read past beginning of medium"},
    {0x3B,0x0B,"Position past end of medium"},
    {0x3B,0x0C,"Position past beginning of medium"},
    {0x3B,0x0D,"Medium destination element full"},
    {0x3B,0x0E,"Medium source element empty"},
    {0x3B,0x0F,"End of medium reached"},
    {0x3B,0x11,"Medium magazine not accessible"},
    {0x3B,0x12,"Medium magazine removed"},
    {0x3B,0x13,"Medium magazine inserted"},
    {0x3B,0x14,"Medium magazine locked"},
    {0x3B,0x15,"Medium magazine unlocked"},
    {0x3B,0x16,"Mechanical positioning or changer error"},
    {0x3B,0x17,"Read past end of user object"},
    {0x3B,0x18,"Element disabled"},
    {0x3B,0x19,"Element enabled"},
    {0x3B,0x1a,"Data transfer device removed"},
    {0x3B,0x1b,"Data transfer device inserted"},
    {0x3B,0x1c,"Too many logical objects on partition to support operation"},
    {0x3B,0x20,"Element static information changed"},
    {0x3D,0x00,"Invalid bits in identify message"},
    {0x3E,0x00,"Logical unit has not self-configured yet"},
    {0x3E,0x01,"Logical unit failure"},
    {0x3E,0x02,"Timeout on logical unit"},
    {0x3E,0x03,"Logical unit failed self-test"},
    {0x3E,0x04,"Logical unit unable to update self-test log"},
    {0x3F,0x00,"Target operating conditions have changed"},
    {0x3F,0x01,"Microcode has been changed"},
    {0x3F,0x02,"Changed operating definition"},
    {0x3F,0x03,"Inquiry data has changed"},
    {0x3F,0x04,"Component device attached"},
    {0x3F,0x05,"Device identifier changed"},
    {0x3F,0x06,"Redundancy group created or modified"},
    {0x3F,0x07,"Redundancy group deleted"},
    {0x3F,0x08,"Spare created or modified"},
    {0x3F,0x09,"Spare deleted"},
    {0x3F,0x0A,"Volume set created or modified"},
    {0x3F,0x0B,"Volume set deleted"},
    {0x3F,0x0C,"Volume set deassigned"},
    {0x3F,0x0D,"Volume set reassigned"},
    {0x3F,0x0E,"Reported luns data has changed"},
    {0x3F,0x0F,"Echo buffer overwritten"},
    {0x3F,0x10,"Medium loadable"},
    {0x3F,0x11,"Medium auxiliary memory accessible"},
    {0x3F,0x12,"iSCSI IP address added"},
    {0x3F,0x13,"iSCSI IP address removed"},
    {0x3F,0x14,"iSCSI IP address changed"},
    {0x3F,0x15,"Inspect referrals sense descriptors"},
    {0x3F,0x16,"Microcode has been changed without reset"},
    {0x3F,0x17,"Zone transition to full"},
    {0x3F,0x18,"Bind completed"},
    {0x3F,0x19,"Bind redirected"},
    {0x3F,0x1A,"Subsidiary binding changed"},

    /*
     * ASC 0x40, 0x41 and 0x42 overridden by "additional2" array entries
     * for ascq > 1. Preferred error message for this group is
     * "Diagnostic failure on component nn (80h-ffh)".
     */
    {0x40,0x00,"Ram failure (should use 40 nn)"},
    {0x41,0x00,"Data path failure (should use 40 nn)"},
    {0x42,0x00,"Power-on or self-test failure (should use 40 nn)"},

    {0x43,0x00,"Message error"},
    {0x44,0x00,"Internal target failure"},
    {0x44,0x01,"Persistent reservation information lost"},
    {0x44,0x71,"ATA device failed Set Features"},
    {0x45,0x00,"Select or reselect failure"},
    {0x46,0x00,"Unsuccessful soft reset"},
    {0x47,0x00,"SCSI parity error"},
    {0x47,0x01,"Data phase CRC error detected"},
    {0x47,0x02,"SCSI parity error detected during st data phase"},
    {0x47,0x03,"Information unit iuCRC error detected"},
    {0x47,0x04,"Asynchronous information protection error detected"},
    {0x47,0x05,"Protocol service CRC error"},
    {0x47,0x06,"Phy test function in progress"},
    {0x47,0x7F,"Some commands cleared by iSCSI protocol event"},
    {0x48,0x00,"Initiator detected error message received"},
    {0x49,0x00,"Invalid message error"},
    {0x4A,0x00,"Command phase error"},
    {0x4B,0x00,"Data phase error"},
    {0x4B,0x01,"Invalid target port transfer tag received"},
    {0x4B,0x02,"Too much write data"},
    {0x4B,0x03,"Ack/nak timeout"},
    {0x4B,0x04,"Nak received"},
    {0x4B,0x05,"Data offset error"},
    {0x4B,0x06,"Initiator response timeout"},
    {0x4B,0x07,"Connection lost"},
    {0x4B,0x08,"Data-in buffer overflow - data buffer size"},
    {0x4B,0x09,"Data-in buffer overflow - data buffer descriptor area"},
    {0x4B,0x0A,"Data-in buffer error"},
    {0x4B,0x0B,"Data-out buffer overflow - data buffer size"},
    {0x4B,0x0C,"Data-out buffer overflow - data buffer descriptor area"},
    {0x4B,0x0D,"Data-out buffer error"},
    {0x4B,0x0E,"PCIe fabric error"},
    {0x4B,0x0f,"PCIe completion timeout"},
    {0x4B,0x10,"PCIe completer abort"},
    {0x4B,0x11,"PCIe poisoned tlp received"},
    {0x4B,0x12,"PCIe ecrc check failed"},
    {0x4B,0x13,"PCIe unsupported request"},
    {0x4B,0x14,"PCIe acs violation"},
    {0x4B,0x15,"PCIe tlp prefix blocked"},
    {0x4C,0x00,"Logical unit failed self-configuration"},
    /*
     * ASC 0x4D overridden by an "additional2" array entry
     * so there is no need to have them here.
     */
    /* {0x4D,0x00,"Tagged overlapped commands (nn = queue tag)"}, */

    {0x4E,0x00,"Overlapped commands attempted"},
    {0x50,0x00,"Write append error"},
    {0x50,0x01,"Write append position error"},
    {0x50,0x02,"Position error related to timing"},
    {0x51,0x00,"Erase failure"},
    {0x51,0x01,"Erase failure - incomplete erase operation detected"},
    {0x52,0x00,"Cartridge fault"},
    {0x53,0x00,"Media load or eject failed"},
    {0x53,0x01,"Unload tape failure"},
    {0x53,0x02,"Medium removal prevented"},
    {0x53,0x03,"Medium removal prevented by data transfer element"},
    {0x53,0x04,"Medium thread or unthread failure"},
    {0x53,0x05,"Volume identifier invalid"},
    {0x53,0x06,"Volume identifier missing"},
    {0x53,0x07,"Duplicate volume identifier"},
    {0x53,0x08,"Element status unknown"},
    {0x53,0x09,"Data transfer device error - load failed"},
    {0x53,0x0A,"Data transfer device error - unload failed"},
    {0x53,0x0B,"Data transfer device error - unload missing"},
    {0x53,0x0C,"Data transfer device error - eject failed"},
    {0x53,0x0D,"Data transfer device error - library communication failed"},
    {0x54,0x00,"SCSI to host system interface failure"},
    {0x55,0x00,"System resource failure"},
    {0x55,0x01,"System buffer full"},
    {0x55,0x02,"Insufficient reservation resources"},
    {0x55,0x03,"Insufficient resources"},
    {0x55,0x04,"Insufficient registration resources"},
    {0x55,0x05,"Insufficient access control resources"},
    {0x55,0x06,"Auxiliary memory out of space"},
    {0x55,0x07,"Quota error"},
    {0x55,0x08,"Maximum number of supplemental decryption keys exceeded"},
    {0x55,0x09,"Medium auxiliary memory not accessible"},
    {0x55,0x0a,"Data currently unavailable"},
    {0x55,0x0b,"Insufficient power for operation"},
    {0x55,0x0c,"Insufficient resources to create rod"},
    {0x55,0x0d,"Insufficient resources to create rod token"},
    {0x55,0x0e,"Insufficient zone resources"},
    {0x55,0x0f,"Insufficient zone resources to complete write"},
    {0x55,0x10,"Maximum number of streams open"},
    {0x55,0x11,"Insufficient resources to bind"},
    {0x57,0x00,"Unable to recover table-of-contents"},
    {0x58,0x00,"Generation does not exist"},
    {0x59,0x00,"Updated block read"},
    {0x5A,0x00,"Operator request or state change input"},
    {0x5A,0x01,"Operator medium removal request"},
    {0x5A,0x02,"Operator selected write protect"},
    {0x5A,0x03,"Operator selected write permit"},
    {0x5B,0x00,"Log exception"},
    {0x5B,0x01,"Threshold condition met"},
    {0x5B,0x02,"Log counter at maximum"},
    {0x5B,0x03,"Log list codes exhausted"},
    {0x5C,0x00,"Rpl status change"},
    {0x5C,0x01,"Spindles synchronized"},
    {0x5C,0x02,"Spindles not synchronized"},
    {0x5D,0x00,"Failure prediction threshold exceeded"},
    {0x5D,0x01,"Media failure prediction threshold exceeded"},
    {0x5D,0x02,"Logical unit failure prediction threshold exceeded"},
    {0x5D,0x03,"spare area exhaustion prediction threshold exceeded"},
    {0x5D,0x10,"Hardware impending failure general hard drive failure"},
    {0x5D,0x11,"Hardware impending failure drive error rate too high" },
    {0x5D,0x12,"Hardware impending failure data error rate too high" },
    {0x5D,0x13,"Hardware impending failure seek error rate too high" },
    {0x5D,0x14,"Hardware impending failure too many block reassigns"},
    {0x5D,0x15,"Hardware impending failure access times too high" },
    {0x5D,0x16,"Hardware impending failure start unit times too high" },
    {0x5D,0x17,"Hardware impending failure channel parametrics"},
    {0x5D,0x18,"Hardware impending failure controller detected"},
    {0x5D,0x19,"Hardware impending failure throughput performance"},
    {0x5D,0x1A,"Hardware impending failure seek time performance"},
    {0x5D,0x1B,"Hardware impending failure spin-up retry count"},
    {0x5D,0x1C,"Hardware impending failure drive calibration retry count"},
    {0x5D,0x1D,"Hardware impending failure power loss protection circuit"},
    {0x5D,0x20,"Controller impending failure general hard drive failure"},
    {0x5D,0x21,"Controller impending failure drive error rate too high" },
    {0x5D,0x22,"Controller impending failure data error rate too high" },
    {0x5D,0x23,"Controller impending failure seek error rate too high" },
    {0x5D,0x24,"Controller impending failure too many block reassigns"},
    {0x5D,0x25,"Controller impending failure access times too high" },
    {0x5D,0x26,"Controller impending failure start unit times too high" },
    {0x5D,0x27,"Controller impending failure channel parametrics"},
    {0x5D,0x28,"Controller impending failure controller detected"},
    {0x5D,0x29,"Controller impending failure throughput performance"},
    {0x5D,0x2A,"Controller impending failure seek time performance"},
    {0x5D,0x2B,"Controller impending failure spin-up retry count"},
    {0x5D,0x2C,"Controller impending failure drive calibration retry count"},
    {0x5D,0x30,"Data channel impending failure general hard drive failure"},
    {0x5D,0x31,"Data channel impending failure drive error rate too high" },
    {0x5D,0x32,"Data channel impending failure data error rate too high" },
    {0x5D,0x33,"Data channel impending failure seek error rate too high" },
    {0x5D,0x34,"Data channel impending failure too many block reassigns"},
    {0x5D,0x35,"Data channel impending failure access times too high" },
    {0x5D,0x36,"Data channel impending failure start unit times too high" },
    {0x5D,0x37,"Data channel impending failure channel parametrics"},
    {0x5D,0x38,"Data channel impending failure controller detected"},
    {0x5D,0x39,"Data channel impending failure throughput performance"},
    {0x5D,0x3A,"Data channel impending failure seek time performance"},
    {0x5D,0x3B,"Data channel impending failure spin-up retry count"},
    {0x5D,0x3C,"Data channel impending failure drive calibration retry count"},
    {0x5D,0x40,"Servo impending failure general hard drive failure"},
    {0x5D,0x41,"Servo impending failure drive error rate too high" },
    {0x5D,0x42,"Servo impending failure data error rate too high" },
    {0x5D,0x43,"Servo impending failure seek error rate too high" },
    {0x5D,0x44,"Servo impending failure too many block reassigns"},
    {0x5D,0x45,"Servo impending failure access times too high" },
    {0x5D,0x46,"Servo impending failure start unit times too high" },
    {0x5D,0x47,"Servo impending failure channel parametrics"},
    {0x5D,0x48,"Servo impending failure controller detected"},
    {0x5D,0x49,"Servo impending failure throughput performance"},
    {0x5D,0x4A,"Servo impending failure seek time performance"},
    {0x5D,0x4B,"Servo impending failure spin-up retry count"},
    {0x5D,0x4C,"Servo impending failure drive calibration retry count"},
    {0x5D,0x50,"Spindle impending failure general hard drive failure"},
    {0x5D,0x51,"Spindle impending failure drive error rate too high" },
    {0x5D,0x52,"Spindle impending failure data error rate too high" },
    {0x5D,0x53,"Spindle impending failure seek error rate too high" },
    {0x5D,0x54,"Spindle impending failure too many block reassigns"},
    {0x5D,0x55,"Spindle impending failure access times too high" },
    {0x5D,0x56,"Spindle impending failure start unit times too high" },
    {0x5D,0x57,"Spindle impending failure channel parametrics"},
    {0x5D,0x58,"Spindle impending failure controller detected"},
    {0x5D,0x59,"Spindle impending failure throughput performance"},
    {0x5D,0x5A,"Spindle impending failure seek time performance"},
    {0x5D,0x5B,"Spindle impending failure spin-up retry count"},
    {0x5D,0x5C,"Spindle impending failure drive calibration retry count"},
    {0x5D,0x60,"Firmware impending failure general hard drive failure"},
    {0x5D,0x61,"Firmware impending failure drive error rate too high" },
    {0x5D,0x62,"Firmware impending failure data error rate too high" },
    {0x5D,0x63,"Firmware impending failure seek error rate too high" },
    {0x5D,0x64,"Firmware impending failure too many block reassigns"},
    {0x5D,0x65,"Firmware impending failure access times too high" },
    {0x5D,0x66,"Firmware impending failure start unit times too high" },
    {0x5D,0x67,"Firmware impending failure channel parametrics"},
    {0x5D,0x68,"Firmware impending failure controller detected"},
    {0x5D,0x69,"Firmware impending failure throughput performance"},
    {0x5D,0x6A,"Firmware impending failure seek time performance"},
    {0x5D,0x6B,"Firmware impending failure spin-up retry count"},
    {0x5D,0x6C,"Firmware impending failure drive calibration retry count"},
    {0x5D,0x73,"Media impending failure endurance limit met"},
    {0x5D,0xFF,"Failure prediction threshold exceeded (false)"},
    {0x5E,0x00,"Low power condition on"},
    {0x5E,0x01,"Idle condition activated by timer"},
    {0x5E,0x02,"Standby condition activated by timer"},
    {0x5E,0x03,"Idle condition activated by command"},
    {0x5E,0x04,"Standby condition activated by command"},
    {0x5E,0x05,"Idle_b condition activated by timer"},
    {0x5E,0x06,"Idle_b condition activated by command"},
    {0x5E,0x07,"Idle_c condition activated by timer"},
    {0x5E,0x08,"Idle_c condition activated by command"},
    {0x5E,0x09,"Standby_y condition activated by timer"},
    {0x5E,0x0a,"Standby_y condition activated by command"},
    {0x5E,0x41,"Power state change to active"},
    {0x5E,0x42,"Power state change to idle"},
    {0x5E,0x43,"Power state change to standby"},
    {0x5E,0x45,"Power state change to sleep"},
    {0x5E,0x47,"Power state change to device control"},
    {0x60,0x00,"Lamp failure"},
    {0x61,0x00,"Video acquisition error"},
    {0x61,0x01,"Unable to acquire video"},
    {0x61,0x02,"Out of focus"},
    {0x62,0x00,"Scan head positioning error"},
    {0x63,0x00,"End of user area encountered on this track"},
    {0x63,0x01,"Packet does not fit in available space"},
    {0x64,0x00,"Illegal mode for this track"},
    {0x64,0x01,"Invalid packet size"},
    {0x65,0x00,"Voltage fault"},
    {0x66,0x00,"Automatic document feeder cover up"},
    {0x66,0x01,"Automatic document feeder lift up"},
    {0x66,0x02,"Document jam in automatic document feeder"},
    {0x66,0x03,"Document miss feed automatic in document feeder"},
    {0x67,0x00,"Configuration failure"},
    {0x67,0x01,"Configuration of incapable logical units failed"},
    {0x67,0x02,"Add logical unit failed"},
    {0x67,0x03,"Modification of logical unit failed"},
    {0x67,0x04,"Exchange of logical unit failed"},
    {0x67,0x05,"Remove of logical unit failed"},
    {0x67,0x06,"Attachment of logical unit failed"},
    {0x67,0x07,"Creation of logical unit failed"},
    {0x67,0x08,"Assign failure occurred"},
    {0x67,0x09,"Multiply assigned logical unit"},
    {0x67,0x0A,"Set target port groups command failed"},
    {0x67,0x0B,"ATA device feature not enabled"},
    {0x67,0x0C,"Command rejected"},
    {0x67,0x0D,"Explicit bind not allowed"},
    {0x68,0x00,"Logical unit not configured"},
    {0x68,0x01,"Subsidiary logical unit not configured"},
    {0x69,0x00,"Data loss on logical unit"},
    {0x69,0x01,"Multiple logical unit failures"},
    {0x69,0x02,"Parity/data mismatch"},
    {0x6A,0x00,"Informational, refer to log"},
    {0x6B,0x00,"State change has occurred"},
    {0x6B,0x01,"Redundancy level got better"},
    {0x6B,0x02,"Redundancy level got worse"},
    {0x6C,0x00,"Rebuild failure occurred"},
    {0x6D,0x00,"Recalculate failure occurred"},
    {0x6E,0x00,"Command to logical unit failed"},
    {0x6F,0x00,"Copy protection key exchange failure - authentication "
               "failure"},
    {0x6F,0x01,"Copy protection key exchange failure - key not present"},
    {0x6F,0x02,"Copy protection key exchange failure - key not established"},
    {0x6F,0x03,"Read of scrambled sector without authentication"},
    {0x6F,0x04,"Media region code is mismatched to logical unit region"},
    {0x6F,0x05,"Drive region must be permanent/region reset count error"},
    {0x6F,0x06,"Insufficient block count for binding nonce recording"},
    {0x6F,0x07,"Conflict in binding nonce recording"},
    {0x6F,0x08,"Insufficient permission"},
    {0x6F,0x09,"Invalid drive-host pairing server"},
    {0x6F,0x0A,"Drive-host pairing suspended"},
    /*
     * ASC 0x70 overridden by an "additional2" array entry
     * so there is no need to have them here.
     */
    /* {0x70,0x00,"Decompression exception short algorithm id of nn"}, */

    {0x71,0x00,"Decompression exception long algorithm id"},
    {0x72,0x00,"Session fixation error"},
    {0x72,0x01,"Session fixation error writing lead-in"},
    {0x72,0x02,"Session fixation error writing lead-out"},
    {0x72,0x03,"Session fixation error - incomplete track in session"},
    {0x72,0x04,"Empty or partially written reserved track"},
    {0x72,0x05,"No more track reservations allowed"},
    {0x72,0x06,"RMZ extension is not allowed"},
    {0x72,0x07,"No more test zone extensions are allowed"},
    {0x73,0x00,"CD control error"},
    {0x73,0x01,"Power calibration area almost full"},
    {0x73,0x02,"Power calibration area is full"},
    {0x73,0x03,"Power calibration area error"},
    {0x73,0x04,"Program memory area update failure"},
    {0x73,0x05,"Program memory area is full"},
    {0x73,0x06,"RMA/PMA is almost full"},
    {0x73,0x10,"Current power calibration area almost full"},
    {0x73,0x11,"Current power calibration area is full"},
    {0x73,0x17,"RDZ is full"},
    {0x74,0x00,"Security error"},
    {0x74,0x01,"Unable to decrypt data"},
    {0x74,0x02,"Unencrypted data encountered while decrypting"},
    {0x74,0x03,"Incorrect data encryption key"},
    {0x74,0x04,"Cryptographic integrity validation failed"},
    {0x74,0x05,"Error decrypting data"},
    {0x74,0x06,"Unknown signature verification key"},
    {0x74,0x07,"Encryption parameters not useable"},
    {0x74,0x08,"Digital signature validation failure"},
    {0x74,0x09,"Encryption mode mismatch on read"},
    {0x74,0x0a,"Encrypted block not raw read enabled"},
    {0x74,0x0b,"Incorrect Encryption parameters"},
    {0x74,0x0c,"Unable to decrypt parameter list"},
    {0x74,0x0d,"Encryption algorithm disabled"},
    {0x74,0x10,"SA creation parameter value invalid"},
    {0x74,0x11,"SA creation parameter value rejected"},
    {0x74,0x12,"Invalid SA usage"},
    {0x74,0x21,"Data encryption configuration prevented"},
    {0x74,0x30,"SA creation parameter not supported"},
    {0x74,0x40,"Authentication failed"},
    {0x74,0x61,"External data encryption key manager access error"},
    {0x74,0x62,"External data encryption key manager error"},
    {0x74,0x63,"External data encryption key not found"},
    {0x74,0x64,"External data encryption request not authorized"},
    {0x74,0x6e,"External data encryption control timeout"},
    {0x74,0x6f,"External data encryption control error"},
    {0x74,0x71,"Logical unit access not authorized"},
    {0x74,0x79,"Security conflict in translated device"},
    {0, 0, NULL}
};

#else   /* SG_SCSI_STRINGS */

struct sg_lib_asc_ascq_range_t sg_lib_asc_ascq_range[] =
{
    {0, 0, 0, NULL}
};

struct sg_lib_asc_ascq_t sg_lib_asc_ascq[] =
{
    {0, 0, NULL}
};
#endif /* SG_SCSI_STRINGS */

const char * sg_lib_sense_key_desc[] = {
    "No Sense",                 /* Filemark, ILI and/or EOM; progress
                                   indication (during FORMAT); power
                                   condition sensing (REQUEST SENSE) */
    "Recovered Error",          /* The last command completed successfully
                                   but used error correction */
    "Not Ready",                /* The addressed target is not ready */
    "Medium Error",             /* Data error detected on the medium */
    "Hardware Error",           /* Controller or device failure */
    "Illegal Request",
    "Unit Attention",           /* Removable medium was changed, or
                                   the target has been reset */
    "Data Protect",             /* Access to the data is blocked */
    "Blank Check",              /* Reached unexpected written or unwritten
                                   region of the medium */
    "Vendor specific(9)",       /* Vendor specific */
    "Copy Aborted",             /* COPY or COMPARE was aborted */
    "Aborted Command",          /* The target aborted the command */
    "Equal",                    /* SEARCH DATA found data equal (obsolete) */
    "Volume Overflow",          /* Medium full with data to be written */
    "Miscompare",               /* Source data and data on the medium
                                   do not agree */
    "Completed"                 /* may occur for successful cmd (spc4r23) */
};

const char * sg_lib_pdt_strs[32] = {    /* should have 2**5 elements */
    /* 0 */ "disk",
    "tape",
    "printer",                  /* obsolete, spc5r01 */
    "processor",        /* often SAF-TE device, copy manager */
    "write once optical disk",  /* obsolete, spc5r01 */
    /* 5 */ "cd/dvd",
    "scanner",                  /* obsolete */
    "optical memory device",
    "medium changer",
    "communications",           /* obsolete */
    /* 0xa */ "graphics [0xa]", /* obsolete */
    "graphics [0xb]",           /* obsolete */
    "storage array controller",
    "enclosure services device",
    "simplified direct access device",
    "optical card reader/writer device",
    /* 0x10 */ "bridge controller commands",
    "object based storage",
    "automation/driver interface",
    "security manager device",  /* obsolete, spc5r01 */
    "host managed zoned block",
    "0x15", "0x16", "0x17", "0x18",
    "0x19", "0x1a", "0x1b", "0x1c", "0x1d",
    "well known logical unit",
    "unknown or no device type", /* coupled with PQ=3 for not accessible
                                    via this lu's port (try the other) */
};

const char * sg_lib_transport_proto_strs[] =
{
    "Fibre Channel Protocol for SCSI (FCP-5)",  /* now at fcp5r01 */
    "SCSI Parallel Interface (SPI-5)",  /* obsolete in spc5r01 */
    "Serial Storage Architecture SCSI-3 Protocol (SSA-S3P)",
    "Serial Bus Protocol for IEEE 1394 (SBP-3)",
    "SCSI RDMA Protocol (SRP)",
    "Internet SCSI (iSCSI)",
    "Serial Attached SCSI Protocol (SPL-4)",
    "Automation/Drive Interface Transport (ADT-2)",
    "AT Attachment Interface (ACS-2)",          /* 0x8 */
    "USB Attached SCSI (UAS-2)",
    "SCSI over PCI Express (SOP)",
    "PCIe",                             /* added in spc5r02 */
    "Oxc", "Oxd", "Oxe",
    "No specific protocol"
};

/* SCSI Feature Sets array. code->value, pdt->peri_dev_type (-1 for SPC) */
struct sg_lib_value_name_t sg_lib_scsi_feature_sets[] =
{
    {SCSI_FS_SPC_DISCOVERY_2016, -1, "Discovery 2016"},
    {SCSI_FS_SBC_BASE_2010, PDT_DISK, "SBC Base 2010"},
    {SCSI_FS_SBC_BASE_2016, PDT_DISK, "SBC Base 2016"},
    {SCSI_FS_SBC_BASIC_PROV_2016, PDT_DISK, "Basic provisioning 2016"},
    {SCSI_FS_SBC_DRIVE_MAINT_2016, PDT_DISK, "Drive maintenance 2016"},
    {SCSI_FS_ZBC_HOST_AWARE_2020, PDT_ZBC, "Host Aware 2020"},
    {SCSI_FS_ZBC_HOST_MANAGED_2020, PDT_ZBC, "Host Managed 2020"},
    {SCSI_FS_ZBC_DOMAINS_REALMS_2020, PDT_ZBC, "Domains and Realms 2020"},
    {0x0, 0, NULL},     /* 0x0 is reserved sfs; trailing sentinel */
};

#if (SG_SCSI_STRINGS && HAVE_NVME && (! IGNORE_NVME))

/* Commands sent to the NVMe Admin Queue (queue id 0) have the following
 * names in the NVM Express 1.3a document dated 20171024 */
struct sg_lib_simple_value_name_t sg_lib_nvme_admin_cmd_arr[] =
{
    {0x0,  "Delete I/O Submission Queue"},      /* first mandatory command */
    {0x1,  "Create I/O Submission Queue"},
    {0x2,  "Get Log Page"},
    {0x4,  "Delete I/O Completion Queue"},
    {0x5,  "Create I/O Completion Queue"},
    {0x6,  "Identify"},
    {0x8,  "Abort"},
    {0x9,  "Set Features"},
    {0xa,  "Get Features"},
    {0xc,  "Asynchronous Event Request"},       /* last mandatory command */
    {0xd,  "Namespace Management"},             /* first optional command */
    {0x10, "Firmware commit"},
    {0x11, "Firmware image download"},
    {0x14, "Device Self-test"},
    {0x15, "Namespace Attachment"},
    {0x18, "Keep Alive"},
    {0x19, "Directive Send"},
    {0x1a, "Directive Receive"},
    {0x1c, "Virtualization Management"},
    {0x1d, "NVMe-MI Send"},    /* SES SEND DIAGNOSTIC cmd passes thru here */
    {0x1e, "NVMe-MI Receive"}, /* RECEIVE DIAGNOSTIC RESULTS thru here */
    {0x7c, "Doorbell Buffer Config"},
    {0x7f, "NVMe over Fabrics"},

    /* I/O command set specific 0x80 to 0xbf */
    {0x80, "Format NVM"},               /* first NVM specific */
    {0x81, "Security Send"},
    {0x82, "Security Receive"},
    {0x84, "Sanitize"},                 /* last NVM specific in 1.3a */
    {0x86, "Get LBA status"},           /* NVM specific, new in 1.4 */
    /* Vendor specific 0xc0 to 0xff */
    {0xffff, NULL},                     /* Sentinel */
};

/* Commands sent any NVMe non-Admin Queue (queue id >0) for the NVM command
 * set have the following names in the NVM Express 1.3a document dated
 * 20171024 */
struct sg_lib_simple_value_name_t sg_lib_nvme_nvm_cmd_arr[] =
{
    {0x0,  "Flush"},                    /* first mandatory command */
    {0x1,  "Write"},
    {0x2,  "Read"},                     /* last mandatory command */
    {0x4,  "Write Uncorrectable"},      /* first optional command */
    {0x5,  "Compare"},
    {0x8,  "Write Zeroes"},
    {0x9,  "Dataset Management"},
    {0xd,  "Reservation Register"},
    {0xe,  "Reservation Report"},
    {0x11, "Reservation Acquire"},
    {0x15, "Reservation Release"},      /* last optional command in 1.3a */

    /* Vendor specific 0x80 to 0xff */
    {0xffff, NULL},                     /* Sentinel */
};


/* .value is completion queue's DW3 as follows: ((DW3 >> 17) & 0x3ff)
 * .peri_dev_type is an index for the sg_lib_scsi_status_sense_arr[]
 * .name is taken from NVMe 1.3a document, section 4.6.1.2.1 with less
 *   capitalization.
 * NVMe term bits 31:17 of DW3 in the completion field as the "Status
 * Field" (SF). Bit 31 is "Do not retry" (DNR) and bit 30 is "More" (M).
 * Bits 29:28 are reserved, bit 27:25 are the "Status Code Type" (SCT)
 * and bits 24:17 are the Status Code (SC). This table is in ascending
 * order of its .value field so a binary search could be done on it.  */
struct sg_lib_value_name_t sg_lib_nvme_cmd_status_arr[] =
{
    /* Generic command status values, Status Code Type (SCT): 0h
     * Lowest 8 bits are the Status Code (SC), in this case:
     *   00h - 7Fh: Applicable to Admin Command Set, or across multiple
     *              command sets
     *   80h - BFh: I/O Command Set Specific status codes
     *   c0h - FFh: I/O Vendor Specific status codes            */
    {0x0,   0, "Successful completion"},
    {0x1,   1, "Invalid command opcode"},
    {0x2,   2, "Invalid field in command"},
    {0x3,   2, "Command id conflict"},
    {0x4,   3, "Data transfer error"},
    {0x5,   4, "Command aborted due to power loss notication"},
    {0x6,   5, "Internal error"},
    {0x7,   6, "Command abort requested"},
    {0x8,   6, "Command aborted due to SQ deletion"},
    {0x9,   6, "Command aborted due to failed fused command"},
    {0xa,   6, "Command aborted due to missing fused command"},
    {0xb,   7, "Invalid namespace or format"},
    {0xc,   5, "Command sequence error"},
    {0xd,   5, "Invalid SGL segment descriptor"},
    {0xe,   5, "Invalid number of SGL descriptors"},
    {0xf,   5, "Data SGL length invalid"},
    {0x10,  5, "Matadata SGL length invalid"},
    {0x11,  5, "SGL descriptor type invalid"},
    {0x12,  5, "Invalid use of controller memory buffer"},
    {0x13,  5, "PRP offset invalid"},
    {0x14,  2, "Atomic write unit exceeded"},
    {0x15,  8, "Operation denied"},
    {0x16,  5, "SGL offset invalid"},
    {0x17,  5, "Reserved [0x17]"},
    {0x18,  5, "Host identifier inconsistent format"},
    {0x19,  5, "Keep alive timeout expired"},
    {0x1a,  5, "Keep alive timeout invalid"},
    {0x1b,  6, "Command aborted due to Preempt and Abort"},
    {0x1c, 10, "Sanitize failed"},
    {0x1d, 11, "Sanitize in progress"},
    {0x1e,  5, "SGL data block granularity invalid"},
    {0x1f,  5, "Command not supported for queue in CMB"},
    {0x20,  18, "Namespace is write protected"},        /* NVMe 1.4 */
    {0x21,  6, "Command interrupted"},                  /* NVMe 1.4 */
    {0x22,  5, "Transient transport error"},            /* NVMe 1.4 */

    /* 0x80 - 0xbf: I/O command set specific */
    /* Generic command status values, NVM (I/O) Command Set */
    {0x80, 12, "LBA out of range"},
    {0x81,  3, "Capacity exceeded"},
    {0x82, 13, "Namespace not ready"},
    {0x83, 14, "Reservation conflict"},
    {0x84, 15, "Format in progress"},
    /* 0xc0 - 0xff: vendor specific */

    /* Command specific status values, Status Code Type (SCT): 1h */
    {0x100, 5, "Completion queue invalid"},
    {0x101, 5, "Invalid queue identifier"},
    {0x102, 5, "Invalid queue size"},
    {0x103, 5, "Abort command limit exceeded"},
    {0x104, 5, "Reserved [0x104]"},
    {0x105, 5, "Asynchronous event request limit exceeded"},
    {0x106, 5, "Invalid firmware slot"},
    {0x107, 5, "Invalid firmware image"},
    {0x108, 5, "Invalid interrupt vector"},
    {0x109, 5, "Invalid log page"},
    {0x10a,16, "Invalid format"},
    {0x10b, 5, "Firmware activation requires conventional reset"},
    {0x10c, 5, "Invalid queue deletion"},
    {0x10d, 5, "Feature identifier not saveable"},
    {0x10e, 5, "Feature not changeable"},
    {0x10f, 5, "Feature not namespace specific"},
    {0x110, 5, "Firmware activation requires NVM subsystem reset"},
    {0x111, 5, "Firmware activation requires reset"},
    {0x112, 5, "Firmware activation requires maximum time violation"},
    {0x113, 5, "Firmware activation prohibited"},
    {0x114, 5, "Overlapping range"},
    {0x115, 5, "Namespace insufficient capacity"},
    {0x116, 5, "Namespace identifier unavailable"},
    {0x117, 5, "Reserved [0x107]"},
    {0x118, 5, "Namespace already attached"},
    {0x119, 5, "Namespace is private"},
    {0x11a, 5, "Namespace not attached"},
    {0x11b, 3, "Thin provisioning not supported"},
    {0x11c, 3, "Controller list invalid"},
    {0x11d,17, "Device self-test in progress"},
    {0x11e,18, "Boot partition write prohibited"},
    {0x11f, 5, "Invalid controller identifier"},
    {0x120, 5, "Invalid secondary controller state"},
    {0x121, 5, "Invalid number of controller resources"},
    {0x122, 5, "Invalid resource identifier"},
    {0x123, 5, "Sanitize prohibited while PM enabled"},         /* NVMe 1.4 */
    {0x124, 5, "ANA group identifier invalid"},                 /* NVMe 1.4 */
    {0x125, 5, "ANA attach failed"},                            /* NVMe 1.4 */

    /* Command specific status values, Status Code Type (SCT): 1h
     * for NVM (I/O) Command Set */
    {0x180, 2, "Conflicting attributes"},
    {0x181,19, "Invalid protection information"},
    {0x182,18, "Attempted write to read only range"},
    /* 0x1c0 - 0x1ff: vendor specific */

    /* Media and Data Integrity error values, Status Code Type (SCT): 2h */
    {0x280,20, "Write fault"},
    {0x281,21, "Unrecovered read error"},
    {0x282,22, "End-to-end guard check error"},
    {0x283,23, "End-to-end application tag check error"},
    {0x284,24, "End-to-end reference tag check error"},
    {0x285,25, "Compare failure"},
    {0x286, 8, "Access denied"},
    {0x287,26, "Deallocated or unwritten logical block"},
    /* 0x2c0 - 0x2ff: vendor specific */

    /* Leave this Sentinel value at end of this array */
    {0x3ff, 0, NULL},
};

/* The sg_lib_nvme_cmd_status_arr[n].peri_dev_type field is an index
 * to this array. It allows an NVMe status (error) value to be mapped
 * to this SCSI tuple: status, sense_key, additional sense code (asc) and
 * asc qualifier (ascq). For brevity SAM_STAT_CHECK_CONDITION is written
 * as 0x2. */
struct sg_lib_4tuple_u8 sg_lib_scsi_status_sense_arr[] =
{
    {SAM_STAT_GOOD, SPC_SK_NO_SENSE, 0, 0},     /* it's all good */ /* 0 */
    {SAM_STAT_CHECK_CONDITION, SPC_SK_ILLEGAL_REQUEST, 0x20, 0x0},/* opcode */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x24, 0x0},   /* field in cdb */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x0, 0x0},
    {SAM_STAT_TASK_ABORTED, SPC_SK_ABORTED_COMMAND, 0xb, 0x8},
    {0x2, SPC_SK_HARDWARE_ERROR, 0x44, 0x0},   /* internal error */ /* 5 */
    {SAM_STAT_TASK_ABORTED, SPC_SK_ABORTED_COMMAND, 0x0, 0x0},
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x20, 0x9},   /* invalid LU */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x20, 0x2},   /* access denied */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x2c, 0x0},   /* cmd sequence error */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x31, 0x3},   /* sanitize failed */ /* 10 */
    {0x2, SPC_SK_NOT_READY, 0x4, 0x1b}, /* sanitize in progress */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x21, 0x0},   /* LBA out of range */
    {0x2, SPC_SK_NOT_READY, 0x4, 0x0},  /* not reportable; 0x1: becoming */
    {SAM_STAT_RESERVATION_CONFLICT, 0x0, 0x0, 0x0},
    {0x2, SPC_SK_NOT_READY, 0x4, 0x4},  /* format in progress */  /* 15 */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x31, 0x1},  /* format failed */
    {0x2, SPC_SK_NOT_READY, 0x4, 0x9},  /* self-test in progress */
    {0x2, SPC_SK_DATA_PROTECT, 0x27, 0x0},      /* write prohibited */
    {0x2, SPC_SK_ILLEGAL_REQUEST, 0x10, 0x5},  /* protection info */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x3, 0x0}, /* periph dev w fault */ /* 20 */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x11, 0x0},      /* unrecoc rd */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x10, 0x1},      /* PI guard */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x10, 0x2},      /* PI app tag */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x10, 0x2},      /* PI app tag */
    {0x2, SPC_SK_MISCOMPARE, 0x1d, 0x0},        /* during verify */ /* 25 */
    {0x2, SPC_SK_MEDIUM_ERROR, 0x21, 0x6},      /* read invalid data */

    /* Leave this Sentinel value at end of this array */
    {0xff, 0xff, 0xff, 0xff},
};

/* These are the error (or warning) exit status values and their associated
 * strings. They combine utility input syntax errors, SCSI status and sense
 * key categories, OS errors (e.g. ENODEV for device not found), one that
 * indicates NVMe non-zero status plus listing those that a Unix OS generates
 * for any executable (that fails). The convention is 0 means no error and
 * that in Unix the exit status is an (unsigned) 8 bit value. */
struct sg_value_2names_t sg_exit_str_arr[] = {
    {0,  "No errors", "may also convey true"},
    {1,  "Syntax error", "command line options (usually)"},
    {2,  "Device not ready", "type: sense key"},
    {3,  "Medium or hardware error", "type: sense key (plus blank check for "
         "tape)"},
    {5,  "Illegal request", "type: sense key, apart from Invalid opcode"},
    {6,  "Unit attention", "type: sense key"},
    {7,  "Data protect", "type: sense key; write protected media?"},
    {9,  "Illegal request, Invalid opcode", "type: sense key + asc,ascq"},
    {10, "Copy aborted", "type: sense key"},
    {11, "Aborted command",
         "type: sense key, other than protection related (asc=0x10)"},
    {14, "Miscompare", "type: sense key"},
    {15, "File error", NULL},
    {17, "Illegal request with Info field", NULL},
    {18, "Medium or hardware error with Info", NULL},
    {20, "No sense key", "type: probably additional sense code"},
    {21, "Recovered error (warning)", "tye: sense key"},
         /* N.B. this is a warning not error */
    {22, "LBA out of range", NULL},
    {24, "Reservation conflict", "type: SCSI status"},
    {25, "Condition met", "type: SCSI status"}, /* from PRE-FETCH command */
    {26, "Busy", "type: SCSI status"},   /* could be transport issue */
    {27, "Task set full", "type: SCSI status"},
    {28, "ACA aactive", "type: SCSI status"},
    {29, "Task aborted", "type: SCSI status"},
    {31, "Contradict", "command line options contradict or select bad mode"},
    {32, "Logic error", "unexpected situation, contact author"},
    {33, "SCSI command timeout", NULL},         /* OS timed out command */
    {36, "No errors (false)", NULL},
    {40, "Aborted command, protection error", NULL},
    {41, "Aborted command, protection error with Info field", NULL},
    {47, "flock (Unix system call) error", NULL},       /* ddpt */
    {48, "NVMe command with non-zero status", NULL},
    {50, "An OS error occurred", "(errno > 46 or negative)"},
    /* OS errors (errno in Unix) from 1 to 46 mapped into this range */
    {97, "Malformed SCSI command", "trouble building command"},
    {98, "Some other sense error", "try '-v' option for more information"},
    {99, "Some other error", "possible transport of driver issue"},
    {100, "Parameter list length error", NULL}, /* these for ddpt, xcopy */
    {101, "Invalid field in parameter", NULL},
    {102, "Too many segments in parameters", NULL},
    {103, "Target underrun", NULL},
    {104, "Target overrun", NULL},
    {105, "Operation in progress", NULL},
    {106, "Insufficient resources to create ROD", NULL},
    {107, "Insufficient resources to create ROD token", NULL},
    {108, "Commands cleared by device server", NULL},
    {109, "See leave_reason for error", NULL},        /* internal error */
    /* DDPT_CAT_TOKOP_BASE: asc=0x23, ascq=110 follow */
    {110, "Invalid token operation, cause not reportable", NULL},
    {111, "Invalid token operation, unsupported token type", NULL},
    {112, "Invalid token operation, remote token usage not supported", NULL},
    {113, "Invalid token operation, remote token creation not supported",
          NULL},
    {114, "Invalid token operation, token unknown", NULL},
    {115, "Invalid token operation, token corrupt", NULL},
    {116, "Invalid token operation, token revoked", NULL},
    {117, "Invalid token operation, token expired", NULL},
    {118, "Invalid token operation, token cancelled", NULL},
    {119, "Invalid token operation, token deleted", NULL},
    {120, "Invalid token operation, invalid token length", NULL},

    /* The following error codes are generated by a Unix OS */
    {126, "Utility found but did not have execute permissions", NULL},
    {127, "Utility to be executed was not found", NULL},
    {128, "Utility stopped/aborted by signal number: 0", "signal # 0 ??"},
    /* 128 + <signal_number>: signal number that aborted the utility.
                              real time signals start at offset SIGRTMIN */
    /* OS signals from 1 to 126 mapped into this range (129 to 254) */
    {255, "Utility returned 255 or higher", "Windows error number?"},
    {0xffff, NULL, NULL},       /* end marking sentinel */
};

#else           /* (SG_SCSI_STRINGS && HAVE_NVME && (! IGNORE_NVME)) */

struct sg_lib_simple_value_name_t sg_lib_nvme_admin_cmd_arr[] =
{

    /* Vendor specific 0x80 to 0xff */
    {0xffff, NULL},                     /* Sentinel */
};

struct sg_lib_simple_value_name_t sg_lib_nvme_nvm_cmd_arr[] =
{

    /* Vendor specific 0x80 to 0xff */
    {0xffff, NULL},                     /* Sentinel */
};

struct sg_lib_value_name_t sg_lib_nvme_cmd_status_arr[] =
{

    /* Leave this Sentinel value at end of this array */
    {0x3ff, 0, NULL},
};

struct sg_lib_4tuple_u8 sg_lib_scsi_status_sense_arr[] =
{

    /* Leave this Sentinel value at end of this array */
    {0xff, 0xff, 0xff, 0xff},
};

struct sg_value_2names_t sg_exit_str_arr[] = {
    {0xffff, NULL, NULL},       /* end marking sentinel */
};

#endif           /* (SG_SCSI_STRINGS && HAVE_NVME && (! IGNORE_NVME)) */
