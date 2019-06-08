/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009-2016 Cavium, Inc.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/io.h>

#include "mdio-cavium.h"

static void cavium_mdiobus_set_mode(struct cavium_mdiobus *p,
				    enum cavium_mdiobus_mode m)
{
	union cvmx_smix_clk smi_clk;

	if (m == p->mode)
		return;

	smi_clk.u64 = oct_mdio_readq(p->register_base + SMI_CLK);
	smi_clk.s.mode = (m == C45) ? 1 : 0;
	smi_clk.s.preamble = 1;
	oct_mdio_writeq(smi_clk.u64, p->register_base + SMI_CLK);
	p->mode = m;
}

static int cavium_mdiobus_c45_addr(struct cavium_mdiobus *p,
				   int phy_id, int regnum)
{
	union cvmx_smix_cmd smi_cmd;
	union cvmx_smix_wr_dat smi_wr;
	int timeout = 1000;

	cavium_mdiobus_set_mode(p, C45);

	smi_wr.u64 = 0;
	smi_wr.s.dat = regnum & 0xffff;
	oct_mdio_writeq(smi_wr.u64, p->register_base + SMI_WR_DAT);

	regnum = (regnum >> 16) & 0x1f;

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 0; /* MDIO_CLAUSE_45_ADDRESS */
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = regnum;
	oct_mdio_writeq(smi_cmd.u64, p->register_base + SMI_CMD);

	do {
		/* Wait 1000 clocks so we don't saturate the RSL bus
		 * doing reads.
		 */
		__delay(1000);
		smi_wr.u64 = oct_mdio_readq(p->register_base + SMI_WR_DAT);
	} while (smi_wr.s.pending && --timeout);

	if (timeout <= 0)
		return -EIO;
	return 0;
}

int cavium_mdiobus_read(struct mii_bus *bus, int phy_id, int regnum)
{
	struct cavium_mdiobus *p = bus->priv;
	union cvmx_smix_cmd smi_cmd;
	union cvmx_smix_rd_dat smi_rd;
	unsigned int op = 1; /* MDIO_CLAUSE_22_READ */
	int timeout = 1000;

	if (regnum & MII_ADDR_C45) {
		int r = cavium_mdiobus_c45_addr(p, phy_id, regnum);

		if (r < 0)
			return r;

		regnum = (regnum >> 16) & 0x1f;
		op = 3; /* MDIO_CLAUSE_45_READ */
	} else {
		cavium_mdiobus_set_mode(p, C22);
	}

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = op;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = regnum;
	oct_mdio_writeq(smi_cmd.u64, p->register_base + SMI_CMD);

	do {
		/* Wait 1000 clocks so we don't saturate the RSL bus
		 * doing reads.
		 */
		__delay(1000);
		smi_rd.u64 = oct_mdio_readq(p->register_base + SMI_RD_DAT);
	} while (smi_rd.s.pending && --timeout);

	if (smi_rd.s.val)
		return smi_rd.s.dat;
	else
		return -EIO;
}
EXPORT_SYMBOL(cavium_mdiobus_read);

int cavium_mdiobus_write(struct mii_bus *bus, int phy_id, int regnum, u16 val)
{
	struct cavium_mdiobus *p = bus->priv;
	union cvmx_smix_cmd smi_cmd;
	union cvmx_smix_wr_dat smi_wr;
	unsigned int op = 0; /* MDIO_CLAUSE_22_WRITE */
	int timeout = 1000;

	if (regnum & MII_ADDR_C45) {
		int r = cavium_mdiobus_c45_addr(p, phy_id, regnum);

		if (r < 0)
			return r;

		regnum = (regnum >> 16) & 0x1f;
		op = 1; /* MDIO_CLAUSE_45_WRITE */
	} else {
		cavium_mdiobus_set_mode(p, C22);
	}

	smi_wr.u64 = 0;
	smi_wr.s.dat = val;
	oct_mdio_writeq(smi_wr.u64, p->register_base + SMI_WR_DAT);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = op;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = regnum;
	oct_mdio_writeq(smi_cmd.u64, p->register_base + SMI_CMD);

	do {
		/* Wait 1000 clocks so we don't saturate the RSL bus
		 * doing reads.
		 */
		__delay(1000);
		smi_wr.u64 = oct_mdio_readq(p->register_base + SMI_WR_DAT);
	} while (smi_wr.s.pending && --timeout);

	if (timeout <= 0)
		return -EIO;

	return 0;
}
EXPORT_SYMBOL(cavium_mdiobus_write);
#if 0
static void _set_led(int l)
{
	int mbus = 0;
	int maddr = 7;
	uint16_t val = 0;
	if (gd->ogd.board_desc.board_type == CVMX_BOARD_TYPE_UBNT_E200
	    && (gd->ogd.board_desc.rev_major == BOARD_E200_MAJOR
		|| gd->ogd.board_desc.rev_major == BOARD_E202_MAJOR)) {
		mbus = 1;
		maddr = 1;
	}
	switch (l) {
	case 0:
		val = 0x10;
		break;
	case 1:
		val = 0x8;
		break;
	default:
		break;
	}
	cvmx_mdio_write(mbus, maddr, 0x10, val);
}

static inline int cvmx_mdio_write(int bus_id, int phy_id, int location, int val)
{
#if defined(CVMX_BUILD_FOR_LINUX_KERNEL) && defined(CONFIG_PHYLIB)
	return -1;
#else
	cvmx_smix_cmd_t smi_cmd;
	cvmx_smix_wr_dat_t smi_wr;

	if (octeon_has_feature(OCTEON_FEATURE_MDIO_CLAUSE_45))
		__cvmx_mdio_set_clause22_mode(bus_id);

	smi_wr.u64 = 0;
	smi_wr.s.dat = val;
	cvmx_write_csr(CVMX_SMIX_WR_DAT(bus_id), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = MDIO_CLAUSE_22_WRITE;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = location;
	cvmx_write_csr(CVMX_SMIX_CMD(bus_id), smi_cmd.u64);

	if (CVMX_WAIT_FOR_FIELD64(CVMX_SMIX_WR_DAT(bus_id), cvmx_smix_wr_dat_t, pending, ==, 0, CVMX_MDIO_TIMEOUT))
		return -1;

	return 0;
#endif
}
#endif

MODULE_DESCRIPTION("Common code for OCTEON and Thunder MDIO bus drivers");
MODULE_AUTHOR("David Daney");
MODULE_LICENSE("GPL");
