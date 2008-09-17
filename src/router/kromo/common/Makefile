
#
# *********************************************************
# *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
# *********************************************************

# This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
# the contents of this file may not be disclosed to third parties,
# copied or duplicated in any form without the prior written
# permission of CyberTAN Inc.

# This software should be used as a reference only, and it not
# intended for production use!


# THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
#
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


all:

install:
#	-cp -a Cysaja.asp $(INSTALLDIR)/www/
#	-cp -a SysInfo.htm $(INSTALLDIR)/www/
#	-cp -a SysInfo1.htm $(INSTALLDIR)/www/
ifeq ($(CONFIG_MSSID),y)
	-cp -a Info.htm $(INSTALLDIR)/www/Info.htm
	-cp -a Info.live.htm $(INSTALLDIR)/www/Info.live.htm
else
	-cp -a Info.htm.v23 $(INSTALLDIR)/www/Info.htm
	-cp -a Info.live.htm.v23 $(INSTALLDIR)/www/Info.live.htm
endif
ifeq ($(CONFIG_REGISTER),y)
	-cp -a register.asp $(INSTALLDIR)/www/register.asp
	-cp -a Registered.asp $(INSTALLDIR)/www/Register.asp
endif

ifdef EMI_TEST
	cp -a emi_test.asp $(INSTALLDIR)/www/
endif

ifdef SYSLOG_SUPPORT
	cp -a Log_all.asp $(INSTALLDIR)/www/
	cp -a Log_settings.asp $(INSTALLDIR)/www/
endif

clean:
