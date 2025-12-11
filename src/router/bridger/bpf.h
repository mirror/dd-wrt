// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_BPF_H
#define __BRIDGER_BPF_H

extern int bridger_bpf_prog_fd;
extern int bridger_bpf_tx_prog_fd;

int bridger_bpf_init(void);
void bridger_bpf_dev_policy_set(struct device *dev);
void bridger_bpf_flow_upload(struct bridger_flow *flow);
void bridger_bpf_flow_update(struct bridger_flow *flow);
void bridger_bpf_flow_delete(struct bridger_flow *flow);

#endif
