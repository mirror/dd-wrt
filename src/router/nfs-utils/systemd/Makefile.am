## Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in

udev_rulesdir = /usr/lib/udev/rules.d/
udev_files = 60-nfs.rules

unit_files =  \
    nfs-client.target \
    rpc_pipefs.target \
    \
    nfs-mountd.service \
    nfs-server.service \
    nfs-utils.service \
    rpc-statd-notify.service \
    rpc-statd.service \
    \
    proc-fs-nfsd.mount \
    fsidd.service

rpc_pipefs_mount_file = \
    var-lib-nfs-rpc_pipefs.mount

if CONFIG_NFSV4
unit_files += \
    nfs-idmapd.service
endif

if CONFIG_NFSV4SERVER
unit_files += \
    nfsv4-exportd.service \
    nfsv4-server.service
endif

if CONFIG_NFSV41
unit_files += \
    nfs-blkmap.service
endif

if CONFIG_GSS
unit_files += \
    auth-rpcgss-module.service \
    rpc-gssd.service

if CONFIG_SVCGSS
unit_files += \
    rpc-svcgssd.service
endif
endif

if CONFIG_NFSDCLD
unit_files += \
    nfsdcld.service
endif

man5_MANS	= nfs.conf.man
man7_MANS	= nfs.systemd.man
EXTRA_DIST = $(unit_files) $(udev_files) $(man5_MANS) $(man7_MANS)

generator_dir = $(unitdir)/../system-generators

EXTRA_PROGRAMS	= nfs-server-generator rpc-pipefs-generator
genexecdir = $(generator_dir)

COMMON_SRCS = systemd.c systemd.h

nfs_server_generator_SOURCES = $(COMMON_SRCS) nfs-server-generator.c

rpc_pipefs_generator_SOURCES = $(COMMON_SRCS) rpc-pipefs-generator.c

nfs_server_generator_LDADD = ../support/export/libexport.a \
			     ../support/nfs/libnfs.la \
			     ../support/misc/libmisc.a \
			     ../support/reexport/libreexport.a \
			     $(LIBPTHREAD)


rpc_pipefs_generator_LDADD = ../support/nfs/libnfs.la

if INSTALL_SYSTEMD
genexec_PROGRAMS = nfs-server-generator rpc-pipefs-generator
install-data-hook: $(unit_files) $(udev_files)
	mkdir -p $(DESTDIR)/$(unitdir)
	cp $(unit_files) $(DESTDIR)/$(unitdir)
	cp $(rpc_pipefs_mount_file) $(DESTDIR)/$(unitdir)/$(rpc_pipefsmount)
	mkdir -p $(DESTDIR)/$(udev_rulesdir)
	cp $(udev_files) $(DESTDIR)/$(udev_rulesdir)
endif
