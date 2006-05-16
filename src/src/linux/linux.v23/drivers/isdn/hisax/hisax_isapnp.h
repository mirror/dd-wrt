#ifndef __HISAX_ISAPNP_H__
#define __HISAX_ISAPNP_H__

#include <linux/isapnp.h>

#ifdef COMPAT_NEED_ISAPNP_DRIVER
struct isapnp_driver {
	struct list_head node;
	char *name;
	const struct isapnp_device_id *id_table;	/* NULL if wants all devices */
	int  (*probe)  (struct pci_dev *dev, const struct isapnp_device_id *id);	/* New device inserted */
	void (*remove) (struct pci_dev *dev);	/* Device removed (NULL if not a hot-plug capable driver) */
};
#endif
#ifdef __ISAPNP__

int isapnp_register_driver(struct isapnp_driver *drv);
void isapnp_unregister_driver(struct isapnp_driver *drv);

#else

static inline int isapnp_register_driver(struct isapnp_driver *drv) 
{ 
	return 0;
}

static inline void isapnp_unregister_driver(struct isapnp_driver *drv) 
{ 
}

#endif

#endif
