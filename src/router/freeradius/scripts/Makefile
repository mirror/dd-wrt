# $Id: 526c32134a98eef36b709f031c17536b1cc88e8b $

include ../Make.inc

all:

radwatch:
	$(INSTALL) -m 755 radwatch		$(R)$(sbindir)

install:
	$(INSTALL) -m 755 rc.radiusd		$(R)$(sbindir)
	$(INSTALL) -m 755 radsqlrelay		$(R)$(bindir)
	$(INSTALL) -m 755 radwatch		$(R)$(sbindir)
	$(INSTALL) -m 755 raddebug		$(R)$(sbindir)
	$(INSTALL) -m 755 cryptpasswd		$(R)$(bindir)/radcrypt

clean:
