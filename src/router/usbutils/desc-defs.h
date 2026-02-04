// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB descriptor definitions
 *
 * Copyright (C) 2017-2018 Michael Drake <michael.drake@codethink.co.uk>
 */

#ifndef _DESC_DEFS_H
#define _DESC_DEFS_H

/* ---------------------------------------------------------------------- */

/**
 * Descriptor field value type.
 *
 * Note that there are more types here than exist in the descriptor definitions
 * in the specifications.  This is because the type here is used to do `lsusb`
 * specific rendering of certain fields.
 *
 * Note that the use of certain types mandates the setting of various entries
 * in the type-specific anonymous union in `struct desc`.
 */
enum desc_type {
	DESC_CONSTANT,       /** Plain numerical value; no annotation. */
	DESC_NUMBER,         /** Plain numerical value; no annotation. */
	DESC_NUMBER_POSTFIX, /**< Number with a postfix string. */
	DESC_BITMAP,         /**< Plain hex rendered value; no annotation. */
	DESC_BCD,            /**< Binary coded decimal */
	DESC_BMCONTROL_1,    /**< UAC1 style bmControl field */
	DESC_BMCONTROL_2,    /**< UAC2/UAC3 style bmControl field */
	DESC_STR_DESC_INDEX, /**< String index. */
	DESC_CS_STR_DESC_ID, /**< UAC3 style class-specific string request. */
	DESC_TERMINAL_STR,   /**< Audio terminal string. */
	DESC_BITMAP_STRINGS, /**< Bitfield with string per bit. */
	DESC_NUMBER_STRINGS, /**< Use for enum-style value to string. */
	DESC_EXTENSION,      /**< Various possible descriptor extensions. */
	DESC_SNOWFLAKE,  /**< Value with custom annotation callback function. */
};

/**
 * Callback function for the DESC_SNOWFLAKE descriptor field value type.
 *
 * This is used when some special rendering of the value is required, which
 * is specific to the field in question, so no generic type's rendering is
 * suitable.
 *
 * The core descriptor dumping code will have already dumped the numerical
 * value for the field, but not the trailing newline character.  It is up
 * to the callback function to ensure it always finishes by writing a '\n'
 * character to stdout.
 *
 * \param[in] value    The value to dump a human-readable representation of.
 * \param[in] indent   The current indent level.
 */
typedef void (*desc_snowflake_dump_fn)(
		unsigned long long value,
		unsigned int indent);


/**
 * Descriptor field definition.
 *
 * Whole descriptors can be defined as NULL terminated arrays of these
 * structures.
 */
struct desc {
	const char *field;   /**< Field's name */
	unsigned int size;   /**< Byte size of field, if (size_field == NULL) */
	const char *size_field; /**< Name of field specifying field size. */
	enum desc_type type; /**< Field's value type. */

	/** Anonymous union containing type-specific data. */
	union {
		/**
		 * Corresponds to types DESC_BMCONTROL_1 and DESC_BMCONTROL_2.
		 *
		 * Must be a NULL terminated array of '\0' terminated strings.
		 */
		const char * const *bmcontrol;
		/** Corresponds to type DESC_BITMAP_STRINGS */
		struct {
			/** Must contain '\0' terminated strings. */
			const char * const *strings;
			/** Number of strings in strings array. */
			unsigned int count;
		} bitmap_strings;
		/**
		 * Corresponds to type DESC_NUMBER_STRINGS.
		 *
		 * Must be a NULL terminated array of '\0' terminated strings.
		 */
		const char * const *number_strings;
		/**
		 * Corresponds to type DESC_NUMBER_POSTFIX.
		 *
		 * Must be a '\0' terminated string.
		 */
		const char *number_postfix;
		/**
		 * Corresponds to type DESC_EXTENSION.
		 *
		 * This allows the value of this field to be processed by
		 * another descriptor definition.  The definition used to
		 * process the value of this field can be controlled by
		 * the value of another field.
		 */
		struct {
			/**
			 * Name of field specifying descriptor type to select.
			 */
			const char *type_field;
			/**
			 * Array of descriptor definitions and their
			 * associated types values.  Array must be terminated
			 * by entry with NULL `desc` member.
			 */
			const struct desc_ext {
				/**
				 * Array of descriptor field definitions.
				 * Terminated by entry with NULL `field` member.
				 */
				const struct desc *desc;
				/**
				 * Type value for this descriptor definition.
				 * If it matches the type read from the
				 * field `type_field`, then this descriptor
				 * definition will be used to decode this value.
				 */
				unsigned int type;
			} *d;
		} extension;
		/**
		 * Corresponds to type DESC_SNOWFLAKE.
		 *
		 * Callback function called to annotate snowflake value type.
		 */
		desc_snowflake_dump_fn snowflake;
	};

	/** Grouping of array-specific fields. */
	struct {
		bool array; /**< True if entry is an array. */
		bool bits;  /**< True if array length is specified in bits */
		/** Name of field specifying the array entry count. */
		const char *length_field1;
		/** Name of field specifying multiplier for array entry count. */
		const char *length_field2;
	} array;
};

/* ---------------------------------------------------------------------- */

/* Undefined descriptor */
extern const struct desc desc_undefined[];

/* Audio Control (AC) descriptor definitions */
extern const struct desc * const desc_audio_ac_header[3];
extern const struct desc * const desc_audio_ac_effect_unit[3];
extern const struct desc * const desc_audio_ac_input_terminal[3];
extern const struct desc * const desc_audio_ac_output_terminal[3];
extern const struct desc * const desc_audio_ac_extended_terminal[3];
extern const struct desc * const desc_audio_ac_power_domain[3];
extern const struct desc * const desc_audio_ac_mixer_unit[3];
extern const struct desc * const desc_audio_ac_selector_unit[3];
extern const struct desc * const desc_audio_ac_processing_unit[3];
extern const struct desc * const desc_audio_ac_feature_unit[3];
extern const struct desc * const desc_audio_ac_extension_unit[3];
extern const struct desc * const desc_audio_ac_clock_source[3];
extern const struct desc * const desc_audio_ac_clock_selector[3];
extern const struct desc * const desc_audio_ac_clock_multiplier[3];
extern const struct desc * const desc_audio_ac_sample_rate_converter[3];

/* Audio Streaming (AS) descriptor definitions */
extern const struct desc * const desc_audio_as_interface[3];
extern const struct desc * const desc_audio_as_isochronous_audio_data_endpoint[3];

/* Device Capability (DC) descriptor definitions */
extern const struct desc desc_usb3_dc_configuration_summary[];

/* ---------------------------------------------------------------------- */

#endif /* _DESC_DEFS_H */
