srcdir=.


PATHS= -DSSHDIR=\"$(sysconfdir)\" \
	-D_PATH_SSH_PROGRAM=\"$(SSH_PROGRAM)\" \
	-D_PATH_SSH_ASKPASS_DEFAULT=\"$(ASKPASS_PROGRAM)\" \
	-D_PATH_SFTP_SERVER=\"$(SFTP_SERVER)\" \
	-D_PATH_SSH_KEY_SIGN=\"$(SSH_KEYSIGN)\" \
	-D_PATH_SSH_PIDDIR=\"$(piddir)\" \
	-D_PATH_PRIVSEP_CHROOT_DIR=\"$(PRIVSEP_PATH)\" \
	-DSSH_RAND_HELPER=\"$(RAND_HELPER)\"

LDFLAGS=$(LDLTO) -L. -Lopenbsd-compat/ $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections

CFLAGS=$(COPTS) $(LTO) -Wall -Wpointer-arith -Wuninitialized -Wsign-compare -Wno-pointer-sign -Wformat-security -fno-builtin-memset -std=gnu99  -I$(SSLPATH)/include $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections

CPPFLAGS=$(COPTS) $(LTO) -I. -I$(srcdir)  $(PATHS) -DHAVE_CONFIG_H
#LIBS=-lresolv


LIBSSH_OBJS=xmalloc.o log.o buffer.o bufaux.o misc.o fatal.o
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<


all: sftp-server


LIBCOMPAT=openbsd-compat/libopenbsd-compat.a
$(LIBCOMPAT): always
	(cd openbsd-compat && $(MAKE))
always:


libssh.a: $(LIBSSH_OBJS)
	$(AR) rv $(LTOPLUGIN) $@ $(LIBSSH_OBJS)

sftp-server: $(LIBCOMPAT) libssh.a sftp.o sftp-common.o sftp-server.o sftp-server-main.o
	$(CC) -o $@ sftp-server.o sftp-common.o sftp-server-main.o $(LDFLAGS) -lssh -lopenbsd-compat

clean:
	rm -f *.o *.a logintest config.cache config.log
	rm -f *.out core survey
	(cd openbsd-compat && $(MAKE) clean)




install:
	install -D sftp-server $(INSTALLDIR)/usr/libexec/sftp-server