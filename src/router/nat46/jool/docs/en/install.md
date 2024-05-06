---
language: en
layout: default
category: Documentation
title: Installation
---

[Documentation](documentation.html) > [Installation](documentation.html#installation) > Other distros

# Jool Installation

## Index

1. [Introduction](#introduction)
2. [Updating your system](#updating-your-system)
2. [Installing Dependencies](#installing-dependencies)
3. [Downloading the Code](#downloading-the-code)
4. [Compilation and Installation](#compilation-and-installation)
5. [Uninstalling](#uninstalling)

## Introduction

A full installation of Jool is eleven binaries:

- [Kernel modules](https://en.wikipedia.org/wiki/Loadable_kernel_module):
	- `jool.ko`, `jool_siit.ko` and `jool_common.ko`: The Stateful NAT64, the SIIT and the functionality that is shared between the previous two. They are the actual translators and do most of the work.
- [Userspace](https://en.wikipedia.org/wiki/User_space) tools:
	- `jool` and `jool_siit`: Two console clients which can be used to configure the modules above.
	- `joold`: An userspace daemon that can synchronize state between different NAT64 Jool instances.
- Userspace libraries:
	- `libxt_JOOL.so` and `libxt_JOOL_SIIT.so`: Two shared objects that enable Jool-themed iptables rules.
	- `libjoolargp.la`, `libjoolnl.la` and `libjoolutil.la` (extensions may vary): Three shared libraries containing common functionality for the other userspace components.

This document will explain how to compile and install all of that on most Linux distributions.

In following console segments, `$` indicates the command can be executed freely; `#` means it requires admin privileges.

## Updating your system

This is not always necessary, but aside from fetching security patches, it maximizes the probability of easily acquiring the proper kernel headers later.

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt update
user@T:~# apt upgrade
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum update
 
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper update
 
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -Syy
user@T:~# pacman -Su
{% endhighlight %}

If you got a new kernel, best load it:

{% highlight bash %}
user@T:~# /sbin/reboot
{% endhighlight %}

## Installing Dependencies

Of course, I can't list exact dependencies for every single distribution, but here is a small list compiled from previous troubleshooting experiences. It should at least get you started if your environment is different.

> This section was last revised in 2020-01-08, using the following Linux flavors:
>
> - Debian 10.2 (`cat /etc/debian_version`)
> - Ubuntu 18.04.3 LTS (`lsb_release -a`, `cat /etc/os-release`, `hostnamectl`)
> - Raspberri Pi Desktop (Debian Buster) September 2019 (This is just the name of the ISO; Raspberri does not seem to have a dedicated version string. `/etc/os-release` refers to Debian 10 and `/etc/debian_version` says `10.2`.)
> - CentOS 7.7.1908 (Core) (`cat /etc/system-release`, `cat /etc/centos-release`)
> - OpenSUSE Leap 15.1 (`cat /usr/lib/os-release`)
> - Arch Linux 5.4.8-arch1-1 (Found at the top of the login prompt. Might not be the right one, because it's the same as `uname -r`. Dunno; can't find a better string.)
>
> For Ubuntu and Raspberri, use the Debian tabs.

First, you need your build essentials:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install build-essential pkg-config
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install gcc make elfutils-libelf-devel
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install gcc
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -S base-devel
{% endhighlight %}

The modules need your kernel headers (and make sure your kernel is [supported](intro-jool.html#compatibility) by your version of Jool):

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install linux-headers-$(uname -r)
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install kernel-devel
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# # Unneeded
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -S linux-headers
{% endhighlight %}

The userspace clients and the daemon need the [Development Library and Headers for libnl-genl-3](http://www.infradead.org/~tgr/libnl/):

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install libnl-genl-3-dev
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install libnl3-devel
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install libnl3-devel
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# # Installed by default, apparently
{% endhighlight %}

The iptables shared objects need the [Netfilter xtables Library development files](http://www.netfilter.org/):

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install libxtables-dev # It's called iptables-dev in Ubuntu 16.04
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install iptables-devel
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install libxtables-devel
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# # Installed by default, apparently
{% endhighlight %}

You also want DKMS, for automatic module rebuild during kernel updates:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install dkms
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# # https://fedoraproject.org/wiki/EPEL
user@T:~# yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
user@T:~# yum install dkms
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install dkms
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -S dkms
{% endhighlight %}

If you're going to clone the git repository, you need git and the autotools:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install git autoconf libtool
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install git automake libtool
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install git-core automake autoconf libtool
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -S git # autotools already included in base-devel.
{% endhighlight %}

But if you prefer to install the official release, you will need a `.tar.gz` extraction tool. Most distros include this by default, but just for completeness sake,

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">CentOS</span>
	<span class="distro-selector" onclick="showDistro(this);">openSUSE</span>
	<span class="distro-selector" onclick="showDistro(this);">Arch</span>
</div>

<!-- Debian -->
{% highlight bash %}
user@T:~# apt install tar
{% endhighlight %}

<!-- CentOS -->
{% highlight bash %}
user@T:~# yum install tar
{% endhighlight %}

<!-- openSUSE -->
{% highlight bash %}
user@T:~# zypper install tar
{% endhighlight %}

<!-- Arch -->
{% highlight bash %}
user@T:~# pacman -S tar
{% endhighlight %}

## Downloading the Code

You have two options:

1. Official tarballs hosted at [Downloads](download.html).
2. Cloning the [Git repository]({{ site.repository-url }}).

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">tarballs</span>
	<span class="distro-selector" onclick="showDistro(this);">git clone</span>
</div>

<!-- tarballs -->
{% highlight bash %}
$ wget https://github.com/NICMx/Jool/releases/download/v{{ site.latest-version }}/jool-{{ site.latest-version }}.tar.gz
$ tar -xzf jool-{{ site.latest-version }}.tar.gz
{% endhighlight %}

<!-- git clone -->
{% highlight bash %}
$ git clone https://github.com/NICMx/Jool.git
 
{% endhighlight %}

The repository version sometimes includes slight bugfixes not present in the latest official tarball, which you can access by sticking to the latest commit of the `main` branch. (Tarballs and `main` are considered stable, other branches are development.)

## Compilation and Installation

> ![Warning!](../images/warning.svg) Please note: If you have previously installed Jool from a package (.deb or otherwise), the binaries installed here will not necessarily override them. It supposedly depends on distro, but Source Jool and package Jool tend to be installed in different locations by default. This means that, if you've previously installed from a package, both versions will coexist in your system, and the one you actually run will not necessarily be the one you expect.
> 
> To minimize confusion, it is strongly recommended that you uninstall package Jool before proceeding.

The kernel modules and the userspace applications need to be compiled and installed separately.

This is how you compile and install the kernel modules:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">tarball</span>
	<span class="distro-selector" onclick="showDistro(this);">git clone</span>
</div>

<!-- tarball -->
{% highlight bash %}
user@T:~# /sbin/dkms install jool-{{ site.latest-version }}/
{% endhighlight %}

<!-- git clone -->
{% highlight bash %}
user@T:~# /sbin/dkms install Jool/
{% endhighlight %}

And this is how you compile and install the userspace applications:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">tarball</span>
	<span class="distro-selector" onclick="showDistro(this);">git clone</span>
</div>

<!-- tarball -->
{% highlight bash %}
user@T:~$ cd jool-{{ site.latest-version }}/
user@T:~$
user@T:~$ ./configure
user@T:~$ make
user@T:~# make install
{% endhighlight %}

<!-- git clone -->
{% highlight bash %}
user@T:~$ cd Jool/
user@T:~$ ./autogen.sh
user@T:~$ ./configure
user@T:~$ make
user@T:~# make install
{% endhighlight %}

## Uninstalling

### Userspace Clients

Simply run `make uninstall` in the directory where you compiled them:

```bash
user@T:~$ cd jool-{{ site.latest-version }}/
user@T:~# make uninstall
```

If you no longer have the directory where you compiled them, download it again and do this instead:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">tarball</span>
	<span class="distro-selector" onclick="showDistro(this);">git clone</span>
</div>

<!-- tarball -->
```bash
user@T:~$ cd jool-{{ site.latest-version }}/
user@T:~$
user@T:~$ ./configure
user@T:~# make uninstall
```

<!-- git clone -->
```bash
user@T:~$ cd Jool/
user@T:~$ ./autogen.sh
user@T:~$ ./configure
user@T:~# make uninstall
```

### Kernel Modules (if installed by DKMS)

Use `dkms remove`. Here's an example in which I'm trying to remove version 4.0.1:

```bash
$ /sbin/dkms status
jool, 4.0.1.git.v4.0.1, 4.15.0-54-generic, x86_64: built
jool, 4.0.6.git.v4.0.6, 4.15.0-54-generic, x86_64: installed
$
$ sudo /sbin/dkms remove jool/4.0.1.git.v4.0.1 --all

-------- Uninstall Beginning --------
Module:  jool
Version: 4.0.1.git.v4.0.1
Kernel:  4.15.0-54-generic (x86_64)
-------------------------------------

Status: This module version was INACTIVE for this kernel.
depmod...

DKMS: uninstall completed.

------------------------------
Deleting module version: 4.0.1.git.v4.0.1
completely from the DKMS tree.
------------------------------
Done.
$
$ /sbin/dkms status
jool, 4.0.6.git.v4.0.6, 4.15.0-54-generic, x86_64: installed
```

### Kernel Modules (if installed by Kbuild in accordance with old documentation)

Delete the `.ko` files and re-index by way of `depmod`:

```bash
$ sudo rm /lib/modules/$(uname -r)/extra/jool_siit.ko
$ sudo rm /lib/modules/$(uname -r)/extra/jool.ko
$ sudo rm -f /lib/modules/$(uname -r)/extra/jool_common.ko
$ sudo depmod
```
