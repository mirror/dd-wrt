# Jool's Userspace Tools

## `nat64/`

Source code of the `jool` client.

This is the userspace application admins use to send requests to the `jool` kernel module.

## `siit/`

Source code of the `jool_siit` client.

This is the userspace application admins use to send requests to the `jool_siit` kernel module.

## `iptables/`

Source code of the iptables shared objects.

These are iptables plugins, which enable `JOOL` and `JOOL_SIIT` targets. They are used by admins to create iptables rules which define which packets are meant to be handled by Jool instances.

## `joold/`

Source code of `joold`.

`joold` is the [session synchronization](https://nicmx.github.io/Jool/en/session-synchronization.html) daemon, intended to enable NAT64 redundancy. (Though alternate instances are intended to be used as backups, not simultaneously.)

## `argp/`

Source code of `libjoolargp`, a shared library containing command line option parsing that's common to `jool_siit` and `jool`.

It's a layer between the clients and `libjoolnl`.

## `nl/`

Source code of `libjoolnl`, a shared library any application can theoretically use to send requests to the kernel modules. Presently used by `jool`, `jool_siit` and `joold`.

## `util/`

Source code of `libjoolutil`, a random utility shared library. Usable by any or all of the above.
