/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#define SERVER_STATE_STARTING_UP	0
#define SERVER_STATE_RUNNING		1
#define SERVER_STATE_RESETTING		2
#define SERVER_STATE_SHUTTING_DOWN	3

#define SERVER_CONF_NETBIOS_NAME	0
#define SERVER_CONF_SERVER_STRING	1
#define SERVER_CONF_WORK_GROUP		2

extern int smbd_debugging;

struct smbd_server_config {
	unsigned int		flags;
	unsigned int		state;
	short			signing;
	short			enforced_signing;
	short			min_protocol;
	short			max_protocol;
	unsigned short		tcp_port;
	unsigned short		ipc_timeout;
	unsigned long		ipc_last_active;
	unsigned long		deadtime;

	char			*conf[SERVER_CONF_WORK_GROUP + 1];
};

extern struct smbd_server_config server_conf;

int smbd_set_netbios_name(char *v);
int smbd_set_server_string(char *v);
int smbd_set_work_group(char *v);

char *smbd_netbios_name(void);
char *smbd_server_string(void);
char *smbd_work_group(void);

static inline int smbd_server_running(void)
{
	return READ_ONCE(server_conf.state) == SERVER_STATE_RUNNING;
}

static inline int smbd_server_configurable(void)
{
	return READ_ONCE(server_conf.state) < SERVER_STATE_RESETTING;
}

int server_queue_ctrl_init_work(void);
int server_queue_ctrl_reset_work(void);
#endif /* __SERVER_H__ */
