// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/mutex.h>
#include <linux/property.h>
#include <linux/phylink.h>
#include <linux/pcs/pcs.h>
#include <linux/pcs/pcs-provider.h>

MODULE_DESCRIPTION("PCS library");
MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_LICENSE("GPL");

struct fwnode_pcs_provider {
	struct list_head link;

	struct fwnode_handle *fwnode;
	struct phylink_pcs *(*get)(struct fwnode_reference_args *pcsspec,
				   void *data);

	void *data;
};

static LIST_HEAD(fwnode_pcs_providers);
static DEFINE_MUTEX(fwnode_pcs_mutex);
static BLOCKING_NOTIFIER_HEAD(fwnode_pcs_notify_list);

int register_fwnode_pcs_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&fwnode_pcs_notify_list, nb);
}
EXPORT_SYMBOL_GPL(register_fwnode_pcs_notifier);

struct phylink_pcs *fwnode_pcs_simple_get(struct fwnode_reference_args *pcsspec,
					  void *data)
{
	return data;
}
EXPORT_SYMBOL_GPL(fwnode_pcs_simple_get);

int fwnode_pcs_add_provider(struct fwnode_handle *fwnode,
			    struct phylink_pcs *(*get)(struct fwnode_reference_args *pcsspec,
						       void *data),
			    void *data)
{
	struct fwnode_pcs_provider *pp;

	if (!fwnode)
		return 0;

	pp = kzalloc(sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	pp->fwnode = fwnode_handle_get(fwnode);
	pp->data = data;
	pp->get = get;

	mutex_lock(&fwnode_pcs_mutex);
	list_add(&pp->link, &fwnode_pcs_providers);
	mutex_unlock(&fwnode_pcs_mutex);
	pr_debug("Added pcs provider from %pfwf\n", fwnode);

	fwnode_dev_initialized(fwnode, true);

	blocking_notifier_call_chain(&fwnode_pcs_notify_list,
				     FWNODE_PCS_PROVIDER_ADD,
				     fwnode);

	return 0;
}
EXPORT_SYMBOL_GPL(fwnode_pcs_add_provider);

void fwnode_pcs_del_provider(struct fwnode_handle *fwnode)
{
	struct fwnode_pcs_provider *pp;

	if (!fwnode)
		return;

	mutex_lock(&fwnode_pcs_mutex);
	list_for_each_entry(pp, &fwnode_pcs_providers, link) {
		if (pp->fwnode == fwnode) {
			list_del(&pp->link);
			fwnode_dev_initialized(pp->fwnode, false);
			fwnode_handle_put(pp->fwnode);
			kfree(pp);
			break;
		}
	}
	mutex_unlock(&fwnode_pcs_mutex);
}
EXPORT_SYMBOL_GPL(fwnode_pcs_del_provider);

static int fwnode_parse_pcsspec(const struct fwnode_handle *fwnode, int index,
				const char *name,
				struct fwnode_reference_args *out_args)
{
	int ret = -ENOENT;

	if (!fwnode)
		return -ENOENT;

	if (name)
		index = fwnode_property_match_string(fwnode, "pcs-names",
						     name);

	ret = fwnode_property_get_reference_args(fwnode, "pcs-handle",
						 "#pcs-cells",
						 -1, index, out_args);
	if (ret || (name && index < 0))
		return ret;

	return 0;
}

static struct phylink_pcs *
fwnode_pcs_get_from_pcsspec(struct fwnode_reference_args *pcsspec)
{
	struct fwnode_pcs_provider *provider;
	struct phylink_pcs *pcs = ERR_PTR(-EPROBE_DEFER);

	if (!pcsspec)
		return ERR_PTR(-EINVAL);

	mutex_lock(&fwnode_pcs_mutex);
	list_for_each_entry(provider, &fwnode_pcs_providers, link) {
		if (provider->fwnode == pcsspec->fwnode) {
			pcs = provider->get(pcsspec, provider->data);
			if (!IS_ERR(pcs))
				break;
		}
	}
	mutex_unlock(&fwnode_pcs_mutex);

	return pcs;
}

static struct phylink_pcs *__fwnode_pcs_get(struct fwnode_handle *fwnode,
					    int index, const char *con_id)
{
	struct fwnode_reference_args pcsspec;
	struct phylink_pcs *pcs;
	int ret;

	ret = fwnode_parse_pcsspec(fwnode, index, con_id, &pcsspec);
	if (ret)
		return ERR_PTR(ret);

	pcs = fwnode_pcs_get_from_pcsspec(&pcsspec);
	fwnode_handle_put(pcsspec.fwnode);

	return pcs;
}

struct phylink_pcs *fwnode_pcs_get(struct fwnode_handle *fwnode, int index)
{
	return __fwnode_pcs_get(fwnode, index, NULL);
}
EXPORT_SYMBOL_GPL(fwnode_pcs_get);

struct phylink_pcs *
fwnode_phylink_pcs_get_from_fwnode(struct fwnode_handle *fwnode,
				   struct fwnode_handle *pcs_fwnode)
{
	struct fwnode_reference_args pcsspec;
	int i = 0;
	int ret;

	while (true) {
		ret = fwnode_parse_pcsspec(fwnode, i, NULL, &pcsspec);
		if (ret)
			break;

		if (pcsspec.fwnode == pcs_fwnode)
			break;

		i++;
	}

	return fwnode_pcs_get(fwnode, i);
}
EXPORT_SYMBOL_GPL(fwnode_phylink_pcs_get_from_fwnode);

static int fwnode_phylink_pcs_count(struct fwnode_handle *fwnode,
				    unsigned int *num_pcs)
{
	struct fwnode_reference_args out_args;
	int index = 0;
	int ret;

	while (true) {
		ret = fwnode_property_get_reference_args(fwnode, "pcs-handle",
							 "#pcs-cells",
							 -1, index, &out_args);
		/* We expect to reach an -ENOENT error while counting */
		if (ret)
			break;

		fwnode_handle_put(out_args.fwnode);
		index++;
	}

	/* Update num_pcs with parsed PCS */
	*num_pcs = index;

	/* Return error if we didn't found any PCS */
	return index > 0 ? 0 : -ENOENT;
}

int fwnode_phylink_pcs_parse(struct fwnode_handle *fwnode,
			     struct phylink_pcs **available_pcs,
			     unsigned int *num_pcs)
{
	int i;

	if (!fwnode_property_present(fwnode, "pcs-handle"))
		return -ENODEV;

	/* With available_pcs NULL, only count the PCS */
	if (!available_pcs)
		return fwnode_phylink_pcs_count(fwnode, num_pcs);

	for (i = 0; i < *num_pcs; i++) {
		struct phylink_pcs *pcs;

		pcs = fwnode_pcs_get(fwnode, i);
		if (IS_ERR(pcs))
			return PTR_ERR(pcs);

		available_pcs[i] = pcs;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(fwnode_phylink_pcs_parse);
