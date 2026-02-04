// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB descriptor dumping
 *
 * Copyright (C) 2017-2018 Michael Drake <michael.drake@codethink.co.uk>
 */

#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <libusb.h>

#include "desc-defs.h"
#include "desc-dump.h"
#include "usbmisc.h"
#include "names.h"

/**
 * Print a description of a bmControls field value, using a given string array.
 *
 * Handles the DESC_BMCONTROL_1 and DESC_BMCONTROL_2 field types.  The former
 * is one bit per string, and the latter is 2 bits per string, with the
 * additional bit specifying whether the control is read-only.
 *
 * \param[in] bmcontrols  The value to dump a human-readable representation of.
 * \param[in] strings     Array of human-readable strings, must be NULL terminated.
 * \param[in] type        The type of the value in bmcontrols.
 * \param[in] indent      The current indent level.
 */
static void desc_bmcontrol_dump(
		unsigned long long bmcontrols,
		const char * const * strings,
		enum desc_type type,
		unsigned int indent)
{
	static const char * const setting[] = {
		"read-only",
		"ILLEGAL VALUE (0b10)",
		"read/write"
	};
	unsigned int count = 0;
	unsigned int control;

	assert((type == DESC_BMCONTROL_1) ||
	       (type == DESC_BMCONTROL_2));

	while (strings[count] != NULL) {
		if (strings[count][0] != '\0') {
			if (type == DESC_BMCONTROL_1) {
				if ((bmcontrols >> count) & 0x1) {
					printf("%*s%s Control\n",
							indent * 2, "",
							strings[count]);
				}
			} else {
				control = (bmcontrols >> (count * 2)) & 0x3;
				if (control) {
					printf("%*s%s Control (%s)\n",
							indent * 2, "",
							strings[count],
							setting[control-1]);
				}
			}
		}
		count++;
	}
}

/**
 * Read N bytes from descriptor data buffer into a value.
 *
 * Only supports values of up to 8 bytes.
 *
 * \param[in] buf     Buffer containing the bytes to read.
 * \param[in] offset  Offset in buffer to start reading bytes from.
 * \param[in] bytes   Number of bytes to read.
 * \return Value contained within the given bytes.
 */
static unsigned long long get_n_bytes_as_ull(
		const unsigned char *buf,
		unsigned int offset,
		unsigned int bytes)
{
	unsigned long long ret = 0;

	if (bytes > 8) {
		fprintf(stderr, "Bad descriptor definition; Field size > 8.\n");
		exit(EXIT_FAILURE);
	}

	buf += offset;

	switch (bytes) {
	case 8: ret |= ((unsigned long long)buf[7]) << 56; /* fall-through */
	case 7: ret |= ((unsigned long long)buf[6]) << 48; /* fall-through */
	case 6: ret |= ((unsigned long long)buf[5]) << 40; /* fall-through */
	case 5: ret |= ((unsigned long long)buf[4]) << 32; /* fall-through */
	case 4: ret |= ((unsigned long long)buf[3]) << 24; /* fall-through */
	case 3: ret |= ((unsigned long long)buf[2]) << 16; /* fall-through */
	case 2: ret |= ((unsigned long long)buf[1]) <<  8; /* fall-through */
	case 1: ret |= ((unsigned long long)buf[0]);
	}

	return ret;
}

/**
 * Get the size of a descriptor field in bytes.
 *
 * Normally the size is provided in the entry's size parameter, but some
 * fields have a variable size, with the actual size being stored in as
 * the value of another field.
 *
 * \param[in] buf    Descriptor data.
 * \param[in] desc   First field in the descriptor definition array.
 * \param[in] entry  The descriptor definition field to get size for.
 * \return Size of the field in bytes.
 */
static unsigned int get_entry_size(
		const unsigned char *buf,
		const struct desc *desc,
		const struct desc *entry);

/**
 * Read a value from a field of given name.
 *
 * \param[in] buf    Descriptor data.
 * \param[in] desc   First field in the descriptor definition array.
 * \param[in] field  The name of the field to get the value for.
 * \return The value from the given field.
 */
static unsigned long long get_value_from_field(
		const unsigned char *buf,
		const struct desc *desc,
		const char *field)
{
	size_t offset = 0;
	const struct desc *current;
	unsigned long long value = 0;

	/* Search descriptor definition array for the field who's value
	 * gives the value of the entry we're interested in. */
	for (current = desc; current->field != NULL; current++) {
		if (strcmp(current->field, field) == 0) {
			value = get_n_bytes_as_ull(buf, offset,
					current->size);
			break;
		}

		/* Keep track of our offset in the descriptor data
		 * as we look for the field we want. */
		offset += get_entry_size(buf, desc, current);
	}

	return value;
}

/**
 * Dump a number as hex to stdout.
 *
 * \param[in] buf     Descriptor buffer to get values to render from.
 * \param[in] width   Character width to right-align value inside.
 * \param[in] offset  Offset in buffer to start of value to render.
 * \param[in] bytes   Byte length of value to render.
 */
static void hex_renderer(
		const unsigned char *buf,
		unsigned int width,
		unsigned int offset,
		unsigned int bytes)
{
	unsigned int align = (width >= bytes * 2) ? width - bytes * 2 : 0;
	printf(" %*s0x%0*llx", align, "", bytes * 2,
			get_n_bytes_as_ull(buf, offset, bytes));
}

/**
 * Dump a number to stdout.
 *
 * Single-byte numbers a rendered as decimal, otherwise hexadecimal is used.
 *
 * \param[in] buf     Descriptor buffer to get values to render from.
 * \param[in] width   Character width to right-align value inside.
 * \param[in] offset  Offset in buffer to start of value to render.
 * \param[in] bytes   Byte length of value to render.
 */
static void number_renderer(
		const unsigned char *buf,
		unsigned int width,
		unsigned int offset,
		unsigned int bytes)
{
	if (bytes == 1) {
		/* Render small numbers as decimal */
		printf("   %*u", width, buf[offset]);
	} else {
		/* Otherwise render as hexadecimal */
		hex_renderer(buf, width, offset, bytes);
	}
}

/**
 * Render a field's value to stdout.
 *
 * The manner of rendering the value is dependent on the value type.
 *
 * \param[in] dev           LibUSB device handle.
 * \param[in] current       Descriptor definition field to render.
 * \param[in] current_size  Size of value to render.
 * \param[in] buf           Byte array containing the descriptor date to dump.
 * \param[in] buf_len       Byte length of `buf`.
 * \param[in] desc          First field in the descriptor definition.
 * \param[in] indent        Current indent level.
 * \param[in] offset        Offset to current value in `buf`.
 */
static void value_renderer(
		libusb_device_handle *dev,
		const struct desc *current,
		unsigned int current_size,
		const unsigned char *buf,
		unsigned int buf_len,
		const struct desc *desc,
		unsigned int indent,
		size_t offset)
{
	/** Maximum amount of characters to right align numerical values by. */
	const unsigned int size_chars = 4;

	switch (current->type) {
	case DESC_NUMBER: /* fall-through */
	case DESC_CONSTANT:
		number_renderer(buf, size_chars, offset, current_size);
		printf("\n");
		break;
	case DESC_NUMBER_POSTFIX:
		number_renderer(buf, size_chars, offset, current_size);
		printf("%s\n", current->number_postfix);
		break;
	case DESC_NUMBER_STRINGS: {
		unsigned int i;
		unsigned long long value = get_n_bytes_as_ull(buf, offset, current_size);
		number_renderer(buf, size_chars, offset, current_size);
		for (i = 0; i <= value; i++) {
			if (current->number_strings[i] == NULL) {
				break;
			}
			if (value == i) {
				printf(" %s", current->number_strings[i]);
			}
		}
		printf("\n");
		break;
	}
	case DESC_BCD: {
		unsigned int i;
		printf("  %2x", buf[offset + current_size - 1]);
		for (i = 1; i < current_size; i++) {
			printf(".%02x", buf[offset + current_size - 1 - i]);
		}
		printf("\n");
		break;
	}
	case DESC_BITMAP:
		hex_renderer(buf, size_chars, offset, current_size);
		printf("\n");
		break;
	case DESC_BMCONTROL_1: /* fall-through */
	case DESC_BMCONTROL_2:
		hex_renderer(buf, size_chars, offset, current_size);
		printf("\n");
		desc_bmcontrol_dump(
				get_n_bytes_as_ull(buf, offset, current_size),
				current->bmcontrol, current->type, indent + 1);
		break;
	case DESC_BITMAP_STRINGS: {
		unsigned int i;
		unsigned long long value = get_n_bytes_as_ull(buf, offset, current_size);
		hex_renderer(buf, size_chars, offset, current_size);
		printf("\n");
		for (i = 0; i < current->bitmap_strings.count; i++) {
			if (current->bitmap_strings.strings[i] == NULL) {
				continue;
			}
			if (((value >> i) & 0x1) == 0) {
				continue;
			}
			printf("%*s%s\n", (indent + 1) * 2, "",
					current->bitmap_strings.strings[i]);
		}
		break;
	}
	case DESC_STR_DESC_INDEX: {
		char *string;
		number_renderer(buf, size_chars, offset, current_size);
		string = get_dev_string(dev, buf[offset]);
		if (string) {
			printf(" %s\n", string);
			free(string);
		} else {
			printf("\n");
		}
		break;
	}
	case DESC_CS_STR_DESC_ID:
		number_renderer(buf, size_chars, offset, current_size);
		/* TODO: Add support for UAC3 class-specific String descriptor */
		printf("\n");
		break;
	case DESC_TERMINAL_STR:
		number_renderer(buf, size_chars, offset, current_size);
		printf(" %s\n", names_audioterminal(
				get_n_bytes_as_ull(buf, offset, current_size)));
		break;
	case DESC_EXTENSION: {
		unsigned int type = get_value_from_field(buf, desc,
				current->extension.type_field);
		const struct desc *ext_desc;
		const struct desc_ext *ext;

		/* Lookup the extension descriptor definitions to use, */
		for (ext = current->extension.d; ext->desc != NULL; ext++) {
			if (ext->type == type) {
				ext_desc = ext->desc;
				break;
			}
		}

		/* If the type didn't match a known type, use the
		 * undefined descriptor. */
		if (ext->desc == NULL) {
			ext_desc = desc_undefined;
		}

		desc_dump(dev, ext_desc, buf + offset,
				buf_len - offset, indent);

		break;
	}
	case DESC_SNOWFLAKE:
		number_renderer(buf, size_chars, offset, current_size);
		current->snowflake(
				get_n_bytes_as_ull(buf, offset, current_size),
				indent + 1);
		break;
	}
}

/* Documented at forward declaration above. */
static unsigned int get_entry_size(
		const unsigned char *buf,
		const struct desc *desc,
		const struct desc *entry)
{
	unsigned int size = entry->size;

	if (entry->size_field != NULL) {
		/* Variable field length, given by `size_field`'s value. */
		size = get_value_from_field(buf, desc, entry->size_field);
	}

	if (size == 0) {
		fprintf(stderr, "Bad descriptor definition; "
				"'%s' field has zero size.\n", entry->field);
		exit(EXIT_FAILURE);
	}

	return size;
}

/**
 * Get the number of entries needed by an descriptor definition array field.
 *
 * The number of entries is either calculated from length_field parameters,
 * which indicate which other field(s) contain values representing the
 * array length, or the array length is calculated from the buf_len parameter,
 * which should ultimately have been derived from the bLength field in the raw
 * descriptor data.
 *
 * \param[in] buf          Descriptor data.
 * \param[in] buf_len      Byte length of `buf`.
 * \param[in] desc         First field in the descriptor definition.
 * \param[in] array_entry  Array field to get entry count for.
 * \return Number of entries in array.
 */
static unsigned int get_array_entry_count(
		const unsigned char *buf,
		unsigned int buf_len,
		const struct desc *desc,
		const struct desc *array_entry)
{
	const struct desc *current;
	unsigned int entries = 0;

	if (array_entry->array.length_field1) {
		/* We can get the array size from the length_field1. */
		entries = get_value_from_field(buf, desc,
				array_entry->array.length_field1);

		if (array_entry->array.length_field2 != NULL) {
			/* There's a second field specifying length.  The two
			 * lengths are multiplied. */
			entries *= get_value_from_field(buf, desc,
					array_entry->array.length_field2);
		}

		/* If the bits flag is set, then the entry count so far
		 * was a bit count, and we need to get a byte count. */
		if (array_entry->array.bits) {
			entries = (entries / 8) + (entries & 0x7) ? 1 : 0;
		}
	} else {
		/* Inferred array length.  We haven't been given a field to get
		 * length from; start with the descriptor's byte-length, and
		 * subtract the sizes of all the other fields. */
		unsigned int size = buf_len;

		for (current = desc; current->field != NULL; current++) {
			if (current == array_entry)
				continue;

			if (current->array.array) {
				unsigned int count;
				/* We can't deal with two inferred-length arrays
				 * in one descriptor definition, because its
				 * an unresolvable ambiguity.  If this
				 * happens it's a flaw in the descriptor
				 * definition. */
				if (current->array.length_field1 == NULL) {
					return 0xffffffff;
				}
				count = get_array_entry_count(buf, buf_len,
						desc, current);
				if (count == 0xffffffff) {
					fprintf(stderr, "Bad descriptor definition; "
						"multiple inferred-length arrays.\n");
					exit(EXIT_FAILURE);
				}
				size -= get_entry_size(buf, desc, current) *
						count;
			} else {
				size -= get_entry_size(buf, desc, current);
			}
		}

		entries = size / get_entry_size(buf, desc, array_entry);
	}

	return entries;
}

/**
 * Get the number of characters needed to dump an array index
 *
 * \param[in] array_entries  Number of entries in array.
 * \return number of characters required to render largest possible index.
 */
static unsigned int get_char_count_for_array_index(unsigned int array_entries)
{
	/* Arrays are zero-indexed, so largest index is array_entries - 1. */
	if (array_entries > 100) {
		/* [NNN] */
		return 5;
	} else if (array_entries > 10) {
		/* [NN] */
		return 4;
	}

	/* [N] */
	return 3;
}

/**
 * Render a field's name.
 *
 * \param[in] entry       Current entry number (for arrays).
 * \param[in] entries     Entry count (for arrays).
 * \param[in] field_len   Character width of field name space for alignment.
 * \param[in] current     Descriptor definition of field to render.
 * \param[in] indent      Current indent level.
 */
static void field_render(
		unsigned int entry,
		unsigned int entries,
		unsigned int field_len,
		const struct desc *current,
		unsigned int indent)
{
	if (current->array.array) {
		unsigned int needed_chars = field_len -
				get_char_count_for_array_index(entries) -
				strlen(current->field);
		printf("%*s%s(%u)%*s", indent * 2, "",
				current->field, entry,
				needed_chars, "");
	} else {
		printf("%*s%-*s", indent * 2, "",
				field_len, current->field);
	}
}

/* Function documented in desc-dump.h */
void desc_dump(
		libusb_device_handle *dev,
		const struct desc *desc,
		const unsigned char *buf,
		unsigned int buf_len,
		unsigned int indent)
{
	unsigned int entry;
	unsigned int entries;
	unsigned int needed_chars;
	unsigned int current_size;
	unsigned int field_len = 18;
	const struct desc *current;
	size_t offset = 0;

	/* Find the buffer length, if we've been instructed to read it from
	 * the first field. */
	if ((buf_len == DESC_BUF_LEN_FROM_BUF) && (desc != NULL)) {
		buf_len = get_n_bytes_as_ull(buf, offset, desc->size);
	}

	/* Increase `field_len` to be sufficient for character length of
	 * longest field name for this descriptor. */
	for (current = desc; current->field != NULL; current++) {
		needed_chars = 0;
		if (current->array.array) {
			entries = get_array_entry_count(buf, buf_len,
					desc, current);
			needed_chars = get_char_count_for_array_index(entries);
		}
		if (strlen(current->field) + needed_chars > field_len) {
			field_len = strlen(current->field) + needed_chars;
		}
	}

	/* Step through each field, and dump it. */
	for (current = desc; current->field != NULL; current++) {
		entries = 1;
		if (current->array.array) {
			/* Array type fields may have more than one entry. */
			entries = get_array_entry_count(buf, buf_len,
					desc, current);
		}

		current_size = get_entry_size(buf, desc, current);

		for (entry = 0; entry < entries; entry++) {
			/* Check there's enough data in buf for this entry. */
			if (offset + current_size > buf_len) {
				unsigned int i;
				printf("%*sWarning: Length insufficient for "
						"descriptor type.\n",
						(indent - 1) * 2, "");
				for (i = offset; i < buf_len; i++) {
					printf("%02x ", buf[i]);
				}
				printf("\n");
				return;
			}

			/* Dump the field name */
			if (current->type != DESC_EXTENSION) {
				field_render(entry, entries, field_len,
						current, indent);
			}

			/* Dump the value */
			value_renderer(dev, current, current_size, buf, buf_len,
					desc, indent, offset);

			if (current->type == DESC_EXTENSION) {
				/* A desc extension consumes all remaining
				 * value buffer. */
				offset = buf_len;
			} else {
				/* Advance offset in buffer */
				offset += current_size;
			}
		}
	}

	/* Check for junk at end of descriptor. */
	if (offset < buf_len) {
		unsigned int i;
		printf("%*sWarning: Junk at end of descriptor (%zu bytes):\n",
				(indent - 1) * 2, "", buf_len - offset);
		printf("%*s", indent * 2, "");
		for (i = offset; i < buf_len; i++) {
			printf("%02x ", buf[i]);
		}
		printf("\n");
	}
}
