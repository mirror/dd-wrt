---
language: en
layout: default
category: Documentation
title: Debian
---

[Documentation](documentation.html) > [Installation](documentation.html#installation) > Debian

# Jool in Debian

You have three options: Installing the _Debian Release_, the _Standalone Package_ or compiling from source.

Here's a comparison between the three:

| | Debian Release | Standalone Package | Source |
|-|----------------|------------|--------|
| Available in Debian | yes | yes | yes |
| Available in some of Debian's derivatives<br />(such as Ubuntu) | no | yes | yes |
| Available in the amd64 architecture | yes | yes | yes |
| Available in other architectures | yes | no | yes |
| Automatic updates (through Debian) | yes | no | no |
| Latest version always available | no `*` | yes | yes |

At time of writing (2019-11-21), the Debian Release is present in the [`unstable`](https://wiki.debian.org/DebianUnstable) and [`testing`](https://wiki.debian.org/DebianTesting) Debian release branches. For the time being, Jool is stuck there due to being a recent addition to Debian. Please note that adding these branches to your sources puts your entire system in bleeding edge territory; If this is an issue for you, consider apt pinning or the other options.

`*` After an official Jool release, its corresponding latest Debian Release might take up to a few days to be approved and served by Debian.

This document explains how to install the Debian Release and the Standalone Package. To compile from source, visit [this page](install.html).

## Installing the Debian packages

> ![Warning!](../images/warning.svg) If you have already installed Jool from source in your machine, then those binaries may conflict with the ones installed here.
>
> You may uninstall source-installed binaries by following [these steps](install.html#uninstalling).

Make sure you have your current kernel headers:

```bash
user@T:~# apt install linux-headers-$(uname -r)
```

Then choose whether you want the Debian Release or the Standalone Package:

### Debian Release

Follow [these steps](https://wiki.debian.org/DebianUnstable#Installation) to switch to `unstable`, or [these steps](https://wiki.debian.org/DebianTesting#How_to_use_Debian_.28next-stable.29_Testing) to switch to `testing`, then install like any other formal Debian package:

```bash
user@T:~# apt update
user@T:~# apt install jool-dkms jool-tools
```

Here's a quick link back to the [basic tutorials list](documentation.html#basic-tutorials).

### Standalone Package

Download the standalone `.deb` packages from [Downloads](download.html) and install them like so:

```bash
user@T:~# apt install ./jool-dkms_{{ site.latest-version }}-1_all.deb ./jool-tools_{{ site.latest-version }}-1_amd64.deb
```

These have been tested in Debian 10 and Ubuntu 20.04.

Here's a quick link back to the [basic tutorials list](documentation.html#basic-tutorials).

## Uninstallation

Whether you installed the Debian Release or the Standalone Package, the procedure is the same:

```bash
user@T:~# apt remove jool-dkms jool-tools
```

