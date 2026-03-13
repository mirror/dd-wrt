# s1g hostapd configuration specification

This document aims to outline the proposed S1G hostapd configuration. The
discussion and design within this document is driven by the core requirement to
allow Morse Micro customers to natively configure hostapd for S1G operations
(802.11ah).  The structure of this document is as follows. First it discusses
the user-level API which will allow users to natively configure 802.11ah APs.
Following that, the implementation details for the API will be discussed.

## User-level API

This section contains the proposed Sub-1 GHz hostapd API that will make it
possible for Morse Micro customers to configure APs natively.

```bash
# IEEE 802.11ah (S1G) supported.
# 0 = disabled (default)
# 1 = enabled
ieee80211ah=1

# Channel number (IEEE 802.11)
# (default: 0, i.e., not set)
# Please note that some drivers do not use this value from hostapd and the
# channel will need to be configured separately with iwconfig.
#
# For IEEE 802.11ah (S1G), the channel given is the operating channel. Which is
# specific to the regulatory domain. An operating class (op_class) also must be
# specified.
#
# If CONFIG_ACS build option is enabled, the channel can be selected
# automatically at run time by setting channel=acs_survey or channel=0, both of
# which will enable the ACS survey based algorithm.
channel=42

# Operating class to use (IEEE 802.11, Annex E).
# 
# This option allows hostapd to specify the operating class of the channel
# configured with the channel parameter. Channel and op_class together can
# uniquely identify channels across different bands, including the 6 GHz band.
#
# Global operating classes are valid (IEEE 802.11, Table E-4) with country code.
#
# For IEEE 80211.ah (S1G), an operating class must be specified.
# In this case, a global operating class, or a S1G specific operating class
# (IEEE 802.11, Annex E, Table E-5) (which are region-specific) is permitted.
op_class=69

# Country code (ISO/IEC 3166-1). Used to set regulatory domain.
# Set as needed to indicate country in which device is operating.
# This can limit available channels and transmit power.
# These two octets are used as the first two octets of the Country String
# (dot11CountryString)
#
# For 802.11ah (S1G) channel / frequency definitions change between
# different countries and operating classes. 
# The country code must be specified unless a local operating class
# is provided (in which case a provided country code must match).
#
# E.g. EU channel 36 is 863 MHz, JP channel 36 is 917MHz.
country_code=AU

# Sub-1 GHz primary channel bandwidth.
# 0 = 1 Mhz (default)
# 1 = 2 MHz 
s1g_prim_chwidth=1

# s1g_capab: S1G capabilities (list of flags)
#
# Short GI for 1MHz: [SHORT-GI-1]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and CBW = 1Mhz
# 0 = Not supported
# 1 = Supported (default)
#
# Short GI for 2MHz: [SHORT-GI-2]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and CBW = 2Mhz
# 0 = Not supported
# 1 = Supported (default)
#
# Short GI for 4MHz: [SHORT-GI-4]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and CBW = 4Mhz
# 0 = Not supported
# 1 = Supported (default)
#
# Short GI for 8MHz: [SHORT-GI-8]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and CBW = 8Mhz
# 0 = Not supported
# 1 = Supported (default)
#
# Short GI for 16MHz: [SHORT-GI-16]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and CBW = 16Mhz
# 0 = Not supported
# 1 = Supported (default)
#
# Short GI for channel bandwidths and subbandwidths: [SHORT-GI-ALL]
# Indicates short GI support for reception of packets transmitted with TXVECTOR
# params format equal to S1G and the configured channel bandwidth (including sub
# bandwidths). I.e This flag will enable SGI for 1,2 and 4MHz when the BSS channel
# operating bandwidth is 4MHz.
# 0 = Not supported
# 1 = Supported (default)

s1g_capab=[SHORT-GI-ALL]
```

The following section contains the additional proposed API that will be added at
a later date as required my Morse Micro.

```bash
# Sub-1 Ghz traveling pilots. Whether traveling pilots is enabled for sub-1 GHz.
# 0 = disabled
# 1 = enabled (default)
s1g_traveling_pilots=1

# Sub-1 GHz BSS color (0-7).
s1g_bss_color=0

#Sub-1 GHz MPDU maximum length.
# 0 = 3895 octets
# 1 = 7991 octets
s1g_max_mpdu=1

# Sub-1 GHz A-MPDU support.
# 0 = unsupported
# 1 = supported (default)
s1g_ampdu=1

# Sub-1 GHz A-MPDU length exponent. Indicates the maximum length of A-MPDU the
# STA can receive. Defined by 2(13 + s1g_max_mdpu_len_exp) - 1  octets.
# 0 = AMPDU length of 8k
# 1 = AMPDU length of 16k
# 2 = AMPDU length of 32k
# 3 = AMPDU length of 65k
s1g_max_ampdu_len_exp=0


# Sub-1 GHz basic NSS/MCS support. Indicates an 8-bit combination of 2-bit
# values. Each of these 2 bit values are used to indicate the max S1G-MCS
# supported for n spatial streams. More specifically bits 0 and 1 represent 1SS
# through bits 6 and 7 representing 4SS.
# Each of these 2-bit values is indicated as follows:
# 0 = indicates support for max S1G-MCS 2
# 1 = indicates support for max S1G-MCS 7
# 2 = indicates support for max S1G-MCS 9
# 3 = indicates that n spatial streams is not supported.
s1g_basic_mcs_nss_set=0x55
```

## Implementation detail

There are two key ways that features specified in the above API can be
implemented:

- **Using morse_cli for existing operations**

For most of the configuration, there is no current way to pass them via
ieee80211 APIs with nl80211. Instead, this configuration must be supplied via
the vendor-specific command in nl80211 which passes the information directly
to the driver.

We could implement these vendor-specific commands in hostapd to send these
directly. Alternatively, morse_cli is an existing tool which already does
this. For simplicity, we have opted to use morse_cli.

- **Pass configuration via nl80211**

The rest of the configuration can be passed to the driver via nl80211 by
mapping it to existing 802.11n/ac/ax configurations and commands. This is
currently being done with some configurations already. In instances where a
command does not already exist as a translation within hostapd, changes would
also be required to the Morse Micro driver.

We will be using a combination of both approaches as we both integrate the
initial configuration options and add more in the future. For configuration
options where morse_cli is already used it will be called as-is by hostapd.
Alternatively, where a translation for 802.11n already exists, or, there is a
sensible mapping for a configuration, we will pass the configuration via
nl80211.

This approach will mean that a particular device will be unable to support
both 802.11ah and 802.11n/ac/ax simultaneously.

### Capabilities

Each of the S1G commands and capabilities added to hostapd will be implemented
as below. This takes its inspiration from the 802.11ax configuration section
pre-existing in hostapd.

```bash
#s1g_short_guard_1mhz: Whether short guard is enabled for 1 MHz
# 0 = disabled
# 1 = enabled (Default)
s1g_short_guard_1mhz=1
```
### Extensibility

Given a large number of possible configurations are not included in the initial
implementation, a key part of the implementation is ensuring it's done in an
extensible and well-documented manner, to allow it for more options to be added
easily in the future.

There are two methods for adding a configuration option. The first is to use a
translation between 802.11n and 802.11ah. The second is to use morse_cli to
send the configuration to the driver. These methods are described below.

#### Adding new configuration

The following sections document adding a new configuration option to
hostapd. An example is provided throughout.

#### hostapd.conf

This file contains the API for hostapd, any configuration added to hostapd
should be documented here.

```c
# Sub-1 GHz primary channel bandwidth.
# 0 = 1 MHz
# 1 = 2 MHz
s1g_prim_chwidth=1
```

#### ap_config.h

ap_config.h contains the hostapd configuration information. New configurations
must be added to the hostapd_config struct, as shown below.

```c
struct hostapd_config {
    ...
    u8 s1g_prim_chwidth;
    ...
};
```

##### config_file.c

config_file.c contains the necessary functions to parse a hostapd.conf file.
New configurations must be added to the configuration parser as follows:

```c
static int hostapd_config_fill(struct hostapd_config *conf,
    struct hostapd_bss_config *bss,
    const char *buf, char *pos, int line)
{
...
	} else if (os_strcmp(buf, "s1g_prim_chwidth") == 0) {
		conf->s1g_prim_chwidth = atoi(pos);
...
};
```

#### Applying configuration - morse_cli

##### hostapd.c

New configuration that must be sent to the driver with morse_cli is provided
in the setup_interface function.

This assumes that the configuration requires a one-time startup message sent
to the driver.

```c
static int setup_interface(struct hostapd_iface *iface)
{
    ...

	u8 ret;
	char morse_cli_set_chan[256] = {0};

	sprintf(morse_cli_set_chan, "morse_cli -i %s channel -n %d -c %d -o %d -p %d",
			iface->conf->bss[0]->iface,
			iface->conf->s1g_chan.prim_1mhz_chan_index,
			iface->conf->s1g_chan.freq_khz,
			iface->conf->s1g_chan.op_chan_bw,
			iface->conf->s1g_chan.prim_chan_bw);

	ret = system(morse_cli_set_chan);

	if (!ret)
	    wpa_printf(MSG_ERROR, "error found in configuration file, invalid "
		       "channel configuration");
    ...
};
```

#### Applying configuration - nl80211 translation

Applying a configuration with a nl80211 translation is done on a case by
case basis. Some configuration options are dependant on other options being set.
An example is as follows:

```c
	if (conf->wmm_enabled < 0)
		conf->wmm_enabled = hapd->iconf->ieee80211n |
			hapd->iconf->ieee80211ax;
			hapd->iconf->ieee80211ax | hapd->iconf->ieee80211ah;

```

For example, WMM is enabled whenever ieee80211n/ax is enabled. In this
instance, we've also enabled it while ieee80211ah is enabled.
