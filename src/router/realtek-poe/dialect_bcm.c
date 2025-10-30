/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "tek-poe.h"

static const struct dialect_map_entry bcm_dialect_mapping[] = {
	[MCU_SET_POWER_MGMT_MODE] 	= {0x17, 1},
	[MCU_SET_POWER_BUDGET]		= {0x18, 1},
	[MCU_ENABLE_PORT_MAPPING]	= {0x02, 1},
	[PORT_ENABLE]			= {0x00, 1},
	[PORT_ENABLE_CLASSIFICATION]	= {0x11, CMD_IS_4PORT},
	[PORT_SET_DETECTION_TYPE]	= {0x10, CMD_HAS_ALL_PORT},
	[PORT_SET_PRIORITY]		= {0x1a, CMD_IS_4PORT},
	[PORT_SET_POE_MODE]		= {0x1c, CMD_IS_4PORT},
	[PORT_SET_DISCONNECT_TYPE]	= {0x13, CMD_HAS_ALL_PORT},
	[PORT_SET_POWER_LIMIT_TYPE]	= {0x15, CMD_IS_4PORT},
	[PORT_SET_POWER_LIMIT]		= {0x16, 1},

	[MCU_GET_SYSTEM_INFO]		= {0x20, 1},
	[MCU_GET_POWER_STATS]		= {0x23, 1},
	[MCU_GET_EXT_CONFIG]		= {0x2b, 1},
	[PORT_GET_STATUS]		= {0x21, 1},
	[PORT_GET_CONFIG]		= {0x25, 1},
	[PORT_GET_EXT_CONFIG]		= {0x26, 1},
	[PORT_GET_SHORT_STATUS]		= {0x28, CMD_IS_4PORT},
	[PORT_GET_POWER_STATS]		= {0x30, 1},
};

static const struct dialect_map bcm_dialect_map = {
	.entries = bcm_dialect_mapping,
	.len = ARRAY_SIZE(bcm_dialect_mapping),
};

static const struct dialect_ops bcm_ops = {
	/* No overrides for broadcom dialect. */
};

const struct dialect_desc broadcom_dialect = {
	.map = &bcm_dialect_map,
	.ops = &bcm_ops,
};
