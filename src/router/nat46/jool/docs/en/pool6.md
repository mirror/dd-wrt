---
language: en
layout: default
category: Documentation
title: IPv4 Transport Address Pool
---

[Documentation](documentation.html) > [IP/ICMP Translation in Detail](documentation.html#ipicmp-translation-in-detail) > The IPv6 Address Pool

# The IPv6 Address Pool

## Index

1. [Introduction](#introduction)
2. [Specification Summary](#specification-summary)
3. [NAT64 Jool's pool6](#nat64-jools-pool6)
4. [SIIT Jool's pool6](#siit-jools-pool6)

## Introduction

A translator's "IPv6 address pool" is a handful of prefixes which are used by IP/ICMP Translation's eldest IP **address** translation algorithm (which is defined in [RFC 6052](https://tools.ietf.org/html/rfc6052)).

Because of its internal variable naming conventions, Jool simply calls it "pool6" (as a shorthand for "IPv6 pool") in pretty much every situation. Also, because its instance capabilities render a complex pool6 reduntant, Jool manages only **one** pool6 prefix per instance.

[Vanilla SIIT](intro-xlat.html#siit-traditional) uses pool6 to translate every address, while [NAT64](intro-xlat.html#stateful-nat64) only uses it to translate the destination address of incoming IPv6 packets and the source address of incoming IPv4 packets.

## Specification Summary

pool6's prefix length (PL) must be 32, 40, 48, 56, 64 or 96. [As defined by the RFC](https://tools.ietf.org/html/rfc6052#section-2.2), pool6 address translation is performed according to the following table:

	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|PL| 0-------------32--40--48--56--64--72--80--88--96--104---------|
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|32|     prefix    |v4(32)         | u | suffix                    |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|40|     prefix        |v4(24)     | u |(8)| suffix                |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|48|     prefix            |v4(16) | u | (16)  | suffix            |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|56|     prefix                |(8)| u |  v4(24)   | suffix        |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|64|     prefix                    | u |   v4(32)      | suffix    |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	|96|     prefix                                    |    v4(32)     |
	+--+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

Given a pool6 prefix, any given IPv4 address is encoded in the `v4` slots of its byte array. `u` and `suffix` are always zero. More often than not, PL=96 is used because it's the most intuitive option.

Here are a few examples:

| pool6              | IPv4 address | IPv6 counterpart               | Notes                                                                     |
|--------------------|--------------|--------------------------------|---------------------------------------------------------------------------|
| 64:ff9b::/96       | 192.0.2.1    | 64:ff9b::192.0.2.1             | The IPv4 address is simply stuck at the end of the prefix.                |
| 2001:db8::/32      | 192.0.2.1    | 2001:db8:c000:0201::           | The IPv4 address is located in bits 32-63. Everything after that is zero. |
| 2001:db8:aaaa::/48 | 192.0.2.1    | 2001:db8:aaaa:c000:0002:0100:: | The IPv4 address is located in bits 48-87 with a gap among bits 64-71.    |

Just note that the translation is bidirectional; given a pool6 prefix, one can infer the IPv4 counterpart of an IPv6 address (assuming that the IPv6 address matches the prefix) as well as the IPv6 counterpart of an IPv4 address.

## NAT64 Jool's pool6

Because pool6 is the only currently standardized means for a NAT64 to translate IPv6 destinations and IPv4 sources, a pool6 prefix is quintessential NAT64 configuration. NAT64 Jool simply cannot do anything without one.

Also, a synchronization issue was discovered during the review of Jool 4.0.0, and the most realistic solution turned out to be to turn pool6 into a constant. (For now.)

Thus, NAT64 Jool's pool6 can only be specified during [instance creation](usr-flags-instance.html). It cannot be changed or removed after that:

{% highlight bash %}
user@T:~# jool instance add --iptables --pool6 64:ff9b::/96
{% endhighlight %}

## SIIT Jool's pool6

Stateless Jool does not suffer from NAT64's synch issue, so its pool6 can be changed at any time. Note, however, that `pool6` is now a [global configuration field](usr-flags-global.html#--pool6), and not its own database. You no longer tweak it by means of `--pool6 --add`, but rather `global update pool6`:

{% highlight bash %}
user@T:~# jool_siit instance add --iptables -6 64:ff9b::/96
user@T:~# jool_siit global update pool6 2001:db8::/96
{% endhighlight %}

SIIT has two main address translation mechanisms: pool6 and [EAMs](eamt.html). Both of them can be used to translate any address. Consequently, SIIT Jool can operate with a void pool6 if you so desire:

{% highlight bash %}
user@T:~# jool_siit eamt add ...
user@T:~# jool_siit eamt add ...
user@T:~# jool_siit eamt add ...
user@T:~# jool_siit global update pool6 null
{% endhighlight %}
