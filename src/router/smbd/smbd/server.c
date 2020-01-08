// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include "glob.h"
#include "oplock.h"
#include "misc.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif
#include <linux/workqueue.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "server.h"
#include "smb_common.h"
#include "smbstatus.h"
#include "buffer_pool.h"
#include "connection.h"
#include "transport_ipc.h"
#include "mgmt/user_session.h"
#include "crypto_ctx.h"

#ifdef CONFIG_SMB_SERVER_DEBUGGING
int smbd_debugging;
#endif

struct smbd_server_config server_conf;

enum SERVER_CTRL_TYPE {
	SERVER_CTRL_TYPE_INIT,
	SERVER_CTRL_TYPE_RESET,
};

struct server_ctrl_struct {
	int			type;
	struct work_struct	ctrl_work;
};

static DEFINE_MUTEX(ctrl_lock);

static int ___server_conf_set(int idx, char *val)
{
	if (idx > sizeof(server_conf.conf))
		return -EINVAL;

	if (!val || val[0] == 0x00)
		return -EINVAL;

	kfree(server_conf.conf[idx]);
	server_conf.conf[idx] = kstrdup(val, GFP_KERNEL);
	if (!server_conf.conf[idx])
		return -ENOMEM;
	return 0;
}

int smbd_set_netbios_name(char *v)
{
	return ___server_conf_set(SERVER_CONF_NETBIOS_NAME, v);
}

int smbd_set_server_string(char *v)
{
	return ___server_conf_set(SERVER_CONF_SERVER_STRING, v);
}

int smbd_set_work_group(char *v)
{
	return ___server_conf_set(SERVER_CONF_WORK_GROUP, v);
}

char *smbd_netbios_name(void)
{
	return server_conf.conf[SERVER_CONF_NETBIOS_NAME];
}

char *smbd_server_string(void)
{
	return server_conf.conf[SERVER_CONF_SERVER_STRING];
}

char *smbd_work_group(void)
{
	return server_conf.conf[SERVER_CONF_WORK_GROUP];
}

/**
 * check_conn_state() - check state of server thread connection
 * @smbd_work:     smb work containing server thread information
 *
 * Return:	0 on valid connection, otherwise 1 to reconnect
 */
static inline int check_conn_state(struct smbd_work *work)
{
	struct smb_hdr *rsp_hdr;

	if (smbd_conn_exiting(work) || smbd_conn_need_reconnect(work)) {
		rsp_hdr = RESPONSE_BUF(work);
		rsp_hdr->Status.CifsError = STATUS_CONNECTION_DISCONNECTED;
		return 1;
	}
	return 0;
}

/* @FIXME what a mess... god help. */

#define TCP_HANDLER_CONTINUE	0
#define TCP_HANDLER_ABORT	1

static int __process_request(struct smbd_work *work,
			     struct smbd_conn *conn,
			     unsigned int *cmd)
{
	struct smb_version_cmds *cmds;
	unsigned int command;
	int ret;

	if (check_conn_state(work))
		return TCP_HANDLER_CONTINUE;

	if (smbd_verify_smb_message(work)) {
		work->send_no_response = 1;
		return TCP_HANDLER_ABORT;
	}

	command = conn->ops->get_cmd_val(work);
	*cmd = command;

andx_again:
	if (command >= conn->max_cmds) {
		conn->ops->set_rsp_status(work, STATUS_INVALID_PARAMETER);
		return TCP_HANDLER_CONTINUE;
	}

	cmds = &conn->cmds[command];
	if (!cmds->proc) {
		smbd_err("*** not implemented yet cmd = %x\n", command);
		conn->ops->set_rsp_status(work, STATUS_NOT_IMPLEMENTED);
		return TCP_HANDLER_CONTINUE;
	}

	if (work->sess && conn->ops->is_sign_req &&
		conn->ops->is_sign_req(work, command)) {
		ret = conn->ops->check_sign_req(work);
		if (!ret) {
			conn->ops->set_rsp_status(work, STATUS_DATA_ERROR);
			return TCP_HANDLER_CONTINUE;
		}
	}

	ret = cmds->proc(work);

	if (ret < 0)
		smbd_debug("Failed to process %u [%d]\n", command, ret);
	/* AndX commands - chained request can return positive values */
	else if (ret > 0) {
		command = ret;
		*cmd = command;
		goto andx_again;
	}

	if (work->send_no_response)
		return TCP_HANDLER_ABORT;
	return TCP_HANDLER_CONTINUE;
}

static void __handle_smbd_work(struct smbd_work *work,
				struct smbd_conn *conn)
{
	unsigned int command = 0;
	int rc;

	if (conn->ops->allocate_rsp_buf(work))
		return;

	if (conn->ops->is_transform_hdr &&
		conn->ops->is_transform_hdr(REQUEST_BUF(work))) {
		rc = conn->ops->decrypt_req(work);
		if (rc < 0) {
			conn->ops->set_rsp_status(work, STATUS_DATA_ERROR);
			goto send;
		}

		work->encrypted = true;
	}

	rc = conn->ops->init_rsp_hdr(work);
	if (rc) {
		/* either uid or tid is not correct */
		conn->ops->set_rsp_status(work, STATUS_INVALID_HANDLE);
		goto send;
	}

	if (conn->ops->check_user_session) {
		rc = conn->ops->check_user_session(work);
		if (rc < 0) {
			command = conn->ops->get_cmd_val(work);
			conn->ops->set_rsp_status(work,
					STATUS_USER_SESSION_DELETED);
			goto send;
		} else if (rc > 0) {
			rc = conn->ops->get_smbd_tcon(work);
			if (rc < 0) {
				conn->ops->set_rsp_status(work,
					STATUS_NETWORK_NAME_DELETED);
				goto send;
			}
		}
	}

	do {
		rc = __process_request(work, conn, &command);
		if (rc == TCP_HANDLER_ABORT)
			break;
	} while (is_chained_smb2_message(work));

send:
	smb3_preauth_hash_rsp(work);
	if (work->sess && work->sess->enc && work->encrypted &&
		conn->ops->encrypt_resp) {
		rc = conn->ops->encrypt_resp(work);
		if (rc < 0) {
			conn->ops->set_rsp_status(work, STATUS_DATA_ERROR);
			goto send;
		}
	} else if (work->sess && (work->sess->sign ||
		smb3_final_sess_setup_resp(work) ||
		(conn->ops->is_sign_req &&
		conn->ops->is_sign_req(work, command))))
		conn->ops->set_sign_rsp(work);

	smbd_conn_write(work);
}

/**
 * handle_smbd_work() - process pending smb work requests
 * @smbd_work:	smb work containing request command buffer
 *
 * called by kworker threads to processing remaining smb work requests
 */
static void handle_smbd_work(struct work_struct *wk)
{
	struct smbd_work *work = container_of(wk, struct smbd_work, work);
	struct smbd_conn *conn = work->conn;

	atomic64_inc(&conn->stats.request_served);

	__handle_smbd_work(work, conn);

	smbd_conn_try_dequeue_request(work);
	smbd_free_work_struct(work);
	atomic_dec(&conn->r_count);
}

/**
 * queue_smbd_work() - queue a smb request to worker thread queue
 *		for proccessing smb command and sending response
 * @conn:	connection instance
 *
 * read remaining data from socket create and submit work.
 */
static int queue_smbd_work(struct smbd_conn *conn)
{
	struct smbd_work *work;

	work = smbd_alloc_work_struct();
	if (!work) {
		smbd_err("allocation for work failed\n");
		return -ENOMEM;
	}

	work->conn = conn;
	work->request_buf = conn->request_buf;
	conn->request_buf = NULL;

	if (smbd_init_smb_server(work)) {
		smbd_free_work_struct(work);
		return -EINVAL;
	}

	smbd_conn_enqueue_request(work);
	atomic_inc(&conn->r_count);
	/* update activity on connection */
	conn->last_active = jiffies;
	INIT_WORK(&work->work, handle_smbd_work);
	smbd_queue_work(work);
	return 0;
}

static int smbd_server_process_request(struct smbd_conn *conn)
{
	return queue_smbd_work(conn);
}

static int smbd_server_terminate_conn(struct smbd_conn *conn)
{
	smbd_sessions_deregister(conn);
	destroy_lease_table(conn);
	return 0;
}

static void smbd_server_tcp_callbacks_init(void)
{
	struct smbd_conn_ops ops;

	ops.process_fn = smbd_server_process_request;
	ops.terminate_fn = smbd_server_terminate_conn;

	smbd_conn_init_server_callbacks(&ops);
}

static void server_conf_free(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(server_conf.conf); i++) {
		kfree(server_conf.conf[i]);
		server_conf.conf[i] = NULL;
	}
}

static int server_conf_init(void)
{
	WRITE_ONCE(server_conf.state, SERVER_STATE_STARTING_UP);
	server_conf.enforced_signing = 0;
	server_conf.min_protocol = smbd_min_protocol();
	server_conf.max_protocol = smbd_max_protocol();
	return 0;
}

static void server_ctrl_handle_init(struct server_ctrl_struct *ctrl)
{
	int ret;

	ret = smbd_conn_transport_init();
	if (ret) {
		server_queue_ctrl_reset_work();
		return;
	}

	WRITE_ONCE(server_conf.state, SERVER_STATE_RUNNING);
}

static void server_ctrl_handle_reset(struct server_ctrl_struct *ctrl)
{
	smbd_ipc_soft_reset();
	smbd_conn_transport_destroy();
	server_conf_free();
	server_conf_init();
	WRITE_ONCE(server_conf.state, SERVER_STATE_STARTING_UP);
}

static void server_ctrl_handle_work(struct work_struct *work)
{
	struct server_ctrl_struct *ctrl;

	ctrl = container_of(work, struct server_ctrl_struct, ctrl_work);

	mutex_lock(&ctrl_lock);
	switch (ctrl->type) {
	case SERVER_CTRL_TYPE_INIT:
		server_ctrl_handle_init(ctrl);
		break;
	case SERVER_CTRL_TYPE_RESET:
		server_ctrl_handle_reset(ctrl);
		break;
	default:
		pr_err("Unknown server work type: %d\n", ctrl->type);
	}
	mutex_unlock(&ctrl_lock);
	kfree(ctrl);
	module_put(THIS_MODULE);
}

static int __queue_ctrl_work(int type)
{
	struct server_ctrl_struct *ctrl;

	ctrl = kmalloc(sizeof(struct server_ctrl_struct), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	__module_get(THIS_MODULE);
	ctrl->type = type;
	INIT_WORK(&ctrl->ctrl_work, server_ctrl_handle_work);
	queue_work(system_long_wq, &ctrl->ctrl_work);
	return 0;
}

int server_queue_ctrl_init_work(void)
{
	return __queue_ctrl_work(SERVER_CTRL_TYPE_INIT);
}

int server_queue_ctrl_reset_work(void)
{
	return __queue_ctrl_work(SERVER_CTRL_TYPE_RESET);
}

static ssize_t stats_show(struct class *class,
			  struct class_attribute *attr,
			  char *buf)
{
	/*
	 * Inc this each time you change stats output format,
	 * so user space will know what to do.
	 */
	static int stats_version = 2;
	static const char * const state[] = {
		"startup",
		"running",
		"reset",
		"shutdown"
	};

	ssize_t sz = scnprintf(buf,
				PAGE_SIZE,
				"%d %s %d %lu\n",
				stats_version,
				state[server_conf.state],
				server_conf.tcp_port,
				server_conf.ipc_last_active / HZ);
	return sz;
}

static ssize_t kill_server_store(struct class *class,
				 struct class_attribute *attr,
				 const char *buf,
				 size_t len)
{
	if (!sysfs_streq(buf, "hard"))
		return len;

	smbd_err("kill command received\n");
	mutex_lock(&ctrl_lock);
	WRITE_ONCE(server_conf.state, SERVER_STATE_RESETTING);
	__module_get(THIS_MODULE);
	server_ctrl_handle_reset(NULL);
	module_put(THIS_MODULE);
	mutex_unlock(&ctrl_lock);
	return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
static CLASS_ATTR_RO(stats);
static CLASS_ATTR_WO(kill_server);

static struct attribute *smbd_control_class_attrs[] = {
	&class_attr_stats.attr,
	&class_attr_kill_server.attr,
	NULL,
};
ATTRIBUTE_GROUPS(smbd_control_class);

static struct class smbd_control_class = {
	.name		= "smbd-control",
	.owner		= THIS_MODULE,
	.class_groups	= smbd_control_class_groups,
};
#else
static struct class_attribute smbd_control_class_attrs[] = {
	__ATTR_RO(stats),
	__ATTR_WO(kill_server),
	__ATTR_NULL,
};

static struct class smbd_control_class = {
	.name		= "smbd-control",
	.owner		= THIS_MODULE,
	.class_attrs	= smbd_control_class_attrs,
};
#endif

static int smbd_server_shutdown(void)
{
	WRITE_ONCE(server_conf.state, SERVER_STATE_SHUTTING_DOWN);

	class_unregister(&smbd_control_class);
	smbd_workqueue_destroy();
	smbd_ipc_release();
	smbd_conn_transport_destroy();
	smbd_free_session_table();
	smbd_crypto_destroy();
	smbd_free_global_file_table();
	destroy_lease_table(NULL);
	smbd_destroy_buffer_pools();
	server_conf_free();
	return 0;
}

static int __init smbd_server_init(void)
{
	int ret;

	ret = class_register(&smbd_control_class);
	if (ret) {
		smbd_err("Unable to register smbd-control class\n");
		return ret;
	}

	smbd_server_tcp_callbacks_init();

	ret = server_conf_init();
	if (ret)
		return ret;

	ret = smbd_init_buffer_pools();
	if (ret)
		return ret;

	ret = smbd_init_session_table();
	if (ret)
		goto error;

	ret = smbd_ipc_init();
	if (ret)
		goto error;

	ret = smbd_init_global_file_table();
	if (ret)
		goto error;

	ret = smbd_inode_hash_init();
	if (ret)
		goto error;

	ret = smbd_crypto_create();
	if (ret)
		goto error;

	ret = smbd_workqueue_init();
	if (ret)
		goto error;
	return 0;

error:
	smbd_server_shutdown();
	return ret;
}

/**
 * exit_smb_server() - shutdown forker thread and free memory at module exit
 */
static void __exit smbd_server_exit(void)
{
	smbd_server_shutdown();
	smbd_release_inode_hash();
}

#ifdef CONFIG_SMB_SERVER_DEBUGGING
module_param(smbd_debugging, int, 0644);
MODULE_PARM_DESC(smbd_debugging, "Enable/disable SMBD debugging output");
#endif

MODULE_AUTHOR("Namjae Jeon <linkinjeon@gmail.com>");
MODULE_VERSION(SMBD_VERSION);
MODULE_DESCRIPTION("Linux kernel CIFS/SMB SERVER");
MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 11, 0)
MODULE_SOFTDEP("pre: arc4");
MODULE_SOFTDEP("pre: ecb");
MODULE_SOFTDEP("pre: hmac");
MODULE_SOFTDEP("pre: md4");
MODULE_SOFTDEP("pre: md5");
MODULE_SOFTDEP("pre: nls");
MODULE_SOFTDEP("pre: aes");
MODULE_SOFTDEP("pre: cmac");
MODULE_SOFTDEP("pre: sha256");
MODULE_SOFTDEP("pre: sha512");
MODULE_SOFTDEP("pre: aead2");
MODULE_SOFTDEP("pre: ccm");
MODULE_SOFTDEP("pre: gcm");
#endif
module_init(smbd_server_init)
module_exit(smbd_server_exit)
