subdir-m += avl
subdir-m += nvpair
subdir-m += unicode
subdir-m += zcommon
subdir-m += zfs
subdir-m += zpios
subdir-m += icp

INSTALL_MOD_DIR = extra


ZFS_MODULE_CFLAGS += -include $(TOP)/spl/spl_config.h
ZFS_MODULE_CFLAGS += -include $(TOP)/zfs/zfs_config.h
ZFS_MODULE_CFLAGS += -I$(TOP)/zfs/tools/include -I$(TOP)/spl/include -I$(TOP)/spl
export ZFS_MODULE_CFLAGS


all: modules
	$(MAKE) -C tools
	find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	find . -name *.la -exec touch {} +

configure:
	cd tools/libtirpc && ./bootstrap
	cd tools/libtirpc && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" LDFLAGS="-L$(TOP)/zlib" --disable-gssapi --disable-static --prefix=/usr
	make -C tools/libtirpc
# for bootstrapping
	cd tools && ./autogen.sh
	cd tools && ./configure --enable-fast-install --with-sysroot=yes --libdir=/usr/lib --host=$(ARCH)-linux CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/zlib -I$(TOP)/zfs/tools/include -I$(TOP)/zfs/tools/libtirpc  -I$(TOP)/zfs/tools/libtirpc/tirpc" LDFLAGS="-L$(TOP)/zlib -L$(TOP)/e2fsprogs/lib/uuid -L$(TOP)/zfs/tools/libtirpc/src/.libs -L$(TOP)/util-linux/.libs" --prefix=/usr
	find . -name *.la -exec sed -i 's/relink_command/# relink_command/g' {} +
	find . -name *.la -exec touch {} +


modules:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) CONFIG_ZFS=m $@


install:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) INSTALL_MOD_PATH=$(INSTALLDIR) INSTALL_MOD_DIR=extra modules_install
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules.*
	make -C tools/libtirpc install DESTDIR=$(INSTALLDIR)
	make -C tools install DESTDIR=$(INSTALLDIR)
	rm -rf $(INSTALLDIR)/usr/include
	rm -rf $(INSTALLDIR)/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/usr/share
	rm -f $(INSTALLDIR)/usr/lib/*.a
	rm -f $(INSTALLDIR)/usr/lib/*.la


clean:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=`pwd` CROSS_COMPILE="ccache $(ARCH)-linux-uclibc-" O=$(LINUXDIR) $@
