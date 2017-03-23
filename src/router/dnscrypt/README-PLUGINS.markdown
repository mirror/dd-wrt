DNSCrypt Plugins
================

Overview
--------

`dnscrypt-proxy` can be extended with plugins.

A plugin can implement *pre-filters* and *post-filters*.

A DNS query received by the proxy will pass through all *pre-filters*
before being encrypted, signed, and sent to the upstream DNS resolver.

Once a response has been received by the proxy, this response is
going to be validated and decrypted before passing through all
*post-filters*, and eventually the end result will eventually be
delivered to the client.

Filters are given the packets in wire format. Every filter can inspect and
alter these packets at will. A filter can also tell the proxy to totally
ignore a query.

    query -> pre-plugins -> encryption/authentication -> resolver

    client <- post-plugins <- verification/decryption <- resolver

Usage
-----

Technically, a plugin is just a native shared library. A good old `.so` file on
Unix, a `.dylib` file on OSX and a `.DLL` file on Windows.

Support for plugins is enabled by default on most platforms.

If the `ltdl` library is found on the system, it will be picked up. If
not, a built-in copy will be used instead.

If you intend to distribute a binary package that should run on
systems without the `ltdl` library (which is probably the case on
Windows), add `--with-included-ltdl`:

    ./configure --with-included-ltdl

Plugins can inspect and mangle packets in any way, but before
reinventing the wheel, take a look at what the `ldns` library has to
offer. `ldns` makes it really easy to parse and build any kind of DNS
packet, can validate DNSSEC records, and is rock solid.

If `ldns` is available on the current system, additional example
plugins will be compiled.

If the `./configure` isn't given a different prefix, example plugins
are installed in `/usr/local/lib/dnscrypt-proxy`.

`dnscrypt-proxy` can load any number of plugins using the `--plugin`
switch, followed by the path to a plugin (library or libtool `.la` file):

    dnscrypt-proxy \
        --plugin=/usr/local/lib/dnscrypt-proxy/libdcplugin_example.la \
        --plugin=/usr/local/lib/dnscrypt-proxy/libdcplugin_example2.la

A full path is actually not required for plugins sitting in the default
plugins directory (`/usr/local/lib/dnscrypt-proxy` by default):

    dnscrypt-proxy \
        --plugin=libdcplugin_example.la \
        --plugin=libdcplugin_example2.la

Filters will always be applied sequentially, in the given order.

These plugins can also be enabled in the configuration file with one or
more `Plugin` directives:

    Plugin libdcplugin_example.la
    Plugin libdcplugin_example2.la

On Unix systems, a file containing a plugin must be owned either by root,
or by the user running the proxy.

You can relax this rule. This can be especially useful on OSX when using
Homebrew, that encourages /usr/local to be owned by a non-root user.
In order to relax this rule, use `--enable-relaxed-plugins-permissions`:

    ./configure --enable-relaxed-plugins-permissions

Each plugin can optionally parse one or more arguments:

    --plugin=...libdcplugin_example.la,--one,--two,--three=4
    --plugin=...libdcplugin_example2.la,127.0.0.1

Plugins are reloaded when the proxy receives a `HUP` signal. This can
be used to rotate log files, or reload updated configuration files.

The plugin API
--------------

When compiled with support for plugins, `dnscrypt-proxy` installs
development headers (in `/usr/local/include/dnscrypt` with the default
prefix).

In addition, the `dnscrypt-proxy` source code ships with a few example
plugins in the `src/plugins` directory to get you started.

The `<dnscrypt/plugin.h>` header file is the only one you need to
include in a plugin. Feel free to take a look at its Doxygen documentation.

The bare minimum a plugin needs is to mention `DCPLUGIN_MAIN(__FILE__)`
and to implement is a `dcplugin_init()` function.

This function is evaluated when the proxy starts, and can optionally
parse a list of arguments:

    #include <dnscrypt/plugin.h>

    DCPLUGIN_MAIN(__FILE__);

    int
    dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
    {
        return 0;
    }

The DCPlugin type is an opaque structure that can store plugin-local
data using the `dcplugin_get_user_data()` and `dcplugin_set_user_data()` macros.

A filter can implement a pre-filter, a post-filter, or both. The
related functions are optional.

    DCPluginSyncFilterResult
    dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet);

    DCPluginSyncFilterResult
    dcplugin_sync_post_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet);

These functions are given an opaque `DCPluginDNSPacket` type, storing
the packet in wire format, and metadata, including the client IP address.

Avoid accessing these metadata directly, and use the macros defined in
`dnscrypt/plugins.h` instead.

A filter can alter the content of the DNS packet, and change its
length with `dcplugin_set_wire_data_len()`. However, the size of the
packet should never be larger than the size returned by
`dcplugin_get_wire_data_max_len`.

The return code of a filter can be either of:
- `DCP_SYNC_FILTER_RESULT_OK`
- `DCP_SYNC_FILTER_RESULT_DIRECT` (only in a pre-filter) to bypass the
resolver and directly send the (possibly modified) packet to the client.
Post-filters will be bypassed as well.
- `DCP_SYNC_FILTER_RESULT_KILL` in order to drop the packet.
- `DCP_SYNC_FILTER_RESULT_ERROR` to drop the packet and indicate that a
non-fatal error occurred.

API documentation
-----------------

For more info about the API, see the online [dnscrypt API
documentation](https://dnscrypt.org/plugin-api/plugin_8h.html).

The `dnscrypt-proxy` source code also ships with example plugins you
may want to take a look at, in the `src/plugins` directory.
