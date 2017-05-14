Using the DNSCrypt client on Windows
====================================

`dnscrypt-proxy` is DNSCrypt client that works on many platforms,
including Windows.

It doesn't provide any user interface, and has to be set up using the
command-line.

Independent projects such as [Simple DNSCrypt](https://simplednscrypt.org/)
provide a user interface on top of `dnscrypt-proxy`, so that the core
client code can always be up-to-date, and the same as other platforms.

However, using `dnscrypt-proxy` directly is fairly simple and opens a
lot of options.

Quickstart
----------

The following instructions are provided as offline documentation, but
better/more up to date information is available online:
[dnscrypt-proxy guide](https://github.com/jedisct1/dnscrypt-proxy/wiki).

1) Download and extract the latest
[Windows package for dnscrypt](https://download.dnscrypt.org/dnscrypt-proxy/).

2) Extract the `dnscrypt-proxy-win32` or `dnscrypt-proxy-win64` folder
anywhere, but this has to be a permanent location.

3) The `dnscrypt-resolver.csv` file includes a list of public DNS
resolvers supporting the DNSCrypt protocol. The most recent version
can be previewed online:
[public DNS resolvers supporting DNSCrypt](https://dnscrypt.org/dnscrypt-resolvers.html)
and downloaded:
[dnscrypt-resolvers.csv](https://download.dnscrypt.org/dnscrypt-proxy/dnscrypt-resolvers.csv).

Choose one that fits your needs. Its identifier ("resolver name") is in
the first column (for example: `dnscrypt.org-fr`).

4) Edit the configuration file `dnscrypt-proxy.conf`.

5) Open an elevated command prompt (see below), enter the dnscrypt-proxy folder and type:

    .\dnscrypt-proxy.exe dnscrypt-proxy.conf --test=0

This command just tests if everything is properly installed on your
end, and if the resolver is properly working. If everything looks fine,
the command should display the server key fingerprint and exit right away.

If an error is displayed, retry with a different server.

5) So far, so good? Now, enable the service for real:

    .\dnscrypt-proxy.exe --install-with-config-file=dnscrypt-proxy.conf

6) Open the network preferences ("Network connections", then select
your network adapter and hit "Properties"). Then in the "Internet Protocol
Version 4 (TCP/IPv4)" settings use `127.0.0.1` instead of the default DNS
resolver address.

Congratulations, you're now using DNSCrypt!

IPv6
----

Do you have IPv6 connectivity? No problem!

In the preferences pane for a given network connection, Windows has
IPv6-specific settings under "Internet Protocol Version 6 (TCP/IPv6)".

The DNS server addresses have to be IPv6 addresses, so enter the
IPv6 representation of `127.0.0.1`:

    ::FFFF:127.0.0.1

Done. You can now enjoy the DNSCrypt proxy both for IPv4 and IPv6
connections.

How to open an elevated command prompt
--------------------------------------

On Windows 8.1 and Windows 10, press the Windows key + the X key and
select "Windows Command Prompt (Admin)" or "Windows PowerShell (Admin)".

If you get the message:

    [ERROR] Unable to bind (UDP) [Address already in use [WSAEADDRINUSE ]]

chances are that you tried to start the proxy from a limited
(non-elevated) command prompt.

Temporarily disabling DNSCrypt
------------------------------

`dnscrypt-proxy` receives DNS queries from your applications on
`127.0.0.1` (by default), wraps them into DNSCrypt queries, forwards
them to the real DNS resolver, securely receives and verifies the
responses, and forwards the legitimate ones to your client applications.

If you changed the DNS settings for a given network interface to
`127.0.0.1`, you can revert these settings to what they used to be (or
to any non-DNSCrypt resolver) anytime in order to stop using DNSCrypt.
Changing the address back to `127.0.0.1` makes the network interface
use the DNSCrypt proxy again.

The Windows service
-------------------

On Windows, `dnscrypt-proxy` can run as a Windows service, and this is how it
was set up in the quickstart section above.

In addition to the command-line switches available on other platforms,
the Windows builds of the proxy add the following switches:

- `--install`: install the proxy as a service.
- `--install-with-config-file=<config file>`: install the proxy as a
service, using the provided configuration file (`dnscrypt-proxy.conf`).
Double check that the configuration file is valid prior to installing
the service.
- `--uninstall`: uninstall the service (but not the software - the
service can be restarted later)
- `--service-name=<name>`: set the service name (by default:
`dnscrypt-proxy`). Multiple services with a different configuration can run
simultaneously if they use distinct service names. `--service-name`
must be combined with `--install`, `--install-with-config-file` or
`--uninstall`.

Example: how to try a different DNSCrypt resolver:

Step 1 - Uninstall the previous service:

    .\dnscrypt-proxy --uninstall

Step 2 - Reinstall/restart the service, with the new settings:

    .\dnscrypt-proxy -R <new name> --install

Sharing the proxy with the local network
----------------------------------------

By default, only the Windows machine running the proxy can use it.

However, it can be convenient to make it accessible from any device on
the local network. For example, smartphones and tablets can use the
Windows machine as a DNS resolver (which will actually be the DNSCrypt
proxy) instead of running DNSCrypt themselves.

In order to do so, just add the following option to the command-line:
`--local-address=0.0.0.0`.

That is:

    .\dnscrypt-proxy -R <name> --install --local-address=0.0.0.0

And use the IP address of the Windows machine in the DNS settings of
any devices of the local network.

Removing the software from the system
-------------------------------------

If this DNSCrypt client doesn't fit your needs, we are very sorry for
this, and we'd love to hear about how we could make it better.

So, go to the "Support" section of the
[DNSCrypt](https://dnscrypt.org/) site and tell us your story.

Removing `dnscrypt-proxy` from your system is straightforward.

Before doing so, make sure that the DNS settings of your network
interfaces have been restored to what they were before (which, most of
the time, is just "DHCP").

Then, uninstall the service:

    .\dnscrypt-proxy --uninstall

And delete the directory.

Advanced configuration
----------------------

Many additional features (logging, filtering...) can be enabled by
loading a configuration file. This requires at least dnscrypt-proxy
version 1.8.0.

1) Make sure that the service is not running:

    .\dnscrypt-proxy --uninstall

2) Edit the `dnscrypt-proxy.conf` configuration file according to your
needs.

3) Check that that configuration actually works as expected, by
starting the proxy without installing the service:

    .\dnscrypt-proxy dnscrypt-proxy.conf

Check that errors are not printed, and that DNS queries sent to the
configured IP addresses receive responses. Hit `Control`+`C` in order to
stop the server and get back to the interactive command prompt.

4) If that setup looks fine, install the Windows service so that it
loads that configuration file automatically:

    .\dnscrypt-proxy --install-with-config-file=dnscrypt-proxy.conf

Plugins
-------

Plugins should be listed as paths to the `.DLL` files, optionally
followed by a coma and plugin-specific arguments:

    .\dnscrypt-proxy -R <name> --plugin=libdcplugin_example_ldns_aaaa_blocking.dll
    .\dnscrypt-proxy -R <name> --plugin=libdcplugin_example_ldns_blocking.dll,--domains=C:\blacklisted-domains.txt

The service should be restarted after the registry has been updated.

Windows registry keys
---------------------

Startup options can specified as subkeys from a registry key:
`HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\dnscrypt-proxy\Parameters`

By default, the service is named `dnscrypt-proxy`, but this can be changed
with the `--service-name` command-line switch when installing the service.

The following registry values are recognized:

Registry Value    | Type
----------------- | --------------
ConfigFile        | REG_SZ
ResolversList     | REG_SZ
ResolverName      | REG_SZ
LocalAddress      | REG_SZ
ProviderKey       | REG_SZ
ProviderName      | REG_SZ
ResolverAddress   | REG_SZ
EDNSPayloadSize   | REG_DWORD
MaxActiveRequests | REG_DWORD
TCPOnly           | REG_DWORD
EphemeralKeys     | REG_DWORD
IgnoreTimestamps  | REG_DWORD
ClientKeyFile     | REG_SZ
LogFile           | REG_SZ
LogLevel          | REG_DWORD
Plugins           | REG_MULTI_SZ

Detail of registry values:

    ResolversList     : Full path to the `dnscrypt-resolvers.csv` file.
                        Equivalent to the `resolvers-list` parameter.
    ResolverName      : Resolver name in the `dnscrypt-resolvers.csv` file.
                        This is the first column (`Name`) in that CSV file.
                        Equivalent to the `resolver-name` parameter.
    LocalAddress      : IP address where `dnscrypt-proxy` listen for DNS request.
                        Equivalent to the `local-address` parameter.
    ProviderKey       : DNS server key.
                        `Provider public key` column in the `dnscrypt-resolvers.csv` file.
                        Equivalent to the `provider-key` parameter.
    ProviderName      : DNS server name.
                        `Provider name` column in the `dnscrypt-resolvers.csv` file.
                        Equivalent to the `provider-name` parameter.
    ResolverAddress   : DNS server IP.
                        `Resolver address` column in the `dnscrypt-resolvers.csv` file.
                        Equivalent to the `resolver-address` parameter.
    EDNSPayloadSize   : EDNS size.
                        Must be between `1` and `65507` (IPv4) or `65535` (IPv6-only).
                        Equivalent to the `edns-payload-size` parameter.
    MaxActiveRequests : Maximum number of client DNS requests to process concurrently.
                        Must be equal or greater than `1`.
                        Equivalent to the `max-active-requests` parameter.
    TCPOnly           : Send DNS queries to upstream servers using only TCP if set to `1`.
                        Must be `1` or `0`.
                        Equivalent to the `tcp-only` parameter.
    EphemeralKeys     : Create a new key pair for every query.
                        Must be `1` or `0`.
                        Equivalent to the `ephemeral-keys` parameter.
    IgnoreTimestamps  : Must be `1` or `0`.
                        Equivalent to the `ignore-timestamps` parameter. Do not enable blindly.
    ClientKeyFile     : Use a static key pair. This is the path to a file storing the secret key.
                        Equivalent to the `client-key` parameter.
    LogFile           : Log file for `dnscrypt-proxy`.
                        Equivalent to the `logfile` parameter.
    LogLevel          : Maximum log level.
                        Equivalent to the `loglevel` parameter.
    Plugins           : Set of plugins to be loaded by `dnscrypt-proxy`.
                        Equivalent to one or more `plugin` command-line arguments.

Plugins Example (INF):

````
HKLM,"SYSTEM\CurrentControlSet\services\dnscrypt-proxy\Parameters",0x10000,"C:\Program Files\DNSCrypt\libdcplugin_example_ldns_blocking.dll,--domains=C:\Program Files\DNSCrypt\Names.txt,--ips=C:\Program Files\DNSCrypt\IPs.txt,--logfile=C:\DNSCrypt-Block.log"
````

For example, in order to listen to a local address different from the default
`127.0.0.1`, the key to put the custom IP address in is
`HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\dnscrypt-proxy\Parameters\LocalAddress`.

Unless `ConfigFile` is set, two entries are mandatory:
- `ResolversList`: has to be set to the full path to the `dnscrypt-resolvers.csv` file.
- `ResolverName`: has to be set to the resolver name to be used. See
the `dnscrypt-resolvers.csv` file for a list of compatible public resolvers.

These entries are automatically created/updated when installing the service.
