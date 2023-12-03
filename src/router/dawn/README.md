![DAWN PICTURE](https://image.ibb.co/nbmNfJ/dawn_bla.png)

# DAWN
DAWN is a decentralized controller for wi-fi clients (eg, laptops, phones)
that aims to ensure that each client is connected to an access point (AP,
aka "wifi router") that will give good network throughput.  This reduces
wastage of radio capacity due to weak / distant radio connections that
cause transmission retry, degradation of speed, etc.

Decentralized means that DAWN is a peer-peer network of instances - there
is no controlling "master" node.  Every instance of DAWN will find its
peers via uMDNS (aka Bonjour) network discovery. Further DAWN instances
can therefore be added without explicit configuration of the whole
network, reducing network maintenance overhead.

## Installation and Configuration

Installing DAWN is quite simple.  The TL;DR version for anyone not wanting
to read the [installation guide](INSTALL.md) is:

- Install DAWN on each AP that you want to be a part of the optimization
network
- **Be sure to install a full wpad-\* version and not wpad-basic**
- Be sure to configure extender routers as "dumb AP"

Top tip from the [configuration guide](CONFIGURE.md):

- Be sure to enable the DAWN functions that you want (at least "kicking")

## How DAWN Works
Each DAWN instance gathers information about two types of client: those
that are currently connected to the host AP, plus those querying whether
to connect.  It then shares that information with other DAWN instances.
Each then has a view of how well any AP can "see" each client device, and
that then allows DAWN to steer a client to a different AP when appropriate.

For example, if a device is currently connected to AP1 with a signal
strength of -65dB it may be quite happy to stay there because many devices
will not look to roam until the signal goes past -70dB (note the minus
sign: -70dB is more negative / worse than -65dB).  However, if DAWN on AP1
knows that the same device can see AP2 at a level of -55dB then DAWN can
tell the device to switch to AP2, which will improve overall radio
performance if applied to multiple devices across multiple AP.

DAWN can also evaluate other parameters that a device may not include in
its roaming decisions, such as currently connected clients per AP,
availability of speed enhancing capabilities, etc.  However, it is
ultimately for a client device to decide which AP it will connect to so it
may ignore DAWN's proposal, especially if it is using a significantly
different algorithm to DAWN.

DAWN works best with networks where AP and devices have 802.11k
capabilities for discovering the quality of radio signals, plus 802.11v
for requesting devices to move to a different AP.  If 802.11r is enabled
for fast, seamless transfer of a device across APs then it enhances the
overall user experience, but DAWN doesn't directly use it.

802.11k/r/v were developed to enable the type of functionality that DAWN
provides, so older devices that do not have these capabilities are by
definition harder to control.  DAWN will try to steer "legacy" devices if
configured to do so, but it can be a less than perfect user experience due
to the time taken to create connections to the new AP.

## LuCI App
There is a luci app called
[luci-app-dawn](https://github.com/openwrt/luci/tree/master/applications/luci-app-dawn)
that adds some DAWN information to the management interface of an OpenWrt device.

NB: As of early-2022 it hasn't had the developer attention it needs
recently, so may not be working as well as you might like.

## Developers
If you want to use versions of DAWN that are not fully packaged yet or to
work on a fork of the code yourself then see the [Developer
Guide](DEVELOPER.md).
