#include "netlink.h"

/**
 * Prepares a PF_NETLINK socket into the kernel.
 *
 * Returns: Socket fd if succesful, -1 otherwise.
 */
int netlink_init() {
	int netlink_socket;
	int buffersize = 16 * 1024 * 1024;
	
	netlink_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT); 
	if (netlink_socket == -1) {
		return -1;
	}
	
	/*
	 * We're trying to override buffer size. If we fail, we attempt to set a big buffer and pray.
	 */
	if (setsockopt(netlink_socket, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize))) {
		/* Somewhat safe default. */
		buffersize = 106496;
		
		if (setsockopt(netlink_socket, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize))) {
			close(netlink_socket);
			return -1;
		}
	}
	
	return netlink_socket;
}

/**
 * Connects using a netlink socket.
 *
 * @1 Netlink socket
 *
 * Returns: @1 if succesful, -1 otherwise.
 */
int netlink_connect(int netlink_socket) {
	struct sockaddr_nl snl;

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	if (connect(netlink_socket, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl))) {
		close(netlink_socket);
		return -1;
	}

	return netlink_socket;
}

/**
 * Binds using a netlink socket.
 *
 * @1 Netlink socket
 *
 * Returns: @1 if succesful, -1 otherwise.
 */
int netlink_bind(int netlink_socket) {
	struct sockaddr_nl snl;

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	if (bind(netlink_socket, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl))) {
		close(netlink_socket);
		return -1;
	}

	return netlink_socket;
}
