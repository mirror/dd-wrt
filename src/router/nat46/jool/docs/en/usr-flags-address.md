---
language: en
layout: default
category: Documentation
title: address Mode
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > `address` Mode

# `address` Mode

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
   1. [`<IP Address>`](#ip-address)
   2. [Flags](#flags)
4. [Examples](#examples)

## Description

Sends address translation queries to the module. Meant for educative and configuration testing purposes.

Only SIIT Jool implements this feature for now.

## Syntax

	jool_siit address query [--verbose] <IP Address>

If `<IP Address>` is an IPv4 address, prints its translated IPv6 version. If `<IP Address>` is an IPv6 address, prints its translated IPv4 version.

The translation is performed according to the instance's [EAMT table](eamt.html) and [RFC6052 prefix](pool6.html).

## Arguments

### `<IP Address>`

The address you want to translate.

### Flags

| **Flag** | **Description** |
| `--verbose` | Print some details regarding the translation operation. |

## Examples

Configuration:

```bash
$ jool_siit instance add --iptables --pool6 64:ff9b::/96
$ jool_siit eamt add 192.0.2.1      2001:db8:aaaa::
$ jool_siit eamt add 192.0.2.2/32   2001:db8:bbbb::b/128
$ jool_siit eamt add 192.0.2.16/28  2001:db8:cccc::/124
$ jool_siit eamt add 192.0.2.128/26 2001:db8:dddd::/64
$ jool_siit eamt add 192.0.2.192/29 2001:db8:eeee:8::/62
$ jool_siit eamt add 192.0.2.224/31 64:ff9b::/127
```

Queries:

```bash
$ jool_siit address query 192.0.2.1
2001:db8:aaaa::
$
$ jool_siit address query 2001:db8:aaaa::
192.0.2.1
$
$ jool_siit address query --verbose 192.0.2.225
  Query: 192.0.2.225
  Scheme: EAMT
    EAM: 64:ff9b::/127 | 192.0.2.224/31
  Operation: 192.0.2.225 - 192.0.2.224/31 + 64:ff9b::/127 = 64:ff9b::1
  Result: 64:ff9b::1
$
$ jool_siit address query --verbose 64:ff9b::c000:2f8
  Query: 64:ff9b::c000:2f8
  Scheme: RFC 6052 prefix
    Prefix: 64:ff9b::/96
  Operation: 64:ff9b::c000:2f8 - 64:ff9b::/96 = 192.0.2.248
  Result: 192.0.2.248
$
$ jool_siit address query 2001:db8::1
Error: The kernel module returned error 22: The input address lacks both pool6 prefix and EAM.
```
