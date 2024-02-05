// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Oracle, Inc.
 * All Rights Reserved.
 */

#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "io.h"
#include "libfrog/crc32c.h"
#include "libfrog/crc32cselftest.h"

static int
crc32cselftest_f(
	int		argc,
	char		**argv)
{
	return crc32c_test(0) != 0;
}

static const cmdinfo_t	crc32cselftest_cmd = {
	.name		= "crc32cselftest",
	.cfunc		= crc32cselftest_f,
	.argmin		= 0,
	.argmax		= 0,
	.canpush	= 0,
	.flags		= CMD_FLAG_ONESHOT | CMD_FLAG_FOREIGN_OK |
			  CMD_NOFILE_OK | CMD_NOMAP_OK,
	.oneline	= N_("self test of crc32c implementation"),
};

void
crc32cselftest_init(void)
{
	add_command(&crc32cselftest_cmd);
}
