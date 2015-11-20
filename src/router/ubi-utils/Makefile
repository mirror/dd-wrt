
# -*- sh -*-

VERSION = 1.5.2

CPPFLAGS += -D_GNU_SOURCE -I./include -I$(BUILDDIR)/include -I./ubi-utils/include $(ZLIBCPPFLAGS) $(LZOCPPFLAGS) $(UUIDCPPFLAGS)
WITHOUT_XATTR=1

ifeq ($(WITHOUT_XATTR), 1)
  CPPFLAGS += -DWITHOUT_XATTR
endif
ifeq ($(WITHOUT_LZO), 1)
  CPPFLAGS += -DWITHOUT_LZO
else
  LZOLDLIBS = -llzo2
endif

TESTS = tests

MISC_BINS = \
	ftl_format doc_loadbios ftl_check mtd_debug docfdisk \
	serve_image recv_image mtdpart flash_erase flash_lock \
	flash_unlock flash_otp_info flash_otp_dump flash_otp_lock \
	flash_otp_write flashcp
UBI_BINS = \
	ubiupdatevol ubimkvol ubirmvol ubicrc32 ubinfo ubiattach \
	ubidetach ubinize ubiformat ubirename mtdinfo ubirsvol ubiblock
UBIFS_BINS = \
	mkfs.ubifs/mkfs.ubifs
JFFSX_BINS = \
	mkfs.jffs2 sumtool jffs2reader jffs2dump
NAND_BINS = \
	nanddump nandwrite nandtest nftldump nftl_format
NOR_BINS = \
	rfddump rfdformat

BINS = $(addprefix misc-utils/,$(MISC_BINS))
BINS += $(addprefix ubi-utils/,$(UBI_BINS))
BINS += $(addprefix ubifs-utils/,$(UBIFS_BINS))
BINS += $(addprefix jffsX-utils/,$(JFFSX_BINS))
BINS += $(addprefix nand-utils/,$(NAND_BINS))
BINS += $(addprefix nor-utils/,$(NOR_BINS))
SCRIPTS = $(addprefix misc-utils/,flash_eraseall)

TARGETS = $(BINS)
TARGETS += lib/libmtd.a
TARGETS += ubi-utils/libubi.a

OBJDEPS = $(BUILDDIR)/include/version.h

include common.mk

CLEAN_FIND = find "$(BUILDDIR)/" -xdev '(' -name '*.[ao]' -o -name '.*.c.dep' ')'

clean::
ifneq ($(BUILDDIR)/.git,)
ifneq ($(BUILDDIR),.)
ifneq ($(BUILDDIR),$(CURDIR))
	rm -rf $(BUILDDIR)
endif
endif
endif
# findutils v4.1.x (RHEL 4) do not have '+' syntax
	@if test -d "$(BUILDDIR)/"; then \
		$(CLEAN_FIND) -exec rm -f {} + 2> /dev/null || \
		$(CLEAN_FIND) -exec rm -f {} \; ; \
	fi
	rm -f $(BUILDDIR)/include/version.h
	$(MAKE) -C $(TESTS) clean

install:: $(addprefix $(BUILDDIR)/,${BINS} ${SCRIPTS})
	mkdir -p ${DESTDIR}/${SBINDIR}
	install -m 0755 $^ ${DESTDIR}/${SBINDIR}/
	mkdir -p ${DESTDIR}/${MANDIR}/man1
	install -m 0644 jffsX-utils/mkfs.jffs2.1 ${DESTDIR}/${MANDIR}/man1/
	-gzip -9f ${DESTDIR}/${MANDIR}/man1/*.1

tests::
	$(MAKE) -C $(TESTS)

cscope:
	cscope -bR

$(BUILDDIR)/include/version.h: $(BUILDDIR)/include/version.h.tmp
	$(call BECHO,CHK)
	$(Q)cmp -s $@ $@.tmp && rm -f $@.tmp || mv $@.tmp $@
$(BUILDDIR)/include/version.h.tmp:
	${Q}mkdir -p $(dir $@)
	$(Q)echo '#define VERSION "$(VERSION)"' > $@

#
# Utils in top level
#
obj-mkfs.jffs2 = compr_rtime.o compr_zlib.o compr_lzo.o compr.o rbtree.o
LDFLAGS_mkfs.jffs2 = $(ZLIBLDFLAGS) $(LZOLDFLAGS) $(CPPFLAGS)
LDLIBS_mkfs.jffs2  = -lz $(LZOLDLIBS)

LDFLAGS_jffs2reader = $(ZLIBLDFLAGS) $(LZOLDFLAGS)
LDLIBS_jffs2reader  = -lz $(LZOLDLIBS)

$(foreach v,$(MISC_BINS),$(eval $(call mkdep,misc-utils/,$(v))))
$(foreach v,$(JFFSX_BINS),$(eval $(call mkdep,jffsX-utils/,$(v))))
$(foreach v,$(NAND_BINS),$(eval $(call mkdep,nand-utils/,$(v))))
$(foreach v,$(NOR_BINS),$(eval $(call mkdep,nor-utils/,$(v))))

#
# Common libmtd
#
obj-libmtd.a = libmtd.o libmtd_legacy.o libcrc32.o libfec.o
$(call _mkdep,lib/,libmtd.a)

#
# Utils in ubi-utils/ subdir
#
obj-libiniparser.a = libiniparser.o dictionary.o
obj-libscan.a      = libscan.o
obj-libubi.a       = libubi.o
obj-libubigen.a    = libubigen.o

obj-mtdinfo   = libubigen.a
obj-ubinize   = libubigen.a libiniparser.a
obj-ubiformat = libubigen.a libscan.a

$(foreach v,libubi.a libubigen.a libiniparser.a libscan.a,$(eval $(call _mkdep,ubi-utils/,$(v))))
$(foreach v,$(UBI_BINS),$(eval $(call mkdep,ubi-utils/,$(v),libubi.a ubiutils-common.o)))

#
# Utils in ubifs-utils subdir
#
obj-mkfs.ubifs = crc16.o lpt.o compr.o devtable.o \
	hashtable/hashtable.o hashtable/hashtable_itr.o
LDFLAGS_mkfs.ubifs = $(ZLIBLDFLAGS) $(LZOLDFLAGS) $(UUIDLDFLAGS)
LDLIBS_mkfs.ubifs = -lz $(LZOLDLIBS) -lm -luuid
$(call mkdep,ubifs-utils/mkfs.ubifs/,mkfs.ubifs,,ubi-utils/libubi.a)
