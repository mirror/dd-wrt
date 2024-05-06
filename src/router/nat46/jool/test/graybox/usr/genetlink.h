#ifndef _GRAYBOX_USR_GENETLINK_H
#define _GRAYBOX_USR_GENETLINK_H

#include <netlink/msg.h>

int nlsocket_setup(char *family);
void nlsocket_teardown();

int nlsocket_create_msg(int cmd, struct nl_msg **msg);
typedef int (*joolnl_response_cb)(struct nlattr **attrs, void *);
int nlsocket_send(struct nl_msg *msg, joolnl_response_cb cb, void *cb_arg);

int netlink_print_error(int error);

#endif
