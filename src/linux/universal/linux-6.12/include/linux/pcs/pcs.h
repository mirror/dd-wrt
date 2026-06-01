/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __LINUX_PCS_H
#define __LINUX_PCS_H

#include <linux/phylink.h>

enum fwnode_pcs_notify_event {
	FWNODE_PCS_PROVIDER_ADD,
};

#if IS_ENABLED(CONFIG_FWNODE_PCS)
/**
 * register_fwnode_pcs_notifier - Register a notifier block for fwnode
 *				  PCS events
 * @nb: pointer to the notifier block
 *
 * Registers a notifier block to the fwnode_pcs_notify_list blocking
 * notifier chain. This allows phylink instance to subscribe for
 * PCS provider events.
 *
 * Returns 0 or a negative error.
 */
int register_fwnode_pcs_notifier(struct notifier_block *nb);

/**
 * fwnode_pcs_get - Retrieves a PCS from a firmware node
 * @fwnode: firmware node
 * @index: index fwnode PCS handle in firmware node
 *
 * Get a PCS from the firmware node at index.
 *
 * Returns a pointer to the phylink_pcs or a negative
 * error pointer. Can return -EPROBE_DEFER if the PCS is not
 * present in global providers list (either due to driver
 * still needs to be probed or it failed to probe/removed)
 */
struct phylink_pcs *fwnode_pcs_get(struct fwnode_handle *fwnode,
				   int index);

/**
 * fwnode_phylink_pcs_get_from_fwnode - Retrieves the PCS provided
 *					by the firmware node from a
 *					firmware node
 * @fwnode: firmware node
 * @pcs_fwnode: PCS firmware node
 *
 * Parse 'pcs-handle' in 'fwnode' and get the PCS that match
 * 'pcs_fwnode' firmware node.
 *
 * Returns a pointer to the phylink_pcs or a negative
 * error pointer. Can return -EPROBE_DEFER if the PCS is not
 * present in global providers list (either due to driver
 * still needs to be probed or it failed to probe/removed)
 */
struct phylink_pcs *
fwnode_phylink_pcs_get_from_fwnode(struct fwnode_handle *fwnode,
				   struct fwnode_handle *pcs_fwnode);

/**
 * fwnode_phylink_pcs_parse - generic PCS parse for fwnode PCS provider
 * @fwnode: firmware node
 * @available_pcs: pointer to preallocated array of PCS
 * @num_pcs: where to store count of parsed PCS
 *
 * Generic helper function to fill available_pcs array with PCS parsed
 * from a "pcs-handle" fwnode property defined in firmware node up to
 * passed num_pcs.
 *
 * If available_pcs is NULL, num_pcs is updated with the count of the
 * parsed PCS.
 *
 * Returns 0 or a negative error.
 */
int fwnode_phylink_pcs_parse(struct fwnode_handle *fwnode,
			     struct phylink_pcs **available_pcs,
			     unsigned int *num_pcs);
#else
static inline int register_fwnode_pcs_notifier(struct notifier_block *nb)
{
	return -EOPNOTSUPP;
}

static inline struct phylink_pcs *fwnode_pcs_get(struct fwnode_handle *fwnode,
						 int index)
{
	return ERR_PTR(-ENOENT);
}

static inline struct phylink_pcs *
fwnode_phylink_pcs_get_from_fwnode(struct fwnode_handle *fwnode,
				   struct fwnode_handle *pcs_fwnode)
{
	return ERR_PTR(-ENOENT);
}

static inline int fwnode_phylink_pcs_parse(struct fwnode_handle *fwnode,
					   struct phylink_pcs **available_pcs,
					   unsigned int *num_pcs)
{
	return -EOPNOTSUPP;
}
#endif

#endif /* __LINUX_PCS_H */
