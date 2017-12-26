EOIP
====

Kernel mode EOIP (Ethernet Over IP) tunnel compatible with MikroTik RouterOS

There are several projects doing the same job with userland utilities via tap interfaces and raw sockets. While a userland application is easier to install and maintain it lacks the perfomance and stability of an in-kernel module. Especially for the simple job of adding and stripping the EOIP tunneling headers. The userland tunneling application may be good for testing, research or concept proof projects but not suitable for production environments with high bandwidth requirements.

This project's goals are:

- to solve the perfomance issue with EOIP on Linux
- to make EOIP tunneling support a standard part of the Linux world


Install
-------

This code was developed on a 3.2.44 linux kernel and tested up to 3.2.51. It should not be hard to adapt it to older kernels and 3.4.x series. Bigger changes are required for 3.10+ kernel series.

- To patch a kernel tree:

```
cd path-to-kernel-source/linux-3.2.44
patch -p1 < path-to-eoip/kernel-patch/kernel-3.2.44-eoip-gre-demux.patch
patch -p1 < path-to-eoip/kernel-patch/kernel-3.2.44-eoip-buildconf.patch
patch -p1 < path-to-eoip/kernel-patch/kernel-3.2.44-eoip.patch
```

afterwards configure the kernel in the usual ways `make (menu/x/...)config` and do not forget to select `IP: EOIP tunnels over IP` located under `Networking options` from `Networking support`

EOIP tunnel depends on `IP: GRE demultiplexer` - if it not selected then EOIP tunnel is not shown at all

Besides on embedded systems it is recommended to build EOIP and GRE demux as modules.

- To build the modules out of the kernel tree:

```
cd path-to-eoip/out-of-tree-X.Y.Z
make
make install
```

For this to work at least the running kernel's headers should be available.

On Debian/Ubuntu systems this build process will place the newly built modules in `/lib/modules/x.x.x.x/misc`. Note that there will be two versions of `gre.ko` (the GRE demux).
At least on 3.2.x it is safe to replace the original version with the modified one because it is backwards compatible.

The `eoip.ko` module cannot operate properly without the newly built version of GRE demux (`gre.ko`). If the original `gre.ko` is loaded then it should be removed and the newly built `gre.ko` loaded before loading `eoip.ko`.

- To build the userland management utility `eoip`:

```
cd path-to-eoip
make
```


Userland management utility
---------------------------

##### `eoip` - tunnel management utility

- to create new eoip tunnel interface:

```
    eoip add tunnel-id <tunnel-id> [name <if-name>]
             [local <src-address>] [remote <dst-address>]
             [link <ifindex>] [ttl <ttl>] [tos <tos>]
```

- to change existing eoip tunnel interface:

```
    eoip change name <if-name> tunnel-id <tunnel-id>
                [local <src-address>] [remote <dst-address>]
                [link <ifindex>] [ttl <ttl>] [tos <tos>]
```

- to list existing eoip tunnels:

```
    eoip list
```


Roadmap
-------

- make a patch for iproute2 to include eoip support

- work towards making this code good enough for inclusion in official kernel/iproute2 releases


Development process
-------------------

This code was developed based on information gathered from sniffed datagrams and information from similar projects without involving any reverse engineering of code from the closed source commercial product

The protocol is not documented and although it looks like there are no deviations in the header format this cannot be guaranteed in all environments or for future releases of the commercial product

Protocol spec
-------------

After the IP header (which can be fragmented, MTU 1500 is usually used for tunnels) a GRE-like datagram follows.
Note that RFC 1701 is mentioned in MikroTik's docs but there is nothing in common between the standard and the actual protocol used.

Header format (taken from https://github.com/katuma/eoip):

    
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |       GRE FLAGS 0x20 0x01     |      Protocol Type  0x6400    | = MAGIC "\x20\x01\x64\x00"
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Encapsulated frame length   |           Tunnel ID           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Ethernet frame...                                             |


Strangely enough the frame length is kept into network byte order and tunnel ID is in little endian byte order.


Bugs
----

This code was tested and works without problems on quite a few different 32/64bit x86 systems.

No testing was done on non-x86 and big endian hardware.

There is no guarantee that there are no bugs left. Patches are welcome.


License
-------

All code and code modifications in this project are released under the GPL licence. Look at the COPYING file.

