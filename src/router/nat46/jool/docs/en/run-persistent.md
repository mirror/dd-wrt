---
language: en
layout: default
category: Documentation
title: Persistence
---

# Persistence

## Index

1. [Introduction](#introduction)
2. [Debian](#debian)
3. [Not Debian](#not-debian)

## Introduction

If you installed Jool's [Debian package](debian.html), chances are your distribution manages daemons by way of systemd or System V. Jool's Debian package ships with systemd unit files and System V scripts you can use to enable Jool automatically after every boot.

If you did not install Jool's Debian package, you're in for a bit more trouble. You can download our Debian unit files or scripts and adapt them to your needs.

This document explains how to do both of these.

## Debian

First, provide a [configuration file](config-atomic.html) in `/etc/jool/`. You can find some examples in `/usr/share/doc/jool-tools/examples/`:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">SIIT</span>
	<span class="distro-selector" onclick="showDistro(this);">NAT64</span>
</div>

<!-- SIIT -->
{% highlight bash %}
mkdir /etc/jool
cp /usr/share/doc/jool-tools/examples/jool_siit.conf /etc/jool
nano /etc/jool/jool_siit.conf
{% endhighlight %}

<!-- NAT64 -->
{% highlight bash %}
mkdir /etc/jool
cp /usr/share/doc/jool-tools/examples/jool.conf      /etc/jool
nano /etc/jool/jool.conf
{% endhighlight %}

Once you're set, try your service out:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">SIIT Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">NAT64 Debian</span>
	<span class="distro-selector" onclick="showDistro(this);">SIIT Ubuntu</span>
	<span class="distro-selector" onclick="showDistro(this);">NAT64 Ubuntu</span>
</div>

<!-- SIIT Debian -->
{% highlight bash %}
systemctl start jool_siit
{% endhighlight %}

<!-- NAT64 Debian -->
{% highlight bash %}
systemctl start jool
{% endhighlight %}

<!-- SIIT Ubuntu -->
{% highlight bash %}
service jool_siit start
{% endhighlight %}

<!-- NAT64 Ubuntu -->
{% highlight bash %}
service jool      start
{% endhighlight %}

The service creates the instance in the global network namespace. It's a perfectly average instance, so you can query or further tweak it normally:

<div class="distro-menu">
	<span class="distro-selector" onclick="showDistro(this);">SIIT</span>
	<span class="distro-selector" onclick="showDistro(this);">NAT64</span>
</div>

<!-- SIIT -->
{% highlight bash %}
jool_siit instance display
jool_siit -i "init" global display
jool_siit -i "init" eamt display
# etc
{% endhighlight %}

<!-- NAT64 -->
{% highlight bash %}
jool      instance display
jool      -i "init" global display
jool      -i "init" pool4 display
# etc
{% endhighlight %}

(But any changes meant to be persistent need to be included in the configuration file.)

That's all. If the service is configured correctly, it will start automatically on every boot.

## Not Debian

First, figure out whether you're using systemd or System V, and whether you need an SIIT or a NAT64.

- [This](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/jool-tools.jool_siit.service) is Debian's SIIT systemd unit file. (It probably needs to be renamed as `/lib/systemd/system/jool_siit.service`.)
- [This](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/jool-tools.jool.service) is Debian's NAT64 systemd unit file. (It probably needs to be renamed as `/lib/systemd/system/jool.service`.)
- [This](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/jool-tools.jool_siit.init) is Debian's SIIT System V script. (It probably needs to be renamed as `/etc/init.d/jool_siit`.)
- [This](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/jool-tools.jool.init) is Debian's NAT64 System V script.  (It probably needs to be renamed as `/etc/init.d/jool_siit`.)

Grab the one you need and adapt it to your needs. To wit, the only thing you might need to modify is the path to the jool userspace client binary. In the files above it's `/usr/bin/jool_siit` and `/usr/bin/jool`, but your installation might have likely placed them in `/usr/local/bin` instead.

Once that's done, simply follow the [Debian directions above](#debian). [This](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/examples/jool_siit.conf) is the sample SIIT sample configuration file, and [this](https://raw.githubusercontent.com/NICMx/Jool/debian/debian/examples/jool.conf) is the NAT64 sample configuration file.
