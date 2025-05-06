/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef TEK_POE_H
#define TEK_POE_H

#include <stddef.h>
#include <stdint.h>
#include <libubox/utils.h>

#define ULOG_DBG(fmt, ...) ulog(LOG_DEBUG, fmt, ## __VA_ARGS__)

#define GET_STR(a, b)	((a) < ARRAY_SIZE(b) ? (b)[a] : NULL)
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#define MAX_PORT	48

struct port_state {
	const char *status;
	const char *poe_mode;
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

struct config {
	float budget;
	float budget_guard;

	unsigned int port_count;
	uint8_t pse_id_set_budget_mask;
	struct port_config ports[MAX_PORT];
};

static inline uint16_t read16_be(uint8_t *raw)
{
	return (uint16_t)raw[0] << 8 | raw[1];
}

static inline void write16_be(uint8_t *raw, uint16_t value)
{
	raw[0] = value >> 8;
	raw[1] =  value & 0xff;
}

#endif /* TEK_POE_H */
