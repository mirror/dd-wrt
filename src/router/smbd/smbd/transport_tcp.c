// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/freezer.h>

#include "smb_common.h"
#include "server.h"
#include "auth.h"
#include "buffer_pool.h"
#include "connection.h"
#include "transport_tcp.h"

struct interface {
	struct task_struct	*smbd_kthread;
	struct socket		*smbd_socket;
	struct list_head	entry;
	char			*name;
	struct mutex		sock_release_lock;
};

static LIST_HEAD(iface_list);

struct tcp_transport {
	struct smbd_transport		transport;
	struct socket			*sock;
	struct kvec			*iov;
	unsigned int			nr_iov;
};

static struct smbd_transport_ops smbd_tcp_transport_ops;

#define SMBD_TRANS(t)	(&(t)->transport)
#define TCP_TRANS(t)	((struct tcp_transport *)container_of(t, \
				struct tcp_transport, transport))

static inline void smbd_tcp_nodelay(struct socket *sock)
{
	int val = 1;

	kernel_setsockopt(sock, SOL_TCP, TCP_NODELAY,
		(char *)&val, sizeof(val));
}

static inline void smbd_tcp_reuseaddr(struct socket *sock)
{
	int val = 1;

	kernel_setsockopt(sock, SOL_TCP, SO_REUSEADDR,
		(char *)&val, sizeof(val));
}

static inline void smbd_tcp_rev_timeout(struct socket *sock, unsigned int sec)
{
	struct timeval tv = { .tv_sec = sec, .tv_usec = 0 };

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
	kernel_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO_OLD, (char *)&tv,
			  sizeof(tv));
#else
	kernel_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
			  sizeof(tv));
#endif
}

static inline void smbd_tcp_snd_timeout(struct socket *sock, unsigned int sec)
{
	struct timeval tv = { .tv_sec = sec, .tv_usec = 0 };

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
	kernel_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO_OLD, (char *)&tv,
			  sizeof(tv));
#else
	kernel_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,
			  sizeof(tv));
#endif
}

static struct tcp_transport *alloc_transport(struct socket *client_sk)
{
	struct tcp_transport *t;
	struct smbd_conn *conn;

	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (!t)
		return NULL;
	t->sock = client_sk;

	conn = smbd_conn_alloc();
	if (!conn) {
		kfree(t);
		return NULL;
	}

	conn->transport = SMBD_TRANS(t);
	SMBD_TRANS(t)->conn = conn;
	SMBD_TRANS(t)->ops = &smbd_tcp_transport_ops;
	return t;
}

static void free_transport(struct tcp_transport *t)
{
	kernel_sock_shutdown(t->sock, SHUT_RDWR);
	sock_release(t->sock);
	t->sock = NULL;

	smbd_conn_free(SMBD_TRANS(t)->conn);
	kfree(t->iov);
	kfree(t);
}

/**
 * kvec_array_init() - initialize a IO vector segment
 * @new:	IO vector to be initialized
 * @iov:	base IO vector
 * @nr_segs:	number of segments in base iov
 * @bytes:	total iovec length so far for read
 *
 * Return:	Number of IO segments
 */
static unsigned int kvec_array_init(struct kvec *new, struct kvec *iov,
				    unsigned int nr_segs, size_t bytes)
{
	size_t base = 0;

	while (bytes || !iov->iov_len) {
		int copy = min(bytes, iov->iov_len);

		bytes -= copy;
		base += copy;
		if (iov->iov_len == base) {
			iov++;
			nr_segs--;
			base = 0;
		}
	}

	memcpy(new, iov, sizeof(*iov) * nr_segs);
	new->iov_base += base;
	new->iov_len -= base;
	return nr_segs;
}

/**
 * get_conn_iovec() - get connection iovec for reading from socket
 * @t:		TCP transport instance
 * @nr_segs:	number of segments in iov
 *
 * Return:	return existing or newly allocate iovec
 */
static struct kvec *get_conn_iovec(struct tcp_transport *t,
				     unsigned int nr_segs)
{
	struct kvec *new_iov;

	if (t->iov && nr_segs <= t->nr_iov)
		return t->iov;

	/* not big enough -- allocate a new one and release the old */
	new_iov = kmalloc_array(nr_segs, sizeof(*new_iov), GFP_KERNEL);
	if (new_iov) {
		kfree(t->iov);
		t->iov = new_iov;
		t->nr_iov = nr_segs;
	}
	return new_iov;
}

static unsigned short smbd_tcp_get_port(const struct sockaddr *sa)
{
	switch (sa->sa_family) {
	case AF_INET:
		return ntohs(((struct sockaddr_in *)sa)->sin_port);
	case AF_INET6:
		return ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
	}
	return 0;
}

/**
 * smbd_tcp_new_connection() - create a new tcp session on mount
 * @sock:	socket associated with new connection
 *
 * whenever a new connection is requested, create a conn thread
 * (session thread) to handle new incoming smb requests from the connection
 *
 * Return:	0 on success, otherwise error
 */
static int smbd_tcp_new_connection(struct socket *client_sk)
{
	struct sockaddr *csin;
	int rc = 0;
	struct tcp_transport *t;

	t = alloc_transport(client_sk);
	if (!t)
		return -ENOMEM;

	csin = SMBD_TCP_PEER_SOCKADDR(SMBD_TRANS(t)->conn);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 16, 0)
	if (kernel_getpeername(client_sk, csin, &rc) < 0) {
		smbd_err("client ip resolution failed\n");
		rc = -EINVAL;
		goto out_error;
	}
	rc = 0;
#else
	if (kernel_getpeername(client_sk, csin) < 0) {
		smbd_err("client ip resolution failed\n");
		rc = -EINVAL;
		goto out_error;
	}
#endif
	SMBD_TRANS(t)->handler = kthread_run(smbd_conn_handler_loop,
					SMBD_TRANS(t)->conn,
					"ksmbd:%u", smbd_tcp_get_port(csin));
	if (IS_ERR(SMBD_TRANS(t)->handler)) {
		smbd_err("cannot start conn thread\n");
		rc = PTR_ERR(SMBD_TRANS(t)->handler);
		free_transport(t);
	}
	return rc;

out_error:
	free_transport(t);
	return rc;
}

/**
 * smbd_kthread_fn() - listen to new SMB connections and callback server
 * @p:		arguments to forker thread
 *
 * Return:	Returns a task_struct or ERR_PTR
 */
static int smbd_kthread_fn(void *p)
{
	struct socket *client_sk = NULL;
	struct interface *iface = (struct interface *)p;
	int ret;

	while (!kthread_should_stop()) {
		mutex_lock(&iface->sock_release_lock);
		if (!iface->smbd_socket) {
			mutex_unlock(&iface->sock_release_lock);
			break;
		}
		ret = kernel_accept(iface->smbd_socket, &client_sk,
				O_NONBLOCK);
		mutex_unlock(&iface->sock_release_lock);
		if (ret) {
			if (ret == -EAGAIN)
				/* check for new connections every 100 msecs */
				schedule_timeout_interruptible(HZ / 10);
			continue;
		}

		smbd_debug("connect success: accepted new connection\n");
		client_sk->sk->sk_rcvtimeo = SMBD_TCP_RECV_TIMEOUT;
		client_sk->sk->sk_sndtimeo = SMBD_TCP_SEND_TIMEOUT;

		smbd_tcp_new_connection(client_sk);
	}

	smbd_debug("releasing socket\n");
	return 0;
}

/**
 * smbd_create_smbd_kthread() - start forker thread
 *
 * start forker thread(ksmbd/0) at module init time to listen
 * on port 445 for new SMB connection requests. It creates per connection
 * server threads(ksmbd/x)
 *
 * Return:	0 on success or error number
 */
static int smbd_tcp_run_kthread(struct interface *iface)
{
	int rc;
	struct task_struct *kthread;

	kthread = kthread_run(smbd_kthread_fn, (void *)iface,
		"ksmbd-%s", iface->name);
	if (IS_ERR(kthread)) {
		rc = PTR_ERR(kthread);
		return rc;
	}
	iface->smbd_kthread = kthread;

	return 0;
}

/**
 * smbd_tcp_readv() - read data from socket in given iovec
 * @t:		TCP transport instance
 * @iov_orig:	base IO vector
 * @nr_segs:	number of segments in base iov
 * @to_read:	number of bytes to read from socket
 *
 * Return:	on success return number of bytes read from socket,
 *		otherwise return error number
 */
static int smbd_tcp_readv(struct tcp_transport *t,
			   struct kvec *iov_orig,
			   unsigned int nr_segs,
			   unsigned int to_read)
{
	int length = 0;
	int total_read;
	unsigned int segs;
	struct msghdr smbd_msg;
	struct kvec *iov;
	struct smbd_conn *conn = SMBD_TRANS(t)->conn;

	iov = get_conn_iovec(t, nr_segs);
	if (!iov)
		return -ENOMEM;

	smbd_msg.msg_control = NULL;
	smbd_msg.msg_controllen = 0;

	for (total_read = 0; to_read; total_read += length, to_read -= length) {
		try_to_freeze();

		if (!smbd_conn_alive(conn)) {
			total_read = -ESHUTDOWN;
			break;
		}
		segs = kvec_array_init(iov, iov_orig, nr_segs, total_read);

		length = kernel_recvmsg(t->sock, &smbd_msg,
					iov, segs, to_read, 0);

		if (length == -EINTR) {
			total_read = -ESHUTDOWN;
			break;
		} else if (conn->status == SMBD_SESS_NEED_RECONNECT) {
			total_read = -EAGAIN;
			break;
		} else if (length == -ERESTARTSYS || length == -EAGAIN) {
			usleep_range(1000, 2000);
			length = 0;
			continue;
		} else if (length <= 0) {
			total_read = -EAGAIN;
			break;
		}
	}
	return total_read;
}

/**
 * smbd_tcp_read() - read data from socket in given buffer
 * @t:		TCP transport instance
 * @buf:	buffer to store read data from socket
 * @to_read:	number of bytes to read from socket
 *
 * Return:	on success return number of bytes read from socket,
 *		otherwise return error number
 */
static int smbd_tcp_read(struct smbd_transport *t,
		   char *buf,
		   unsigned int to_read)
{
	struct kvec iov;

	iov.iov_base = buf;
	iov.iov_len = to_read;

	return smbd_tcp_readv(TCP_TRANS(t), &iov, 1, to_read);
}

static int smbd_tcp_writev(struct smbd_transport *t,
			struct kvec *iov, int nvecs, int size,
			bool need_invalidate, unsigned int remote_key)

{
	struct msghdr smb_msg = {.msg_flags = MSG_NOSIGNAL};

	return kernel_sendmsg(TCP_TRANS(t)->sock, &smb_msg, iov, nvecs, size);
}

static void smbd_tcp_disconnect(struct smbd_transport *t)
{
	free_transport(TCP_TRANS(t));
}

static void tcp_destroy_socket(struct socket *smbd_socket)
{
	int ret;

	if (!smbd_socket)
		return;

	/* set zero to timeout */
	smbd_tcp_rev_timeout(smbd_socket, 0);
	smbd_tcp_snd_timeout(smbd_socket, 0);

	ret = kernel_sock_shutdown(smbd_socket, SHUT_RDWR);
	if (ret)
		smbd_err("Failed to shutdown socket: %d\n", ret);
	else
		sock_release(smbd_socket);
}

/**
 * create_socket - create socket for ksmbd/0
 *
 * Return:	Returns a task_struct or ERR_PTR
 */
static int create_socket(struct interface *iface)
{
	int ret;
	struct sockaddr_in sin;
	struct socket *smbd_socket;

	ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &smbd_socket);
	if (ret) {
		smbd_err("Can't create socket: %d\n", ret);
		goto out_error;
	}

	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = PF_INET;
	sin.sin_port = htons(server_conf.tcp_port);

	smbd_tcp_nodelay(smbd_socket);
	smbd_tcp_reuseaddr(smbd_socket);

	ret = kernel_setsockopt(smbd_socket,
				SOL_SOCKET,
				SO_BINDTODEVICE,
				iface->name,
				strlen(iface->name));
	if (ret != -ENODEV && ret < 0) {
		smbd_err("Failed to set SO_BINDTODEVICE: %d\n", ret);
		goto out_error;
	}

	ret = kernel_bind(smbd_socket, (struct sockaddr *)&sin, sizeof(sin));
	if (ret) {
		smbd_err("Failed to bind socket: %d\n", ret);
		goto out_error;
	}

	smbd_socket->sk->sk_rcvtimeo = SMBD_TCP_RECV_TIMEOUT;
	smbd_socket->sk->sk_sndtimeo = SMBD_TCP_SEND_TIMEOUT;

	ret = kernel_listen(smbd_socket, SMBD_SOCKET_BACKLOG);
	if (ret) {
		smbd_err("Port listen() error: %d\n", ret);
		goto out_error;
	}

	iface->smbd_socket = smbd_socket;
	ret = smbd_tcp_run_kthread(iface);
	if (ret) {
		smbd_err("Can't start smbd main kthread: %d\n", ret);
		goto out_error;
	}

	return 0;

out_error:
	tcp_destroy_socket(smbd_socket);
	iface->smbd_socket = NULL;
	return ret;
}

int smbd_tcp_init(void)
{
	struct interface *iface;
	struct list_head *tmp;
	int ret;

	if (list_empty(&iface_list))
		return 0;

	list_for_each(tmp, &iface_list) {
		iface = list_entry(tmp, struct interface, entry);
		ret = create_socket(iface);
		if (ret)
			break;
	}

	return ret;
}

static void tcp_stop_kthread(struct task_struct *kthread)
{
	int ret;

	if (!kthread)
		return;

	ret = kthread_stop(kthread);
	if (ret)
		smbd_err("failed to stop forker thread\n");
}

void smbd_tcp_destroy(void)
{
	struct interface *iface, *tmp;

	list_for_each_entry_safe(iface, tmp, &iface_list, entry) {
		list_del(&iface->entry);
		tcp_stop_kthread(iface->smbd_kthread);
		mutex_lock(&iface->sock_release_lock);
		tcp_destroy_socket(iface->smbd_socket);
		iface->smbd_socket = NULL;
		mutex_unlock(&iface->sock_release_lock);
		kfree(iface->name);
		smbd_free(iface);
	}
}

static bool iface_exists(const char *ifname)
{
	struct net_device *netdev;
	bool ret = false;

	rcu_read_lock();
	netdev = dev_get_by_name_rcu(&init_net, ifname);
	if (netdev) {
		if (!(netdev->flags & IFF_UP))
			smbd_err("Device %s is down\n", ifname);
		else
			ret = true;
	}
	rcu_read_unlock();
	return ret;
}

static int alloc_iface(char *ifname)
{
	struct interface *iface;

	if (!ifname)
		return -ENOMEM;

	iface = smbd_alloc(sizeof(struct interface));
	if (!iface) {
		kfree(ifname);
		return -ENOMEM;
	}

	iface->name = ifname;
	list_add(&iface->entry, &iface_list);
	mutex_init(&iface->sock_release_lock);
	return 0;
}

int smbd_tcp_set_interfaces(char *ifc_list, int ifc_list_sz)
{
	int sz = 0;

	if (!ifc_list_sz) {
		struct net_device *netdev;

		rtnl_lock();
		for_each_netdev(&init_net, netdev) {
			if (alloc_iface(kstrdup(netdev->name, GFP_KERNEL)))
				return -ENOMEM;
		}
		rtnl_unlock();
		return 0;
	}

	while (ifc_list_sz > 0) {
		if (iface_exists(ifc_list)) {
			if (alloc_iface(kstrdup(ifc_list, GFP_KERNEL)))
				return -ENOMEM;
		} else {
			smbd_err("Unknown interface: %s\n", ifc_list);
		}

		sz = strlen(ifc_list);
		if (!sz)
			break;

		ifc_list += sz + 1;
		ifc_list_sz -= (sz + 1);
	}

	return 0;
}

static struct smbd_transport_ops smbd_tcp_transport_ops = {
	.read		= smbd_tcp_read,
	.writev		= smbd_tcp_writev,
	.disconnect	= smbd_tcp_disconnect,
};
