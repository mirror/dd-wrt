---
language: en
layout: default
category: Documentation
title: BIB
---

[Documentation](documentation.html) > [NAT64 in Detail](documentation.html#nat64-in-detail) > BIB

# BIB

## Index

1. [What is BIB?](#what-is-bib)
2. [Terminology](#terminology)
3. [Examples](#examples)
4. [Further Reading](#further-reading)

## What is BIB? 

The _Binding Information Base_ (BIB) is a collection of tables in a stateful NAT64. It is defined in [RFC 6146](http://tools.ietf.org/html/rfc6146#section-3.1).

A record in this database defines the relationship between the transport address of an IPv6 socket and the IPv4 transport address Jool is using to mask it.

> ![Note](../images/bulb.svg) Strictly speaking, a "transport address" is an IP address and a port. The port part tends to imply it represents a TCP or UDP connection.
> 
> For all implementation intents and purposes, ICMP behaves just like a layer-4 protocol. For this reason, we extend the expression "transport address" to ICMP. ICMP does not have ports, so when we say "ICMP transport address" we actually mean an IP address and an ICMP identifier.

> ![Warning](../images/warning.svg) BIB only exposes information regarding IPv6 nodes; you can't say a BIB entry can define the relationship between the transport address of an IPv4 socket and the IPv6 transport address Jool is using to "mask" it.
> 
> This information can be found in the [session tables](usr-flags-session.html).

An IPv4 transport address mask is always one that belongs to Jool. Therefore, the IPv4 transport address of the BIB entries normally belong to [pool4](pool4.html).

> ![Note](../images/bulb.svg) Sometimes you might find BIB entries that hold IPv4 transport addresses that might not belong to pool4. This can happen if you recently removed pool4 entries using [`--quick`](usr-flags-pool4.html#--quick).

## Terminology

* We call "**BIB entry**" a record in a BIB table (ie. a descriptor of a mask). It contains three fields: An IPv6 transport address, its IPv4 mask (also a transport address), and a type (Static or Dynamic).
	- When an IPv6 node tries to speak to an IPv4 node, Jool needs to create a mapping so communication can happen. Entries are called "**Dynamic**" when they are automatically created as they are needed (and are deleted automatically as they expire).
	- Manually created mappings can be used to grant permanent names to IPv6 sockets. These masks do not expire automatically and IPv4 nodes can reliably use them to start communication. These entries are called "**Static**", and are analogous to [port forwarding](https://en.wikipedia.org/wiki/Port_forwarding) in normal NATs.
* We call "**BIB table**" a collection of records which share a protocol. There are three supported protocols (TCP, UDP and ICMP), therefore Jool has three BIB tables.
* We call "**BIB**" the collection of Jool's three BIB tables.

## Examples

Except for the "Entry ID" column, this is how a BIB table can look like:

| Entry ID | IPv6 transport address | IPv4 transport address | Protocol |
|----------|------------------------|------------------------|----------|
|    01    | 2001:db8::1#40000      | 192.0.2.4#40000        | Dynamic  |
|    02    | 2001:db8::2#40000      | 198.51.100.10#50000    | Dynamic  |
|    03    | 2001:db8::3#80         | 203.0.113.43#80        | Static   |

### Entry 01

This record claims the 2001:db8::1 node (using port 40000) is interacting with IPv4. The NAT64 is fooling at least one IPv4 node into thinking port 40000 2001:db8::1 is actually port 40000 of 192.0.2.4.

Again, this doesn't tell us who 2001:db8::1 is talking to.

### Entry 02

02 is similar to 01, except the masking port is different from the original port. As mentioned in [pool4](pool4.html), NAT64s don't care about preserving ports, so this record is actually more natural than the previous one.

Record 02 claims there is (or was, recently) a socket in port 40000 of address 2001:db8::2, and at least one IPv4 node thinks it is 50000 in 198.51.100.10.

### Entry 03

This record was created manually. The ports suggest it refers to some HTTP service the operator wants available on IPv4.

This entry will not expire automatically, so IPv4 clients can reliably query the server via 203.0.113.43 port 80.

## Further Reading

- You can upload static BIB entries via [`bib`](usr-flags-bib.html).

