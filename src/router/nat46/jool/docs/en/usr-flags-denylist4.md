---
language: en
layout: default
category: Documentation
title: denylist4 Mode
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > `denylist4` Mode

# `denylist4` Mode

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
	1. [Operations](#operations)
	2. [Options](#options)
4. [Examples](#examples)

## Description

Interacts with Jool's denylist address pool.

The pool lists IPv4 translation addresses which are _banned_ from [pool6](usr-flags-pool6.html)-based translation. If an incoming IPv4 packet contains one of these addresses (which would be translated via pool6 and not the EAMT), Jool will silently abort translation. If an incoming IPv6 packet translates (through pool6) into an IPv4 packet that contains one of these addresses, Jool will also silently abort translation. 

denylist4 applies on most addresses. The only exception is source addresses from ICMP errors. These are translated regardless of denylist4 for the sake of troubleshooting purposes.

As an aside, there are some addresses Jool will refuse to translate, regardless of `denylist4`. These include

- The addresses that belong to Jool's node (because Jool can only be used in a forwarding fashion, currently).
- Software addresses (0.0.0.0/8).
- Host addresses (127.0.0.0/8).
- Link-local addresses (169.254.0.0/16).
- Multicast addresses (224.0.0.0/4).
- Limited broadcast (255.255.255.255/32).

The denylist is mostly only relevant in Netfilter Jool, because iptables Jool already has matching conditionals by nature.

> ![Note!](../images/bulb.svg) Before Jool 4.1.4, `denylist4` used to be called "`blacklist4`." The latter still works, but is considered a deprecated alias.

## Syntax

	jool_siit denylist4 (
		display [--csv]
		| add <IPv4-prefix>
		| remove <IPv4-prefix>
		| flush
	)

## Arguments

### Operations

* `display`: The pool's addresses/prefixes are printed in standard output.
* `add`: Uploads `<IPv4-prefix>` to the pool.
* `remove`: Deletes `<IPv4-prefix>` from the pool.
* `flush`: Removes all addresses/prefixes from the pool.

`<IPv4-prefix>`'s prefix length defaults to 32.

### Options

| **Flag** | **Description** |
| `--csv` | Print the table in [_Comma/Character-Separated Values_ format](http://en.wikipedia.org/wiki/Comma-separated_values). This is intended to be redirected into a .csv file. |

## Examples

These examples assume that the name of the Jool instance is "default."

Add addresses:

	# jool_siit denylist4 add 192.0.2.0/28
	# jool_siit denylist4 add 198.51.100.0/30
	# jool_siit denylist4 add 203.0.113.8/32

Display the current addresses:

	# jool_siit denylist4 display
	+--------------------+
	|        IPv4 Prefix |
	+--------------------+
	|       192.0.2.0/28 |
	|    198.51.100.0/30 |
	|     203.0.113.8/32 |
	+--------------------+

Remove an entry:

	# jool_siit denylist4 remove 198.51.100.0/30

Clear the table:

	# jool_siit denylist4 flush

