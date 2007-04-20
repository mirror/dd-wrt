# $Id: Makefile,v 1.2 2005/06/14 08:19:39 bogdan_iancu Exp $
#
# Digest Authentication - Radius support
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=auth_radius.so
DEFS+=-I$(LOCALBASE)/include
LIBS=-L$(LOCALBASE)/lib -lradiusclient-ng

include ../../Makefile.modules
