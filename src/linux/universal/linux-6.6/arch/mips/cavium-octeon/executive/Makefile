#
# Makefile for the Cavium Octeon specific kernel interface routines
# under Linux.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 2005-2008 Cavium Networks
#
ccflags-y := -DCVMX_BUILD_FOR_LINUX_KERNEL -DCVMX_ENABLE_CSR_ADDRESS_CHECKING=0 -DOCTEON_MODEL=OCTEON_CN70XX -DCVMX_ENABLE_PARAMETER_CHECKING=0 -Wno-error

obj-y += cvmx-bootmem.o cvmx-l2c.o cvmx-sysinfo.o octeon-model.o	\
	octeon-feature.o
obj-y += cvmx-pko.o cvmx-spi.o cvmx-cmd-queue.o cvmx-helper-cfg.o	\
	cvmx-ipd.o cvmx-global-resources.o cvmx-range.o	cvmx-agl.o	\
	cvmx-pki.o cvmx-helper-srio.o cvmx-helper-ilk.o	cvmx-qlm.o	\
	cvmx-srio.o cvmx-ilk.o cvmx-qlm-tables.o cvmx-spi4000.o		\
	cvmx-twsi.o cvmx-appcfg-transport.o cvmx-fpa-resource.o		\
	cvmx-pko-internal-ports-range.o cvmx-helper-agl.o		\
	cvmx-helper-board.o cvmx-helper.o cvmx-helper-xaui.o		\
	cvmx-helper-rgmii.o cvmx-helper-sgmii.o cvmx-helper-npi.o	\
	cvmx-helper-loop.o cvmx-helper-spi.o cvmx-helper-util.o		\
	cvmx-pki-resources.o cvmx-bgx.o cvmx-pko3-queue.o		\
	cvmx-helper-bgx.o cvmx-pko3.o cvmx-helper-pki.o			\
	cvmx-helper-pko3.o cvmx-pko3-resources.o cvmx-helper-pko.o	\
	cvmx-helper-ipd.o cvmx-sso-resources.o cvmx-ocla.o cvmx-boot-vector.o \
	cvmx-lap.o cvmx-osm.o cvmx-ila.o

obj-y += cvmx-helper-errata.o cvmx-helper-jtag.o
obj-$(CONFIG_PCI) += cvmx-pcie.o

obj-$(CONFIG_USB_OCTEON_HCD)		+= cvmx-usb.o
obj-$(CONFIG_CAVIUM_OCTEON_RAPIDIO) 	+= cvmx-dma-engine.o
obj-$(CONFIG_CAVIUM_OCTEON_NAND)	+= cvmx-bch.o cvmx-nand.o
obj-$(CONFIG_CAVIUM_OCTEON_ERROR_TREE)	+= cvmx-error-trees.o
obj-$(CONFIG_CAVIUM_GDB)		+= cvmx-debug.o cvmx-debug-handler.o cvmx-debug-uart.o cvmx-debug-remote.o
