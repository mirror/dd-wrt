// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) 2020 Carlos Neira cneirabustos@gmail.com */

#define _GNU_SOURCE
#include <test_progs.h>
#include "test_ns_current_pid_tgid.skel.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "network_helpers.h"

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

static int get_pid_tgid(pid_t *pid, pid_t *tgid,
			struct test_ns_current_pid_tgid__bss *bss)
{
	struct stat st;
	int err;

	*pid = syscall(SYS_gettid);
	*tgid = getpid();

	err = stat("/proc/self/ns/pid", &st);
	if (!ASSERT_OK(err, "stat /proc/self/ns/pid"))
		return err;

	bss->dev = st.st_dev;
	bss->ino = st.st_ino;
	bss->user_pid = 0;
	bss->user_tgid = 0;
	return 0;
}

static int test_current_pid_tgid_tp(void *args)
{
	struct test_ns_current_pid_tgid__bss  *bss;
	struct test_ns_current_pid_tgid *skel;
	int ret = -1, err;
	pid_t tgid, pid;

	skel = test_ns_current_pid_tgid__open();
	if (!ASSERT_OK_PTR(skel, "test_ns_current_pid_tgid__open"))
		return ret;

	bpf_program__set_autoload(skel->progs.tp_handler, true);

	err = test_ns_current_pid_tgid__load(skel);
	if (!ASSERT_OK(err, "test_ns_current_pid_tgid__load"))
		goto cleanup;

	bss = skel->bss;
	if (get_pid_tgid(&pid, &tgid, bss))
		goto cleanup;

	err = test_ns_current_pid_tgid__attach(skel);
	if (!ASSERT_OK(err, "test_ns_current_pid_tgid__attach"))
		goto cleanup;

	/* trigger tracepoint */
	usleep(1);
	if (!ASSERT_EQ(bss->user_pid, pid, "pid"))
		goto cleanup;
	if (!ASSERT_EQ(bss->user_tgid, tgid, "tgid"))
		goto cleanup;
	ret = 0;

cleanup:
	test_ns_current_pid_tgid__destroy(skel);
	return ret;
}

static int test_current_pid_tgid_cgrp(void *args)
{
	struct test_ns_current_pid_tgid__bss *bss;
	struct test_ns_current_pid_tgid *skel;
	int server_fd = -1, ret = -1, err;
	int cgroup_fd = *(int *)args;
	pid_t tgid, pid;

	skel = test_ns_current_pid_tgid__open();
	if (!ASSERT_OK_PTR(skel, "test_ns_current_pid_tgid__open"))
		return ret;

	bpf_program__set_autoload(skel->progs.cgroup_bind4, true);

	err = test_ns_current_pid_tgid__load(skel);
	if (!ASSERT_OK(err, "test_ns_current_pid_tgid__load"))
		goto cleanup;

	bss = skel->bss;
	if (get_pid_tgid(&pid, &tgid, bss))
		goto cleanup;

	skel->links.cgroup_bind4 = bpf_program__attach_cgroup(
		skel->progs.cgroup_bind4, cgroup_fd);
	if (!ASSERT_OK_PTR(skel->links.cgroup_bind4, "bpf_program__attach_cgroup"))
		goto cleanup;

	server_fd = start_server(AF_INET, SOCK_STREAM, NULL, 0, 0);
	if (!ASSERT_GE(server_fd, 0, "start_server"))
		goto cleanup;

	if (!ASSERT_EQ(bss->user_pid, pid, "pid"))
		goto cleanup;
	if (!ASSERT_EQ(bss->user_tgid, tgid, "tgid"))
		goto cleanup;
	ret = 0;

cleanup:
	if (server_fd >= 0)
		close(server_fd);
	test_ns_current_pid_tgid__destroy(skel);
	return ret;
}

static void test_ns_current_pid_tgid_new_ns(int (*fn)(void *), void *arg)
{
	int wstatus;
	pid_t cpid;

	/* Create a process in a new namespace, this process
	 * will be the init process of this new namespace hence will be pid 1.
	 */
	cpid = clone(fn, child_stack + STACK_SIZE,
		     CLONE_NEWPID | SIGCHLD, arg);

	if (!ASSERT_NEQ(cpid, -1, "clone"))
		return;

	if (!ASSERT_NEQ(waitpid(cpid, &wstatus, 0), -1, "waitpid"))
		return;

	if (!ASSERT_OK(WEXITSTATUS(wstatus), "newns_pidtgid"))
		return;
}

static void test_in_netns(int (*fn)(void *), void *arg)
{
	struct nstoken *nstoken = NULL;

	SYS(cleanup, "ip netns add ns_current_pid_tgid");
	SYS(cleanup, "ip -net ns_current_pid_tgid link set dev lo up");

	nstoken = open_netns("ns_current_pid_tgid");
	if (!ASSERT_OK_PTR(nstoken, "open_netns"))
		goto cleanup;

	test_ns_current_pid_tgid_new_ns(fn, arg);

cleanup:
	if (nstoken)
		close_netns(nstoken);
	SYS_NOFAIL("ip netns del ns_current_pid_tgid");
}

/* TODO: use a different tracepoint */
void serial_test_ns_current_pid_tgid(void)
{
	if (test__start_subtest("root_ns_tp"))
		test_current_pid_tgid_tp(NULL);
	if (test__start_subtest("new_ns_tp"))
		test_ns_current_pid_tgid_new_ns(test_current_pid_tgid_tp, NULL);
	if (test__start_subtest("new_ns_cgrp")) {
		int cgroup_fd = -1;

		cgroup_fd = test__join_cgroup("/sock_addr");
		if (ASSERT_GE(cgroup_fd, 0, "join_cgroup")) {
			test_in_netns(test_current_pid_tgid_cgrp, &cgroup_fd);
			close(cgroup_fd);
		}
	}
}
