#
# Makefile for ubi-utils
#

KERNELHDR := ../include

# CFLAGS += -Werror
CPPFLAGS += -Iinclude -Isrc -I$(KERNELHDR)

LIBS = libubi libubigen libiniparser libscan
TARGETS = ubiupdatevol ubimkvol ubirmvol ubicrc32 ubinfo ubiattach \
          ubidetach ubinize ubiformat ubirename mtdinfo ubirsvol

VPATH = src
LDLIBS = -L$(BUILDDIR)/../lib -lmtd

include ../common.mk

# And the below is the rule to get final executable from its .o and ubiutils-common.o
$(TARGETS): $(addprefix $(BUILDDIR)/,\
	libubi.a ubiutils-common.o)
#	$(CC) $(CFLAGS) $(filter %.o, $^) -L. -lubi -o $@

$(BUILDDIR)/ubicrc32: $(addprefix $(BUILDDIR)/,\
	ubicrc32.o)
#	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/ubinize: $(addprefix $(BUILDDIR)/,\
	ubinize.o ubiutils-common.o libiniparser.a libubigen.a)
#	$(CC) $(CFLAGS) $(filter %.o, $^) -L. -liniparser -lubigen -o $@

$(BUILDDIR)/mtdinfo: $(addprefix $(BUILDDIR)/,\
	libubigen.a ubiutils-common.o)
#	$(CC) $(CFLAGS) $(filter %.o, $^) -L. -lmtd -lubigen -o $@

$(BUILDDIR)/ubiformat: $(addprefix $(BUILDDIR)/,\
	ubiformat.o ubiutils-common.o libscan.a libubi.a libubigen.a)
#	$(CC) $(CFLAGS) $(filter %.o, $^) -L. -lmtd -lscan -lubi -lubigen -o $@

$(BUILDDIR)/libubi.a: $(BUILDDIR)/libubi.o

$(BUILDDIR)/libubigen.a: $(BUILDDIR)/libubigen.o

$(BUILDDIR)/libiniparser.a: $(addprefix $(BUILDDIR)/,\
	libiniparser.o dictionary.o)

$(BUILDDIR)/libscan.a: $(addprefix $(BUILDDIR)/,\
	libscan.o)

clean::
	rm -f $(addsuffix .a, $(LIBS))

install::
	mkdir -p ${DESTDIR}/${SBINDIR}
	install -m 0755 ${TARGETS} ${DESTDIR}/${SBINDIR}/

uninstall:
	for file in ${TARGETS}; do \
		$(RM) ${DESTDIR}/${SBINDIR}/$$file; \
	done
