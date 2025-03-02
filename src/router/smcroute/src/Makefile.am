AUTOMAKE_OPTIONS     = subdir-objects

sbin_PROGRAMS	     = smcrouted smcroutectl
smcrouted_SOURCES    = smcrouted.c conf.c conf.h mroute.c mroute.h iface.c \
		       iface.h inet.c inet.h ipc.c ipc.h kern.c kern.h	   \
		       log.c log.h mcgroup.c mcgroup.h msg.c msg.h	   \
		       notify.c notify.h pidfile.c queue.h script.c	   \
		       script.h socket.c socket.h timer.c timer.h util.h

smcrouted_CFLAGS     = -W -Wall -Wextra -Wno-deprecated-declarations -std=gnu99
smcrouted_CPPFLAGS   = -D_ATFILE_SOURCE -D_INCOMPLETE_XOPEN_C063
smcrouted_CPPFLAGS  += -DSYSCONFDIR=\"@sysconfdir@\" -DRUNSTATEDIR=\"@runstatedir@\"
smcrouted_LDADD	     = $(LIBS) $(LIBOBJS) @LIB_RT@ @LIB_PTHREAD@

if USE_LIBCAP
smcrouted_SOURCES   += cap.c cap.h
smcrouted_LDADD	    += -lcap
endif

if HAVE_LIBSYSTEMD
smcrouted_SOURCES   += systemd.c
smcrouted_CFLAGS    += $(libsystemd_CFLAGS)
smcrouted_LDADD	    += $(libsystemd_LIBS)
endif

if USE_MRDISC
smcrouted_SOURCES   += mrdisc.c mrdisc.h
endif

smcroutectl_SOURCES  = smcroutectl.c msg.h util.h
smcroutectl_CFLAGS   = -W -Wall -Wextra -std=gnu99
smcroutectl_CPPFLAGS = -DRUNSTATEDIR=\"@runstatedir@\"
smcroutectl_LDADD    = $(LIBS) $(LIBOBJS)
