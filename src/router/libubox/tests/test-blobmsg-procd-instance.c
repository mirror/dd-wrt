#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <libgen.h>

#include "blobmsg.h"
#include "utils.h"

enum {
	INSTANCE_ATTR_COMMAND,
	INSTANCE_ATTR_ENV,
	INSTANCE_ATTR_DATA,
	INSTANCE_ATTR_NETDEV,
	INSTANCE_ATTR_FILE,
	INSTANCE_ATTR_TRIGGER,
	INSTANCE_ATTR_RESPAWN,
	INSTANCE_ATTR_NICE,
	INSTANCE_ATTR_LIMITS,
	INSTANCE_ATTR_WATCH,
	INSTANCE_ATTR_ERROR,
	INSTANCE_ATTR_USER,
	INSTANCE_ATTR_GROUP,
	INSTANCE_ATTR_STDOUT,
	INSTANCE_ATTR_STDERR,
	INSTANCE_ATTR_NO_NEW_PRIVS,
	INSTANCE_ATTR_JAIL,
	INSTANCE_ATTR_TRACE,
	INSTANCE_ATTR_SECCOMP,
	INSTANCE_ATTR_PIDFILE,
	INSTANCE_ATTR_RELOADSIG,
	INSTANCE_ATTR_TERMTIMEOUT,
	INSTANCE_ATTR_FACILITY,
	__INSTANCE_ATTR_MAX
};

static const struct blobmsg_policy instance_attr[__INSTANCE_ATTR_MAX] = {
	[INSTANCE_ATTR_COMMAND] = { "command", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_ENV] = { "env", BLOBMSG_TYPE_TABLE },
	[INSTANCE_ATTR_DATA] = { "data", BLOBMSG_TYPE_TABLE },
	[INSTANCE_ATTR_NETDEV] = { "netdev", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_FILE] = { "file", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_TRIGGER] = { "triggers", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_RESPAWN] = { "respawn", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_NICE] = { "nice", BLOBMSG_TYPE_INT32 },
	[INSTANCE_ATTR_LIMITS] = { "limits", BLOBMSG_TYPE_TABLE },
	[INSTANCE_ATTR_WATCH] = { "watch", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_ERROR] = { "error", BLOBMSG_TYPE_ARRAY },
	[INSTANCE_ATTR_USER] = { "user", BLOBMSG_TYPE_STRING },
	[INSTANCE_ATTR_GROUP] = { "group", BLOBMSG_TYPE_STRING },
	[INSTANCE_ATTR_STDOUT] = { "stdout", BLOBMSG_TYPE_BOOL },
	[INSTANCE_ATTR_STDERR] = { "stderr", BLOBMSG_TYPE_BOOL },
	[INSTANCE_ATTR_NO_NEW_PRIVS] = { "no_new_privs", BLOBMSG_TYPE_BOOL },
	[INSTANCE_ATTR_JAIL] = { "jail", BLOBMSG_TYPE_TABLE },
	[INSTANCE_ATTR_TRACE] = { "trace", BLOBMSG_TYPE_BOOL },
	[INSTANCE_ATTR_SECCOMP] = { "seccomp", BLOBMSG_TYPE_STRING },
	[INSTANCE_ATTR_PIDFILE] = { "pidfile", BLOBMSG_TYPE_STRING },
	[INSTANCE_ATTR_RELOADSIG] = { "reload_signal", BLOBMSG_TYPE_INT32 },
	[INSTANCE_ATTR_TERMTIMEOUT] = { "term_timeout", BLOBMSG_TYPE_INT32 },
	[INSTANCE_ATTR_FACILITY] = { "facility", BLOBMSG_TYPE_STRING },
};

static void test_blobmsg_procd_instance(const char *filename)
{
#define BUF_LEN 2048
	int r = 0;
	size_t len = 0;
	FILE *fd = NULL;
	char *buf = NULL;
	struct blob_attr *tb[__INSTANCE_ATTR_MAX];
	const char *fname = basename((char *) filename);

	fd = fopen(filename, "r");
	if (!fd) {
		fprintf(stderr, "unable to open %s\n", fname);
		return;
	}

	buf = malloc(BUF_LEN+1);
	if (!buf)
		return;

	len = fread(buf, 1, BUF_LEN, fd);
	fclose(fd);

	r = blobmsg_parse(instance_attr, __INSTANCE_ATTR_MAX, tb, buf, len);
	if (r)
		goto out;

	if (!tb[INSTANCE_ATTR_COMMAND] || !tb[INSTANCE_ATTR_NICE] || !tb[INSTANCE_ATTR_STDERR])
		goto out;

	if (!blobmsg_check_attr_list(tb[INSTANCE_ATTR_COMMAND], BLOBMSG_TYPE_STRING))
		goto out;

	if (blobmsg_get_u32(tb[INSTANCE_ATTR_NICE]) != 19)
		goto out;

	if (!blobmsg_get_bool(tb[INSTANCE_ATTR_STDERR]))
		goto out;

	fprintf(stderr, "%s: OK\n", fname);
out:
	free(buf);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <blobmsg.bin>\n", argv[0]);
		return 3;
	}

	test_blobmsg_procd_instance(argv[1]);

	return 0;
}
