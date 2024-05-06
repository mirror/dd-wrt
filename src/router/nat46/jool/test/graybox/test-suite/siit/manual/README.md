# Graybox Tests: manual

In contrast with the "auto" (pktgen) tests, the manual tests were generated manually. They are a bunch of packet exchanges improvised after the auto tests, but before Graybox became a more formal endeavor.

## 6791v64

Test of pool6791v4, populated version.

	packet 6791v64t
		40	IPv6	src:4000::1
		8	ICMPv6
		40	IPv6	swap
		8	UDP
		32	Payload

	packet 6791v64e
		20	IPv4	!df ttl-- swap src:203.0.113.8
		8	ICMPv4
		20	IPv4	!df
		8	UDP
		32	Payload

Source address is untranslatable, so it gets assigned the pool6791v4 entry.

## 6791v64-empty

Test of pool6791v64, empty version.

	packet 6791v64t-empty
		Same as 6791v64t.

	packet 6791v64e-empty
		20	IPv4	!df ttl-- swap src:198.51.100.1
		8	ICMPv4
		20	IPv4	!df
		8	UDP
		32	Payload

Environment: Empty pool6791v4.

## 6791v46

- Set `pool6` to null
- Set `rfc6791v6-prefix` to 2::2
- Add EAM entry: 198.51.100 | 2001:db8:1c6:3364::/72
- Add EAM entry: 192.0.2 | 2001:db8:1c0:2/72

	packet 6791v46t
		20	IPv4	src:2.0.0.2
		8	ICMPv4
		20	IPv4	ttl-- swap
		8	UDP
		4	Payload

	packet 6791v46e
		40	IPv6	ttl-- src:2::2 dst:2001:db8:1c0:2:21::
		8	ICMPv6
		40	IPv6	ttl--
		8	UDP
		4	Payload

## 6791v46-empty

- Set pool6 to null
- Set rfc6791v6-prefix to null
- Add EAM entry: 198.51.100 | 2001:db8:1c6:3364::/72
- Add EAM entry: 192.0.2 | 2001:db8:1c0:2/72

	packet 6791v46t-empty
		Same as 6791v46t.

	packet 6791v46e-empty
		40	IPv6	ttl-- src:2001:db8:1c0:2:1:: dst:2001:db8:1c0:2:21::
		8	ICMPv6
		40	IPv6	ttl--
		8	UDP
		4	Payload

## 6791v66

This is an amalgamation between the complications of ICMP errors, the RFC 6791v4 pool and hairpinning. Each might already have dedicated tests, but an aggregated version is welcomed. This is the intended story:

Original packet:

	2001:db8:3::1 -> 2001:db8:01[10.0.0.[0].10]::

Translates into

	1.0.0.1 -> 10.0.0.10

Hairpin, therefore

	2001:db8:01[1.0.0.[0].1]:: -> 2001:db8:2::a

Random IPv6 router triggers ICMP error:

	Outer packet:
		2001:db8::5 -> 2001:db8:01[1.0.0.[0].1]::
	Internal packet:
		2001:db8:01[1.0.0.[0].1]:: -> 2001:db8:2::a

Translates into

	Outer packet:
		<pool6791v4> -> 1.0.0.1
	Internal packet:
		1.0.0.1 -> 10.0.0.10

Hairpin, therefore

	Outer packet:
		2001:db8:01[pool6791v4]:: -> 2001:db8:3::1
	Internal packet:
		2001:db8:3::1 -> 2001:db8:01[10.0.0.[0].10]::

So:

	packet 6791v66t
		40	IPv6		src:2001:db8::5 dst:2001:db8:101:0:1::
		8	ICMPv6		type:1 code:0
		40	IPv6		src:2001:db8:101:0:1:: dst:2001:db8:2::a
		8	UDP
		4	Payload

	packet 6791v66e
		40	IPv6		src:2001:db8:1cb:71:8:: dst:2001:db8:3::1 ttl--
		8	ICMPv6		type:1 code:0
		40	IPv6		src:2001:db8:3::1 dst:2001:db8:10a:0:a::
		8	UDP
		4	Payload
