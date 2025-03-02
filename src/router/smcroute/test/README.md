Module Tests
============

The following tests verify fundamental functionality of SMCRoute when
`configure --enable-test`.  Required tools to be installed and available
in `$PATH`:

  - `ip` and `bridge` (iproute2 package, not the BusyBox variants)
  - `iptables`, for the 1:1 NAT test
  - `ping`
  - `tshark` (because `tcpdump -w foo.pcap` doesn't work in an unshare)
  - `valgrind`, for the memleak test

> **Note:* one of the tests makes use of `iptables`, which may not work
> in an `unshare(1)`, unless you have v1.8.7 or newer that supports the
> `XTABLES_LOCKFILE` environment variable.  As a workaround you can
> `chmod a+rw /var/run/xtables.lock`.
>
> Also, the GRE test requires your system to have the `ip_gre.ko` kernel
> module loaded.  If not, the test will be skipped.


Running
-------

To run the tests:

    ~$ sudo modprobe ip_gre      # if you have sudo capabilities
    ~$ cd src/smcroute
    ~/src/smcroute$ ./autogen.sh
    ~/src/smcroute$ ./configure --enable-test --enable-mrdisc
    ~/src/smcroute$ make -j9
    ~/src/smcroute$ make check

Each unit test is standalone.  To manually run select tests:

    ~/src/smcroute$ cd test/
    ~/src/smcroute/test$ unshare -mrun ./testname.sh

The tools `ping` and `tshark` are used to create and listen to multicast
streams "routed by" SMCRoute.

> **Note:** these tests must be run in sequence, not in parallel,
>   because they use the same interface names *and*, most importantly,
>   we may run on a kernel w/o multicast policy routing support!

[1]: https://github.com/libnet/nemesis
[2]: https://github.com/troglobit/smcroute/actions/workflows/build.yml


Topologies
----------

The following test topologies are employed to verify different aspects
and use-cases supported by SMCRoute.

### Basic

Interfaces `a1` and `a2` are Linux dummy type interfaces.

                        SMCRoute
                 .------ router -----.
                /                     \
    MC -----> a1                       a2 ------> MC


### Basic Plus

Same as Basic, but with more inbound/output interfaces, useful for
testing wildcard interface matching.

                        SMCRoute
                 .====== router =====.
                ////               \\\\
     MC ----> a1///                 \\\b1 ------> MC 
     MC ----> a2//                   \\b2 ------> MC 
     MC ----> a3/                     \b3 ------> MC 
     MC ----> a4                       b4 ------> MC 


### Basic w/ VLANs

Interfaces `a1` and `a2` are Linux dummy type interfaces with VLAN
interfaces created on top.  The topology sets up two VLAN interfaces
per dummy interface, VID 100 and 110.

                            SMCRoute
                  .------.== router ==.------.
                 /      /              \      \
    MC --> a1.100    a1.110          a2.100  a2.110 --> MC
                 \  /                    \  /
                  a1                      a2


### Bridged w/ VLANs

Two VETH pairs (a1:b1 and a2:b2) are attached to a bridge with VLAN
filtering enabled.  On top of the bridge two VLAN interfaces are
created on which routing takes place.

                       SMCRoute
                    .-- router --.
                   /              \
                 vlan1         vlan2
                      \       /
                       bridge0
    MC -----> a1       /     \        a2 -----> MC
               '------'       '------'

Both bridge ports, `a1` and `a2`, are untagged members of each VLAN.

> **Note:** interface `a1` and `vlan1` are in the same VLAN (VID 1), and
>           interface `a2` and `vlan2` are in the same VLAN (VID 2).


### Isolated Endpoints Bridged w/ VLANs

Like the default bridge, but each endpoint is in an isolated network
namespace.  This allows setting both IPv4 & IPv6 addresses on all
interfaces and using ping6 as emitter instead of requiring [nemesis][1].

                            SMCRoute
                         .-- router --.
                        /              \
      netns: left     vlan1         vlan2    netns: right
     .-------------.       \       /        .-------------.
     |             |        bridge0         |             |
     | MC --> eth0 |        /     \         | eth0 --> MC |
     |            `--------'       '---------'            |
     '-------------'                        '-------------'

Both bridge ports, `a1` and `a2`, are untagged members of each VLAN.

> **Note:** interface `a1` and `vlan1` are in the same VLAN (VID 1), and
>           interface `a2` and `vlan2` are in the same VLAN (VID 2).


### Isolated

Similar to Basic, but with two VETH pairs with the outer end of each in
an isolated network namespace.  Purpose is to emulate true end devices.

                            SMCRoute
     netns: left         .-- router --.        netns: right
    .-----------.       /              \      .-----------.
    |           |     b1                b2    |           |
    | MC --> a1-|-----'                  `----|-a2 --> MC |
    |           |                             |           |
    '-----------'     VETH pairs: aN//brN     '-----------'
                        In netns: eth0


### Multi Domain

This topology is intended to be used for testing multiple routers.  The
bridge in the shared segment is only used to connect the two VETH pairs.


          netns: left                       netns: right
         .-------------.                   .-------------.
         |  smcrouted  |                   |  smcrouted  |
         |    /   \    |       br0         |    /   \    |
    MC --> eth1   eth0 |      /   \        | eth0   eth1 <-- MC
         |            `------'     '-------'             |
         '-------------'  192.168.0.0/24   '-------------'
           10.0.0.0/24                       10.0.0.0/24

The idea is to provide a very tricky, but also very common, use case;
replicated setups with the same IP subnet forwarding multicast to a
shared LAN.  Commonly worked around using 1:1 NAT, or IP masquerading.
Neither of which is really best friends with IP multicast routing.


Tests
-----

### Advanced Routing

The test use ten ingressing multicast streams, starting at 225.1.2.3.
With a single initial SSM route matching one of the streams.  The
remaining nine are installed in the kernel MFC with a stop-filter.

 - Verify that the SSM route works
 - Verify that no other multicast stream is forwarded

Now, add an ASM route for 225.1.2.3/29 and verify that the new ASM route
matches a subset of the installed stop-filters, changing them to proper
SSM routes.

Remove the 225.1.2.3/29 route and verify that the filters are changed
back to stop-filters, and that the first SSM stream (which is in the
same range) is not affected.

**Topology:** Basic


### Basic Routing

Verifies routing between two interfaces a1 and a2.  Multicast is
injected on a1 and tcpdump verifies function on a2.

**Topology:** Basic

												 
### Bridge VLANs

Slightly more advanced test case, a bridge with two VLAN interfaces on
top and two VETH pairs acting as untagged ports to the bridge in each of
the VLANs.  The other end of each VETH pair is placed in an isolated
network namespace to allow proper IP networking to be set up.

**Topology:** Isolated Bridged w/ VLANs


### Dynamic Routes

Different combinations of `(*,G/LEN)` to `(S/LEN, G)` routing is tested
here for both IPv4 and IPv6.

**Topology:** Basic


### Expiration of Dynamic Routes

The command line option `smcrouted -c SEC` can be used to tweak the
cache timeout for `(*,G)` routes.  This test verifies that it works
as intended for both IPv4 and IPv6.

**Topology:** Basic


### Include Files

This test does not traffic or forwarding verification, it only tests
that the `include` directive for `smcroute.conf` works.

**Topology:** Basic


### IPv6 (S,G) and (*,G) Forwarding

Similar to the Bridged VLAN test, only with IPv6.

**Topology:** Basic


### Isolated (*,G) Forwarding

This test is currently very similar to the Basic test, but can easily be
extended with IPv6 support as well.  The trick here is to use the nested
network namespace support, introduced in the new Isolated topology.

The Isolated topology allows setting interface addresses, both IPv4 and
IPv6 (!), regardless of the environment (and as long as the underlying
Linux kernel supports it).  This means a standard tool like `ping` can
be used to send multicast.  Lowering the barrier of entry to run tests.

**Topology:** Isolated


### Join/Leave ASM/SSM

Verify ASM & SSM join and leave for IPv4 & IPv6.  Since ASM and SSM
cannot be mixed on the same interface (fallback to ASM occurs), we
use different interfaces and verify operation by inspecting the Linux
`ip maddr` and `/proc/net/mcfilter` output.

**Topology:** Basic


### Join/Leave ASM/SSM with Prefix Length

Verify ASM & SSM join and leave for IPv4 & IPv6 in various prefix length
combinations; `(S/LEN, G)`, `(S, G/LEN)`, and `(S/LEN, G/LEN)`.  Since
ASM and SSM cannot be mixed on the same interface (fallback to ASM
occurs), we use different interfaces and verify operation by inspecting
the Linux `ip maddr` and `/proc/net/mcfilter` output.

**Topology:** Basic


### Multiple Routers with 1:1 NAT

Verify multicast forwarding from two separate LANs to a shared segment,
where the two separate LANs have the same IP subnet.  To make things
worse, the same IP source address will be used for both multicast
senders.

Note, this test requires the host system also has `iptables`.  If the
test cannot find the `iptables` tool, it will `exit 77` to trigger a
SKIP in the test suite.

**Topology:** Multi Domain


### Poison Pill Routes

Verifies `(*,G/LEN)` routes from a set of approved inbound interfaces
are properly installed in the kernel MFC.  While blocking traffic to the
same groups from other inbound interfaces using stop-filter, or "poison
pill", routes (no outbound interfaces).  For details, see issue #143.

Note, this test assumed the origin `S` of inbound traffic differs
between inbound interfaces.  I.e., the same `S` on different inbound
interfaces is not supported by `smcrouted`.

**Topology:** Multi


### Reload .conf File (IPv4)

Verifies that reloading the .conf file using `SIGHUP` or `reload`
command to `smcroutectl` does not disturb established flows, and
that the resulting configuration is properly set in the kernel.

**Topology:** Multi


### Reload .conf File (IPv6)

The same as the IPv4 test, but for IPv6.

**Topology:** Multi


### VLAN Interfaces

Similar to the basic routing test, except VLAN interfaces are created on
top of the base interfaces, and routing takes place there.  This test is
based on [troglobit/smcroute#161][issue-161].

**Topology:** Basic w/ VLANs


[issue-161]: https://github.com/troglobit/smcroute/issues/161
