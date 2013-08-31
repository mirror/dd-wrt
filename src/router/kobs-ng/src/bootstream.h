/*
* Copyright (C) 2010 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef BOOTSTREAM_H
#define BOOTSTREAM_H

#include <stdint.h>

//! An AES-128 cipher block is 16 bytes.
typedef uint8_t cipher_block_t[16];

//! An AES-128 key is 128 bits, or 16 bytes.
typedef uint8_t aes128_key_t[16];

//! A SHA-1 digest is 160 bits, or 20 bytes.
typedef uint8_t sha1_digest_t[20];

struct version_t {
	uint16_t m_major;
	uint16_t m_pad0;
	uint16_t m_minor;
	uint16_t m_pad1;
	uint16_t m_revision;
	uint16_t m_pad2;
};

struct boot_image_header_t {
	sha1_digest_t m_digest;
	uint8_t m_signature[4];
	uint8_t m_majorVersion;
	uint8_t m_minorVersion;
	uint16_t m_flags;
	uint32_t m_imageBlocks;
	uint32_t m_firstBootTagBlock;
	uint32_t m_firstBootableSectionID;
	uint16_t m_keyCount;
	uint16_t m_keyDictionaryBlock;
	uint16_t m_headerBlocks;
	uint16_t m_sectionCount;
	uint16_t m_sectionHeaderSize;
	uint8_t m_padding0[6];
	uint64_t m_timestamp;
	struct version_t m_productVersion;
	struct version_t m_componentVersion;
	uint16_t m_driveTag;
	uint8_t m_padding1[6];
};

#define ROM_DISPLAY_PROGRESS (1 << 0)
#define ROM_VERBOSE_PROGRESS (1 << 1)

struct section_header_t {
	uint32_t m_identifier;
	uint32_t m_offset;
	uint32_t m_length;
	uint32_t m_flags;
};

#define ROM_SECTION_BOOTABLE (1 << 0)
#define ROM_SECTION_CLEARTEXT (1 << 1)

struct dek_dictionary_entry_t
{
	cipher_block_t m_mac;	//!< CBC-MAC of the header.
	aes128_key_t m_dek;	//!< AES-128 key with which the image payload is encrypted.
};

struct boot_command_t {
	uint8_t m_checksum;
	uint8_t m_tag;
	uint16_t m_flags;
	uint32_t m_address;
	uint32_t m_count;
	uint32_t m_data;
};
#define ROM_NOP_CMD	0x00
#define ROM_TAG_CMD	0x01
#define ROM_LOAD_CMD	0x02
#define ROM_FILL_CMD	0x03
#define ROM_JUMP_CMD	0x04
#define ROM_CALL_CMD	0x05
#define ROM_MODE_CMD	0x06

static inline uint8_t boot_command_chksum(const struct boot_command_t *bc)
{
	const uint8_t *bytes = (const uint8_t *)bc;
	uint8_t checksum;
	int i;

	checksum = 0x5a;
	for (i = 1; i < sizeof(*bc); i++)
		checksum += bytes[i];

	return checksum;
}

#define BOOTSTREAM_BLOCK_SIZE	16
#define BOOTSTREAM_ALIGN_SIZE(x) (((x) + (BOOTSTREAM_BLOCK_SIZE - 1)) & ~BOOTSTREAM_BLOCK_SIZE)

int bootstream_verify(int flags, FILE *fp, const uint8_t *key, long *end_of_file);

char *vec_ascii(const uint8_t *data, char *txt);
char *sha_ascii(const uint8_t *data, char *txt);
uint8_t *ascii_vec(const char *txt, uint8_t *vec);

#endif
