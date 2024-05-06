---
language: en
layout: default
category: Documentation
title: stats Mode
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > `stats` Mode

# `stats` mode

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
   1. [Operations](#operations)
   2. [Options](#options)
4. [Examples](#examples)

## Description

Prints a bunch of counters tracked by the Jool instance.

They will give you an overview of the instance's view on your traffic. In particular, if Jool seems to be dropping packets, the respective counter should tell you why. (As long as Jool is the one dropping them, not the kernel.)

More than anything, `stats` is intended for short-term troubleshooting purposes. The counters, their IDs and their descriptions are not set in stone, and can differ across different Jool versions.

As of version 4.1.5, `stats` is Jool's only kernelside operation handler that does not require `CAP_NET_ADMIN` privileges.

## Syntax

	(jool_siit | jool) stats (
		display [--all] [--explain] [--csv] [--no-headers]
	)

## Arguments

### Operations

* `display`: Print the counters in standard output.

### Options

| Flag           | Description                                                                 |
|----------------|-----------------------------------------------------------------------------|
| `--all`        | Print all the counters known to Jool. (Not just the ones that aren't zero.) |
| `--explain`    | Also print an explanation of each counter.                                  |
| `--csv`        | Print the table in [_Comma/Character-Separated Values_ format](http://en.wikipedia.org/wiki/Comma-separated_values). This is intended to be redirected into a .csv file. |
| `--no-headers` | Do not print table headers (when `--csv` is active).                        |

## Examples

{% highlight bash %}
user@T:~# jool stats display
JSTAT_SUCCESS: 35
JSTAT_BIB_ENTRIES: 5
JSTAT_SESSIONS: 8
JSTAT_BIB4_NOT_FOUND: 1
JSTAT_FAILED_ROUTES: 1
JSTAT_PKT_TOO_BIG: 2
JSTAT_ICMP6ERR_SUCCESS: 1


user@T:~# jool stats display --explain
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

JSTAT_FAILED_ROUTES: 1
The translated packet could not be routed; the kernel's routing function
errored. Cause is unknown. (It usually happens because the packet's
destination address could not be found in the routing table.)

JSTAT_PKT_TOO_BIG: 2
Translations cancelled: Translated IPv4 packet did not fit in the
outgoing interface's MTU. A Packet Too Big or Fragmentation Needed ICMP
error was returned to the client.

JSTAT_ICMP6ERR_SUCCESS: 1
ICMPv6 errors (created by Jool, not translated) sent successfully.


user@T:~# jool stats display --csv --explain > stats.csv
{% endhighlight %}

[stats.csv](../obj/stats.csv)

## Time Series Data Options

### prometheus `jool-exporter`

A python `jool-exporter` binary exports data for prometheus collection.
The code is written as jool adds and removes stats they show up in the
exported data.

* `pip install jool-exporter`

It is recommended to run jool-exporter via SystemD.
Pull requests + enhancements welcome!

**More Information:**

* [GttHub Repo](https://github.com/cooperlees/jool-exporter)
  * [README](https://github.com/cooperlees/jool-exporter/blob/main/README.md)
* [PyPI](https://pypi.org/project/jool-exporter/)

*Please Note: This tool is not maintained by the core Jool team.*
