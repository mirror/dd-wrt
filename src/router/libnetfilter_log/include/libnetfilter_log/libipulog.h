#ifndef _LIBIPULOG_H
#define _LIBIPULOG_H

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/* FIXME: glibc sucks */
#ifndef MSG_TRUNC 
#define MSG_TRUNC	0x20
#endif

#define ULOG_MAC_LEN	80
#define ULOG_PREFIX_LEN	32
#define ULOG_IFNAMSIZ	16

/* Format of the ULOG packets passed through netlink */
typedef struct ulog_packet_msg {
	unsigned long mark;
	long timestamp_sec;
	long timestamp_usec;
	unsigned int hook;
	char indev_name[ULOG_IFNAMSIZ];
	char outdev_name[ULOG_IFNAMSIZ];
	size_t data_len;
	char prefix[ULOG_PREFIX_LEN];
	unsigned char mac_len;
	unsigned char mac[ULOG_MAC_LEN];
	unsigned char payload[0];
} ulog_packet_msg_t;

struct ipulog_handle;
extern int ipulog_errno;

u_int32_t ipulog_group2gmask(u_int32_t group);

struct ipulog_handle *ipulog_create_handle(u_int32_t gmask, u_int32_t rmem);

void ipulog_destroy_handle(struct ipulog_handle *h);

ssize_t ipulog_read(struct ipulog_handle *h,
		    unsigned char *buf, size_t len, int timeout);

ulog_packet_msg_t *ipulog_get_packet(struct ipulog_handle *h,
				     const unsigned char *buf,
				     size_t len);

const char *ipulog_strerror(int errcode);

void ipulog_perror(const char *s);

enum 
{
	IPULOG_ERR_NONE = 0,
	IPULOG_ERR_IMPL,
	IPULOG_ERR_HANDLE,
	IPULOG_ERR_SOCKET,
	IPULOG_ERR_BIND,
	IPULOG_ERR_RECVBUF,
	IPULOG_ERR_RECV,
	IPULOG_ERR_NLEOF,
	IPULOG_ERR_TRUNC,
	IPULOG_ERR_INVGR,
	IPULOG_ERR_INVNL,
};
#define IPULOG_MAXERR IPULOG_ERR_INVNL


#endif /* _LIBIPULOG_H */
