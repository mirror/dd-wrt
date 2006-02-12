# $Id: Makefile,v 1.4.8.1 2004/07/18 22:56:23 sobomax Exp $
#
# Digest Authentication - Radius support
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=auth_radius.so
DEFS+=-I$(LOCALBASE)/include
LIBS=-L$(LOCALBASE)/lib -L/usr/pkg/lib -lradiusclient

include ../../Makefile.modules
