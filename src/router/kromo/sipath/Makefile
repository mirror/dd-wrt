
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

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


all:

install:
	install -d $(INSTALLDIR)/usr/lib/webif
	-cp -a *.awk $(INSTALLDIR)/usr/lib/webif
	-cp -a *.asp $(INSTALLDIR)/www
	-cp -a *.css $(INSTALLDIR)/www/
	-cp -a *.sed $(INSTALLDIR)/www/
	-cp -a *.html $(INSTALLDIR)/www/



clean:
