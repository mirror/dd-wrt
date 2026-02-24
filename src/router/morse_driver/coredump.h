/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <linux/types.h>

#define MORSE_COREDUMP_FILE_VERSION      (1)

/* List of available methods to generate a coredump */
enum morse_coredump_method {
	/* Method for coredump is to call out to an external userspace script */
	COREDUMP_METHOD_USERSPACE_SCRIPT = 0,
	/* Use the system bus (e.g. SDIO, SPI, etc) to read mem regions on chip */
	COREDUMP_METHOD_BUS              = 1,
};

/* Describe the type of a memory region */
enum morse_coredump_mem_region_type {
	/* no specific distinction made on memory region */
	MORSE_MEM_REGION_TYPE_GENERAL = 1,
	/* Assertion information */
	MORSE_MEM_REGION_TYPE_ASSERT_INFO = 2,
};

/* Describe an on-chip memory region */
struct morse_coredump_mem_region {
	struct list_head list;
	/* memory region type */
	enum morse_coredump_mem_region_type type;
	/* start address of memory region */
	u32 start;
	/* length of memory region */
	u32 len;
};

enum morse_coredump_reason {
	/* unknown reason / reset state */
	MORSE_COREDUMP_REASON_UNKNOWN = 0,
	/* the health-check command failed */
	MORSE_COREDUMP_REASON_HEALTH_CHECK_FAILED = 1,
	/* user requested a coredump (vendor command) */
	MORSE_COREDUMP_REASON_USER_REQUEST = 2,
	/* chip has notified host of stop */
	MORSE_COREDUMP_REASON_CHIP_INDICATED_STOP = 3,
};

/* Describe note types when parsing notes in a morse coredump file
 * Note; Note type enum starts at a high number to avoid overlap with
 *       standard/upstream elf note types.
 */
enum morse_coredump_note_type {
	/* Intepret note data as a u32 */
	MORSE_COREDUMP_NOTE_TYPE_U32 = 0x900,
	/* Intepret note data as a u64 */
	MORSE_COREDUMP_NOTE_TYPE_U64 = 0x901,
	/* Intepret note data as an ascii str */
	MORSE_COREDUMP_NOTE_TYPE_STR = 0x902,
	/* Intepret note data as a u8 array */
	MORSE_COREDUMP_NOTE_TYPE_BIN = 0x903,
};

/* Contains data to later create a coredump. It can contain
 * memory descriptors (filled from metadata provided in fw binary)
 * and data (upon coredump).
 */
struct morse_coredump_data {
	/* Reason for coredump creation */
	enum morse_coredump_reason reason;
	/* Stop information string (may be NULL if not available) */
	char *information;
	/* system timestamp of when the data was filled */
	struct timespec64 timestamp;
	/* chip-memory regions of interest upon a coredump */
	struct {
		/* dynamically allocated to store memory region info / contents */
		struct list_head regions;
	} memory;
};

/**
 * morse_coredump_destroy() - Destroy any dynamically allocated memory for
 *                            coredump creation.
 *
 * @mors: Morse chip object
 */
void morse_coredump_destroy(struct morse *mors);

/**
 * morse_coredump_new() - Initialise a new coredump with a reason code.
 *                        The driver will typically schedule restart work
 *                        after this is called, eventually invoking
 *                        morse_coredump().
 *
 * @mors: Morse chip object
 * @reason: Reason for coredump
 *
 * return 0 on success
 */
int morse_coredump_new(struct morse *mors, enum morse_coredump_reason reason);

/**
 * morse_coredump() - Generate a morse coredump file.
 *
 * @mors: Morse chip object
 *
 * return 0 on success
 */
int morse_coredump(struct morse *mors);

/**
 * morse_coredump_add_memory_region() - Add the provided memory region descriptor
 *                                      to the list of on-chip memory regions to
 *                                      read on a coredump.
 *
 * @mors: Morse chip object
 * @desc: Memory region descriptor from the fw_info TLV
 *
 * return 0 on success
 */
int morse_coredump_add_memory_region(struct morse *mors,
						const struct morse_fw_info_tlv_coredump_mem *desc);

/**
 * morse_coredump_remove_memory_regions() - Remove all memory region descriptors
 *                                          for core-dumping.
 *
 * @mors: Morse chip object
 */
void morse_coredump_remove_memory_regions(struct morse *mors);

/**
 * morse_coredump_set_fw_binary_str() - Store the name of fw binary used to
 *                                      initialise the chip.
 *
 * @mors: Morse chip object
 * @str: The name of binary (or NULL to clear)
 */
void morse_coredump_set_fw_binary_str(struct morse *mors, const char *str);

/**
 * morse_coredump_set_fw_version_str() - Store the name of fw version string
 *                                       stored int the chip.
 *
 * @mors: Morse chip object
 * @str: The version string (or NULL to clear)
 */
void morse_coredump_set_fw_version_str(struct morse *mors, const char *str);

/**
 * morse_coredump_reason_to_str() - Convert coredump reason to string
 *
 * @reason: Reason for failure
 *
 * return string representing failure
 */
static inline const char *morse_coredump_reason_to_str(enum morse_coredump_reason reason)
{
	switch (reason) {
	case MORSE_COREDUMP_REASON_HEALTH_CHECK_FAILED:
		return "health check failure";
	case MORSE_COREDUMP_REASON_USER_REQUEST:
		return "user request";
	case MORSE_COREDUMP_REASON_CHIP_INDICATED_STOP:
		return "chip indicated stop";
	default:
		return "unknown";
	}
}
