/*
 * Copyright (c) 2005-2021, Douglas Gilbert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sdparm.h"


#define PROTO_IDENT_STR "0: fcp; 1: spi; 4: srp; 5: iscsi; 6: sas/spl; " \
        "7: adt;\t8: ata/acs; 9: uas; 10: sop"

/*
 * sdparm is a utility program to access and change SCSI device
 * (logical unit) mode page fields and do some other housekeeping.
 *
 * This file contains data tables that may be useful for other
 * programs. The data in these tables is derived from various (draft)
 * documents found at http://www.t10.org
 */


/* SSC's medium partition mode page has a variable number of
   partition size fields which are treated as descriptors here */
static struct sdparm_mode_descriptor_t ssc_mpa_desc = {
    3, 1, 1, 8, 2, -1, -1, false, "SSC medium partition"
};

/* SMC's transport geometry parameters mode page doesn't give the number
   of following descriptors but rather parameter length (in bytes).
   This is flagged by -1 in num_descs_inc (third) field */
static struct sdparm_mode_descriptor_t smc_tg_desc = {
    1, 1, -1, 2, 2, -1, -1, false, "SMC transport geometry"
};

/* SBC's logical block provisioning mode page doesn't give the number
   of following descriptors but rather parameter length (in bytes).
   This is flagged by -1 in num_descs_inc (third) field */
static struct sdparm_mode_descriptor_t sbc_lbp_desc = {
    2, 2, -1, 16, 8, -1, -1, false, "SBC logical block provisioning"
};

/* SBC's application tag mode page doesn't give the number of
   following descriptors but rather parameter length (in bytes).
   This is flagged by -1 in num_descs_inc (third) field */
static struct sdparm_mode_descriptor_t sbc_atag_desc = {
    2, 2, -1, 16, 24, -1, -1, false, "SBC application tag"
};

/* SPC's command duration limit A/B mode pages don't give the number of
   following descriptors but rather parameter length (in bytes).
   This is flagged by -1 in num_descs_inc (third) field */
static struct sdparm_mode_descriptor_t spc_cdl_desc = {
    2, 2, -1, 8, 4, -1, -1, false, "command duration limit"
};

/* SBC's IO advice hints grouping mode page doesn't give the number of
   following descriptors but rather parameter length (in bytes).
   This is flagged by -1 in num_descs_inc (third) field */
static struct sdparm_mode_descriptor_t sbc_ioadvi_desc = {
    2, 2, -1, 16, 16, -1, -1, false, "IO advice hints group"
};

/* Mode pages that aren't specific to any transport protocol or vendor.
   Note that all standard peripheral device types are included in this array.
   The pages are listed in acronym alphabetical order. */
struct sdparm_mode_page_t sdparm_gen_mode_pg[] = {
    {ADC_MP, MSP_ADC_DT_DPP, PDT_ADC, 0, "addp",
        "DT device primary port (ADC)", NULL},
    {ADC_MP, MSP_ADC_LU, PDT_ADC, 0, "adlu", "logical unit (ADC)", NULL},
    {ADC_MP, MSP_ADC_TGT_DEV, PDT_ADC, 0, "adtd", "Target device (ADC)",
        NULL},
    {ADC_MP, MSP_ADC_TD_SN, PDT_ADC, 0, "adts",
        "Target device serial number (ADC)", NULL},
    {CONTROL_MP, MSP_SAT_AFC, -1, 0, "afc", "SAT ATA Feature control", NULL},
    {POWER_MP, MSP_SAT_POWER, -1, 0, "apo", "SAT ATA Power condition", NULL},
    {CONTROL_MP, MSP_SBC_APP_TAG, PDT_DISK_ZBC, 0, "atag", "Application tag "
        "(SBC)", &sbc_atag_desc},
    {IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 0, "bc", "Background control (SBC)",
        NULL},
    {CONTROL_MP, MSP_SBC_BACK_OP, PDT_DISK, 0, "bop",
        "Background operation control (SBC)", NULL},
    {CACHING_MP, 0, PDT_DISK_ZBC, 0, "ca", "Caching (SBC)", NULL},
    {CONTROL_MP, MSP_SPC_CDLA, -1, 0, "cdla", "Command duration limit A",
        &spc_cdl_desc},
    {CONTROL_MP, MSP_SPC_CDLB, -1, 0, "cdlb", "Command duration limit B",
        &spc_cdl_desc},
    {CONTROL_MP, MSP_SPC_CDLT2A, -1, 0, "cdt2a", "Command duration limit T2A",
        NULL /* spc6r01 */ },
    {CONTROL_MP, MSP_SPC_CDLT2B, -1, 0, "cdt2b", "Command duration limit T2B",
        NULL /* spc6r01 */ },
    {MMCMS_MP, 0, PDT_MMC, 1, "cms", "CD/DVD (MM) capabilities and "
        "mechanical status (MMC)", NULL},        /* read only */
    {CONTROL_MP, 0, -1, 0, "co", "Control", NULL},
    {CONTROL_MP, MSP_SPC_CE, -1, 0, "coe", "Control extension", NULL},
    {CONTROL_MP, MSP_SSC_CDP, -1, 0, "cdp", "Control data protection (SSC)",
        NULL},
    {DATA_COMPR_MP, 0, PDT_TAPE, 0, "dac", "Data compression (SSC)", NULL},
    {DEV_CONF_MP, 0, PDT_TAPE, 0, "dc", "Device configuration (SSC)", NULL},
    {DEV_CAP_MP, 0, PDT_MCHANGER, 0, "dca", "Device capabilities (SMC)",
        NULL},
    {DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 0, "dce",
        "Device configuration extension (SSC)", NULL},
    {DISCONNECT_MP, 0, -1, 0, "dr",
        "Disconnect-reconnect (SPC + transports)", NULL},
    {ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 0, "eaa",
        "Element address assignment (SMC)", NULL},
    {DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 0, "edc",
        "Extended device capabilities (SMC)", NULL},
    {ES_MAN_MP, 0, PDT_SES, 0, "esm", "Enclosure services management (SES)",
        NULL},
    {FLEX_DISK_MP, 0, PDT_DISK, 0, "fd", "Flexible disk (SBC)", NULL},
    {FORMAT_MP, 0, PDT_DISK, 0, "fo", "Format (SBC)", NULL},
    {IEC_MP, 0, -1, 0, "ie", "Informational exceptions control", NULL},
    {CONTROL_MP, MSP_SBC_IO_ADVI, 0, 0, "ioad", "IO advice hints grouping",
        &sbc_ioadvi_desc},
    {IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 0, "lbp", "Logical block "
        "provisioning (SBC)", &sbc_lbp_desc},
    {MED_CONF_MP, 0, PDT_TAPE, 0, "mco", "Medium configuration (SSC)", NULL},
    {MED_PART_MP, 0, PDT_TAPE, 0, "mpa", "Medium partition (SSC)",
        &ssc_mpa_desc},
    {MRW_MP, 0, PDT_MMC, 0, "mrw", "Mount rainier reWritable (MMC)", NULL},
    {NOTCH_MP, 0, PDT_DISK, 0, "not", "Notch and partition (SBC)", NULL},
    {CONTROL_MP, MSP_SAT_PATA, -1, 0, "pat", "SAT pATA control", NULL},
    {PROT_SPEC_LU_MP, 0, -1, 0, "pl", "Protocol specific logical unit", NULL},
    {POWER_MP, 0, -1, 0, "po", "Power condition", NULL},
    {POWER_MP, MSP_SPC_PS, -1, 0, "ps", "Power consumption", NULL},
    {POWER_OLD_MP, 0, PDT_DISK, 0, "poo", "Power condition - old version",
        NULL}, /* POWER_OLD_MP for disks as it clashes with old MMC specs */
    {PROT_SPEC_PORT_MP, 0, -1, 0, "pp", "Protocol specific port", NULL},
    {RBC_DEV_PARAM_MP, 0, PDT_RBC, 0, "rbc", "RBC device parameters (RBC)",
        NULL},
    {RIGID_DISK_MP, 0, PDT_DISK, 0, "rd", "Rigid disk (SBC)", NULL},
    {RW_ERR_RECOVERY_MP, 0, -1, 0, "rw", "Read write error recovery", NULL},
        /* since in SBC, SSC and MMC treat RW_ERR_RECOVERY_MP as if in SPC */
    {TRANS_GEO_PAR_MP, 0, PDT_MCHANGER, 0, "tgp", "Transport geometry "
        "parameters (SMC)", &smc_tg_desc},
    {TIMEOUT_PROT_MP, 0, PDT_MMC, 0, "tp", "Timeout and protect (MMC)", NULL},
    {V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 0, "ve",
        "Verify error recovery (SBC)", NULL},
    {WRITE_PARAM_MP, 0, PDT_MMC, 0, "wp", "Write parameters (MMC)", NULL},
    {XOR_MP, 0, PDT_DISK, 0, "xo", "XOR control (SBC)", NULL},
        /* XOR control mode page made obsolete in sbc3r32 */
    {CONTROL_MP, MSP_ZB_D_CTL, PDT_DISK_ZBC, 0, "zbdct",
        "Zoned block device control (ZBC)", NULL},
    {0, 0, 0, 0, NULL, NULL, NULL},
};

/* Array for transport id and corresponding acronyms. The
 * sg_get_trans_proto_str() function from the sg3_utils' library provides
 * the full protocol (transport) name. Those transports commented with
 * "none" don't have transport specific mode pages at this time. */
struct sdparm_val_desc_t sdparm_transport_id[] = {
    {TPROTO_FCP, "fcp"},
    {TPROTO_SPI, "spi"},
    {TPROTO_SSA, "ssa"},
    {TPROTO_1394, "sbp"},       /* none */
    {TPROTO_SRP, "srp"},
    {TPROTO_ISCSI, "iscsi"},    /* none */
    {TPROTO_SAS, "sas"},
    {TPROTO_ADT, "adt"},
    {TPROTO_ATA, "ata"},        /* none */
    {TPROTO_UAS, "uas"},        /* none */
    {TPROTO_SOP, "sop"},        /* none */
    {TPROTO_PCIE, "pcie"},      /* none */
    {0xc, "u0xc"},              /* leading "u" so not number */
    {0xd, "u0xd"},
    {0xe, "u0xe"},
    {TPROTO_NONE, "none"},
    {-1, NULL},
};

struct sdparm_val_desc_t sdparm_add_transport_acron[] = {
    {TPROTO_SPI, "para"},
    {TPROTO_SAS, "spl"},
    {TPROTO_PCIE, "nvme"},
    {TPROTO_ATA, "sata"},
    {TPROTO_SRP, "ib"},         /* InfiniBand */
    {TPROTO_UAS, "usb"},
    {-1, NULL},
};

static struct sdparm_mode_page_t sdparm_fcp_mode_pg[] = {    /* FCP-3,5 */
    {DISCONNECT_MP, 0, -1, 0, "dr", "Disconnect-reconnect (FCP)", NULL},
    {PROT_SPEC_LU_MP, 0, -1, 0, "luc", "lu: control (FCP)", NULL},
    {PROT_SPEC_PORT_MP, 0, -1, 0, "pc", "port: control (FCP)", NULL},
    {PROT_SPEC_LU_MP, 0, -1, 0, "pl", "lu: control (generic name)", NULL},
    {PROT_SPEC_PORT_MP, 0, -1, 0, "pp", "port: control (generic name)", NULL},
    {0, 0, 0, 0, NULL, NULL, NULL},
};

static struct sdparm_mode_page_t sdparm_spi_mode_pg[] = {    /* SPI-4 */
    {DISCONNECT_MP, 0, -1, 0, "dr", "Disconnect-reconnect (SPI)", NULL},
    {PROT_SPEC_LU_MP, 0, -1, 0, "luc", "lu: control (SPI)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 0, "mc",
        "port: margin control (SPI)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 0, "ns",
        "port: negotiated settings (SPI)", NULL},
    {PROT_SPEC_PORT_MP, 0, -1, 0, "psf", "port: short format (SPI)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 1, "rtc",
        "port: report transfer capabilities (SPI)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 0, "stc",
        "port: saved training config value (SPI)", NULL},
    /* second preference name so put out of alphabetical order */
    {PROT_SPEC_LU_MP, 0, -1, 0, "pl", "lu: control (generic name)", NULL},
    {PROT_SPEC_PORT_MP, 0, -1, 0, "pp", "port: short format (generic name)",
        NULL},
    {0, 0, 0, 0, NULL, NULL, NULL},
};

static struct sdparm_mode_page_t sdparm_srp_mode_pg[] = {    /* SRP */
    {DISCONNECT_MP, 0, -1, 0, "dr", "Disconnect-reconnect (SRP)", NULL},
    {0, 0, 0, 0, NULL, NULL, NULL},
};

static struct sdparm_mode_descriptor_t sas_pcd_desc = {   /* desc SAS/SPL */
    7, 1, 0, 8, 48, -1, -1, false, "SAS phy"
};

static struct sdparm_mode_descriptor_t sas_e_phy_desc = {  /* desc SAS/SPL */
    7, 1, 0, 8, -1, 2, 2, false, "Enhanced phy"
};

/* This one has a strange format, no number of descriptors and each
 * descriptor can have a variable size. */
static struct sdparm_mode_descriptor_t sas_oob_m_c_desc = { /* desc SAS/SPL */
    -1, -1, 0, 8, -1, 2, 2, true, "Attribute control"
};

/* N.B. In SAS 2.1 the spec was split with the upper levels going into the
 * SAS Protocol Layer (SPL) document. So now the SPL drafts are the
 * relevant SAS references. */
static struct sdparm_mode_page_t sdparm_sas_mode_pg[] = {    /* SAS/SPL */
    {DISCONNECT_MP, 0, -1, 0, "dr", "Disconnect-reconnect (SAS)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 0, "oobm",     /* spl5r01 */
        "Out of band management control (SAS)", &sas_oob_m_c_desc},
    {PROT_SPEC_LU_MP, 0, -1, 0, "pl", "Protocol specific logical unit (SAS)",
        NULL},
    {PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 0, "pcd",
        "Phy control and discover (SAS)", &sas_pcd_desc},
    {PROT_SPEC_PORT_MP, 0, -1, 0, "pp", "Protocol specific port (SAS)", NULL},
    {PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 0, "sep",
        "Enhanced phy control (SAS)", &sas_e_phy_desc},
    {PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 0, "spc",
        "Shared port control (SAS)", NULL},
    {0, 0, 0, 0, NULL, NULL, NULL},
};


/* These VPD pages are listed in alphabetical order based on their
 * 'acron' field. The standard inquiry response is added to this list. */
struct sdparm_vpd_page_t sdparm_vpd_pg[] = {
    {VPD_ATA_INFO, 0, -1, "ai", "ATA information (SAT)"},
    {VPD_ASCII_OP_DEF, 0, -1, "aod",
     "ASCII implemented operating definition (obs)"},
    {VPD_AUTOMATION_DEV_SN, 0, PDT_TAPE, "adsn", "Automation device serial "
     "number (SSC)"},
    {VPD_BLOCK_DEV_CHARS, 0, PDT_DISK, "bdc", "Block device characteristics "
     "(SBC)"},
    {VPD_BLOCK_DEV_C_EXTENS, 0, PDT_DISK, "bdce", "Block device "
     "characteristics extension (SBC)"},
    {VPD_BLOCK_LIMITS, 0, PDT_DISK, "bl", "Block limits (SBC)"},
    {VPD_BLOCK_LIMITS_EXT, 0, PDT_DISK, "ble",
     "Block limits extension (SBC)"},
    {VPD_CFA_PROFILE_INFO, 0, -1, "cfa", "CFA profile information"},
    {VPD_CON_POS_RANGE, 0, 0, "cpr", "Concurrent positioning ranges (SBC)"},
    {VPD_DEVICE_CONSTITUENTS, 0, -1, "dc", "Device constituents"},
    {VPD_DEVICE_ID, 0, -1, "di", "Device identification"},
    {VPD_DEVICE_ID, VPD_DI_SEL_AS_IS, -1, "di_asis", "Like 'di' "
     "but designators ordered as found"},
    {VPD_DEVICE_ID, VPD_DI_SEL_LU, -1, "di_lu", "Device identification, "
     "lu only"},
    {VPD_DEVICE_ID, VPD_DI_SEL_TPORT, -1, "di_port", "Device "
     "identification, target port only"},
    {VPD_DEVICE_ID, VPD_DI_SEL_TARGET, -1, "di_target", "Device "
     "identification, target device only"},
    {VPD_DTDE_ADDRESS, 0, 1, "dtde",
     "Data transfer device element address (SSC)"},
    {VPD_EXT_INQ, 0, -1, "ei", "Extended inquiry data"},
    {VPD_FORMAT_PRESETS, 0, 0, "fp", "Format presets (SBC)"},
    {VPD_IMP_OP_DEF, 0, -1, "iod",
     "Implemented operating definition (obs)"},
    {VPD_LB_PROTECTION, 0, PDT_TAPE, "lbpro", "Logical block protection "
     "(SSC)"},
    {VPD_LB_PROVISIONING, 0, PDT_DISK, "lbpv", "Logical block provisioning "
     "(SBC)"},
    {VPD_MAN_ASS_SN, 0, PDT_TAPE, "mas",
     "Manufacturer assigned serial number (SSC)"},
    {VPD_MAN_ASS_SN, 0, PDT_ADC, "masa",
     "Manufacturer assigned serial number (ADC)"},
    {VPD_MAN_NET_ADDR, 0, -1, "mna", "Management network addresses"},
    {VPD_MODE_PG_POLICY, 0, -1, "mpp", "Mode page policy"},
    {SG_NVME_VPD_NICR, 0, -1, "nicr", "NVMe Identify controller response"},
    {VPD_OSD_INFO, 0, PDT_OSD, "oi", "OSD information"},
    {VPD_POWER_CONDITION, 0, -1, "pc", "Power condition"},
    {VPD_POWER_CONSUMPTION, 0, -1, "psm", "Power consumption"},
    {VPD_PROTO_LU, 0, -1, "pslu", "Protocol-specific logical unit "
     "information"},
    {VPD_PROTO_PORT, 0, -1, "pspo", "Protocol-specific port information"},
    {VPD_REFERRALS, 0, PDT_DISK, "ref", "Referrals (SBC)"},
    {VPD_SA_DEV_CAP, 0, PDT_TAPE, "sad",
     "Sequential access device capabilities (SSC)"},
    {VPD_SCSI_FEATURE_SETS, 0, -1, "sfs", "SCSI Feature sets"},
    {VPD_SOFTW_INF_ID, 0, -1, "sii", "Software interface identification"},
    {VPD_NOT_STD_INQ, 0, -1, "sinq", "Standard inquiry response"},
    {VPD_UNIT_SERIAL_NUM, 0, -1, "sn", "Unit serial number"},
    {VPD_SCSI_PORTS, 0, -1, "sp", "SCSI ports"},
    {VPD_SUP_BLOCK_LENS, 0, PDT_DISK, "sbl", "Supported block lengths and "
     "protection types (SBC)"},
    {VPD_SUPPORTED_VPDS, 0, -1, "sv", "Supported VPD pages"},
    {VPD_TA_SUPPORTED, 0, PDT_TAPE, "tas", "TapeAlert supported flags (SSC)"},
    {VPD_3PARTY_COPY, 0, -1, "tpc", "Third party copy (SPC + SBC)"},
    {VPD_ZBC_DEV_CHARS, 0, -1, "zbdch", "Zoned block device characteristics "
     "(SBC + ZBC)"},
    {0, 0, 0, NULL, NULL},
};

/* Generic (i.e. non-transport specific) mode page items follow, */
/* sorted by mode page (then subpage) number in ascending order */
struct sdparm_mode_page_item sdparm_mitem_arr[] = {

    /* Read write error recovery mode page [0x1] sbc2, mmc5, ssc3 */
    /* treat as spc since various command sets implement variants */
    {"AWRE", RW_ERR_RECOVERY_MP, 0, -1, 2, 7, 1, MF_COMMON,
        "Automatic write reallocation enabled", NULL},
    {"ARRE", RW_ERR_RECOVERY_MP, 0, -1, 2, 6, 1, MF_COMMON,
        "Automatic read reallocation enabled", NULL},
    {"TB", RW_ERR_RECOVERY_MP, 0, -1, 2, 5, 1, 0,
        "Transfer block", NULL},
    {"RC", RW_ERR_RECOVERY_MP, 0, -1, 2, 4, 1, 0,
        "Read continuous", "0: error recovery may cause delays\t"
        "1: transfer data without waiting for error recovery"},
    {"EER", RW_ERR_RECOVERY_MP, 0, -1, 2, 3, 1, 0,
        "Enable early recovery (obsolete)",             /* in sbc4r02 */
        "1: increase chance of mis-detection or mis-correction of error"},
    {"PER", RW_ERR_RECOVERY_MP, 0, -1, 2, 2, 1, MF_COMMON,
        "Post error", "0: do not post recovered errors\t"
        "1: report recovered errors (via sense key: recovered error)"},
    {"DTE", RW_ERR_RECOVERY_MP, 0, -1, 2, 1, 1, 0,
        "Data terminate on error",
        "1: terminate data transfer when recovered error detected"},
    {"DCR", RW_ERR_RECOVERY_MP, 0, -1, 2, 0, 1, 0,
        "Disable correction (obsolete)", NULL},         /* in sbc4r02 */
    {"RRC", RW_ERR_RECOVERY_MP, 0, -1, 3, 7, 8, 0,
        "Read retry count", NULL},
    {"COR_S", RW_ERR_RECOVERY_MP, 0, -1, 4, 7, 8, 0,
        "Correction span (obsolete)", NULL},
    {"HOC", RW_ERR_RECOVERY_MP, 0, -1, 5, 7, 8, 0,
        "Head offset count (obsolete)", NULL},
    {"DSOC", RW_ERR_RECOVERY_MP, 0, -1, 6, 7, 8, 0,
        "Data strobe offset count (obsolete)", NULL},
    {"LBPERE", RW_ERR_RECOVERY_MP, 0, 0, 7, 7, 1, 0, /* SBC */
        "Logical block provisioning error reporting enabled", NULL},
    {"MWR", RW_ERR_RECOVERY_MP, 0, 0, 7, 6, 2, 0, /* sbc4r10 */
        "Misaligned write reporting", "0: disabled, don't report\t"
        "1: enabled, complete and report\t2: terminate, terminate "
        "and report"},
    {"EMCDR", RW_ERR_RECOVERY_MP, 0, 5, 7, 1, 2, 0, /* MMC */
        "Enhanced media certification and defect reporting", NULL},
    {"WRC", RW_ERR_RECOVERY_MP, 0, -1, 8, 7, 8, 0,
        "Write retry count", NULL},
    {"ERWS", RW_ERR_RECOVERY_MP, 0, 5, 9, 7, 24, 0, /* MMC, was ERTL */
        "Error reporting window size (blocks)", NULL},
    {"RTL", RW_ERR_RECOVERY_MP, 0, 0, 10, 7, 16, 0, /* SBC */
        "Recovery time limit (ms)", "0: default, -1: 65.5 seconds"},

    /* Disconnect-reconnect mode page [0x2]: spc-4 + */
    /* See transport sections for more detailed information about this page */
    {"BFR", DISCONNECT_MP, 0, -1, 2, 7, 8, 0,
        "Buffer full ratio",
        "fraction where this value is numerator, 256 is denominator"},
    {"BER", DISCONNECT_MP, 0, -1, 3, 7, 8, 0,
        "Buffer empty ratio",
        "fraction where this value is numerator, 256 is denominator"},
    {"BIL", DISCONNECT_MP, 0, -1, 4, 7, 16, 0,
        "Bus inactivity limit", "for unit see specific transport"},
    {"DTL", DISCONNECT_MP, 0, -1, 6, 7, 16, 0,
        "Disconnect time limit", "for unit see specific transport"},
    {"CTL", DISCONNECT_MP, 0, -1, 8, 7, 16, 0,
        "Connect time limit", "for unit see specific transport"},
    {"MBS", DISCONNECT_MP, 0, -1, 10, 7, 16, 0,
        "Maximum burst size (512 bytes)", NULL},
    {"EMDP", DISCONNECT_MP, 0, -1, 12, 7, 1, 0,
        "Enable modify data pointers",
        "1: target may send data out of order"},
    {"FA", DISCONNECT_MP, 0, -1, 12, 6, 3, 0,
        "Fair arbitration", NULL},
    {"DIMM", DISCONNECT_MP, 0, -1, 12, 3, 1, 0,
        "Disconnect immediate", NULL},
    {"DTDC", DISCONNECT_MP, 0, -1, 12, 2, 3, 0,
        "Data transfer disconnect control", NULL},
    {"FBS", DISCONNECT_MP, 0, -1, 14, 7, 16, 0,
        "First burst size (512 bytes)", NULL},

    /* Format mode page [0x3] sbc2 (obsolete) */
    {"TPZ", FORMAT_MP, 0, PDT_DISK, 2, 7, 16, 0,
        "Tracks per zone", NULL},
    {"ASPZ", FORMAT_MP, 0, PDT_DISK, 4, 7, 16, 0,
        "Alternate sectors per zone", NULL},
    {"ATPZ", FORMAT_MP, 0, PDT_DISK, 6, 7, 16, 0,
        "Alternate tracks per zone", NULL},
    {"ATPLU", FORMAT_MP, 0, PDT_DISK, 8, 7, 16, 0,
        "Alternate tracks per logical unit", NULL},
    {"SPT", FORMAT_MP, 0, PDT_DISK, 10, 7, 16, 0,
        "Sectors per track", NULL},
    {"DBPPS", FORMAT_MP, 0, PDT_DISK, 12, 7, 16, 0,
        "Data bytes per physical sector", NULL},
    {"INTLV", FORMAT_MP, 0, PDT_DISK, 14, 7, 16, 0,
        "Interleave", NULL},
    {"TSF", FORMAT_MP, 0, PDT_DISK, 16, 7, 16, 0,
        "Track skew factor", NULL},
    {"CSF", FORMAT_MP, 0, PDT_DISK, 18, 7, 16, 0,
        "Cylinder skew factor", NULL},
    {"SSEC", FORMAT_MP, 0, PDT_DISK, 20, 7, 1, 0,
        "Soft sector", NULL},
    {"HSEC", FORMAT_MP, 0, PDT_DISK, 20, 6, 1, 0,
        "Hard sector", NULL},
    {"RMB", FORMAT_MP, 0, PDT_DISK, 20, 5, 1, 0,
        "Removable", NULL},
    {"SURF", FORMAT_MP, 0, PDT_DISK, 20, 4, 1, 0,
        "Surface", NULL},

    /* Mount Rainier reWritable mode page [0x3] mmc4  */
    {"LBAS", MRW_MP, 0, PDT_MMC, 3, 0, 1, 0,
        "LBA space", NULL},

    /* Rigid disk mode page [0x4] sbc2 (obsolete) */
    {"NOC", RIGID_DISK_MP, 0, PDT_DISK, 2, 7, 24, 0,
        "Number of cylinders", NULL},
    {"NOH", RIGID_DISK_MP, 0, PDT_DISK, 5, 7, 8, 0,
        "Number of heads", NULL},
    {"SCWP", RIGID_DISK_MP, 0, PDT_DISK, 6, 7, 24, 0,
        "Starting cylinder for write precompensation", NULL},
    {"SCRWC", RIGID_DISK_MP, 0, PDT_DISK, 9, 7, 24, 0,
        "Starting cylinder for reduced write current", NULL},
    {"DSR", RIGID_DISK_MP, 0, PDT_DISK, 12, 7, 16, 0,
        "Device step rate", NULL},
    {"LZC", RIGID_DISK_MP, 0, PDT_DISK, 14, 7, 24, 0,
        "Landing zone cylinder", NULL},
    {"RPL", RIGID_DISK_MP, 0, PDT_DISK, 17, 1, 2, 0,
        "Rotational position locking", NULL},
    {"ROTO", RIGID_DISK_MP, 0, PDT_DISK, 18, 7, 8, 0,
        "Rotational offset", NULL},
    {"MRR", RIGID_DISK_MP, 0, PDT_DISK, 20, 7, 16, 0,
        "Medium rotation rate (rpm)", NULL},

    /* Flexible disk mode page [0x5] sbc (obsolete by sbc2r11) */
    {"XRATE", FLEX_DISK_MP, 0, PDT_DISK, 2, 7, 16, 0,
        "Transfer rate", NULL},
    {"NUM_HD", FLEX_DISK_MP, 0, PDT_DISK, 4, 7, 8, 0,
        "Number of heads", NULL},
    {"SECT_TR", FLEX_DISK_MP, 0, PDT_DISK, 5, 7, 8, 0,
        "Sectors per track", NULL},
    {"BYTE_SECT", FLEX_DISK_MP, 0, PDT_DISK, 6, 7, 16, 0,
        "Bytes per sector", NULL},
    {"NUM_CYL", FLEX_DISK_MP, 0, PDT_DISK, 8, 7, 16, 0,
        "Number of cylinders", NULL},
    /* Surely the rest (starting with 'write precompensation') are no
     * longer used. Some USB mass storage devices (flash) use this mpage. */

    /* Write parameters mode page [0x5] mmc5 */
    {"BUFE", WRITE_PARAM_MP, 0, PDT_MMC, 2, 6, 1, MF_COMMON,
        "Buffer underrun free recording enable", NULL},
    {"LS_V", WRITE_PARAM_MP, 0, PDT_MMC, 2, 5, 1, 0,
        "Link size valid", NULL},
    {"TST_W", WRITE_PARAM_MP, 0, PDT_MMC, 2, 4, 1, 0,
        "Test write", NULL},
    {"WR_T", WRITE_PARAM_MP, 0, PDT_MMC, 2, 3, 4, MF_COMMON,
        "Write type",
        "0: packet/incremental; 1: track-at-once\t"
        "2: session-at-once; 3: raw; 4: layer jump recording"},
    {"MULTI_S", WRITE_PARAM_MP, 0, PDT_MMC, 3, 7, 2, MF_COMMON,
        "Multi session",
        "0: next session not allowed (no BO pointer)\t"
        "1: next session not allowed\t"
        "3: next session allowed (indicated by BO pointer)"},
    {"FP", WRITE_PARAM_MP, 0, PDT_MMC, 3, 5, 1, 0,
        "Fixed packet type", NULL},
    {"COPY", WRITE_PARAM_MP, 0, PDT_MMC, 3, 4, 1, 0,
        "Serial copy management system (SCMS) enable", NULL},
    {"TRACK_M", WRITE_PARAM_MP, 0, PDT_MMC, 3, 3, 4, 0,
        "Track mode", NULL},
    {"DBT", WRITE_PARAM_MP, 0, PDT_MMC, 4, 3, 4, 0,
        "Data block type", NULL},
    {"LINK_S", WRITE_PARAM_MP, 0, PDT_MMC, 5, 7, 8, 0,
        "Link size", NULL},
    {"IAC", WRITE_PARAM_MP, 0, PDT_MMC, 7, 5, 6, 0,
        "Initiator application code", NULL},
    {"SESS_F", WRITE_PARAM_MP, 0, PDT_MMC, 8, 7, 8, 0,
        "Session format", NULL},
    {"PACK_S", WRITE_PARAM_MP, 0, PDT_MMC, 10, 7, 32, 0,
        "Packet size", NULL},
    {"APL", WRITE_PARAM_MP, 0, PDT_MMC, 14, 7, 16, 0,
        "Audio pause length (blocks)", NULL},

    /* Device parameters mode page [0x6] rbc */
    {"WCD", RBC_DEV_PARAM_MP, 0, PDT_RBC, 2, 0, 1, MF_COMMON,
        "Write cache disable", NULL},
    {"LBS", RBC_DEV_PARAM_MP, 0, PDT_RBC, 3, 7, 16, MF_COMMON,
        "Logical block size", NULL},
    {"NLBS", RBC_DEV_PARAM_MP, 0, PDT_RBC, 5, 7, 40, (MF_COMMON | MF_HEX),
        "Number of logical blocks", NULL},
    {"P_P", RBC_DEV_PARAM_MP, 0, PDT_RBC, 10, 7, 8, 0,
        "Power/performance", NULL},
    {"READD", RBC_DEV_PARAM_MP, 0, PDT_RBC, 11, 3, 1, 0,
        "Read disable", NULL},
    {"WRITED", RBC_DEV_PARAM_MP, 0, PDT_RBC, 11, 2, 1, 0,
        "Write disable", NULL},
    {"FORMATD", RBC_DEV_PARAM_MP, 0, PDT_RBC, 11, 1, 1, 0,
        "Format disable", NULL},
    {"LOCKD", RBC_DEV_PARAM_MP, 0, PDT_RBC, 11, 0, 1, 0,
        "Lock disable", NULL},

    /* Verify error recovery mode page [0x7] sbc2 */
    {"V_EER", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 2, 3, 1, 0,
        "Enable early recovery (obsolete) ", NULL}, /* in sbc4r02 */
    {"V_PER", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 2, 2, 1, 0,
        "Post error", NULL},
    {"V_DTE", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 2, 1, 1, 0,
        "Data terminate on error", NULL},
    {"V_DCR", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 2, 0, 1, 0,
        "Disable correction (obsolete)", NULL},    /* in sbc4r02 */
    {"V_RC", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 3, 7, 8, 0,
        "Verify retry count", NULL},
    {"V_COR_S", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 4, 7, 8, 0,
        "Verify correction span (obsolete)", NULL},
    {"V_RTL", V_ERR_RECOVERY_MP, 0, PDT_DISK_ZBC, 10, 7, 16, 0,
        "Verify recovery time limit (ms)", NULL},

    /* Caching mode page [0x8] sbc2 */
    {"IC", CACHING_MP, 0, PDT_DISK_ZBC, 2, 7, 1, 0,
        "Initiator control",
        "0: disk uses own adaptive caching algorithm\t"
        "1: disk caching algorithm controlled by NCS or CCS"},
    {"ABPF", CACHING_MP, 0, PDT_DISK_ZBC, 2, 6, 1, 0,
        "Abort pre-fetch", NULL},
    {"CAP", CACHING_MP, 0, PDT_DISK_ZBC, 2, 5, 1, 0,
        "Caching analysis permitted", NULL},
    {"DISC", CACHING_MP, 0, PDT_DISK_ZBC, 2, 4, 1, 0,
        "Discontinuity",
        "0: pre-fetch truncated or wrapped at time discontinuity\t"
        "1: pre-fetch continues across time discontinuity"},
    {"SIZE", CACHING_MP, 0, PDT_DISK_ZBC, 2, 3, 1, 0,
        "Size enable",
        "0: number of cache segments (NCS) controls cache segmentation\t"
        "1: the cache segment size (CCS) controls cache segmentation"},
    {"WCE", CACHING_MP, 0, PDT_DISK_ZBC, 2, 2, 1, MF_COMMON,
        "Write cache enable", NULL},
    {"MF", CACHING_MP, 0, PDT_DISK_ZBC, 2, 1, 1, 0,
        "Multiplication factor",
        "0: MIPF and MAPF specify blocks\t"
        "1: multiply MIPF and MAPF by blocks in read command"},
    {"RCD", CACHING_MP, 0, PDT_DISK_ZBC, 2, 0, 1, MF_COMMON,
        "Read cache disable", NULL},
    {"DRRP", CACHING_MP, 0, PDT_DISK_ZBC, 3, 7, 4, 0,
        "Demand read retention priority",
        "0: treat requested and other data equally\t"
        "1: replace requested data before other data\t"
        "15: replace other data before requested data"},
    {"WRP", CACHING_MP, 0, PDT_DISK_ZBC, 3, 3, 4, 0,
        "Write retention priority",
        "0: treat requested and other data equally\t"
        "1: replace requested data before other data\t"
        "15: replace other data before requested data"},
    {"DPTL", CACHING_MP, 0, PDT_DISK_ZBC, 4, 7, 16, 0,
        "Disable pre-fetch transfer length", NULL},
    {"MIPF", CACHING_MP, 0, PDT_DISK_ZBC, 6, 7, 16, 0,
        "Minimum pre-fetch", NULL},
    {"MAPF", CACHING_MP, 0, PDT_DISK_ZBC, 8, 7, 16, 0,
        "Maximum pre-fetch", NULL},
    {"MAPFC", CACHING_MP, 0, PDT_DISK_ZBC, 10, 7, 16, 0,
        "Maximum pre-fetch ceiling", NULL},
    {"FSW", CACHING_MP, 0, PDT_DISK_ZBC, 12, 7, 1, 0,
        "Force sequential write", NULL},
    {"LBCSS", CACHING_MP, 0, PDT_DISK_ZBC, 12, 6, 1, 0,
        "Logical block cache segment size",
        "0: CSS unit is bytes; 1: CSS unit is blocks"},
    {"DRA", CACHING_MP, 0, PDT_DISK_ZBC, 12, 5, 1, 0,
        "Disable read ahead", NULL},
    {"SYNC_PROG", CACHING_MP, 0, PDT_DISK_ZBC, 12, 2, 2, 0, /* sbc3r33 */
        "Synchronous cache progress indication",
        "0: no pollable sense data during sync\t"
        "1: allow pollable sense data, allow all commands during sync\t"
        "2: allow pollable sense data, allow some commands during sync"},
    {"NV_DIS", CACHING_MP, 0, PDT_DISK_ZBC, 12, 0, 1, 0,
        "Non-volatile cache disable", NULL},
    {"NCS", CACHING_MP, 0, PDT_DISK_ZBC, 13, 7, 8, 0,
        "Number of cache segments", NULL},
    {"CSS", CACHING_MP, 0, PDT_DISK_ZBC, 14, 7, 16, 0,
        "Cache segment size", NULL},

    /* Control mode page [0xa] spc3 */
    {"TST", CONTROL_MP, 0, -1, 2, 7, 3, 0,
        "Task set type",
        "0: lu maintains one task set for all I_T nexuses\t"
        "1: lu maintains separate task sets for each I_T nexus"},
    {"TMF_ONLY", CONTROL_MP, 0, -1, 2, 4, 1, 0,
        "Task management functions only", NULL},
    {"DPICZ", CONTROL_MP, 0, -1, 2, 3, 1, 0,
        "Disable protection information check if protect field zero", NULL},
    {"D_SENSE", CONTROL_MP, 0, -1, 2, 2, 1, 0,
        "Descriptor format sense data", NULL},
    {"GLTSD", CONTROL_MP, 0, -1, 2, 1, 1, 0,
        "Global logging target save disable", NULL},
    {"RLEC", CONTROL_MP, 0, -1, 2, 0, 1, 0,
        "Report log exception condition", NULL},
    {"QAM", CONTROL_MP, 0, -1, 3, 7, 4, 0,
        "Queue algorithm modifier",
        "0: restricted re-ordering; 1: unrestricted"},
    {"NUAR", CONTROL_MP, 0, -1, 3, 3, 1, 0,
        "No unit attention on release", NULL},
    {"QERR", CONTROL_MP, 0, -1, 3, 2, 2, 0,
        "Queue error management",
        "0: only affected task gets CC; 1: affected tasks aborted\t"
        "3: affected tasks aborted on same I_T nexus"},
    {"VS_CTL", CONTROL_MP, 0, -1, 4, 7, 1, 0,
        "Vendor specific [byte 4, bit 7]", NULL},
    {"RAC", CONTROL_MP, 0, -1, 4, 6, 1, 0,
        "Report a check", NULL},
    {"UA_INTLCK", CONTROL_MP, 0, -1, 4, 5, 2, 0,
        "Unit attention interlocks control",
        "0: unit attention cleared with check condition status\t"
        "2: unit attention not cleared with check condition status\t"
        "3: as 2 plus ua on busy, task set full or reservation conflict"},
    {"SWP", CONTROL_MP, 0, -1, 4, 3, 1, MF_COMMON,
        "Software write protect", NULL},
    {"ATO", CONTROL_MP, 0, -1, 5, 7, 1, 0,
        "Application tag owner", NULL},
    {"TAS", CONTROL_MP, 0, -1, 5, 6, 1, 0,
        "Task aborted status",
        "0: tasks aborted without response to app client\t"
        "1: any other I_T nexuses receive task aborted"},
    {"ATMPE", CONTROL_MP, 0, -1, 5, 5, 1, 0,    /* spc4r27 */
        "Application tag mode page enabled", NULL},
    {"RWWP", CONTROL_MP, 0, -1, 5, 4, 1, 0,     /* spc4r27 */
        "Reject write without protection", NULL},
    {"SBLP", CONTROL_MP, 0, -1, 5, 3, 1, 0,     /* spc5r02 */
        "Supported block lengths and protection information", NULL},
    {"AUTOLOAD", CONTROL_MP, 0, -1, 5, 2, 3, 0,
        "Autoload mode",
        "0: medium loaded for full access\t"
        "1: loaded for medium auxiliary access only\t"
        "2: medium shall not be loaded"},
    {"BTP", CONTROL_MP, 0, -1, 8, 7, 16, 0,
        "Busy timeout period (100us)",
        "0: undefined\t"
        "0ffffh (-1): unlimited"},
    {"ESTCT", CONTROL_MP, 0, -1, 10, 7, 16, 0,
        "Extended self test completion time (sec)",
        "0ffffh (-1) takes 65535 seconds or longer"},

    /* Control extension mode subpage [0xa,0x1] spc3 */
    {"DLC", CONTROL_MP, MSP_SPC_CE, -1, 4, 3, 1, 0,   /* spc5r02 */
        "Device life control",
        "0: may degrade performance to prolong life\t"
        "1: shall not degrade performance"},
    {"TCMOS", CONTROL_MP, MSP_SPC_CE, -1, 4, 2, 1, 0,
        "Timestamp changeable by methods outside standard", NULL},
    {"SCSIP", CONTROL_MP, MSP_SPC_CE, -1, 4, 1, 1, 0,
        "SCSI timestamp commands take precedence over other methods", NULL},
    {"IALUAE", CONTROL_MP, MSP_SPC_CE, -1, 4, 0, 1, 0,
        "Implicit asymmetric logical unit access enabled", NULL},
    {"INIT_PR", CONTROL_MP, MSP_SPC_CE, -1, 5, 3, 4, 0,
        "Initial command priority", "0: none or vendor; 1: highest; "
        "15: lowest"},
    {"MSDL", CONTROL_MP, MSP_SPC_CE, -1, 6, 7, 8, 0,  /* spc4r34 */
        "Maximum sense data length", "0: unlimited"},
    {"NSQCC", CONTROL_MP, MSP_SPC_CE, -1, 7, 7, 8, 0, /* spc6r05 */
        "Non-sequestered command count", NULL},
    {"SQCO", CONTROL_MP, MSP_SPC_CE, -1, 8, 7, 8, 0,  /* spc6r05 */
        "Sequestered command order",
        "0: oldest\t1: best IOPS\t2: IOPS and other sources"},

    /* Application tag mode subpage: atag [0xa,0x2] sbc3r25 */
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 16 more than shown in t10's descriptor format table) */
    {"AT_LAST", CONTROL_MP, MSP_SBC_APP_TAG, PDT_DISK_ZBC, 16, 7, 1, 0,
        "Last", NULL},
    {"AT_LBAT", CONTROL_MP, MSP_SBC_APP_TAG, PDT_DISK_ZBC, 22, 7, 16, MF_HEX,
        "Logical block application tag", NULL},
    {"AT_LBA", CONTROL_MP, MSP_SBC_APP_TAG, PDT_DISK_ZBC, 24, 7, 64, MF_HEX,
        "Logical block address", "start LBA for this application tag"},
    {"AT_COUNT", CONTROL_MP, MSP_SBC_APP_TAG, PDT_DISK_ZBC, 32, 7, 64,
     MF_HEX | MF_ALL_1S, "Logical block count", NULL},

    /* Command duration limit A mode subpage: cdla [0xa,0x3] spc5 */
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 8 more than shown in t10's descriptor format table) */
    {"CDA_UNIT", CONTROL_MP, MSP_SPC_CDLA, -1, 8, 7, 3, 0,
        "CDLA unit",
        "0: no duration limit\t"
        "4: 1 microsecond\t"
        "5: 10 microseconds\t"
        "6: 500 microseconds"},
    {"CDA_LIMIT", CONTROL_MP, MSP_SPC_CDLA, -1, 10, 7, 16, 0,
        "Command duration limit", NULL},

    /* Command duration limit B mode subpage: cdlb [0xa,0x4] spc5 */
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 8 more than shown in t10's descriptor format table) */
    {"CDB_UNIT", CONTROL_MP, MSP_SPC_CDLB, -1, 8, 7, 3, 0,
        "CDL unit",
        "0: no duration limit\t"
        "4: 1 microsecond\t"
        "5: 10 microseconds\t"
        "6: 500 microseconds"},
    {"CDB_LIMIT", CONTROL_MP, MSP_SPC_CDLB, -1, 10, 7, 16, 0,
        "Command duration limit", NULL},

    /* IO advice hints grouping mode subpage: ioad [0xa,0x5] sbc4 */
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 16 more than shown in t10's descriptor format table) */
    {"IOA_MODE", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 16, 7, 2, 0,
        "IO advice hints mode", "0: valid; 1: invalid"},
    {"CS_EN", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 16, 1, 1, 0,
        "Cache segment enable", NULL},
    {"IC_EN", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 16, 0, 1, 0,
        "Information collection enable", NULL},
    /* Assume Logical Block Markup (LBM) descriptor type 0 (i.e. access
     * patterns) */
    {"ACDLU", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 20, 7, 1, 0,
        "Access continue during low utilization", NULL},
    {"RLBSR", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 20, 5, 2, 0,
        "Related logical blocks and subsequent reads",
     "0: no information; 1: LBs associated, no subsequent reads expected;\t"
     "3: LBs associated, subsequent reads expected"},
    {"LBM_DT", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 20, 3, 4, 0,
        "LBM descriptor type", "0: access patterns; else trouble"},
    {"OV_FR", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 21, 7, 2, 0,
        "Overall frequency", "0: equally; 1: less; 2: more"},
    {"RW_FR", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 21, 5, 2, 0,
        "Read/write frequency", "0: equally; 1: rd > wr; 2: wr > rd"},
    {"WR_SE", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 21, 3, 2, 0,
        "Write sequentiality",
        "0: equally; 1: random more; 2: sequential more"},
    {"RD_SE", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 21, 1, 2, 0,
        "Read sequentiality",
        "0: equally; 1: random more; 2: sequential more"},
    {"IO_CL", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 22, 7, 4, 0,
        "IO class",
        "0: none; 1: meta-data; 4: small colloection; 5: large collection"},
    {"SU_IO", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 22, 3, 2, 0,
        "Subsequent I/O",
        "0: unknown; 1: low probability; 2: high probability"},
    {"OSI_PR", CONTROL_MP, MSP_SBC_IO_ADVI, -1, 22, 1, 2, 0,
        "Operating System Initialization (OSI) proximity",
        "0: unknown; 1: improbable; 2: probable"},

    /* Background operation control mode subpage: bop [0xa,0x6] sbc4 */
    {"BO_MODE", CONTROL_MP, MSP_SBC_BACK_OP, PDT_DISK, 4, 7, 2, 0,
        "Background operation mode", "host initiated advanced background "
        "operations:\t"
        "0: suspended during IO\t"
        "1: continue during IO"},

    /* Command duration limit T2A mode subpage: cdt2a [0xa,0x7] spc6 */
    {"PVCDG", CONTROL_MP, MSP_SPC_CDLT2A, -1, 7, 7, 4, 0,
        "Perf versus command duration guidelines",
        "Maximium percentage increase in average command completion times:\t"
        "0: 0%\t1: 0.5%\t...\t6: 3%\t7: 4%\t8: 5%\t9: 8%\t10: 10%"
        "11: 15%\t12: 20%"},
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 8 more than shown in t10's descriptor format table) */
    {"T2CDLU", CONTROL_MP, MSP_SPC_CDLT2A, -1, 8, 3, 4, MF_CLASH_OK,
        "T2 command duration limit units", "0: none\t6: 500 nanoseconds\t"
        "8: 1 microsecond\t10: 10 milliseconds\t14: 500 milliseconds"},
    {"MXINATI", CONTROL_MP, MSP_SPC_CDLT2A, -1, 10, 7, 16, MF_CLASH_OK,
        "Max inactive time", NULL},
    {"MXACTTI", CONTROL_MP, MSP_SPC_CDLT2A, -1, 12, 7, 16, MF_CLASH_OK,
        "Max active time", NULL},
    {"MXINATP", CONTROL_MP, MSP_SPC_CDLT2A, -1, 14, 7, 4, MF_CLASH_OK,
        "Max inactive time policy", "0: asap\t"
        "13: good, completed, data currently unavailable\t"
        "15: terminate, aborted command, command timeout before processing"},
    {"MXACTTP", CONTROL_MP, MSP_SPC_CDLT2A, -1, 14, 3, 4, MF_CLASH_OK,
        "Max active time policy", "0: asap\t"
        "13: good, completed, data currently unavailable\t"
        "14: as per 15, may report largest LBA processed"
        "15: terminate, aborted command, command timeout before processing"},
    {"CDGUID", CONTROL_MP, MSP_SPC_CDLT2A, -1, 18, 7, 16, MF_CLASH_OK,
        "Command duration guideline",
        "0: ignore\t>0: preferred command duration"},
    {"CDGUPOL", CONTROL_MP, MSP_SPC_CDLT2A, -1, 22, 7, 16, MF_CLASH_OK,
        "Command duration guideline policy", "0: asap\t"
        "1: next highest CDL descriptor\t"
        "2: continue as if no CDL\t"
        "13: good, completed, data currently unavailable\t"
        "15: terminate, aborted command, command timeout before processing"},
    {"BYP_SEQ", CONTROL_MP, MSP_SPC_CDLT2A, -1, 23, 0, 1, MF_CLASH_OK,
        "Bypass sequestration", NULL},

    /* Command duration limit T2B mode subpage: cdt2b [0xa,0x8] spc6 */
    {"PVLC", CONTROL_MP, MSP_SPC_CDLT2B, -1, 7, 3, 4, 0,
        "Perf versus latency control",
        "This field is not defined in SPC6r05??\t"
        "Maximium percentage increase in average command completion times:\t"
        "0: 0%\t1: 0.5%\t...\t6: 3%\t7: 4%\t8: 5%\t9: 8%\t10: 10%"
        "11: 15%\t12: 20%"},
    /* descriptor starts here, <start_byte> is relative to start of mode
     * page (i.e. 8 more than shown in t10's descriptor format table) */
    {"T2CDLU", CONTROL_MP, MSP_SPC_CDLT2B, -1, 8, 3, 4, MF_CLASH_OK,
        "T2 command duration limit units", "0: none\t6: 500 nanoseconds\t"
        "8: 1 microsecond\t10: 10 milliseconds\t14: 500 milliseconds"},
    {"MXINATI", CONTROL_MP, MSP_SPC_CDLT2B, -1, 10, 7, 16, MF_CLASH_OK,
        "Max inactive time", NULL},
    {"MXACTTI", CONTROL_MP, MSP_SPC_CDLT2B, -1, 12, 7, 16, MF_CLASH_OK,
        "Max active time", NULL},
    {"MXINATP", CONTROL_MP, MSP_SPC_CDLT2B, -1, 14, 7, 4, MF_CLASH_OK,
        "Max inactive time policy", "0: asap\t"
        "13: good, completed, data currently unavailable\t"
        "15: terminate, aborted command, command timeout before processing"},
    {"MXACTTP", CONTROL_MP, MSP_SPC_CDLT2B, -1, 14, 3, 4, MF_CLASH_OK,
        "Max active time policy", "0: asap\t"
        "13: good, completed, data currently unavailable\t"
        "14: as per 15, may report largest LBA processed"
        "15: terminate, aborted command, command timeout before processing"},
    {"CDGUID", CONTROL_MP, MSP_SPC_CDLT2B, -1, 18, 7, 16, MF_CLASH_OK,
        "Command duration guideline",
        "0: ignore\t>0: preferred command duration"},
    {"CDGUPOL", CONTROL_MP, MSP_SPC_CDLT2B, -1, 22, 7, 16, MF_CLASH_OK,
        "Command duration guideline policy", "0: asap\t"
        "1: next highest CDL descriptor\t"
        "2: continue as if no CDL\t"
        "13: good, completed, data currently unavailable\t"
        "15: terminate, aborted command, command timeout before processing"},
    {"BYP_SEQ", CONTROL_MP, MSP_SPC_CDLT2B, -1, 23, 0, 1, MF_CLASH_OK,
        "Bypass sequestration", NULL},

    /* Zoned Block device Control mode subpage: zbcc [0xa,0x9] zbc2r04a */
    /* Probably only applies to host-managed ZBC (pdt=0x14) but set pdt=-1
     * in these entries in case it could apply to host-aware (pdt=0x0) */
    {"URSWRZ_M", CONTROL_MP, MSP_ZB_D_CTL, PDT_DISK_ZBC, 4, 0, 1, 0,
        "Unrestricted read in sequential write required management",
        "0: not configure to support reading unwritten blocks\t"
        "1: configure to support reading unwritten blocks"},
    {"U_UA_CTL", CONTROL_MP, MSP_ZB_D_CTL, PDT_DISK_ZBC, 5, 0, 1, 0,
        "Unrestricted read in sequential write required zone unit "
        "attention control",
        "0: issue 'Mode parameters changed' UA when URSWRZ changed\t"
        "1: issue 'Inquiry data has changed' UA when URSWRZ changed"},

    /* Control data protection mode subpage: cdp [0xa,0xf0] ssc4 */
    {"LBPM", CONTROL_MP, MSP_SSC_CDP, PDT_TAPE, 4, 7, 8, 0,
        "Logical block protection method", "0: none\t"
        "1: Reed-Solomon CRC\t2: CRC32C (Castagnoli)\t>= 0xf0: vendor"},
    {"LBPIL", CONTROL_MP, MSP_SSC_CDP, PDT_TAPE, 5, 5, 6, 0,
        "Logical block protection information length", NULL},
    {"LBP_W", CONTROL_MP, MSP_SSC_CDP, PDT_TAPE, 6, 7, 1, 0,
        "Logical block protection during write", NULL},
    {"LBP_R", CONTROL_MP, MSP_SSC_CDP, PDT_TAPE, 6, 6, 1, 0,
        "Logical block protection during read", NULL},
    {"RBDP", CONTROL_MP, MSP_SSC_CDP, PDT_TAPE, 6, 5, 1, 0,
        "Recover buffered data protected", NULL},

    /* SAT: pATA control mode subpage: pat [0xa,0xf1] sat-r09 */
    /* treat as spc since could be disk or ATAPI */
    {"MWD2", CONTROL_MP, MSP_SAT_PATA, -1, 4, 6, 1, 0,
        "Multi word DMA bit 2", NULL},
    {"MWD1", CONTROL_MP, MSP_SAT_PATA, -1, 4, 5, 1, 0,
        "Multi word DMA bit 1", NULL},
    {"MWD0", CONTROL_MP, MSP_SAT_PATA, -1, 4, 4, 1, 0,
        "Multi word DMA bit 0", NULL},
    {"PIO4", CONTROL_MP, MSP_SAT_PATA, -1, 4, 1, 1, 0,
        "Parallel IO bit 4", NULL},
    {"PIO3", CONTROL_MP, MSP_SAT_PATA, -1, 4, 0, 1, 0,
        "Parallel IO bit 3", NULL},
    {"UDMA6", CONTROL_MP, MSP_SAT_PATA, -1, 5, 6, 1, 0,
        "Ultra DMA bit 6", NULL},
    {"UDMA5", CONTROL_MP, MSP_SAT_PATA, -1, 5, 5, 1, 0,
        "Ultra DMA bit 5", NULL},
    {"UDMA4", CONTROL_MP, MSP_SAT_PATA, -1, 5, 4, 1, 0,
        "Ultra DMA bit 4", NULL},
    {"UDMA3", CONTROL_MP, MSP_SAT_PATA, -1, 5, 3, 1, 0,
        "Ultra DMA bit 3", NULL},
    {"UDMA2", CONTROL_MP, MSP_SAT_PATA, -1, 5, 2, 1, 0,
        "Ultra DMA bit 2", NULL},
    {"UDMA1", CONTROL_MP, MSP_SAT_PATA, -1, 5, 1, 1, 0,
        "Ultra DMA bit 1", NULL},
    {"UDMA0", CONTROL_MP, MSP_SAT_PATA, -1, 5, 0, 1, 0,
        "Ultra DMA bit 0", NULL},

    /* SAT: ATA feature control mode subpage: afc [0xa,0xf2] 20-085r4 */
    /* treat as spc since could be disk or ATAPI */
    {"CDL_CTRL", CONTROL_MP, MSP_SAT_AFC, -1, 4, 1, 2, 0,
        "Command duration limits control",
        "0: ATA 0->cdl_action, no CDL mpages supported\t"
        "1: ATA 0->cdl_action, CDL A mpage supported, maybe CDL B\t"
        "2: ATA 1->cdl_action, CDL T2A mpage supported, maybe CDL T2B"},

    /* Notch and partition mode page [0xc] sbc2 (obsolete in sbc2r14) */
    {"ND", NOTCH_MP, 0, PDT_DISK, 2, 7, 1, 0,
        "Notched device", NULL},
    {"LPN", NOTCH_MP, 0, PDT_DISK, 2, 6, 1, 0,
        "Logical or physical notch", "0: physical; 1: logical"},
    {"MNN", NOTCH_MP, 0, PDT_DISK, 4, 7, 16, 0,
        "Maximum number of notches", NULL},
    {"ANOT", NOTCH_MP, 0, PDT_DISK, 6, 7, 16, 0,
        "Active notch", "origin 1, 0 for all"},
    {"SBOU", NOTCH_MP, 0, PDT_DISK, 8, 7, 32, MF_HEX,
        "Starting boundary", NULL},
    {"EBOU", NOTCH_MP, 0, PDT_DISK, 12, 7, 32, MF_HEX,
        "Ending boundary", NULL},
    {"PNOT", NOTCH_MP, 0, PDT_DISK, 16, 7, 64, MF_HEX,
        "Pages notched",
        "bit map of mpages altered by notching\tMSb: mpage 0x3f"},

    /* Power condition mode page: poo, obsolete block-device-only version */
    /*   [0xd] sbc (replacement page now at 0x1a) */
    {"IDLE-OLD", POWER_OLD_MP, 0, PDT_DISK, 3, 1, 1, 0,
        "Idle timer active", NULL},
    {"STBY-OLD", POWER_OLD_MP, 0, PDT_DISK, 3, 0, 1, 0,
        "Standby timer active", NULL},
    {"ICT-OLD", POWER_OLD_MP, 0, PDT_DISK, 4, 7, 32, 0,
        "Idle condition timer (100 ms)", NULL},
    {"SCT-OLD", POWER_OLD_MP, 0, PDT_DISK, 8, 7, 32, 0,
        "Standby condition timer (100 ms)", NULL},

    /* Data compression mode page: dac [0xf] ssc3 */
    {"DCE", DATA_COMPR_MP, 0, PDT_TAPE, 2, 7, 1, MF_COMMON,
        "Data compression enable", NULL},
    {"DCC", DATA_COMPR_MP, 0, PDT_TAPE, 2, 6, 1, MF_COMMON,
        "Data compression capable", NULL},
    {"DDE", DATA_COMPR_MP, 0, PDT_TAPE, 3, 7, 1, MF_COMMON,
        "Data decompression enable", NULL},
    {"RED", DATA_COMPR_MP, 0, PDT_TAPE, 3, 6, 2, 0,
        "Report exception on decompression", NULL},
    {"COMPR_A", DATA_COMPR_MP, 0, PDT_TAPE, 4, 7, 32, 0,
        "Compression algorithm",
        "0: none; 1: default; 5: ALDC (2048 byte); 16: IDRC; 32: DCLZ"},
    {"DCOMPR_A", DATA_COMPR_MP, 0, PDT_TAPE, 8, 7, 32, 0,
        "Decompression algorithm",
        "0: none; 1: default; 5: ALDC (2048 byte); 16: IDRC; 32: DCLZ"},

    /* XOR control mode page: xo [0x10] sbc2 << obsolete in sbc3r32>> */
    {"XORDIS", XOR_MP, 0, PDT_DISK, 2, 1, 1, 0,
        "XOR disable", NULL},
    {"MXWS", XOR_MP, 0, PDT_DISK, 4, 7, 32, 0,
        "Maximum XOR write size (blocks)", NULL},

    /* Device configuration mode page: dc [0x10] ssc3 */
    {"CAF", DEV_CONF_MP, 0, PDT_TAPE, 2, 5, 1, 0,
        "Change active format", NULL},
    {"ACT_F", DEV_CONF_MP, 0, PDT_TAPE, 2, 4, 5, 0,
        "Active format", NULL},
    {"ACT_P", DEV_CONF_MP, 0, PDT_TAPE, 3, 7, 8, 0,
        "Active partition", NULL},
    {"WOBFR", DEV_CONF_MP, 0, PDT_TAPE, 4, 7, 8, 0,
        "Write object buffer full ratio", NULL},
    {"ROBER", DEV_CONF_MP, 0, PDT_TAPE, 5, 7, 8, 0,
        "Read object buffer empty ratio", NULL},
    {"WDT", DEV_CONF_MP, 0, PDT_TAPE, 6, 7, 16, 0,
        "Write delay time (100 ms)", NULL},
    {"OBR", DEV_CONF_MP, 0, PDT_TAPE, 8, 7, 1, 0,
        "Object buffer recovery", NULL},
    {"LOIS", DEV_CONF_MP, 0, PDT_TAPE, 8, 6, 1, 0,
        "Logical object identifiers supported", NULL},
    {"RSMK", DEV_CONF_MP, 0, PDT_TAPE, 8, 5, 1, MF_COMMON,
        "Report setmarks (obsolete)", NULL},
    {"AVC", DEV_CONF_MP, 0, PDT_TAPE, 8, 4, 1, 0,
        "Automatic velocity control", NULL},
    {"SOCF", DEV_CONF_MP, 0, PDT_TAPE, 8, 3, 2, 0,
        "Stop on consecutive filemarks", NULL},
    {"ROBO", DEV_CONF_MP, 0, PDT_TAPE, 8, 1, 1, 0,
        "Recover object buffer order", NULL},
    {"REW", DEV_CONF_MP, 0, PDT_TAPE, 8, 0, 1, 0,
        "Report early warning", NULL},
    {"GAP_S", DEV_CONF_MP, 0, PDT_TAPE, 9, 7, 8, 0,
        "Gap size (obsolete)", NULL},
    {"EOD_D", DEV_CONF_MP, 0, PDT_TAPE, 10, 7, 3, 0,
        "EOD (end-of-data) defined",
        "0: default; 1: format defined; 2: SOCF; 3: not supported"},
    {"EEG", DEV_CONF_MP, 0, PDT_TAPE, 10, 4, 1, 0,
        "Enable EOD generation", NULL},
    {"SEW", DEV_CONF_MP, 0, PDT_TAPE, 10, 3, 1, MF_COMMON,
        "Synchronize early warning", NULL},
    {"SWP_T", DEV_CONF_MP, 0, PDT_TAPE, 10, 2, 1, 0,
        "Software write protect (tape)", NULL},
    {"BAML", DEV_CONF_MP, 0, PDT_TAPE, 10, 1, 1, 0,
        "Block address mode lock", NULL},
    {"BAM", DEV_CONF_MP, 0, PDT_TAPE, 10, 0, 1, 0,
        "Block address mode", NULL},
    {"OBSAEW", DEV_CONF_MP, 0, PDT_TAPE, 11, 7, 24, 0,
        "Object buffer size at early warning", NULL},
    {"SDCA", DEV_CONF_MP, 0, PDT_TAPE, 14, 7, 8, MF_COMMON,
        "Select data compression algorithm", NULL},
    {"WTRE", DEV_CONF_MP, 0, PDT_TAPE, 15, 7, 2, 0,
        "WORM tamper read enable", NULL},
    {"OIR", DEV_CONF_MP, 0, PDT_TAPE, 15, 5, 1, 0,
        "Only if reserved", NULL},
    {"ROR", DEV_CONF_MP, 0, PDT_TAPE, 15, 4, 2, 0,
        "Rewind on reset",
        "0: vendor specific; 1: to BOP 0 on lu reset\t"
        "2: hold position on lu reset"},
    {"ASOCWP", DEV_CONF_MP, 0, PDT_TAPE, 15, 2, 1, 0,
        "Associated write protection", NULL},
    {"PERSWP", DEV_CONF_MP, 0, PDT_TAPE, 15, 1, 1, 0,
        "Persistent write protection", NULL},
    {"PRMWP", DEV_CONF_MP, 0, PDT_TAPE, 15, 0, 1, 0,
        "Permanent write protection", NULL},

    /* Device configuration extension mode subpage: dce [0x10,1 ] ssc3 */
    {"PE_UN", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 4, 7, 4, 0,
        "PEWS units", "Units: 0: MB, 1: GB, 2: TB"},
    {"TARPF", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 4, 3, 1, 0,
        "TapeAlert respect parameter fields", NULL},
    {"TASER", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 4, 2, 1, 0,
        "TapeAlert select except reporting", NULL},
    {"TARPC", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 4, 1, 1, 0,
        "TapeAlert respect page control", NULL},
    {"TAPLSD", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 4, 0, 1, 0,
        "TapeAlert prevent log sense deactivation", NULL},
    {"WR_MOD", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 5, 7, 4, 0,
        "Write mode",
        "0: overwrite allowed; 1: append only; 0xe,0xf: vendor specific"},
    {"SEM", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 5, 3, 4, 0,
        "Short erase mode",
        "0: as per SSC-2; 1: erase has no effect; 2: record EOD indication"},
    {"PEWS", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 6, 7, 16, 0,
        "Programmable early warning size",
        "size units depend on PE_UN field; 0: MB, 1: GB, 2: TB"},
    {"VCELBRE", DEV_CONF_MP, MSP_DEV_CONF_EXT, PDT_TAPE, 8, 0, 1, 0,
        "Volume containing encrypted logical blocks requires encryption",
        NULL},

    /* Medium partition mode page: mpa [0x11] ssc3 */
    {"MAX_AP", MED_PART_MP, 0, PDT_TAPE, 2, 7, 8, 0,
        "Maximum additional partitions", NULL},
    {"APD", MED_PART_MP, 0, PDT_TAPE, 3, 7, 8, 0,
        "Additional partitions defined", NULL},
    {"FDP", MED_PART_MP, 0, PDT_TAPE, 4, 7, 1, 0,
        "Fixed data partitions", NULL},
    {"SDP", MED_PART_MP, 0, PDT_TAPE, 4, 6, 1, 0,
        "Select data partitions", NULL},
    {"IDP", MED_PART_MP, 0, PDT_TAPE, 4, 5, 1, 0,
        "Initiator defined partitions", NULL},
    {"PSUM", MED_PART_MP, 0, PDT_TAPE, 4, 4, 2, 0,
        "Partition size unit of measure",
        "0: bytes; 1: kilobytes; 2: megabytes; 3: 10**(partition_units)"},
    {"POFM", MED_PART_MP, 0, PDT_TAPE, 4, 2, 1, 0,
        "Partition on format", NULL},
    {"CLEAR", MED_PART_MP, 0, PDT_TAPE, 4, 1, 1, 0,
        "Erase partition(s) (in concert with ADDP)", NULL},
    {"ADDP", MED_PART_MP, 0, PDT_TAPE, 4, 0, 1, 0,
        "Additional partition bit (in concert with CLEAR)", NULL},
    {"MFR", MED_PART_MP, 0, PDT_TAPE, 5, 7, 8, 0,
        "Medium format recognition",
        "0: incapable; 1: format recognition; 2: partition recognition\t"
        "3: format and partition recognition"},
    {"PART_T", MED_PART_MP, 0, PDT_TAPE, 6, 7, 4, 0,
        "Partition type", "0: vendor specific or unknown\t"
        "1: optimized for streaming\t"
        "2: reduces total native capacity\t"
        "0x3-0xe: capable of format and partition recognition\t"
        "0xf: multiple partition types"},
    {"PART_U", MED_PART_MP, 0, PDT_TAPE, 6, 3, 4, 0,
        "Partition units (exponent of 10, bytes)", NULL},
    /* "descriptor" starts here */
    {"P_SZ", MED_PART_MP, 0, PDT_TAPE, 8, 7, 16, 0,
        "Partition size", NULL},

    /* Enclosure services management mode page: esm [0x14] ses2 */
    {"ENBLTC", ES_MAN_MP, 0, PDT_SES, 5, 0, 1, MF_COMMON,
        "Enable timed completion", NULL},
    {"MTCT", ES_MAN_MP, 0, PDT_SES, 6, 7, 16, MF_COMMON,
        "Maximum task completion time (100ms)", NULL},

    /* Protocol specific logical unit control mode page: pl [0x18] spc3 */
    {"LUPID", PROT_SPEC_LU_MP, 0, -1, 2, 3, 4, 0,
        "Logical unit's (transport) protocol identifier",
        PROTO_IDENT_STR "\t"
        "[try adding '-t <transport>' to get more fields]"},

    /* Protocol specific port control mode page: pp [0x19] spc3 */
    {"PPID", PROT_SPEC_PORT_MP, 0, -1, 2, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR "\t"
        "[try adding '-t <transport>' to get more fields]"},

    /* Power condition mode page: po [0x1a] spc3 (expanded in spc4r18) */
    /* In sdparm v1.11 changed IDLE->IDLE_A; STANDBY->STANDBY_Z; */
    /* ICT->IACT and SCT->SZCT */
    {"PM_BG", POWER_MP, 0, -1, 2, 7, 2, 0,      /* added spc4r24 */
        "Power management, background functions, precedence",
        "0: vendor specific; 1: background function higher\t"
        "2: power management higher"},
    {"STANDBY_Y", POWER_MP, 0, -1, 2, 0, 1, 0,
        "Standby_y timer enable", NULL},
    {"IDLE_C", POWER_MP, 0, -1, 3, 3, 1, 0,
        "Idle_c timer enable", NULL},
    {"IDLE_B", POWER_MP, 0, -1, 3, 2, 1, 0,
        "Idle_b timer enable", NULL},
    {"IDLE_A", POWER_MP, 0, -1, 3, 1, 1, 0,
        "Idle_a timer enable", NULL},
    {"STANDBY_Z", POWER_MP, 0, -1, 3, 0, 1, 0,
        "Standby_z timer enable", NULL},
    {"IACT", POWER_MP, 0, -1, 4, 7, 32, 0,
        "Idle_a condition timer (100 ms)", NULL},
    {"SZCT", POWER_MP, 0, -1, 8, 7, 32, 0,
        "Standby_z condition timer (100 ms)", NULL},
    {"IBCT", POWER_MP, 0, -1, 12, 7, 32, 0,
        "Idle_b condition timer (100 ms)", NULL},
    {"ICCT", POWER_MP, 0, -1, 16, 7, 32, 0,
        "Idle_c condition timer (100 ms)", NULL},
    {"SYCT", POWER_MP, 0, -1, 20, 7, 32, 0,
        "Standby_y condition timer (100 ms)", NULL},
    /* The "0: restricted (SAS-2)" became obsolete in spc5r01 */
    {"CCF_IDLE", POWER_MP, 0, -1, 39, 7, 2, 0,     /* changed spc4r35 */
        "check condition if from idle_c",          /* was FIDCPC (spc4r25) */
        "0: restricted (SAS-2); 1: disabled; 2: enabled"},
    {"CCF_STAND", POWER_MP, 0, -1, 39, 5, 2, 0,    /* changed spc4r35 */
        "check condition if from a standby",         /* was FSBCPC */
        "0: restricted (SAS-2); 1: disabled; 2: enabled"},
    {"CCF_STOPP", POWER_MP, 0, -1, 39, 3, 2, 0,    /* changed spc4r35 */
        "check condition if from stopped",         /* was FSTCPC */
        "0: restricted (SAS-2); 1: disabled; 2: enabled"},

    /* Power consumption mode page: ps [0x1a,1] added spc4r33 */
    {"ACT_LEV", POWER_MP, MSP_SPC_PS, -1, 6, 1, 2, 0,
        "Active level",
        "0: per PC_ID field; 1: highest; 2: intermediate; 3: lowest"},
    {"PC_ID", POWER_MP, MSP_SPC_PS, -1, 7, 7, 8, 0,
        "Power consumption identifier",
        "references Power consumption VPD page"},

    /* SAT ATA Power condition mode page: apo [0x1a,0xf1] sat2 */
    {"APMP", POWER_MP, MSP_SAT_POWER, -1, 5, 0, 1, 0,
        "Advanced Power Management (APM) enabled/change", NULL},
    {"APM", POWER_MP, MSP_SAT_POWER, -1, 6, 7, 8, 0,
        "Advanced Power Management (APM) value",
        "0: disable APM feature set; >0: enable"},

    /* Informational exception control mode page: ie [0x1c] sbc */
    {"PERF", IEC_MP, 0, -1, 2, 7, 1, 0,
        "Performance (impact of ie operations)",
        "0: normal (some delays); 1: abridge ie operations"},
    {"EBF", IEC_MP, 0, -1, 2, 5, 1, 0,
        "Enable background function", NULL},
    {"EWASC", IEC_MP, 0, -1, 2, 4, 1, MF_COMMON,
        "Enable warning", NULL},
    {"DEXCPT", IEC_MP, 0, -1, 2, 3, 1, MF_COMMON,
        "Disable exceptions", NULL},
    {"TEST", IEC_MP, 0, -1, 2, 2, 1, 0,
        "Test (simulate device failure)", NULL},
    {"EBACKERR", IEC_MP, 0, -1, 2, 1, 1, 0,
        "Enable background (scan + self test) error reporting", NULL},
    {"LOGERR", IEC_MP, 0, -1, 2, 0, 1, 0,
        "Log informational exception errors", NULL},
    {"MRIE", IEC_MP, 0, -1, 3, 3, 4, MF_COMMON,
        "Method of reporting informational exceptions",
        "0: no reporting; 1: async reporting (obs); 2: unit attention\t"
        "3: conditional recovered error; 4: recovered error\t"
        "5: check condition with no sense; 6: request sense only"},
    {"INTT", IEC_MP, 0, -1, 4, 7, 32, 0,
        "Interval timer (100 ms)", NULL},
    {"REPC", IEC_MP, 0, -1, 8, 7, 32, 0,
        "Report count (or Test flag number [SSC-3])", NULL},

    /* Background control mode subpage: bc [0x1c,0x1] sbc3 */
    {"S_L_FULL", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 4, 2, 1, 0,
        "Suspend on log full", NULL},
    {"LOWIR", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 4, 1, 1, 0,
        "Log only when intervention required", NULL},
    {"EN_BMS", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 4, 0, 1, 0,
        "Enable background medium scan", NULL},
    {"EN_PS", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 5, 0, 1, 0,
        "Enable pre-scan", NULL},
    {"BMS_I", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 6, 7, 16, 0,
        "Background medium scan interval time (hour)", NULL},
    {"BPS_TL", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 8, 7, 16, 0,
        "Background pre-scan time limit (hour)", "0: no limit"},
    {"MIN_IDLE", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 10, 7, 16, 0,
        "Minumum idle time before background scan (ms)", NULL},
    {"MAX_SUSP", IEC_MP, MSP_BACK_CTL, PDT_DISK_ZBC, 12, 7, 16, 0,
        "Maximum time to suspend background scan (ms)", NULL},

    /* Logical block provisioning mode subpage: lbp [0x1c,0x2] sbc3 */
    {"SITUA", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 4, 0, 1, 0,
        "Single initiator threshold unit attention", NULL},
    /* descriptor starts here, the <start_byte> is relative to the start
     * of the mode page (i.e. 16 more than t10's descriptor format table) */
    {"LBP_EN", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 16, 7, 1, 0,
        "Threshold enabled", NULL},
    {"LBP_TYPE", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 16, 5, 3, 0,
        "Threshold type", "0: soft threshold count\t"
        "1: threshold count is a percentage"},
    {"LBP_ARM", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 16, 2, 3, 0,
        "Threshold arming", NULL},
    {"LBP_RES", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 17, 7, 8, 0,
        "Threshold resource", NULL},
    {"LBP_COUNT", IEC_MP, MSP_SBC_LB_PROV, PDT_DISK, 20, 7, 32, 0,
        "Threshold count", NULL},

    /* Medium configuration mode page: mco [0x1d] ssc3 */
    {"WORMM", MED_CONF_MP, 0, PDT_TAPE, 2, 0, 1, 0,
        "Worm mode", NULL},
    {"WMLR", MED_CONF_MP, 0, PDT_TAPE, 4, 7, 8, 0,
        "Worm volume label restrictions",       /* mode->volume renaming */
        "0: disallow overwrite\t1: disallow some format labels overwrite\t"
        "2: allow all format labels to be overwritten"},
    {"WMFR", MED_CONF_MP, 0, PDT_TAPE, 5, 7, 8, 0,
        "Worm volume filemark restrictions",    /* mode->volume renaming */
        "2: allow filemarks before EOD except closest to BOP\t"
        "3: allow any number of filemarks before EOD"},

    /* Timeout and protect mode page: tp [0x1d] mmc5 */
    {"G3E", TIMEOUT_PROT_MP, 0, PDT_MMC, 4, 3, 1, 0,
        "Group 3 timeout capability enable", NULL},
    {"TMOE", TIMEOUT_PROT_MP, 0, PDT_MMC, 4, 2, 1, 0,
        "Timeout enable", NULL},
    {"DISP", TIMEOUT_PROT_MP, 0, PDT_MMC, 4, 1, 1, 0,
        "Disable (unavailable) until power cycle", NULL},
    {"SWPP", TIMEOUT_PROT_MP, 0, PDT_MMC, 4, 0, 1, 0,
        "Software write protect until power cycle", NULL},
    {"G1MT", TIMEOUT_PROT_MP, 0, PDT_MMC, 6, 7, 16, 0,
        "Group 1 minimum timeout (sec)", NULL},
    {"G2MT", TIMEOUT_PROT_MP, 0, PDT_MMC, 8, 7, 16, 0,
        "Group 2 minimum timeout (sec)", NULL},


    /* Element address assignment mode page: eaa [0x1d] smc2 */
    {"FMTEA", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 2, 7, 16, 0,
        "First medium transport element address", NULL},
    {"NMTE", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 4, 7, 16, 0,
        "Number of medium transport elements", NULL},
    {"FSEA", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 6, 7, 16, 0,
        "First storage element address", NULL},
    {"NSE", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 8, 7, 16, 0,
        "Number of storage elements", NULL},
    {"FIEEA", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 10, 7, 16, 0,
        "First import/export element address", NULL},
    {"NIEE", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 12, 7, 16, 0,
        "Number of import/export elements", NULL},
    {"FDTEA", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 14, 7, 16, 0,
        "First data transfer element address", NULL},
    {"NDTE", ELE_ADDR_ASS_MP, 0, PDT_MCHANGER, 16, 7, 16, 0,
        "Number of data transfer elements", NULL},

    /* Transport geometry parameters mode page: tgp [0x1e] smc2 */
    /* transport geometry descriptor starts here, <start_byte> is relative
     * to start of mode page (i.e. 2 more than shown in t10's descriptor
     * table */
    {"ROTAT", TRANS_GEO_PAR_MP, 0, PDT_MCHANGER, 2, 0, 1, 0,
        "Rotation for double sided media handling", NULL},
    {"MNTES", TRANS_GEO_PAR_MP, 0, PDT_MCHANGER, 3, 7, 8, 0,
        "Member number in transport element set", NULL},

    /* Device capabilities mode page: dca [0x1f] smc3 */
    {"STORDT", DEV_CAP_MP, 0, PDT_MCHANGER, 2, 3, 1, 0,
        "Storage for data transfer element", NULL},
    {"STORIE", DEV_CAP_MP, 0, PDT_MCHANGER, 2, 2, 1, 0,
        "Storage for import/export element", NULL},
    {"STORST", DEV_CAP_MP, 0, PDT_MCHANGER, 2, 1, 1, 0,
        "Storage for storage element", NULL},
    {"STORMT", DEV_CAP_MP, 0, PDT_MCHANGER, 2, 0, 1, 0,
        "Storage for medium transport element", NULL},
    {"ACE", DEV_CAP_MP, 0, PDT_MCHANGER, 3, 2, 1, 0,
        "Auto clean enabled", NULL},
    {"VTRP", DEV_CAP_MP, 0, PDT_MCHANGER, 3, 1, 1, 0,
        "Volume tag reader present", NULL},
    {"S2C", DEV_CAP_MP, 0, PDT_MCHANGER, 3, 0, 1, 0,
        "SMC-2 capabilities supported", NULL},
    {"MT_RA", DEV_CAP_MP, 0, PDT_MCHANGER, 4, 7, 2, 0,
        "Medium transport elements support Read Attribute", NULL},
    {"MT2DT", DEV_CAP_MP, 0, PDT_MCHANGER, 4, 3, 1, 0,
        "Medium transport -> data transfer; Move Medium", NULL},
    {"MT2IE", DEV_CAP_MP, 0, PDT_MCHANGER, 4, 2, 1, 0,
        "Medium transport -> import/export; Move Medium", NULL},
    {"MT2ST", DEV_CAP_MP, 0, PDT_MCHANGER, 4, 1, 1, 0,
        "Medium transport -> storage; Move Medium", NULL},
    {"MT2MT", DEV_CAP_MP, 0, PDT_MCHANGER, 4, 0, 1, 0,
        "Medium transport -> medium transport; Move Medium", NULL},
    {"ST_RA", DEV_CAP_MP, 0, PDT_MCHANGER, 5, 7, 2, 0,
        "Storage elements support Read Attribute", NULL},
    {"ST2DT", DEV_CAP_MP, 0, PDT_MCHANGER, 5, 3, 1, 0,
        "Storage -> data transfer; Move Medium", NULL},
    {"ST2IE", DEV_CAP_MP, 0, PDT_MCHANGER, 5, 2, 1, 0,
        "Storage -> import/export; Move Medium", NULL},
    {"ST2ST", DEV_CAP_MP, 0, PDT_MCHANGER, 5, 1, 1, 0,
        "Storage -> storage; Move Medium", NULL},
    {"ST2MT", DEV_CAP_MP, 0, PDT_MCHANGER, 5, 0, 1, 0,
        "Storage -> medium transport; Move Medium", NULL},
    {"IE_RA", DEV_CAP_MP, 0, PDT_MCHANGER, 6, 7, 2, 0,
        "Import/export elements support Read Attribute", NULL},
    {"IE2DT", DEV_CAP_MP, 0, PDT_MCHANGER, 6, 3, 1, 0,
        "Import/export -> data transfer; Move Medium", NULL},
    {"IE2IE", DEV_CAP_MP, 0, PDT_MCHANGER, 6, 2, 1, 0,
        "Import/export -> import/export; Move Medium", NULL},
    {"IE2ST", DEV_CAP_MP, 0, PDT_MCHANGER, 6, 1, 1, 0,
        "Import/export -> storage; Move Medium", NULL},
    {"IE2MT", DEV_CAP_MP, 0, PDT_MCHANGER, 6, 0, 1, 0,
        "Import/export -> medium transport; Move Medium", NULL},
    {"DT_RA", DEV_CAP_MP, 0, PDT_MCHANGER, 7, 7, 2, 0,
        "Data transfer elements support Read Attribute", NULL},
    {"DT2DT", DEV_CAP_MP, 0, PDT_MCHANGER, 7, 3, 1, 0,
        "Data transfer -> data transfer; Move Medium", NULL},
    {"DT2IE", DEV_CAP_MP, 0, PDT_MCHANGER, 7, 2, 1, 0,
        "Data transfer -> import/export; Move Medium", NULL},
    {"DT2ST", DEV_CAP_MP, 0, PDT_MCHANGER, 7, 1, 1, 0,
        "Data transfer -> storage; Move Medium", NULL},
    {"DT2MT", DEV_CAP_MP, 0, PDT_MCHANGER, 7, 0, 1, 0,
        "Data transfer -> medium transport; Move Medium", NULL},
    {"MT_WA", DEV_CAP_MP, 0, PDT_MCHANGER, 12, 7, 2, 0,
        "Medium transport elements support Write Attribute", NULL},
    {"MTEDT", DEV_CAP_MP, 0, PDT_MCHANGER, 12, 3, 1, 0,
        "Medium transport -> data transfer; Exchange Medium", NULL},
    {"MTEIE", DEV_CAP_MP, 0, PDT_MCHANGER, 12, 2, 1, 0,
        "Medium transport -> import/export; Exchange Medium", NULL},
    {"MTEST", DEV_CAP_MP, 0, PDT_MCHANGER, 12, 1, 1, 0,
        "Medium transport -> storage; Exchange Medium", NULL},
    {"MTEMT", DEV_CAP_MP, 0, PDT_MCHANGER, 12, 0, 1, 0,
        "Medium transport -> medium transport; Exchange Medium", NULL},
    {"ST_WA", DEV_CAP_MP, 0, PDT_MCHANGER, 13, 7, 2, 0,
        "Storage elements support Write Attribute", NULL},
    {"STEDT", DEV_CAP_MP, 0, PDT_MCHANGER, 13, 3, 1, 0,
        "Storage -> data transfer; Exchange Medium", NULL},
    {"STEIE", DEV_CAP_MP, 0, PDT_MCHANGER, 13, 2, 1, 0,
        "Storage -> import/export; Exchange Medium", NULL},
    {"STEST", DEV_CAP_MP, 0, PDT_MCHANGER, 13, 1, 1, 0,
        "Storage -> storage; Exchange Medium", NULL},
    {"STEMT", DEV_CAP_MP, 0, PDT_MCHANGER, 13, 0, 1, 0,
        "Storage -> medium transport; Exchange Medium", NULL},
    {"IE_WA", DEV_CAP_MP, 0, PDT_MCHANGER, 14, 7, 2, 0,
        "Import/export elements support Write Attribute", NULL},
    {"IEEDT", DEV_CAP_MP, 0, PDT_MCHANGER, 14, 3, 1, 0,
        "Import/export -> data transfer; Exchange Medium", NULL},
    {"IEEIE", DEV_CAP_MP, 0, PDT_MCHANGER, 14, 2, 1, 0,
        "Import/export -> import/export; Exchange Medium", NULL},
    {"IEEST", DEV_CAP_MP, 0, PDT_MCHANGER, 14, 1, 1, 0,
        "Import/export -> storage; Exchange Medium", NULL},
    {"IEEMT", DEV_CAP_MP, 0, PDT_MCHANGER, 14, 0, 1, 0,
        "Import/export -> medium transport; Exchange Medium", NULL},
    {"DT_WA", DEV_CAP_MP, 0, PDT_MCHANGER, 15, 7, 2, 0,
        "Data transfer elements support Write Attribute", NULL},
    {"DTEDT", DEV_CAP_MP, 0, PDT_MCHANGER, 15, 3, 1, 0,
        "Data transfer -> data transfer; Exchange Medium", NULL},
    {"DTEIE", DEV_CAP_MP, 0, PDT_MCHANGER, 15, 2, 1, 0,
        "Data transfer -> import/export; Exchange Medium", NULL},
    {"DTEST", DEV_CAP_MP, 0, PDT_MCHANGER, 15, 1, 1, 0,
        "Data transfer -> storage; Exchange Medium", NULL},
    {"DTEMT", DEV_CAP_MP, 0, PDT_MCHANGER, 15, 0, 1, 0,
        "Data transfer -> medium transport; Exchange Medium", NULL},

    /* Extended device capabilities mode page: edc [0x1f,0x41] smc3 */
    {"MVPRV", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 5, 1, 0,
        "Move prevented to import/export element", NULL},
    {"MVCL", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 4, 1, 0,
        "Move closes import/export element", NULL},
    {"MVOP", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 3, 1, 0,
        "Move opens import/export element", NULL},
    {"USRCL", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 2, 1, 0,
        "User control import/export element close", NULL},
    {"USROP", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 1, 1, 0,
        "User control import/export element open", NULL},
    {"IEST", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 4, 0, 1, 0,
        "Import/export element state", NULL},
    {"DTETA", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 5, 4, 1, 0,
        "Data transfer element empty on door access", NULL},
    {"RSSEA", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 5, 3, 1, 0,
        "Return to source storage element address", NULL},
    {"MVTRY", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 5, 2, 1, 0,
        "Move tray", NULL},
    {"IEMGZ", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 5, 1, 1, 0,
        "Import/export magazine", NULL},
    {"SMGZ", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 5, 0, 1, 0,
        "Storage magazine", NULL},
    {"TREXC", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 6, 2, 1, 0,
        "True exchange capable", NULL},
    {"LCKIE", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 6, 1, 1, 0,
        "Lock import/export element", NULL},
    {"LCKD", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 6, 0, 1, 0,
        "Lock door", NULL},
    {"SPMER", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 7, 2, 1, 0,
        "Source pre-move eject required", NULL},
    {"DPMER", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 7, 1, 1, 0,
        "Destination pre-move eject required", NULL},
    {"PEPOS", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 7, 0, 1, 0,
        "Pre-eject position", NULL},
    {"UCST", DEV_CAP_MP, MSP_EXT_DEV_CAP, PDT_MCHANGER, 8, 0, 1, 0,
        "Unassigned cleaning storage", NULL},

    /* CD/DVD (MM) capabilities and mechanical status mode page: cms */
    /* [0x2a] obsolete in mmc4 and mmc5, last valid in mmc3 */
    /* MRSS field was already obsolete in mmc3 */
    {"D_RAM_R", MMCMS_MP, 0, PDT_MMC, 2, 5, 1, 0,
        "DVD-RAM read", NULL},
    {"D_R_R", MMCMS_MP, 0, PDT_MMC, 2, 4, 1, 0,
        "DVD-R read", NULL},
    {"D_ROM_R", MMCMS_MP, 0, PDT_MMC, 2, 3, 1, 0,
        "DVD-ROM read", NULL},
    {"METH2", MMCMS_MP, 0, PDT_MMC, 2, 2, 1, 0,
        "Method 2", NULL},
    {"CD_RW_R", MMCMS_MP, 0, PDT_MMC, 2, 1, 1, 0,
        "CD-RW read", NULL},
    {"CD_R_R", MMCMS_MP, 0, PDT_MMC, 2, 0, 1, 0,
        "CD-R read", NULL},
    {"D_RAM_W", MMCMS_MP, 0, PDT_MMC, 3, 5, 1, 0,
        "DVD-RAM write", NULL},
    {"D_R_W", MMCMS_MP, 0, PDT_MMC, 3, 4, 1, 0,
        "DVD-R write", NULL},     /* was D_R_R, wrong, clashed with above */
    {"TST_WR", MMCMS_MP, 0, PDT_MMC, 3, 2, 1, 0,
        "Test write", NULL},      /* was TST_W but clashed with page 0x5 */
    {"CD_RW_W", MMCMS_MP, 0, PDT_MMC, 3, 1, 1, 0,
        "CD-RW write", NULL},
    {"CD_R_W", MMCMS_MP, 0, PDT_MMC, 3, 0, 1, 0,
        "CD-R write", NULL},
    {"BUF", MMCMS_MP, 0, PDT_MMC, 4, 7, 1, 0,
        "Buffer underrun free recording", NULL},
    {"MULT_S", MMCMS_MP, 0, PDT_MMC, 4, 6, 1, 0,
        "Multi session", NULL},   /* was MULTI_S but clashed with page 0x5 */
    {"M2F2", MMCMS_MP, 0, PDT_MMC, 4, 5, 1, 0,
        "Mode 2 form 2", NULL},
    {"M2F1", MMCMS_MP, 0, PDT_MMC, 4, 4, 1, 0,
        "Mode 2 form 1", NULL},
    {"DP_2", MMCMS_MP, 0, PDT_MMC, 4, 3, 1, 0,
        "Digital port 2", NULL},
    {"DP_1", MMCMS_MP, 0, PDT_MMC, 4, 2, 1, 0,
        "Digital port 1", NULL},
    {"COMP", MMCMS_MP, 0, PDT_MMC, 4, 1, 1, 0,
        "Composite", NULL},
    {"AUDIO_P", MMCMS_MP, 0, PDT_MMC, 4, 0, 1, 0,
        "Audio play", NULL},
    {"RBC", MMCMS_MP, 0, PDT_MMC, 5, 7, 1, 0,
        "Read bar code", NULL},
    {"UPC", MMCMS_MP, 0, PDT_MMC, 5, 6, 1, 0,
        "Uniform product code", NULL},
    {"ISRC", MMCMS_MP, 0, PDT_MMC, 5, 5, 1, 0,
        "International standard recording code", NULL},
    {"C2PS", MMCMS_MP, 0, PDT_MMC, 5, 4, 1, 0,
        "C 2 pointers supported", NULL},
    {"RW_DC", MMCMS_MP, 0, PDT_MMC, 5, 3, 1, 0,
        "R-W de-interleaved and corrected", NULL},
    {"RW_S", MMCMS_MP, 0, PDT_MMC, 5, 2, 1, 0,
        "R-W supported", NULL},
    {"CDDA_SA", MMCMS_MP, 0, PDT_MMC, 5, 1, 1, 0,
        "CD-DA stream accurate", NULL},
    {"CDDA_CS", MMCMS_MP, 0, PDT_MMC, 5, 0, 1, 0,
        "CD-DA commands supported", NULL},
    {"LMT", MMCMS_MP, 0, PDT_MMC, 6, 7, 3, 0,
        "Loading mechanism type", NULL},
    {"EJECT", MMCMS_MP, 0, PDT_MMC, 6, 3, 1, 0,
        "Eject (individual or magazine)", NULL},
    {"PJ", MMCMS_MP, 0, PDT_MMC, 6, 2, 1, 0,
        "Prevent jumper", NULL},
    {"LS", MMCMS_MP, 0, PDT_MMC, 6, 1, 1, 0,
        "Lock state", NULL},
    {"LOCK", MMCMS_MP, 0, PDT_MMC, 6, 0, 1, 0,
        "Lock (supported)", NULL},
    {"RWILI", MMCMS_MP, 0, PDT_MMC, 7, 5, 1, 0,
        "R-W in lead in", NULL},
    {"SCC", MMCMS_MP, 0, PDT_MMC, 7, 4, 1, 0,
        "Side change capable", NULL},
    {"SSS", MMCMS_MP, 0, PDT_MMC, 7, 3, 1, 0,
        "Software slot selection", NULL},
    {"CSDP", MMCMS_MP, 0, PDT_MMC, 7, 2, 1, 0,
        "Changer supports disc present", NULL},
    {"SCM", MMCMS_MP, 0, PDT_MMC, 7, 1, 1, 0,
        "Separate channel mute", NULL},
    {"SVL", MMCMS_MP, 0, PDT_MMC, 7, 0, 1, 0,
        "Separate volume levels", NULL},
    {"MRSS", MMCMS_MP, 0, PDT_MMC, 8, 7, 16, 0,
        "Maximum read speed supported (kBps) (obs)", NULL},
    {"NVLS", MMCMS_MP, 0, PDT_MMC, 10, 7, 16, 0,
        "Number of volume levels supported", NULL},
    {"BSS", MMCMS_MP, 0, PDT_MMC, 12, 7, 16, 0,
        "Buffer size supported (1024 bytes)", NULL},
    {"LENGTH", MMCMS_MP, 0, PDT_MMC, 17, 5, 2, 0,
        "Length (bit length of IEC958 words)", NULL},
    {"LSBF", MMCMS_MP, 0, PDT_MMC, 17, 3, 1, 0,
        "LSB (least significant bit) first", NULL},
    {"RCK", MMCMS_MP, 0, PDT_MMC, 17, 2, 1, 0,
        "High on LRCK indicates left channel", NULL},
    {"BCKF", MMCMS_MP, 0, PDT_MMC, 17, 1, 1, 0,
        "BCK signal falling edge", NULL},
    {"CMRS", MMCMS_MP, 0, PDT_MMC, 22, 7, 16, 0,
        "Copy management revision supported", NULL},
    {"RCS", MMCMS_MP, 0, PDT_MMC, 27, 1, 2, 0,
        "Rotation control selected", NULL},
    {"CWSS", MMCMS_MP, 0, PDT_MMC, 28, 7, 16, 0,
        "Current write speed selected", NULL},

    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, NULL},
};


/* << Transport protocol specific mode page items follow >> */

static struct sdparm_mode_page_item sdparm_mitem_fcp_arr[] = {
    /* disconnect-reconnect mode page [0x2] fcp3-5 */
    {"BFR", DISCONNECT_MP, 0, -1, 2, 7, 8, 0,   /* obs fcp-5 */
        "Buffer full ratio", NULL},
    {"BER", DISCONNECT_MP, 0, -1, 3, 7, 8, 0,   /* obs fcp-5 */
        "Buffer empty ratio", NULL},
    {"BIL", DISCONNECT_MP, 0, -1, 4, 7, 16, MF_COMMON,  /* obs fcp-5 */
        "Bus inactivity limit (transmission words)", NULL},
    {"DTL", DISCONNECT_MP, 0, -1, 6, 7, 16, MF_COMMON,  /* obs fcp-5 */
        "Disconnect time limit (128 transmission words)", NULL},
    {"CTL", DISCONNECT_MP, 0, -1, 8, 7, 16, MF_COMMON,  /* obs fcp-5 */
        "Connect time limit (128 transmission words)", NULL},
    {"MBS", DISCONNECT_MP, 0, -1, 10, 7, 16, MF_COMMON | MF_CLASH_OK,
        "Maximum burst size (512 bytes)", NULL},
    {"EMDP", DISCONNECT_MP, 0, -1, 12, 7, 1, MF_CLASH_OK,
        "Enable modify data pointers", NULL},
    {"FAA", DISCONNECT_MP, 0, -1, 12, 6, 1, 0,  /* obs fcp-5 */
        "Fairness access A [FCP_DATA]", NULL},
    {"FAB", DISCONNECT_MP, 0, -1, 12, 5, 1, 0,  /* obs fcp-5 */
        "Fairness access B [FCP_XFER]", NULL},
    {"FAC", DISCONNECT_MP, 0, -1, 12, 4, 1, 0,  /* obs fcp-5 */
        "Fairness access C [FCP_RSP]", NULL},
    {"FBS", DISCONNECT_MP, 0, -1, 14, 7, 16, MF_CLASH_OK,
        "First burst size (512 bytes)", "0: no limit"},

    /* protocol specific logical unit mode page [0x18] fcp3-5 */
    {"LUPID", PROT_SPEC_LU_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Logical unit's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"EPDC", PROT_SPEC_LU_MP, 0, -1, 3, 0, 1, MF_COMMON,
        "Enable precise delivery checking", NULL},

    /* protocol specific port control page [0x19] fcp3-5 */
    {"PPID", PROT_SPEC_PORT_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"DTFD", PROT_SPEC_PORT_MP, 0, -1, 3, 7, 1, MF_COMMON,  /* obs fcp-5 */
        "Disable target fabric discovery", NULL},
    {"PLPB", PROT_SPEC_PORT_MP, 0, -1, 3, 6, 1, MF_COMMON,  /* obs fcp-5 */
        "Prevent loop port bypass", NULL},
    {"DDIS", PROT_SPEC_PORT_MP, 0, -1, 3, 5, 1, 0,      /* obs fcp-5 */
        "Disable discovery", NULL},
    {"DLM", PROT_SPEC_PORT_MP, 0, -1, 3, 4, 1, 0,       /* obs fcp-5 */
        "Disable loop master", NULL},
    {"RHA", PROT_SPEC_PORT_MP, 0, -1, 3, 3, 1, 0,       /* obs fcp-5 */
        "Require hard address", NULL},
    {"ALWI", PROT_SPEC_PORT_MP, 0, -1, 3, 2, 1, 0,      /* obs fcp-5 */
        "Allow login without loop initialization", NULL},
    {"DTIPE", PROT_SPEC_PORT_MP, 0, -1, 3, 1, 1, 0,     /* obs fcp-5 */
        "Disable target initialized port enable", NULL},
    {"DTOLI", PROT_SPEC_PORT_MP, 0, -1, 3, 0, 1, 0,     /* obs fcp-5 */
        "Disable target originated loop initialization", NULL},
    {"RRTVU", PROT_SPEC_PORT_MP, 0, -1, 6, 2, 3, MF_CLASH_OK,
        "Resource recovery timeout value unit", NULL},
    {"RR_TOV", PROT_SPEC_PORT_MP, 0, -1, 6, 2, 3, MF_CLASH_OK,
        "Resource recovery timeout value unit", NULL},
    {"SIRRTV", PROT_SPEC_PORT_MP, 0, -1, 7, 7, 8, 0,
        "Sequence initiative resource recovery timeout value", NULL},

    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, NULL},
};

/* SPI == SCSI Parallel Interface (legacy) */
static struct sdparm_mode_page_item sdparm_mitem_spi_arr[] = {
    /* disconnect-reconnect mode page [0x2] spi4 */
    {"BFR", DISCONNECT_MP, 0, -1, 2, 7, 8, 0,
        "Buffer full ratio", NULL},
    {"BER", DISCONNECT_MP, 0, -1, 3, 7, 8, 0,
        "Buffer empty ratio", NULL},
    {"BIL", DISCONNECT_MP, 0, -1, 4, 7, 16, MF_COMMON,
        "Bus inactivity limit (100 us)", NULL},
    {"PDTL", DISCONNECT_MP, 0, -1, 6, 7, 16, MF_COMMON,
        "Physical disconnect time limit (100 us)", NULL},
    {"CTL", DISCONNECT_MP, 0, -1, 8, 7, 16, MF_COMMON,
        "Connect time limit (100 us)", NULL},
    {"MBS", DISCONNECT_MP, 0, -1, 10, 7, 16, MF_COMMON | MF_CLASH_OK,
        "Maximum burst size (512 bytes)", NULL},
    {"EMDP", DISCONNECT_MP, 0, -1, 12, 7, 1, MF_CLASH_OK,
        "Enable modify data pointers", NULL},
    {"FA", DISCONNECT_MP, 0, -1, 12, 6, 3, 0,
        "Fair arbitration", NULL},
    {"DIMM", DISCONNECT_MP, 0, -1, 12, 3, 1, 0,
        "Disconnect immediate", NULL},
    {"DTDC", DISCONNECT_MP, 0, -1, 12, 2, 3, 0,
        "Data transfer disconnect control", NULL},

    /* protocol specific logical unit control mode page [0x18] spi4 */
    {"LUPID", PROT_SPEC_LU_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Logical unit's (transport) protocol identifier",
        PROTO_IDENT_STR },

    /* protocol specific port control page [0x19] spi4 */
    {"PPID", PROT_SPEC_PORT_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"STT", PROT_SPEC_PORT_MP, 0, -1, 4, 7, 16, MF_COMMON,
        "Synchronous transfer timeout (ms)", NULL},

    /* margin control subpage  [0x19,0x1] spi4 */
    {"PPID_1", PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"DS", PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 7, 7, 4, 0,
        "Driver strength", NULL},
    {"DA", PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 8, 7, 4, 0,
        "Driver asymmetry", NULL},
    {"DP", PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 8, 3, 4, 0,
        "Driver precompensation", NULL},
    {"DSR", PROT_SPEC_PORT_MP, MSP_SPI_MC, -1, 9, 7, 4, 0,
        "Driver slew rate", NULL},

    /* saved training configuration subpage [0x19,0x2] spi4 */
    {"PPID_2", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"DB0", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 10, 7, 32, MF_HEX,
        "DB(0) value", NULL},
    {"DB1", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 14, 7, 32, MF_HEX,
        "DB(1) value", NULL},
    {"DB2", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 18, 7, 32, MF_HEX,
        "DB(2) value", NULL},
    {"DB3", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 22, 7, 32, MF_HEX,
        "DB(3) value", NULL},
    {"DB4", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 26, 7, 32, MF_HEX,
        "DB(4) value", NULL},
    {"DB5", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 30, 7, 32, MF_HEX,
        "DB(5) value", NULL},
    {"DB6", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 34, 7, 32, MF_HEX,
        "DB(6) value", NULL},
    {"DB7", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 38, 7, 32, MF_HEX,
        "DB(7) value", NULL},
    {"DB8", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 42, 7, 32, MF_HEX,
        "DB(8) value", NULL},
    {"DB9", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 46, 7, 32, MF_HEX,
        "DB(9) value", NULL},
    {"DB10", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 50, 7, 32, MF_HEX,
        "DB(10) value", NULL},
    {"DB11", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 54, 7, 32, MF_HEX,
        "DB(11) value", NULL},
    {"DB12", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 58, 7, 32, MF_HEX,
        "DB(12) value", NULL},
    {"DB13", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 62, 7, 32, MF_HEX,
        "DB(13) value", NULL},
    {"DB14", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 66, 7, 32, MF_HEX,
        "DB(14) value", NULL},
    {"DB15", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 70, 7, 32, MF_HEX,
        "DB(15) value", NULL},
    {"P_CRCA", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 74, 7, 32, MF_HEX,
        "P_CRCA value", NULL},
    {"P1", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 78, 7, 32, MF_HEX,
        "P1 value", NULL},
    {"BSY", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 82, 7, 32, MF_HEX,
        "BSY value", NULL},
    {"SEL", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 86, 7, 32, MF_HEX,
        "SEL value", NULL},
    {"RST", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 90, 7, 32, MF_HEX,
        "RST value", NULL},
    {"REQ", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 94, 7, 32, MF_HEX,
        "REQ value", NULL},
    {"ACK", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 98, 7, 32, MF_HEX,
        "ACK value", NULL},
    {"ATN", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 102, 7, 32, MF_HEX,
        "ATN value", NULL},
    {"C_D", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 106, 7, 32, MF_HEX,
        "C/D value", NULL},
    {"I_O", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 110, 7, 32, MF_HEX,
        "I/O value", NULL},
    {"MSG", PROT_SPEC_PORT_MP, MSP_SPI_STC, -1, 114, 7, 32, MF_HEX,
        "MSG value", NULL},

    /* negotiated settings subpage [0x19,0x3] spi4 */
    {"PPID_3", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"TPF", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 6, 7, 8, 0,
        "Transfer period factor", NULL},
    {"RAO", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 8, 7, 8, 0,
        "REQ/ACK offset", NULL},
    {"TWE", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 9, 7, 8, 0,
        "Transfer width exponent", NULL},
    {"POB", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 10, 6, 7, 0,
        "Protocol option bits", NULL},
    {"TM", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 11, 3, 2, 0,
        "Transceiver mode", NULL},
    {"SPE", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 11, 1, 1, 0,
        "Sent PCOMP_EN bit (for current I_T nexus)", NULL},
    {"RPE", PROT_SPEC_PORT_MP, MSP_SPI_NS, -1, 11, 0, 1, 0,
        "Received PCOMP_EN bit (for current I_T nexus)", NULL},

    /* report transfer capabilities subpage [0x19,0x4] spi4 */
    {"PPID_4", PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"MTPF", PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 6, 7, 8, 0,
        "Minimum transfer period factor", NULL},
    {"MRAO", PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 8, 7, 8, 0,
        "Maximum REQ/ACK offset", NULL},
    {"MTWE", PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 9, 7, 8, 0,
        "Maximum transfer width exponent", NULL},
    {"POBS", PROT_SPEC_PORT_MP, MSP_SPI_RTC, -1, 10, 7, 8, 0,
        "Protocol option bits supported", NULL},

    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, NULL},
};

/* SRP == SCSI RDMA protocol */
static struct sdparm_mode_page_item sdparm_mitem_srp_arr[] = {
    /* disconnect-reconnect mode page [0x2] srp */
    {"MBS", DISCONNECT_MP, 0, -1, 10, 7, 16, MF_COMMON | MF_CLASH_OK,
        "Maximum burst size (512 bytes)", NULL},
    {"EMDP", DISCONNECT_MP, 0, -1, 12, 7, 1, MF_CLASH_OK,
        "Enable modify data pointers", NULL},
    {"FBS", DISCONNECT_MP, 0, -1, 14, 7, 16, MF_CLASH_OK,
        "First burst size (512 bytes)", NULL},  /* srp2r00 */

    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, NULL},
};

/* SAS == Serial Attached SCSI */
static struct sdparm_mode_page_item sdparm_mitem_sas_arr[] = {
    /* disconnect-reconnect mode page [0x2] sas/spl */
    /* spl3r6 dropped the "time" from the name of BITL, keep acronym */
    {"BITL", DISCONNECT_MP, 0, -1, 4, 7, 16, MF_COMMON,
        "Bus inactivity (time) limit (100us or see BILUNIT)",
        "0: no bus inactivity time limit\t"
        "1-65535: limit in units of 100 us"},
    {"MCTL", DISCONNECT_MP, 0, -1, 8, 7, 16, MF_COMMON,
        "Connect time limit (100us or see CTLUNIT)",
        "0: no maximum connection time limit\t"
        "1-65535: limit in units of 100 us"},
    {"MBS", DISCONNECT_MP, 0, -1, 10, 7, 16, MF_COMMON | MF_CLASH_OK,
        "Maximum burst size (512 bytes)",
        "0: no maximum burst size\t"
        "1-65535: limit in units of 512 bytes\t"
        "Ignored by persistent connections"},
        /* obsoleted spl3r2, re-instated spl3r3 */
    {"CTLUNIT", DISCONNECT_MP, 0, -1, 13, 3, 2, MF_CLASH_OK,
        "Connect time limit unit",
        "0: 100 microsecond unit\t"
        "1: 1 microsecond unit\t"},
    {"BILUNIT", DISCONNECT_MP, 0, -1, 13, 1, 2, MF_CLASH_OK,    /* 21-021r3 */
        "Bus inactivity (time) limit unit",
        "0: 100 microsecond unit\t"
        "1: 1 microsecond unit\t"},
    {"FBS", DISCONNECT_MP, 0, -1, 14, 7, 16, MF_CLASH_OK,       /* 21-021r3 */
        "First burst size (512 bytes)",
        "0: no first burst size (no data-out before xfer_ready)\t"
        "1-65535: maximum first burst size in units of 512 bytes"},

    /* protocol specific logical unit mode page [0x18] sas/spl */
    {"LUPID", PROT_SPEC_LU_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Logical unit's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"TLR", PROT_SPEC_LU_MP, 0, -1, 2, 4, 1, MF_COMMON,
        "Transport layer retries", "0: disabled; 1: enabled (on target)"},

    /* protocol specific port mode page [0x19] sas/spl */
    {"PPID", PROT_SPEC_PORT_MP, 0, -1, 2, 3, 4, MF_COMMON | MF_CLASH_OK,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"CAWT", PROT_SPEC_PORT_MP, 0, -1, 2, 6, 1, 0,
        "Continue AWT (arbitration wait time (timer))", NULL},
    {"BAE", PROT_SPEC_PORT_MP, 0, -1, 2, 5, 1, 0,
        "Broadcast asynchronous event", NULL},
    {"RLM", PROT_SPEC_PORT_MP, 0, -1, 2, 4, 1, MF_COMMON,
        "Ready LED meaning",
        "0: usually on, flash when command processing; off when stopped\t"
        "1: usually off, flash when command processing"},
    {"ITNLT", PROT_SPEC_PORT_MP, 0, -1, 4, 7, 16, MF_COMMON,
        "I_T nexus loss time (ms)",
        "0: vendor specific\t"
        "2000: recommended in SPL-3\t"
        "0ffffh (-1): never recognize IT nexus loss"},
    {"IRT", PROT_SPEC_PORT_MP, 0, -1, 6, 7, 16, MF_COMMON,
        "Initiator response timeout (ms)",
        "0: disable initiator response timeout timer"},
    {"RTOL", PROT_SPEC_PORT_MP, 0, -1, 8, 7, 16, MF_COMMON,
        "Reject to open limit (10 us)",         /* added in sas2r14 */
        "0: vendor specific"},
    {"MAXR", PROT_SPEC_PORT_MP, 0, -1, 10, 7, 8, MF_COMMON,
        "Maximum allowed xfer ready", NULL},    /* added in spl4r01 */

    /* phy control and discover mode page [0x19,0x1] sas/spl */
    {"PPID_1", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"GENC", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 6, 7, 8, 0,
        "Generation code", "0: unknown, 1..255: valid"},
    {"NOP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 7, 7, 8, MF_COMMON,
        "Number of phys", "one descriptor per phy"},
    /* Phy mode descriptor starts here, <start_byte> relative to start of
     * mode page (i.e. 8 more than t10's phy mode descriptor table) */
    {"PHID", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 9, 7, 8, 0,
        "Phy identifier", NULL},
    {"ADT", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 12, 6, 3, 0,
        "Attached SAS device type",    /* the word SAS added in spl4r01 */
        "0: no device attached; 1: end device\t"
        "2: expander device; " /* in SAS-1.1 this was a "edge expander" */
        "3: expander device (fanout, SAS-1.1)"}, /* obsolete in SAS-2 */
    {"AREAS", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 12, 3, 4, 0,
        "Attached reason (other end did link reset)",
        "0: unknown; 1: power on; 2: hard reset; 3: SMP phy control\t"
        "4: loss of dword sync; 5: mux problem; ..."},
    {"REAS", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 13, 7, 4, 0,
        "Reason (for starting link reset)",
        "0: unknown; 1: power on; 2: hard reset; 3: SMP phy control\t"
        "4: loss of dword sync; 5: mux problem; ..."},
    {"NLLR", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 13, 3, 4, 0,
        "Negotiated logical link rate",         /* sas2r07 */
        "0: unknown; 1: disabled; 2: phy reset problem; 3: spinup hold\t"
        "4: port selector; 5: resetting; 6: attached unsupported\t"
        "8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps; 12: 22.5 Gbps"},
    {"ASIP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 14, 3, 1, 0,
        "Attached SSP initiator port", NULL},
    {"ATIP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 14, 2, 1, 0,
        "Attached STP initiator port", NULL},
    {"AMIP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 14, 1, 1, 0,
        "Attached SMP initiator port", NULL},
    {"ASTP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 15, 3, 1, 0,
        "Attached SSP target port", NULL},
    {"ATTP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 15, 2, 1, 0,
        "Attached STP target port", NULL},
    {"AMTP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 15, 1, 1, 0,
        "Attached SMP target port", NULL},
    {"SASA", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 16, 7, 64,
         MF_HEX | MF_COMMON, "SAS address", NULL},
    {"ASASA", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 24, 7, 64,
         MF_HEX | MF_COMMON, "Attached SAS address", NULL},
    {"APHID", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 32, 7, 8, 0,
        "Attached phy identifier", NULL},
    {"APERCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 7, 1, 0,
        "Attached persistent capable", NULL},
    {"APOWCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 6, 2, 0,
        "Attached power capable", "0: not; 1: can consume; 2: can source"},
    {"ASLCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 4, 1, 0,
        "Attached slumber capable", NULL},
    {"APACAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 3, 1, 0,
        "Attached partial capable", NULL},
    {"AIZPER", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 2, 1, 0,
        "Attached inside ZPSDS persistent", NULL},
    {"AREQIZ", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 1, 1, 0,
        "Attached requested inside ZPSDS", NULL},
    {"ABRCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 33, 0, 1, 0,
        "Attached break reply capable", NULL},
    {"AAPTACAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 34, 2, 1, 0,
        "Attached APTA capable", NULL},
    {"ASMPPCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 34, 1, 1, 0,
        "Attached SMP priority capable", NULL},
    {"APOWDCAP", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 34, 0, 1, 0,
        "Attached power disable capable", NULL},
    {"PMILR", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 40, 7, 4, 0,
        "Programmed minimum link rate",
        "0: not programmed; 8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps\t"
        "12: 22.5 Gbps"},
    {"HMILR", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 40, 3, 4, 0,
        "Hardware minimum link rate",
        "8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps; 12: 22.5 Gbps"},
    {"PMALR", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 41, 7, 4, 0,
        "Programmed maximum link rate",
        "0: not programmed; 8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps\t"
        "12: 22.5 Gbps"},
    {"HMALR", PROT_SPEC_PORT_MP, MSP_SAS_PCD, -1, 41, 3, 4, 0,
        "Hardware maximum link rate",
        "8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps; 12: 22.5 Gbps"},

    /* shared port control mode page [0x19,0x2] sas/spl */
    {"PPID_2", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"PLT", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 6, 7, 16, 0,
        "Power loss timeout(ms)", NULL},
    {"PGRATO", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 9, 7, 8, 0,
        "Power grant timeout(sec)", NULL},
    {"4PHYS", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 10, 2, 1, 0,
        "4 phy wide port(s) when set", "If more than 4 phys, group adjacent "
        "(by phy id) phys"},     /* start of spl5r07 addition */
    {"2PHYS", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 10, 1, 1, 0,
        "2 phy wide port(s) when set", "If more than 2 phys, group adjacent "
        "(by phy id) phys"},
    {"1PHY", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 10, 0, 1, 0,
        "single phy (narrow) ports", "Each phy is a SCSI port with own SAS "
        "address"},
    {"PMCDT", PROT_SPEC_PORT_MP, MSP_SAS_SPC, -1, 11, 7, 8, 0,
        "port mode change delay time (unit: seconds)", "Minimum time device "
        "remains offline after change"},    /* end of spl5r07 addition */

    /* SAS-2 Enhanced phy mode page [0x19,0x3] sas/spl */
    {"PPID_3", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 5, 3, 4, 0,
        "Port's (transport) protocol identifier",
        PROTO_IDENT_STR },
    {"GENC_1", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 6, 7, 8, 0,
        "Generation code", "0: unknown, 1..255: valid"},
    {"NOP_1", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 7, 7, 8, 0,
        "Number of phys", "one descriptor per phy"},
    /* Phy mode descriptor starts here, <start_byte> relative to start of
     * mode page (i.e. 8 more than t10's phy mode descriptor table) */
    {"PHID_1", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 9, 7, 8, 0,
        "Phy identifier", NULL},
    {"PPCAP", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 12, 7, 32, MF_HEX,
        "Programmed phy capabilities", NULL},
    {"CPCAP", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 16, 7, 32, MF_HEX,
        "Current phy capabilities", NULL},
    {"APCAP", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 20, 7, 32, MF_HEX,
        "Attached phy capabilities", NULL},
    {"OPT_M_EN", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 26, 5, 1, 0,
        "Optical mode enabled", NULL},
    {"N_SSC", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 26, 4, 1, 0,
        "Negotiated spread spectrum clocking", NULL},
    {"NPLR", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 26, 3, 4, 0,
        "Negotiated physical link rate",
        "0: unknown; 1: disabled; 2: phy reset problem; 3: spinup hold\t"
        "4: port selector; 5: resetting; 6: attached unsupported\t"
        "8: 1.5 Gbps; 9: 3 Gbps; 10: 6 Gbps; 11: 12 Gbps; 12: 22.5 Gbps"},
    {"EN_SL", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 27, 2, 1, 0,
        "Enable slumber phy power condition", NULL},
    {"EN_PA", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 27, 1, 1, 0,
        "Enable partial phy power condition", NULL},
    {"HMS", PROT_SPEC_PORT_MP, MSP_SAS_E_PHY, -1, 27, 0, 1, 0,
        "Hardware muxing supported", NULL},     /* obsolete spl5r01 */

    /* SPL-5 Out of band management control mode page [0x19,0x4] sas/spl */
    /* SFF-8609 related: Management Interface for drive thermal conditions */
    {"OOB_RE", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 4, 7, 1, 0,
        "Out of band reporting enabled", "MSelect 1->0: send stopping "
        "transmission packet\t0->1: send protocol revision code packet"},
    {"OOB_PRV", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 6, 7, 16, MF_HEX,
        "Out of band protocol revision code",
        "example: SFF-8609 revison 1.2 is code 0x102"},
    {"OOB_D_ID", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 8, 3, 4,
        0, "Out of band descriptor identifier",
        "0: temperature attribute; 1-15: restricted for SFF-8209"},
    /* MF_DESC_ID_B0-3 bits are all zero for Temperature attribute */
    {"TA_TRE", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 12, 0, 1, MF_CLASH_OK,
        "Temperature attribute, temperature reporting enabled", NULL},
    {"TA_RI", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 13, 7, 8, MF_CLASH_OK,
        "Temperature attribute, reporting interval (seconds)", NULL},
    {"TA_MRI", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 14, 7, 8, MF_CLASH_OK,
        "Temperature attribute, minimum reporting interval (seconds)", NULL},
    {"TA_C_UP", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 15, 7, 4, MF_CLASH_OK,
        "Temperature attribute, change up (Celsius)", NULL},
    {"TA_C_DO", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 15, 3, 4, MF_CLASH_OK,
        "Temperature attribute, change down (Celsius)", NULL},
    {"TA_TM", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 16, 1, 2, MF_CLASH_OK,
        "Temperature attribute, test mode",
        "0: test mode disabled, transfer actual temperature\t"
        "1: TM enabled, send incrementing sequence of temps\t"
        "2: TM enabled, send decrementing sequence of temps\t"
        "3: TM enabled, send value in TA_TM_T every interval"},
    {"TA_TM_T", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 18, 7, 8,
        MF_CLASH_OK | MF_TWOS_COMP,
        "Temperature attribute test mode temperature", NULL},
#if 0
    {"MY_TEST", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 13, 7, 8,
        MF_CLASH_OK | MF_DESC_ID_B0,
        "My test (dpg), 8 bit number, desc_id=1", NULL},
    {"MY_EXTRA", PROT_SPEC_PORT_MP, MSP_SAS_OOB_M_C, -1, 18, 7, 8,
        MF_CLASH_OK | MF_DESC_ID_B0,
        "My test (dpg), 8 bit number, desc_id=1", NULL},
#endif

    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, NULL},
};

/* fixed length, indexed by transport protocol number */
struct sdparm_transport_pair sdparm_transport_mp[] = {
    {sdparm_fcp_mode_pg, sdparm_mitem_fcp_arr}, /* 0 */
    {sdparm_spi_mode_pg, sdparm_mitem_spi_arr},
    {NULL, NULL},
    {NULL, NULL},
    {sdparm_srp_mode_pg, sdparm_mitem_srp_arr},
    {NULL, NULL},
    {sdparm_sas_mode_pg, sdparm_mitem_sas_arr},
    {NULL, NULL},
    {NULL, NULL},       /* 8: ata (SAT mpages in generic) */
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {sdparm_gen_mode_pg, sdparm_mitem_arr}, /* 15: none, treat as generic */
};

const char * sdparm_ansi_version_arr[] =
{
    "no conformance claimed",
    "SCSI-1",
    "SCSI-2",
    "SPC",
    "SPC-2",
    "SPC-3",
    "SPC-4",
    "ANSI version: 7",
};

const char * sdparm_network_service_type_arr[] =
{
    "unspecified",
    "storage configuration service",
    "diagnostics",
    "status",
    "logging",
    "code download",
    "copy service",
    "administrative configuration service",
    "reserved[0x8]", "reserved[0x9]",
    "reserved[0xa]", "reserved[0xb]", "reserved[0xc]", "reserved[0xd]",
    "reserved[0xe]", "reserved[0xf]", "reserved[0x10]", "reserved[0x11]",
    "reserved[0x12]", "reserved[0x13]", "reserved[0x14]", "reserved[0x15]",
    "reserved[0x16]", "reserved[0x17]", "reserved[0x18]", "reserved[0x19]",
    "reserved[0x1a]", "reserved[0x1b]", "reserved[0x1c]", "reserved[0x1d]",
    "reserved[0x1e]", "reserved[0x1f]",
};

const char * sdparm_mode_page_policy_arr[] =
{
    "shared",
    "per target port",
    "per initiator port (obsolete)",    /* made obsolete in SPC-4 */
    "per I_T nexus",
};

struct sdparm_command_t sdparm_command_arr[] =
{
    {CMD_CAPACITY, "capacity", "ca", NULL},
    {CMD_EJECT, "eject", "ej", NULL},
    {CMD_LOAD, "load", "lo", NULL},
    {CMD_PROFILE, "profile", "pr", NULL},
    {CMD_READY, "ready", "re", NULL},
    {CMD_SENSE, "sense", "se", NULL},
    {CMD_SPEED, "speed", "sp", "new_speed_kbps"},
    {CMD_START, "start", "sta", NULL},
    {CMD_STOP, "stop", "sto", NULL},
    {CMD_SYNC, "sync", "sy", NULL},
    {CMD_UNLOCK, "unlock", "un", NULL},
    {-1, NULL, NULL, NULL},
};

struct sdparm_val_desc_t sdparm_profile_arr[] = {
        {0x0, "No current profile"},
        {0x1, "Non-removable disk (obs)"},
        {0x2, "Removable disk"},
        {0x3, "Magneto optical erasable"},
        {0x4, "Optical write once"},
        {0x5, "AS-MO"},
        {0x8, "CD-ROM"},
        {0x9, "CD-R"},
        {0xa, "CD-RW"},
        {0x10, "DVD-ROM"},
        {0x11, "DVD-R sequential recording"},
        {0x12, "DVD-RAM"},
        {0x13, "DVD-RW restricted overwrite"},
        {0x14, "DVD-RW sequential recording"},
        {0x15, "DVD-R dual layer sequental recording"},
        {0x16, "DVD-R dual layer jump recording"},
        {0x17, "DVD-RW dual layer"},
        {0x18, "DVD-Download disc recording"},
        {0x1a, "DVD+RW"},
        {0x1b, "DVD+R"},
        {0x20, "DDCD-ROM"},
        {0x21, "DDCD-R"},
        {0x22, "DDCD-RW"},
        {0x2a, "DVD+RW dual layer"},
        {0x2b, "DVD+R dual layer"},
        {0x40, "BD-ROM"},
        {0x41, "BD-R SRM"},
        {0x42, "BD-R RRM"},
        {0x43, "BD-RE"},
        {0x50, "HD DVD-ROM"},
        {0x51, "HD DVD-R"},
        {0x52, "HD DVD-RAM"},
        {0x53, "HD DVD-RW"},
        {0x58, "HD DVD-R dual layer"},
        {0x5a, "HD DVD-RW dual layer"},
        {0xffff, "Non-conforming profile"},
        {-1, NULL},
};
