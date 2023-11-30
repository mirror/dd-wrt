.. SPDX-License-Identifier: GPL-2.0

2023.3 (2023-11-15)
===================

* support latest kernels (4.14 - 6.7)
* coding style cleanups and refactoring
* only warn about too small MTU when soft interfaces wasn't already reduced
* bugs squashed:

  - Hold rtnl lock during MTU update via netlink

2023.2 (2023-08-16)
===================

* support latest kernels (4.14 - 6.5)
* bugs squashed:

  - avoid potential invalid memory access when processing ELP/OGM2 packets
  - drop pending DAT worker when interface shuts down
  - inform network stack about automatically adjusted MTUs
  - keep user defined MTU limit when MTU is recalculated
  - fix packet memory leak when sending OGM2 via inactive interfaces
  - fix TT memory leak for roamed back clients

2023.1 (2023-05-25)
===================

* support latest kernels (4.14 - 6.4)
* drop single unicast transfer optimization for unsnoopable IP addresses
* prepare infrastructure for multicast packets with multiple unicast destination
  addresses

2023.0 (2023-01-26)
===================

* support latest kernels (4.14 - 6.2)

2022.3 (2022-11-10)
===================

* support latest kernels (4.9 - 6.1)
* coding style cleanups and refactoring
* bugs squashed:

  - limit the minimum MTU of hard-interface to avoid
    "Forced to purge local tt entries" errors

2022.2 (2022-07-26)
===================

* support latest kernels (4.9 - 5.19)

2022.1 (2022-05-06)
===================

* support latest kernels (4.9 - 5.18)
* bugs squashed:

  - resolve "time-of-check-time-of-use" race condition when checking the
    the network namespace of a lower device
  - fix sanity check of network devices in different namespaces with
    colliding IDs
  - prevent transmission errors after splitting large GRO packets into
    smaller fragments

2022.0 (2022-02-03)
===================

* support latest kernels (4.9 - 5.17)
* dropped support for kernels < 4.9
* coding style cleanups and refactoring
* allow netlink usage in unprivileged containers
* bugs squashed:

  - don't send link-local multicast to mcast routers

2021.4 (2021-11-19)
===================

* support latest kernels (4.4 - 5.16)
* coding style cleanups and refactoring
* bugs squashed:

  - fix error handling during interface initialization

2021.3 (2021-09-14)
===================

* support latest kernels (4.4 - 5.15)
* coding style cleanups and refactoring
* reduced memory copy overhead when sending broadcasts

2021.2 (2021-08-20)
===================

* support latest kernels (4.4 - 5.14)
* coding style cleanups and refactoring
* add MRD + routable IPv4 multicast with bridges support
* rewrite of broadcast queuing
* bugs squashed:

  - avoid kernel warnings on timing related checks

2021.1 (2021-05-18)
===================

* support latest kernels (4.4 - 5.13)
* coding style cleanups and refactoring
* bugs squashed:

  - correctly initialize padding when sending out translation table TVLVs

2021.0 (2021-01-28)
===================

* support latest kernels (4.4 - 5.11)
* coding style cleanups and refactoring
* drop support for sysfs+debugfs
* allow to select routing algorithm during creation of interface
* bugs squashed:

  - allocate enough reserved room on fragments for lower devices

2020.4 (2020-10-27)
===================

* support latest kernels (4.4 - 5.10)
* coding style cleanups and refactoring
* bugs squashed:

  - fix incorrect reroute handling of multicast packets
  - improve handling of multicast packets by bridge loop avoidance

2020.3 (2020-08-24)
===================

* support latest kernels (4.4 - 5.9)
* coding style cleanups and refactoring
* introduce a configurable per interface hop penalty
* bugs squashed:

  - avoid uninitialized chaddr when handling DHCP
  - fix own OGMv2 check in aggregation receive handling
  - fix "NOHZ: local_softirq_pending 08" warnings caused by BLA

2020.2 (2020-07-06)
===================

* support latest kernels (4.4 - 5.8)
* coding style cleanups and refactoring
* dropped support for kernels < 4.4
* re-enabled link speed detection for interfaces without auto negotiation

2020.1 (2020-04-24)
===================

* support latest kernels (3.16 - 5.7)
* coding style cleanups and refactoring
* bugs squashed:

  - fix reference leaks in throughput_override sysfs file
  - fix reference leak in B.A.T.M.A.N. V OGM error handling
  - fix network coding random weighting

2020.0 (2020-03-04)
===================

* support latest kernels (3.16 - 5.6)
* coding style cleanups and refactoring
* use wifi tx rates as fallback for the B.A.T.M.A.N. V throughput estimation
* disable deprecated sysfs support by default
* bugs squashed:

  - fix crash during the scheduling of OGMs for removed interfaces

2019.5 (2019-12-12)
===================

* support latest kernels (3.16 - 5.5)
* coding style cleanups and refactoring
* bugs squashed:

  - fix DAT candidate selection on little endian systems

2019.4 (2019-10-25)
===================

* support latest kernels (3.16 - 5.4)
* coding style cleanups and refactoring
* implement aggregation of OGM2 packets
* bugs squashed:

  - fix length validation in netlink messages
  - fix out of buffer read when parsing aggregated packets
  - avoid race condition in OGM(2) packet modification and submission

2019.3 (2019-08-01)
===================

* support latest kernels (3.16 - 5.3)
* coding style cleanups and refactoring
* add routable multicast optimizations
* bugs squashed:

  - fix duplicated OGMs on NETDEV_UP
  - fix dumping of multicast flags

2019.2 (2019-05-23)
===================

* support latest kernels (3.16 - 5.2)
* coding style cleanups and refactoring
* implement multicast-to-unicast support for multiple targets
* deprecate sysfs support and make it optional
* refresh DAT entry timeouts on incoming ARP Replies
* bugs squashed:

  - fix multicast tt/tvlv worker locking
  - drop roam tvlv handler when unregistering mesh interface

2019.1 (2019-03-28)
===================

* support latest kernels (3.16 - 5.1)
* coding style cleanups and refactoring
* implement meshif, hardif and vlan interface via netlink
* snoop DHCP ACKs to improve DAT cache efficiency
* bugs squashed:

  - avoid potential reference counter underflows and use-after free errors for
    several objects
  - fix GCC warning when B.A.T.M.A.N. V is build in but cfg80211 is not
    available
  - reject too small VLAN packets before they are processed further

2019.0 (2019-02-01)
===================

* support latest kernels (3.16 - 5.0)
* coding style cleanups and refactoring
* allow to enable debug tracing without full batman-adv debugfs support
* enable inconsistency reporting for most netlink dump commands
* bugs squashed:

  - avoid unnecessary kernel warning (panic) during detection of interface loops
  - work around incorrect ethernet header offset in transmit code path

2018.4 (2018-11-14)
===================

* support latest kernels (3.16 - 4.20)
* coding style cleanups and refactoring
* improve tx performance by avoiding unnecessary locking
* add help messages regarding deprecation of debugfs files
* support for debug message tracepoints
* explicit ELP padding to allow TVLVs in the future
* bugs squashed:

  - fix skb_over_panic for merged fragments with small tailroom

2018.3 (2018-09-14)
===================

* support latest kernels (3.16 - 4.19)
* coding style cleanups and refactoring
* enable the DAT by default for the in-tree Linux module
* bugs squashed:

  - fix segfault when writing to sysfs files under batman_adv/ with
    CONFIG_BATMAN_ADV_DEBUG enabled
  - avoid information leakage in probe ELP packets
  - adjust reference counters when queue_work fails
  - prevent duplicated entries in various lists


2018.2 (2018-07-10)
===================

* support latest kernels (3.16 - 4.18)
* dropped support for kernels < 3.16 (note the removed build directory)
* coding style cleanups and refactoring
* avoid old nodes disabling multicast optimizations completely
* disable ethtool based B.A.T.M.A.N. V speed detection for virtual interfaces
* enable B.A.T.M.A.N. V compilation by default
* disable debugfs support by default
* bugs squashed:

  - fix TT sync flags for intermediate TT responses
  - avoid race in TT TVLV allocator helper
  - prevent TT request storms by not sending inconsistent TT TVLVs
  - fix best gw refcnt after netlink dump
  - adjust debugfs paths for interfaces after a namechange
  - fix single entry TT non-sync flag storage
  - fix multicast TT issues with bogus ROAM flags
  - work around insufficient memory initialization in cfg80211's station info

2018.1 (2018-04-25)
===================

* support latest kernels (3.2 - 4.17)
* coding style cleanups and refactoring
* add DAT cache and multicast flags netlink support
* avoid redundant multicast TT entries
* bugs squashed:

  - update data pointers after skb_cow()
  - fix header size check in batadv_dbg_arp()
  - fix skbuff rcsum on packet reroute
  - fix multicast-via-unicast transmission with AP isolation
  - fix packet loss for broadcasted DHCP packets to a server
  - fix multicast packet loss with a single WANT_ALL_IPV4/6 flag

2018.0 (2018-02-26)
===================

* support latest kernels (3.2 - 4.16)
* coding style cleanups and refactoring
* mark licenses clearer, change UAPI header from ISC to MIT
* bugs squashed:

  - fix packet checksum handling in receive path
  - fix handling of large number of interfaces
  - fix netlink dumping of gateways and BLA claims+backbones

2017.4 (2017-12-05)
===================

* support latest kernels (3.2 - 4.15)
* coding style cleanups and refactoring
* documentation cleanup
* bugs squashed:

  - avoid spurious warnings from bat_v neigh_cmp implementation
  - fix check of gateway availability in B.A.T.M.A.N. V
  - fix locking for bidirectional TQ check counters
  - remove leak of stack bits in fragmentation header priority


2017.3 (2017-09-28)
===================

* support latest kernels (3.2 - 4.14)
* coding style cleanups and refactoring
* bugs squashed:

  - fix TT sync flag inconsistencies
  - ignore invalid throughput values from wifi interfaces


2017.2 (2017-07-28)
===================

* support latest kernels (3.2 - 4.13)
* avoid bridge loop detection mac addresses in translation tables
* coding style cleanups and refactoring
* bugs squashed:

  - ignore invalid throughput values from wifi interfaces


2017.1 (2017-05-23)
===================

* support latest kernels (3.2 - 4.12)
* bridge loop avoidance improvements for various corner cases
* reduction of maximum fragment size
* coding style cleanups and refactoring
* bugs squashed:

  - fix rx packet/bytes stats on local DAT ARP reply


2017.0.1 (2017-03-07)
=====================

* support latest kernels (3.2 - 4.11)
* bugs squashed:

  - avoid API incompatibilities with new EWMA implementation
  - generate equally size fragments to reduce chance of padding or MTU problems
  - fix initialization of gateway sel class when BATMAN_V is compiled in


2017.0 (2017-02-28)
===================

* support latest kernels (3.2 - 4.11)
* remove bridge loop avoidance addresses from translation table
* allow to build B.A.T.M.A.N. V without cfg80211 integration on Linux < 3.16
* coding style cleanups and refactoring
* bugs squashed:

  - fix interface reference counter on fragmentation errors
  - avoid double free on fragment merge error
  - fix support for up to 16 fragments
  - fix build of multicast compatibility code on some architectures


2016.5 (2016-12-15)
===================

* support latest kernels (3.2 - 4.10)
* change installation path back to $INSTALL_MOD_DIR/updates/net/batman-adv
* add simple (re)broadcast avoidance
* reduce multicast TT and TVLV update delays under B.A.T.M.A.N. V
* support B.A.T.M.A.N. V throughput detection when using VLANs on top of WiFi
* improve documentation of sysfs and debugfs files
* coding style cleanups and refactoring
* bugs squashed:

  - detect missing primaryif during tp_send as error
  - fix re-adding of previously removed interfaces via rtnetlink
  - fix rare race conditions on interface removal
  - handle allocation error when generating TT responses


2016.4 (2016-10-27)
===================

* support latest kernels (3.2 - 4.9)
* add GW selection algorithm for B.A.T.M.A.N. V
* add support for querying most tables/lists via netlink
* adjusted device modifications for easier handling via rtnl (ip-link)
* disabled (bad) debugfs support in network namespaces
* add improved compat-patches support (note the new build directory)
* reduced translation table memory consumption
* coding style cleanups and refactoring
* bugs squashed:

  - avoid locking problems when modifying interfaces
  - fix sysfs errors on fast device deletion/creation events
  - add missing memory barriers for neighbor list modifications
  - fix tpmeter debug log output


2016.3 (2016-09-01)
===================

* support latest kernels (3.2 - 4.8)
* multicast optimization support for bridged setups
* initial, limited support for batman-adv netlink family
* throughput meter implementation
* support for frame priority in fragment packets
* kernel doc updates and coding style cleanups
* bugs squashed:

  - fix (re-)initialization of ELP tweaking options
  - fix elp packet data reservation
  - fix nullptr dereference after using vlan_insert_tag
  - fix various reference counting bugs in bonding, bla, tt and
    originator code
  - fix speedy join in gateway client mode


2016.2 (2016-06-09)
===================

* support latest kernels (3.2 - 4.7)
* initial, limited support for network namespaces
* kernel doc updates and coding style cleanups
* cleanup of legacy kernel compat code
* support for detection and reporting of complex bridge loops
* bugs squashed:

  - fix some TT issues: double free for full table request structs
    (created problems on multi-core systems) and a double put of VLAN
    objects
  - fix various B.A.T.M.A.N. V issues: fix mac address after address
    change, ELP orig address on secondary interfaces, null pointer
    dereference in metric comparison, refcount issues, ...
  - fix skb deref after transmission
  - avoid duplicate neigh_node additions
  - replace WARN with rate limited output on non-existing VLAN
  - fix ICMP RR ethernet header access after skb_linearize
  - fix memleak of untagged vlan when removing interface via rtnl-link
  - fix build against Debian Stretch kernels


2016.1 (2016-04-21)
===================

* support latest kernels (3.2 - 4.6)
* drop support for older kernels 2.6.29 - 3.1
* B.A.T.M.A.N. V routing algorithm support
* kernel doc updates and coding style cleanups
* conversion to the kref based reference counting framework
* bugs squashed:

  - fix endless loop in bat-on-bat netdevice check when using veth
  - fix various reference counting issues
  - init neigh node last seen field
  - fix integer overflow in batadv_iv_ogm_calc_tq
  - fix broadcast/ogm queue limit on a removed interface
  - fix DAT candidate selection to use VID
  - fix unexpected free of bcast_own on add_if error
  - check skb size before using encapsulated ETH+VLAN header


2016.0 (2016-01-19)
===================

* support latest kernels (2.6.29 - 4.5)
* add list of unique single hop neighbors and export it via debugfs
* massive kernel doc updates and coding style cleanups
* redesign/fix RCU handling when cleaning up to avoid bad memory access
* increase bridge loop avoidance wait time to 60 seconds
* remove bridge loop avoidance state when it gets disabled
* support for interfaces which switch from non-ethernet to ethernet mode
* bugs squashed:

  - fix lockdep splat when doing mcast_free or batadv_tlv_container_remove
  - fix invalid memory access when shrinking buffer for the OGM-return-rate
    measurement on interface removal


2015.2 (2015-11-23)
===================

* support latest kernels (2.6.29 - 4.4)
* cleanup of coding style and kernel docs
* fix includes in various files
* add lower layer head/tail room to avoid problems when slave devices
  encapsulate packets and have not enough space available
* fix hard_header_len which allows sending packets shorter than 64byte
* Remove obsolete deleted attribute for gateway node to simplify
  code and avoid delayed free of structures referenced by the gateway
* Add lockdep asserts to find locking problems
* bugs squashed:

  - Fix gw_bandwidth calculation on 32 bit systems
  - prevent potential hlist double deletion
  - fix soft interface access on unload
  - fix invalid stack access in DAT
  - lock CRC access in bridge loop avoidance
  - fix TT client duplicate detection with VLANs
  - fix excess VLANs in TT requests
  - avoid keeping false temporary TT entries
  - fix TT speedy join for DAT cache replies
  - fix TT memory leak on add with invalid VLAN


2015.1 (2015-08-04)
===================

* support latest kernels (2.6.29 - 4.2)
* cleanup of coding style
* cleanup of the compatibility layer
* convert to the Linux source directory structure
* adjust default configuration

  - disable network coding
  - enable bridge loop avoidance

* bugs squashed:

  - avoid DAT to mess up local LAN state
  - fix race conditions in DAT/NC/TT/MCAST TVLV handlers
  - fix build system POSIX compatibility
  - fix gateway selection in fast connection (1) gw_mode
  - fix initialization of detected gateway, which caused hangs on unloads
  - fix race conditions in the translation table
  - fix kernel crash due to missing NULL checks in vlan handlers
  - fix potentially broken header access by multicast optimization
  - fix broadcast packets cleanup for purged outgoing interface


2015.0 (2015-04-28)
===================

* support latest kernels (2.6.29 - 4.1)
* cleanup of coding style and add kerneldoc
* bugs squashed:
  - fix incorrect lockdep warning in network coding
  - fix condition when bonding should be used
  - fix support of bridged batman-adv devices with kernel < 2.6.39


2014.4.0 (2015-01-05)
=====================

* support latest kernels (2.6.29 - 3.19)
* double default hop penalty
* bugs squashed:

  - fix wrong size calculations and out of order support in
    fragmentation (fixes CVE-2014-9428)
  - fix double fetch in RCU for old kernels (<3.9)
  - fix NULL dereference and check in gateway code
  - fix multicast counters
  - fix network coding SKB control block initialization
  - fix last_seen initialization for orig nodes


2014.3.0 (2014-07-21)
=====================

* support latest kernels (2.6.29 - 3.16)
* drop QinQ claim frames in bridge loop avoidance
* fix a bogus warning from batadv_is_on_batman_iface()
* removed SET_ETHTOOL_OPS
* style improvements:

  - remove semi-colon after macro definition
  - add blank line between declarations and the rest of the code


2014.2.0 (2014-05-15)
=====================

* support latest kernels (2.6.29 - 3.15)
* add multicast optimization for certain type of multicast networks
  to send data only to nodes actually registered using new TVLVs
  and the translation table
* use ether_addr_copy instead of memcpy
* remove obsolete reset mac headers
* bugs squashed:

  - fix various (reference counting) bugs introduced by the multi
    interface optimization
  - fix a reference count problem when sending fragmented packets
  - count references for originator nodes referenced by gateway
  - fix local TT check for outgoing arp requests in DAT
  - fix TT inconsistencies when VLANs are re-created very fast
  - update TT entries for each VLAN when the MAC address of the
    parent interface is changed
  - improve documentation of DAT, TT and general kerneldoc


2014.1.0 (2014-03-13)
=====================

* support latest kernels (2.6.29 - 3.14)
* add mesh wide multi interface optimization, which replaces the old
  interface alternating and bonding features with a new network wide
  implementation
* add mesh wide client isolation based on fwmark by using and
  propagating the new isolation flag in TT
* send every DHCP packet as bat-unicast when gateway feature is used
* add new build checks for packet sizes to avoid architecture dependent
  problems
* bugs squashed:

  - deselect current gateway when switching away from client mode
  - fix batman-adv header MTU calculation
  - fix potential paging error for unicast transmissions
  - fix vlan refcounter imbalance on failure paths
  - fix TT-TVLV parsing and a TVLV leak
  - fix TT CRC computation by ensuring byte order
  - fix function names, paranthesis, comments, warnings, chachacha ...


2014.0.0 (2014-01-04)
=====================

* support latest kernels (2.6.29 - 3.13)
* This release contains major rework to allow better backward compatibility
  in the future. Unfortunately these changes require a bump of the compat
  version to 15, making this and future releases incompatible to the previous
  releases.
* add TVLV container infrastructure for OGMs and TT packets
* remove vis functionality (replaced by userspace tool alfred)
* reorder packet types and flags in packet types
* move some packet members (gw flags, TT, ...) into TVLV containers
* rewrite the fragementation code for more fragments (up to 16), more
  general usage (not only unicast packets) and reassembly on the way
* add VLAN awareness to TT, DAT and AP isolation
* use CRC32 instead of CRC16 for TT
* generalize many functions (neighbor comparison, etc) to prepare
  code-sharing of BATMAN IV and BATMAN V
* set SKB priority according to content (for correct WMM classification)
* add a dummy soft-interface rx mode handler to allow static multicast
  listeners
* bugs squashed:

  - various packet alignment and size fixes (especially on ARM)
  - white space, typos, kernel doc, etc
  - improve backward compatibility code


2013.4.0 (2013-10-13)
=====================

* support latest kernels (2.6.29 - 3.12)
* bugs squashed:

  - fix potential kernel paging errors for unicast transmissions
  - fix network coding initialization for multiple soft interfaces
  - fix BLA VLAN handling regression
  - improve backward compatibility code


2013.3.0 (2013-07-20)
=====================

* support latest kernels (2.6.29 - 3.11)
* send each broadcast only once on non-wireless interfaces
* change VID handling to prepare enhanced VLAN features
* bugs squashed:

  - forward late OGMs from best next hop
  - avoid double free of bat_counters
  - fix rcu barrier miss
  - use the proper header len when checking the TTVN
  - make DAT drop ARP requests targeting local clients

* style improvements, code moving and refactoring


2013.2.0 (2013-04-20)
=====================

* support latest kernels (2.6.29 - 3.10)
* add network coding feature
* add rtnl interface configuration support
* fix rtnl and sysfs locking issue
* avoid duplicate interface enslaving
* bugs squashed:

  - verify tt len to not exceed packet len
  - fix identification of own mac addresses

* style improvements (change seq_printf to seq_puts)


2013.1.0 (2013-03-02)
=====================

* support latest kernels (2.6.29 - 3.9)
* add lots of kerneldoc in types.h
* clean up (kerneldoc alignment, group tt definitions, renaming, ...)
* TT improvements:

  - add CRC to debugging tables
  - ignore multicast addresses
  - reduce local TT timeout from 1 hour to 10 minutes

* Initialize lockdep class keys for hashes
* unbloat bat_priv if debug is not enabled
* bugs squashed

  - fix possible sysfs/rtnl deadlock when deregistering
  - fix some DAT bugs (skb leak, invalid MAC addresses, NULL pointer dereference)


2013.0.0 (2013-01-12)
=====================

* support latest kernels (2.6.29 - 3.8)
* cleanup of coding style and add kerneldoc
* add new distributed ARP table feature to cache ARP entries
* remove __packed attribute whenever possible to allow better compiler
  optimizations
* use kernel-registered BATMAN Ethernet type
* block ECTP traffic to remove integration problems when using BLA
* allow roaming multiple times in TT
* bugs squashed

  - fix lockdeps for interfaces
  - disallow batman-over-batman configurations
  - various BLA fixes (wait at startup, hashing, duplist, ...)
  - fix TT roaming cases
  - fix TT packet rerouting
  - fix TT flags handling
  - fix speedy join/BLA interaction bug
  - fix random jitter calculation


2012.4.0 (2012-10-29)
=====================

* support latest kernels (2.6.29 - 3.7)
* cleanup of coding style
* integration of remaining packet counters in ethtool statistic
* speedy join of new non-mesh clients based on broadcast packets
* added bridge loop avoidance backbone gateway debugfs table
* workaround for kernel bug when running on top of vlan device
* bugs squashed

  - announcement of own MAC address after modification
  - wrong counting of received OGMs on x86
  - route flapping in multiple interfaces setup
  - receiving of translation table requests for foreign nodes
  - invalid memory access after failure during the interface appending
  - wrong calculation of packet crc which lead to dropping of broadcast
    packets when bridge loop avoidance is enabled


2012.3.0 (2012-08-19)
=====================

* support latest kernels (2.6.29 - 3.6)
* added namespace-like prefix for all batman-adv symbols
* integrated extended statistic support using ethtool
* important bugs fixed

  - correct endianness for translation table crc
  - avoid race condition in translation table replacements
  - ensure gateway gets selected
  - allow broadcasts with enabled AP isolation
  - fix vis output for multiple interface configurations
  - fix race condition during adds in hashes
  - fix dropped DHCP packets with enabled bridge loop avoidance and gateway
    support
  - don't leak information through uninitialized packets fields


2012.2.0 (2012-06-12)
=====================

* support latest kernels (2.6.29 - 3.5)
* cleanup of coding style (use of named constants instead of values, refactored
  code to reduce readability, replace bitarrays with kernel functionality, ...)
* tweaking hop penalty to reduce route flapping
* enhanced the framework to support multiple routing algorithms
* reimplemented the bridge loop avoidance with support for multiple active
  backbone gateways
* some bugs fixed (translation table flag handling, correct unicast rerouting,
  endianness fixed for translation table crc, avoid of routing loops by being
  strict on forwarded ogms, ...)


2012.1.0 (2012-03-30)
=====================

* support latest kernels (2.6.29 - 3.4)
* makefile rewrite (install target, cleanups, new selection system for features)
* cleanup of coding style (explicit marking of common headers, common unit for
  time specific defines, ...)
* Added framework to switch between different routing algorithms
* some bugs fixes (distinguish between wrap-around ttvn and uninitialized ttvn
  value, ...)


2012.0.0 (2012-02-05)
=====================

* support latest kernels (2.6.29 - 3.3)
* Fix bat_socket_read memory corruption (CVE-2011-4604)
* Cleanup of gateway handling code
* many bugs (hang when softif creation fails, memory leaks when hashes
  table cannot be filled, wrong filter for missed ogms, many smaller
  translation table problems, ...) fixed


2011.4.0 (2011-11-13)
=====================

* support latest kernels (2.6.29 - 3.2)
* starting of code refactoring to support multiple protocol versions
* added support for AP-isolation to prevent non-mesh WIFI clients to talk to
  each other over the mesh
* some bugs (memory leaks in the translation table, wrong initialization of
  ethernet addresses of translation table entries, ...) squashed


2011.3.1 (2011-10-18)
=====================

* don't send all packets to selected gateway as broadcast
* prevent translation table corruptions caused by uninitialized memory and
  invalid sizes send through client announcement mechanism


2011.3.0 (2011-08-21)
=====================

* support latest kernels (2.6.29 - 3.1)
* remove compat code for < 2.6.29
* cleanup of coding style
* improve client and roaming announcement mechanisms
* add framework to inform userspace of gateway changes using uevent
* improve gateway handling to filter out incoming DHCP renewal requests
* many bugs (acceptance of delayed rebroadcasts, unsigned char on powerpc used
  to store -1, ...) squashed


2011.2.0 (2011-06-19)
=====================

* support latest kernels (2.6.21 - 3.0)
* cleanup of coding style
* rename of HNA to TT
* support for multi vlan in bridge loop detection
* many bug fixes (rcu protection of router/primary_if/softif_neigh,
  race condition in TQ calculation, deadlock when creating new mesh
  interfaces, ...)


2011.1.0 (2011-04-17)
=====================

* support latest kernels (2.6.21 - 2.6.39)
* cleanup of coding style
* removal of the big orig_hash lock and usage of fine grained locking
* many bug fixes (fragmented packets linearisation, fragments numbering,
  verification of added interfaces, ...)


2011.0.0 (2011-01-29)
=====================

* support latest kernels (2.6.21 - 2.6.38)
* conversion of reference counting to kref
* allow merging and refragmentation of unicast packets during transfers
* add softif bridge loop detection
* make hop_penalty configurable through sysfs
* reimplement most of the batman-adv hash functionality
* support for optimized DHCP based gateway selection
* cleanup of the sysfs code


2010.2.0 (2010-11-21)
=====================

* support latest kernels (2.6.21 - 2.6.37)
* further cleanup of coding style
* new rcu and referenced based interface management
* support for multiple mesh clouds
* create packets directly in socket buffers
* add layer2 unicast packet fragmentation
* usage of optimised kernel functionality for ogm counting
* many bugs (false warnings, processing of big ogms, ...) squashed


2010.1.0 (2010-09-04)
=====================

* support latest kernels (2.6.21 - 2.6.36)
* further cleanup of coding style
* recording of routes for batman icmp messages
* move of complex sysfs files to debugfs
* change output of all sysfs files to single-value-only
* reintroduce virtual file for the debug log
* bonding and alternating added
* add ttl to broadcasts
* change all sequence numbers to 32 bit
* show last-seen in originator table
* many bugs (rounding issues, locking, netdev event handler, ...) squashed


2010.0.0 (2010-06-18)
=====================

* support latest kernels (2.6.21 - 2.6.35)
* further code refactoring and cleaning for coding style
* move from procfs based configuration to sysfs
* reorganized sequence number handling
* limit queue lengths for batman and broadcast packets
* many bugs (endless loop and rogue packets on shutdown, wrong tcpdump output,
  missing frees in error situations, sleeps in atomic contexts) squashed


0.2.1 (2010-03-21)
==================

* support latest kernels (2.6.20 - 2.6.33)
* receive packets directly using skbs, remove old sockets and threads
* fix various regressions in the vis server
* don't disable interrupts while sending
* replace internal logging mechanism with standard kernel logging
* move vis formats into userland, one general format remains in the kernel
* allow MAC address to be set, correctly initialize them
* code refactoring and cleaning for coding style
* many bugs (null pointers, locking, hash iterators) squashed


0.2 (2009-11-07)
================

* support latest kernels (2.6.20 - 2.6.31)
* temporary routing loops / TTL code bug / ghost entries in originator table fixed
* internal packet queue for packet aggregation & transmission retry (ARQ)
  for payload broadcasts added
* interface detection converted to event based handling to avoid timers
* major linux coding style adjustments applied
* all kernel version compatibility functions has been moved to compat.h
* use random ethernet address generator from the kernel
* /sys/module/batman_adv/version to export kernel module version
* vis: secondary interface export for dot draw format + JSON output format added
* many bugs (alignment issues, race conditions, deadlocks, etc) squashed


0.1 (2008-12-28)
================

* support latest kernels (2.6.20 - 2.6.28)
* LOTS of cleanup: locking, stack usage, memory leaks
* Change Ethertype from 0x0842 to 0x4305
  unregistered at IEEE, if you want to sponsor an official Ethertype ($2500)
  please contact us


0.1-beta (2008-05-05)
=====================

* layer 2 meshing based on BATMAN TQ algorithm in kernelland
* operates on any ethernet like interface
* supports IPv4, IPv6, DHCP, etc
* is controlled via /proc/net/batman-adv/
* bridging via brctl is supported
* interface watchdog (interfaces can be (de)activated dynamically)
* offers integrated vis server which meshes/syncs with other vis servers in range
