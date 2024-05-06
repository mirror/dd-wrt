---
language: en
layout: default
category: Documentation
title: Cheat Sheet
---

# Cheat Sheet

## SIIT (EAMT)

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_siit
	jool_siit instance add --netfilter
	jool_siit eamt add 2001:db8:AAAA::A 192.0.2.123
	jool_siit eamt add 2001:db8:BBBB::/120 203.0.113.0/24

- The `2001:db8:AAAA::A` node will be known by IPv4 nodes as `192.0.2.123`.
- Alternatively, the `192.0.2.123` node will be known by IPv6 nodes as `2001:db8:AAAA::A`.
- `2001:db8:BBBB::X` will be known by IPv4 nodes as `203.0.113.Y`. (Where `X` is the hexadecimal representation of `Y`.)
- Alternatively, `203.0.113.Y` will be known by IPv4 nodes as `2001:db8:BBBB::X`.

## SIIT (traditional)

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_siit
	jool_siit instance add --netfilter --pool6 64:ff9b:1::/96

- `64:ff9b:1::X` will be known by IPv4 nodes as `X`. (Where `X` is any IPv4 address.)
- Alternatively, `X` will be known by IPv6 nodes as `64:ff9b:1::X`.

## SIIT-DC

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_siit
	jool_siit instance add --netfilter --pool6 64:ff9b:1::/96
	jool_siit eamt add 2001:db8::1 192.0.2.1
	jool_siit eamt add 2001:db8::2 192.0.2.2
	jool_siit eamt add 2001:db8::3 192.0.2.3

- The IPv4 Internet is known by the IPv6 Data Center as the `64:ff9b:1::/96` network.
- Include one EAMT entry for every Data Center server you need accesible from IPv4. (Eg. Server `2001:db8::1` will be known by IPv4 clients as `192.0.2.1`.)

## SIIT-DC Dual Translation

BR:

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_siit
	jool_siit instance add --netfilter --pool6 64:ff9b:1::/96
	jool_siit eamt add 2001:db8::1 192.0.2.1
	jool_siit eamt add 2001:db8::2 192.0.2.2
	jool_siit eamt add 2001:db8::3 192.0.2.3
	jool_siit eamt add 2001:db8::4 192.0.2.4

- Same as SIIT-DC, except you also have an IPv4 server (192.0.2.4) in your Data Center, and you're pretending it's another IPv6 server.

ER:

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_siit
	jool_siit instance add --netfilter --pool6 64:ff9b:1::/96
	jool_siit eamt add 2001:db8::4 192.0.2.4

- Used by the IPv4 server to reach (and be reached by) the DC's IPv6 backbone.

## Stateful NAT64

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool
	jool instance add --netfilter --pool6 64:ff9b:1::/96

- The IPv4 Internet is the `64:ff9b:1::/96` network.
- The translator will mask the IPv6 network with its own address.
- Similar to a NAPT, only the "private" side (ie. the IPv6 network) can initiate communication.
	- Unless you set up [Port Fortwarding](usr-flags-bib.html).

## MAP-T

BR:

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_mapt
	jool_mapt instance add --netfilter --dmr 64:ff9b:1::/96
	jool_mapt fmrt add 2001:db8::/32 192.0.2.0/24 16 6

- Include one FMR for every connected MAP domain.

CE:

	sysctl -w net.ipv4.conf.all.forwarding=1
	sysctl -w net.ipv6.conf.all.forwarding=1

	modprobe jool_mapt
	jool_mapt instance add --netfilter --dmr 64:ff9b:1::/96
	jool_mapt global update end-user-ipv6-prefix 2001:db8:AAAA::/48
	jool_mapt global update bmr 2001:db8::/32 192.0.2.0/24 16 6
	jool_mapt global update map-t-type CE

- You can also add FMRs, in case you want to connect this CE to other MAP domains without needing the BR's help.

