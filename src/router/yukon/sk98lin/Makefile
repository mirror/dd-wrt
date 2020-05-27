#******************************************************************************
#
# Project:      GEnesis, PCI Gigabit Ethernet Adapter
# Version:      $Revision: 1.5 $
# Date:         $Date: 2009/07/27 12:43:38 $
# Purpose:      The main driver source module
#
#******************************************************************************

#******************************************************************************
#
#	(C)Copyright 1998-2002 SysKonnect GmbH.
#	(C)Copyright 2002-2007 Marvell.
#
#	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
#      Server Adapters.
#
#	Address all question to: msgg-linux@marvell.com
#
#      LICENSE:
#      (C)Copyright Marvell.
#      
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#      
#      The information in this file is provided "AS IS" without warranty.
#      /LICENSE
#
#******************************************************************************


# Include the SDK support
# SDKPARAM += -DMV_INCLUDE_SDK_SUPPORT

# Include the AVB support
# AVBPARAM += -DSK_AVB

ifdef SDKPARAM
	SDK_OPS += \
		fwptrn.o	\
		skspi.o		\
		skpflash.o	\
		skgeasfapi.o	\
		skgespilole.o	\
		fwfunctions.o	\
		fwapp.o		\
		fwhci.o		\
		fwimage.o	\
		fwmain.o	\
		fwos.o
else
	SDK_OPS +=
endif

ifdef AVBPARAM
include $(SUBDIRS)/Makefile.avb
endif

obj-m += sk98lin.o
sk98lin-objs    :=	\
		skge.o		\
		sky2.o		\
		skethtool.o	\
		sky2le.o	\
		skdim.o		\
		skaddr.o	\
		skgehwt.o	\
		skgeinit.o	\
		skgepnmi.o	\
		skgesirq.o	\
		sktwsi.o	\
		sklm80.o	\
		skqueue.o	\
		sktimer.o	\
		skvpd.o		\
		skxmac2.o	\
		skproc.o	\
		skcsum.o	\
		$(AVB_OPS)	\
		$(SDK_OPS)
# DBGDEF =  \
# -DDEBUG

ifdef DEBUG
DBGDEF +=  \
-DSK_DEBUG_CHKMOD=0x00000000L \
-DSK_DEBUG_CHKCAT=0x00000000L
endif


# **** possible debug modules for SK_DEBUG_CHKMOD *****************
# SK_DBGMOD_MERR        0x00000001L     /* general module error indication */
# SK_DBGMOD_HWM         0x00000002L     /* Hardware init module */
# SK_DBGMOD_RLMT        0x00000004L     /* RLMT module */
# SK_DBGMOD_VPD         0x00000008L     /* VPD module */
# SK_DBGMOD_I2C         0x00000010L     /* I2C module */
# SK_DBGMOD_PNMI        0x00000020L     /* PNMI module */
# SK_DBGMOD_CSUM        0x00000040L     /* CSUM module */
# SK_DBGMOD_ADDR        0x00000080L     /* ADDR module */
# SK_DBGMOD_DRV         0x00010000L     /* DRV module */

# **** possible debug categories for SK_DEBUG_CHKCAT **************
# *** common modules ***
# SK_DBGCAT_INIT        0x00000001L     module/driver initialization
# SK_DBGCAT_CTRL        0x00000002L     controlling: add/rmv MCA/MAC and other controls (IOCTL)
# SK_DBGCAT_ERR         0x00000004L     error handling paths
# SK_DBGCAT_TX          0x00000008L     transmit path
# SK_DBGCAT_RX          0x00000010L     receive path
# SK_DBGCAT_IRQ         0x00000020L     general IRQ handling
# SK_DBGCAT_QUEUE       0x00000040L     any queue management
# SK_DBGCAT_DUMP        0x00000080L     large data output e.g. hex dump
# SK_DBGCAT_FATAL       0x00000100L     large data output e.g. hex dump

# *** driver (file skge.c) ***
# SK_DBGCAT_DRV_ENTRY           0x00010000      entry points
# SK_DBGCAT_DRV_???             0x00020000      not used
# SK_DBGCAT_DRV_MCA             0x00040000      multicast
# SK_DBGCAT_DRV_TX_PROGRESS     0x00080000      tx path
# SK_DBGCAT_DRV_RX_PROGRESS     0x00100000      rx path
# SK_DBGCAT_DRV_PROGRESS        0x00200000      general runtime
# SK_DBGCAT_DRV_???             0x00400000      not used
# SK_DBGCAT_DRV_PROM            0x00800000      promiscuous mode
# SK_DBGCAT_DRV_TX_FRAME        0x01000000      display tx frames
# SK_DBGCAT_DRV_ERROR           0x02000000      error conditions
# SK_DBGCAT_DRV_INT_SRC         0x04000000      interrupts sources
# SK_DBGCAT_DRV_EVENT           0x08000000      driver events

EXTRA_CFLAGS += -I/xfs/sk98/sk98lin/src -DSK_USE_CSUM -DSK_DIAG_SUPPORT \
		-DYUKON -DYUK2 -DCONFIG_SK98LIN_ZEROCOPY \
		-DSK_NO_RLMT -DSK_LINUX -DUSE_SK_RSS_SUPPORT \
		-DSK98LIN_DIAG \
		$(DBGDEF) $(SKPARAM) $(SDKPARAM) $(AVBPARAM) $(ADDPARAM)

clean:
	rm -f core *.o *.a *.s


