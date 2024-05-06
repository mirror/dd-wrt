---
language: en
layout: default
category: Documentation
title: Daemon Configuration Options
---

[Documentation](documentation.html) > [Other Configuration](documentation.html#other-configuration) > Daemon Configuration Options

# Daemon Configuration Options

## Index

1. [Introduction](#introduction)
2. [Network Socket Configuration File](#network-socket-configuration-file)
	1. [`multicast address`](#multicast-address)
	2. [`multicast port`](#multicast-port)
	3. [`in interface`](#in-interface)
	4. [`out interface`](#out-interface)
	5. [`reuseaddr`](#reuseaddr)
	6. [`ttl`](#ttl)

## Introduction

`joold` (Jool's userspace daemon binary) is part of the [Session Synchronization](session-synchronization.html) gimmic. Follow the link for context.

It expects two optional files as program arguments:

	$ joold [/path/to/netsocket/config] [/path/to/modsocket/config]

The "net socket" file name defaults to `netsocket.json`, and the "module socket" file name defaults to `modsocket.json`. (They are both expected to be found in the same directory the command is executed in.)

## Network Socket Configuration File

This is a Json file that configures the daemon's SS **network** socket. (ie. The one it uses to communicate to other synchronization daemons.) Here are two example of its contents:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">IPv6</span>
	<span class="distro-selector" onclick="showDistro(this);">IPv4</span>
</div>

<!-- IPv6 -->
{% highlight json %}
{
	"multicast address": "ff08::db8:64:64",
	"multicast port": "6464",
	"in interface": "eth0",
	"out interface": "eth0",
	"reuseaddr": 1,
	"ttl": 3
}
{% endhighlight %}

<!-- IPv4 -->
{% highlight json %}
{
	"multicast address": "233.252.0.64",
	"multicast port": "6464",
	"in interface": "192.0.2.1",
	"out interface": "192.0.2.1",
	"reuseaddr": 1,
	"ttl": 3
}
{% endhighlight %}

These are the options:

### `multicast address`

- Type: String (IPv4/v6 address)
- Default: None (Field is mandatory)

Address the SS traffic will be sent to and listened from.

### `multicast port`

- Type: String (port number or service name)
- Default: None (Field is mandatory)

TCP port where the SS traffic will be sent to and listened from.

### `in interface`

- Type: String
- Default: NULL (kernel chooses an interface and address for you)

Address or interface to bind the socket in.

If `multicast address` is IPv4, this should be one addresses from the interface where the SS traffic is expected to be received. If `multicast address` is IPv6, this should be the name of the interface (eg. "eth0").

Though they are optional, it is strongly recommended that you define both `in interface` and `out interface` to ensure the SS traffic does not leak through other interfaces.

### `out interface`

- Type: String
- Default: NULL (kernel chooses an interface and address for you)

If `multicast address` is IPv4, this should be one addresses from the interface where the multicast traffic is expected to be sent. If `multicast address` is IPv6, this should be the name of the interface (eg. "eth0").

Though they are optional, it is strongly recommended that you define both `in interface` and `out interface` to ensure the SS traffic does not leak through other interfaces.

### `reuseaddr`

- Type: Integer
- Default: 0

Same as `SO_REUSEADDR`. From `man 7 socket`:

	SO_REUSEADDR
		Indicates that the rules used in validating addresses supplied
		in a bind(2) call should allow reuse of local addresses. For
		AF_INET sockets this means that a socket may bind, except when
		there is an active listening socket bound to the address. When
		the listening socket is bound to INADDR_ANY with a specific port
		then it is not possible to bind to this port for any local
		address. Argument is an integer boolean flag.

A rather more humane explanation can be found in [Stack Overflow](http://stackoverflow.com/questions/14388706):

	In other words, for multicast addresses `SO_REUSEADDR` behaves exactly
	as `SO_REUSEPORT` for unicast addresses.

	...

	Basically, `SO_REUSEPORT` allows you to bind an arbitrary number of
	sockets to exactly the same source address and port as long as all prior
	bound sockets also had `SO_REUSEPORT` set before they were bound. If the
	first socket that is bound to an address and port does not have
	`SO_REUSEPORT` set, no other socket can be bound to exactly the same
	address and port, regardless if this other socket has `SO_REUSEPORT` set
	or not, until the first socket releases its binding again.

You do not want a hanging joold to prevent future joolds from having access to the SS traffic, so there is likely no reason to ever turn this value off. Unless you have a specific reason to change this, you should **always** include this value, and **always** override the default.

### `ttl`

- Type: Integer
- Default: 1

Same as `IP_MULTICAST_TTL`. From `man 7 ip`:

	IP_MULTICAST_TTL (since Linux 1.2)
		Set or read the time-to-live value of outgoing multicast packets
		for this socket. It is very important for multicast packets to
		set the smallest TTL possible. The default is 1 which means that
		multicast packets don't leave the local network unless the user
		program explicitly requests it. Argument is an integer.

## Module Socket Configuration File

This is a Json file that configures the daemon's SS **Netlink** socket. (ie. the one it uses to communicate with its designated Jool instance.) Here's an example of its contents:

```json
{
	"instance": "potato"
}
```

These are the options:

### `instance`

Name of the instance the daemon is supposed to synchronize. It's the one you designate during [`jool instance add`](usr-flags-instance.html). As usual, it defaults to "`default`."

The instance is expected to exist within the same network namespace the daemon is running in.

