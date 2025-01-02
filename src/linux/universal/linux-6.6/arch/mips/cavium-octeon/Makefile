#
# Makefile for the Cavium Octeon specific kernel interface routines
# under Linux.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 2005-2009 Cavium Networks
#
ccflags-y := -DCVMX_BUILD_FOR_LINUX_KERNEL -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DOCTEON_MODEL=OCTEON_CN70XX -DCVMX_ENABLE_PARAMETER_CHECKING=0 -Wno-error

obj-y := cpu.o setup.o octeon-platform.o octeon-irq.o csrc-octeon.o octeon-hw-status.o
obj-y += csrc-octeon-ptp.o
obj-y += dma-octeon.o
obj-y += octeon-memcpy.o
obj-y += executive/
obj-y += crypto/

obj-$(CONFIG_MTD)		      += flash_setup.o
obj-$(CONFIG_SMP)		      += smp.o
obj-$(CONFIG_OCTEON_ILM)	      += oct_ilm.o
obj-$(CONFIG_OCTEON_FPA3)		+= octeon-fpa3.o
#obj-$(CONFIG_USB)		      += octeon-usb.o
