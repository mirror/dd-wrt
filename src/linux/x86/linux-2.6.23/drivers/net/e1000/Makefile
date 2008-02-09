################################################################################
#
# Intel PRO/1000 Linux driver
# Copyright(c) 1999 - 2006 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
#
# The full GNU General Public License is included in this distribution in
# the file called "COPYING".
#
# Contact Information:
# Linux NICS <linux.nics@intel.com>
# e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
# Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
#
################################################################################

#
# Makefile for the Intel(R) PRO/1000 ethernet driver
#

obj-$(CONFIG_E1000) += e1000.o

FAMILYC = e1000_82540.o e1000_82542.o e1000_82571.o e1000_82541.o \
	  e1000_82543.o e1000_ich8lan.o e1000_80003es2lan.o

# core driver files
e1000-objs := e1000_main.o $(FAMILYC) e1000_mac.o e1000_nvm.o e1000_phy.o \
	 e1000_manage.o e1000_param.o e1000_ethtool.o kcompat.o e1000_api.o
