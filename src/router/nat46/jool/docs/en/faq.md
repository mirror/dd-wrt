---
language: en
layout: default
category: FAQ
title: FAQ
---

[Documentation](documentation.html) > [Miscellaneous](documentation.html#miscellaneous) > FAQ

# Troubleshooting/FAQ

## Index

1. [Error: Don't know what to do: The packet I just received does not follow Jool's protocol.](#error-dont-know-what-to-do-the-packet-i-just-received-does-not-follow-jools-protocol)
1. [Why is Jool not doing anything?](#why-is-jool-not-doing-anything)
2. [Why is my ping not working?](#why-is-my-ping-not-working)
3. [Jool is intermitently unable to translate traffic.](#jool-is-intermitently-unable-to-translate-traffic)
4. [The throughput is terrible!](#the-throughput-is-terrible)

## Error: Don't know what to do: The packet I just received does not follow Jool's protocol.

Sorry; this error message is supposed to be 

	Version mismatch. The userspace client's version is X.X.X.X,
	but the kernel module is Y.Y.Y.Y.
	Please update the (userspace client|kernel module).

but, because of [this bug](https://github.com/NICMx/Jool/issues/340), the relevant protocol details changed between Jool 4.1.4 and Jool 4.1.5.

The problem is that you updated either the userspace client or the kernel module, but not both.

You can check your userspace client version with

```bash
$ jool --version
X.X.X.X
```

And the kernel module version by querying `dmesg` after a successful `modprobe`:

```bash
$ sudo modprobe jool
$ dmesg -t | grep inserted
Jool: Core Jool vY.Y.Y.Y module inserted.
NAT64 Jool vY.Y.Y.Y module inserted.
```

Update either artifact by following the [corresponding instructions](documentation.html#installation).

## Why is Jool not doing anything?

First off: As of Jool 4, modprobing the module is no longer enough. Did you [create an instance](usr-flags-instance.html)?

If so, then try printing your instance's active [statistics](usr-flags-stats.html):

{% highlight bash %}
user@T:~# jool -i "<Your instance's name>" stats display --explain
JSTAT_SUCCESS: 35
Successful translations. (Note: 'Successful translation' does not imply
that the packet was actually delivered.)

JSTAT_BIB_ENTRIES: 5
Number of BIB entries currently held in the BIB.

JSTAT_SESSIONS: 8
Number of session entries currently held in the BIB.

JSTAT_BIB4_NOT_FOUND: 1
Translations cancelled: IPv4 packet did not match a BIB entry from the
database.

JSTAT_FAILED_ROUTES: 1045
The translated packet could not be routed; the kernel's routing function
errored. Cause is unknown. (It usually happens because the packet's
destination address could not be found in the routing table.)

{% endhighlight %}

Given the output above, for example, I'd try looking into the routing table.

If `stats` proves insufficient, you can [enable debug logging](usr-flags-global.html#logging-debug).

## Why is my ping not working?

Probably because you started the ping on the same machine (or rather, network namespace) your translator instance is attached to.

At present, Netfilter Jool only hooks itself to [`PRE_ROUTING`](https://netfilter.org/documentation/HOWTO/netfilter-hacking-HOWTO-3.html). It does **not** attach itself to `LOCAL_OUT`. This means it can only translate traffic that inbounds from some interface (physical or otherwise). It does **not** intercept packets sourced from its own network namespace.

Because iptables provides matching functionality, it makes more sense to attach iptables Jool instances to `LOCAL_OUT`. And AFAIK, there's nothing stopping you from doing so. But don't quote me; I haven't tested it.

## Jool is intermitently unable to translate traffic.

Did you run something in the lines of

{% highlight bash %}
ip addr flush dev eth1
{% endhighlight %}

?

Then you might have deleted the interface's <a href="http://en.wikipedia.org/wiki/Link-local_address" target="_blank">Link address</a>.

Link addresses are used by several relevant IPv6 protocols. In particular, they are used by the *Neighbor Discovery Protocol*, which means if you don't have them, the translating machine will have trouble finding its IPv6 neighbors.

Check the output of `ip addr`. 

<div class="highlight"><pre><code class="bash">user@T:~$ /sbin/ip address
1: lo: &lt;LOOPBACK,UP,LOWER_UP&gt; mtu 16436 qdisc noqueue state UNKNOWN 
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: <strong>eth0</strong>: &lt;BROADCAST,MULTICAST,UP,LOWER_UP&gt; mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:83:d9:40 brd ff:ff:ff:ff:ff:ff
    inet6 2001:db8:aaaa::1/64 <strong>scope global</strong> 
       valid_lft forever preferred_lft forever
    inet6 fe80::a00:27ff:fe83:d940/64 <strong>scope link</strong> 
       valid_lft forever preferred_lft forever
3: <strong>eth1</strong>: &lt;BROADCAST,MULTICAST,UP,LOWER_UP&gt; mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:c6:01:48 brd ff:ff:ff:ff:ff:ff
    inet6 2001:db8:bbbb::1/64 <strong>scope global</strong> tentative 
       valid_lft forever preferred_lft forever
</code></pre></div>

Interface _eth0_ is correctly configured; it has both a "scope global" address (used for typical traffic) and a "scope link" address (used for internal management). Interface _eth1_ lacks a link address, and is therefore a headache inducer.

The easiest way to restore scope link addresses, we have found, is to just reset the interface:

{% highlight bash %}
ip link set eth1 down
ip link set eth1 up
{% endhighlight %}

Yes, I'm serious:

<div class="highlight"><pre><code class="bash">user@T:~$ /sbin/ip address
1: lo: &lt;LOOPBACK,UP,LOWER_UP&gt; mtu 16436 qdisc noqueue state UNKNOWN 
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eth0: &lt;BROADCAST,MULTICAST,UP,LOWER_UP&gt; mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:83:d9:40 brd ff:ff:ff:ff:ff:ff
    inet6 2001:db8:aaaa::1/64 scope global 
       valid_lft forever preferred_lft forever
    inet6 fe80::a00:27ff:fe83:d940/64 scope link 
       valid_lft forever preferred_lft forever
3: eth1: &lt;BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:c6:01:48 brd ff:ff:ff:ff:ff:ff
    inet6 fe80::a00:27ff:fec6:148/64 <strong>scope link</strong> 
       valid_lft forever preferred_lft forever
</code></pre></div>

(Note, you need to add the global address again)

Also, for future reference, keep in mind that the "correct" way to flush an interface is

{% highlight bash %}
ip addr flush dev eth1 scope global
{% endhighlight %}

IPv4 doesn't need link addresses that much.

## The throughput is terrible!

[Try disabling offloads](offloads.html).

If you're running Jool in a guest virtual machine, something important to keep in mind is that you might rather or also have to disable offloads in the [VM host](http://en.wikipedia.org/wiki/Hypervisor)'s uplink interface.

And please [report](contact.html) that this happened to you. Because of recent developments, Jool should be offload-independent as of version 4.0.0.
