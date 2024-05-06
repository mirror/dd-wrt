---
language: en
layout: default
category: Documentation
title: Atomic Configuration
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > Atomic Configuration

# Atomic Configuration

## Index

1. [Introduction](#introduction)
2. [Syntax](#syntax)
2. [Semantics](#semantics)
4. [Examples](#examples)
	1. [SIIT](#siit)
	2. [NAT64](#nat64)

## Introduction

"Atomic Configuration" is a means to set up more than one of Jool's parameters at once (using a single `jool`/`jool_siit` call). Either all or none of the new configuration will be applied, so you don't have to worry about rolling back.

You can also think of it as "file configuration mode," since [JSON](https://www.json.org/) files are the means through which Atomic Configuration is retrieved.

## Syntax

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">SIIT</span>
	<span class="distro-selector" onclick="showDistro(this);">NAT64</span>
</div>

<!-- SIIT -->
{% highlight bash %}
jool_siit [-i <instance name>] file handle <path to json file> [--force]
{% endhighlight %}

<!-- NAT64 -->
{% highlight bash %}
jool      [-i <instance name>] file handle <path to json file> [--force]
{% endhighlight %}

`--force` silences warnings. (If you don't silence them, sometimes they will cause operation abortion; eg. [overlapping EAM entries](usr-flags-eamt.html#overlapping-eam-entries).)

## Semantics

The file describes one Jool instance. If the instance does not exist, it will be created. If it does exist, it will be updated. It will be an ordinary instance; you can subsequently apply any non-atomic operations on it, and delete it using [`instance remove`](usr-flags-instance.html) as usual.

Most of the options are the same as their userspace client counterparts, so see [Examples](#examples) for a couple of full JSON files with embedded links to the relevant client documentation.

On the top level, the mandatory fields are the instance name (either through the `-i` client argument or the `instance` JSON tag) and the `framework` tag (which must be set to either [`netfilter`](intro-jool.html#netfilter) or [`iptables`](intro-jool.html#iptables)).

Aside from vital fields from individual entries, everything else is optional, and will be initialized (or reinitialized) to **default values** ([NOT "old" values!](#changes-from-jool-3)) on omission.

Unrecognized tags will trigger errors, but any amount of `comment`s are allowed (and ignored) on all object contexts.

## Examples

### SIIT

<pre><code>{
	"comment": {
		"description": "Sample full SIIT configuration.",
		"notes": [
			"192.0.2/24, 198.51.100/24 and 2001:db8::/32 are",
			"documentation blocks (RFC 5737 and RFC 3849), and you",
			"WILL need to change or remove them for your setup."
		],
		"last update": "2022-02-09"
	},
	
	"instance": "sample-siit",
	"framework": "netfilter",

	"global": {
		"<a href="usr-flags-global.html#manually-enabled">manually-enabled</a>": true,
		"<a href="usr-flags-global.html#pool6">pool6</a>": "2001:db8:0::/96",
		"<a href="usr-flags-global.html#lowest-ipv6-mtu">lowest-ipv6-mtu</a>": 1280,
		"<a href="usr-flags-global.html#logging-debug">logging-debug</a>": false,
		"<a href="usr-flags-global.html#zeroize-traffic-class">zeroize-traffic-class</a>": false,
		"<a href="usr-flags-global.html#override-tos">override-tos</a>": false,
		"<a href="usr-flags-global.html#tos">tos</a>": 0,
		"<a href="usr-flags-global.html#mtu-plateaus">mtu-plateaus</a>": [
			65535, 32000, 17914, 8166,
			4352, 2002, 1492, 1006,
			508, 296, 68
		],
		"<a href="usr-flags-global.html#amend-udp-checksum-zero">amend-udp-checksum-zero</a>": false,
		"<a href="usr-flags-global.html#eam-hairpin-mode">eam-hairpin-mode</a>": "intrinsic",
		"<a href="usr-flags-global.html#randomize-rfc6791-addresses">randomize-rfc6791-addresses</a>": true,
		"<a href="usr-flags-global.html#rfc6791v6-prefix">rfc6791v6-prefix</a>": null,
		"<a href="usr-flags-global.html#rfc6791v4-prefix">rfc6791v4-prefix</a>": null
	},

	"<a href="usr-flags-eamt.html">eamt</a>": [
		{
			"comment": "Comments allowed here too.",
			"ipv6 prefix": "2001:db8:1::/128",
			"ipv4 prefix": "192.0.2.0"
		}, {
			"ipv6 prefix": "2001:db8:2::",
			"ipv4 prefix": "192.0.2.1/32"
		}, {
			"ipv6 prefix": "2001:db8:3::/124",
			"ipv4 prefix": "192.0.2.16/28"
		}
	],

	"comment": "This comment is relevant to denylist4 maybe.",
	"<a href="usr-flags-denylist4.html">denylist4</a>": [
		"198.51.100.0",
		"198.51.100.2/32",
		"198.51.100.32/27"
	]
}</code></pre>

Conceptually, updating an SIIT instance through atomic configuration is the same as dropping it and creating it anew. In practice, the former prevents the small window of time in which no translator exists.

Minimal SIIT configuration:

<pre><code>{
	"instance": "siit-minimal",
	"framework": "netfilter"
}</code></pre>

### NAT64

There is one major caveat here: The current implementation of BIB/session is [not suitable to guarantee the atomicity of simultaneous modifications to a running database](https://github.com/NICMx/Jool/blob/v3.5.0/usr/common/target/json.c#L715). Therefore, **the `bib` tag below is only handled if the JSON file is being used to create a new instance. It will be silently ignored on updates**.

Sorry. This does not necessarily mean that atomic updating of static BIB entries will never be implemented, but there are no plans for now.

Also, `pool6` is mandatory and immutable (as normal). It must be set during instance creation and retain the same value on subsequent updates.

<pre><code>{
	"comment": {
		"description": "Sample full NAT64 configuration.",
		"notes": [
			"192.0.2/24 and 2001:db8::/32 are documentation blocks",
			"(RFC 5737 and RFC 3849), and you WILL need to change or",
			"remove them for your setup."
		],
		"last update": "2022-02-09"
	},

	"instance": "sample-nat64",
	"framework": "netfilter",

	"global": {
		"<a href="usr-flags-global.html#manually-enabled">manually-enabled</a>": true,
		"<a href="usr-flags-global.html#pool6">pool6</a>": "64:ff9b::/96",
		"<a href="usr-flags-global.html#lowest-ipv6-mtu">lowest-ipv6-mtu</a>": 1280,
		"<a href="usr-flags-global.html#logging-debug">logging-debug</a>": false,
		"<a href="usr-flags-global.html#zeroize-traffic-class">zeroize-traffic-class</a>": false,
		"<a href="usr-flags-global.html#override-tos">override-tos</a>": false,
		"<a href="usr-flags-global.html#tos">tos</a>": 0,
		"<a href="usr-flags-global.html#mtu-plateaus">mtu-plateaus</a>": [
			65535, 32000, 17914, 8166,
			4352, 2002, 1492, 1006,
			508, 296, 68
		],
		"<a href="usr-flags-global.html#address-dependent-filtering">address-dependent-filtering</a>": false,
		"<a href="usr-flags-global.html#drop-externally-initiated-tcp">drop-externally-initiated-tcp</a>": false,
		"<a href="usr-flags-global.html#drop-icmpv6-info">drop-icmpv6-info</a>": false,
		"<a href="usr-flags-global.html#source-icmpv6-errors-better">source-icmpv6-errors-better</a>": true,
		"<a href="usr-flags-global.html#f-args">f-args</a>": 11,
		"<a href="usr-flags-global.html#handle-rst-during-fin-rcv">handle-rst-during-fin-rcv</a>": false,
		"<a href="usr-flags-global.html#tcp-est-timeout">tcp-est-timeout</a>": "2:00:00",
		"<a href="usr-flags-global.html#tcp-trans-timeout">tcp-trans-timeout</a>": "0:04:00",
		"<a href="usr-flags-global.html#udp-timeout">udp-timeout</a>": "0:05:00",
		"<a href="usr-flags-global.html#icmp-timeout">icmp-timeout</a>": "0:01:00",
		"<a href="usr-flags-global.html#logging-bib">logging-bib</a>": false,
		"<a href="usr-flags-global.html#logging-session">logging-session</a>": false,
		"<a href="usr-flags-global.html#maximum-simultaneous-opens">maximum-simultaneous-opens</a>": 10,
		"<a href="usr-flags-global.html#ss-enabled">ss-enabled</a>": false,
		"<a href="usr-flags-global.html#ss-flush-asap">ss-flush-asap</a>": true,
		"<a href="usr-flags-global.html#ss-flush-deadline">ss-flush-deadline</a>": 2000,
		"<a href="usr-flags-global.html#ss-capacity">ss-capacity</a>": 512,
		"<a href="usr-flags-global.html#ss-max-payload">ss-max-payload</a>": 1452,
		"<a href="usr-flags-global.html#ss-max-sessions-per-packet">ss-max-sessions-per-packet</a>": 10
	},

	"<a href="usr-flags-pool4.html">pool4</a>": [
		{
			"comment": "mark, port range and max-iterations are optional.",
			"protocol": "TCP",
			"prefix": "192.0.2.1/32"
		}, {
			"protocol": "TCP",
			"prefix": "192.0.2.1/32",
			"port range": "80"
		}, {
			"mark": 0,
			"protocol": "UDP",
			"prefix": "192.0.2.1/32",
			"port range": "61001-62000",
			"max-iterations": 1500
		}, {
			"mark": 0,
			"protocol": "ICMP",
			"prefix": "192.0.2.1/32",
			"port range": "1000-2000"
		}
	],

	"<a href="usr-flags-bib.html">bib</a>": [
		{
			"protocol": "TCP",
			"ipv6 address": "2001:db8::1#80",
			"ipv4 address": "192.0.2.1#80"
		}, {
			"protocol": "UDP",
			"ipv6 address": "2001:db8::2#10000",
			"ipv4 address": "192.0.2.1#61500"
		}, {
			"protocol": "ICMP",
			"ipv6 address": "2001:db8:AAAA::1#44",
			"ipv4 address": "192.0.2.1#1044"
		}
	]
}</code></pre>

Updating a NAT64 instance through atomic configuration is not the same as dropping the instance and then creating another one in its place. Aside from skipping the translatorless time window through the former, you get to keep the BIB/session database.

Minimal NAT64 configuration:

<pre><code>{
	"instance": "nat64-minimal",
	"framework": "netfilter",

	"global": {
		"<a href="usr-flags-global.html#pool6">pool6</a>": "64:ff9b::/96"
	}
}</code></pre>

