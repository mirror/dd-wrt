#include "command.h"

struct command_def_t commands[] = {
	/* Rule name      | Flags                  | Arity | Function pointer */
	{"print",           FLAG_NONE,               1,  1,  cmd_print},
	{"print-event",     FLAG_NONE,               0,  1,  cmd_printdebug},
	{"printdebug",      FLAG_NONE,               0,  1,  cmd_printdebug},
	{"setenv",          FLAG_NONE,               2,  2,  cmd_setenv},
	{"remove",          FLAG_NONE,               1,  1,  cmd_remove},
	{"chown",           FLAG_NONE,               2,  2,  cmd_chown},
	{"chgrp",           FLAG_NONE,               2,  2,  cmd_chgrp},
	{"chmod",           FLAG_NONE,               2,  2,  cmd_chmod},
	{"exec",            FLAG_FORKS | FLAG_EXECS, 1, -1,  cmd_execcmd},
	{"run",             FLAG_EXECS,              1,  1,  cmd_run},
	{"mknod",           FLAG_NONE,               2,  2,  cmd_mknod},
	{"makedev",         FLAG_NONE,               2,  2,  cmd_mknod},
	{"load-firmware",   FLAG_SLOW,               1,  1,  cmd_firmware},
	{"serialize",       FLAG_NONE,               1,  1,  cmd_serialize},

	/* Branching rules */
	{"next",            FLAG_NONE, /*old*/       0,  0,  cmd_nextevent},
	{"next-event",      FLAG_NONE, /*new*/       0,  0,  cmd_nextevent},

	{"next_if_failed",  FLAG_NONE, /*old*/       0,  1,  cmd_branchevent},
	{"branch-event",    FLAG_NONE, /*new*/       0,  1,  cmd_branchevent},

	{"break_if_failed", FLAG_NONE, /*old*/       0,  1,  cmd_branchrule},
	{"branch-rule",     FLAG_NONE, /*new*/       0,  1,  cmd_branchrule},

	/* Keywords that alter flags, but do not get executed */
	{"flag-slow",       FLAG_SLOW,               0,  0,  NULL},

	/* Ignored but accepted keywords (for backward compatibility) */
	{"nothrottle",      FLAG_NONE,               0,  0,  NULL},
	{"break",           FLAG_NONE,               0,  0,  NULL},

	/* DO NOT DELETE. */
	{NULL, 0, 0, 0, NULL}
};

/**
 * Function supplementing 'echo > file'
 *
 * @1 File to be written to
 * @2 Data to be written
 * @3 Data size
 *
 * Returns: 0 on success, -1 on failure.
 */
static int echo_to_file(const char *filename, const char *data, size_t size) {
	FILE *fp;
	size_t written;

	fp = fopen(filename, "w");
	if (fp == NULL)
		return -1;
	written = fwrite(data, size, 1, fp);
	fclose(fp);

	return (written == size) ? 0 : -1;
}

/**
 * Function supplementing 'echo >> file'
 *
 * @1 File to be appended to
 * @2 Data to be appended
 * @3 Data size
 *
 * Returns: 0 on success, -1 on failure.
 */
static int append_to_file(const char *filename, const char *data, size_t size) {
	FILE *fp;
	size_t written;

	fp = fopen(filename, "a");
	if (fp == NULL)
		return -1;
	written = fwrite(data, size, 1, fp);
	fclose(fp);

	return (written == size) ? 0 : -1;
}

/**
 * Function supplementing 'mkdir -p'.
 *
 * @1 Path to be mkdir'd
 * @2 Mode of the new dirs
 *
 * Returns: 0 if success, -1 otherwise
 */
static int mkdir_p(char *fullpath, mode_t mode) {
	char *path;
	char *ptr;
	int rv;
	struct stat statbuf;
	
	fullpath = strdup(fullpath);
	path = dirname(fullpath);

	/* Check if it isn't directory already. */
	rv = stat(path, &statbuf);
	if (rv == 0 && S_ISDIR(statbuf.st_mode)) {
		free(fullpath);
		return 0;
	}
	
	/* We don't attempt to create root, obviously. */
	for (ptr = path; *ptr == '/'; ptr++) { }

	ptr = strchr(ptr, '/');
	while (ptr != NULL && *ptr != '\0') {
		*ptr = '\0';

		rv = mkdir(path, mode);
		if (rv != 0 && errno != EEXIST) {
			free(fullpath);
			return -1;
		}

		*ptr='/';
		ptr = strchr(ptr+1, '/');
	}

	rv = mkdir(path, mode);

	free(fullpath);
	return (rv == 0) ? 0 : -1;
}

static int serialize_unix(char *fn, struct uevent_t *uevent) {
	int sock;
	int rv;
	socklen_t addrlen;
	struct sockaddr_un saddr_unix;
	struct stat statbuf;

	/*
	 * Logic compatible with udev.
	 */
	if (fn[0] == '@') {
		/*
		 * Abstract unix socket.
		 */
		strncpy(saddr_unix.sun_path, fn, sizeof(saddr_unix.sun_path));
		saddr_unix.sun_path[0] = '\0';
		addrlen = sizeof(saddr_unix.sun_family) + strlen(saddr_unix.sun_path) + 1;
	} else if (stat(fn, &statbuf) == 0 && S_ISSOCK(statbuf.st_mode)) {
		/*
		 * Existing unix socket.
		 */
		strncpy(saddr_unix.sun_path, fn, sizeof(saddr_unix.sun_path));
		addrlen = sizeof(saddr_unix.sun_family) + strlen(saddr_unix.sun_path) + 1;
	} else {
		/*
		 * No existing unix socket, imply abstract.
		 */
		strncpy(saddr_unix.sun_path+1, fn, sizeof(saddr_unix.sun_path)-1);
		saddr_unix.sun_path[0] = '\0';
		addrlen = sizeof(saddr_unix.sun_family) + strlen(saddr_unix.sun_path);
	}

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
		return -1;

	rv = sendto(sock, uevent->plain, (size_t)uevent->plain_s,
	       0, (struct sockaddr *)&saddr_unix, addrlen);

	if (close(sock))
		return -1;
	return rv;
}

RULES_COMMAND_F(cmd_serialize) {
	char *fn = argv[0];

	if (strcmp("socket:", fn) == 0) {
		return serialize_unix(&fn[strlen("socket:")], ctx->uevent);
	} else {
		return append_to_file(fn, ctx->uevent->plain, ctx->uevent->plain_s);
	}
}

RULES_COMMAND_F(cmd_print) {
	printf("%s\n", argv[0]);
	return 0;
}

/*
 * printdebug
 */
RULES_COMMAND_F(cmd_printdebug) {
	int i;
	
	if (argv[0] != NULL)
		printf("Event #%" PRId64 ": %s\n", ctx->uevent->seqnum, argv[0]);
	else
		printf("Event #%" PRId64 "\n", ctx->uevent->seqnum);

	for (i = 0; i < ctx->uevent->env_vars_c; i++) {
		printf("%15s = %s\n", ctx->uevent->env_vars[i].key, ctx->uevent->env_vars[i].value);
	}
	printf("\n");

	return 0;
}

/*
 * setenv <key> <value>
 */
RULES_COMMAND_F(cmd_setenv) {
	char *buf;
	size_t key_len, value_len;

	/*
	 * Register the environment variable in the uevent structure,
	 * too. This will allow proper cleanup of created env. vars.
	 * in case of single-process execution.
	 */
	key_len = strlen(argv[0]);
	value_len = strlen(argv[1]);

	buf = xmalloc(key_len + value_len + 2);
	memcpy(buf, argv[0], key_len);
	buf[key_len] = '=';
	memcpy(buf + key_len + 1, argv[1], value_len);
	buf[key_len + value_len + 1] = '\0';
	
	uevent_add_env(ctx->uevent, buf);

	free(buf);

	return setenv(argv[0], argv[1], 1);
}

/*
 * remove <file>
 */
RULES_COMMAND_F(cmd_remove) {
	int rv;
	rv = unlink(argv[0]);
	if (rv != 0 && errno == EISDIR)
		rv = rmdir(argv[0]);
	return rv;
}

/*
 * chown <user> <file>
 */
RULES_COMMAND_F(cmd_chown) {
	struct passwd *pwd;
	
	pwd = getpwnam(argv[0]);
	if (pwd == NULL)
		return -1;

	return chown(argv[1], pwd->pw_uid, -1);
}

/*
 * chgrp <group> <file>
 */
RULES_COMMAND_F(cmd_chgrp) {
	struct group *grp;
	
	grp = getgrnam(argv[0]);
	if (grp == NULL)
		return -1;

	return chown(argv[1], -1, grp->gr_gid);
}

/*
 * chmod <mode> <file>
 */
RULES_COMMAND_F(cmd_chmod) {
	return chmod(argv[1], strtoul(argv[0], NULL, 8));
}

/*
 * exec <filename> [<arg1> [<arg2> [...]]]
 */
RULES_COMMAND_F(cmd_execcmd) {
	pid_t p;
	int status;

	p = fork();
	switch (p) {
		case -1:
			return -1;

		case 0:
			execvp(argv[0], argv);
			exit(127);
			break;

		default:
			if (waitpid(p, &status, 0) == -1)
				return -1;
			return WEXITSTATUS(status);
	}

	return -1;
}

/*
 * run <command>
 */
RULES_COMMAND_F(cmd_run) {
	return WEXITSTATUS(system(argv[0]));
}

/*
 * mknod <path> <mode>
 */
RULES_COMMAND_F(cmd_mknod) {
	char *devpath;
	char *minor, *major;
	char *subsystem;
	int rv;
	mode_t devmode;

	major = uevent_getvalue(ctx->uevent, "MAJOR");
	if (major == NULL)
		return -1;

	minor = uevent_getvalue(ctx->uevent, "MINOR");
	if (minor == NULL)
		return -1;

	subsystem = uevent_getvalue(ctx->uevent, "SUBSYSTEM");
	if (subsystem == NULL)
		return -1;

	/* We don't actually use this, it's just a check. */
	devpath = uevent_getvalue(ctx->uevent, "DEVPATH");
	if (devpath == NULL)
		return -1;
	
	devmode = strtoul(argv[1], NULL, 8);
	if (!strcmp(subsystem, "block"))
		devmode |= S_IFBLK;
	else
		devmode |= S_IFCHR;
	
	mkdir_p(argv[0], 0755);
	rv = mknod(argv[0], devmode, makedev(atoi(major), atoi(minor)));
	chmod(argv[0], devmode);

	return rv;
}

/*
 * firmware <firmware directory>
 */
RULES_COMMAND_F(cmd_firmware) {
	char buffer[1024];
	char *devpath;
	char *firmware;
	char firmware_path[PATH_MAX];
	char sysfs_path_loading[PATH_MAX];
	char sysfs_path_data[PATH_MAX];
	int rv;
	FILE *infp, *outfp;
	size_t inlen, outlen;

	devpath = uevent_getvalue(ctx->uevent, "DEVPATH");
	if (devpath == NULL)
		return -1;

	firmware = uevent_getvalue(ctx->uevent, "FIRMWARE");
	if (firmware == NULL)
		return -1;

	if (snprintf(sysfs_path_loading, PATH_MAX, "/sys%s/loading", devpath) >= PATH_MAX)
		return -1;
	if (snprintf(sysfs_path_data, PATH_MAX, "/sys%s/data", devpath) >= PATH_MAX)
		return -1;
	if (snprintf(firmware_path, PATH_MAX, "%s/%s", argv[0], firmware) >= PATH_MAX)
		return -1;

	echo_to_file(sysfs_path_loading, "1\n", 2);

	infp = fopen(firmware_path, "r");
	if (infp == NULL) {
		echo_to_file(sysfs_path_loading, "-1\n", 2);
		return -1;
	}
	outfp = fopen(sysfs_path_data, "w");
	if (outfp == NULL) {
		fclose(infp);
		echo_to_file(sysfs_path_loading, "-1\n", 2);
		return -1;
	}

	rv = 0;
	while ((inlen = fread(buffer, 1, 1024, infp)) > 0) {
		outlen = fwrite(buffer, 1, inlen, outfp);
		if (outlen != inlen) {
			rv = -1;
			break;
		}
	}

	fclose(infp);
	fclose(outfp);

	echo_to_file(sysfs_path_loading, "0\n", 2);

	return rv;
}

/*
 * next-event
 */
RULES_COMMAND_F(cmd_nextevent) {
	ctx->branching = BR_UEVENT;
	return 0;
}

/*
 * branch-event [<success>]
 */
RULES_COMMAND_F(cmd_branchevent) {
	if (argv[0] != NULL && !strcmp(argv[0], "success")) {
		if (ctx->last_rv == 0)
			ctx->branching = BR_UEVENT;
	} else {
		if (ctx->last_rv != 0)
			ctx->branching = BR_UEVENT;
	}
	return 0;
}

/*
 * branch-rule [<success>]
 */
RULES_COMMAND_F(cmd_branchrule) {
	if (argv[0] != NULL && !strcmp(argv[0], "success")) {
		if (ctx->last_rv == 0)
			ctx->branching = BR_RULE;
	} else {
		if (ctx->last_rv != 0)
			ctx->branching = BR_RULE;
	}
	return 0;
}
