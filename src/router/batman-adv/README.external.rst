.. SPDX-License-Identifier: GPL-2.0

==========================
BATMAN-ADV external module
==========================

Introduction
============

The  batman-adv  module  is  shipped  as part of the Linux kernel
and as an external module. The external  module   allows  to  get
new    features without  upgrading  to  a  newer  kernel  version
and to get batman-adv specific bug fixes for  kernels  that   are
not   supported   anymore.  It compiles  against  and should work
with  Linux 4.14  -  6.7.  Supporting  older  versions   is   not
planned,  but it's probably easy to backport it. If you work on a
backport, feel free to contact us.  :-)


COMPILE
=======

To compile against your currently installed  kernel, just type::

  # make

if you want to compile against some other kernel, use::

  # make KERNELPATH=/path/to/kernel

if you want to install this module::

  # sudo make install


CONFIGURATION
=============

The     in-kernel    module    can    be    configured    through
menuconfig.   When  compiling outside  of the kernel tree,  it is
necessary  to  configure  it  using    the   make  options.  Each
option  can  be  set  to y (enabled), n (disabled) or m (build as
module).  Available  options  and  their    possible   values are
(default marked with an "*")

 * ``CONFIG_BATMAN_ADV_BATMAN_V=[y*|n]`` (B.A.T.M.A.N. V routing algorithm)
 * ``CONFIG_BATMAN_ADV_BLA=[y*|n]`` (B.A.T.M.A.N. bridge loop avoidance)
 * ``CONFIG_BATMAN_ADV_DAT=[y*|n]`` (B.A.T.M.A.N. Distributed ARP Table)
 * ``CONFIG_BATMAN_ADV_DEBUG=[y|n*]`` (B.A.T.M.A.N. debugging)
 * ``CONFIG_BATMAN_ADV_MCAST=[y*|n]`` (B.A.T.M.A.N. multicast optimizations)
 * ``CONFIG_BATMAN_ADV_NC=[y|n*]`` (B.A.T.M.A.N. Network Coding)
 * ``CONFIG_BATMAN_ADV_TRACING=[y|n*]`` (B.A.T.M.A.N. tracing support)

e.g., debugging can be enabled by::

  # make CONFIG_BATMAN_ADV_DEBUG=y

Keep  in  mind  that  all  options  must  also  be added to ``make install``
call.
