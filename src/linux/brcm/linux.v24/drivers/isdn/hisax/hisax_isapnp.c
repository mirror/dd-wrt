// FIXME copied
static const struct isapnp_device_id *
isapnp_match_device(const struct isapnp_device_id *ids, struct pci_dev *dev)
{
	DBG(1,"");

	while (ids->card_vendor || ids->card_device) {
		if ((ids->card_vendor == ISAPNP_ANY_ID || ids->card_vendor == dev->bus->vendor) &&
		    (ids->card_device == ISAPNP_ANY_ID || ids->card_device == dev->bus->device) &&
                    (ids->vendor == ISAPNP_ANY_ID || ids->vendor == dev->vendor) &&
                    (ids->function == ISAPNP_ANY_ID || ids->function == dev->device))
			return ids;
		ids++;
	}
	return NULL;
}

/**
 * pci_dev_driver - get the pci_driver of a device
 * @dev: the device to query
 *
 * Returns the appropriate pci_driver structure or %NULL if there is no 
 * registered driver for the device.
 */
struct pci_driver *isapnp_dev_driver(const struct pci_dev *dev)
{
	return dev->driver;
}

static int isapnp_announce_device(struct isapnp_driver *drv, struct pci_dev *dev)
{
	const struct isapnp_device_id *id;
	int ret = 0;

	DBG(1,"");

	if (drv->id_table) {
		id = isapnp_match_device(drv->id_table, dev);
		if (!id) {
			ret = 0;
			goto out;
		}
	} else
		id = NULL;

//	dev_probe_lock();
	if (drv->probe(dev, id) >= 0) {
		dev->driver = (struct pci_driver *) drv;
		ret = 1;
	}
//	dev_probe_unlock();
out:
	return ret;
}

/**
 * FIXME pci_register_driver - register a new pci driver
 * @drv: the driver structure to register
 * 
 * Adds the driver structure to the list of registered drivers
 * Returns the number of pci devices which were claimed by the driver
 * during registration.  The driver remains registered even if the
 * return value is zero.
 */
int isapnp_register_driver(struct isapnp_driver *drv)
{
	struct pci_dev *dev;
	int count = 0;

	DBG(1,"");

	list_add_tail(&drv->node, &isapnp_drivers);
	isapnp_for_each_dev(dev) {
		if (!isapnp_dev_driver(dev))
			count += isapnp_announce_device(drv, dev);
	}
	return count;
}

/**
 * pci_unregister_driver - unregister a pci driver
 * @drv: the driver structure to unregister
 * 
 * Deletes the driver structure from the list of registered PCI drivers,
 * gives it a chance to clean up by calling its remove() function for
 * each device it was responsible for, and marks those devices as
 * driverless.
 */

void isapnp_unregister_driver(struct isapnp_driver *drv)
{
	struct pci_dev *dev;

	DBG(1,"");

	list_del(&drv->node);
	isapnp_for_each_dev(dev) {
		if (dev->driver == (struct pci_driver *) drv) {
			if (drv->remove)
				drv->remove(dev);
			dev->driver = NULL;
		}
	}
}

