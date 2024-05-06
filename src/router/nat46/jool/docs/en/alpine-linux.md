---
language: en
layout: default
category: Documentation
title: Alpine Linux
---

[Documentation](documentation.html) > [Installation](documentation.html#installation) > Alpine Linux

# Jool in Alpine Linux

Compiling and installing kernel modules is not the way things are meat to be done in Alpine Linux. Fortunately, the Alpine Linux folks are kind enough to provide official packages for Jool.

Please note that these binaries are not maintained nor supervised by the Jool team. We are still available for advice if issues arise, however.

And finally: It might take an indeterminate amount of time for the latest version of Jool to be uploaded to the Alpine Linux's repository. Remember that you can find previous versions of this site's documentation in the [download page](download.html).

## Installing Jool

> ![Warning!](../images/warning.svg) If you have somehow previously installed Jool from source in your machine, then those binaries may conflict with the ones installed here.
>
> You may uninstall source-installed binaries by following [these steps](install.html#uninstalling).

You need Alpine Linux v3.11 at least (or edge).

	apk add jool-tools

If you have one of the official kernel packages installed (e.g. `linux-vanilla`), this will automatically install the kernel modules.

If you get `ERROR: unsatisfiable constraints: jool-tools (missing)`, you probably don't have the community repository listed in your `/etc/apk/repositories`. In that case, follow the instructions on [this page](https://wiki.alpinelinux.org/wiki/Enable_Community_Repository) and try it again.

## Using Jool

Adjust configuration files in directory `/etc/jool` to your needs. Then you can start services `jool`, `jool_siit` and/or `joold` using `rc-service <svcname> start` (or `/etc/init.d/<svcname> start`). To make the services start automatically at the system startup, add them to a runlevel using `rc-update add <svcname>`.

If you want to run multiple instances of `jool`, `jool_siit` or `joold`, you can make symlinks for the init scripts named `<svcname>.<instance>` (or `<svcname>-<instance>`). Each service expects JSON configuration file named after the init script plus `.conf` suffix (e.g. `jool_siit.conf`, `jool_siit.lan.conf`, ...) in directory `/etc/jool`. For example:

	ln -s jool_siit /etc/init.d/jool_siit.lan
	cp /etc/jool/jool_siit.conf /etc/jool/jool_siit.lan.conf
	rc-service jool_siit.lan start
