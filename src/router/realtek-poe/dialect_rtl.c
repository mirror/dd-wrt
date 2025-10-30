/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "tek-poe.h"

#include <errno.h>
#include <string.h>
#include <libubox/ulog.h>

/*
 * Error code that the realtek dialect uses to respond to some broadcom command
 * IDs. Usually indicates that the broadcom dialect is erroneously selected.
 */
#define RTL_REPLY_NOT_IMPLEMENTED		0x61

/*
 * Some commands are intentionally not mapped
 *
 *  - [MCU_ENABLE_PORT_MAPPING] = {0x02, 0},
 *    The port mapping is a persistent command in the realtek dialect, and has
 *    an extra field to change the number of ports. A reset will not recover
 *    from a bad mapping, so it's best not to send this command.
 *
 *  - [PORT_ENABLE_CLASSIFICATION] = {0x11, CMD_IS_4PORT},
 *    This command appears to be the same as PORT_SET_POWER_LIMIT, but using
 *    units of 0.1 W instead of 0.2 W. It appears to be redundant.
 */
static const struct dialect_map_entry rtl_dialect_map[] = {
	[MCU_SET_POWER_BUDGET]		= {0x04, 1},
	[MCU_SET_POWER_MGMT_MODE] 	= {0x10, 1},
	[PORT_ENABLE]			= {0x01, 1},
	[PORT_SET_AUTO_POWERUP]		= {0x08, CMD_IS_4PORT},
	[PORT_SET_DETECTION_TYPE]	= {0x09, CMD_IS_4PORT},
	[PORT_SET_POE_MODE]		= {0x0c, CMD_IS_4PORT},
	[PORT_SET_DISCONNECT_TYPE]	= {0x0f, CMD_IS_4PORT},
	[PORT_SET_POWER_LIMIT_TYPE]	= {0x12, CMD_IS_4PORT},
	[PORT_SET_POWER_LIMIT]		= {0x13, 1},
	[PORT_SET_PRIORITY]		= {0x15, CMD_IS_4PORT},

	[MCU_GET_SYSTEM_INFO]		= {0x40, 1},
	[MCU_GET_POWER_STATS]		= {0x41, 1},
	[MCU_GET_EXT_CONFIG]		= {0x4a, 1},
	[PORT_GET_STATUS]		= {0x42, 1},
	[PORT_GET_POWER_STATS]		= {0x44, 1},
	[PORT_GET_CONFIG]		= {0x48, 1},
	[PORT_GET_EXT_CONFIG]		= {0x49, 1},

};

static int rtl_cmd_pse_up(struct mcu *mcu, uint8_t enable)
{
	uint8_t cmd[] = { 0x00, 0x00, enable };

	return mcu_queue_buf(mcu, cmd, sizeof(cmd));
}

static int rtl_cmd_reset_enable(struct mcu *mcu, bool enable)
{
	uint8_t cmd[] = { 0x02, 0x00, enable };

	return mcu_queue_buf(mcu, cmd, sizeof(cmd));
}

static int rtl_cmd_4_port_group_status(struct mcu *mcu, uint8_t start_port)
{
	uint8_t cmd[] = { 0x43, 0x00, start_port / 4};

	return mcu_queue_buf(mcu, cmd, sizeof(cmd));
}

static int rtl_reply_port_group_status(struct mcu_state *state, uint8_t *reply)
{

	int i, port, pstate;

	port = reply[2] * 4;
	for (i = 3; i < 11; i += 2, port++) {
		pstate = reply[i];

		if (port == 0xff) {
			continue;
		} else if (port >= MAX_PORT) {
			ULOG_DBG("Invalid port status packet (port=%d)\n", port);
			return -1;
		}

		state->ports[port].status = port_short_status_to_str(pstate);
	}

	return 0;
}

static int rtl_cmd_why_u_reset(struct mcu *mcu)
{
	uint8_t cmd[] = { 0x47, 0x00, 0x01 };

	return mcu_queue_buf(mcu, cmd, sizeof(cmd));
}

static int poe_default_reply_handler(uint8_t *reply)
{
	int cmd = reply[0];
	int ret = reply[2];

	if (ret)
		ULOG_WARN("Command 0x%x replied with error 0x%x\n", cmd, ret);
	return 0;
}

static int rtl_handle_reply(struct mcu_state *ctx, uint8_t *reply, size_t len)
{
	if (len != 12)
		return -EINVAL;

	switch (reply[0]) {
	case 0x43:
		rtl_reply_port_group_status(ctx, reply);
		break;
	default:
		poe_default_reply_handler(reply);
		break;
	}

	return 0;
}

static int rtl_initial_setup(struct mcu *mcu, const struct config *config)
{
	rtl_cmd_pse_up(mcu, true);
	rtl_cmd_why_u_reset(mcu);
	rtl_cmd_reset_enable(mcu, false);

	return 0;
}

static int chicken_reset(struct mcu *mcu)
{
	/* Might have to also send rtl_cmd_reset_enable(false); */
	rtl_cmd_reset_enable(mcu, true);
	return 0;
}

static int rtl_poll(struct mcu *mcu, const struct config *config)
{
	size_t i;

	for (i = 0; i < config->port_count; i += 4)
		rtl_cmd_4_port_group_status(mcu, i);

	return 0;
}

static const struct dialect_ops realtek_ops = {
	.init_async = rtl_initial_setup,
	.poll_async = rtl_poll,
	.handle_reply = rtl_handle_reply,
	.reset = chicken_reset,
};

static const struct dialect_map realtek_map = {
	.entries = rtl_dialect_map,
	.len = ARRAY_SIZE(rtl_dialect_map),
};

const struct dialect_desc realtek_dialect = {
	.map = &realtek_map,
	.ops = &realtek_ops,
};
