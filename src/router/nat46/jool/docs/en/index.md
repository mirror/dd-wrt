---
language: en
layout: default
category: Home
title: Home
---

# Home

-------------------

## Introduction

Jool is an Open Source [SIIT and NAT64](intro-xlat.html) for Linux.

* [Click here](documentation.html) to start getting acquainted with the software.
* [Click here](download.html) to download Jool.

-------------------

## Status

> ![Warning](../images/warning.svg) The project's development has slowed down to essential maintenance. Bugfixing and support will remain active, but there will be no new features in the foreseeable future.

- The most mature version is [4.1.11](download.html#41x).
- The second release candidate for version [4.2.0](download.html#42x) is also available now.
- **jool.mx is no longer maintained. Please use https://nicmx.github.io/Jool instead.**

-------------------

## Latest News

### 2023-12-23

Version 4.1.11 has been released. Bugfixes:

- [#407](https://github.com/NICMx/Jool/issues/407): Patch compilation in some environments.
- [#409](https://github.com/NICMx/Jool/issues/409): Move the Debian systemd service to After=network-pre.target, to prevent deadlock during boot in some environments.
- [750909d](https://github.com/NICMx/Jool/commit/750909dd3f0df8771883121b1820f7e10010ff31): When running into an untranslatable address, print it clearly in the logs.
- [#413](https://github.com/NICMx/Jool/issues/413): Enhance validations of pool4, BIB and session userspace requests.
- [#415](https://github.com/NICMx/Jool/issues/415), [Debian#1057445](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1057445): Add support for kernel 6.4, 6.5, 6.6 and 6.7-rc6.
- [#417](https://github.com/NICMx/Jool/issues/417): Add support for RHEL 9.2 and 9.3.
- [Debian#1046037](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1046037): Add the "distclean" target to the kernel module Makefiles.
- [Debian#1057703](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1057703): Remove mime-support from build dependencies.
- [Debian#1041856](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1041856): Remove all links to jool.mx in the Debian package.

There have also been several patches [in](https://github.com/NICMx/Jool/commit/4eb5b3e3ec9d671067e571f6eee08d95a4a34091) [the](https://github.com/NICMx/Jool/commit/8c78ed47c51f2ee10cc63058014212887c340122) [joold](https://github.com/NICMx/Jool/commit/07e6fd9a4eb93cc27f271ef1cd526b155edc66a4) [code](https://github.com/NICMx/Jool/commit/4fcfe184d9444ef25d3e5ab5995a06c8bed8b9d2), but this is still an [ongoing effort](https://github.com/NICMx/Jool/issues/410). You might want to abstain from using joold at the moment.
