// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_NL_H
#define __BRIDGER_NL_H

int bridger_nl_init(void);

int bridger_nl_device_attach(struct device *dev, bool tx);
void bridger_nl_device_detach(struct device *dev, bool tx);

int bridger_nl_flow_offload_add(struct bridger_flow *flow);
void bridger_nl_flow_offload_update(struct bridger_flow *flow);
void bridger_nl_flow_offload_del(struct bridger_flow *flow);

int bridger_nl_fdb_refresh(struct fdb_entry *f);

#endif
