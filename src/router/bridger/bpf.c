// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#include <sys/resource.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <glob.h>
#include <unistd.h>

#include "bridger.h"

static struct bpf_object *obj;
static struct bpf_program *prog, *tx_prog;
static int map_pending = -1;
static int map_offload = -1;
static int map_policy = -1;
static struct uloop_timeout poll_timer;
int bridger_bpf_prog_fd = -1;
int bridger_bpf_tx_prog_fd = -1;

static int bridger_bpf_pr(enum libbpf_print_level level, const char *format,
		     va_list args)
{
	return vfprintf(stderr, format, args);
}

static void bridger_init_env(void)
{
	struct rlimit limit = {
		.rlim_cur = RLIM_INFINITY,
		.rlim_max = RLIM_INFINITY,
	};

	setrlimit(RLIMIT_MEMLOCK, &limit);
}

void bridger_bpf_flow_upload(struct bridger_flow *flow)
{
	struct bridger_offload_flow val;

	if (bpf_map_lookup_elem(map_offload, &flow->key, &val) == 0)
		flow->offload.packets = val.packets;

	bpf_map_update_elem(map_offload, &flow->key, &flow->offload, BPF_ANY);
}

void bridger_bpf_flow_update(struct bridger_flow *flow)
{
	uint64_t prev_packets = flow->offload.packets;

	bpf_map_lookup_elem(map_offload, &flow->key, &flow->offload);

	flow->cur_packets = flow->offload.packets - prev_packets;
	bridger_ewma(&flow->avg_packets, flow->cur_packets);
}

void bridger_bpf_flow_delete(struct bridger_flow *flow)
{
	bpf_map_delete_elem(map_offload, &flow->key);
}

void bridger_bpf_dev_policy_set(struct device *dev)
{
	struct bridger_policy_flow val = {};
	unsigned int ifindex = device_ifindex(dev);
	struct device *rdev = NULL;
	int out_vlan;

	if (dev->redirect_dev)
		rdev = device_get(dev->redirect_dev);

	if (!rdev) {
		bpf_map_delete_elem(map_policy, &ifindex);
		return;
	}

	out_vlan = device_vlan_get_output(rdev, dev->pvid);
	if (out_vlan < 0) {
		bpf_map_delete_elem(map_policy, &ifindex);
		return;
	}

	val.flow.vlan = out_vlan;
	val.flow.target_port = device_ifindex(rdev);
	if (dev->master)
		memcpy(val.bridge_mac, dev->master->addr, ETH_ALEN);
	bpf_map_update_elem(map_policy, &ifindex, &val, BPF_ANY);
}

static void bridger_bpf_poll_pending(struct uloop_timeout *timeout)
{
	struct bridger_flow_key key = {};
	struct bridger_pending_flow val;

	while (bpf_map_get_next_key(map_pending, &key, &key) == 0) {
		if (bpf_map_lookup_elem(map_pending, &key, &val))
			continue;

		bpf_map_delete_elem(map_pending, &key);
		bridger_check_pending_flow(&key, &val);
	}

	uloop_timeout_set(timeout, 1000);
}

static bool
bridger_get_map_fd(int *fd, const char *name)
{
	*fd = bpf_object__find_map_fd_by_name(obj, name);
	if (*fd < 0) {
		D("Could not find map '%s'\n", name);
		return false;
	}

	return true;
}

static int
bridger_create_program(void)
{
	DECLARE_LIBBPF_OPTS(bpf_object_open_opts, opts,
		.pin_root_path = BRIDGER_DATA_PATH,
	);
	int err;

	obj = bpf_object__open_file(BRIDGER_PROG_PATH, &opts);
	err = libbpf_get_error(obj);
	if (err) {
		perror("bpf_object__open_file");
		return -1;
	}

	prog = bpf_object__find_program_by_name(obj, "bridger_input");
	tx_prog = bpf_object__find_program_by_name(obj, "bridger_output");
	if (!prog || !tx_prog) {
		D("Can't find classifier prog\n");
		return -1;
	}

	bpf_program__set_type(tx_prog, BPF_PROG_TYPE_SCHED_CLS);
	bpf_program__set_type(prog, BPF_PROG_TYPE_SCHED_CLS);

	err = bpf_object__load(obj);
	if (err) {
		perror("bpf_object__load");
		return -1;
	}

	libbpf_set_print(NULL);

	unlink(BRIDGER_PIN_PATH);
	err = bpf_program__pin(prog, BRIDGER_PIN_PATH);
	if (err) {
		D("Failed to pin program to %s: %s\n",
		  BRIDGER_PIN_PATH, strerror(-err));
	}

	if (!bridger_get_map_fd(&map_pending, "pending_flows") ||
	    !bridger_get_map_fd(&map_offload, "offload_flows") ||
	    !bridger_get_map_fd(&map_policy, "dev_policy"))
		return -1;

	bridger_bpf_prog_fd = bpf_program__fd(prog);
	bridger_bpf_tx_prog_fd = bpf_program__fd(tx_prog);

	return 0;
}

int bridger_bpf_init(void)
{
	glob_t g;
	int ret;
	int i;

	if (glob(BRIDGER_DATA_PATH "/*", 0, NULL, &g) == 0) {
		for (i = 0; i < g.gl_pathc; i++)
			unlink(g.gl_pathv[i]);
	}

	libbpf_set_print(bridger_bpf_pr);

	bridger_init_env();

	ret = bridger_create_program();
	if (ret)
		return ret;

	poll_timer.cb = bridger_bpf_poll_pending;
	bridger_bpf_poll_pending(&poll_timer);

	return 0;
}
