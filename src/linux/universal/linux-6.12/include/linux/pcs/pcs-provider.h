/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __LINUX_PCS_PROVIDER_H
#define __LINUX_PCS_PROVIDER_H

/**
 * fwnode_pcs_simple_get - Simple xlate function to retrieve PCS
 * @pcsspec: reference arguments
 * @data: Context data (assumed assigned to the single PCS)
 *
 * Returns the PCS. (pointed by data)
 */
struct phylink_pcs *fwnode_pcs_simple_get(struct fwnode_reference_args *pcsspec,
					  void *data);

/**
 * fwnode_pcs_add_provider - Registers a new PCS provider
 * @np: Firmware node
 * @get: xlate function to retrieve the PCS
 * @data: Context data
 *
 * Register and add a new PCS to the global providers list
 * for the firmware node. A function to get the PCS from
 * firmware node with the use fwnode reference arguments.
 * To the get function is also passed the interface type
 * requested for the PHY. PCS driver will use the passed
 * interface to understand if the PCS can support it or not.
 *
 * Returns 0 on success or -ENOMEM on allocation failure.
 */
int fwnode_pcs_add_provider(struct fwnode_handle *fwnode,
			    struct phylink_pcs *(*get)(struct fwnode_reference_args *pcsspec,
						       void *data),
			    void *data);

/**
 * fwnode_pcs_del_provider - Removes a PCS provider
 * @fwnode: Firmware node
 */
void fwnode_pcs_del_provider(struct fwnode_handle *fwnode);

#endif /* __LINUX_PCS_PROVIDER_H */
