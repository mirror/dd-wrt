/*
 * Copyright (c) 2021 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>
#include <dirent.h>

#include <netlink.h>
#include <owner.h>

static char *pid2name(pid_t pid)
{
	char procname[256], *prog;
	FILE *fp;
	int ret;

	ret = snprintf(procname, sizeof(procname), "/proc/%lu/stat", (unsigned long)pid);
	if (ret < 0 || ret > (int)sizeof(procname))
		return NULL;

	fp = fopen(procname, "r");
	if (!fp)
		return NULL;

	ret = fscanf(fp, "%*u (%m[^)]", &prog);

	fclose(fp);

	if (ret == 1)
		return prog;

	return NULL;
}

static char *portid2name(pid_t pid, uint32_t portid, unsigned long inode)
{
	const struct dirent *ent;
	char procname[256];
	DIR *dir;
	int ret;

	ret = snprintf(procname, sizeof(procname), "/proc/%lu/fd/", (unsigned long)pid);
	if (ret < 0 || ret >= (int)sizeof(procname))
		return NULL;

	dir = opendir(procname);
	if (!dir)
		return NULL;

	for (;;) {
		unsigned long ino;
		char tmp[128];
		ssize_t rl;

		ent = readdir(dir);
		if (!ent)
			break;

		if (ent->d_type != DT_LNK)
			continue;

		ret = snprintf(procname, sizeof(procname), "/proc/%d/fd/%s",
			       pid, ent->d_name);
		if (ret < 0 || ret >= (int)sizeof(procname))
			continue;

		rl = readlink(procname, tmp, sizeof(tmp));
		if (rl <= 0 || rl >= (ssize_t)sizeof(tmp))
			continue;

		tmp[rl] = 0;

		ret = sscanf(tmp, "socket:[%lu]", &ino);
		if (ret == 1 && ino == inode) {
			closedir(dir);
			return pid2name(pid);
		}
	}

	closedir(dir);
	return NULL;
}

static char *name_by_portid(uint32_t portid, unsigned long inode)
{
	const struct dirent *ent;
	char *prog;
	DIR *dir;

	/* Many netlink users use their process ID to allocate the first port id. */
	prog = portid2name(portid, portid, inode);
	if (prog)
		return prog;

	/* no luck, search harder. */
	dir = opendir("/proc");
	if (!dir)
		return NULL;

	for (;;) {
		unsigned long pid;
		char *end;

		ent = readdir(dir);
		if (!ent)
			break;

		if (ent->d_type != DT_DIR)
			continue;

		pid = strtoul(ent->d_name, &end, 10);
		if (pid <= 1 || *end)
			continue;

		if (pid == portid) /* already tried */
			continue;

		prog = portid2name(pid, portid, inode);
		if (prog)
			break;
	}

	closedir(dir);
	return prog;
}

char *get_progname(uint32_t portid)
{
	FILE *fp = fopen("/proc/net/netlink", "r");
	uint32_t portid_check;
	unsigned long inode;
	int ret, prot;

	if (!fp)
		return NULL;

	for (;;) {
		char line[256];

		if (!fgets(line, sizeof(line), fp))
			break;

		ret = sscanf(line, "%*x %d %u %*x %*d %*d %*x %*d %*u %lu\n",
			     &prot, &portid_check, &inode);

		if (ret == EOF)
			break;

		if (ret == 3 && portid_check == portid && prot == NETLINK_NETFILTER) {
			static uint32_t last_portid;
			static uint32_t last_inode;
			static char *last_program;
			char *prog;

			fclose(fp);

			if (last_portid == portid && last_inode == inode)
				return last_program;

			prog = name_by_portid(portid, inode);

			free(last_program);
			last_program = prog;
			last_portid = portid;
			last_inode = inode;
			return prog;
		}
	}

	fclose(fp);
	return NULL;
}
