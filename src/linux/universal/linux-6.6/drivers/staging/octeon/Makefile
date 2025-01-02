# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2005-2009 Cavium Networks
#
ccflags-y := -DCVMX_BUILD_FOR_LINUX_KERNEL -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DOCTEON_MODEL=OCTEON_CN70XX -DCVMX_ENABLE_PARAMETER_CHECKING=0

#
# Makefile for Cavium OCTEON on-board ethernet driver
#

obj-${CONFIG_OCTEON_ETHERNET} :=  octeon-ethernet.o

octeon-ethernet-y := ethernet.o
octeon-ethernet-y += ethernet-mdio.o
octeon-ethernet-y += ethernet-mem.o
octeon-ethernet-y += ethernet-rgmii.o
octeon-ethernet-y += ethernet-rx.o
octeon-ethernet-y += ethernet-sgmii.o
octeon-ethernet-y += ethernet-spi.o
octeon-ethernet-y += ethernet-tx.o
