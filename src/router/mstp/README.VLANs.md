Using mstpd with Trunk Ports / VLANs
====================================

There are currently two different ways to implement spanning tree on
trunk ports using Linux bridging and mstpd:

1. Single Spanning Tree (RSTP/STP)
2. Per-VLAN Spanning Tree (PVST+)

Single Spanning Tree (RSTP/STP)
-------------------------------

* Create a Linux bridge and attach the trunk interfaces to it.
* Enable STP on the bridge.
* Force mstpd to use RSTP (`mstpctl setforcevers <bridge> rstp`).
* Create Linux VLAN interfaces on top of the bridge interface.
* If necessary, create a Linux bridge for each VLAN and attach the
  relevant VLAN interface to it.  Do not enable STP on these VLAN
  bridges.

This should be compatible with most switches.  If a connected switch
speaks MSTP, it will fall back to RSTP on our port.  If a connected
switch speak STP, mstpd will fall back to STP on that port.

However, note that Cisco switches in 'pvst' or 'rapid-pvst' mode **will
not** fall back to RSTP in this case.  To use a single Spanning Tree
with Cisco switches, you must put the Cisco switches in 'mst' mode
(`spanning-tree mode mst`).  Cisco switches in 'mst' mode will fall back
to PVST+ on ports connected to other switches that speak PVST+.

Per-VLAN Spanning Tree (PVST+)
------------------------------

* Create Linux VLAN interfaces on top of each trunk interface.
* For each VLAN, create a Linux bridge and attach the relevant VLAN
  interfaces to it.
* If the native VLAN is used (if the trunk may carry untagged packets),
  create another Linux bridge and attach the trunk interfaces to it.
  Then create `ebtables` rules to prevent this bridge from processing
  tagged packets, as described at
  http://blog.rackspace.com/vms-vlans-and-bridges-oh-my-part-2
* Enable STP on each bridge.
* Force mstpd to use RSTP on each bridge
  (`mstpctl setforcevers <bridge> rstp`).

This is only compatible with other switches that speak PVST+.
