sbin_PROGRAMS = chilli

# add -pg to enable gprof
AM_CFLAGS = -D_GNU_SOURCE -fno-builtin -DSBINDIR='"$(sbindir)"' 

chilli_SOURCES = chilli.c defs.h tun.c tun.h cmdline.c cmdline.h ippool.c ippool.h radius.h radius.c md5.c md5.h redir.h redir.c dhcp.c dhcp.h syserr.c syserr.h iphash.c iphash.h lookup.c lookup.h

# chilli_LDFLAGS = -lcrypt

