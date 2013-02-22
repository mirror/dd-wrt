
CPPFLAGS += -I../include -I../ubi-utils/include
CPPFLAGS += $(ZLIBCPPFLAGS) $(LZOCPPFLAGS)

ALL_SOURCES=*.[ch] hashtable/*.[ch]

TARGETS = mkfs.ubifs

MKFS_UBIFS_OBJS = $(addprefix $(BUILDDIR)/,\
	crc16.o lpt.o compr.o devtable.o \
	hashtable/hashtable.o hashtable/hashtable_itr.o)

ifeq ($(WITHOUT_LZO), 1)
  CPPFLAGS += -DWITHOUT_LZO
else
  LZOLDLIBS = -llzo2
endif

ifeq ($(WITHOUT_XZ), 1)
  CPPFLAGS += -DWITHOUT_XZ
else
ifneq ($(LZMA_STATIC_LIB),)
  MKFS_UBIFS_OBJS += $(LZMA_STATIC_LIB)
else
  XZLDLIBS = -llzma
endif
endif

LDLIBS_mkfs.ubifs = -lz $(LZOLDLIBS) $(XZLDLIBS) -lm -luuid -L$(BUILDDIR)/../ubi-utils/ -lubi
LDLIBS_mkfs.ubifs += -L$(BUILDDIR)/../lib -lmtd
LDLIBS_mkfs.ubifs += $(ZLIBLDFLAGS) $(LZOLDFLAGS) $(XZLDFLAGS)

include ../common.mk

$(BUILDDIR)/mkfs.ubifs: $(MKFS_UBIFS_OBJS)

clean::
	rm -f $(BUILDDIR)/hashtable/*.o cscope.*

cscope:
	@echo $(ALL_SOURCES) > cscope.files
	@cscope -bR
	@rm cscope.files

install:: ${TARGETS}
	mkdir -p ${DESTDIR}/${SBINDIR}
	install -m 0755 ${TARGETS} ${DESTDIR}/${SBINDIR}/
