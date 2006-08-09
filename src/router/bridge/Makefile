# $Id: Makefile,v 1.1 2005/09/28 11:52:19 seg Exp $

all:				brctl/brctl brctl/brctld misc/bidi misc/tap

clean:
				$(MAKE) -C brctl clean
				$(MAKE) -C doc clean
				$(MAKE) -C libbridge clean
				$(MAKE) -C misc clean
				rcsclean *
				rm -f *~

brctl/brctl:			brctl/brctl.c brctl/brctl.h brctl/brctl_cmd.c brctl/brctl_disp.c libbridge/libbridge.a libbridge/libbridge.h
				$(MAKE) -C brctl

brctl/brctld:			brctl/brctld.c brctl/brctl.h brctl/brctl_cmd.c brctl/brctl_disp.c libbridge/libbridge.a libbridge/libbridge.h
				$(MAKE) -C brctl

libbridge/libbridge.a:		libbridge/libbridge.h libbridge/libbridge_devif.c libbridge/libbridge_if.c libbridge/libbridge_init.c libbridge/libbridge_misc.c libbridge/libbridge_private.h
				$(MAKE) -C libbridge

misc/bidi:			misc/bidi.c
				$(MAKE) -C misc

misc/tap:			misc/tap.c
				$(MAKE) -C misc
