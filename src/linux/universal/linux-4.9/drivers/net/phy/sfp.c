#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/sfp.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "mdio-i2c.h"
#include "swphy.h"

enum {
	GPIO_MODDEF0,
	GPIO_LOS,
	GPIO_TX_FAULT,
	GPIO_TX_DISABLE,
	GPIO_RATE_SELECT,
	GPIO_MAX,

	SFP_F_PRESENT = BIT(GPIO_MODDEF0),
	SFP_F_LOS = BIT(GPIO_LOS),
	SFP_F_TX_FAULT = BIT(GPIO_TX_FAULT),
	SFP_F_TX_DISABLE = BIT(GPIO_TX_DISABLE),
	SFP_F_RATE_SELECT = BIT(GPIO_RATE_SELECT),

	SFP_E_INSERT = 0,
	SFP_E_REMOVE,
	SFP_E_DEV_DOWN,
	SFP_E_DEV_UP,
	SFP_E_TX_FAULT,
	SFP_E_TX_CLEAR,
	SFP_E_LOS_HIGH,
	SFP_E_LOS_LOW,
	SFP_E_TIMEOUT,

	SFP_MOD_EMPTY = 0,
	SFP_MOD_PROBE,
	SFP_MOD_PRESENT,
	SFP_MOD_ERROR,

	SFP_DEV_DOWN = 0,
	SFP_DEV_UP,

	SFP_S_DOWN = 0,
	SFP_S_INIT,
	SFP_S_WAIT_LOS,
	SFP_S_LINK_UP,
	SFP_S_TX_FAULT,
	SFP_S_REINIT,
	SFP_S_TX_DISABLE,
};

static const char *gpio_of_names[] = {
	"moddef0",
	"los",
	"tx-fault",
	"tx-disable",
	"rate-select",
};

static const enum gpiod_flags gpio_flags[] = {
	GPIOD_IN,
	GPIOD_IN,
	GPIOD_IN,
	GPIOD_ASIS,
	GPIOD_ASIS,
};

#define T_INIT_JIFFIES	msecs_to_jiffies(300)
#define T_RESET_US	10
#define T_FAULT_RECOVER	msecs_to_jiffies(1000)

/* SFP module presence detection is poor: the three MOD DEF signals are
 * the same length on the PCB, which means it's possible for MOD DEF 0 to
 * connect before the I2C bus on MOD DEF 1/2.
 *
 * The SFP MSA specifies 300ms as t_init (the time taken for TX_FAULT to
 * be deasserted) but makes no mention of the earliest time before we can
 * access the I2C EEPROM.  However, Avago modules require 300ms.
 */
#define T_PROBE_INIT	msecs_to_jiffies(300)
#define T_PROBE_RETRY	msecs_to_jiffies(100)

/*
 * SFP modules appear to always have their PHY configured for bus address
 * 0x56 (which with mdio-i2c, translates to a PHY address of 22).
 */
#define SFP_PHY_ADDR	22

/*
 * Give this long for the PHY to reset.
 */
#define T_PHY_RESET_MS	50

static DEFINE_MUTEX(sfp_mutex);

struct sfp {
	struct device *dev;
	struct i2c_adapter *i2c;
	struct mii_bus *i2c_mii;
	struct net_device *ndev;
	struct phylink *phylink;
	struct phy_device *mod_phy;

	unsigned int (*get_state)(struct sfp *);
	void (*set_state)(struct sfp *, unsigned int);
	int (*read)(struct sfp *, bool, u8, void *, size_t);

	struct gpio_desc *gpio[GPIO_MAX];

	unsigned int state;
	struct delayed_work poll;
	struct delayed_work timeout;
	struct mutex sm_mutex;
	unsigned char sm_mod_state;
	unsigned char sm_dev_state;
	unsigned short sm_state;
	unsigned int sm_retries;

	struct sfp_eeprom_id id;

	struct notifier_block netdev_nb;
};

static unsigned long poll_jiffies;

static unsigned int sfp_gpio_get_state(struct sfp *sfp)
{
	unsigned int i, state, v;

	for (i = state = 0; i < GPIO_MAX; i++) {
		if (gpio_flags[i] != GPIOD_IN || !sfp->gpio[i])
			continue;

		v = gpiod_get_value_cansleep(sfp->gpio[i]);
		if (v)
			state |= BIT(i);
	}

	return state;
}

static void sfp_gpio_set_state(struct sfp *sfp, unsigned int state)
{
	if (state & SFP_F_PRESENT) {
		/* If the module is present, drive the signals */
		if (sfp->gpio[GPIO_TX_DISABLE])
			gpiod_direction_output(sfp->gpio[GPIO_TX_DISABLE],
						state & SFP_F_TX_DISABLE);
		if (state & SFP_F_RATE_SELECT)
			gpiod_direction_output(sfp->gpio[GPIO_RATE_SELECT],
						state & SFP_F_RATE_SELECT);
	} else {
		/* Otherwise, let them float to the pull-ups */
		if (sfp->gpio[GPIO_TX_DISABLE])
			gpiod_direction_input(sfp->gpio[GPIO_TX_DISABLE]);
		if (state & SFP_F_RATE_SELECT)
			gpiod_direction_input(sfp->gpio[GPIO_RATE_SELECT]);
	}
}

static int sfp__i2c_read(struct i2c_adapter *i2c, u8 bus_addr, u8 dev_addr,
	void *buf, size_t len)
{
	struct i2c_msg msgs[2];
	int ret;

	msgs[0].addr = bus_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &dev_addr;
	msgs[1].addr = bus_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = buf;

	ret = i2c_transfer(i2c, msgs, ARRAY_SIZE(msgs));
	if (ret < 0)
		return ret;

	return ret == ARRAY_SIZE(msgs) ? len : 0;
}

static int sfp_i2c_read(struct sfp *sfp, bool a2, u8 addr, void *buf,
	size_t len)
{
	return sfp__i2c_read(sfp->i2c, a2 ? 0x51 : 0x50, addr, buf, len);
}

static int sfp_i2c_configure(struct sfp *sfp, struct i2c_adapter *i2c)
{
	struct mii_bus *i2c_mii;
	int ret;

	if (!i2c_check_functionality(i2c, I2C_FUNC_I2C))
		return -EINVAL;

	sfp->i2c = i2c;
	sfp->read = sfp_i2c_read;

	i2c_mii = mdio_i2c_alloc(sfp->dev, i2c);
	if (IS_ERR(i2c_mii))
		return PTR_ERR(i2c_mii);

	i2c_mii->name = "SFP I2C Bus";
	i2c_mii->phy_mask = ~0;

	ret = mdiobus_register(i2c_mii);
	if (ret < 0) {
		mdiobus_free(i2c_mii);
		return ret;
	}

	sfp->i2c_mii = i2c_mii;

	return 0;
}


/* Interface */
static unsigned int sfp_get_state(struct sfp *sfp)
{
	return sfp->get_state(sfp);
}

static void sfp_set_state(struct sfp *sfp, unsigned int state)
{
	sfp->set_state(sfp, state);
}

static int sfp_read(struct sfp *sfp, bool a2, u8 addr, void *buf, size_t len)
{
	return sfp->read(sfp, a2, addr, buf, len);
}

static unsigned int sfp_check(void *buf, size_t len)
{
	u8 *p, check;

	for (p = buf, check = 0; len; p++, len--)
		check += *p;

	return check;
}

static const char *sfp_link_len(char *buf, size_t size, unsigned int length,
	unsigned int multiplier)
{
	if (length == 0)
		return "unsupported/unspecified";

	if (length == 255) {
		*buf++ = '>';
		size -= 1;
		length -= 1;
	}

	length *= multiplier;

	if (length >= 1000)
		snprintf(buf, size, "%u.%0*ukm",
			length / 1000,
			multiplier > 100 ? 1 :
			multiplier > 10 ? 2 : 3,
			length % 1000);
	else
		snprintf(buf, size, "%um", length);

	return buf;
}

struct bitfield {
	unsigned int mask;
	unsigned int val;
	const char *str;
};

static const struct bitfield sfp_options[] = {
	{
		.mask = SFP_OPTIONS_HIGH_POWER_LEVEL,
		.val = SFP_OPTIONS_HIGH_POWER_LEVEL,
		.str = "hpl",
	}, {
		.mask = SFP_OPTIONS_PAGING_A2,
		.val = SFP_OPTIONS_PAGING_A2,
		.str = "paginga2",
	}, {
		.mask = SFP_OPTIONS_RETIMER,
		.val = SFP_OPTIONS_RETIMER,
		.str = "retimer",
	}, {
		.mask = SFP_OPTIONS_COOLED_XCVR,
		.val = SFP_OPTIONS_COOLED_XCVR,
		.str = "cooled",
	}, {
		.mask = SFP_OPTIONS_POWER_DECL,
		.val = SFP_OPTIONS_POWER_DECL,
		.str = "powerdecl",
	}, {
		.mask = SFP_OPTIONS_RX_LINEAR_OUT,
		.val = SFP_OPTIONS_RX_LINEAR_OUT,
		.str = "rxlinear",
	}, {
		.mask = SFP_OPTIONS_RX_DECISION_THRESH,
		.val = SFP_OPTIONS_RX_DECISION_THRESH,
		.str = "rxthresh",
	}, {
		.mask = SFP_OPTIONS_TUNABLE_TX,
		.val = SFP_OPTIONS_TUNABLE_TX,
		.str = "tunabletx",
	}, {
		.mask = SFP_OPTIONS_RATE_SELECT,
		.val = SFP_OPTIONS_RATE_SELECT,
		.str = "ratesel",
	}, {
		.mask = SFP_OPTIONS_TX_DISABLE,
		.val = SFP_OPTIONS_TX_DISABLE,
		.str = "txdisable",
	}, {
		.mask = SFP_OPTIONS_TX_FAULT,
		.val = SFP_OPTIONS_TX_FAULT,
		.str = "txfault",
	}, {
		.mask = SFP_OPTIONS_LOS_INVERTED,
		.val = SFP_OPTIONS_LOS_INVERTED,
		.str = "los-",
	}, {
		.mask = SFP_OPTIONS_LOS_NORMAL,
		.val = SFP_OPTIONS_LOS_NORMAL,
		.str = "los+",
	}, { }
};

static const struct bitfield diagmon[] = {
	{
		.mask = SFP_DIAGMON_DDM,
		.val = SFP_DIAGMON_DDM,
		.str = "ddm",
	}, {
		.mask = SFP_DIAGMON_INT_CAL,
		.val = SFP_DIAGMON_INT_CAL,
		.str = "intcal",
	}, {
		.mask = SFP_DIAGMON_EXT_CAL,
		.val = SFP_DIAGMON_EXT_CAL,
		.str = "extcal",
	}, {
		.mask = SFP_DIAGMON_RXPWR_AVG,
		.val = SFP_DIAGMON_RXPWR_AVG,
		.str = "rxpwravg",
	}, { }
};

static const char *sfp_bitfield(char *out, size_t outsz, const struct bitfield *bits, unsigned int val)
{
	char *p = out;
	int n;

	*p = '\0';
	while (bits->mask) {
		if ((val & bits->mask) == bits->val) {
			n = snprintf(p, outsz, "%s%s",
				     out != p ? ", " : "",
				     bits->str);
			if (n == outsz)
				break;
			p += n;
			outsz -= n;
		}
		bits++;
	}

	return out;
}

static const char *sfp_connector(unsigned int connector)
{
	switch (connector) {
	case SFP_CONNECTOR_UNSPEC:
		return "unknown/unspecified";
	case SFP_CONNECTOR_SC:
		return "SC";
	case SFP_CONNECTOR_FIBERJACK:
		return "Fiberjack";
	case SFP_CONNECTOR_LC:
		return "LC";
	case SFP_CONNECTOR_MT_RJ:
		return "MT-RJ";
	case SFP_CONNECTOR_MU:
		return "MU";
	case SFP_CONNECTOR_SG:
		return "SG";
	case SFP_CONNECTOR_OPTICAL_PIGTAIL:
		return "Optical pigtail";
	case SFP_CONNECTOR_HSSDC_II:
		return "HSSDC II";
	case SFP_CONNECTOR_COPPER_PIGTAIL:
		return "Copper pigtail";
	default:
		return "unknown";
	}
}

static const char *sfp_encoding(unsigned int encoding)
{
	switch (encoding) {
	case SFP_ENCODING_UNSPEC:
		return "unspecified";
	case SFP_ENCODING_8B10B:
		return "8b10b";
	case SFP_ENCODING_4B5B:
		return "4b5b";
	case SFP_ENCODING_NRZ:
		return "NRZ";
	case SFP_ENCODING_MANCHESTER:
		return "MANCHESTER";
	default:
		return "unknown";
	}
}

/* Helpers */
static void sfp_module_tx_disable(struct sfp *sfp)
{
	dev_dbg(sfp->dev, "tx disable %u -> %u\n",
		sfp->state & SFP_F_TX_DISABLE ? 1 : 0, 1);
	sfp->state |= SFP_F_TX_DISABLE;
	sfp_set_state(sfp, sfp->state);
}

static void sfp_module_tx_enable(struct sfp *sfp)
{
	dev_dbg(sfp->dev, "tx disable %u -> %u\n",
		sfp->state & SFP_F_TX_DISABLE ? 1 : 0, 0);
	sfp->state &= ~SFP_F_TX_DISABLE;
	sfp_set_state(sfp, sfp->state);
}

static void sfp_module_tx_fault_reset(struct sfp *sfp)
{
	unsigned int state = sfp->state;

	if (state & SFP_F_TX_DISABLE)
		return;

	sfp_set_state(sfp, state | SFP_F_TX_DISABLE);

	udelay(T_RESET_US);

	sfp_set_state(sfp, state);
}

/* SFP state machine */
static void sfp_sm_set_timer(struct sfp *sfp, unsigned int timeout)
{
	if (timeout)
		mod_delayed_work(system_power_efficient_wq, &sfp->timeout,
				 timeout);
	else
		cancel_delayed_work(&sfp->timeout);
}

static void sfp_sm_next(struct sfp *sfp, unsigned int state,
			unsigned int timeout)
{
	sfp->sm_state = state;
	sfp_sm_set_timer(sfp, timeout);
}

static void sfp_sm_ins_next(struct sfp *sfp, unsigned int state, unsigned int timeout)
{
	sfp->sm_mod_state = state;
	sfp_sm_set_timer(sfp, timeout);
}

static void sfp_sm_phy_detach(struct sfp *sfp)
{
	phy_stop(sfp->mod_phy);
	if (sfp->phylink)
		phylink_disconnect_phy(sfp->phylink);
	phy_device_remove(sfp->mod_phy);
	phy_device_free(sfp->mod_phy);
	sfp->mod_phy = NULL;
}

static int sfp_sm_probe_phy(struct sfp *sfp)
{
	struct phy_device *phy;
	int err;

	msleep(T_PHY_RESET_MS);

	phy = mdiobus_scan(sfp->i2c_mii, SFP_PHY_ADDR);
	if (IS_ERR(phy)) {
		if (PTR_ERR(phy) == -ENODEV) {
			dev_dbg(sfp->dev, "no PHY detected\n");
			return -EAGAIN;
		}
		dev_err(sfp->dev, "mdiobus scan returned %ld\n", PTR_ERR(phy));
		return PTR_ERR(phy);
	}

	if (sfp->phylink) {
		err = phylink_connect_phy(sfp->phylink, phy);
		if (err) {
			phy_device_remove(phy);
			phy_device_free(phy);
			dev_err(sfp->dev, "phylink_connect_phy failed: %d\n",
				err);
			return err;
		}
	}

	sfp->mod_phy = phy;
	phy_start(phy);

	return 0;
}

static void sfp_sm_link_up(struct sfp *sfp)
{
	if (sfp->phylink)
		phylink_enable(sfp->phylink);

	sfp_sm_next(sfp, SFP_S_LINK_UP, 0);
}

static void sfp_sm_link_down(struct sfp *sfp)
{
	if (sfp->phylink)
		phylink_disable(sfp->phylink);
}

static void sfp_sm_link_check_los(struct sfp *sfp)
{
	unsigned int los = sfp->state & SFP_F_LOS;

	/* FIXME: what if neither SFP_OPTIONS_LOS_INVERTED nor
	 * SFP_OPTIONS_LOS_NORMAL are set?  For now, we assume
	 * the same as SFP_OPTIONS_LOS_NORMAL set.
	 */
	if (sfp->id.ext.options & SFP_OPTIONS_LOS_INVERTED)
		los ^= SFP_F_LOS;

	if (los)
		sfp_sm_next(sfp, SFP_S_WAIT_LOS, 0);
	else
		sfp_sm_link_up(sfp);
}

static void sfp_sm_fault(struct sfp *sfp, bool warn)
{
	if (sfp->sm_retries && !--sfp->sm_retries) {
		dev_err(sfp->dev, "module persistently indicates fault, disabling\n");
		sfp_sm_next(sfp, SFP_S_TX_DISABLE, 0);
	} else {
		if (warn)
			dev_err(sfp->dev, "module transmit fault indicated\n");

		sfp_sm_next(sfp, SFP_S_TX_FAULT, T_FAULT_RECOVER);
	}
}

static void sfp_sm_mod_init(struct sfp *sfp)
{
	int ret = 0;

	sfp_module_tx_enable(sfp);

	if (!sfp->phylink)
		return;

	/* Setting the serdes link mode is guesswork: there's no
	 * field in the EEPROM which indicates what mode should
	 * be used.
	 *
	 * If it's a gigabit-only fiber module, it probably does
	 * not have a PHY, so switch to 802.3z negotiation mode.
	 * Otherwise, switch to SGMII mode (which is required to
	 * support non-gigabit speeds) and probe for a PHY.
	 */
	if (sfp->id.base.e1000_base_t ||
	    sfp->id.base.e100_base_lx ||
	    sfp->id.base.e100_base_fx)
		ret = sfp_sm_probe_phy(sfp);

	if (!ret) {
		/* Wait t_init before indicating that the link is up, provided
		 * the current state indicates no TX_FAULT.  If TX_FAULT clears
		 * this time, that's fine too.
		 */
		sfp_sm_next(sfp, SFP_S_INIT, T_INIT_JIFFIES);
		sfp->sm_retries = 5;
		return;
	}

	if (ret == -EAGAIN)
		sfp_sm_set_timer(sfp, T_PROBE_RETRY);
	else
		sfp_sm_next(sfp, SFP_S_TX_DISABLE, 0);
}

static int sfp_sm_mod_probe(struct sfp *sfp)
{
	/* SFP module inserted - read I2C data */
	struct sfp_eeprom_id id;
	char vendor[17];
	char part[17];
	char sn[17];
	char date[9];
	char rev[5];
	char options[80];
	u8 check;
	int err;

	err = sfp_read(sfp, false, 0, &id, sizeof(id));
	if (err < 0) {
		dev_err(sfp->dev, "failed to read EEPROM: %d\n", err);
		return -EAGAIN;
	}

	if (err != sizeof(id)) {
		dev_err(sfp->dev, "EEPROM short read: %d\n", err);
		return -EAGAIN;
	}

	/* Validate the checksum over the base structure */
	check = sfp_check(&id.base, sizeof(id.base) - 1);
	if (check != id.base.cc_base) {
		dev_err(sfp->dev,
			"EEPROM base structure checksum failure: 0x%02x\n",
			check);
		print_hex_dump(KERN_ERR, "sfp EE: ", DUMP_PREFIX_OFFSET,
			       16, 1, &id, sizeof(id.base) - 1, true);
		return -EINVAL;
	}

	check = sfp_check(&id.ext, sizeof(id.ext) - 1);
	if (check != id.ext.cc_ext) {
		dev_err(sfp->dev,
			"EEPROM extended structure checksum failure: 0x%02x\n",
			check);
		memset(&id.ext, 0, sizeof(id.ext));
	}

	sfp->id = id;

	memcpy(vendor, sfp->id.base.vendor_name, 16);
	vendor[16] = '\0';
	memcpy(part, sfp->id.base.vendor_pn, 16);
	part[16] = '\0';
	memcpy(rev, sfp->id.base.vendor_rev, 4);
	rev[4] = '\0';
	memcpy(sn, sfp->id.ext.vendor_sn, 16);
	sn[16] = '\0';
	date[0] = sfp->id.ext.datecode[4];
	date[1] = sfp->id.ext.datecode[5];
	date[2] = '-';
	date[3] = sfp->id.ext.datecode[2];
	date[4] = sfp->id.ext.datecode[3];
	date[5] = '-';
	date[6] = sfp->id.ext.datecode[0];
	date[7] = sfp->id.ext.datecode[1];
	date[8] = '\0';

	dev_info(sfp->dev, "module %s %s rev %s sn %s dc %s\n", vendor, part, rev, sn, date);
	dev_info(sfp->dev, "  %s connector, encoding %s, nominal bitrate %u.%uGbps +%u%% -%u%%\n",
		 sfp_connector(sfp->id.base.connector),
		 sfp_encoding(sfp->id.base.encoding),
		 sfp->id.base.br_nominal / 10,
		 sfp->id.base.br_nominal % 10,
		 sfp->id.ext.br_max, sfp->id.ext.br_min);
	dev_info(sfp->dev, "  1000BaseSX%c 1000BaseLX%c 1000BaseCX%c 1000BaseT%c 100BaseTLX%c 1000BaseFX%c BaseBX10%c BasePX%c\n",
		 sfp->id.base.e1000_base_sx ? '+' : '-',
		 sfp->id.base.e1000_base_lx ? '+' : '-',
		 sfp->id.base.e1000_base_cx ? '+' : '-',
		 sfp->id.base.e1000_base_t ? '+' : '-',
		 sfp->id.base.e100_base_lx ? '+' : '-',
		 sfp->id.base.e100_base_fx ? '+' : '-',
		 sfp->id.base.e_base_bx10 ? '+' : '-',
		 sfp->id.base.e_base_px ? '+' : '-');

	if (!sfp->id.base.sfp_ct_passive && !sfp->id.base.sfp_ct_active &&
	    !sfp->id.base.e1000_base_t) {
		char len_9um[16], len_om[16];

		dev_info(sfp->dev, "  Wavelength %unm, fiber lengths:\n",
			 be16_to_cpup(&sfp->id.base.optical_wavelength));

		if (sfp->id.base.link_len[0] == 255)
			strcpy(len_9um, ">254km");
		else if (sfp->id.base.link_len[1] && sfp->id.base.link_len[1] != 255)
			sprintf(len_9um, "%um",
				sfp->id.base.link_len[1] * 100);
		else if (sfp->id.base.link_len[0])
			sprintf(len_9um, "%ukm", sfp->id.base.link_len[0]);
		else if (sfp->id.base.link_len[1] == 255)
			strcpy(len_9um, ">25.4km");
		else
			strcpy(len_9um, "unsupported");

		dev_info(sfp->dev, "    9µm SM    : %s\n", len_9um);
		dev_info(sfp->dev, " 62.5µm MM OM1: %s\n",
			 sfp_link_len(len_om, sizeof(len_om),
				      sfp->id.base.link_len[3], 10));
		dev_info(sfp->dev, "   50µm MM OM2: %s\n",
			 sfp_link_len(len_om, sizeof(len_om),
				      sfp->id.base.link_len[2], 10));
		dev_info(sfp->dev, "   50µm MM OM3: %s\n",
			 sfp_link_len(len_om, sizeof(len_om),
				      sfp->id.base.link_len[5], 10));
		dev_info(sfp->dev, "   50µm MM OM4: %s\n",
			 sfp_link_len(len_om, sizeof(len_om),
				      sfp->id.base.link_len[4], 10));
	} else {
		char len[16];
		dev_info(sfp->dev, "  Copper length: %s\n",
			 sfp_link_len(len, sizeof(len),
				      sfp->id.base.link_len[4], 1));
	}

	dev_info(sfp->dev, "  Options: %s\n",
		 sfp_bitfield(options, sizeof(options), sfp_options,
			      be16_to_cpu(sfp->id.ext.options)));
	dev_info(sfp->dev, "  Diagnostics: %s\n",
		 sfp_bitfield(options, sizeof(options), diagmon,
			      sfp->id.ext.diagmon));

	/* We only support SFP modules, not the legacy GBIC modules. */
	if (sfp->id.base.phys_id != SFP_PHYS_ID_SFP ||
	    sfp->id.base.phys_ext_id != SFP_PHYS_EXT_ID_SFP) {
		dev_err(sfp->dev, "module is not SFP - phys id 0x%02x 0x%02x\n",
			sfp->id.base.phys_id, sfp->id.base.phys_ext_id);
		return -EINVAL;
	}

	/*
	 * What isn't clear from the SFP documentation is whether this
	 * specifies the encoding expected on the TD/RD lines, or whether
	 * the TD/RD lines are always 8b10b encoded, but the transceiver
	 * converts.  Eg, think of a copper SFP supporting 1G/100M/10M
	 * ethernet: this requires 8b10b encoding for 1G, 4b5b for 100M,
	 * and manchester for 10M.
	 */
	/* 1Gbit ethernet requires 8b10b encoding */
	if (sfp->id.base.encoding != SFP_ENCODING_8B10B) {
		dev_err(sfp->dev, "module does not support 8B10B encoding\n");
		return -EINVAL;
	}

	if (sfp->phylink) {
		__ETHTOOL_DECLARE_LINK_MODE_MASK(support) = { 0, };
		int mode;
		u8 port;

		phylink_set(support, Autoneg);
		phylink_set(support, Pause);
		phylink_set(support, Asym_Pause);

		/* Set ethtool support from the compliance fields. */
		if (sfp->id.base.e10g_base_sr)
			phylink_set(support, 10000baseSR_Full);
		if (sfp->id.base.e10g_base_lr)
			phylink_set(support, 10000baseLR_Full);
		if (sfp->id.base.e10g_base_lrm)
			phylink_set(support, 10000baseLRM_Full);
		if (sfp->id.base.e10g_base_er)
			phylink_set(support, 10000baseER_Full);
		if (sfp->id.base.e1000_base_sx ||
		    sfp->id.base.e1000_base_lx ||
		    sfp->id.base.e1000_base_cx)
			phylink_set(support, 1000baseX_Full);
		if (sfp->id.base.e1000_base_t) {
			phylink_set(support, 1000baseT_Half);
			phylink_set(support, 1000baseT_Full);
		}

		/* port is the physical connector, set this from the
		 * connector field.
		 */
		switch (sfp->id.base.connector) {
		case SFP_CONNECTOR_SC:
		case SFP_CONNECTOR_FIBERJACK:
		case SFP_CONNECTOR_LC:
		case SFP_CONNECTOR_MT_RJ:
		case SFP_CONNECTOR_MU:
		case SFP_CONNECTOR_OPTICAL_PIGTAIL:
			phylink_set(support, FIBRE);
			port = PORT_FIBRE;
			break;

		case SFP_CONNECTOR_RJ45:
			phylink_set(support, TP);
			port = PORT_TP;
			break;

		case SFP_CONNECTOR_UNSPEC:
			if (sfp->id.base.e1000_base_t) {
				phylink_set(support, TP);
				port = PORT_TP;
				break;
			}
			/* fallthrough */
		case SFP_CONNECTOR_SG: /* guess */
		case SFP_CONNECTOR_MPO_1X12:
		case SFP_CONNECTOR_MPO_2X16:
		case SFP_CONNECTOR_HSSDC_II:
		case SFP_CONNECTOR_COPPER_PIGTAIL:
		case SFP_CONNECTOR_NOSEPARATE:
		case SFP_CONNECTOR_MXC_2X16:
		default:
			/* a guess at the supported link modes */
			dev_warn(sfp->dev, "Guessing link modes, please report...\n");
			phylink_set(support, 1000baseT_Half);
			phylink_set(support, 1000baseT_Full);
			port = PORT_OTHER;
			break;
		}

		/* Setting the serdes link mode is guesswork: there's no
		 * field in the EEPROM which indicates what mode should
		 * be used.
		 *
		 * If it's a gigabit-only fiber module, it probably does
		 * not have a PHY, so switch to 802.3z negotiation mode.
		 * Otherwise, switch to SGMII mode (which is required to
		 * support non-gigabit speeds) and probe for a PHY.
		 */
		if (!sfp->id.base.e1000_base_t &&
		    !sfp->id.base.e100_base_lx &&
		    !sfp->id.base.e100_base_fx) {
			mode = MLO_AN_8023Z;
		} else {
			mode = MLO_AN_SGMII;
		}

		phylink_set_link(sfp->phylink, mode, port, support);
	}

	return 0;
}

static void sfp_sm_mod_remove(struct sfp *sfp)
{
	if (sfp->mod_phy)
		sfp_sm_phy_detach(sfp);

	sfp_module_tx_disable(sfp);

	memset(&sfp->id, 0, sizeof(sfp->id));

	dev_info(sfp->dev, "module removed\n");
}

static void sfp_sm_event(struct sfp *sfp, unsigned int event)
{
	mutex_lock(&sfp->sm_mutex);

	dev_dbg(sfp->dev, "SM: enter %u:%u:%u event %u\n",
		sfp->sm_mod_state, sfp->sm_dev_state, sfp->sm_state, event);

	/* This state machine tracks the insert/remove state of
	 * the module, and handles probing the on-board EEPROM.
	 */
	switch (sfp->sm_mod_state) {
	default:
		if (event == SFP_E_INSERT) {
			sfp_module_tx_disable(sfp);
			sfp_sm_ins_next(sfp, SFP_MOD_PROBE, T_PROBE_INIT);
		}
		break;

	case SFP_MOD_PROBE:
		if (event == SFP_E_REMOVE) {
			sfp_sm_ins_next(sfp, SFP_MOD_EMPTY, 0);
		} else if (event == SFP_E_TIMEOUT) {
			int err = sfp_sm_mod_probe(sfp);

			if (err == 0)
				sfp_sm_ins_next(sfp, SFP_MOD_PRESENT, 0);
			else if (err == -EAGAIN)
				sfp_sm_set_timer(sfp, T_PROBE_RETRY);
			else
				sfp_sm_ins_next(sfp, SFP_MOD_ERROR, 0);
		}
		break;

	case SFP_MOD_PRESENT:
	case SFP_MOD_ERROR:
		if (event == SFP_E_REMOVE) {
			sfp_sm_mod_remove(sfp);
			sfp_sm_ins_next(sfp, SFP_MOD_EMPTY, 0);
		}
		break;
	}

	/* This state machine tracks the netdev up/down state */
	switch (sfp->sm_dev_state) {
	default:
		if (event == SFP_E_DEV_UP)
			sfp->sm_dev_state = SFP_DEV_UP;
		break;

	case SFP_DEV_UP:
		if (event == SFP_E_DEV_DOWN) {
			/* If the module has a PHY, avoid raising TX disable
			 * as this resets the PHY. Otherwise, raise it to
			 * turn the laser off.
			 */
			if (!sfp->mod_phy)
				sfp_module_tx_disable(sfp);
			sfp->sm_dev_state = SFP_DEV_DOWN;
		}
		break;
	}

	/* Some events are global */
	if (sfp->sm_state != SFP_S_DOWN &&
	    (sfp->sm_mod_state != SFP_MOD_PRESENT ||
	     sfp->sm_dev_state != SFP_DEV_UP)) {
		if (sfp->sm_state == SFP_S_LINK_UP &&
		    sfp->sm_dev_state == SFP_DEV_UP)
			sfp_sm_link_down(sfp);
		if (sfp->mod_phy)
			sfp_sm_phy_detach(sfp);
		sfp_sm_next(sfp, SFP_S_DOWN, 0);
		mutex_unlock(&sfp->sm_mutex);
		return;
	}

	/* The main state machine */
	switch (sfp->sm_state) {
	case SFP_S_DOWN:
		if (sfp->sm_mod_state == SFP_MOD_PRESENT &&
		    sfp->sm_dev_state == SFP_DEV_UP)
			sfp_sm_mod_init(sfp);
		break;

	case SFP_S_INIT:
		if (event == SFP_E_TIMEOUT && sfp->state & SFP_F_TX_FAULT)
			sfp_sm_fault(sfp, true);
		else if (event == SFP_E_TIMEOUT || event == SFP_E_TX_CLEAR)
			sfp_sm_link_check_los(sfp);
		break;

	case SFP_S_WAIT_LOS:
		if (event == SFP_E_TX_FAULT)
			sfp_sm_fault(sfp, true);
		else if (event ==
			 (sfp->id.ext.options & SFP_OPTIONS_LOS_INVERTED ?
			  SFP_E_LOS_HIGH : SFP_E_LOS_LOW))
			sfp_sm_link_up(sfp);
		break;

	case SFP_S_LINK_UP:
		if (event == SFP_E_TX_FAULT) {
			sfp_sm_link_down(sfp);
			sfp_sm_fault(sfp, true);
		} else if (event ==
			   (sfp->id.ext.options & SFP_OPTIONS_LOS_INVERTED ?
			    SFP_E_LOS_LOW : SFP_E_LOS_HIGH)) {
			sfp_sm_link_down(sfp);
			sfp_sm_next(sfp, SFP_S_WAIT_LOS, 0);
		}
		break;

	case SFP_S_TX_FAULT:
		if (event == SFP_E_TIMEOUT) {
			sfp_module_tx_fault_reset(sfp);
			sfp_sm_next(sfp, SFP_S_REINIT, T_INIT_JIFFIES);
		}
		break;

	case SFP_S_REINIT:
		if (event == SFP_E_TIMEOUT && sfp->state & SFP_F_TX_FAULT) {
			sfp_sm_fault(sfp, false);
		} else if (event == SFP_E_TIMEOUT || event == SFP_E_TX_CLEAR) {
			dev_info(sfp->dev, "module transmit fault recovered\n");
			sfp_sm_link_check_los(sfp);
		}
		break;

	case SFP_S_TX_DISABLE:
		break;
	}

	dev_dbg(sfp->dev, "SM: exit %u:%u:%u\n",
		sfp->sm_mod_state, sfp->sm_dev_state, sfp->sm_state);

	mutex_unlock(&sfp->sm_mutex);
}

static int sfp_module_info(void *priv, struct ethtool_modinfo *modinfo)
{
	struct sfp *sfp = priv;

	/* locking... and check module is present */

	if (sfp->id.ext.sff8472_compliance) {
		modinfo->type = ETH_MODULE_SFF_8472;
		modinfo->eeprom_len = ETH_MODULE_SFF_8472_LEN;
	} else {
		modinfo->type = ETH_MODULE_SFF_8079;
		modinfo->eeprom_len = ETH_MODULE_SFF_8079_LEN;
	}
	return 0;
}

static int sfp_module_eeprom(void *priv, struct ethtool_eeprom *ee, u8 *data)
{
	struct sfp *sfp = priv;
	unsigned int first, last, len;
	int ret;

	if (ee->len == 0)
		return -EINVAL;

	first = ee->offset;
	last = ee->offset + ee->len;
	if (first < ETH_MODULE_SFF_8079_LEN) {
		len = last;
		if (len > ETH_MODULE_SFF_8079_LEN)
			len = ETH_MODULE_SFF_8079_LEN;
		len -= first;

		ret = sfp->read(sfp, false, first, data, len);
		if (ret < 0)
			return ret;

		first += len;
		data += len;
	}
	if (first >= ETH_MODULE_SFF_8079_LEN && last > first) {
		len = last - first;

		ret = sfp->read(sfp, true, first, data, len);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static const struct phylink_module_ops sfp_module_ops = {
	.get_module_info = sfp_module_info,
	.get_module_eeprom = sfp_module_eeprom,
};

static void sfp_timeout(struct work_struct *work)
{
	struct sfp *sfp = container_of(work, struct sfp, timeout.work);

	sfp_sm_event(sfp, SFP_E_TIMEOUT);
}

static void sfp_check_state(struct sfp *sfp)
{
	unsigned int state, i, changed;

	state = sfp_get_state(sfp);
	changed = state ^ sfp->state;
	changed &= SFP_F_PRESENT | SFP_F_LOS | SFP_F_TX_FAULT;

	for (i = 0; i < GPIO_MAX; i++)
		if (changed & BIT(i))
			dev_dbg(sfp->dev, "%s %u -> %u\n", gpio_of_names[i],
				!!(sfp->state & BIT(i)), !!(state & BIT(i)));

	state |= sfp->state & (SFP_F_TX_DISABLE | SFP_F_RATE_SELECT);
	sfp->state = state;

	if (changed & SFP_F_PRESENT)
		sfp_sm_event(sfp, state & SFP_F_PRESENT ?
				SFP_E_INSERT : SFP_E_REMOVE);

	if (changed & SFP_F_TX_FAULT)
		sfp_sm_event(sfp, state & SFP_F_TX_FAULT ?
				SFP_E_TX_FAULT : SFP_E_TX_CLEAR);

	if (changed & SFP_F_LOS)
		sfp_sm_event(sfp, state & SFP_F_LOS ?
				SFP_E_LOS_HIGH : SFP_E_LOS_LOW);
}

static irqreturn_t sfp_irq(int irq, void *data)
{
	struct sfp *sfp = data;

	sfp_check_state(sfp);

	return IRQ_HANDLED;
}

static void sfp_poll(struct work_struct *work)
{
	struct sfp *sfp = container_of(work, struct sfp, poll.work);

	sfp_check_state(sfp);
	mod_delayed_work(system_wq, &sfp->poll, poll_jiffies);
}

static int sfp_netdev_notify(struct notifier_block *nb, unsigned long act, void *data)
{
	struct sfp *sfp = container_of(nb, struct sfp, netdev_nb);
	struct netdev_notifier_info *info = data;
	struct net_device *ndev = info->dev;

	if (!sfp->ndev || ndev != sfp->ndev)
		return NOTIFY_DONE;

	switch (act) {
	case NETDEV_UP:
		sfp_sm_event(sfp, SFP_E_DEV_UP);
		break;

	case NETDEV_GOING_DOWN:
		sfp_sm_event(sfp, SFP_E_DEV_DOWN);
		break;

	case NETDEV_UNREGISTER:
		if (sfp->mod_phy && sfp->phylink)
			phylink_disconnect_phy(sfp->phylink);
		phylink_unregister_module(sfp->phylink, sfp);
		sfp->phylink = NULL;
		dev_put(sfp->ndev);
		sfp->ndev = NULL;
		break;
	}
	return NOTIFY_OK;
}

static struct sfp *sfp_alloc(struct device *dev)
{
	struct sfp *sfp;

	sfp = kzalloc(sizeof(*sfp), GFP_KERNEL);
	if (!sfp)
		return ERR_PTR(-ENOMEM);

	sfp->dev = dev;

	mutex_init(&sfp->sm_mutex);
	INIT_DELAYED_WORK(&sfp->poll, sfp_poll);
	INIT_DELAYED_WORK(&sfp->timeout, sfp_timeout);

	sfp->netdev_nb.notifier_call = sfp_netdev_notify;

	return sfp;
}

static void sfp_destroy(struct sfp *sfp)
{
	cancel_delayed_work_sync(&sfp->poll);
	cancel_delayed_work_sync(&sfp->timeout);
	if (sfp->i2c_mii) {
		mdiobus_unregister(sfp->i2c_mii);
		mdiobus_free(sfp->i2c_mii);
	}
	if (sfp->i2c)
		i2c_put_adapter(sfp->i2c);
	of_node_put(sfp->dev->of_node);
	kfree(sfp);
}

static void sfp_cleanup(void *data)
{
	struct sfp *sfp = data;

	sfp_destroy(sfp);
}

static int sfp_probe(struct platform_device *pdev)
{
	struct sfp *sfp;
	bool poll = false;
	int irq, err, i;

	sfp = sfp_alloc(&pdev->dev);
	if (IS_ERR(sfp))
		return PTR_ERR(sfp);

	platform_set_drvdata(pdev, sfp);

	err = devm_add_action(sfp->dev, sfp_cleanup, sfp);
	if (err < 0)
		return err;

	if (pdev->dev.of_node) {
		struct device_node *node = pdev->dev.of_node;
		struct device_node *np;

		np = of_parse_phandle(node, "i2c-bus", 0);
		if (np) {
			struct i2c_adapter *i2c;

			i2c = of_find_i2c_adapter_by_node(np);
			of_node_put(np);
			if (!i2c)
				return -EPROBE_DEFER;

			err = sfp_i2c_configure(sfp, i2c);
			if (err < 0) {
				i2c_put_adapter(i2c);
				return err;
			}
		}

		for (i = 0; i < GPIO_MAX; i++) {
			sfp->gpio[i] = devm_gpiod_get_optional(sfp->dev,
					   gpio_of_names[i], gpio_flags[i]);
			if (IS_ERR(sfp->gpio[i]))
				return PTR_ERR(sfp->gpio[i]);
		}

		sfp->get_state = sfp_gpio_get_state;
		sfp->set_state = sfp_gpio_set_state;

		np = of_parse_phandle(node, "sfp,ethernet", 0);
		if (!np) {
			dev_err(sfp->dev, "missing sfp,ethernet property\n");
			return -EINVAL;
		}

		sfp->ndev = of_find_net_device_by_node(np);
		if (!sfp->ndev) {
			dev_err(sfp->dev, "ethernet device not found\n");
			return -EPROBE_DEFER;
		}

		dev_hold(sfp->ndev);
		put_device(&sfp->ndev->dev);

		sfp->phylink = phylink_lookup_by_netdev(sfp->ndev);
		if (!sfp->phylink) {
			dev_err(sfp->dev, "phylink for %s not found\n",
				netdev_name(sfp->ndev));
			return -EPROBE_DEFER;
		}

		phylink_disable(sfp->phylink);
		phylink_register_module(sfp->phylink, sfp, &sfp_module_ops);
	}

	sfp->state = sfp_get_state(sfp);
	if (sfp->gpio[GPIO_TX_DISABLE] &&
	    gpiod_get_value_cansleep(sfp->gpio[GPIO_TX_DISABLE]))
		sfp->state |= SFP_F_TX_DISABLE;
	if (sfp->gpio[GPIO_RATE_SELECT] &&
	    gpiod_get_value_cansleep(sfp->gpio[GPIO_RATE_SELECT]))
		sfp->state |= SFP_F_RATE_SELECT;
	sfp_set_state(sfp, sfp->state);
	sfp_module_tx_disable(sfp);
	if (sfp->state & SFP_F_PRESENT)
		sfp_sm_event(sfp, SFP_E_INSERT);

	for (i = 0; i < GPIO_MAX; i++) {
		if (gpio_flags[i] != GPIOD_IN || !sfp->gpio[i])
			continue;

		irq = gpiod_to_irq(sfp->gpio[i]);
		if (!irq) {
			poll = true;
			continue;
		}

		err = devm_request_threaded_irq(sfp->dev, irq, NULL, sfp_irq,
						IRQF_ONESHOT |
						IRQF_TRIGGER_RISING |
						IRQF_TRIGGER_FALLING,
						dev_name(sfp->dev), sfp);
		if (err)
			poll = true;
	}

	if (poll)
		mod_delayed_work(system_wq, &sfp->poll, poll_jiffies);

	register_netdevice_notifier(&sfp->netdev_nb);

	return 0;
}

static int sfp_remove(struct platform_device *pdev)
{
	struct sfp *sfp = platform_get_drvdata(pdev);

	unregister_netdevice_notifier(&sfp->netdev_nb);
	if (sfp->ndev)
		dev_put(sfp->ndev);

	return 0;
}

static const struct of_device_id sfp_of_match[] = {
	{ .compatible = "sff,sfp", },
	{ },
};
MODULE_DEVICE_TABLE(of, sfp_of_match);

static struct platform_driver sfp_driver = {
	.probe = sfp_probe,
	.remove = sfp_remove,
	.driver = {
		.name = "sfp",
		.of_match_table = sfp_of_match,
	},
};

static int sfp_init(void)
{
	poll_jiffies = msecs_to_jiffies(100);

	return platform_driver_register(&sfp_driver);
}
module_init(sfp_init);

static void sfp_exit(void)
{
	platform_driver_unregister(&sfp_driver);
}
module_exit(sfp_exit);

MODULE_ALIAS("platform:sfp");
MODULE_AUTHOR("Russell King");
MODULE_LICENSE("GPL v2");
