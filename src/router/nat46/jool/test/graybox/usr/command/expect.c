#include "usr/command/expect.h"

#include <errno.h>
#include "usr/log.h"
#include "usr/command/common.h"

static int parse_exceptions(char const *exceptions,
		struct expect_add_request *req)
{
	char *str_copy;
	char *token;
	unsigned int i;

	str_copy = strdup(exceptions);
	if (!str_copy) {
		pr_err("Out of memory.");
		return -ENOMEM;
	}

	i = 0;
	token = strtok(str_copy, ",");
	while (token) {
		if (i >= MAX_EXCEPTIONS) {
			pr_err("Too many exceptions.");
			goto fail;
		}

		req->exceptions[i] = strtoul(token, NULL, 10);
		if (errno && errno != ERANGE) {
			pr_err("Exception is not unsigned integer: %s", token);
			goto fail;
		}

		token = strtok(NULL, ",");
		i++;
	}

	req->exceptions_len = i;
	free(str_copy);
	return 0;

fail:
	free(str_copy);
	return -EINVAL;
}

int expect_init_request(int argc, char **argv, enum graybox_command *cmd,
		struct expect_add_request *req)
{
	int error;

	if (argc < 1) {
		pr_err("expect needs an operation as first argument.");
		return -EINVAL;
	}

	if (strcasecmp(argv[0], "add") == 0) {
		*cmd = COMMAND_EXPECT_ADD;

		if (argc < 2) {
			pr_err("expect add needs a packet as argument.");
			return -EINVAL;
		}

		req->file_name = argv[1];
		error = load_pkt(argv[1], &req->pkt, &req->pkt_len);
		if (error)
			return error;
		return (argc >= 3) ? parse_exceptions(argv[2], req) : 0;

	} else if (strcasecmp(argv[0], "flush") == 0) {
		*cmd = COMMAND_EXPECT_FLUSH;
		return 0;
	}

	pr_err("Unknown operation for expect: %s", argv[0]);
	return -EINVAL;
}

void expect_add_clean(struct expect_add_request *req)
{
	if (req->pkt)
		free(req->pkt);
}

int expect_add_build_pkt(struct expect_add_request *req, struct nl_msg *pkt)
{
	struct nlattr *root;
	unsigned int i;
	int error;

	error = nla_put_string(pkt, ATTR_FILENAME, req->file_name);
	if (error < 0)
		goto packet_too_small;

	error = nla_put(pkt, ATTR_PKT, req->pkt_len, req->pkt);
	if (error < 0)
		goto packet_too_small;

	if (req->exceptions_len) {
		root = nla_nest_start(pkt, NLA_F_NESTED | ATTR_EXCEPTIONS);
		if (!root) {
			error = -NLE_NOMEM;
			goto packet_too_small;
		}

		for (i = 0; i < req->exceptions_len; i++) {
			if (req->exceptions[i] > 0xFFFFu)
				goto out_of_range;
			error = nla_put_u16(pkt, 1, req->exceptions[i]);
			if (error)
				goto packet_too_small;
		}

		nla_nest_end(pkt, root);
	}

	return 0;

packet_too_small:
	pr_err("Packet too small.");
	return error;
out_of_range:
	pr_err("Exception %lu is out of range (0-%u)", req->exceptions[i], 0xFFFFu);
	return -EINVAL;
}
