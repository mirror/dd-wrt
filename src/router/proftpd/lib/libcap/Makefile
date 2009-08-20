## This libcap (for proftpd) is originally from libcap-1.10,
## at ftp://linux.kernel.org/pub/libs/security/linux-privs.
## This interface is SPECIFIC TO THE LINUX 2.2 KERNEL!!!  IT IS NOT GUARANTEED
## TO WORK ON ANY PRIOR OR LATER VERSION (ie: 2.1.x or 2.3.x).
## If this library stops working, please contact core@proftpd.org.

#
# defines
#
topdir=$(shell pwd)/..
include ../../Make.rules
#
# Library version
#
LIBNAME=libcap.a
#

FILES=cap_alloc cap_proc cap_extint cap_flag cap_text cap_sys

# for later when there is filesystem support for cap's:
#FILES += cap_file 

INCLS=libcap.h cap_names.h $(INCS)
OBJS=$(addsuffix .o, $(FILES))

all: $(LIBNAME)

_makenames: _makenames.c cap_names.sed
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

cap_names.h: _makenames
	./_makenames > cap_names.h

cap_names.sed: Makefile /usr/include/linux/capability.h
	@echo "=> making cap_names.c from <linux/capability.h>"
	@sed -ne '/^#define[ \t]CAP[_A-Z]\+[ \t]\+[0-9]\+/{s/^#define \([^ \t]*\)[ \t]*\([^ \t]*\)/  \{ \2, \"\1\" \},/;y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;p;}' < /usr/include/linux/capability.h | fgrep -v 0x > cap_names.sed
#	@sed -ne '/^#define[ \t]CAP[_A-Z]\+[ \t]\+[0-9]\+/{s/^#define CAP_\([^ \t]*\)[ \t]*\([^ \t]*\)/  \{ \2, \"\1\" \},/;y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;p;}' < /usr/include/linux/capability.h | fgrep -v 0x > cap_names.sed

$(LIBNAME): $(OBJS)
	ar rcu $@ $(OBJS)

%.o: %.c $(INCLS)
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	mkdir -p -m 0755 $(INCDIR)/sys
	install -m 0644 include/sys/capability.h $(INCDIR)/sys
	mkdir -p -m 0755 $(LIBDIR)
	install -m 0644 $(MINLIBNAME) $(LIBDIR)/$(MINLIBNAME)
	ln -sf $(MINLIBNAME) $(LIBDIR)/$(MAJLIBNAME)
	ln -sf $(MAJLIBNAME) $(LIBDIR)/$(LIBNAME)
	-/sbin/ldconfig

clean:
	$(LOCALCLEAN)
	rm -f $(OBJS) $(LIBNAME)*
	rm -f cap_names.h cap_names.sed _makenames

