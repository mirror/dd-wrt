---
language: en
layout: default
category: Documentation
title: OpenWRT
---

[Documentation](documentation.html) > [Installation](documentation.html#installation) > OpenWRT

# Jool in OpenWRT/LEDE

## Index

1. [Introduction](#introduction)
2. [Installing Jool](#installing-jool)

## Introduction

Compiling and installing kernel modules is not the way things are meat to be done in OpenWRT. Fortunately, the OpenWRT folks are kind enough to provide official packages for Jool. If you intend to use this distribution, please keep in mind the notes in this document while following the rest of the tutorials in the documentation.

Please also note that these binaries are not maintained nor supervised by the Jool team. We are still available for advice if issues arise, however.

And finally: It might take an indeterminate amount of time for the latest version of Jool to be uploaded to OpenWRT's repository. Remember that you can find previous versions of this site's documentation in the [download page](download.html).

## Installing Jool

For OpenWRT 22.03 the netfilter version of Jool is available as pre-built packages and can configured and installed from the LuCI web interface.

### LuCI web console

1. System > Software: Install `jool-tools-netfilter` (this will install `kmod-jool-netfilter` and other dependencies).
2. System > Startup > Local Startup: Add the following to /etc/rc.local:
```
jool instance add --pool6 64:ff9b::/96
```
3. System > Reboot > Perform reboot
4. Confirm working NAT64 from a device inside your LAN `ping 64:ff9b::8.8.8.8`

### Command line

Using your router command line (e.g. SSH into the device).

```
# Install packages
opkg update
opkg install kmod-jool-netfilter jool-tools-netfilter
```

```
# Add the following line to /etc/rc.local (before the exit 0)
jool instance add --pool6 64:ff9b::/96
```

```
# Confirm working NAT64 from a device inside your LAN
ping 64:ff9b::8.8.8.8
```

To check Jool's version, run

```
jool --version
```

As of 2022-10-24, the above installs Jool 4.1.6.1, with "(Xtables disabled)".

## Note: older versions

On older versions there may be different packages, e.g. OpenWRT 21.02 had packages `kmod-jool` and `jool-tools` with some additional module and iptables setup. More details are in the past documentation.

