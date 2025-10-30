/* SPDX-License-Identifier: GPL-2.0 */

#include "tek-poe.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <shutils.h>
#include <ddnvram.h>

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include <libubus.h>

typedef int (*poe_reply_handler)(struct mcu_state *mcu, uint8_t *reply);

/* Careful with this; Only works for set_detection/disconnect_type commands. */
#define PORT_ID_ALL	0x7f
#define MAX_RETRIES	5

#define CMD_SIZE	12
#define OFFSET_CHECKSUM	(CMD_SIZE - 1)

struct mcu {
	struct uloop_timeout error_timeout;
	struct list_head pending_cmds;
	struct ustream_fd stream;
	struct mcu_state state;
	uint8_t cmd_seq;
};

struct cmd {
	struct list_head list;
	uint8_t cmd[CMD_SIZE];
	unsigned int num_retries;
};

struct poe_ctx {
	struct mcu mcu;
	struct config config;
	struct ubus_auto_conn conn;
	struct blob_buf blob_buf;
	struct uloop_timeout state_timeout;
	unsigned int hardcore_hacking_mode_en : 1;
};

static struct poe_ctx *ubus_to_poe_ctx(struct ubus_context *u)
{
	struct ubus_auto_conn *c = container_of(u, struct ubus_auto_conn, ctx);
	return container_of(c, struct poe_ctx, conn);
}

extern int ifexists(const char *ifname);

static void load_port_config(struct config *cfg, int id)
{
	const char *id_str, *enable, *priority, *poe_plus;
	char name[32];
	sprintf(name, "lan%02d",id);
	if (!ifexists(name))
	    return;
	enable = nvram_default_nget("1", "%s_poe_enable", name);
	priority = nvram_nget("%s_poe_priority", name);
	poe_plus = nvram_default_nget("1", "%s_poe_plus", name); // 802.3at

	cfg->port_count = MAX(cfg->port_count, id);
	id--;

	strncpy(cfg->ports[id].name, name, sizeof(cfg->ports[id].name));
	cfg->ports[id].valid = 1;
	cfg->ports[id].enable = enable ? !strcmp(enable, "1") : 0;
	cfg->ports[id].priority = priority ? strtoul(priority, NULL, 0) : 0;
	if (cfg->ports[id].priority > 3)
		cfg->ports[id].priority = 3;

	if (poe_plus && !strcmp(poe_plus, "1"))
		cfg->ports[id].power_up_mode = 3;
}

static void load_global_config(struct config *cfg)

{
	const char *budget, *guardband;

	budget = nvram_default_get("poe_budget", "170");
	guardband = nvram_safe_get("poe_guard");

	cfg->budget = budget ? strtof(budget, NULL) : 31.0;
	cfg->budget_guard = cfg->budget / 10;
	if (guardband && *guardband)
		cfg->budget_guard = strtof(guardband, NULL);
}

static char *get_board_compatible(void)
{
	char name[128];
	int fd, ret;

	fd = open("/sys/firmware/devicetree/base/compatible", O_RDONLY);
	if (fd < 0)
		return NULL;

	ret = read(fd, name, sizeof(name));
	if (ret < 0)
		return NULL;

	close(fd);

	return strndup(name, ret);
}

static void config_apply_quirks(struct config *config)
{
	if (nvram_match("DD_BOARD","Zyxel GS1900-24HP v1")) {
		/* Send budget command to first 8 PSE IDs */
		config->pse_id_set_budget_mask = 0xff;
	}
}

static void config_load(struct config *cfg, int init)
{
	memset(cfg->ports, 0, sizeof(cfg->ports));


	if (init) {
		load_global_config(cfg);
		config_apply_quirks(cfg);
	}
	int i;
	for (i=1;i<51;i++) {
		load_port_config(cfg, i);
	}

}

static void log_packet(int log_level, const char *prefix, const uint8_t d[12])
{
	ulog(log_level,
	     "%s %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
	     prefix, d[0], d[1], d[2], d[3], d[4], d[5],
		     d[6], d[7], d[8], d[9], d[10], d[11]);
}

static int mcu_cmd_send(struct mcu *mcu, struct cmd *cmd)
{
	if (mcu->error_timeout.pending)
		return -EBUSY;

	log_packet(LOG_DEBUG, "TX ->", cmd->cmd);
	return ustream_write(&mcu->stream.stream, (void *)cmd->cmd, 12, false);
}

static int mcu_cmd_next(struct mcu *mcu)
{
	struct cmd *cmd;

	if (list_empty(&mcu->pending_cmds))
		return -EAGAIN;

	cmd = list_first_entry(&mcu->pending_cmds, struct cmd, list);

	return mcu_cmd_send(mcu, cmd);
}

static int mcu_queue_cmd(struct mcu *mcu, uint8_t *cmd_buf, size_t len)
{
	int i, empty = list_empty(&mcu->pending_cmds);
	struct cmd *cmd = malloc(sizeof(*cmd));
	int ret = 0;

	memset(cmd, 0, sizeof(*cmd));
	memset(cmd->cmd, 0xff, CMD_SIZE);
	memcpy(cmd->cmd, cmd_buf, len);

	mcu->cmd_seq++;
	cmd->cmd[1] = mcu->cmd_seq;
	cmd->cmd[OFFSET_CHECKSUM] = 0;

	for (i = 0; i < OFFSET_CHECKSUM; i++)
		cmd->cmd[OFFSET_CHECKSUM] += cmd->cmd[i];

	list_add_tail(&cmd->list, &mcu->pending_cmds);

	if (empty)
	    ret = mcu_cmd_send(mcu, cmd);

	return ret;
}

static int poet_cmd_4_port(struct mcu *mcu, uint8_t cmd_id, uint8_t port[4],
			   uint8_t data[4])
{
	uint8_t cmd[] = { cmd_id, 0x00, port[0], data[0], port[1], data[1],
					port[2], data[2], port[3], data[3] };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x00 - Set port enable
 *	0: Disable
 *	1: Enable
 */
static int poe_cmd_port_enable(struct mcu *mcu, uint8_t port, uint8_t enable)
{
	uint8_t cmd[] = { 0x00, 0x00, port, enable };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_cmd_port_mapping_enable(struct mcu *mcu, bool enable)
{
	uint8_t cmd[] = { 0x02, 0x00, enable };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x10 - Set port detection type
 *	1: Legacy Capacitive Detection only
 *	2: IEEE 802.3af 4-Point Detection only (Default)
 *	3: IEEE 802.3af 4-Point followed by Legacy
 *	4: IEEE 802.3af 2-Point detection (Not Supported)
 *	5: IEEE 802.3af 2-Point followed by Legacy
 */
static int poe_cmd_port_detection_type(struct mcu *mcu, uint8_t port,
				       uint8_t type)
{
	uint8_t cmd[] = { 0x10, 0x00, port, type };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x11 - Set port classification
 *	0: Disable
 *	1: Enable
 */
static int poe_cmd_port_classification(struct mcu *mcu, uint8_t port[4],
				       uint8_t enable[4])
{
	return poet_cmd_4_port(mcu, 0x11, port, enable);
}

/* 0x13 - Set port disconnect type
 *	0: none
 *	1: AC-disconnect
 *	2: DC-disconnect
 *	3: DC with delay
 */
static int poe_cmd_port_disconnect_type(struct mcu *mcu, uint8_t port,
					uint8_t type)
{
	uint8_t cmd[] = { 0x13, 0x00, port, type };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x15 - Set port power limit type
 *	0: None. Power limit is 16.2W if the connected device is “low power”,
 *	   or the set high power limit if the device is “high power”.
 *	1: Class based. The power limit for class 4 devices is determined by the high power limit.
 *	2: User defined
 */
static int poe_cmd_port_power_limit_type(struct mcu *mcu, uint8_t port[4],
					 uint8_t limit[4])
{
	return poet_cmd_4_port(mcu, 0x15, port, limit);
}

/* 0x16 - Set port power budget
 *	values in 0.2W increments
 */
static int poe_cmd_port_power_budget(struct mcu *mcu, uint8_t port,
				     uint8_t budget)
{
	uint8_t cmd[] = { 0x16, 0x00, port, budget };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x17 - Set power management mode
 *	0: None (No Power Management mode) (Default in Semi-Auto mode)
 *	1: Static Power Management with Port Priority(Default in Automode)
 *	2: Dynamic Power Management with Port Priority
 *	3: Static Power Management without Port Priority
 *	4: Dynamic Power Management without Port Priority
 */
static int poe_cmd_power_mgmt_mode(struct mcu *mcu, uint8_t mode)
{
	uint8_t cmd[] = { 0x17, 0x00, mode };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x18 - Set global power budget */
static int poe_cmd_global_power_budget(struct mcu *mcu, uint8_t pse,
				       float budget, float guard)
{
	uint8_t cmd[] = { 0x18, 0x00, pse, 0x00, 0x00, 0x00, 0x00 };

	write16_be(cmd + 3, budget * 10);
	write16_be(cmd + 5, guard * 10);

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

/* 0x1a - Set port priority
 *	0: Low
 *	1: Normal
 *	2: High
 *	3: Critical
 */
static int poe_set_port_priority(struct mcu *mcu, uint8_t port[4],
				 uint8_t priority[4])
{
	return poet_cmd_4_port(mcu, 0x1a, port, priority);
}

/* 0x1c - Set port power-up mode
 *	0: PoE
 *	1: legacy
 *	2: pre-PoE+
 *	3: PoE+
 */
static int poe_set_port_power_up_mode(struct mcu *mcu, uint8_t port[4],
				      uint8_t mode[4])
{
	return poet_cmd_4_port(mcu, 0x1c, port, mode);
}

/* 0x20 - Get system info */
static int poe_cmd_status(struct mcu *mcu)
{
	uint8_t cmd[] = { 0x20 };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_status(struct mcu_state *state, uint8_t *reply)
{
	const char *mode[] = {
		"Semi-auto I2C",
		"Semi-auto UART",
		"Auto I2C",
		"Auto UART"
	};
	const char *mcu_names[] = {
		"ST Micro ST32F100 Microcontroller",
		"Nuvoton M05xx LAN Microcontroller",
		"ST Micro STF030C8 Microcontroller",
		"Nuvoton M058SAN Microcontroller",
		"Nuvoton NUC122 Microcontroller"
	};
	const char *status[] = {
		"Global Disable pin is de-asserted:No system reset from the previous query cmd:Configuration saved",
		"Global Disable pin is de-asserted:No system reset from the previous query cmd:Configuration Dirty",
		"Global Disable pin is de-asserted:System reseted:Configuration saved",
		"Global Disable pin is de-asserted:System reseted:Configuration Dirty",
		"Global Disable Pin is asserted:No system reset from the previous query cmd:Configuration saved",
		"Global Disable Pin is asserted:No system reset from the previous query cmd:Configuration Dirty",
		"Global Disable Pin is asserted:System reseted:Configuration saved",
		"Global Disable Pin is asserted:System reseted:Configuration Dirty"
	};

	state->sys_mode = GET_STR(reply[2], mode);
	state->num_detected_ports = reply[3];
	state->port_map_en = reply[4];
	state->device_id =  read16_be(reply + 5);
	state->sys_version = reply[7];
	state->sys_mcu = GET_STR(reply[8], mcu_names);
	state->sys_status = GET_STR(reply[9], status);
	state->sys_ext_version = reply[10];

	return 0;
}

/* 0x21 - Get port status */
static int poe_cmd_port_status(struct mcu *mcu, uint8_t port)
{
	uint8_t cmd[] = { 0x21, 0x00, port };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_port_status(struct mcu_state *state, uint8_t *reply)
{
	unsigned int idx = reply[2];
	struct port_state *port = &state->ports[idx];

	if (idx > state->num_detected_ports) {
		ULOG_WARN("Invalid port in status reply (port=%d)\n", idx);
		return -EPROTO;
	}

	port->class_info = reply[5];
	port->pd_type = reply[6];
	port->mpss_mask = reply[7];
	port->has_detailed_state = 1;

	return 0;
}

/* 0x23 - Get power statistics */
static int poe_cmd_power_stats(struct mcu *mcu)
{
	uint8_t cmd[] = { 0x23 };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_power_stats(struct mcu_state *state, uint8_t *reply)
{
	state->power_consumption = read16_be(reply + 2) * 0.1;
	state->reported_power_budget = read16_be(reply + 4) * 0.1;

	return 0;
}

/* 0x25 - Get port config */
static int poe_cmd_port_config(struct mcu *mcu, uint8_t port)
{
	uint8_t cmd[] = { 0x25, 0x00, port };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_port_config(struct mcu_state *state, uint8_t *reply)
{
	unsigned int idx = reply[2];
	struct port_state *port = &state->ports[idx];

	if (idx > state->num_detected_ports) {
		ULOG_WARN("Invalid port in config reply (port=%d)\n", idx);
		return -EPROTO;
	}

	port->enabled = reply[3];
	port->auto_powerup = reply[4];
	port->detection_type = reply[5];
	port->classification_enable = reply[6];
	port->disconnect_type = reply[7];
	port->pair = reply[8];
	port->has_config_info = 1;

	return 0;
}

/* 0x26 - Get extended port config */
static int poe_cmd_port_ext_config(struct mcu *mcu, uint8_t port)
{
	uint8_t cmd[] = { 0x26, 0x00, port };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_port_ext_config(struct mcu_state *state, uint8_t *reply)
{
	unsigned int idx = reply[2];
	struct port_state *port = &state->ports[idx];

	const char *mode[] = {
		"PoE",
		"Legacy",
		"pre-PoE+",
		"PoE+"
	};

	if (idx > state->num_detected_ports) {
		ULOG_WARN("Invalid port in ext config reply (port=%d)\n", idx);
		return -EPROTO;
	}

	port->poe_mode = GET_STR(reply[3], mode);
	port->power_limit_type = reply[4];
	port->power_budget = reply[5] * 0.2;
	port->priority = reply[6];
	port->primary_pse_output = reply[7];
	/* In the broadcom dialect, pse output and mapping are synonymous. */
	port->mapping = port->primary_pse_output;

	return 0;
}

/* 0x28 - Get all all port status */
static int poe_cmd_4_port_status(struct mcu *mcu, uint8_t p1, uint8_t p2,
				 uint8_t p3, uint8_t p4)
{
	uint8_t cmd[] = { 0x28, 0x00, p1, 1, p2, 1, p3, 1, p4, 1 };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_4_port_status(struct mcu_state *state, uint8_t *reply)
{
	int i, port, pstate;

	const char *status[] = {
		[0] = "Disabled",
		[1] = "Searching",
		[2] = "Delivering power",
		[4] = "Fault",
		[5] = "Other fault",
		[6] = "Requesting power",
	};

	for (i = 2; i < OFFSET_CHECKSUM; i+=2) {
		port = reply[i];
		pstate = reply[i + 1];

		if (port == 0xff) {
			continue;
		} else if (port >= MAX_PORT) {
			ULOG_WARN("Invalid port status packet (port=%d)\n", port);
			return -1;
		}

		state->ports[port].status = GET_STR(pstate & 0xf, status);
	}

	return 0;
}

/* 0x2b - Get extended device config */
static int poe_cmd_get_extended_config(struct mcu *mcu)
{
	uint8_t cmd[] = { 0x2b };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_extended_config(struct mcu_state *state, uint8_t *reply)
{
	state->uvlo_threshold = reply[2] * 0.06445 + 33.0;
	state->pre_alloc = reply[3];
	state->powerup_mode = reply[4];
	state->disconnect_type = reply[5];
	state->ddflag = reply[6];
	state->ovlo_threshold = reply[7] * 0.06445 + 57.0;
	state->num_pse = reply[8];
	state->has_ext_cfg_info = 1;

	return 0;
}

/* 0x30 - Get port power statistics */
static int poe_cmd_port_power_stats(struct mcu *mcu, uint8_t port)
{
	uint8_t cmd[] = { 0x30, 0x00, port };

	return mcu_queue_cmd(mcu, cmd, sizeof(cmd));
}

static int poe_reply_port_power_stats(struct mcu_state *state, uint8_t *reply)
{
	int port_idx = reply[2];

	state->ports[port_idx].watt = read16_be(reply + 9) * 0.1;
	return 0;
}

static poe_reply_handler reply_handler[] = {
	[0x20] = poe_reply_status,
	[0x21] = poe_reply_port_status,
	[0x23] = poe_reply_power_stats,
	[0x26] = poe_reply_port_ext_config,
	[0x25] = poe_reply_port_config,
	[0x28] = poe_reply_4_port_status,
	[0x2b] = poe_reply_extended_config,
	[0x30] = poe_reply_port_power_stats,
};

static void mcu_clear_timeout(struct uloop_timeout *t)
{
	struct mcu *mcu = container_of(t, struct mcu, error_timeout);

	mcu_cmd_next(mcu);
}

static void handle_f0_reply(struct mcu *mcu, struct cmd *cmd, uint8_t *reply)
{
	const char *reason;

	const char *reasons[] = {
		[0xd] = "request-incomplete",
		[0xe] = "request-bad-checksum",
		[0xf] = "not-ready",
	};

	reason = GET_STR((uint8_t)(reply[0] - 0xf0), reasons);
	reason = reason ? reason : "unknown";

	/* Log the first reply, then only log complete failures. */
	if (cmd->num_retries == 0) {
		ULOG_NOTE("MCU rejected command: %s\n", reason);
		log_packet(LOG_NOTICE, "\tCMD:   ", cmd->cmd);
		log_packet(LOG_NOTICE, "\treply: ", reply);
	}

	if (!mcu->error_timeout.pending) {
		if (++cmd->num_retries > MAX_RETRIES) {
			ULOG_ERR("Aborting request (%02x) after %d attempts\n",
				 cmd->cmd[0], cmd->num_retries);
			free(cmd);
			return;
		}

		/* Wait for the MCU to recover */
		mcu->error_timeout.cb = mcu_clear_timeout;
		uloop_timeout_set(&mcu->error_timeout, 100);
	}

	list_add(&cmd->list, &mcu->pending_cmds);
}

static int mcu_handle_reply(struct mcu *mcu, uint8_t *reply)
{
	struct cmd *cmd = NULL;
	uint8_t sum = 0, i, cmd_id, cmd_seq;

	log_packet(LOG_DEBUG, "RX <-", reply);

	if (list_empty(&mcu->pending_cmds)) {
		ULOG_ERR("received unsolicited reply\n");
		return -1;
	}

	cmd = list_first_entry(&mcu->pending_cmds, struct cmd, list);
	list_del(&cmd->list);
	cmd_id = cmd->cmd[0];
	cmd_seq = cmd->cmd[1];

	for (i = 0; i < OFFSET_CHECKSUM; i++)
		sum += reply[i];

	if (reply[OFFSET_CHECKSUM] != sum) {
		ULOG_DBG("received reply with bad checksum\n");
		free(cmd);
		return -1;
	}

	if ((reply[0] & 0xf0) == 0xf0) {
		handle_f0_reply(mcu, cmd, reply);
		return -1;
	}

	free(cmd);

	if ((reply[0] != cmd_id) || (reply[0] > ARRAY_SIZE(reply_handler))) {
		ULOG_DBG("received reply with bad command id\n");
		return -1;
	}

	if (reply[1] != cmd_seq) {
		ULOG_DBG("received reply with bad sequence number\n");
		return -1;
	}

	if (reply_handler[reply[0]]) {
		return reply_handler[reply[0]](&mcu->state, reply);
	}

	return 0;
}

static void poe_stream_msg_cb(struct ustream *s, int bytes)
{
	struct ustream_fd *ufd = container_of(s, struct ustream_fd, stream);
	struct mcu *mcu = container_of(ufd, struct mcu, stream);
	int len;
	uint8_t *reply = (uint8_t *)ustream_get_read_buf(s, &len);

	if (len < 12)
		return;
	mcu_handle_reply(mcu, reply);
	ustream_consume(s, 12);
	mcu_cmd_next(mcu);
}

static void poe_stream_notify_cb(struct ustream *s)
{
	if (!s->eof)
		return;

	ULOG_ERR("tty error, shutting down\n");
	exit(-1);
}

static int poe_stream_open(char *dev, struct ustream_fd *s, speed_t speed)
{
	int ret, tty;

	struct termios tio = {
		.c_oflag = 0,
		.c_iflag = 0,
		.c_cflag = speed | CS8 | CREAD | CLOCAL,
		.c_lflag = 0,
		.c_cc = {
			[VMIN] = 1,
		}
	};

	tty = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (tty < 0) {
		ULOG_ERR("%s: device open failed: %s\n", dev, strerror(errno));
		return -1;
	}

	ret = tcsetattr(tty, TCSANOW, &tio);
	if (ret) {
		ULOG_ERR("Can't configure serial port: %s", strerror(errno));
		return -errno;
	}

	s->stream.string_data = false;
	s->stream.notify_read = poe_stream_msg_cb;
	s->stream.notify_state = poe_stream_notify_cb;

	ustream_fd_init(s, tty);
	tcflush(tty, TCIFLUSH);

	return 0;
}

static int poet_setup(struct mcu* mcu, const struct port_config *ports,
		      size_t num_ports)
{
	uint8_t port_ids[4], priorities[4], powerup_mode[4], limit_type[4];
	uint8_t enable_all[4] = {1, 1, 1, 1};
	size_t i = 0, num_okay = 0;

	do {
		for ( ; i < num_ports; i++) {
			if (!ports[i].enable)
				continue;

			port_ids[num_okay] = i;
			priorities[num_okay] = ports[i].priority;
			powerup_mode[num_okay] = ports[i].power_up_mode;
			limit_type[num_okay] = (ports[i].power_budget) ? 2 : 1;

			if (++num_okay == 4)
				break;
		};

		memset(enable_all + num_okay, 0xff, 4 - num_okay);
		memset(port_ids + num_okay, 0xff, 4 - num_okay);
		memset(priorities + num_okay, 0xff, 4 - num_okay);
		memset(powerup_mode + num_okay, 0xff, 4 - num_okay);
		memset(limit_type + num_okay, 0xff, 4 - num_okay);

		poe_set_port_priority(mcu, port_ids, priorities);
		poe_set_port_power_up_mode(mcu, port_ids, powerup_mode);
		poe_cmd_port_classification(mcu, port_ids, enable_all);
		poe_cmd_port_power_limit_type(mcu, port_ids, limit_type);

		num_okay = 0;
	} while (++i < num_ports);

	return 0;
}

static int poe_port_setup(struct mcu* mcu, const struct config *cfg)
{
	size_t i;

	poe_cmd_port_disconnect_type(mcu, PORT_ID_ALL, 2);
	poe_cmd_port_detection_type(mcu, PORT_ID_ALL, 3);

	for (i = 0; i < cfg->port_count; i++) {
		if (!cfg->ports[i].enable || !cfg->ports[i].power_budget)
			continue;

		poe_cmd_port_power_budget(mcu, i, cfg->ports[i].power_budget);
	}

	poet_setup(mcu, cfg->ports, cfg->port_count);

	for (i = 0; i < cfg->port_count; i++)
		poe_cmd_port_enable(mcu, i, !!cfg->ports[i].enable);

	return 0;
}

static void poe_set_power_budget(struct mcu* mcu, const struct config *config)
{
	unsigned int pse;

	for (pse = 0; pse < 8; pse++) {
		if (!(config->pse_id_set_budget_mask & (1 << pse)))
			continue;

		poe_cmd_global_power_budget(mcu, pse, config->budget,
					    config->budget_guard);
	}
}

static int poe_initial_setup(struct mcu* mcu, const struct config *cfg)
{
	poe_cmd_status(mcu);
	poe_cmd_power_mgmt_mode(mcu, 2);
	poe_cmd_port_mapping_enable(mcu, false);
	poe_set_power_budget(mcu, cfg);

	poe_port_setup(mcu, cfg);

	return 0;
}

static void state_timeout_cb(struct uloop_timeout *t)
{
	struct poe_ctx *poe = container_of(t, struct poe_ctx, state_timeout);
	const struct config *cfg = &poe->config;
	struct mcu *mcu = &poe->mcu;
	size_t i;

	poe_cmd_power_stats(mcu);
	if (poe->hardcore_hacking_mode_en)
		poe_cmd_get_extended_config(mcu);

	for (i = 0; i < cfg->port_count; i += 4)
		poe_cmd_4_port_status(mcu, i, i + 1, i + 2, i + 3);

	for (i = 0; i < cfg->port_count; i++) {
		if (poe->hardcore_hacking_mode_en) {
			poe_cmd_port_status(mcu, i);
			poe_cmd_port_config(mcu, i);
		}

		poe_cmd_port_ext_config(mcu, i);
		poe_cmd_port_power_stats(mcu, i);
	}

	uloop_timeout_set(t, 2 * 1000);
}

static int ubus_poe_info_cb(struct ubus_context *ctx, struct ubus_object *obj,
			    struct ubus_request_data *req, const char *method,
			    struct blob_attr *msg)
{
	struct poe_ctx *poe = ubus_to_poe_ctx(ctx);
	const struct mcu_state *state = &poe->mcu.state;
	const struct config *cfg = &poe->config;
	struct blob_buf *b = &poe->blob_buf;
	char tmp[16];
	size_t i;
	void *c;

	blob_buf_init(b, 0);

	snprintf(tmp, sizeof(tmp), "v%u.%u",
		 state->sys_version, state->sys_ext_version);
	blobmsg_add_string(b, "firmware", tmp);
	if (state->sys_mcu)
		blobmsg_add_string(b, "mcu", state->sys_mcu);
	blobmsg_add_double(b, "budget", cfg->budget);
	blobmsg_add_double(b, "consumption", state->power_consumption);

	c = blobmsg_open_table(b, "ports");
	for (i = 0; i < cfg->port_count; i++) {
		void *p;

		if (!cfg->ports[i].valid)
			continue;

		p = blobmsg_open_table(b, cfg->ports[i].name);

		blobmsg_add_u32(b, "priority", cfg->ports[i].priority);

		if (state->ports[i].poe_mode)
			blobmsg_add_string(b, "mode", state->ports[i].poe_mode);
		if (state->ports[i].status)
			blobmsg_add_string(b, "status", state->ports[i].status);
		else
			blobmsg_add_string(b, "status", "unknown");
		if (state->ports[i].watt)
			blobmsg_add_double(b, "consumption", state->ports[i].watt);

		blobmsg_close_table(b, p);
	}
	blobmsg_close_table(b, c);

	ubus_send_reply(ctx, req, b->head);

	return UBUS_STATUS_OK;
}

static int ubus_poe_debug_cb(struct ubus_context *ctx, struct ubus_object *obj,
			    struct ubus_request_data *req, const char *method,
			    struct blob_attr *msg)
{
	struct poe_ctx *poe = ubus_to_poe_ctx(ctx);
	const struct mcu_state *state = &poe->mcu.state;
	const struct config *cfg = &poe->config;
	struct blob_buf *b = &poe->blob_buf;
	size_t i;
	void *c, *p;

	blob_buf_init(b, 0);

	blobmsg_add_double(b, "reported_budget", state->reported_power_budget);


	blobmsg_add_u32(b, "num_detected_ports", state->num_detected_ports);
	blobmsg_add_u32(b, "port_map_en", state->port_map_en);
	blobmsg_add_u32(b, "device_id", state->device_id);

	if (state->has_ext_cfg_info) {
		blobmsg_add_double(b, "uvlo_threshold", state->uvlo_threshold);
		blobmsg_add_double(b, "ovlo_threshold", state->ovlo_threshold);
		blobmsg_add_u32(b, "pre_alloc", state->pre_alloc);
		blobmsg_add_u32(b, "powerup_mode", state->powerup_mode);
		blobmsg_add_u32(b, "disconnect_type", state->disconnect_type);
		blobmsg_add_u32(b, "ddflag", state->ddflag);
		blobmsg_add_u32(b, "num_pse", state->num_pse);
	}

	c = blobmsg_open_table(b, "ports");
	for (i = 0; i < cfg->port_count; i++) {
		if (!cfg->ports[i].valid)
			continue;

		p = blobmsg_open_table(b, cfg->ports[i].name);

		blobmsg_add_u32(b, "power_limit_type", state->ports[i].power_limit_type);
		blobmsg_add_double(b, "power_budget", state->ports[i].power_budget);
		blobmsg_add_u32(b, "priority", state->ports[i].priority);
		blobmsg_add_u32(b, "primary_pse_output", state->ports[i].primary_pse_output);
		blobmsg_add_u32(b, "mapping", state->ports[i].mapping);

		if (state->ports[i].has_config_info) {
			blobmsg_add_u32(b, "enabled", state->ports[i].enabled);
			blobmsg_add_u32(b, "auto_powerup", state->ports[i].auto_powerup);
			blobmsg_add_u32(b, "detection_type", state->ports[i].detection_type);
			blobmsg_add_u32(b, "classification_enable", state->ports[i].classification_enable);
			blobmsg_add_u32(b, "disconnect_type", state->ports[i].disconnect_type);
			blobmsg_add_u32(b, "pair", state->ports[i].pair);
		}

		if (state->ports[i].has_detailed_state) {
			blobmsg_add_u32(b, "class_info", state->ports[i].class_info);
			blobmsg_add_u32(b, "pd_type", state->ports[i].pd_type);
			blobmsg_add_u32(b, "mpss_mask", state->ports[i].mpss_mask);
		}

		blobmsg_close_table(b, p);
	}
	blobmsg_close_table(b, c);

	p = blobmsg_open_table(b, "mapping");
	blobmsg_add_u32(b, "enabled", state->port_map_en);
	for (i = 0; i < cfg->port_count; i++) {
		if (!cfg->ports[i].valid)
			continue;

		blobmsg_add_u32(b, cfg->ports[i].name, state->ports[i].mapping);
	}
	blobmsg_close_table(b, p);

	ubus_send_reply(ctx, req, b->head);

	return UBUS_STATUS_OK;
}

static const struct blobmsg_policy ubus_poe_sendframe_policy[] = {
	{ "frame", BLOBMSG_TYPE_STRING },
};

static int
ubus_poe_sendframe_cb(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(ubus_poe_sendframe_policy)];
	struct poe_ctx *poe = ubus_to_poe_ctx(ctx);
	struct mcu *mcu = &poe->mcu;
	char *frame, *next, *end;
	size_t cmd_len = 0;
	unsigned long byte_val;
	uint8_t cmd[CMD_SIZE];
	int ret;

	if (!poe->hardcore_hacking_mode_en)
		return UBUS_STATUS_PERMISSION_DENIED;

	blobmsg_parse(ubus_poe_sendframe_policy,
		      ARRAY_SIZE(ubus_poe_sendframe_policy),
		      tb, blob_data(msg), blob_len(msg));
	if (!*tb)
		return UBUS_STATUS_INVALID_ARGUMENT;

	frame = blobmsg_get_string(*tb);
	end = frame + strlen(frame);
	next = frame;

	while ((next < end) && (cmd_len < sizeof(cmd))) {
		errno = 0;
		byte_val = strtoul(frame, &next, 16);
		if (errno || (frame == next) || (byte_val > 0xff))
			return UBUS_STATUS_INVALID_ARGUMENT;

		cmd[cmd_len++] = byte_val;
		frame = next;
	}

	ret = mcu_queue_cmd(mcu, cmd, cmd_len);
	return (ret < 0) ?  UBUS_STATUS_SYSTEM_ERROR : UBUS_STATUS_OK;
}

static int ubus_poe_reload_cb(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req, const char *method,
			      struct blob_attr *msg)
{
	struct poe_ctx *poe = ubus_to_poe_ctx(ctx);

	config_load(&poe->config, 0);
	poe_port_setup(&poe->mcu, &poe->config);

	return UBUS_STATUS_OK;
}

static const struct blobmsg_policy ubus_poe_manage_policy[] = {
	{ "port", BLOBMSG_TYPE_STRING },
	{ "enable", BLOBMSG_TYPE_BOOL },
};

static int ubus_poe_manage_cb(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req, const char *method,
			      struct blob_attr *msg)
{
	struct blob_attr *tb[ARRAY_SIZE(ubus_poe_manage_policy)];
	struct poe_ctx *poe = ubus_to_poe_ctx(ctx);
	const struct config *cfg = &poe->config;
	const struct port_config *port;
	struct mcu *mcu = &poe->mcu;
	const char *port_name;
	size_t i;

	blobmsg_parse(ubus_poe_manage_policy,
		      ARRAY_SIZE(ubus_poe_manage_policy),
		      tb, blob_data(msg), blob_len(msg));
	if (!tb[0] || !tb[1])
		return UBUS_STATUS_INVALID_ARGUMENT;

	port_name = blobmsg_get_string(tb[0]);
	for (i = 0; i < cfg->port_count; i++) {
		port = &cfg->ports[i];
		if (!port->enable || strcmp(port_name, port->name))
			continue;
		return poe_cmd_port_enable(mcu, i, blobmsg_get_bool(tb[1]));
	}
	return UBUS_STATUS_INVALID_ARGUMENT;
}

static const struct ubus_method ubus_poe_methods[] = {
	UBUS_METHOD_NOARG("info", ubus_poe_info_cb),
	UBUS_METHOD_NOARG("debug", ubus_poe_debug_cb),
	UBUS_METHOD_NOARG("reload", ubus_poe_reload_cb),
	UBUS_METHOD("sendframe", ubus_poe_sendframe_cb, ubus_poe_sendframe_policy),
	UBUS_METHOD("manage", ubus_poe_manage_cb, ubus_poe_manage_policy),
};

static struct ubus_object_type ubus_poe_object_type =
	UBUS_OBJECT_TYPE("poe", ubus_poe_methods);

static struct ubus_object ubus_poe_object = {
	.name = "poe",
	.type = &ubus_poe_object_type,
	.methods = ubus_poe_methods,
	.n_methods = ARRAY_SIZE(ubus_poe_methods),
};

static void ubus_connect_handler(struct ubus_context *ctx)
{
	int ret;

	ret = ubus_add_object(ctx, &ubus_poe_object);
	if (ret)
		ULOG_ERR("Failed to add object: %s\n", ubus_strerror(ret));
}

int main(int argc, char **argv)
{
	int ch;

	struct poe_ctx poe = {
		.state_timeout.cb = state_timeout_cb,
		.conn.cb = ubus_connect_handler,
		.config = {
			.budget = 65,
			.budget_guard = 7,
			.pse_id_set_budget_mask = 0x01,
		},
	};

	INIT_LIST_HEAD(&poe.mcu.pending_cmds);
	ulog_open(ULOG_STDIO | ULOG_SYSLOG, LOG_DAEMON, "realtek-poe");
	ulog_threshold(LOG_INFO);

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			ulog_threshold(LOG_DEBUG);
			poe.hardcore_hacking_mode_en = 1;
			break;
		}
	}

	if (!poe.hardcore_hacking_mode_en)
		dd_daemon();

	config_load(&poe.config, 1);

	uloop_init();
	ubus_auto_connect(&poe.conn);

	if (poe_stream_open("/dev/ttyS1", &poe.mcu.stream, B19200) < 0)
		return -1;
	poe_initial_setup(&poe.mcu, &poe.config);
	uloop_timeout_set(&poe.state_timeout, 1000);
	uloop_run();
	uloop_done();

	return 0;
}
