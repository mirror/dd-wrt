Abstract
==========================================================================
This document describes the OLSRd link quality extension that utilizes
linux NL80211 to apply wireless link information in the link quality
calculation.

Design
==========================================================================
Each second the latest information is gathered from linux NL80211. This
data contains the MAC addresses of the neighbor stations. To match this
MAC address with the neighbors IP, the linux ARP cache is queried.

Implementation
==========================================================================
The extension adds an external dependency to build the code namely libnl.
This library is used to simplify the IPC communication with linux kernel.

The files src/linux/nl80211_link_info.* are doing the actual wireless link
status gathering. A modified link ffeth quality plugin will use the new
link status information for link quality calculations. This link quality
plugin is using #ifdef LINUX_NL80211 statements as much as possible to
make it easy to merge the code back into the original ffeth plugin. The
benefit of merging those link quality plugins will be less duplicate code
and less code to maintain.

Cost calculation
==========================================================================
A penalty is added to the old cost from the ffeth plugin, depending on the
signal strength and the link bandwidth. Both penalties can have a maximum
value of 1.0.

Costs = EXT + BandwidthPenalty + SignalPenalty

BandwidthPenalty = 1 - ( ActualBandwidth / ReferenceBandwidth)

SignalPenalty = LookupSignalPenaltyTable(SignalStrenghtOfNeighbor)

Both penalties are added into the two unused bytes of LQ_HELLO messages.
Currently the nodes won't use this value when received from their neighbor
and only use their own NL80211 information.

Considerations
==========================================================================
It is designed mainly for IPv4, but should work with minimal effort on
IPv6 as well. Majority of that work will be actually testing it on IPv6.

The netlink code is blocking, this shouldn't cause major problems but a
more ideal design would be non-blocking.

Current version does not use the NL80211 data received from it's neighbors.
A discussion is needed to find out if this is required or not.

Currently both penalties have a maximum of 1.0, which might not be enough.
If that's the case, a configurable multiplier for both penalties might be
interesting to add.

The value for ReferenceBandwidth is hardcoded to 54 MBit.
The values in the signal strength penalty table are hardcoded.
It's desirable to have them configurable through the configuration file.

Add configuration option to completely disable the use of NL80211 data, in
case the plugin is merged with the existing ff_eth link quality plugin.