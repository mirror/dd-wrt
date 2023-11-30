.. SPDX-License-Identifier: GPL-2.0

==========================================================
batctl - B.A.T.M.A.N. advanced control and management tool
==========================================================

Introduction
============

Why do I need batctl ? B.A.T.M.A.N. advanced operates on layer 2 and thus all
hosts participating in the virtual switch are completely transparent for all
protocols above layer 2. Therefore the common diagnosis tools do not work as
expected. To overcome these problems batctl was created. At the moment batctl
contains ping, traceroute, tcpdump and interfaces to the kernel module
settings.


How does it work ?
------------------

batctl uses the raw packet sockets to inject custom icmp packets into the data
flow. That's why ping and traceroute work almost like their IP based
counterparts. Tcpdump was designed because B.A.T.M.A.N. advanced encapsulates
all traffic within batman packets, so that the normal tcpdump would not
recognize the packets.


The bat-hosts file
------------------

This file is similar to the /etc/hosts file. You can write one MAC address and
one host name per line. batctl will analyze the file to find the matching MAC
address to your provided host name. Host names are much easier to remember than
MAC addresses.  ;)


Commands
========


batctl interface
----------------

display or modify the interface settings

Usage::

  batctl interface|if [add|del iface(s)]

Example::

  $  batctl interface
  eth0: active


batctl ping
-----------

Sends a Layer 2 batman-adv ping to check round trip time and connectivity

Usage::

  batctl ping [parameters] mac|bat-host|host-name|IP-address
  parameters:
           -c ping packet count
           -h print this help
           -i interval in seconds
           -t timeout in seconds
           -T don't try to translate mac to originator address
           -R record route

Example::

  $ batctl ping fe:fe:00:00:09:01
  PING fe:fe:00:00:09:01 (fe:fe:00:00:09:01) 19(47) bytes of data
  19 bytes from fe:fe:00:00:09:01 icmp_seq=1 ttl=43 time=8.74 ms
  19 bytes from fe:fe:00:00:09:01 icmp_seq=2 ttl=43 time=7.48 ms
  19 bytes from fe:fe:00:00:09:01 icmp_seq=3 ttl=43 time=8.23 ms
  ^C--- fe:fe:00:00:09:01 ping statistics ---
  3 packets transmitted, 3 received, 0% packet loss
  rtt min/avg/max/mdev = 7.476/8.151/8.743/1.267 ms


batctl statistics
-----------------

The batman-adv kernel module maintains a number of traffic counters which are exported
to user space. With batctl these counters can be easily retrieved. The output may vary
depending on which features have been compiled into the kernel module. For example, if
the distributed arp table (short: dat) wasn't selected as an option at compile time
its counters won't be shown.

Each module subsystem has its own counters which are indicated by their prefixes:

mgmt:
  mesh protocol counters
tt:
  translation table counters
dat:
  distributed arp table counters

All counters without a prefix concern payload (pure user data) traffic.

Usage::

  batctl statistics

Example::

  $ batctl statistics
          tx: 14
          tx_bytes: 1316
          tx_errors: 0
          rx: 14
          rx_bytes: 1316
          forward: 0
          forward_bytes: 0
          mgmt_tx: 18
          mgmt_tx_bytes: 762
          mgmt_rx: 17
          mgmt_rx_bytes: 1020
          tt_request_tx: 0
          tt_request_rx: 0
          tt_response_tx: 0
          tt_response_rx: 0
          tt_roam_adv_tx: 0
          tt_roam_adv_rx: 0
          dat_request_tx: 0
          dat_request_rx: 0
          dat_reply_tx: 1
          dat_reply_rx: 0


batctl tcpdump
--------------

tcpdump layer 2 and/or layer 3 traffic on the given interface

Usage::

  batctl tcpdump [parameters] interface [interface]
  parameters:
           -c compat filter - only display packets matching own compat version (14)
           -h print this help
           -n don't convert addresses to bat-host names
           -p dump specific packet type
           -x dump all packet types except specified
  packet types:
                    1 - batman ogm packets
                    2 - batman icmp packets
                    4 - batman unicast packets
                    8 - batman broadcast packets
                   16 - batman unicast tvlv packets
                   32 - batman fragmented packets
                   64 - batman tt / roaming packets
                  128 - non batman packets
                  129 - batman ogm & non batman packets

tcpdump supports standard interfaces as well as raw wifi interfaces running in monitor mode.

Example output for tcpdump::

  $ batctl tcpdump mesh0
  01:51:42.401188 BAT kansas: OGM via neigh kansas, seqno 6718, tq 255, ttl 50, v 9, flags [..I], length 28
  01:51:42.489735 BAT kansas: OGM via neigh wyoming, seqno 6718, tq 245, ttl 49, v 9, flags [.D.], length 28
  01:51:42.510330 BAT wyoming: OGM via neigh wyoming, seqno 6721, tq 255, ttl 50, v 9, flags [..I], length 28
  01:51:42.601092 BAT wyoming: OGM via neigh kansas, seqno 6721, tq 245, ttl 49, v 9, flags [.D.], length 28
  01:51:43.361076 BAT kansas > wyoming: ICMP echo request, id 0, seq 1, ttl 1, v 9, length 19
  01:51:43.365347 BAT wyoming > kansas: ICMP echo reply, id 0, seq 1, ttl 50, v 9, length 19
  01:51:43.372224 BAT kansas > wyoming: ICMP echo request, id 0, seq 2, ttl 1, v 9, length 19
  01:51:43.376506 BAT wyoming > kansas: ICMP echo reply, id 0, seq 2, ttl 50, v 9, length 19
  01:51:43.381250 BAT kansas: OGM via neigh kansas, seqno 6719, tq 255, ttl 50, v 9, flags [..I], length 28
  01:51:43.386281 BAT kansas > wyoming: ICMP echo request, id 0, seq 3, ttl 1, v 9, length 19
  01:51:43.387910 BAT wyoming > kansas: ICMP echo reply, id 0, seq 3, ttl 50, v 9, length 19
  01:51:43.479503 BAT kansas: OGM via neigh wyoming, seqno 6719, tq 245, ttl 49, v 9, flags [.D.], length 28
  01:51:43.509899 BAT wyoming: OGM via neigh wyoming, seqno 6722, tq 255, ttl 50, v 9, flags [..I], length 28
  01:51:43.600999 BAT wyoming: OGM via neigh kansas, seqno 6722, tq 245, ttl 49, v 9, flags [.D.], length 28
  01:51:44.381064 BAT kansas: OGM via neigh kansas, seqno 6720, tq 255, ttl 50, v 9, flags [..I], length 28


batctl traceroute
-----------------

Traceroute sends 3 packets to each hop, awaits the answers and prints out the
response times.

Usage::

  batctl traceroute [parameters] mac|bat-host|host-name|IP-address

Example::

  $ batctl traceroute fe:fe:00:00:09:01
  traceroute to fe:fe:00:00:09:01 (fe:fe:00:00:09:01), 50 hops max, 19 byte packets
   1: fe:fe:00:00:02:01 4.932 ms  2.338 ms  1.333 ms
   2: fe:fe:00:00:03:01 6.860 ms  1.579 ms  1.260 ms
   3: fe:fe:00:00:04:01 2.342 ms  1.547 ms  1.655 ms
   4: fe:fe:00:00:05:01 2.906 ms  2.211 ms  2.253 ms
   5: fe:fe:00:00:06:01 3.577 ms  2.687 ms  3.088 ms
   6: fe:fe:00:00:07:01 4.217 ms  5.741 ms  3.551 ms
   7: fe:fe:00:00:08:01 5.017 ms  5.547 ms  4.294 ms
   8: fe:fe:00:00:09:01 5.730 ms  4.970 ms  6.437 ms


batctl translate
----------------

Translates a destination (hostname, IPv4, IPv6, MAC, bat_host-name) to the
originator mac address responsible for it.

Usage::

  batctl translate mac|bat-host|host-name|IP-address

Example::

  $ batctl translate www.google.de
  02:ca:fe:af:fe:01
  $ batctl translate 02:ca:fe:af:fe:01
  02:ca:fe:af:fe:01
  $ batctl translate 192.168.1.2
  02:ca:fe:af:fe:05
  $ batctl translate fe:fe:00:00:09:01
  02:ca:fe:af:fe:05
  $ batctl translate 2001::1
  02:ca:fe:af:fe:05


Debug information tables
========================


batctl backbonetable
--------------------

Check the bridge loop avoidance backbone table

Usage::

  batctl backbonetable|bbt

Example::

  Originator           VID   last seen (CRC   )
  4a:97:a4:b8:fc:17 on    -1    1.376s (0x847a)


batctl claimtable
-----------------

Check the bridge loop avoidance claim table table

Usage::

  batctl claimtable|cl

Example::

  Client               VID      Originator        [o] (CRC   )
  e4:95:6e:4f:06:28 on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  08:ee:8b:84:82:8b on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  ac:86:74:9f:4d:80 on    -1 by 02:ba:de:af:fe:01 [*] (0x3b7e)
  60:14:66:6f:ec:52 on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  3a:ef:e8:e0:10:02 on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  56:bd:b4:a7:0b:aa on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  42:3a:6e:68:01:7d on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)
  0c:d7:46:2c:41:39 on    -1 by 02:ba:de:af:fe:01 [*] (0xbb73)


batctl dat_cache
----------------

display the local D.A.T. cache

Usage::

  batctl dat_cache|dc

Example::

  Distributed ARP Table (bat0):
            IPv4             MAC           last-seen
   *     172.100.0.1 b6:9b:d0:ea:b1:13      0:00

where

IPv4:
  is the IP address of a client in the mesh network
MAC:
  is the MAC address associated to that IP
last-seen:
  is the amount of time since last refresh of this entry


batctl gateways
---------------

Check the detected (and maybe selected) gateways

Usage::

  batctl gateways|gwl

Example::

  Router            ( TQ) Next Hop          [outgoingIf]  Bandwidth
  02:62:e7:ab:01:01 (180) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit
  02:62:e7:ab:05:01 (180) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit
  02:62:e7:ab:06:01 (235) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit
  02:62:e7:ab:02:01 (176) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit
  02:62:e7:ab:03:01 (180) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit
  02:62:e7:ab:04:01 (180) ae:1b:bf:52:25:58 [    enp0s1]: 10.0/2.0 MBit


batctl mcast_flags
------------------

display local and remote multicast flags

Usage::

  batctl mcast_flags|mf

Example::

  Multicast flags (own flags: [U46])
  * Bridged [U]                           U
  * No IGMP/MLD Querier [4/6]:            ./.
  * Shadowing IGMP/MLD Querier [4/6]:     4/6
  -------------------------------------------
         Originator Flags
  02:04:64:a4:39:c1 [U..]
  02:04:64:a4:39:c2 [U..]
  02:04:64:a4:39:c3 [...]

where

Originator:
  the MAC address of the originating (primary interface) batman-adv node
Flags:
  multicast flags of the according node
U:
  wants all unsnoopable multicast traffic, meaning other nodes need to always
  forward any multicast traffic destined to ff02::1 or 224.0.0.0/24 to it
4:
  wants all IPv4 multicast traffic, meaning other nodes need to always forward
  any IPv4 multicast traffic to it
6:
  wants all IPv6 multicast traffic, meaning other nodes need to always forward
  any IPv6 multicast traffic to it

If a node does not have multicast optimizations available (e.g. old batman-adv
version or optimizations not compiled in), therefore not announcing any
multicast tvlv/flags, a '-' will be displayed instead of '[...]'.


batctl neighbors
----------------

Check the neighbors table

Usage::

  batctl neighbors|n

Example::

  IF             Neighbor              last-seen
         enp0s1     16:7b:3c:c2:bf:b8    4.612s
         enp0s1     ae:1b:bf:52:25:58    0.740s


batctl originators
------------------

Check the Originators table

Usage::

  batctl originators|o

Example::

  $ batctl originators
  [B.A.T.M.A.N. adv 2011.4.0, MainIF/MAC: eth0/fe:fe:00:00:01:01 (bat0)]
    Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
  fe:fe:00:00:08:01    0.820s   (194) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 65) fe:fe:00:00:02:01 (194)
  fe:fe:00:00:03:01    0.980s   (245) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 81) fe:fe:00:00:02:01 (245)
  fe:fe:00:00:05:01    0.140s   (221) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 76) fe:fe:00:00:02:01 (221)
  fe:fe:00:00:04:01    0.010s   (235) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:02:01 (235) fe:fe:00:00:03:01 ( 81)
  fe:fe:00:00:09:01    0.830s   (187) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 63) fe:fe:00:00:02:01 (187)
  fe:fe:00:00:06:01    0.830s   (213) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 71) fe:fe:00:00:02:01 (213)
  fe:fe:00:00:02:01    0.240s   (255) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 81) fe:fe:00:00:02:01 (255)
  fe:fe:00:00:07:01    0.670s   (200) fe:fe:00:00:02:01 [      eth0]: fe:fe:00:00:03:01 ( 68) fe:fe:00:00:02:01 (200)

Since 2014.1.0, each batman interface has an individual originator table as well which is only used for routing.
These table explain to which neighbor a packet is forwarded when the packet is received on the specified interface.

Example::

  $ batctl originators -i eth0
  [B.A.T.M.A.N. adv master-b82b9b2, IF/MAC: eth0/fe:f0:00:00:02:01 (bat0 BATMAN_IV)]
    Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
  fe:f1:00:00:03:01    0.170s   (255) fe:f1:00:00:03:01 [      eth1]: fe:f1:00:00:03:01 (255)
  fe:f1:00:00:01:01    0.510s   (253) fe:f1:00:00:01:01 [      eth1]: fe:f1:00:00:01:01 (253)
  fe:f0:00:00:05:01    0.660s   (222) fe:f1:00:00:03:01 [      eth1]: fe:f0:00:00:03:01 (198) fe:f1:00:00:03:01 (222)
  fe:f0:00:00:03:01    0.560s   (252) fe:f1:00:00:03:01 [      eth1]: fe:f1:00:00:03:01 (252) fe:f0:00:00:03:01 (240)
  fe:f0:00:00:04:01    0.250s   (240) fe:f1:00:00:03:01 [      eth1]: fe:f1:00:00:03:01 (240) fe:f0:00:00:03:01 (211)
  fe:f0:00:00:01:01    0.850s   (255) fe:f1:00:00:01:01 [      eth1]: fe:f1:00:00:01:01 (255) fe:f0:00:00:01:01 (238)
  $ batctl originators -i eth1
  [B.A.T.M.A.N. adv master-b82b9b2, IF/MAC: eth1/fe:f1:00:00:02:01 (bat0 BATMAN_IV)]
    Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
  fe:f1:00:00:03:01    0.880s   (240) fe:f1:00:00:03:01 [      eth1]: fe:f1:00:00:03:01 (240)
  fe:f1:00:00:01:01    0.250s   (239) fe:f1:00:00:01:01 [      eth1]: fe:f1:00:00:01:01 (239)
  fe:f0:00:00:05:01    0.340s   (211) fe:f1:00:00:03:01 [      eth1]: fe:f0:00:00:03:01 (210) fe:f1:00:00:03:01 (211)
  fe:f0:00:00:03:01    0.260s   (253) fe:f0:00:00:03:01 [      eth0]: fe:f1:00:00:03:01 (240) fe:f0:00:00:03:01 (253)
  fe:f0:00:00:04:01    0.010s   (225) fe:f0:00:00:03:01 [      eth0]: fe:f1:00:00:03:01 (224) fe:f0:00:00:03:01 (225)
  fe:f0:00:00:01:01    0.510s   (255) fe:f0:00:00:01:01 [      eth0]: fe:f1:00:00:01:01 (240) fe:f0:00:00:01:01 (255)



batctl translocal
-----------------

display the local translation table

Usage::

  batctl translocal|tl

Example::

  $ batctl translocal
  Locally retrieved addresses (from bat0) announced via TT (TTVN: 1):
   * fe:fe:00:00:01:01 [RPNXW]

In particular, RPNXW are flags which have the following meanings:

R/Roaming:
  this client moved to another node but it is still kept for consistency reasons
  until the next OGM is sent.
P/noPurge:
  this client represents the local soft interface and will never be deleted.
N/New:
  this client has recently been added but is not advertised in the mesh until
  the next OGM is sent (for consistency reasons).
X/delete:
  this client has to be removed for some reason, but it is still kept for
  consistency reasons until the next OGM is sent.
W/Wireless:
  this client is connected to the node through a wireless device.

If any of the flags is not enabled, a '.' will substitute its symbol.


batctl transglobal
------------------

display the global translation table

Usage::

  batctl transglobal|tg

Example::

  Globally announced TT entries received via the mesh bat0
     Client	     (TTVN)     Originator        (Curr TTVN) Flags
   * fe:fe:00:00:01:01  ( 12) via fe:fe:00:00:01:02       ( 50) [RXW]

where

TTVN:
 is the translation-table-version-number which introduced this client
Curr TTVN:
  is the translation-table-version-number currently advertised by the
  originator serving this client (different clients advertised by the same
  originator have the same Curr TTVN)
Flags that mean:
  R/Roaming:
    this client moved to another node but it is still kept for consistency
    reasons until the next OGM is sent.
  X/delete:
    this client has to be removed for some reason, but it is still kept for
    consistency reasons until the next OGM is sent.
  W/Wireless:
    this client is connected to the node through a wireless device.

If any of the flags is not enabled, a '.' will substitute its symbol.


Settings
========


batctl aggregation
------------------

display or modify the packet aggregation setting

Usage::

  batctl aggregation|ag [0|1]


ap_isolation
------------

display or modify the client isolation setting

Usage::

  batctl ap_isolation|ap [0|1]


bonding
-------

display or modify the bonding setting

Usage::

  batctl bonding|b [0|1]


bridge_loop_avoidance
---------------------

display or modify the bridge_loop_avoidance setting

Usage::

  batctl bridge_loop_avoidance|bl [0|1]


distributed_arp_table
---------------------

display or modify the distributed_arp_table setting

Usage::

  batctl distributed_arp_table|dat [0|1]


batctl elp interval
-------------------

display or modify the elp interval in ms for hard interface

Usage::

  batctl hardif $hardif elp_interval|et [interval]

Example::

  $ batctl hardif eth0 elp_interval 200
  $ batctl hardif eth0 elp_interval
  200


fragmentation
-------------

display or modify the fragmentation setting

Usage::

  batctl fragmentation|f [0|1]


gw_mode
-------

display or modify the gw_mode setting

Usage::

  batctl gw_mode|gw [0|1]


batctl hop_penalty
------------------

display or modify the hop_penalty (0-255)

Usage::

  batctl hop_penalty|hp [penalty]

Example::

  $ batctl hop_penalty
  30
  $ batctl hardif eth0 hop_penalty
  0
  $ batctl hardif eth0 hop_penalty 50
  $ batctl hardif eth0 hop_penalty
  50


batctl isolation_mark
---------------------

display or modify the isolation mark.
This value is used by Extended Isolation feature.

Usage::

  batctl isolation_mark|mark $value[/0x$mask]

* Example 1: ``batctl mark 0x00000001/0xffffffff``
* Example 2: ``batctl mark 0x00040000/0xffff0000``
* Example 3: ``batctl mark 16``
* Example 4: ``batctl mark 0x0f``


batctl loglevel
---------------

display or modify the log level

Usage::

  batctl loglevel|ll [level]

Example::

  $  batctl loglevel
  [x] all debug output disabled (none)
  [ ] messages related to routing / flooding / broadcasting (batman)
  [ ] messages related to route added / changed / deleted (routes)
  [ ] messages related to translation table operations (tt)
  [ ] messages related to bridge loop avoidance (bla)
  [ ] messages related to arp snooping and distributed arp table (dat)
  [ ] messages related to network coding (nc)
  [ ] messages related to multicast (mcast)
  [ ] messages related to throughput meter (tp)


batctl multicast_fanout
-----------------------

display or modify the multicast fanout setting

Usage::

  batctl multicast_fanout|mo [fanout]


batctl multicast_forceflood
---------------------------

display or modify the multicast forceflood setting

Usage::

  batctl multicast_forceflood|mff [0|1]


batctl network_coding
---------------------

display or modify the network coding setting

Usage::

  batctl network_coding|nc [0|1]

Note that network coding requires a working promiscuous mode on all interfaces.


batctl orig_interval
--------------------

display or modify the originator interval in ms

Usage::

  batctl orig_interval|it [interval]

Example::

  $ batctl interval
  1000


batctl throughput override
--------------------------

display or modify the throughput override in kbit/s for hard interface

Usage::

  batctl hardif $hardif throughput_override|to [kbit]

Example::

  $ batctl hardif eth0 throughput_override 15000
  $ batctl hardif eth0 throughput_override 15mbit
  $ batctl hardif eth0 throughput_override
  15.0 MBit


JSON netlink query helper
=========================


batctl bla_backbone_json
------------------------

Query batman-adv for the entries in the known backbones table of
bridge loop avoidance.

Usage::

  batctl meshif <netdev> bla_backbone_json|bbj

Example::

  $ batctl meshif bat0 bla_backbone_json | json_pp
  [
      {
          "bla_backbone": "02:ba:de:af:fe:01",
          "bla_crc": 0,
          "bla_own": true,
          "bla_vid": -1,
          "last_seen_msecs": 920
      },
      {
          "bla_backbone": "02:ba:de:af:fe:01",
          "bla_crc": 33755,
          "bla_own": true,
          "bla_vid": -1,
          "last_seen_msecs": 44
      }
  ]


batctl bla_claim_json
---------------------

Query batman-adv for the entries in the known claims table of
bridge loop avoidance. Only claims from the current node will have have the
key-value ``"bla_own": true``.

Usage::

  batctl meshif <netdev> bla_claim_json|clj

Example::

  $ batctl meshif bat0 bla_claim_json | json_pp
  [
      {
          "bla_address": "a2:30:36:05:e6:32",
          "bla_backbone": "02:ba:de:af:fe:01",
          "bla_crc": 60445,
          "bla_own": true,
          "bla_vid": -1
      },
      {
          "bla_address": "24:18:1d:1c:d2:13",
          "bla_backbone": "02:ba:de:af:fe:01",
          "bla_crc": 60445,
          "bla_own": true,
          "bla_vid": -1
      },
      {
          "bla_address": "68:72:51:68:67:7a",
          "bla_backbone": "02:ba:de:af:fe:01",
          "bla_crc": 60445,
          "bla_own": true,
          "bla_vid": -1
      },
  [...]
  ]


batctl dat_cache_json
---------------------

Query batman-adv for the entries in cache of the distributed arp table.

Usage::

  batctl meshif <netdev> dat_cache_json|dcj

Example::

  $ batctl meshif bat0 dat_cache_json | json_pp
  [
      {
          "dat_cache_hwaddress": "10:8e:e0:62:dc:e8",
          "dat_cache_ip4address": "10.204.32.109",
          "dat_cache_vid": -1,
          "last_seen_msecs": 165752
      },
      {
          "dat_cache_hwaddress": "02:ba:7a:df:06:01",
          "dat_cache_ip4address": "10.204.32.7",
          "dat_cache_vid": -1,
          "last_seen_msecs": 364
      },
    [...]
  ]


batctl gateways_json
--------------------

Query batman-adv for the entries in the gateways list. Only selected gateways
(for the gateway mode "client) will have have the key-value ``"best": true``.

Usage::

  batctl meshif <netdev> gateways_json|gwj

Example::

  $ batctl meshif bat0 gateways_json | json_pp
  [
      {
          "bandwidth_down": 100,
          "bandwidth_up": 20,
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "orig_address": "02:62:e7:ab:01:01",
          "router": "ae:1b:bf:52:25:58",
          "tq": 180
      },
      {
          "bandwidth_down": 100,
          "bandwidth_up": 20,
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "orig_address": "02:62:e7:ab:05:01",
          "router": "ae:1b:bf:52:25:58",
          "tq": 180
      },
      {
          "bandwidth_down": 100,
          "bandwidth_up": 20,
          "best": true,
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "orig_address": "02:62:e7:ab:06:01",
          "router": "ae:1b:bf:52:25:58",
          "tq": 236
      },
    [...]
  ]


batctl hardif_json
------------------

Read the interface state for an interface which is part of a batman-adv
interface.

Usage::

  batctl hardif <netdev> hardif_json|hj

Example::

  $ batctl hardif enp0s1 hardif_json | json_pp
  {
      "active": true,
      "elp_interval": 500,
      "hard_address": "02:ba:de:af:fe:01",
      "hard_ifindex": 3,
      "hard_ifname": "enp0s1",
      "hop_penalty": 0,
      "mesh_ifindex": 9,
      "mesh_ifname": "bat0",
      "throughput_override": 0
  }


batctl hardifs_json
--------------------

Query batman-adv for entries in the list of interfaces added to a batadv
interface.

Usage::

  batctl meshif <netdev> hardifs_json|hj

Example::

  $ batctl meshif bat0 hardifs_json | json_pp
  [
      {
          "active": true,
          "elp_interval": 500,
          "hard_address": "4a:97:a4:b8:fc:17",
          "hard_ifindex": 2,
          "hard_ifname": "dummy0",
          "hop_penalty": 0,
          "mesh_ifindex": 9,
          "mesh_ifname": "bat0",
          "throughput_override": 0
      },
      {
          "active": true,
          "elp_interval": 500,
          "hard_address": "02:ba:de:af:fe:01",
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "hop_penalty": 0,
          "mesh_ifindex": 9,
          "mesh_ifname": "bat0",
          "throughput_override": 0
      }
  ]


batctl mcast_flags_json
-----------------------

Query batman-adv for entries multicast optimization table.

Usage::

  batctl meshif <netdev> mcast_flags_json|mfj

Example::

  $ batctl meshif bat0 mcast_flags_json | json_pp
  [
      {
          "mcast_flags": {
              "all_unsnoopables": true,
              "raw": 1,
              "want_all_ipv4": false,
              "want_all_ipv6": false,
              "want_no_rtr_ipv4": false,
              "want_no_rtr_ipv6": false
          },
          "orig_address": "9e:58:32:59:54:c3"
      },
      {
          "mcast_flags": {
              "all_unsnoopables": true,
              "raw": 1,
              "want_all_ipv4": false,
              "want_all_ipv6": false,
              "want_no_rtr_ipv4": false,
              "want_no_rtr_ipv6": false
          },
          "orig_address": "32:12:17:0a:21:63"
      },
      {
          "mcast_flags": {
              "all_unsnoopables": true,
              "raw": 1,
              "want_all_ipv4": false,
              "want_all_ipv6": false,
              "want_no_rtr_ipv4": false,
              "want_no_rtr_ipv6": false
          },
          "orig_address": "1a:34:8c:c4:fe:13"
      },
  [...]
  ]


batctl mesh_json
----------------

Get the current configuration of the batman-adv mesh interface and its global
state.

The ``hard_ifindex``/``hard_ifname`` only refers to the primary interface.
More interfaces might be attached to theis interface. They can for
example be queried using::

  ip -json link show master bat0

It also doesn't show all batman-adv interfaces on the system. Such
information must be queried using::

  ip -json link show type batadv


Usage::

  batctl meshif <netdev> mesh_json|mj

Example::

  $ batctl meshif bat0 mesh_json | json_pp
  {
      "aggregated_ogms_enabled": true,
      "algo_name": "BATMAN_IV",
      "ap_isolation_enabled": false,
      "bla_crc": 44249,
      "bonding_enabled": false,
      "bridge_loop_avoidance_enabled": true,
      "distributed_arp_table_enabled": true,
      "fragmentation_enabled": true,
      "gw_bandwidth_down": 100,
      "gw_bandwidth_up": 20,
      "gw_mode": "client",
      "gw_sel_class": 20,
      "hard_address": "02:ba:de:af:fe:01",
      "hard_ifindex": 3,
      "hard_ifname": "enp0s1",
      "hop_penalty": 30,
      "isolation_mark": 0,
      "isolation_mask": 0,
      "mcast_flags": {
          "all_unsnoopables": false,
          "raw": 24,
          "want_all_ipv4": false,
          "want_all_ipv6": false,
          "want_no_rtr_ipv4": true,
          "want_no_rtr_ipv6": true
      },
      "mcast_flags_priv": {
          "bridged": false,
          "querier_ipv4_exists": false,
          "querier_ipv4_shadowing": false,
          "querier_ipv6_exists": false,
          "querier_ipv6_shadowing": false,
          "raw": 0
      },
      "mesh_address": "3e:dc:94:68:80:e8",
      "mesh_ifindex": 9,
      "mesh_ifname": "bat0",
      "multicast_fanout": 16,
      "multicast_forceflood_enabled": false,
      "orig_interval": 5000,
      "tt_ttvn": 2,
      "version": "2021.0-15-gc84e5217"
  }


batctl neighbors_json
---------------------

Query batman-adv for the entries in the (direct) neighbors table.

Usage::

  batctl meshif <netdev> neighbors_json|nj

Example::

  $ batctl meshif bat0 neighbors_json
  [
      {
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "last_seen_msecs": 708,
          "neigh_address": "16:7b:3c:c2:bf:b8"
      },
      {
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "last_seen_msecs": 1872,
          "neigh_address": "ae:1b:bf:52:25:58"
      }
  ]


batctl originators_json
-----------------------

Query batman-adv for the entries in the originators table. The table doesn't
only contain the list of best next hops but also all other known entries.
Only next best hops have the key-value ``"best": true``.

Usage::

  batctl meshif <netdev> originators_json|oj

Example::

  $ batctl meshif bat0 originators_json | json_pp
  [
      {
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "last_seen_msecs": 4380,
          "neigh_address": "ae:1b:bf:52:25:58",
          "orig_address": "16:7b:3c:c2:bf:b8",
          "tq": 236
      },
      {
          "best": true,
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "last_seen_msecs": 4380,
          "neigh_address": "16:7b:3c:c2:bf:b8",
          "orig_address": "16:7b:3c:c2:bf:b8",
          "tq": 251
      },
  [...]
      {
          "best": true,
          "hard_ifindex": 3,
          "hard_ifname": "enp0s1",
          "last_seen_msecs": 4728,
          "neigh_address": "ae:1b:bf:52:25:58",
          "orig_address": "12:6d:7d:6f:f9:03",
          "tq": 77
      }
  ]


batctl transtable_global_json
-----------------------------

Query batman-adv for the entries in the global (mac-to-originator) translation
table. Only next best hops have the key-value ``"best": true``.

Usage::

  batctl meshif <netdev> transtable_global_json|tgj

Example::

  $ batctl meshif bat0 transtable_global_json
  [
      {
          "best": true,
          "orig_address": "ea:88:36:b0:fa:4b",
          "tt_address": "1e:df:a8:43:c4:d5",
          "tt_crc32": 3191293109,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": false,
              "pending": false,
              "raw": 2048,
              "roam": false,
              "temp": true,
              "wifi": false
          },
          "tt_last_ttvn": 255,
          "tt_ttvn": 255,
          "tt_vid": -1
      },
      {
          "best": true,
          "orig_address": "6a:d0:7f:eb:86:83",
          "tt_address": "01:00:5e:7f:ff:fa",
          "tt_crc32": 2358926211,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": false,
              "pending": false,
              "raw": 0,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_last_ttvn": 246,
          "tt_ttvn": 245,
          "tt_vid": -1
      },
      {
          "orig_address": "0e:68:8c:7c:0f:1b",
          "tt_address": "01:00:5e:7f:ff:fa",
          "tt_crc32": 1334456817,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": false,
              "pending": false,
              "raw": 0,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_last_ttvn": 21,
          "tt_ttvn": 20,
          "tt_vid": -1
      },
  [...]


batctl transtable_local_json
----------------------------

Query batman-adv for the entries in the (mac-to-originator) translation
table for locally detected MAC addresses.

Usage::

  batctl meshif <netdev> transtable_local_json|tgj

Example::

  $ batctl meshif bat0 transtable_local_json
  [
      {
          "tt_address": "3e:dc:94:68:80:e8",
          "tt_crc32": 3361904636,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": true,
              "pending": false,
              "raw": 256,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_vid": -1
      },
      {
          "tt_address": "3e:dc:94:68:80:e8",
          "tt_crc32": 1436598566,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": true,
              "pending": false,
              "raw": 256,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_vid": 0
      },
      {
          "tt_address": "01:00:5e:00:00:01",
          "tt_crc32": 3361904636,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": true,
              "pending": false,
              "raw": 256,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_vid": -1
      },
      {
          "tt_address": "33:33:ff:68:80:e8",
          "tt_crc32": 3361904636,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": true,
              "pending": false,
              "raw": 256,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_vid": -1
      },
      {
          "tt_address": "33:33:00:00:00:01",
          "tt_crc32": 3361904636,
          "tt_flags": {
              "del": false,
              "isolated": false,
              "new": false,
              "nopurge": true,
              "pending": false,
              "raw": 256,
              "roam": false,
              "temp": false,
              "wifi": false
          },
          "tt_vid": -1
      }
  ]


batctl vlan_json
----------------

Read the vlan state for an vlan on top a batman-adv interface.

Usage::

  batctl meshif <netdev> vid <vid> vlan_json|vj
  batctl vlan <vdev> vlan_json|vj

Example::

  $ batctl meshif bat0 vid 0 vlan_json
  {
      "ap_isolation_enabled": false,
      "mesh_ifindex": 9,
      "mesh_ifname": "bat0",
      "vlanid": 0
  }
  
  $ batctl vlan bat0.1 vlan_json
  {
      "ap_isolation_enabled": false,
      "mesh_ifindex": 9,
      "mesh_ifname": "bat0",
      "vlanid": 1
  }

Advanced Analytics
==================

batctl bisect_iv
----------------

Analyzes the B.A.T.M.A.N. IV logfiles to build a small internal database of all sent sequence
numbers and routing table changes. This database can be used to search for routing loops
(default action), to trace OGMs of  a  host  (use  "-t"  to specify  the  mac address or
bat-host name) throughout the network or to display routing tables of the nodes (use "-r" to
specify the mac address or bat-host name). You can name a specific sequence number or a range
using the "-s"  option  to limit the output's range. Furthermore you can filter the output by
specifying an originator (use "-o" to specify the mac address or bat-host name) to only see
data connected to  this  originator.  If  "-n"  was given batctl will not replace the mac
addresses with bat-host names in the output.

Usage::

  batctl bisect_iv [parameters] <file1> <file2> .. <fileN>
  parameters:
  
           -h print this help
           -l run a loop detection of given mac address or bat-host (default)
           -n don't convert addresses to bat-host names
           -r print routing tables of given mac address or bat-host
           -s seqno range to limit the output
           -t trace seqnos of given mac address or bat-host

Examples::

  $ batctl bisect_iv log/* -l uml3
  Analyzing routing tables of originator: uml3 [all sequence numbers]
  
  Checking host: uml3
  Path towards uml7 (seqno 9 via neigh uml5): -> uml5 -> uml6
  Path towards uml7 (seqno 10 via neigh uml4): -> uml4 -> uml5 -> uml6
  Path towards uml6 (seqno 4 via neigh uml4): -> uml4
  Path towards uml8 (seqno 12 via neigh uml4): -> uml4 -> uml5 -> uml6 -> uml7
  Path towards uml8 (seqno 203 via neigh uml4): -> uml4 -> uml6 -> uml7
  Path towards uml8 (seqno 391 via neigh uml2): -> uml2 -> uml3 -> uml2 aborted due to loop!
  Path towards uml8 (seqno 396 via neigh uml4): -> uml4 -> uml6 -> uml7
  Path towards uml9 (seqno 10 via neigh uml5): -> uml5 -> uml6 -> uml7 -> uml9.
  Path towards uml9 (seqno 10 via neigh uml4): -> uml4 -> uml5 -> uml6 -> uml7 -> uml9.
  Path towards uml9 (seqno 11 via neigh uml4): -> uml4 -> uml6 -> uml7 -> uml8 -> uml9.
  Path towards uml9 (seqno 12 via neigh uml4): -> uml4 -> uml5 -> uml6 -> uml7 -> uml8 -> uml9.
  Path towards uml9 (seqno 21 via neigh uml5): -> uml5 -> uml6 -> uml7 -> uml8 -> uml9.
  Path towards uml9 (seqno 22 via neigh uml4): -> uml4 -> uml5 -> uml6 -> uml7 -> uml8 -> uml9.
  
  $ ./batctl bisect_iv -t uml3 log/*
  Sequence number flow of originator: uml3 [all sequence numbers]
  [...]
  +=> uml3 (seqno 19)
  |- uml2 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
  |   |- uml3 [tq: 154, ttl: 49, neigh: uml2, prev_sender: uml3]
  |   \- uml1 [tq: 154, ttl: 49, neigh: uml2, prev_sender: uml3]
  |       |- uml3 [tq: 51, ttl: 48, neigh: uml1, prev_sender: uml2]
  |       \- uml2 [tq: 51, ttl: 48, neigh: uml1, prev_sender: uml2]
  |- uml5 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
  |   |- uml6 [tq: 33, ttl: 48, neigh: uml5, prev_sender: uml3]
  |   |   |- uml5 [tq: 11, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |   |- uml7 [tq: 11, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |   |   |- uml8 [tq: 3, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   |   |   |- uml6 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   |   |- uml9 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   |   \- uml7 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   |- uml6 [tq: 3, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   |   |- uml9 [tq: 3, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   |   \- uml5 [tq: 3, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   \- uml4 [tq: 11, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |- uml7 [tq: 33, ttl: 48, neigh: uml5, prev_sender: uml3]
  |   \- uml4 [tq: 33, ttl: 48, neigh: uml5, prev_sender: uml3]
  \- uml4 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
      |- uml3 [tq: 106, ttl: 49, neigh: uml4, prev_sender: uml3]
      |- uml6 [tq: 106, ttl: 49, neigh: uml4, prev_sender: uml3]
      |- uml2 [tq: 106, ttl: 49, neigh: uml4, prev_sender: uml3]
      \- uml5 [tq: 106, ttl: 49, neigh: uml4, prev_sender: uml3]
  +=> uml3 (seqno 20)
  |- uml2 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
  |   |- uml3 [tq: 160, ttl: 49, neigh: uml2, prev_sender: uml3]
  |   |- uml1 [tq: 160, ttl: 49, neigh: uml2, prev_sender: uml3]
  |   \- uml4 [tq: 160, ttl: 49, neigh: uml2, prev_sender: uml3]
  |- uml5 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
  |   |- uml3 [tq: 43, ttl: 48, neigh: uml5, prev_sender: uml3]
  |   |- uml6 [tq: 43, ttl: 48, neigh: uml5, prev_sender: uml3]
  |   |   |- uml8 [tq: 16, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |   |- uml5 [tq: 16, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |   |- uml7 [tq: 16, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   |   |   |- uml8 [tq: 5, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   |   |   |- uml6 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   |   |- uml9 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   |   \- uml7 [tq: 0, ttl: 45, neigh: uml8, prev_sender: uml7]
  |   |   |   \- uml6 [tq: 5, ttl: 46, neigh: uml7, prev_sender: uml6]
  |   |   \- uml4 [tq: 16, ttl: 47, neigh: uml6, prev_sender: uml5]
  |   \- uml4 [tq: 43, ttl: 48, neigh: uml5, prev_sender: uml3]
  |- uml1 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
  |   \- uml2 [tq: 49, ttl: 48, neigh: uml1, prev_sender: uml3]
  \- uml4 [tq: 255, ttl: 50, neigh: uml3, prev_sender: uml3]
      |- uml3 [tq: 114, ttl: 49, neigh: uml4, prev_sender: uml3]
      |- uml6 [tq: 114, ttl: 49, neigh: uml4, prev_sender: uml3]
      |- uml2 [tq: 114, ttl: 49, neigh: uml4, prev_sender: uml3]
      \- uml5 [tq: 114, ttl: 49, neigh: uml4, prev_sender: uml3]
  [...]


Appendix
========


batctl and network name spaces
------------------------------

The batman-adv kernel module is netns aware. Mesh instances can be
created in name spaces, and interfaces in that name space added to the
mesh. The mesh interface cannot be moved between name spaces, as is
typical for virtual interfaces.

The following example creates two network namespaces, and uses veth
pairs to connect them together into a mesh of three nodes::

  EMU1="ip netns exec emu1"
  EMU2="ip netns exec emu2"
  
  ip netns add emu1
  ip netns add emu2
  
  ip link add emu1-veth1 type veth peer name emu2-veth1
  ip link set emu1-veth1 netns emu1
  ip link set emu2-veth1 netns emu2
  
  $EMU1 ip link set emu1-veth1 name veth1
  $EMU2 ip link set emu2-veth1 name veth1
  
  $EMU1 ip link set veth1 up
  $EMU2 ip link set veth1 up
  
  ip link add emu1-veth2 type veth peer name veth2
  ip link set emu1-veth2 netns emu1
  $EMU1 ip link set emu1-veth2 name veth2
  
  $EMU1 ip link set veth2 up
  ip link set veth2 up
  
  $EMU1 batctl if add veth1
  $EMU1 batctl if add veth2
  $EMU1 ip link set bat0 up
  
  $EMU2 batctl if add veth1
  $EMU2 ip link set bat0 up
  
  batctl if add veth2
  ip link set bat0 up

alfred and batadv-vis can also be used with name spaces. In this
example, only netns has been used, so there are no filesystem name
spaces. Hence the unix domain socket used by alfred needs to be given
a unique name per instance::

  ($EMU1 alfred -m -i bat0 -u /var/run/emu1-alfred.soc) &
  ($EMU2 alfred -m -i bat0 -u /var/run/emu2-alfred.soc) &
  alfred -m -i bat0 &
  
  ($EMU1 batadv-vis -s -u /var/run/emu1-alfred.soc) &
  ($EMU2 batadv-vis -s -u /var/run/emu2-alfred.soc) &
  batadv-vis -s &
