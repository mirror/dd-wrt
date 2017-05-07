/*
 * Core PHY library, taken from phy.c
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/export.h>
#include <linux/phy.h>

const char *phy_speed_to_str(int speed)
{
	switch (speed) {
	case SPEED_10:
		return "10Mbps";
	case SPEED_100:
		return "100Mbps";
	case SPEED_1000:
		return "1Gbps";
	case SPEED_2500:
		return "2.5Gbps";
	case SPEED_10000:
		return "10Gbps";
	case SPEED_UNKNOWN:
		return "Unknown";
	default:
		return "Unsupported (update phy-core.c)";
	}
}
EXPORT_SYMBOL_GPL(phy_speed_to_str);

const char *phy_duplex_to_str(unsigned int duplex)
{
	if (duplex == DUPLEX_HALF)
		return "Half";
	if (duplex == DUPLEX_FULL)
		return "Full";
	if (duplex == DUPLEX_UNKNOWN)
		return "Unknown";
	return "Unsupported (update phy-core.c)";
}
EXPORT_SYMBOL_GPL(phy_duplex_to_str);

/* A mapping of all SUPPORTED settings to speed/duplex.  This table
 * must be grouped by speed and sorted in descending match priority
 * - iow, descending speed. */
static const struct phy_setting settings[] = {
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_10000baseKR_Full_BIT,
	},
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_10000baseKX4_Full_BIT,
	},
	{
		.speed = SPEED_10000,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
	},
	{
		.speed = SPEED_2500,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_2500baseX_Full_BIT,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_1000baseKX_Full_BIT,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
	},
	{
		.speed = SPEED_1000,
		.duplex = DUPLEX_HALF,
		.bit = ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
	},
	{
		.speed = SPEED_100,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_100baseT_Full_BIT,
	},
	{
		.speed = SPEED_100,
		.duplex = DUPLEX_HALF,
		.bit = ETHTOOL_LINK_MODE_100baseT_Half_BIT,
	},
	{
		.speed = SPEED_10,
		.duplex = DUPLEX_FULL,
		.bit = ETHTOOL_LINK_MODE_10baseT_Full_BIT,
	},
	{
		.speed = SPEED_10,
		.duplex = DUPLEX_HALF,
		.bit = ETHTOOL_LINK_MODE_10baseT_Half_BIT,
	},
};

/**
 * phy_lookup_setting - lookup a PHY setting
 * @speed: speed to match
 * @duplex: duplex to match
 * @mask: allowed link modes
 * @maxbit: bit size of link modes
 * @exact: an exact match is required
 *
 * Search the settings array for a setting that matches the speed and
 * duplex, and which is supported.
 *
 * If @exact is unset, either an exact match or %NULL for no match will
 * be returned.
 *
 * If @exact is set, an exact match, the fastest supported setting at
 * or below the specified speed, the slowest supported setting, or if
 * they all fail, %NULL will be returned.
 */
const struct phy_setting *
phy_lookup_setting(int speed, int duplex, const unsigned long *mask,
		   size_t maxbit, bool exact)
{
	const struct phy_setting *p, *match = NULL, *last = NULL;
	int i;

	for (i = 0, p = settings; i < ARRAY_SIZE(settings); i++, p++) {
		if (p->bit < maxbit && test_bit(p->bit, mask)) {
			last = p;
			if (p->speed == speed && p->duplex == duplex) {
				/* Exact match for speed and duplex */
				match = p;
				break;
			} else if (!exact) {
				if (!match && p->speed <= speed)
					/* Candidate */
					match = p;

				if (p->speed < speed)
					break;
			}
		}
	}

	if (!match && !exact)
		match = last;

	return match;
}
EXPORT_SYMBOL_GPL(phy_lookup_setting);

size_t phy_speeds(unsigned int *speeds, size_t size,
		  unsigned long *mask, size_t maxbit)
{
	size_t count;
	int i;

	for (i = 0, count = 0; i < ARRAY_SIZE(settings) && count < size; i++)
		if (settings[i].bit < maxbit &&
		    test_bit(settings[i].bit, mask) &&
		    (count == 0 || speeds[count - 1] != settings[i].speed))
			speeds[count++] = settings[i].speed;

	return count;
}

static inline void mmd_phy_indirect(struct mii_bus *bus, int prtad, int devad,
				    int addr)
{
	/* Write the desired MMD Devad */
	bus->write(bus, addr, MII_MMD_CTRL, devad);

	/* Write the desired MMD register address */
	bus->write(bus, addr, MII_MMD_DATA, prtad);

	/* Select the Function : DATA with no post increment */
	bus->write(bus, addr, MII_MMD_CTRL, (devad | MII_MMD_CTRL_NOINCR));
}

/**
 * phy_read_mmd_indirect - reads data from the MMD registers
 * @phydev: The PHY device bus
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 *
 * Description: it reads data from the MMD registers (clause 22 to access to
 * clause 45) of the specified phy address.
 * To read these register we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Read  reg 14 // Read MMD data
 */
int phy_read_mmd_indirect(struct phy_device *phydev, int prtad, int devad)
{
	struct phy_driver *phydrv = phydev->drv;
	int addr = phydev->mdio.addr;
	int value = -1;

	if (!phydrv->read_mmd_indirect) {
		struct mii_bus *bus = phydev->mdio.bus;

		mutex_lock(&bus->mdio_lock);
		mmd_phy_indirect(bus, prtad, devad, addr);

		/* Read the content of the MMD's selected register */
		value = bus->read(bus, addr, MII_MMD_DATA);
		mutex_unlock(&bus->mdio_lock);
	} else {
		value = phydrv->read_mmd_indirect(phydev, prtad, devad, addr);
	}
	return value;
}
EXPORT_SYMBOL(phy_read_mmd_indirect);

/**
 * phy_read_mmd - Convenience function for reading a register
 * from an MMD on a given PHY.
 * @phydev: The phy_device struct
 * @devad: The MMD to read from
 * @regnum: The register on the MMD to read
 *
 * Same rules as for phy_read();
 */
int phy_read_mmd(struct phy_device *phydev, int devad, u32 regnum)
{
	if (regnum > (u16)~0 || devad > 32)
		return -EINVAL;

	if (phydev->drv->read_mmd)
		return phydev->drv->read_mmd(phydev, devad, regnum);

	if (phydev->is_c45) {
		u32 addr = MII_ADDR_C45 | (devad << 16) | (regnum & 0xffff);
		return mdiobus_read(phydev->mdio.bus, phydev->mdio.addr, addr);
	}

	return phy_read_mmd_indirect(phydev, regnum, devad);
}
EXPORT_SYMBOL(phy_read_mmd);

/**
 * phy_write_mmd_indirect - writes data to the MMD registers
 * @phydev: The PHY device
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 * @data: data to write in the MMD register
 *
 * Description: Write data from the MMD registers of the specified
 * phy address.
 * To write these register we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Write reg 14 // Write MMD data
 */
void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
				   int devad, u32 data)
{
	struct phy_driver *phydrv = phydev->drv;
	int addr = phydev->mdio.addr;

	if (!phydrv->write_mmd_indirect) {
		struct mii_bus *bus = phydev->mdio.bus;

		mutex_lock(&bus->mdio_lock);
		mmd_phy_indirect(bus, prtad, devad, addr);

		/* Write the data into MMD's selected register */
		bus->write(bus, addr, MII_MMD_DATA, data);
		mutex_unlock(&bus->mdio_lock);
	} else {
		phydrv->write_mmd_indirect(phydev, prtad, devad, addr, data);
	}
}
EXPORT_SYMBOL(phy_write_mmd_indirect);

/**
 * phy_write_mmd - Convenience function for writing a register
 * on an MMD on a given PHY.
 * @phydev: The phy_device struct
 * @devad: The MMD to read from
 * @regnum: The register on the MMD to read
 * @val: value to write to @regnum
 *
 * Same rules as for phy_write();
 */
int phy_write_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val)
{
	if (regnum > (u16)~0 || devad > 32)
		return -EINVAL;

	if (phydev->drv->read_mmd)
		return phydev->drv->write_mmd(phydev, devad, regnum, val);

	if (phydev->is_c45) {
		u32 addr = MII_ADDR_C45 | (devad << 16) | (regnum & 0xffff);

		return mdiobus_write(phydev->mdio.bus, phydev->mdio.addr,
				     addr, val);
	}

	phy_write_mmd_indirect(phydev, regnum, devad, val);

	return 0;
}
EXPORT_SYMBOL(phy_write_mmd);
