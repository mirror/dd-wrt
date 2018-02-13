#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/rhashtable.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_flow_table.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>

static DEFINE_SPINLOCK(flow_offload_hw_pending_list_lock);
static LIST_HEAD(flow_offload_hw_pending_list);

static DEFINE_MUTEX(nf_flow_offload_hw_mutex);

struct flow_offload_hw {
	struct list_head	list;
	enum flow_offload_type	type;
	struct flow_offload	*flow;
	struct nf_conn		*ct;
	possible_net_t		flow_hw_net;
};

static int do_flow_offload_hw(struct net *net, struct flow_offload *flow,
			      int type)
{
	struct net_device *indev;
	int ret, ifindex;

	ifindex = flow->tuplehash[FLOW_OFFLOAD_DIR_ORIGINAL].tuple.iifidx;
	indev = dev_get_by_index(net, ifindex);
	if (WARN_ON(!indev))
		return 0;

	mutex_lock(&nf_flow_offload_hw_mutex);
	ret = indev->netdev_ops->ndo_flow_offload(type, flow);
	mutex_unlock(&nf_flow_offload_hw_mutex);

	dev_put(indev);

	return ret;
}

static void flow_offload_hw_work_add(struct flow_offload_hw *offload)
{
	struct net *net;
	int ret;

	if (nf_ct_is_dying(offload->ct))
		return;

	net = read_pnet(&offload->flow_hw_net);
	ret = do_flow_offload_hw(net, offload->flow, FLOW_OFFLOAD_ADD);
	if (ret >= 0)
		offload->flow->flags |= FLOW_OFFLOAD_HW;
}

static void flow_offload_hw_work_del(struct flow_offload_hw *offload)
{
	struct net *net = read_pnet(&offload->flow_hw_net);

	do_flow_offload_hw(net, offload->flow, FLOW_OFFLOAD_DEL);
}

static void flow_offload_hw_work(struct work_struct *work)
{
	struct flow_offload_hw *offload, *next;
	LIST_HEAD(hw_offload_pending);

	spin_lock_bh(&flow_offload_hw_pending_list_lock);
	list_replace_init(&flow_offload_hw_pending_list, &hw_offload_pending);
	spin_unlock_bh(&flow_offload_hw_pending_list_lock);

	list_for_each_entry_safe(offload, next, &hw_offload_pending, list) {
		switch (offload->type) {
		case FLOW_OFFLOAD_ADD:
			flow_offload_hw_work_add(offload);
			break;
		case FLOW_OFFLOAD_DEL:
			flow_offload_hw_work_del(offload);
			break;
		}
		if (offload->ct)
			nf_conntrack_put(&offload->ct->ct_general);
		list_del(&offload->list);
		kfree(offload);
	}
}

static void flow_offload_queue_work(struct flow_offload_hw *offload)
{
	spin_lock_bh(&flow_offload_hw_pending_list_lock);
	list_add_tail(&offload->list, &flow_offload_hw_pending_list);
	spin_unlock_bh(&flow_offload_hw_pending_list_lock);

	schedule_work(&nf_flow_offload_hw_work);
}

static void flow_offload_hw_add(struct net *net, struct flow_offload *flow,
				struct nf_conn *ct)
{
	struct flow_offload_hw *offload;

	offload = kmalloc(sizeof(struct flow_offload_hw), GFP_ATOMIC);
	if (!offload)
		return;

	nf_conntrack_get(&ct->ct_general);
	offload->type = FLOW_OFFLOAD_ADD;
	offload->ct = ct;
	offload->flow = flow;
	write_pnet(&offload->flow_hw_net, net);

	flow_offload_queue_work(offload);
}

static void flow_offload_hw_del(struct net *net, struct flow_offload *flow)
{
	struct flow_offload_hw *offload;

	offload = kmalloc(sizeof(struct flow_offload_hw), GFP_ATOMIC);
	if (!offload)
		return;

	offload->type = FLOW_OFFLOAD_DEL;
	offload->ct = NULL;
	offload->flow = flow;
	write_pnet(&offload->flow_hw_net, net);

	flow_offload_queue_work(offload);
}

static const struct nf_flow_table_hw flow_offload_hw = {
	.add	= flow_offload_hw_add,
	.del	= flow_offload_hw_del,
	.owner	= THIS_MODULE,
};

static int __init nf_flow_table_hw_module_init(void)
{
	INIT_WORK(&nf_flow_offload_hw_work, flow_offload_hw_work);
	nf_flow_table_hw_register(&flow_offload_hw);

	return 0;
}

static void __exit nf_flow_table_hw_module_exit(void)
{
	struct flow_offload_hw *offload, *next;
	LIST_HEAD(hw_offload_pending);

	nf_flow_table_hw_unregister(&flow_offload_hw);
	cancel_work_sync(&nf_flow_offload_hw_work);

	list_for_each_entry_safe(offload, next, &hw_offload_pending, list) {
		if (offload->ct)
			nf_conntrack_put(&offload->ct->ct_general);
		list_del(&offload->list);
		kfree(offload);
	}
}

module_init(nf_flow_table_hw_module_init);
module_exit(nf_flow_table_hw_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pablo Neira Ayuso <pablo@netfilter.org>");
MODULE_ALIAS("nf-flow-table-hw");
