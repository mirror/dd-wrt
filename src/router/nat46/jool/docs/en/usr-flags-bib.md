---
language: en
layout: default
category: Documentation
title: bib Mode
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > `bib` Mode

# `bib` Mode

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
   1. [`display`](#display)
   2. [`add`](#add)
   3. [`remove`](#remove)
   4. [Flags](#flags)
   5. [Transport addresses](#transport-addresses)
4. [Examples](#examples)

## Description

Interacts with Jool's [Binding Information Base](bib.html).

## Syntax

	jool bib (
		display  [PROTOCOL] [--numeric] [--csv] [--no-headers]
		| add    [PROTOCOL] <IPv4-transport-address> <IPv6-transport-address>
		| remove [PROTOCOL] <IPv4-transport-address> <IPv6-transport-address>
	)

	PROTOCOL := --tcp | --udp | --icmp

> ![../images/warning.svg](../images/warning.svg) **Warning**: Jool 3's `PROTOCOL` label used to be defined as `[--tcp] [--udp] [--icmp]`. The flags are mutually exclusive now, and default to `--tcp`.

## Arguments

### `display`

The BIB table that corresponds to the `PROTOCOL` protocol is printed in standard output.

### `add`

Combines `<IPv4-transport-address>` and `<IPv6-transport-address>` into a static BIB entry, and uploads it to the BIB table that corresponds to the `PROTOCOL` protocol.

Note that the `[<IPv4-transport-address>, PROTOCOL]` tuple must be a member of Jool's [IPv4 pool](usr-flags-pool4.html), so make sure you have registered it there first.

### `remove`

Deletes the BIB entry described by `<IPv4-transport-address>` and/or `<IPv6-transport-address>` from the table that corresponds to the `PROTOCOL` protocol. The entry is not required to be static to be manually removed.

Since both transport addresses are unique within a table, you are allowed to omit one of them during removals.

### Flags

| **Flag** | **Description** |
| `--tcp` | Operate on the TCP table. This is the default protocol. |
| `--udp` | Operate on the UDP table. |
| `--icmp` | Operate on the ICMP table. |
| `--numeric` | By default, `display` will attempt to resolve the names of the IPv6 transport addresses of each BIB entry. _If your nameservers aren't answering, this will pepper standard error with messages and slow the operation down_.<br />Use `--numeric` to disable the lookups. |
| `--csv` | Print the table in [_Comma/Character-Separated Values_ format](http://en.wikipedia.org/wiki/Comma-separated_values). This is intended to be redirected into a .csv file. |
| `--no-headers` | Print the table entries only; omit the headers. |

### Transport addresses

In TCP and UDP, a "transport address" is an union between an IP address and port. The format is "`<IP address>#<port>`".

In ICMP, a "transport address" is an union between an IP address and an ICMP identifier. The format is "`<IP address>#<ICMP identifier>`".

## Examples

Assumptions:

* `192.0.2.4` belongs to the IPv4 pool.
* The name of `2001:db8::6` is "potato.mx".
* `2001:db8::6` already spoke to an IPv4 node recently (so the database will be slightly populated in the beginning.)

Display the TCP table:

{% highlight bash %}
user@T:~# jool bib display
[Dynamic] 192.0.2.4#1234 - potato.mx#1234
{% endhighlight %}

Publish a couple of TCP services:

{% highlight bash %}
user@T:~# jool bib add 2001:db8::6#6  192.0.2.4#4  --tcp
user@T:~# jool bib add 2001:db8::6#66 192.0.2.4#44 --tcp
{% endhighlight %}

Display the TCP table again:

{% highlight bash %}
user@T:~# jool bib display --tcp
[Static] 192.0.2.4#4 - potato.mx#6
[Static] 192.0.2.4#44 - potato.mx#66
[Dynamic] 192.0.2.4#1234 - potato.mx#1234
{% endhighlight %}

Same, but do not query the DNS:

{% highlight bash %}
user@T:~# jool bib display --tcp --numeric
[Static] 192.0.2.4#4 - 2001:db8::6#6
[Static] 192.0.2.4#44 - 2001:db8::6#66
[Dynamic] 192.0.2.4#1234 - 2001:db8::6#1234
{% endhighlight %}

Dump the TCP table on a CSV file:

{% highlight bash %}
user@T:~# jool bib display --tcp --numeric --csv > bib-tcp.csv
{% endhighlight %}

[bib-tcp.csv](../obj/bib-tcp.csv)

Publish a UDP service:

{% highlight bash %}
user@T:~# jool bib add 2001:db8::6#6666 192.0.2.4#4444 --udp
{% endhighlight %}

Remove the UDP entry:

{% highlight bash %}
user@T:~# jool bib remove --udp 2001:db8::6#6666
{% endhighlight %}

