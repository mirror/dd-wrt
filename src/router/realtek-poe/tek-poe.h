/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef TEK_POE_H
#define TEK_POE_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <libubox/utils.h>

#define ULOG_DBG(fmt, ...) ulog(LOG_DEBUG, fmt, ## __VA_ARGS__)

#define GET_STR(a, b)	((a) < ARRAY_SIZE(b) ? (b)[a] : NULL)
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#define MAX_PORT	48

/*
 * Order of commands doesn't matter. These are just an internal representation
 * that gets mapped to a wire command based on the dialect. Value of "0" is
 * reserve for "dialect does not implement command".
 *   MCU_ are global commands
 *   PORT_ are "port" commands
 */
enum poe_cmd {
	CMD_NONE = 0,
	MCU_SET_POWER_MGMT_MODE,
	MCU_SET_POWER_BUDGET,
	MCU_ENABLE_PORT_MAPPING,
	PORT_ENABLE,
	PORT_ENABLE_CLASSIFICATION,
	PORT_SET_DETECTION_TYPE,
	PORT_SET_PRIORITY,
	PORT_SET_POE_MODE,
	PORT_SET_DISCONNECT_TYPE,
	PORT_SET_POWER_LIMIT_TYPE,
	PORT_SET_POWER_LIMIT,
	PORT_SET_AUTO_POWERUP,

	MCU_GET_SYSTEM_INFO,
	MCU_GET_POWER_STATS,
	MCU_GET_EXT_CONFIG,
	PORT_GET_CONFIG,
	PORT_GET_EXT_CONFIG,
	PORT_GET_STATUS,
	PORT_GET_SHORT_STATUS,
	PORT_GET_POWER_STATS,
	CMD_MAX
};

enum poe_cmd_flags {
	CMD_IS_4PORT = 2,
	CMD_IS_4PORT_RTL = 4,
	CMD_HAS_ALL_PORT = 8,
};

struct mcu;

struct port_state {
	const char *status;
	const char *poe_mode;
	float mvolt;
	float mampere;
	float tempc;
	float power_budget;
	float watt;

	unsigned int has_config_info : 1;
	unsigned int has_detailed_state : 1;

	uint8_t power_limit_type;
	uint8_t priority;
	uint8_t primary_pse_output;
	uint8_t mapping;

	uint8_t enabled;
	uint8_t auto_powerup;
	uint8_t detection_type;
	uint8_t classification_enable;
	uint8_t disconnect_type;
	uint8_t pair;

	uint8_t class_info;
	uint8_t pd_type;
	uint8_t mpss_mask;
};

struct mcu_state {
	const char *sys_mode;
	const char *sys_mcu;
	const char *sys_status;
	float power_consumption;
	float reported_power_budget;
	float uvlo_threshold;
	float ovlo_threshold;
	unsigned int num_detected_ports;

	unsigned int has_ext_cfg_info : 1;

	uint16_t device_id;
	uint8_t sys_version;
	uint8_t sys_ext_version;
	uint8_t port_map_en;

	uint8_t pre_alloc;
	uint8_t powerup_mode;
	uint8_t disconnect_type;
	uint8_t ddflag;
	uint8_t num_pse;

	struct port_state ports[MAX_PORT];
};

struct port_config {
	char name[16];
	unsigned int valid : 1;
	unsigned int enable : 1;
	uint8_t priority;
	uint8_t power_up_mode;
	uint8_t power_budget;
};

struct dialect_desc;

struct config {
	const struct dialect_desc *forced_dialect;

	float budget;
	float budget_guard;

	unsigned int forced_baudrate;
	unsigned int port_count;
	uint8_t pse_id_set_budget_mask;
	struct port_config ports[MAX_PORT];
};

struct dialect_ops {
	int (*init_async)(struct mcu *mcu, const struct config *cfg);
	int (*poll_async)(struct mcu *mcu, const struct config *cfg);
	int (*reset)(struct mcu *mcu);
	int (*handle_reply)(struct mcu_state *mcu, uint8_t *reply, size_t len);
};

struct dialect_map_entry {
	uint8_t wire_id;
	uint8_t flags;
};

struct dialect_map {
	const struct dialect_map_entry *entries;
	size_t len;
};

struct dialect_desc {
	const struct dialect_ops *ops;
	const struct dialect_map *map;
};

struct poe_dialect {
	const struct dialect_desc *desc;
	uint8_t reverve_map[0x100];
};

int mcu_queue_buf(struct mcu *mcu, uint8_t *cmd_buf, size_t len);

static inline uint16_t read16_be(uint8_t *raw)
{
	return (uint16_t)raw[0] << 8 | raw[1];
}

static inline void write16_be(uint8_t *raw, uint16_t value)
{
	raw[0] = value >> 8;
	raw[1] =  value & 0xff;
}

static inline int dialect_reverse_map(struct poe_dialect *dialect)
{
	const struct dialect_map *map = dialect->desc->map;
	unsigned int wire_id;
	size_t i;

	for (i = 0; i < map->len; i++) {
		if (!map->entries[i].flags)
			continue;

		wire_id = map->entries[i].wire_id;
		if (wire_id > 0x100)
			return -EINVAL;

		dialect->reverve_map[wire_id] = i;
	}
	return 0;
}

static inline int dialect_lookup_cmd(const struct poe_dialect *dialect,
				     enum poe_cmd cmd)
{
	const struct dialect_map *map = dialect->desc->map;

	if (cmd > map->len || !map->entries[cmd].flags)
		return -EINVAL;

	return map->entries[cmd].wire_id;
}


static inline enum poe_cmd dialect_rev_lookup(const struct poe_dialect *dialect,
					      uint8_t wire_id)
{
	return dialect->reverve_map[wire_id];
}

const char *port_short_status_to_str(uint8_t short_status);
extern const struct dialect_desc broadcom_dialect;
extern const struct dialect_desc realtek_dialect;

#endif /* TEK_POE_H */
