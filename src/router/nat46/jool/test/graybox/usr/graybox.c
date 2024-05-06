#include <errno.h>
#include <stddef.h>
#include <netlink/attr.h>

#include "common/graybox-types.h"
#include "usr/command/expect.h"
#include "usr/command/send.h"
#include "usr/command/stats.h"
#include "usr/genetlink.h"
#include "usr/log.h"

struct request {
	enum graybox_command cmd;
	union {
		struct expect_add_request expect_add;
		struct send_request send;
	};
};

static int request_init(int argc, char **argv, struct request *request)
{
	char *type;

	if (argc < 1) {
		pr_err("I need at least 1 argument. (see `man graybox`)");
		return -EINVAL;
	}
	memset(request, 0, sizeof(*request));
	type = argv[0];
	argc -= 1;
	argv += 1;

	if (strcasecmp(type, "expect") == 0) {
		return expect_init_request(argc, argv, &request->cmd, &request->expect_add);
	} else if (strcasecmp(type, "send") == 0) {
		return send_init_request(argc, argv, &request->cmd, &request->send);
	} else if (strcasecmp(type, "stats") == 0) {
		return stats_init_request(argc, argv, &request->cmd);
	}

	pr_err("'%s' is an unknown operation.", type);
	return -EINVAL;
}

static void request_clean(struct request *req)
{
	switch (req->cmd) {
	case COMMAND_EXPECT_ADD:
		expect_add_clean(&req->expect_add);
		break;
	case COMMAND_SEND:
		send_clean(&req->send);
		break;
	case COMMAND_EXPECT_FLUSH:
	case COMMAND_STATS_DISPLAY:
	case COMMAND_STATS_FLUSH:
		break;
	}
}

static int build_packet(struct request *req, struct nl_msg **result)
{
	struct nl_msg *msg;
	int error;

	error = nlsocket_create_msg(req->cmd, &msg);
	if (error)
		return error;

	switch (req->cmd) {
	case COMMAND_EXPECT_ADD:
		error = expect_add_build_pkt(&req->expect_add, msg);
		break;
	case COMMAND_SEND:
		error = send_build_pkt(&req->send, msg);
		break;
	case COMMAND_EXPECT_FLUSH:
	case COMMAND_STATS_DISPLAY:
	case COMMAND_STATS_FLUSH:
		break;
	}

	if (error) {
		pr_err("Could not write on the packet to kernelspace.");
		nlmsg_free(msg);
		return netlink_print_error(error);
	}

	*result = msg;
	return 0;
}

static int send_request(struct request *req, struct nl_msg *msg)
{
	switch (req->cmd) {
	case COMMAND_EXPECT_ADD:
	case COMMAND_EXPECT_FLUSH:
	case COMMAND_SEND:
	case COMMAND_STATS_FLUSH:
		return nlsocket_send(msg, NULL, NULL);
	case COMMAND_STATS_DISPLAY:
		return nlsocket_send(msg, stats_response_handle, NULL);
	}

	pr_err("Unknown command code: %d", req->cmd);
	return -EINVAL;
}

int main(int argc, char *argv[])
{
	struct request req;
	struct nl_msg *msg = NULL;
	int error;

	error = request_init(argc - 1, argv + 1, &req);
	if (error)
		goto end1;

	error = nlsocket_setup("graybox");
	if (error)
		goto end1;

	error = build_packet(&req, &msg);
	if (error)
		goto end2;

	error = send_request(&req, msg);

	nlmsg_free(msg);
end2:
	nlsocket_teardown();
end1:
	request_clean(&req);
	return error;
}
