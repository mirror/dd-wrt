---
language: en
layout: default
category: Documentation
title: eamt Mode
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > `eamt` Mode

# `eamt` Mode

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
   2. [Operations](#operations)
   4. [Options](#options)
4. [Overlapping EAM Entries](#overlapping-eam-entries)
4. [Examples](#examples)

## Description

Interacts with Jool's Explicit Address Mapping Table (EAMT). See [the introduction](intro-xlat.html#siit-eamt) for a swift overview, our [RFC summary](eamt.html) for more details, or the [EAM RFC]({{ site.draft-siit-eam }}) for the full story.

## Syntax

	jool_siit eamt (
		display [--csv]
		| add <IPv4-prefix> <IPv6-prefix> [--force]
		| remove <IPv4-prefix> <IPv6-prefix>
		| flush
	)

## Arguments

### Operations

* `display`: The EAMT is printed in standard output.
* `add`: Combines `<IPv4-prefix>` and `<IPv6-prefix>` into an EAM entry, and uploads it to Jool's table.
* `remove`: Deletes from the table the EAM entry described by `<IPv4-prefix>` and/or `<IPv6-prefix>`.
* `flush`: Removes all entries from the table.

> ![Warning!](../images/warning.svg) If you want to add many EAM entries at once, `eamt add` might [turn out to be very slow](https://github.com/NICMx/Jool/issues/363). If you run into this problem, try adding them through [atomic configuration](config-atomic.html) instead.

### Options

| **Flag** | **Description** |
| `--csv` | Print the table in [_Comma/Character-Separated Values_ format](http://en.wikipedia.org/wiki/Comma-separated_values). This is intended to be redirected into a .csv file. |
| `--force` | Upload the entry even if overlapping occurs. (See the next section.) |

## Overlapping EAM entries

By default, EAMT entries are not allowed to overlap. You can use `--force` while `add`ing to override this property. When overlapping EAMT entries exist, Jool picks based on longest match prefix.

For example:

| IPv4 Prefix     |     IPv6 Prefix      |
|-----------------|----------------------|
| 192.0.2.0/24    | 2001:db8:aaaa::/120  |
| 192.0.2.8/29    | 2001:db8:bbbb::/125  |

Address `192.0.2.9` matches `192.0.2.8/29` better than `192.0.2.0/24`, so it will get translated as `2001:db8:bbbb::1`, not `2001:db8:aaaa::9`.

Notice this creates assymetry. `2001:db8:aaaa::9` gets translated as `192.0.2.9`, which in turn gets translated as `2001:db8:bbbb::1`. Depending on your use case, this can break communication.

Overlapping EAMT entries exist to help EAM coexist with [IVI](http://www.rfc-editor.org/rfc/rfc6219.txt). Other use cases might arise in the future.

## Examples

These examples below assume that the name of the Jool instance is "`default`."

Add a handful of mappings:

{% highlight bash %}
user@T:~# jool_siit eamt add 192.0.2.1      2001:db8:aaaa::
user@T:~# jool_siit eamt add 192.0.2.2/32   2001:db8:bbbb::b/128
user@T:~# jool_siit eamt add 192.0.2.16/28  2001:db8:cccc::/124
user@T:~# jool_siit eamt add 192.0.2.128/26 2001:db8:dddd::/64
user@T:~# jool_siit eamt add 192.0.2.192/31 64:ff9b::/127
{% endhighlight %}

Display the new table:

{% highlight bash %}
user@T:~# jool_siit eamt display
+---------------------------------------------+--------------------+
|                                 IPv6 Prefix |        IPv4 Prefix |
+---------------------------------------------+--------------------+
|                               64:ff9b::/127 |     192.0.2.192/31 |
|                         2001:db8:dddd::/64  |     192.0.2.128/26 |
|                         2001:db8:cccc::/124 |      192.0.2.16/28 |
|                        2001:db8:bbbb::b/128 |       192.0.2.2/32 |
|                         2001:db8:aaaa::/128 |       192.0.2.1/32 |
+---------------------------------------------+--------------------+
{% endhighlight %}

Dump the database on a CSV file:

{% highlight bash %}
user@T:~# jool_siit eamt display --csv > eamt.csv
{% endhighlight %}

[eamt.csv](../obj/eamt.csv)

Remove the first entry:

{% highlight bash %}
user@T:~# jool_siit eamt remove 2001:db8:aaaa::
{% endhighlight %}

Empty the table:

{% highlight bash %}
user@T:~# jool_siit eamt flush
{% endhighlight %}

