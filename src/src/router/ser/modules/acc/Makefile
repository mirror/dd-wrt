# $Id: Makefile,v 1.14.2.1.4.1 2004/07/18 22:56:23 sobomax Exp $
#
# acc module makefile
#
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=acc.so
LIBS=

# uncomment the next line if you wish to enable SQL accounting
#DEFS+=-DSQL_ACC

# uncomment the next two lines if you wish to enable RADIUS accounting
#DEFS+=-DRAD_ACC -I$(LOCALBASE)/include
#LIBS=-L$(LOCALBASE)/lib -lradiusclient

include ../../Makefile.modules
