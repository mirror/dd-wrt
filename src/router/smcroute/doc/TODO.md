
Refactor MRDISC Support
-----------------------

The current MRDISC implementation is fragile (see issue #175 for an
example), and it also does not work on non-Linux systems.  So the
implementation really needs to be refactored, not just for this but also
for adding IPv6 support (below).


Add Support for IPv6 MRDISC
---------------------------

[RFC4286][1] details both IPv4 and IPv6, which should not be problem
to support in SMCRoute.  Anyone with Wireshark and a bit of patience
could add it.  Your patch is welcome! :)

[1]: https://datatracker.ietf.org/doc/html/rfc4286


Tests must have uniquely named netns (if any)
---------------------------------------------

I've had to rename the R1/R2 netns used in test/gre.sh, because it turns
out these names are shared when we run in the unshare and gre.sh's names
clashed with multi.sh's.  I.e., they were stomping hard on each other's
toes and often the gre.sh test completed before multi.sh, thus causing
the latter to lose both R1 and R2 and the test failed (of course).

We may be able to work around this by running each test in its own root
netns.  Something that could be set up by lib.sh.  I have not verified
this yet, hence this TODO.


Add support for WRONGVIF, somehow
---------------------------------

When an (S,G) for an already installed kernel MFC route suddenly appears
on another inbound interface, the kernel sends a WRONGVIF upcall message
to smcrouted.  Currently we don't act on it, mostly because there is no
clear idea how to deal with such cases from a small routing daemon that
knows nothing of the rest of the layer-3 topology (which may have been
reconfigured due to link loss by, e.g. OSPF).  It can be a valid change
of inbound interface, or looped back traffic that we don't want.

For now, users are recommended to install multiast snooping switches on
their outbound interfaces, and/or leverage to `phyint foo ttl-threshold`
setting to prevent already routed multicast from being looped back.

One idea is to delegate the behavior to our external script facility.


Investigate why MIF thresholds don't seem to work on Linux 5.11.0
------------------------------------------------------------------

Here smcrouted has managed to set the TTL thresholds of three OIFs to
values != 1, but the kernel still lists all of them as `:1`.  See test
`reload6.sh`

    Group                            Origin                           Iif      Pkts  Bytes     Wrong  Oifs
    ff2e:0000:0000:0000:0000:0000:0000:0042 fc00:0000:0000:0000:0000:0000:0000:0001 0          0        0        0  2:1    4:1    5:1


Possibly Exit with Error if Multicast Socket is Busy
----------------------------------------------------

Currently we only log this state and then continue.  Not sure what is
the best approach, but everything read from a .conf will fail so not
much point really continuing?

    smcroute[2359]: IPv4 multicast routing API already in use: Address in use

Proposal: exit with error if either mrouting socket is busy.
          We need to consider this a configuration error.


Support for (re-)enumerating VIFs at runtime
--------------------------------------------

Currently the `-t SEC` startup delay option has to be used if not all
interfaces are available when `smcrouted` starts.  Commonly a problem at
boot, but also if adding a pluggable interface (PCMCIA/USB) at runtime.

Hence, it would be a great addition to SMCRoute if new interface VIF/MIF
mappings could be at least added at runtime.


Support for filtering based on source ADDRESS/LEN
-------------------------------------------------

When setting up a (*,G/LEN) route it may be necessary to filter out some
senders of multicast.  The following is a suggestion for how that might
look, notice the omitted `source` argument:

    mroute from eth0 except 192.168.1.0/24 group 225.1.2.0/24 to eth1 eth2

Filtering multiple sources:

    mroute from eth0 except 192.168.1.0/24,192.168.2.3 group 225.1.2.0/24 to eth1 eth2

This is sometimes also referred to as Administrative Scoping (RFC2365).


Basic support for IGMP/MLD proxying
-----------------------------------

In some setups a semi-dynamic behavior is required, but the only
signaling available is IGMP/MLD.  There exist tools like [igmpproxy][]
and [mcproxy][] for this purpose, which do a great job, but why should
you need to go elsewhere for your basic multicast routing needs?

The idea itself is simple, listen for IGMP/MLD join/leave messages on
enabled interfaces and add/remove routes dynamically from an `upstream`
marked interface.

Possibly an `igmp` flag may be needed as well, for downstream interfaces
we should proxy for.  Resulting `smcroute.conf` may then look like this:

    phyint eth0 upstream
    phyint eth1 igmp

**Note:** the IGMP/MLD signaling may also need to be "proxied" to the
  `upstream` interface, although this could be an optional second step
  enabled by also setting the `igmp` flag on that `upstream` interface.

For more information, see the above mentioned tools and [RFC4605][],
which details exactly this use-case.


[igmpproxy]: https://github.com/pali/igmpproxy
[mcproxy]:   https://github.com/mcproxy/mcproxy
[RFC4605]:   https://www.ietf.org/rfc/rfc4605.txt
