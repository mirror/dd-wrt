ChangeLog
=========

All notable changes to the project are documented in this file.


[v2.5.7][] - 2024-05-09
-----------------------

### Fixes
- Fix #207: crash when adding IPv6 multicast route on a kernel without
  IPv6 multicast support


[v2.5.6][] - 2022-11-28
-----------------------

Despite the new `smcroutectl` batch mode feature, this is primarily a
bug fix release.  Most notably #183 and #187.

### Changes
- Add `smcroutectl` batch support, issue #189.  Based on the IPC support
  added in issue #185, by Alexey Smirnov:

        ~$ sudo smcroutectl -b <<-EOF
               join eth0 225.1.2.3
               add eth0 192.168.1.42 225.1.2.3 eth1 eth2
               rem eth1 225.3.4.5 eth3
               leave eth1 225.3.4.5
               EOF
        ~$

### Fixes
- Fix #178: invalid systemd daemon type Simple/Notify vs simple/notify
- Fix #179: typo in wildcard routes section of README
- Fix #180: minor typo in file and directory names in documentation
- Fix #183: casting in IPC code hides error handling of `recv()`
- Fix #186: NULL pointer dereference in `utimensat()` replacement
  function.  Found accidentally by Alexey Smirnov.  Only triggered on
  systems that don't have a native `utimensat()` in their C-library, or
  if you try to build SMCRoute without using its own build system ...
- Fix #187: strange behavior joining/leaving the same group
- Fix #192: typo in README


[v2.5.5][] - 2021-11-21
-----------------------

### Changes
- Revert extraction of version from GIT tag.  Incompatible with systems
  that do `autoreconf` on a dist. tarball

### Fixes
- Fix #175: Parse error in `/etc/smcroute.conf`.  SMCRoute fails to
  start on interfaces with `mrdisc` disabled, when built with mrdisc
  support and `-N` passed on command line


[v2.5.4][] - 2021-11-13
-----------------------

### Changes
- Automatically extract new version from GIT tag

### Fixes
- Avoid trying to delete inactive VIFs.  Fixing an annoying bogus error:
  *"Failed deleting VIF for iface lo: Resource temporarily unavailable"*
- Fix #171: too small string buffer for IPv6 address causing garbled
  output in periodic expiry callback
- Fix too small buffer for IPv6 address in mroute display functions


[v2.5.3][] - 2021-09-23
-----------------------

### Changes
- New tests to verify add/del of IPv4/IPv6 routes in kernel MFC

### Fixes
- Fix #166: build warning with gcc 10.2.1: "comparison is always true
  due to limited range of data type"
- Fix build warning with `--disable-mrdisc` configure option
- Fix #167: cannot remove routes added with `smcroutectl add`,
  only affects add/del at runtime with smcroutectl, not .conf reload
- Fix #168: build problem on Debian/kFreeBSD, used wrong queue.h


[v2.5.2][] - 2021-08-27
-----------------------

### Changes
- Allow installing routes with no outbound interfaces
- Reinitialize VIFs on reload in case of new interfaces
- Handle cases when interfaces change ifindex, i.e. they've first been
  removed and then re-added with the same name

### Fixes
- Fix VIF leak when deleting interfaces with MRDISC enabled
- Fix handling when an (S,G) moves to another IIF.  This fixes issues
  where the SMCRoute kernel cache was out of sync with the kernel MFC
- Fix handling of lost/disabled interfaces at reload.  This fixes a
  couple of issues where routes were not updated properly at runtime
- Update interface flags on reload, this fixes issues when SMCRoute
  failed to detect interfaces that had their MULTICAST flag set or
  cleared at runtime
. Skip `setsockopt()` for IPC sockets.  This fixes warnings in syslog
  about failing to disable `MULTICAST_LOOP` and `MULTICAST_ALL`


[v2.5.1][] - 2021-08-22
-----------------------

### Changes
- Add .sha256 checksum to dist files, useful for packagers

### Fixes
- Fix #155: systemd Notify integration, restore `NotifyAccess=main`
- Fix #165: ftbfs on older compilers, e.g. gcc 4.8.3, use -std=gnu99
- Fix Documentation refs in systemd unit file, new man pages


[v2.5.0][] - 2021-08-19
-----------------------

**Highlights:** native `/etc/smcroute.d/*.conf` support and seamless
  update/removal of routes and groups by reloading `.conf` files or
  using `smcroutectl`.

Tested on Linux 5.11 and FreeBSD 12.2

### Changes
- Fully automated test suite with 15 tests covering many use cases
- Support for `/etc/smcroute.d/*.conf`, issue #103
- Support for applying changes to `.conf` files without disturbing
  established flows -- i.e., seamless addition or removal of outbound
  interfaces in existing rules, or add/remove routes, without ever
  affecting other routes, issue #103
- Support for route replacement/update `smcroutectl`, issue #115
- Full `(*,G)` wildcard route matching, for IPv4 and IPv6, issue #31
- Variant wildcard route matching with source and group range matching.
  This may of course waste a lot of resources, so handle with care:
  - `(*,G/LEN)`, issue #135 (IPv4), and issue #162 (IPv6)
  - `(S/LEN,G)`, issue #81
  - `(S/LEN,G/LEN)`
- Full SSM/ASM group join support, for both IPv4 and IPv6.  Including
  joining group ranges from both `smcroutectl` and `.conf`, issue #118
  Please note, no SSM support on FreeBSD, only Linux
- New command line option, `-F file.conf` to verify file syntax, issue #100
- The `-I NAME` command line option has changed to `-i NAME`, compat
  support for the previous option remains
- The `mrdisc` flag to the `phyint` directive is now what solely
  controls the functionality per interface.  Previously a mechanism to
  enable/disable the functionality (if enabled) if active routes were in
  place.  However, this did not cover `(*,G)` routes so that has been
  removed to simplify and guarantee full function
- Output format from `smcroutectl` has been extensively changed. E.g,
  new `/LEN` support means wider columns, but heading have also changed
- The `.tar.xz` archive has been has been dropped as distribution format,
  keeping only `tar.gz`

### Fixes
- Fix #120: failed ASM/SSM IGMP join if interface has no address
- Fix #130: dynamic IPv6 routes are not flushed (like IPv4 `(*,G)`)
- Fix #149: `(*,G)` cache timeout callback stops, or never starts
- Fix #151: same log entries
- Fix #156: `smcruotectl show` does not show IPv6 routes
- Fix stochastic timer behavior, e.g. mrdisc announcements experienced
  interference with the `(*,G)` cache timer


[v2.4.4][] - 2019-02-11
-----------------------

### Changes
- Allow same outbound interface as inbound for routes, only warn user
- systemd unit file hardening, recommended by Debian
- Discontinued GPG signing, unused and signed with only one dev key

### Fixes
- Fix #104: IGMP header checksum missing from mrdisc frames
- Fix #105: Unblock *all* matching, and currently blocked, (S,G) to a
  newly installed (*,G) route, only the first know was unblocked
- Fix #106: Timer nanosecond bug causing loss of address refresh on DHCP
  interfaces.  Interface monitoring feature introduced in v2.4.3
- Fix #108: Calling init script with `stop` does not stop `smcrouted`
- Fix #109: ifindex in UNIX/POSIX is an interger, not unsigned short


[v2.4.3][] - 2018-11-06
-----------------------

The Lyon release.

### Changes
- Add `strlcat()` replacement from OpenBSD, use instead of `strcat()`
- `smcrouted` should never log to system console, proposed by Westermo

### Fixes
- `smcrouted` fails to join multicast groups on interfaces that do not
  yet have an IP address when `smcrouted` starts up, or when it receives
  `SIGHUP`, e.g. DHCP client interfaces.  This patch release adds a timer
  refresh of interface addresses that retries multicast group joins until
  an address is set.  This is similar to issue #55, but does not handle
  interfaces that do not exist yet
- Make sure Linux alias interfaces (baseif:num) are registered as
  baseif.  Westermo found that use of alias interfaces cause multiple
  VIFs to be registered for the same base interface causing multicast
  routes to use the wrong inbound or outbound VIF.  Alias interfaces
  use the same underlying physical interface so only one VIF needed
- Fix display of route counters and column alignment
- Minor spelling fixes, found by Debian
- Add missing status command to SysV init script, found by Debian
- Simplify `utimensat()` replacement, `AT_SYMLINK_NOFOLLOW` unused


[v2.4.2][] - 2018-09-09
-----------------------

### Changes
- Add wrapper script `smcroute` for use with old style startup scripts
- Add symlinks to man pages for `smcrouted.8 and` `smcroutectl.8`
- Update SysV init script, daemon now called `smcrouted`

### Fixes
- Fix #96: A `.conf` line may be missing final newline, this is fine
- Spellcheck `smcroute.conf` example
- Fix Lintian warning (Debian) for unbreakable line in man page


[v2.4.1][] - 2018-06-16
-----------------------

### Changes
- Update and spellecheck documentation and example `.conf` file

### Fixes
- Fix #91: Allow re-configuration of unprivileged `smcrouted`.   
  Courtesy of Marcel Patzlaff


[v2.4.0][] - 2018-02-11
-----------------------

### Changes
- Interface wildcard support, Linux `iptables` like syntax, `eth+`
  matches `eth0`, `eth1`, `eth32`.  It can be used where an interface
  name is used: `phyint`, `mroute`, `mgroup`, and even on the command
  line to `smcroutectl`.  Contributed by Martin Buck
- Disable IPv4 [mrdisc][] by default, enable per `phyint` in the `.conf`
  file instead.  When *not* started with `smcrouted -N` mrdisc would
  otherwise be enabled on *all* interfaces found at startup
- Minor doc updates, e.g. clarify need for root or `CAP_NET_ADMIN`
  including some minor man page fixes

### Fixes
- Fix #75: Not possible to remove (*,G) routes using `smcroutectl`
- Fix #76: When removing a kernel route, also remove from internal lists
  otherwise route is shown in `smcroutectl show`.  Conversely, adding a
  route to internal list shall only be done after successful kernel add
- Fix #77: Counter overflow due to wrong type used in `smcroutectl show`
- Fix #78: Document interface wildcard feature
- Fix #80: `smcroutectl` argument parser fixes by Pawel Rozlach
- Fix #84: Check return value of `sigaction()`
- Fix #85: Signal handling is async-signal-unsafe
- Fix #86: Document how to use `iptables` on Linux to modify TTL
- Fix #87: Possible buffer overrun in `ipc_receive()`
- Fix #89: Adding similar (S,G) route should replace existing one if
  inbound interface differs


[v2.3.1][] - 2017-06-13
-----------------------

Bug fix release courtesy of the Westermo WeOS automated testing
framework.  Many thanks to Johan Askerin at Westermo for working
on integrating SMCRoute v2.3 into WeOS v4.22!

### Changes
- Add `utimensat()` replacement for systems that don't have it
- Ignore error messages from `send()` on interface link down

### Fixes
- Fix build error(s) on FreeBSD 9/9.3-RELEASE
- Fix possible invalid interface name reference in new mrdisc support
- Fix log macro bug in the .conf parser
- Fix buggy interface and VIF re-initialization on `SIGHUP`


[v2.3.0][] - 2017-05-28
-----------------------

### Changes
- Support GROUP/LEN matching for IPv4 (*,G) routes
- Support for IPv4 [mrdisc][], [RFC4286][]
- Support for multiple routing tables on Linux, `-t ID`
- `ssmgroup` code folded into general code, now with optional source
- Separation of daemon and client into `smcrouted` and `smcroutectl`
  - Complete new client user interface, `smcroutectl`
  - Support for disabling IPC and client, `--disable-client`
  - Support for disabling `.conf` file support, `--disable-config`
- Show multicast routes and joined groups in client, including stats:
  `smcroutectl show [groups|routes]`
- Support for `-d SEC` startup delay in `smcrouted`
- Unknown (*,G) multicast now blocked by default
- Flush timer, `-c SEC`, for (*,G) routes now enabled by default, 60 sec
- Build ID removed from `configure` script
- Massive code cleanup, refactor and separation into stand-alone modules
- Default system paths are no longer taken from `/usr/include/paths.h`,
  instead the settings from `configure --prefix` are used
- Use of `libcap` for privilige separation is now auto-detected

### Fixes
- Allow use of loopback interface for multicast routes
- Fix IPv4-only build, by Martin Buck
- Fix IPv4 network interface address identification, by Martin Buck
- Support unlimited number of network interfaces, by Martin Buck


[v2.2.2][] - 2017-02-02
-----------------------

### Changes
- New client command, `-F`, for immediately flushing dynamically learned
  (*,G) routes from the cache.

### Fixes
- Fix issue #51: New cache flush timeout option causes endless
  `select()` loop.  Reported by Ramon Fried, @mellowcandle


[v2.2.1][] - 2017-01-09
-----------------------

### Changes
- Add support for a new command line option, `-c SEC`, for timing out
  dynamically learned (*,G) routes.  Issue #17

### Fixes
- Portability, replace use of non-std `__progname` with small function
- Issue #49: systemd unit file missing `-d` to start daemon


[v2.2.0][] - 2016-12-03
-----------------------

### Changes
- Support for dropping root privileges after opening the multicast
  routing socket and creating the PID file
- Support for Source Specific Multicast group subscription (only IPv4)
- Support for systemd, service file included and installed by default

### Fixes
- Remove GNUisms to be able to build and run on Alpine Linux (musl libc)
- Add OpenBSD `queue.h` for systems that do not have any *BSD `sys/queue.h`
- Coding style cleanup and minor refactor


[v2.1.1][] - 2016-08-19
-----------------------

### Changes
- When `SIGHUP` is received SMCRoute now touches its PID file as an
  acknowledgement.  This is used by some process supervision daemons,
  like [Finit](https://github.com/troglobit/finit), on system
  configuration changes to detect when a daemon is done.  The mtime is
  set using the `utimensat()` function to ensure nanosecond resolution.

### Fixes
- Fix issue #38: Minor memory leak at exit.  The Valgrind tool warns
  that all memory is not freed when smcroute exits.  On most modern
  UNIX systems, on platforms with MMU, this is not a problem, but on
  older systems, or uClinux, memory is not freed at program exit.
- Fix issue #39: Removing wildcard route at runtime does not work if no
  kernel routes have been set.
- Fix issue #44: IPv6 disabled by default, despite what `configure` says
  in its help text.  Enabling it disables it ... fixed by enabling IPv6
  by default.


[v2.1.0][] - 2016-02-17
-----------------------

### Changes
- Allow more interfaces to be used for multicast routing, in particular
  on Linux, where interfaces without an IP address can now be used!
  Making it possible to run SMCRoute on DHCP/PPP interaces, issue #13
- Add support for TTL scoping on interfaces, very useful for filtering
  multicast without a firewall: `phyint IFNAME ttl-threshold TTL`
- On Linux a socket filter now filters out ALL traffic on the helper
  sockets where SMCRoute does IGMP/MLD join/leave on multicast groups.
  This should eliminate the extra overhad required to, not only route
  streams, but also send a copy of each packet to SMCRoute.
- Add support for limiting the amount of multicast interfaces (VIFs)
  SMCRoute creates at startup.  Two options are now available, by
  default all multicast capable interfaces are given a VIF and the user
  can selectively disable them one by one.  However, if the `-N` command
  line option is given SMCRoute does *not* enable any VIFs by default,
  the user must then selectively enable interface one by one.  The
  syntax in the config file is:

        phyint IFNAME <enable|disable>

  Use `enable` per interface with `-N` option, or `disable` by default.

- Make build ID optional.  SMCRoute has always had the build date
  hard coded in the binary.  This change makes this optional, and
  defaults to disabled, to facilitate reproducible builds.  For
  more info, see https://wiki.debian.org/ReproducibleBuilds
- Remove generated files from GIT.  Files generated by GNU autotools are
  now only part of the distribution archive, not the GIT repository.
  Use `./autogen.sh` to create the required files when using GIT.
- Updated man page and example `smcroute.conf` with limitations on
  the amount of mgroup rules.
- Add support for executing an external script on config reload and when
  installing a multicast route.  Issue #14

        smcroute -e /path/to/cmd

  The script is called when SMCRoute has started up, or has received
  `SIGHUP` and just reloaded the configuration file, and when a new
  source-less rule have been installed.  See the documentation for
  more information on set environment variables etc.  Issue #14
- Add `--disable-ipv6` option to `configure` script.  Disables IPv6
  support in SMCRoute even though the kernel may support it
- Replaced `-D` option with `-L LVL` to alter log level, issue #24
- The smcroute daemon now behaves more like a regular UNIX daemon.  It
  defaults to using syslog when running in the background and stderr
  when running in the foreground.  A new option `-s` can be used to
  enable syslog when running in the foreground, issue #25
- The smcroute client no longer use syslog, only stderr, issue #25
- When starting the smcroute daemon it is no longer possible to also
  send client commands on the same command line.
- Remove the (unmaintained) in-tree `mcsender` tool.  Both ping(8) and
  iperf(1) can be used in its stead.  The omping(8) tool is another
  tool, engineered specifically for testing multicast.  Issue #30

### Fixes
- Fix issue #10: `smcroute` client loops forever on command if no
  `smcroute` daemon is running
- Install binaries to `/usr/sbin` rather than `/usr/bin`, regression
  introduced in [v2.0.0][].  Fixed by Micha Lenk
- Cleanup fix for no-MMU systems.  Multicast groups were not properly
  cleaned up in the `atexit()` handler -- *only* affects no-MMU systems.
- Do not force automake v1.11, only require *at least* v.11
- SMCRoute operates fine without a config file, so use a less obtrusive
  warning message for missing `/etc/smcroute.conf`


[v2.0.0][] - 2014-09-30
-----------------------

### Changes
- Migrate to full GNU Configure and Build system, add Makefile.am,
  GitHub issue #6 -- heads up, packagers!
- Add standard SysV init script, from Debian. GitHub issue #9

### Fixes
- Multiple fixes of nasty bugs thanks to Coverity static code analysis!
- Cleanup of Linux system anachronisms to make FreeBSD work again,
  GitHub issue #5


[v1.99.2][] - 2013-07-16
------------------------

### Fixes
* Fix issue #2: Loop forever bug when deleting new (*,G) sourceless routes
  Bug report and patch by Jean-Baptiste Maillet


[v1.99.1][] - 2013-07-11
------------------------

### Fixes
- Fix possible memory leak on Linux
- Fix missing #ifdefs when building on systems w/o IPv6
- Fix possible race in Makefile when building in (massive) parallel
- Fix build problems on RedHat EL5/CentOS5, i.e., Linux <= 2.6.25


[v1.99.0][] - 2012-05-13
-------------------------

### Changes
- Feature: Experimental source-less `(*,G)` IPv4 multicast routing.
  Most UNIX kernels are (S,G) based, i.e., you need to supply the
  source address with the multicast group to setup a kernel routing
  rule.  However, daemons like mrouted and pimd emulate `(*,G)` by
  listening for IGMPMSG_NOCACHE messages from the kernel. SMCRoute now
  also implements this, for IPv4 only atm, by placing all `(*,G)`
  routes in a list and adding matching (S,G) routes on-demand at
  runtime. All routes matching this (*,G) are removed when reloading
  the conf file on SIGHUP or when the user sends an IPC (-r) command to
  remove the (*,G) rule.

### Fixes
- Bugfix: SMCRoute segfaults when starting on interface that is up but
  has no valid IPv4 address yet.  Bug introduced in 1.98.3
- Improved error messages including some minor cleanup and readability
  improvements
- Bugfix: Actually check if running as root at startup


[v1.98.3][] - 2011-11-05
------------------------

### Changes
- Check for existence of `asprintf()` to `pidfile()` and add
  `-D_GNU_SOURCE` to `CPPFLAGS` using `AC_GNU_SOURCE` in `configure.ac`
- Cleanup IPv6 `#ifdefs` and replace `IN6_MULTICAST()` with standard
  `IN6_IS_ADDR_MULTICAST()`.  This commit cleans up a lot of the IPv6
  related `#ifdefs`, some minor function name refactoring and squash of
  some `_init` and `_enable` funcs into one for clarity and clearer
  error messages to the user

### Fixes
- Fixes FTBFS when host lacks IPv6 support.


[v1.98.1][] - 2011-11-05
------------------------

### Fixes
- Bugfix: Client failed to send commands to daemon.
- Bugfix: Several FTBFS fixed for GCC 4.6.x and -W -Wall


[v1.98.0][] - 2011-11-04
------------------------

SMCRoute2 Announced!

### Changes
- Feature: Support for `smcroute.conf` configuration file for daemon.
  Add support for reading multicast routes and multicast groups from a
  configuration file.

        mgroup from IFNAME group MCGROUP
        mroute from IFNAME source ADDRESS group MCGROUP to IFNAME [IFNAME ...]

  Both IPv4 and IPv6 address formats are supported
- Feature: Support for signals, reload conf file on `SIGHUP`
- Feature: Add -n switch to support running smcroute in foreground.
- Refactor: Insecure handling of pointers potentially outside array boundaries.
- Refactor: Major cleanup, reindent to Linux C-style, for improved maintainability.

### Fixes
- Bugfix: Invalid use of varargs in call to `snprintf()`, use
  `vsnprintf()` instead
- Bugfix: Invalid `MRouterFD6` fd crashes smcroute, always check for
  valid fd
- Bugfix: Several minor bugfixes; type mismatches and unused return
  values


[v0.95][] - 2011-08-08
----------------------

### Changes
- Feature request #313278: Added support for FreeBSD
  SMCRoute now builds and runs on FreeBSD kernels.  This was successfully
  tested with the FreeBSD port of Debian using FreeBSD 8.1.  Other BSD
  flavours or versions might work too.  Any feedback is appreciated.
  https://alioth.debian.org/tracker/index.php?func=detail&aid=313278
- Feature request #313190: Debug logging is now disabled by default. If you
  want to enable debug logging again, start the daemon with parameter '-D'.
  https://alioth.debian.org/tracker/index.php?func=detail&aid=313190


[v0.94.1][] - 2010-01-13
------------------------

### Fixes
- Bugfix: In case the kernel refuses write access to the file
  /proc/sys/net/ipv6/conf/all/mc_forwarding, don't let smcroute exit
  with an error, but proceed with normal operation without writing a
  "1" to this file.  Apparently newer Linux kernels take care for the
  correct content of this file automatically whenever the IPv6
  multicast routing API is initialized by a process.


[v0.94][] - 2009-11-01
----------------------

### Changes
- Added support for IPv6 multicast routing in smcroute. SMCRoute now
   supports addition and removal of IPv6 multicast routes. It will
   automatically detect which type of route to add or delete based
   on the type (IPv4/IPv6) of addresses provided for the add and
   remove commands.
- Added support for joins and leaves ('j'/'l') to IPv6 multicast groups.
- Added support for sending to IPv6 multicast addresses to mcsender tool.
- Added command line option to mcsender tool to allow user to specify the
   outgoing interface for datagrams sent.
- Added autoconf support for smcroute build.


v0.93 - UNRELEASED
------------------

### Fixes
- Fixed the "smcroute looses output interfaces" bug.
  Carsten Schill, 0.93 unreleased


v0.92 - July 2002
-----------------

### Changes
- Increased the number of supported interfaces
  The 16 interface limit of version 0.90 (interfaces as listed with
  ifconfig) was to small, especially when alias interfaces where
  defined.
  - up to 40 interfaces are no recognized by smcroute
  - this does not change the number of 'virtual interfaces' supported
    by the kernel (32)
  - not all interfaces recognized by smcroute (40) results in a
    'virtual interface' of the kernel (32)

### Fixes
- Fixed the 'mroute: pending queue full, dropping entries' error
  Smcroute 0.90 didn't care about the IGMP messages delivered to the
  UDP socket that establish the MC-Router API. After some time the
  queue for the sockets filled up and the 'pending queue full' message
  was send from the kernel. To my knowledge this didn't affect smcroute
  or the operating system.
  - version 0.92 reads the ICMP messages now from the UDP socket and
    logs them to syslog with daemon/debug
  - smcroute does no further processing of this messages


v0.9 - September 2001
---------------------

### Changes
* Added MC group join (-j) and leave (-l) functionality
  - the options enable/disable the sending of IGMP join messages for
    a multicast group on a specific interface
* Removed the '<OutputIntf> [<OutputIntf>] ...' for the '-r' option
  - they are not used by the kernel to identify the route to remove
  - smcroute will not complain about extra arguments for the '-r' option
    to stay compatible with releases <= 0.80
* Improved error handling for some typical error situations
* Added a test script (tst-smcroute.pl)
* Added a man page

### Fixes
* Fixed some minor bugs


v0.8 - August 2001
------------------

Initial public release by Carsten Schill.


[mrdisc]:     https://github.com/troglobit/mrdisc
[RFC4286]:    https://tools.ietf.org/html/rfc4286
[UNRELEASED]: https://github.com/troglobit/smcroute/compare/2.5.7...HEAD
[v2.5.7]:     https://github.com/troglobit/smcroute/compare/2.5.6...2.5.7
[v2.5.6]:     https://github.com/troglobit/smcroute/compare/2.5.5...2.5.6
[v2.5.5]:     https://github.com/troglobit/smcroute/compare/2.5.4...2.5.5
[v2.5.4]:     https://github.com/troglobit/smcroute/compare/2.5.3...2.5.4
[v2.5.3]:     https://github.com/troglobit/smcroute/compare/2.5.2...2.5.3
[v2.5.2]:     https://github.com/troglobit/smcroute/compare/2.5.1...2.5.2
[v2.5.1]:     https://github.com/troglobit/smcroute/compare/2.5.0...2.5.1
[v2.5.0]:     https://github.com/troglobit/smcroute/compare/2.4.4...2.5.0
[v2.4.4]:     https://github.com/troglobit/smcroute/compare/2.4.3...2.4.4
[v2.4.3]:     https://github.com/troglobit/smcroute/compare/2.4.2...2.4.3
[v2.4.2]:     https://github.com/troglobit/smcroute/compare/2.4.1...2.4.2
[v2.4.1]:     https://github.com/troglobit/smcroute/compare/2.4.1...2.4.1
[v2.4.0]:     https://github.com/troglobit/smcroute/compare/2.3.1...2.4.0
[v2.3.1]:     https://github.com/troglobit/smcroute/compare/2.3.0...2.3.1
[v2.3.0]:     https://github.com/troglobit/smcroute/compare/2.2.2...2.3.0
[v2.2.2]:     https://github.com/troglobit/smcroute/compare/2.2.1...2.2.2
[v2.2.1]:     https://github.com/troglobit/smcroute/compare/2.2.0...2.2.1
[v2.2.0]:     https://github.com/troglobit/smcroute/compare/2.1.1...2.2.0
[v2.1.1]:     https://github.com/troglobit/smcroute/compare/2.1.0...2.1.1
[v2.1.0]:     https://github.com/troglobit/smcroute/compare/2.0.0...2.1.0
[v2.0.0]:     https://github.com/troglobit/smcroute/compare/1.99.2...2.0.0
[v1.99.2]:    https://github.com/troglobit/smcroute/compare/1.99.1...1.99.2
[v1.99.1]:    https://github.com/troglobit/smcroute/compare/1.99.0...1.99.1
[v1.99.0]:    https://github.com/troglobit/smcroute/compare/1.98.3...1.99.0
[v1.98.3]:    https://github.com/troglobit/smcroute/compare/1.98.2...1.98.3
[v1.98.2]:    https://github.com/troglobit/smcroute/compare/1.98.1...1.98.2
[v1.98.1]:    https://github.com/troglobit/smcroute/compare/1.98.0...1.98.1
[v1.98.0]:    https://github.com/troglobit/smcroute/compare/0.95...1.98.0
[v0.95]:      https://github.com/troglobit/smcroute/compare/0.94.1...0.95
[v0.94.1]:    https://github.com/troglobit/smcroute/compare/0.94...0.94.1
[v0.94]:      https://github.com/troglobit/smcroute/compare/0.94.1...0.95
