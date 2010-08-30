#ifndef RULES_COMMAND_H
#define RULES_COMMAND_H 1

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../common.h"
#include "../uevent.h"

#include "execution_ctx.h"

#define FLAG_MASK_SLOW     0x000000ff /* Flags notifying of slow execution. */

#define FLAG_NONE          0x00000000 /* No flag. */
#define FLAG_FORKS	       0x00000001 /* Our code (= not libc's) forks. */
#define FLAG_EXECS         0x00000002 /* We execute external application. */
#define FLAG_SLOW          0x00000004 /* This rule might be slow. */

#define RULES_COMMAND_F(command) \
	int command(struct execution_ctx_t *ctx, int argc, char **argv)

typedef int(*command_f)(struct execution_ctx_t *, int, char **);

struct command_def_t {
	char *name;
	int flags;

	int minarity; /* Must be >= 0 */
	int maxarity; /* Must be >= minarity, or -1 for infinity. */

	command_f command;
};

RULES_COMMAND_F(cmd_print);
RULES_COMMAND_F(cmd_printdebug);
RULES_COMMAND_F(cmd_setenv);
RULES_COMMAND_F(cmd_remove);
RULES_COMMAND_F(cmd_chown);
RULES_COMMAND_F(cmd_chgrp);
RULES_COMMAND_F(cmd_chmod);
RULES_COMMAND_F(cmd_execcmd);
RULES_COMMAND_F(cmd_run);
RULES_COMMAND_F(cmd_mknod);
RULES_COMMAND_F(cmd_firmware);
RULES_COMMAND_F(cmd_nextevent);
RULES_COMMAND_F(cmd_branchevent);
RULES_COMMAND_F(cmd_branchrule);
RULES_COMMAND_F(cmd_serialize);

#endif /* RULES_COMMAND_H */

