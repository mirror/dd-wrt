// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * USB descriptor definitions
 *
 * Copyright (C) 2017-2018 Michael Drake <michael.drake@codethink.co.uk>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "desc-defs.h"

/** Macro for computing number of elements in array. */
#define ARRAY_LEN(a) ((sizeof(a)) / (sizeof(a[0])))

/**
 * Undefined descriptor
 *
 * Ensures remaining data is dumped as garbage at end of descriptor.
 */
const struct desc desc_undefined[] = {
	{ .field = NULL }
};

/** USB Audio Device Class 1 Channel Names.  (Note: Order matters.) */
static const char * const uac1_channel_names[] = {
	"Left Front (L)", "Right Front (R)", "Center Front (C)",
	"Low Frequency Enhancement (LFE)", "Left Surround (LS)",
	"Right Surround (RS)", "Left of Center (LC)", "Right of Center (RC)",
	"Surround (S)", "Side Left (SL)", "Side Right (SR)", "Top (T)"
};

/** USB Audio Device Class 2 Channel Names.  (Note: Order matters.) */
static const char * const uac2_channel_names[] = {
	"Front Left (FL)", "Front Right (FR)", "Front Center (FC)",
	"Low Frequency Effects (LFE)", "Back Left (BL)", "Back Right (BR)",
	"Front Left of Center (FLC)", "Front Right of Center (FRC)",
	"Back Center (BC)", "Side Left (SL)", "Side Right (SR)",
	"Top Center (TC)", "Top Front Left (TFL)", "Top Front Center (TFC)",
	"Top Front Right (TFR)", "Top Back Left (TBL)", "Top Back Center (TBC)",
	"Top Back Right (TBR)", "Top Front Left of Center (TFLC)",
	"Top Front Right of Center (TFRC)", "Left Low Frequency Effects (LLFE)",
	"Right Low Frequency Effects (RLFE)", "Top Side Left (TSL)",
	"Top Side Right (TSR)", "Bottom Center (BC)",
	"Back Left of Center (BLC)", "Back Right of Center (BRC)"
};

/** Audio Control Interface Header bmControls; Human readable bit meanings. */
static const char * const uac2_interface_header_bmcontrols[] = {
	[0] = "Latency control",
	[1] = NULL
};

/** UAC1: 4.3.2 Class-Specific AC Interface Descriptor; Table 4-2. */
static const struct desc desc_audio_1_ac_header[] = {
	{ .field = "bcdADC",        .size = 2, .type = DESC_BCD },
	{ .field = "wTotalLength",  .size = 2, .type = DESC_CONSTANT },
	{ .field = "bInCollection", .size = 1, .type = DESC_CONSTANT },
	{ .field = "baInterfaceNr", .size = 1, .type = DESC_NUMBER,
			.array = { .array = true } },
	{ .field = NULL }
};

/** UAC2: 4.7.2 Class-Specific AC Interface Descriptor; Table 4-5. */
static const struct desc desc_audio_2_ac_header[] = {
	{ .field = "bcdADC",       .size = 2, .type = DESC_BCD },
	{ .field = "bCategory",    .size = 1, .type = DESC_CONSTANT },
	{ .field = "wTotalLength", .size = 2, .type = DESC_NUMBER },
	{ .field = "bmControls",   .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_interface_header_bmcontrols },
	{ .field = NULL }
};

/** UAC3: 4.5.2 Class-Specific AC Interface Descriptor; Table 4-15. */
static const struct desc desc_audio_3_ac_header[] = {
	{ .field = "bCategory",    .size = 1, .type = DESC_CONSTANT },
	{ .field = "wTotalLength", .size = 2, .type = DESC_NUMBER },
	{ .field = "bmControls",   .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_interface_header_bmcontrols },
	{ .field = NULL }
};

/** AudioControl Header descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_header[3] = {
	desc_audio_1_ac_header,
	desc_audio_2_ac_header,
	desc_audio_3_ac_header,
};

/** UAC2: 4.7.2.10 Effect Unit Descriptor; Table 4-15. */
static const struct desc desc_audio_2_ac_effect_unit[] = {
	{ .field = "bUnitID",     .size = 1, .type = DESC_NUMBER },
	{ .field = "wEffectType", .size = 2, .type = DESC_CONSTANT },
	{ .field = "bSourceID",   .size = 1, .type = DESC_CONSTANT },
	{ .field = "bmaControls", .size = 4, .type = DESC_BITMAP,
			.array = { .array = true } },
	{ .field = "iEffects",    .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.9 Effect Unit Descriptor; Table 4-33. */
static const struct desc desc_audio_3_ac_effect_unit[] = {
	{ .field = "bUnitID",          .size = 1, .type = DESC_NUMBER },
	{ .field = "wEffectType",      .size = 2, .type = DESC_CONSTANT },
	{ .field = "bSourceID",        .size = 1, .type = DESC_CONSTANT },
	{ .field = "bmaControls",      .size = 4, .type = DESC_BITMAP,
			.array = { .array = true } },
	{ .field = "wEffectsDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Effect Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_effect_unit[3] = {
	NULL, /* UAC1 not supported */
	desc_audio_2_ac_effect_unit,
	desc_audio_3_ac_effect_unit,
};

/** UAC2 Input Terminal bmControls; Human readable bit meanings. */
static const char * const uac2_input_term_bmcontrols[] = {
	[0] = "Copy Protect",
	[1] = "Connector",
	[2] = "Overload",
	[3] = "Cluster",
	[4] = "Underflow",
	[5] = "Overflow",
	[6] = NULL
};

/** UAC3 Input Terminal bmControls; Human readable bit meanings. */
static const char * const uac3_input_term_bmcontrols[] = {
	[0] = "Insertion",
	[1] = "Overload",
	[2] = "Underflow",
	[3] = "Overflow",
	[4] = "Underflow",
	[5] = NULL
};

/** UAC1: 4.3.2.1 Input Terminal Descriptor; Table 4-3. */
static const struct desc desc_audio_1_ac_input_terminal[] = {
	{ .field = "bTerminalID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",  .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal", .size = 1, .type = DESC_CONSTANT },
	{ .field = "bNrChannels",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wChannelConfig", .size = 2, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac1_channel_names, .count = 12 } },
	{ .field = "iChannelNames",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "iTerminal",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.4 Input Terminal Descriptor; Table 4-9. */
static const struct desc desc_audio_2_ac_input_terminal[] = {
	{ .field = "bTerminalID",     .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",   .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal",  .size = 1, .type = DESC_CONSTANT },
	{ .field = "bCSourceID",      .size = 1, .type = DESC_CONSTANT },
	{ .field = "bNrChannels",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bmChannelConfig", .size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_channel_names, .count = 26 } },
	{ .field = "iChannelNames",   .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bmControls",      .size = 2, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_input_term_bmcontrols },
	{ .field = "iTerminal",       .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.1 Input Terminal Descriptor; Table 4-16. */
static const struct desc desc_audio_3_ac_input_terminal[] = {
	{ .field = "bTerminalID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",      .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceID",         .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",         .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_input_term_bmcontrols },
	{ .field = "wClusterDescrID",    .size = 2, .type = DESC_NUMBER },
	{ .field = "wExTerminalDescrID", .size = 2, .type = DESC_NUMBER },
	{ .field = "wConnectorsDescrID", .size = 2, .type = DESC_NUMBER },
	{ .field = "wTerminalDescrStr",  .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Input Terminal descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_input_terminal[3] = {
	desc_audio_1_ac_input_terminal,
	desc_audio_2_ac_input_terminal,
	desc_audio_3_ac_input_terminal,
};

/** UAC2 Output Terminal bmControls; Human readable bit meanings. */
static const char * const uac2_output_term_bmcontrols[] = {
	[0] = "Copy Protect",
	[1] = "Connector",
	[2] = "Overload",
	[3] = "Underflow",
	[4] = "Overflow",
	[5] = NULL
};

/** UAC3 Output Terminal bmControls; Human readable bit meanings. */
static const char * const uac3_output_term_bmcontrols[] = {
	[0] = "Insertion",
	[1] = "Overload",
	[2] = "Underflow",
	[3] = "Overflow",
	[4] = NULL
};

/** UAC1: 4.3.2.2 Output Terminal Descriptor; Table 4-4. */
static const struct desc desc_audio_1_ac_output_terminal[] = {
	{ .field = "bTerminalID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",  .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal", .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",      .size = 1, .type = DESC_NUMBER },
	{ .field = "iTerminal",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.5 Output Terminal Descriptor; Table 4-10. */
static const struct desc desc_audio_2_ac_output_terminal[] = {
	{ .field = "bTerminalID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",  .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal", .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",      .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceID",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",     .size = 2, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_output_term_bmcontrols },
	{ .field = "iTerminal",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.2 Output Terminal Descriptor; Table 4-17. */
static const struct desc desc_audio_3_ac_output_terminal[] = {
	{ .field = "bTerminalID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "wTerminalType",      .size = 2, .type = DESC_TERMINAL_STR },
	{ .field = "bAssocTerminal",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",          .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceID",         .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",         .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_output_term_bmcontrols },
	{ .field = "wExTerminalDescrID", .size = 2, .type = DESC_NUMBER },
	{ .field = "wConnectorsDescrID", .size = 2, .type = DESC_NUMBER },
	{ .field = "wTerminalDescrStr",  .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Output Terminal descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_output_terminal[3] = {
	desc_audio_1_ac_output_terminal,
	desc_audio_2_ac_output_terminal,
	desc_audio_3_ac_output_terminal,
};

/** UAC3: 4.5.2.3.1 Extended Terminal Header Descriptor; Table 4-18. */
static const struct desc desc_audio_3_ac_extended_terminal_header[] = {
	{ .field = "wDescriptorID", .size = 2, .type = DESC_NUMBER },
	{ .field = "bNrChannels",   .size = 1, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** Extended Terminal descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_extended_terminal[3] = {
	NULL, /* UAC1 not supported */
	NULL, /* UAC2 not supported */
	desc_audio_3_ac_extended_terminal_header,
};

/** UAC3: 4.5.2.15 Power Domain Descriptor; Table 4-46. */
static const struct desc desc_audio_3_ac_power_domain[] = {
	{ .field = "bPowerDomainID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "waRecoveryTime(1)", .size = 2, .type = DESC_NUMBER },
	{ .field = "waRecoveryTime(2)", .size = 2, .type = DESC_NUMBER },
	{ .field = "bNrEntities",       .size = 1, .type = DESC_NUMBER },
	{ .field = "baEntityID",        .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrEntities" } },
	{ .field = "wPDomainDescrStr",  .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Power Domain descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_power_domain[3] = {
	NULL, /* UAC1 not supported */
	NULL, /* UAC2 not supported */
	desc_audio_3_ac_power_domain,
};

/** UAC2 Mixer Unit bmControls; Human readable bit meanings. */
static const char * const uac2_mixer_unit_bmcontrols[] = {
	[0] = "Cluster",
	[1] = "Underflow",
	[2] = "Overflow",
	[3] = NULL
};

/** UAC3 Mixer Unit bmControls; Human readable bit meanings. */
static const char * const uac3_mixer_unit_bmcontrols[] = {
	[0] = "Underflow",
	[1] = "Overflow",
	[2] = NULL
};

/** UAC1: 4.3.2.3 Mixer Unit Descriptor; Table 4-5. */
static const struct desc desc_audio_1_ac_mixer_unit[] = {
	{ .field = "bUnitID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",      .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",     .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wChannelConfig", .size = 2, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac1_channel_names, .count = 12 } },
	{ .field = "iChannelNames",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bmControls",     .size = 1, .type = DESC_BITMAP,
			.array = { .array = true, .bits = true,
					.length_field1 = "bNrInPins",
					.length_field2 = "bNrChannels" } },
	{ .field = "iMixer",         .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.6 Mixer Unit Descriptor; Table 4-11. */
static const struct desc desc_audio_2_ac_mixer_unit[] = {
	{ .field = "bUnitID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",      .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",     .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",    .size = 1, .type = DESC_NUMBER },
	{ .field = "bmChannelConfig",.size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_channel_names, .count = 26 } },
	{ .field = "iChannelNames",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bmMixerControls",.size = 1, .type = DESC_BITMAP,
			.array = { .array = true, .bits = true,
					.length_field1 = "bNrInPins",
					.length_field2 = "bNrChannels" } },
	{ .field = "bmControls",     .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_mixer_unit_bmcontrols },
	{ .field = "iMixer",         .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.5 Mixer Unit Descriptor; Table 4-29. */
static const struct desc desc_audio_3_ac_mixer_unit[] = {
	{ .field = "bUnitID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",      .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",     .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "wClusterDescrID",.size = 2, .type = DESC_NUMBER },
	{ .field = "bmMixerControls",.size = 1, .type = DESC_BITMAP,
			.array = { .array = true } },
	{ .field = "bmControls",     .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_mixer_unit_bmcontrols },
	{ .field = "wMixerDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Mixer Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_mixer_unit[3] = {
	desc_audio_1_ac_mixer_unit,
	desc_audio_2_ac_mixer_unit,
	desc_audio_3_ac_mixer_unit,
};

/** Selector Unit bmControls; Human readable bit meanings. */
static const char * const uac2_selector_unit_bmcontrols[] = {
	[0] = "Selector",
	[1] = NULL
};

/** UAC1: 4.3.2.4 Selector Unit Descriptor; Table 4-6. */
static const struct desc desc_audio_1_ac_selector_unit[] = {
	{ .field = "bUnitID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",  .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID", .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "iSelector",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.7 Selector Unit Descriptor; Table 4-12. */
static const struct desc desc_audio_2_ac_selector_unit[] = {
	{ .field = "bUnitID",    .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",  .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID", .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bmControls", .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_selector_unit_bmcontrols },
	{ .field = "iSelector",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.6 Selector Unit Descriptor; Table 4-30. */
static const struct desc desc_audio_3_ac_selector_unit[] = {
	{ .field = "bUnitID",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",         .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",        .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bmControls",        .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_selector_unit_bmcontrols },
	{ .field = "wSelectorDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Selector Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_selector_unit[3] = {
	desc_audio_1_ac_selector_unit,
	desc_audio_2_ac_selector_unit,
	desc_audio_3_ac_selector_unit,
};

/** UAC1: Table A-7: Processing Unit Process Types */
static const char * const uac1_proc_unit_types[] = {
	[0] = "Undefined",
	[1] = "Up/Down-mix",
	[2] = "Dolby Prologic",
	[3] = "3D Stereo Extender",
	[4] = "Reverberation",
	[5] = "Chorus",
	[6] = "Dyn Range Comp",
	[7] = NULL
};

/** UAC1: Up/Down-mix and Dolby Prologic proc unit descriptor extensions Table 4-9, Table 4-10. */
static const struct desc desc_audio_1_ac_proc_unit_extended[] = {
	{ .field = "bNrModes",         .size = 1, .type = DESC_NUMBER },
	{ .field = "waModes",          .size = 2, .type = DESC_BITMAP,
			.array = { .array = true, .length_field1 = "bNrModes" } },
	{ .field = NULL }
};

/** UAC1: Table A-7: Processing Unit Process Types */
static const struct desc_ext desc_audio_1_ac_proc_unit_specific[] = {
	{ .type = 1, .desc = desc_audio_1_ac_proc_unit_extended },
	{ .type = 2, .desc = desc_audio_1_ac_proc_unit_extended },
	{ .desc = NULL }
};

/** UAC1: 4.3.2.6 Processing Unit Descriptor; Table 4-8. */
static const struct desc desc_audio_1_ac_processing_unit[] = {
	{ .field = "bUnitID",          .size = 1, .type = DESC_NUMBER },
	{ .field = "wProcessType",     .size = 2, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac1_proc_unit_types },
	{ .field = "bNrInPins",        .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",       .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",      .size = 1, .type = DESC_NUMBER },
	{ .field = "wChannelConfig",   .size = 2, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac1_channel_names, .count = 12 } },
	{ .field = "iChannelNames",    .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bControlSize",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",       .size = 1, .type = DESC_BITMAP,
			.array = { .array = true, .length_field1 = "bControlSize" } },
	{ .field = "iProcessing",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "Process-specific", .size = 1, .type = DESC_EXTENSION,
		.extension = { .type_field = "wProcessType", .d = desc_audio_1_ac_proc_unit_specific } },
	{ .field = NULL }
};

/** UAC2: 4.7.2.11.1 Up/Down-mix Processing Unit Descriptor; Table 4-21. */
static const struct desc desc_audio_2_ac_proc_unit_up_down_mix[] = {
	{ .field = "bNrModes",         .size = 1, .type = DESC_NUMBER },
	{ .field = "daModes",          .size = 4, .type = DESC_BITMAP,
			.array = { .array = true, .length_field1 = "bNrModes" } },
	{ .field = NULL }
};

/** UAC2: 4.7.2.11.2 Dolby prologic Processing Unit Descriptor; Table 4-22. */
static const struct desc desc_audio_2_ac_proc_unit_dolby_prologic[] = {
	{ .field = "bNrModes",         .size = 1, .type = DESC_NUMBER },
	{ .field = "daModes",          .size = 4, .type = DESC_BITMAP,
			.array = { .array = true, .length_field1 = "bNrModes" } },
	{ .field = NULL }
};

/** UAC2: Table A-12: Processing Unit Process Types */
static const struct desc_ext desc_audio_2_ac_proc_unit_specific[] = {
	{ .type = 1, .desc = desc_audio_2_ac_proc_unit_up_down_mix },
	{ .type = 2, .desc = desc_audio_2_ac_proc_unit_dolby_prologic },
	{ .desc = NULL }
};

/** UAC2: Table A-12: Processing Unit Process Types */
static const char * const uac2_proc_unit_types[] = {
	[0] = "Undefined",
	[1] = "Up/Down-mix",
	[2] = "Dolby Prologic",
	[3] = "Stereo Extender",
	[4] = NULL
};

/** UAC2: 4.7.2.11 Processing Unit Descriptor; Table 4-20. */
static const struct desc desc_audio_2_ac_processing_unit[] = {
	{ .field = "bUnitID",          .size = 1, .type = DESC_NUMBER },
	{ .field = "wProcessType",     .size = 2, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac2_proc_unit_types },
	{ .field = "bNrInPins",        .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",       .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",      .size = 1, .type = DESC_NUMBER },
	{ .field = "bmChannelConfig",  .size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_channel_names, .count = 26 } },
	{ .field = "iChannelNames",    .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bmControls",       .size = 2, .type = DESC_BITMAP },
	{ .field = "iProcessing",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "Process-specific", .size = 1, .type = DESC_EXTENSION,
		.extension = { .type_field = "wProcessType", .d = desc_audio_2_ac_proc_unit_specific } },
	{ .field = NULL }
};

/** Processor unit Up/Down-mix bmControls; Human readable bit meanings. */
static const char * const uac3_proc_unit_up_down_mix_bmcontrols[] = {
	[0] = "Mode Select",
	[1] = "Underflow",
	[2] = "Overflow",
	[3] = NULL
};

/** UAC3: 4.5.2.10.1 Up/Down-mix Processing Unit Descriptor; Table 4-39. */
static const struct desc desc_audio_3_ac_proc_unit_up_down_mix[] = {
	{ .field = "bmControls",       .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_proc_unit_up_down_mix_bmcontrols },
	{ .field = "bNrModes",         .size = 1, .type = DESC_NUMBER },
	{ .field = "waClusterDescrID", .size = 2, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrModes" } },
	{ .field = NULL }
};

/** Processor unit stereo extender bmControls; Human readable bit meanings. */
static const char * const uac3_proc_unit_stereo_extender_bmcontrols[] = {
	[0] = "Width",
	[1] = "Underflow",
	[2] = "Overflow",
	[3] = NULL
};

/** UAC3: 4.5.2.10.2 Stereo Extender Processing Unit Descriptor; Table 4-40. */
static const struct desc desc_audio_3_ac_proc_unit_stereo_extender[] = {
	{ .field = "bmControls", .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_proc_unit_stereo_extender_bmcontrols },
	{ .field = NULL }
};

/** UAC3: 4.5.2.10.3 Multi Func Proc Unit Descriptor; Algorithms; Table 4-41. */
static const char * const uac3_proc_unit_multi_func_algorithms[] = {
	[0] = "Algorithm Undefined.",
	[1] = "Beam Forming.",
	[2] = "Acoustic Echo Cancellation.",
	[3] = "Active Noise Cancellation.",
	[4] = "Blind Source Separation.",
	[5] = "Noise Suppression/Reduction.",
	[6] = NULL
};

/** Processor unit Multi Func bmControls; Human readable bit meanings. */
static const char * const uac3_proc_unit_multi_func_bmcontrols[] = {
	[0] = "Underflow",
	[1] = "Overflow",
	[2] = NULL
};

/** UAC3: 4.5.2.10.3 Multi Function Processing Unit Descriptor; Table 4-41. */
static const struct desc desc_audio_3_ac_proc_unit_multi_function[] = {
	{ .field = "bmControls",       .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_proc_unit_multi_func_bmcontrols },
	{ .field = "wClusterDescrID",  .size = 2, .type = DESC_NUMBER, },
	{ .field = "bmAlgorithms", .size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = {
				.strings = uac3_proc_unit_multi_func_algorithms,
				.count = 6
			} },
	{ .field = NULL }
};

/** UAC3: Table A-20: Processing Unit Process Types */
static const char * const uac3_proc_unit_types[] = {
	[0] = "Undefined",
	[1] = "Up/Down-mix",
	[2] = "Stereo Extender",
	[3] = "Multi-Function",
	[4] = NULL
};

/** UAC3: Table A-20: Processing Unit Process Types */
static const struct desc_ext desc_audio_3_ac_proc_unit_specific[] = {
	{ .type = 1, .desc = desc_audio_3_ac_proc_unit_up_down_mix },
	{ .type = 2, .desc = desc_audio_3_ac_proc_unit_stereo_extender },
	{ .type = 3, .desc = desc_audio_3_ac_proc_unit_multi_function },
	{ .desc = NULL }
};

/** UAC3: 4.5.2.10 Processing Unit Descriptor; Table 4-38. */
static const struct desc desc_audio_3_ac_processing_unit[] = {
	{ .field = "bUnitID",             .size = 1, .type = DESC_NUMBER },
	{ .field = "wProcessType",        .size = 2, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac3_proc_unit_types },
	{ .field = "bNrInPins",           .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",          .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "wProcessingDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = "Process-specific",    .size = 1, .type = DESC_EXTENSION,
		.extension = { .type_field = "wProcessType", .d = desc_audio_3_ac_proc_unit_specific } },
	{ .field = NULL }
};

/** Processing Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_processing_unit[3] = {
	desc_audio_1_ac_processing_unit,
	desc_audio_2_ac_processing_unit,
	desc_audio_3_ac_processing_unit,
};

/** Audio Control Feature Unit bmControls; Human readable bit meanings. */
static const char * const uac_feature_unit_bmcontrols[] = {
	[ 0] = "Mute",
	[ 1] = "Volume",
	[ 2] = "Bass",
	[ 3] = "Mid",
	[ 4] = "Treble",
	[ 5] = "Graphic Equalizer",
	[ 6] = "Automatic Gain",
	[ 7] = "Delay",
	[ 8] = "Bass Boost",
	[ 9] = "Loudness",
	[10] = "Input gain",
	[11] = "Input gain pad",
	[12] = "Phase inverter",
	[13] = NULL
};

/** UAC1: 4.3.2.5 Feature Unit Descriptor; Table 4-7. */
static const struct desc desc_audio_1_ac_feature_unit[] = {
	{ .field = "bUnitID",      .size = 1,                    .type = DESC_NUMBER },
	{ .field = "bSourceID",    .size = 1,                    .type = DESC_CONSTANT },
	{ .field = "bControlSize", .size = 1,                    .type = DESC_NUMBER },
	{ .field = "bmaControls",  .size_field = "bControlSize", .type = DESC_BMCONTROL_1,
			.bmcontrol = uac_feature_unit_bmcontrols, .array = { .array = true } },
	{ .field = "iFeature",     .size = 1,                    .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.8 Feature Unit Descriptor; Table 4-13. */
static const struct desc desc_audio_2_ac_feature_unit[] = {
	{ .field = "bUnitID",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",   .size = 1, .type = DESC_CONSTANT },
	{ .field = "bmaControls", .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac_feature_unit_bmcontrols, .array = { .array = true } },
	{ .field = "iFeature",    .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.7 Feature Unit Descriptor; Table 4-31. */
static const struct desc desc_audio_3_ac_feature_unit[] = {
	{ .field = "bUnitID",          .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "bmaControls",      .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac_feature_unit_bmcontrols, .array = { .array = true } },
	{ .field = "wFeatureDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Feature Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_feature_unit[3] = {
	desc_audio_1_ac_feature_unit,
	desc_audio_2_ac_feature_unit,
	desc_audio_3_ac_feature_unit,
};

/** UAC2 Extension Unit bmControls; Human readable bit meanings. */
static const char * const uac2_extension_unit_bmcontrols[] = {
	[0] = "Enable",
	[1] = "Cluster",
	[2] = "Underflow",
	[3] = "Overflow",
	[4] = NULL
};

/** UAC3 Extension Unit bmControls; Human readable bit meanings. */
static const char * const uac3_extension_unit_bmcontrols[] = {
	[0] = "Underflow",
	[1] = "Overflow",
	[2] = NULL
};

/** UAC1: 4.3.2.7 Extension Unit Descriptor; Table 4-15. */
static const struct desc desc_audio_1_ac_extension_unit[] = {
	{ .field = "bUnitID",        .size = 1, .type = DESC_NUMBER },
	{ .field = "wExtensionCode", .size = 2, .type = DESC_CONSTANT },
	{ .field = "bNrInPins",      .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",     .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",    .size = 1, .type = DESC_NUMBER },
	{ .field = "wChannelConfig", .size = 2, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac1_channel_names, .count = 12 } },
	{ .field = "iChannelNames",  .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bControlSize",   .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",     .size = 1, .type = DESC_BITMAP,
			.array = { .array = true, .length_field1 = "bControlSize" } },
	{ .field = "iExtension",     .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC2: 4.7.2.12 Extension Unit Descriptor; Table 4-24. */
static const struct desc desc_audio_2_ac_extension_unit[] = {
	{ .field = "bUnitID",         .size = 1, .type = DESC_NUMBER },
	{ .field = "wExtensionCode",  .size = 2, .type = DESC_CONSTANT },
	{ .field = "bNrInPins",       .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",      .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bNrChannels",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bmChannelConfig", .size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_channel_names, .count = 26 } },
	{ .field = "iChannelNames",   .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = "bmControls",      .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_extension_unit_bmcontrols },
	{ .field = "iExtension",      .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.11 Extension Unit Descriptor; Table 4-42. */
static const struct desc desc_audio_3_ac_extension_unit[] = {
	{ .field = "bUnitID",            .size = 1, .type = DESC_NUMBER },
	{ .field = "wExtensionCode",     .size = 2, .type = DESC_CONSTANT },
	{ .field = "bNrInPins",          .size = 1, .type = DESC_NUMBER },
	{ .field = "baSourceID",         .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "wExtensionDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = "bmControls",         .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_extension_unit_bmcontrols },
	{ .field = "wClusterDescrID",    .size = 2, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** Extension Unit descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_extension_unit[3] = {
	desc_audio_1_ac_extension_unit,
	desc_audio_2_ac_extension_unit,
	desc_audio_3_ac_extension_unit,
};

/** UAC2 Clock Source bmControls; Human readable bit meanings. */
static const char * const uac2_clock_source_bmcontrols[] = {
	[0] = "Clock Frequency",
	[1] = "Clock Validity",
	[2] = NULL
};

static const char * const uac2_clk_src_bmattr[] = {
	[0] = "External",
	[1] = "Internal fixed",
	[2] = "Internal variable",
	[3] = "Internal programmable"
};

static const char * const uac3_clk_src_bmattr[] = {
	[0] = "External",
	[1] = "Internal",
	[2] = "(asynchronous)",
	[3] = "(synchronized to SOF)"
};

/** Special rendering function for UAC2 clock source bmAttributes */
static void desc_snowflake_dump_uac2_clk_src_bmattr(
		unsigned long long value,
		unsigned int indent)
{
	printf(" %s clock %s\n",
			uac2_clk_src_bmattr[value & 0x3],
			(value & 0x4) ? uac3_clk_src_bmattr[3] : "");
}

/** Special rendering function for UAC3 clock source bmAttributes */
static void desc_snowflake_dump_uac3_clk_src_bmattr(
		unsigned long long value,
		unsigned int indent)
{
	printf(" %s clock %s\n",
			uac3_clk_src_bmattr[(value & 0x1)],
			uac3_clk_src_bmattr[0x2 | ((value & 0x2) >> 1)]);
}

/** UAC2: 4.7.2.1 Clock Source Descriptor; Table 4-6. */
static const struct desc desc_audio_2_ac_clock_source[] = {
	{ .field = "bClockID",       .size = 1, .type = DESC_CONSTANT },
	{ .field = "bmAttributes",   .size = 1, .type = DESC_SNOWFLAKE,
			.snowflake = desc_snowflake_dump_uac2_clk_src_bmattr },
	{ .field = "bmControls",     .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_source_bmcontrols },
	{ .field = "bAssocTerminal", .size = 1, .type = DESC_CONSTANT },
	{ .field = "iClockSource",   .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.12 Clock Source Descriptor; Table 4-43. */
static const struct desc desc_audio_3_ac_clock_source[] = {
	{ .field = "bClockID",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bmAttributes",       .size = 1, .type = DESC_SNOWFLAKE,
			.snowflake = desc_snowflake_dump_uac3_clk_src_bmattr },
	{ .field = "bmControls",         .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_source_bmcontrols },
	{ .field = "bReferenceTerminal", .size = 1, .type = DESC_NUMBER },
	{ .field = "wClockSourceStr",    .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Clock Source descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_clock_source[3] = {
	NULL, /* UAC1 not supported */
	desc_audio_2_ac_clock_source,
	desc_audio_3_ac_clock_source,
};

/** UAC2 Clock Selector bmControls; Human readable bit meanings. */
static const char * const uac2_clock_selector_bmcontrols[] = {
	[0] = "Clock Selector",
	[1] = NULL
};

/** UAC2: 4.7.2.2 Clock Selector Descriptor; Table 4-7. */
static const struct desc desc_audio_2_ac_clock_selector[] = {
	{ .field = "bClockID",       .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",      .size = 1, .type = DESC_NUMBER },
	{ .field = "baCSourceID",    .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bmControls",     .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_selector_bmcontrols },
	{ .field = "iClockSelector", .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.13 Clock Selector Descriptor; Table 4-44. */
static const struct desc desc_audio_3_ac_clock_selector[] = {
	{ .field = "bClockID",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bNrInPins",          .size = 1, .type = DESC_NUMBER },
	{ .field = "baCSourceID",        .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bNrInPins" } },
	{ .field = "bmControls",         .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_selector_bmcontrols },
	{ .field = "wCSelectorDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Clock Selector descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_clock_selector[3] = {
	NULL, /* UAC1 not supported */
	desc_audio_2_ac_clock_selector,
	desc_audio_3_ac_clock_selector,
};

/** UAC2 Clock Multiplier bmControls; Human readable bit meanings. */
static const char * const uac2_clock_multiplier_bmcontrols[] = {
	[0] = "Clock Numerator",
	[1] = "Clock Denominator",
	[2] = NULL
};

/** UAC2: 4.7.2.3 Clock Multiplier Descriptor; Table 4-8. */
static const struct desc desc_audio_2_ac_clock_multiplier[] = {
	{ .field = "bClockID",         .size = 1, .type = DESC_CONSTANT },
	{ .field = "bCSourceID",       .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",       .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_multiplier_bmcontrols },
	{ .field = "iClockMultiplier", .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.14 Clock Multiplier Descriptor; Table 4-45. */
static const struct desc desc_audio_3_ac_clock_multiplier[] = {
	{ .field = "bClockID",             .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceID",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",           .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_clock_multiplier_bmcontrols },
	{ .field = "wCMultiplierDescrStr", .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Clock Multiplier descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_clock_multiplier[3] = {
	NULL, /* UAC1 not supported */
	desc_audio_2_ac_clock_multiplier,
	desc_audio_3_ac_clock_multiplier,
};

/** UAC2: 4.7.2.9 Sampling Rate Converter Descriptor; Table 4-14. */
static const struct desc desc_audio_2_ac_sample_rate_converter[] = {
	{ .field = "bUnitID",       .size = 1, .type = DESC_CONSTANT },
	{ .field = "bSourceID",     .size = 1, .type = DESC_CONSTANT },
	{ .field = "bCSourceInID",  .size = 1, .type = DESC_CONSTANT },
	{ .field = "bCSourceOutID", .size = 1, .type = DESC_CONSTANT },
	{ .field = "iSRC",          .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.5.2.8 Sampling Rate Converter Descriptor; Table 4-32. */
static const struct desc desc_audio_3_ac_sample_rate_converter[] = {
	{ .field = "bUnitID",       .size = 1, .type = DESC_NUMBER },
	{ .field = "bSourceID",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceInID",  .size = 1, .type = DESC_NUMBER },
	{ .field = "bCSourceOutID", .size = 1, .type = DESC_NUMBER },
	{ .field = "wSRCDescrStr",  .size = 2, .type = DESC_CS_STR_DESC_ID },
	{ .field = NULL }
};

/** Sample Rate Converter descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_ac_sample_rate_converter[3] = {
	NULL, /* UAC1 not supported */
	desc_audio_2_ac_sample_rate_converter,
	desc_audio_3_ac_sample_rate_converter,
};

/** UAC2 AudioStreaming Interface bmControls; Human readable bit meanings. */
static const char * const uac2_as_interface_bmcontrols[] = {
	[0] = "Active Alternate Setting",
	[1] = "Valid Alternate Setting",
	[2] = NULL
};

/** UAC3 AudioStreaming Interface bmControls; Human readable bit meanings. */
static const char * const uac3_as_interface_bmcontrols[] = {
	[0] = "Active Alternate Setting",
	[1] = "Valid Alternate Setting",
	[2] = "Audio Data Format Control",
	[3] = NULL
};

/* wFormatTag hex prefix for format type */
#define UAC_FORMAT_TYPE_I   0x0
#define UAC_FORMAT_TYPE_II  0x1
#define UAC_FORMAT_TYPE_III 0x2

/** Format type I codes; Human-readable values. */
static const char * const audio_data_format_type_i[] = {
	[0] = "TYPE_I_UNDEFINED",
	[1] = "PCM",
	[2] = "PCM8",
	[3] = "IEEE_FLOAT",
	[4] = "ALAW",
	[5] = "MULAW"
};

/** Format type II codes; Human-readable values. */
static const char * const audio_data_format_type_ii[] = {
	[0] = "TYPE_II_UNDEFINED",
	[1] = "MPEG",
	[2] = "AC-3"
};

/** Format type III codes; Human-readable values. */
static const char * const audio_data_format_type_iii[] = {
	[0] = "TYPE_III_UNDEFINED",
	[1] = "IEC1937_AC-3",
	[2] = "IEC1937_MPEG-1_Layer1",
	[3] = "IEC1937_MPEG-Layer2/3/NOEXT",
	[4] = "IEC1937_MPEG-2_EXT",
	[5] = "IEC1937_MPEG-2_Layer1_LS",
	[6] = "IEC1937_MPEG-2_Layer2/3_LS"
};

/** Special rendering function for UAC1 AS interface wFormatTag */
static void desc_snowflake_dump_uac1_as_interface_wformattag(
		unsigned long long value,
		unsigned int indent)
{
	const char *format_string = "undefined";

	/* Format codes are 0xTNNN, where T=Type prefix, NNN = format code. */

	if (value < ((UAC_FORMAT_TYPE_I << 12) +
	              ARRAY_LEN(audio_data_format_type_i))) {
		format_string = audio_data_format_type_i[value];

	} else if ((value >=  (UAC_FORMAT_TYPE_II << 12)) &&
	           (value <= ((UAC_FORMAT_TYPE_II << 12) +
	                      ARRAY_LEN(audio_data_format_type_ii)))) {
		format_string = audio_data_format_type_ii[value & 0xfff];

	} else if ((value >=  (UAC_FORMAT_TYPE_III << 12)) &&
	           (value <= ((UAC_FORMAT_TYPE_III << 12) +
	                      ARRAY_LEN(audio_data_format_type_iii)))) {
		format_string = audio_data_format_type_iii[value & 0xfff];
	}

	printf(" %s\n", format_string);
}

/** Special rendering function for UAC2 AS interface bmFormats */
static void desc_snowflake_dump_uac2_as_interface_bmformats(
		unsigned long long value,
		unsigned int indent)
{
	unsigned int i;

	printf("\n");
	for (i = 0; i < 5; i++) {
		if ((value >> i) & 0x1) {
			printf("%*s%s\n", indent * 2, "",
					audio_data_format_type_i[i + 1]);
		}
	}

}

/** UAC1: 4.5.2 Class-Specific AS Interface Descriptor; Table 4-19. */
static const struct desc desc_audio_1_as_interface[] = {
	{ .field = "bTerminalLink", .size = 1, .type = DESC_CONSTANT },
	{ .field = "bDelay",        .size = 1, .type = DESC_NUMBER_POSTFIX,
			.number_postfix = " frames" },
	{ .field = "wFormatTag",    .size = 2, .type = DESC_SNOWFLAKE,
			.snowflake = desc_snowflake_dump_uac1_as_interface_wformattag },
	{ .field = NULL }
};

/** UAC2: 4.9.2 Class-Specific AS Interface Descriptor; Table 4-27. */
static const struct desc desc_audio_2_as_interface[] = {
	{ .field = "bTerminalLink",   .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",      .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_as_interface_bmcontrols },
	{ .field = "bFormatType",     .size = 1, .type = DESC_CONSTANT },
	{ .field = "bmFormats",       .size = 4, .type = DESC_SNOWFLAKE,
			.snowflake = desc_snowflake_dump_uac2_as_interface_bmformats },
	{ .field = "bNrChannels",     .size = 1, .type = DESC_NUMBER },
	{ .field = "bmChannelConfig", .size = 4, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_channel_names, .count = 26 } },
	{ .field = "iChannelNames",   .size = 1, .type = DESC_STR_DESC_INDEX },
	{ .field = NULL }
};

/** UAC3: 4.7.2 Class-Specific AS Interface Descriptor; Table 4-49. */
static const struct desc desc_audio_3_as_interface[] = {
	{ .field = "bTerminalLink",   .size = 1, .type = DESC_NUMBER },
	{ .field = "bmControls",      .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac3_as_interface_bmcontrols },
	{ .field = "wClusterDescrID", .size = 2, .type = DESC_NUMBER },
	{ .field = "bmFormats",       .size = 8, .type = DESC_BITMAP },
	{ .field = "bSubslotSize",    .size = 1, .type = DESC_NUMBER },
	{ .field = "bBitResolution",  .size = 1, .type = DESC_NUMBER },
	{ .field = "bmAuxProtocols",  .size = 2, .type = DESC_BITMAP },
	{ .field = "bControlSize",    .size = 1, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** AudioStreaming Interface descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_as_interface[3] = {
	desc_audio_1_as_interface,
	desc_audio_2_as_interface,
	desc_audio_3_as_interface,
};

/** UAC1: Data Endpoint bmAttributes; Human readable bit meanings. */
static const char * const uac1_as_endpoint_bmattributes[] = {
	[0] = "Sampling Frequency",
	[1] = "Pitch",
	[2] = "Audio Data Format Control",
	[7] = "MaxPacketsOnly"
};

/** UAC2: Data Endpoint bmAttributes; Human readable bit meanings. */
static const char * const uac2_as_endpoint_bmattributes[] = {
	[7] = "MaxPacketsOnly",
};

/** UAC2 AudioStreaming Interface bmControls; Human readable bit meanings. */
static const char * const uac2_as_isochronous_audio_data_endpoint_bmcontrols[] = {
	[0] = "Pitch",
	[1] = "Data Overrun",
	[2] = "Data Underrun",
	[3] = NULL
};

/** Audio Data Endpoint bLockDelayUnits; Human readable value meanings. */
static const char * const uac_as_isochronous_audio_data_endpoint_blockdelayunits[] = {
	[0] = "Undefined",
	[1] = "Milliseconds",
	[2] = "Decoded PCM samples",
	[3] = NULL
};

/** UAC1: 4.6.1.2 Class-Specific AS Isochronous Audio Data Endpoint Descriptor; Table 4-21. */
static const struct desc desc_audio_1_as_isochronous_audio_data_endpoint[] = {
	{ .field = "bmAttributes",    .size = 1, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac1_as_endpoint_bmattributes, .count = 8 } },
	{ .field = "bLockDelayUnits", .size = 1, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac_as_isochronous_audio_data_endpoint_blockdelayunits },
	{ .field = "wLockDelay",      .size = 2, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** UAC2: 4.10.1.2 Class-Specific AS Isochronous Audio Data Endpoint Descriptor; Table 4-34. */
static const struct desc desc_audio_2_as_isochronous_audio_data_endpoint[] = {
	{ .field = "bmAttributes",    .size = 1, .type = DESC_BITMAP_STRINGS,
			.bitmap_strings = { .strings = uac2_as_endpoint_bmattributes, .count = 8 } },
	{ .field = "bmControls",      .size = 1, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_as_isochronous_audio_data_endpoint_bmcontrols },
	{ .field = "bLockDelayUnits", .size = 1, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac_as_isochronous_audio_data_endpoint_blockdelayunits },
	{ .field = "wLockDelay",      .size = 2, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** UAC3: 4.8.1.2 Class-Specific AS Isochronous Audio Data Endpoint Descriptor; Table 4-52. */
static const struct desc desc_audio_3_as_isochronous_audio_data_endpoint[] = {
	{ .field = "bmControls",      .size = 4, .type = DESC_BMCONTROL_2,
			.bmcontrol = uac2_as_isochronous_audio_data_endpoint_bmcontrols },
	{ .field = "bLockDelayUnits", .size = 1, .type = DESC_NUMBER_STRINGS,
			.number_strings = uac_as_isochronous_audio_data_endpoint_blockdelayunits },
	{ .field = "wLockDelay",      .size = 2, .type = DESC_NUMBER },
	{ .field = NULL }
};

/** Isochronous Audio Data Endpoint descriptor definitions for the three Audio Device Class protocols */
const struct desc * const desc_audio_as_isochronous_audio_data_endpoint[3] = {
	desc_audio_1_as_isochronous_audio_data_endpoint,
	desc_audio_2_as_isochronous_audio_data_endpoint,
	desc_audio_3_as_isochronous_audio_data_endpoint,
};

/** USB3: 9.6.2.7 Configuration Summary Descriptor; Table 9-21. */
const struct desc desc_usb3_dc_configuration_summary[] = {
	{ .field = "bLength",             .size = 1, .type = DESC_NUMBER },
	{ .field = "bDescriptorType",     .size = 1, .type = DESC_CONSTANT },
	{ .field = "bDevCapabilityType",  .size = 1, .type = DESC_NUMBER },
	{ .field = "bcdVersion",          .size = 2, .type = DESC_BCD },
	{ .field = "bClass",              .size = 1, .type = DESC_NUMBER },
	{ .field = "bSubClass",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bProtocol",           .size = 1, .type = DESC_NUMBER },
	{ .field = "bConfigurationCount", .size = 1, .type = DESC_NUMBER },
	{ .field = "bConfigurationIndex", .size = 1, .type = DESC_NUMBER,
			.array = { .array = true, .length_field1 = "bConfigurationCount" } },
	{ .field = NULL }
};
