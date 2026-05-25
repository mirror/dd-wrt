// SPDX-License-Identifier: GPL-2.0-only
/*
 * fwnode helpers for the MDIO (Ethernet PHY) API
 *
 * This file provides helper functions for extracting PHY device information
 * out of the fwnode and using it to populate an mii_bus.
 */

#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/dev_printk.h>
#include <linux/fwnode_mdio.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/pse-pd/pse.h>

MODULE_AUTHOR("Calvin Johnson <calvin.johnson@oss.nxp.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FWNODE MDIO bus (Ethernet PHY) accessors");

static struct pse_control *
fwnode_find_pse_control(struct fwnode_handle *fwnode)
{
	struct pse_control *psec;
	struct device_node *np;

	if (!IS_ENABLED(CONFIG_PSE_CONTROLLER))
		return NULL;

	np = to_of_node(fwnode);
	if (!np)
		return NULL;

	psec = of_pse_control_get(np);
	if (PTR_ERR(psec) == -ENOENT)
		return NULL;

	return psec;
}

static struct mii_timestamper *
fwnode_find_mii_timestamper(struct fwnode_handle *fwnode)
{
	struct mii_timestamper *mii_ts;
	struct of_phandle_args arg;
	int err;

	if (is_acpi_node(fwnode))
		return NULL;

	err = of_parse_phandle_with_fixed_args(to_of_node(fwnode),
					       "timestamper", 1, 0, &arg);
	if (err == -ENOENT)
		return NULL;
	else if (err)
		return ERR_PTR(err);

	if (arg.args_count != 1) {
		mii_ts = ERR_PTR(-EINVAL);
		goto put_node;
	}

	mii_ts = register_mii_timestamper(arg.np, arg.args[0]);

put_node:
	of_node_put(arg.np);
	return mii_ts;
}

int fwnode_mdiobus_phy_device_register(struct mii_bus *mdio,
				       struct phy_device *phy,
				       struct fwnode_handle *child, u32 addr)
{
	int rc;

	rc = fwnode_irq_get(child, 0);
	/* Don't wait forever if the IRQ provider doesn't become available,
	 * just fall back to poll mode
	 */
	if (rc == -EPROBE_DEFER)
		rc = driver_deferred_probe_check_state(&phy->mdio.dev);
	if (rc == -EPROBE_DEFER)
		return rc;

	if (rc > 0) {
		phy->irq = rc;
		mdio->irq[addr] = rc;
	} else {
		phy->irq = mdio->irq[addr];
	}

	if (fwnode_property_read_bool(child, "broken-turn-around"))
		mdio->phy_ignore_ta_mask |= BIT_ULL(addr);

	fwnode_property_read_u32(child, "reset-assert-us",
				 &phy->mdio.reset_assert_delay);
	fwnode_property_read_u32(child, "reset-deassert-us",
				 &phy->mdio.reset_deassert_delay);

	/* Associate the fwnode with the device structure so it
	 * can be looked up later
	 */
	fwnode_handle_get(child);
	device_set_node(&phy->mdio.dev, child);

	/* All data is now stored in the phy struct;
	 * register it
	 */
	rc = phy_device_register(phy);
	if (rc) {
		device_set_node(&phy->mdio.dev, NULL);
		fwnode_handle_put(child);
		return rc;
	}

	dev_dbg(&mdio->dev, "registered phy fwnode %pfw at address %i\n",
		child, addr);
	return 0;
}
EXPORT_SYMBOL(fwnode_mdiobus_phy_device_register);

int fwnode_mdiobus_register_phy(struct mii_bus *bus,
				struct fwnode_handle *child, u32 addr)
{
	struct mii_timestamper *mii_ts = NULL;
	struct pse_control *psec = NULL;
	struct phy_device *phy;
	struct phy_c45_device_ids c45_ids;
	bool is_c45;
	u32 phy_id;
	int rc, retries = 0;

	psec = fwnode_find_pse_control(child);
	if (IS_ERR(psec))
		return PTR_ERR(psec);

	mii_ts = fwnode_find_mii_timestamper(child);
	if (IS_ERR(mii_ts)) {
		rc = PTR_ERR(mii_ts);
		goto clean_pse;
	}

	is_c45 = fwnode_device_is_compatible(child, "ethernet-phy-ieee802.3-c45");

	memset(&c45_ids, 0xff, sizeof(c45_ids));
	c45_ids.devices_in_package = BIT(MDIO_MMD_PMAPMD);
	c45_ids.mmds_present = BIT(MDIO_MMD_PMAPMD);

	if (!fwnode_get_phy_id(child, &phy_id)) {
		if (is_c45) {
			/*
			 * DT supplied a Clause 45 PHY ID in the form
			 * "ethernet-phy-idXXXX.XXXX". Populate synthetic
			 * c45_ids so normal C45 driver matching and module
			 * autoload work.
			 */
			c45_ids.device_ids[MDIO_MMD_PMAPMD] = phy_id;
			phy = phy_device_create(bus, addr, 0, true, &c45_ids);
		} else {
			phy = phy_device_create(bus, addr, phy_id, false, NULL);
		}
	} else {
 		phy = get_phy_device(bus, addr, is_c45);

		if (IS_ERR(phy) && is_c45 && PTR_ERR(phy) == -ENODEV) {
			/*
			 * Some DT-declared Clause 45 PHYs can take time after
			 * power-on/reset before they answer ID reads.
			 * Retry for up to 5 seconds before giving up.
			 */
			for (retries = 1; retries <= 50; retries++) {
				msleep(100);
				phy = get_phy_device(bus, addr, true);
				if (!IS_ERR(phy) || PTR_ERR(phy) != -ENODEV)
					break;
			}
		}
	}

	if (IS_ERR(phy)) {
		rc = PTR_ERR(phy);
		goto clean_mii_ts;
	}

	if (is_acpi_node(child)) {
		phy->irq = bus->irq[addr];

		/* Associate the fwnode with the device structure so it
		 * can be looked up later.
		 */
		phy->mdio.dev.fwnode = fwnode_handle_get(child);

		/* All data is now stored in the phy struct, so register it */
		rc = phy_device_register(phy);
		if (rc) {
			phy->mdio.dev.fwnode = NULL;
			fwnode_handle_put(child);
			goto clean_phy;
		}
	} else if (is_of_node(child)) {
		rc = fwnode_mdiobus_phy_device_register(bus, phy, child, addr);
		if (rc)
			goto clean_phy;
	}

	phy->psec = psec;

	/* phy->mii_ts may already be defined by the PHY driver. A
	 * mii_timestamper probed via the device tree will still have
	 * precedence.
	 */
	if (mii_ts)
		phy->mii_ts = mii_ts;

	if (retries)
		dev_info(&bus->dev,
			 "C45 PHY at address %u became ready after %d ms\n",
			 addr, retries * 100);

	return 0;

clean_phy:
	phy_device_free(phy);
clean_mii_ts:
	unregister_mii_timestamper(mii_ts);
clean_pse:
	pse_control_put(psec);

	return rc;
}
EXPORT_SYMBOL(fwnode_mdiobus_register_phy);
