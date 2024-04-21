#ifndef __BR_OFFLOAD_H
#define __BR_OFFLOAD_H

bool br_offload_input(struct net_bridge_port *p, struct sk_buff *skb);
void br_offload_output(struct sk_buff *skb);
void br_offload_port_state(struct net_bridge_port *p);
void br_offload_fdb_update(const struct net_bridge_fdb_entry *fdb);
int br_offload_init(void);
void br_offload_fini(void);
int br_offload_set_cache_size(struct net_bridge *br, unsigned long val,struct netlink_ext_ack *extack);
int br_offload_set_cache_reserved(struct net_bridge *br, unsigned long val,struct netlink_ext_ack *extack);

static inline void br_offload_skb_disable(struct sk_buff *skb)
{
	struct br_input_skb_cb *cb = (struct br_input_skb_cb *)skb->cb;

	if (cb->offload)
		cb->offload = 0;
}

#endif
