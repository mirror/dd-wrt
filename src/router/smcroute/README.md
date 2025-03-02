SMCRoute - A static multicast routing daemon
============================================
[![License Badge][]][License] [![GitHub Status][]][GitHub] [![Coverity Status][]][Coverity Scan]

Table of Contents
-----------------
* [About](#about)
* [Features](#features)
* [Usage](#usage)
  * [Caveat](#caveat)
  * [Actions Scripts](#action-scripts)
  * [Many Interfaces](#many-interfaces)
  * [Multiple Routing Tables](#multiple-routing-tables)
  * [Client Tool](#client-tool)
* [Wildcard Routes](#wildcard-routes)
* [Multicast Router Discovery](#multicast-router-discovery)
* [Build & Install](#build--install)
  * [Linux Requirements](#linux-requirements)
  * [*BSD Requirements](#bsd-requirements)
  * [General Requirements](#general-requirements)
  * [Configure & Build](#configure--build)
  * [Integration with systemd](#integration-with-systemd)
  * [Static Build](#static-build)
  * [Building from GIT](#building-from-git)
* [Origin & References](#origin--references)


About
-----

SMCRoute is a static multicast routing daemon providing fine grained
control over the multicast forwarding cache (MFC) in the UNIX kernel.
Both IPv4 and IPv6 are fully supported.

SMCRoute can be used as an alternative to dynamic multicast routers like
[mrouted][], [pimd][], or [pim6sd][] in setups where static multicast
routes should be maintained and/or no proper IGMP or MLD signaling
exists.

Multicast routes exist in the UNIX kernel as long as a multicast routing
daemon runs.  On Linux, multiple multicast routers can run simultaneously
using different multicast routing tables.

The full documentation of SMCRoute is available in the manual pages, see
[smcrouted(8)][], [smcroutectl(8)][], and [smcroute.conf(5)][].


Features
--------

All features, except [mrdisc][], are supported for both IPv4 and IPv6.
Please note, some features may not be available on systems other than
Linux.  E.g., FreeBSD does not have SSM group join support.

  - Configuration file support, `/etc/smcroute.conf`
  - Configuration snippet include support, `/etc/smcroute.d/*.conf`
  - Daemon startup options support, `/etc/default/smcroute`
  - Support for seamless reloading of the configuration on `SIGHUP`
  - Source-less on-demand routing, a.k.a. wildcard `(*,G)` based static
    routing, including support for `(*,G/LEN)` and `(S/LEN,G/LEN)`
  - Optional built-in [mrdisc][] support for IPv4, [RFC4286][]
  - Support for multiple routing tables on Linux
  - Client to add/remove routes, join/leave groups, and built-in support
    to show both routes and joined groups
  - Interface wildcard matching, `eth+` matches `eth0, eth15`

> **Note:** `smcroutectl` can be used to freely modify the runtime state
>            of `smcrouted`, but any changes made (routes/groups) are
>            lost when the configuration is reloaded.  This is by design.


Usage
-----

    smcrouted [-nNhsv] [-c SEC] [-d SEC] [-e CMD] [-f FILE] [-i NAME]
	          [-l LVL] [-p USER:GROUP] [-P FILE] [-t ID] [-u FILE]
    
    smcroutectl [-dptv] [-i NAME] [-u FILE] [COMMAND]
    smcroutectl ⟨kill | reload⟩
    smcroutectl ⟨add  | rem⟩    ⟨ROUTE⟩
    smcroutectl ⟨join | leave⟩  ⟨GROUP⟩
    smcroutectl  show [ routes | groups]

To set multicast routes and join groups you must first start the daemon,
which needs *root privileges*, or `CAP_NET_ADMIN`.  Use `smcrouted -n`
to run the daemon in the foreground, as required by modern init daemons
like systemd and [Finit][].

When started from systemd, `smcrouted` runs with the `-n -s` options,
i.e. supervised in the foreground and uses syslog for logging output.
The default log level is `INFO`, this can be adjusted using the file
`/etc/default/smcroute`:

    SMCROUTED_OPTS=-l debug

When configured with `--sysconfdir=/etc`, like most Linux distributions
do, `smcrouted` reads `/etc/smcroute.conf`, which can look something
like this:

    mgroup from eth0 group 225.1.2.3
    mgroup from eth0 group 225.1.2.3 source 192.168.1.42
    mroute from eth0 group 225.1.2.3 source 192.168.1.42 to eth1 eth2

The first line means "Join multicast group 225.1.2.3 on interface eth0".
Useful if `eth0` is not directly connected to the source, but to a LAN
with switches with IGMP snooping.  Joining the group opens up multicast
for that group towards `eth0`.  See below Caveat for limitations.

The second `mgroup` is for source specific group join, i.e. the host
specifies that it wants packets from 192.168.1.42 and no other source.

The third `mroute` line is the actual layer-3 routing entry.  Here we
say that multicast data originating from 192.168.1.42 on `eth0` to the
multicast group 225.1.2.3 should be forwarded to interfaces `eth1` and
`eth2`.

**Note:** To test the above you can use ping from another device.  The
   multicast should be visible as long as your IP# matches the source
   above and you ping 225.1.2.3 -- **REMEMBER TO SET THE TTL >1**

    ping -I eth0 -t 2 225.1.2.3

The TTL is what usually bites people first trying out multicast routing.
Most TCP/IP stacks default to a TTL of 1 for multicast frames, e.g. ping
above requires `-t 2`, or greater.  This limitation is intentional and
reduces the risk of someone accidentally flooding multicast.  Remember,
multicast *behaves like broadcast* unless limited.

The TTL should preferably be set on the sender side, e.g. the camera,
but can also be modified in the firewall on a router.  Be careful though
because the TTL is the only thing that helps prevent routing loops!  On
Linux the following `iptables` command can be used to change the TTL:

    iptables -t mangle -A PREROUTING -i eth0 -d 225.1.2.3 -j TTL --ttl-inc 1

Some commands, like this one, must usually be run with root privileges
or the correct set of capabilities.

### Caveat

On some platforms there is a limit of 20 groups per socket.  This stems
from a limit in BSD UNIX, which also affects Linux.  The setting that
controls this is `IP_MAX_MEMBERSHIPTS`, defined in the system header
file `netinet/in.h`.  Linux users can tweak this with the following
`/proc` setting:

    echo 40 > /proc/sys/net/ipv4/igmp_max_memberships

`smcrouted` probes this at runtime by attempting to join as many groups
as possible (as have been requested), when the kernel accepts no further
joins on a socket, `smcrouted` opens a new one.

For large setups it is recommended to investigate enabling multicast
router ports in the switches, either statically or by enabling support
for multicast router discovery, RFC 4286, or possibly use a dynamic
multicast routing protocol.


### Action Scripts

    smcrouted -e /path/to/script

With `-e CMD` a user script or command can be called when `smcrouted`
receives a `SIGHUP` or installs a multicast route to the kernel.  This
is useful if you, for instance, also run a NAT firewall and need to
flush connection tracking after installing a multicast route.


### Many Interfaces

    smcrouted -N

With the `-N` command line option SMCRoute does *not* prepare all system
interfaces for multicast routing.  Very useful if your system has a lot
of interfaces but only a select few are required for multicast routing.
Use the following in `/etc/smcroute.conf` to enable interfaces:

    phyint eth0 enable
    phyint eth1 enable
    phyint eth2 enable

It is possible to use any interface that supports the `MULTICAST` flag.

Note, however, that depending on the UNIX kernel in use, you may have to
have an interface address set, in the relevant address family, and the
interface may likely also have to be `UP`.


### Multiple Routing Tables

On Linux it is possible to run multiple multicast routing daemons due to
its support for multiple multicast routing tables.  In such setups it
may be useful to change the default identity of SMCRoute:

    smcrouted -i mrt1 -t 1
    smcrouted -i mrt2 -t 2

The `-i NAME` option alters the default syslog name, config file, PID
file, and client socket file name used.  In the first instance above,
`smcrouted` will use:

- `/etc/mrt1.conf`
- `/var/run/mrt1.pid`
- `/var/run/mrt1.sock`

and syslog messages will use the `mrt1` identity as well.  Remember to
use the same `-i NAME` also to `smcroutectl`.


### Client Tool

SMCRoute also has a client interface to interact with the daemon:

    smcroutectl join eth0 225.1.2.3
    smcroutectl add  eth0 192.168.1.42 225.1.2.3 eth1 eth2

If the daemon runs with a different identity the client needs to be
called using the same identity:

    smcrouted   -i mrt
    smcroutectl -i mrt show

There are more commands.  See the man page or the online help for
details:

    smcroutectl help

> **Note:** Root privileges are required by default for `smcroutectl` due
> to the IPC socket permissions.


Wildcard Routes
---------------

Multicast often originates from different sources but usually not at the
same time.  For a more generic setup, and to reduce the number of rules
required, it is possible to set `(*,G)` multicast routes for both IPv4
and IPv6.  Variants include `(*,G/LEN)` and `(S/LEN,G/LEN`.  These
wildcard routes are used as "templates" to match against and install
proper `(S,G)` routes when the kernel informs `smcrouted` of inbound
multicast from new sources.

Example `smcroute.conf`:

    phyint eth0 enable mrdisc
    phyint eth1 enable
    phyint eth2 enable
    
    mgroup from eth0 group 225.1.2.3
    mroute from eth0 group 225.1.2.3 to eth1 eth2

or, from the command line:

    # smcroutectl join eth0 225.1.2.3
    # smcroutectl add  eth0 225.1.2.3 eth1 eth2

Also, see the `smcrouted -c SEC` option for periodic flushing of learned
`(*,G)` rules, including the automatic blocking of unknown multicast, and
the `smcroutectl flush` command.


Multicast Router Discovery
--------------------------

Another interesting feature is multicast router discovery, [mrdisc][],
described in [RFC4286][].  This feature is disabled by default, enable
with `configure --enable-mrdisc`.  When enabled it periodically sends
out an IGMP message on inbound interfaces¹ to alert switches to open up
multicast in that direction.  Not many managed switches have support for
this yet.

> **Note:** [mrdisc][] only works on Linux due to `SO_BINDTODEVICE`.

____  
¹ Notice the `mrdisc` flag to the above `phyint eth0` directive, which
is missing for `eth1` and `eth2`.


Build & Install
---------------

SMCRoute should in theory work on any UNIX like operating system which
supports the BSD MROUTING API.  Both Linux and FreeBSD are tested on a
regular basis.

### Linux Requirements

On Linux the following kernel config is required:

    CONFIG_IP_MROUTE=y
    CONFIG_IP_PIMSM_V1=y
    CONFIG_IP_PIMSM_V2=y
    CONFIG_IP_MROUTE_MULTIPLE_TABLES=y       # For multiple routing tables
    CONFIG_IPV6_MROUTE_MULTIPLE_TABLES=y     # For multiple routing tables

### *BSD Requirements

On *BSD the following kernel config is required:

    options    MROUTING    # Multicast routing
    options    PIM         # pimd extensions used for (*,G) support

FreeBSD support module loading, `kldload(8)`, edit `/boot/loader.conf`:

    ip_mroute_load="yes"
    ip_mroute6_load="yes"

### General Requirements

Check the list of multicast capable interfaces:

    cat /proc/net/dev_mcast

or look for interfaces with the `MULTICAST` flag in the output from:

    ifconfig

Some interfaces have the `MULTICAST` flag disabled by default, like `lo`
and `greN`.  Usually this flag can be enabled administratively.

### Configure & Build

The GNU Configure & Build system use `/usr/local` as the default install
prefix.  In many cases this is useful, but this means the configuration
files, cache, and PID files will also use that prefix.  Most users have
come to expect those files in `/etc/` and `/var/` and configure has a
few useful options that are recommended to use.  For SMCRoute you may
want to use something like this:

    ./configure --prefix=/usr --sysconfdir=/etc --runstatedir=/var/run
    make -j5
    sudo make install-strip

Usually your system reserves `/usr` for native pacakges, so most users
drop `--prefix`, installing to `/usr/local`, or use `--prefix=/opt`.

**Note:** On some systems `--runstatedir` may not be available in the
  configure script, try `--localstatedir=/var` instead.


### Privilege Separation

As of SMCRoute v2.2 support for privilege separation using the `libcap`
library was added.  It is used to drop full root privileges at startup,
retaining only `CAP_NET_ADMIN` for managing the multicast routes.

The build system searches for the `libcap` library and header file(s).
Both `libcap-dev` and `pkg-config` are required.

**Note:** Although support is automatically detected, the build system
          will issue a warning if `libcap` is missing.  This can be
          silenced with `configure --without-libcap`

### Integration with systemd

For systemd integration `libsystemd-dev` and `pkg-config` are required.
When the unit file is installed, `systemctl` can be used to enable and
start `smcrouted`:

    $ sudo systemctl enable smcroute.service
    $ sudo systemctl start smcroute.service

Check that it started properly by inspecting the system log, or:

    $ sudo systemctl status smcroute.service

### Static Build

Some people want to build statically, to do this with `autoconf` add the
following `LDFLAGS=` *after* the configure script.  You may also need to
add `LIBS=...`, which will depend on your particular system:

    ./configure LDFLAGS="-static" ...

### Building from GIT

The `configure` script and the `Makefile.in` files are generated and not
stored in GIT.  So if you checkout the sources from GitHub you first
need to generated these files using `./autogen.sh`.


Origin & References
-------------------

SMCRoute is maintained collaboratively at [GitHub][Home].  Bug reports,
feature requests, patches/pull requests, and documentation fixes are
most welcome.  The project was previously hosted and maintained by
Debian at [Alioth][] and before that by [Carsten Schill][], the original
author.


[smcrouted(8)]:    https://man.troglobit.com/man8/smcrouted.8.html
[smcroutectl(8)]:  https://man.troglobit.com/man8/smcroutectl.8.html
[smcroute.conf(5)]:https://man.troglobit.com/man5/smcroute.conf.5.html
[Finit]:           https://github.com/troglobit/finit
[mrouted]:         https://github.com/troglobit/mrouted
[pimd]:            https://github.com/troglobit/pimd
[pim6sd]:          https://github.com/troglobit/pim6sd
[mrdisc]:          https://github.com/troglobit/mrdisc
[RFC4286]:         https://tools.ietf.org/html/rfc4286
[Home]:            https://github.com/troglobit/smcroute
[Alioth]:          https://alioth.debian.org/projects/smcroute
[Carsten Schill]:  http://www.cschill.de/smcroute/
[License]:         https://en.wikipedia.org/wiki/GPL_license
[License Badge]:   https://img.shields.io/badge/License-GPL%20v2-blue.svg
[GitHub]:          https://github.com/troglobit/smcroute/actions/workflows/build.yml/
[GitHub Status]:   https://github.com/troglobit/smcroute/actions/workflows/build.yml/badge.svg
[Coverity Scan]:   https://scan.coverity.com/projects/3061
[Coverity Status]: https://scan.coverity.com/projects/3061/badge.svg
