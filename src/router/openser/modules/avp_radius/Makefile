# $Id: Makefile,v 1.3 2005/08/30 11:50:45 miconda Exp $
#
# print example module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=avp_radius.so

# Comment the next two line and uncomment the following two if you want
# to enable Radius support
DEFS+= -DWITH_RADIUS_SUPPORT -I$(LOCALBASE)/include
LIBS=-L$(LOCALBASE)/lib -lradiusclient-ng

include ../../Makefile.modules

