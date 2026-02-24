#ifndef _MORSE_FW_H_
#define _MORSE_FW_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/firmware.h>
#include <linux/completion.h>
#include "capabilities.h"
#include "misc.h"
#include "yaps-hw.h"

#define BCF_DATABASE_SIZE               (1024)	/* From firmware */
#define MORSE_FW_DIR                    "morse"
#define MORSE_FW_FULLMAC_STRING		"-flm"
#define MORSE_FW_THIN_LMAC_STRING       "-tlm"
#define MORSE_FW_VIRTUAL_STA_STRING     "-vst"
#define MORSE_FW_EXT                    ".bin"

#define IFLASH_BASE_ADDR	0x400000
#define DFLASH_BASE_ADDR	0xC00000

#define MAX_BCF_NAME_LEN	64

/* FW_CAPABILITIES_FLAGS_WIDTH = ceil(MORSE_CAPS_MAX_HW_LEN / 32) */
#define FW_CAPABILITIES_FLAGS_WIDTH (4)

#if (FW_CAPABILITIES_FLAGS_WIDTH > CAPABILITIES_FLAGS_WIDTH)
#error "Capability subset filled by firmware is to big"
#endif

enum morse_fw_info_tlv_type {
	MORSE_FW_INFO_TLV_BCF_ADDR = 1,
	MORSE_FW_INFO_TLV_COREDUMP_MEM_REGION = 2,
};

struct morse_fw_info_tlv_coredump_mem {
	/* region type (see morse_coredump_mem_region_type) */
	__le32 region_type;
	/* on-chip start address of the region */
	__le32 start;
	/* the length of the region */
	__le32 len;
} __packed;

struct morse_fw_info_tlv {
	__le16 type;
	__le16 length;
	u8 val[];
} __packed;

enum morse_fw_extended_host_table_tag {
	/* The S1G capability tag */
	MORSE_FW_HOST_TABLE_TAG_S1G_CAPABILITIES = 0,
	MORSE_FW_HOST_TABLE_TAG_PAGER_BYPASS_TX_STATUS = 1,
	MORSE_FW_HOST_TABLE_TAG_INSERT_SKB_CHECKSUM = 2,
	MORSE_FW_HOST_TABLE_TAG_YAPS_TABLE = 3,
	MORSE_FW_HOST_TABLE_TAG_PAGER_PKT_MEMORY = 4,
	MORSE_FW_HOST_TABLE_TAG_PAGER_BYPASS_CMD_RESP = 5,
};

struct extended_host_table_tlv_hdr {
	/** The tag used to identify which capability this represents */
	__le16 tag;
	/** The length of the capability structure including this header */
	__le16 length;
} __packed;

struct extended_host_table_capabilities_s1g {
	/** The common header for the capabilities */
	struct extended_host_table_tlv_hdr header;
	/** The capability flags */
	__le32 flags[FW_CAPABILITIES_FLAGS_WIDTH];
	/**
	 * The minimum A-MPDU start spacing required by firmware.
	 * Value | Description
	 * ------|------------
	 * 0     | No restriction
	 * 1     | 1/4 us
	 * 2     | 1/2 us
	 * 3     | 1 us
	 * 4     | 2 us
	 * 5     | 4 us
	 * 6     | 8 us
	 * 7     | 16 us
	 */
	u8 ampdu_mss;
	/** The beamformee STS capability value */
	u8 beamformee_sts_capability;
	/* Number of sounding dimensions */
	u8 number_sounding_dimensions;
	/**
	 * The maximum A-MPDU length. This is the exponent value such that
	 * (2^(13 + exponent) - 1) is the length
	 */
	u8 maximum_ampdu_length;
	/**
	 * Offset to apply to the specification's mmss table to signal further
	 * minimum mpdu start spacing.
	 */
	u8 morse_mmss_offset;
} __packed;

struct extended_host_table_pager_bypass_tx_status {
	struct extended_host_table_tlv_hdr header;
	__le32 tx_status_buffer_addr;
};

struct extended_host_table_pager_bypass_cmd_resp {
	struct extended_host_table_tlv_hdr header;
	__le32 cmd_resp_buffer_addr;
};

struct extended_host_table_insert_skb_checksum {
	struct extended_host_table_tlv_hdr header;
	u8 insert_and_validate_checksum;
};

struct extended_host_table_yaps_table {
	struct extended_host_table_tlv_hdr header;
	struct morse_yaps_hw_table yaps_table;
} __packed;

struct extended_host_table_pager_pkt_memory {
	struct extended_host_table_tlv_hdr header;
	/** Base address of packet memory */
	__le32 base_addr;
	/** Length (bytes) of one page */
	u16 page_len;
	/** Length (bytes) reserved at start of page (should not be modified) */
	u8 page_len_reserved;
	/** Number of pages */
	u8 num;
};

struct extended_host_table {
	/** The length of this table */
	__le32 extended_host_table_length;
	/** Device MAC address */
	u8 dev_mac_addr[6];

	/** Data TLVs in the extended host table*/
	u8 ext_host_table_data_tlvs[];
} __packed;

int morse_firmware_init(struct morse *mors, uint test_mode);

/**
 * @brief Do necessary preparation and then initialise firmware
 *
 * @param mors The global morse config object
 * @param reset_hw Perform non-destructive reset of the chip,
 *                 preserving the existing sdio enumeration whilst
 *                 resetting the firmware state
 * @param reattach_hw Reattach to running hardware
 * @return 0 if success else error code
 */
int morse_firmware_prepare_and_init(struct morse *mors, bool reset_hw, bool reattach_hw);

/**
 * morse_firmware_build_fw_path() - Build path to the firmware image.
 * @mors: The global morse config object.
 *
 * Caller must kfree() the returned value.
 *
 * Return: allocated firmware path, or %NULL on error
 */
char *morse_firmware_build_fw_path(struct morse *mors);

/**
 * morse_firmware_parse_extended_host_table - Read and parse the firmware's extended host table
 *
 * @mors pointer to the chip object
 *
 * Return: 0 if the table was read successfully, error otherwise.
 */
int morse_firmware_parse_extended_host_table(struct morse *mors);

/**
 * morse_firmware_get_host_table_ptr - Read the host table
 *
 * @mors pointer to the chip object
 *
 * Return: 0 if the table was read successfully, error otherwise.
 */
int morse_firmware_get_host_table_ptr(struct morse *mors);

/**
 * morse_firmware_magic_verify - Verify magic number from host table
 *
 * @mors pointer to the chip object
 *
 * Return: 0 if successfully verified, error otherwise.
 */
int morse_firmware_magic_verify(struct morse *mors);

/**
 * morse_firmware_check_compatibility - Verify that the driver is compatible with the firmware
 *
 * @mors pointer to the chip object
 *
 * Return: 0 if compatible, error otherwise.
 */
int morse_firmware_check_compatibility(struct morse *mors);

#endif /* !_MORSE_FW_H_ */
