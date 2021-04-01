SCDOC := scdoc

ARCH := $(shell uname -m)
ifeq ($(ARCH),$(filter $(ARCH),i386 i686))
	override ARCH = x86
endif
ifeq ($(ARCH),$(filter $(ARCH),sh2 sh4))
	override ARCH = sh
endif
ifeq ($(ARCH),$(filter $(ARCH),ppc64le))
	override ARCH = ppc64
endif
ifeq ($(ARCH),$(filter $(ARCH),armv7l))
	override ARCH = arm
endif

LIBDIR := /lib
INCLUDEDIR := /usr/include
PKGCONFIGDIR := /usr/lib/pkgconfig
CFLAGS := -ggdb3 -O2 -Wall
CPPFLAGS := -Iinclude -Iarch/${ARCH} -Iarch/common
EXPORT_UNPREFIXED := yes
FREESTANDING := no

ifeq ($(FREESTANDING),yes)
	CFLAGS += -DFREESTANDING
	EXPORT_UNPREFIXED = no
endif

ifeq ($(EXPORT_UNPREFIXED),yes)
	CFLAGS += -DEXPORT_UNPREFIXED
endif

LIBUCONTEXT_C_SRC = $(wildcard arch/${ARCH}/*.c)
LIBUCONTEXT_S_SRC = $(wildcard arch/${ARCH}/*.S)

LIBUCONTEXT_VERSION := $(shell head -n 1 VERSION)
LIBUCONTEXT_OBJ = ${LIBUCONTEXT_C_SRC:.c=.o} ${LIBUCONTEXT_S_SRC:.S=.o}
LIBUCONTEXT_SOVERSION = 1
LIBUCONTEXT_NAME = libucontext.so
LIBUCONTEXT_STATIC_NAME = libucontext.a
LIBUCONTEXT_PC = libucontext.pc
LIBUCONTEXT_SONAME = libucontext.so.${LIBUCONTEXT_SOVERSION}
LIBUCONTEXT_PATH = ${LIBDIR}/${LIBUCONTEXT_SONAME}
LIBUCONTEXT_STATIC_PATH = ${LIBDIR}/${LIBUCONTEXT_STATIC_NAME}
LIBUCONTEXT_HEADERS = \
	include/libucontext/libucontext.h \
	include/libucontext/bits.h
LIBUCONTEXT_EXAMPLES = \
	examples/cooperative_threading
LIBUCONTEXT_POSIX_NAME = libucontext_posix.so
LIBUCONTEXT_POSIX_STATIC_NAME = libucontext_posix.a
LIBUCONTEXT_POSIX_C_SRC = libucontext_posix.c
LIBUCONTEXT_POSIX_OBJ = ${LIBUCONTEXT_POSIX_C_SRC:.c=.o}
LIBUCONTEXT_POSIX_SONAME = libucontext_posix.so.${LIBUCONTEXT_SOVERSION}
LIBUCONTEXT_POSIX_PATH = ${LIBDIR}/${LIBUCONTEXT_POSIX_SONAME}
LIBUCONTEXT_POSIX_STATIC_PATH = ${LIBDIR}/${LIBUCONTEXT_POSIX_STATIC_NAME}

ifeq ($(FREESTANDING),yes)
	LIBUCONTEXT_POSIX_NAME =
	LIBUCONTEXT_POSIX_STATIC_NAME =
endif

all: ${LIBUCONTEXT_SONAME} ${LIBUCONTEXT_STATIC_NAME} ${LIBUCONTEXT_POSIX_NAME} ${LIBUCONTEXT_POSIX_STATIC_NAME} ${LIBUCONTEXT_PC}

${LIBUCONTEXT_POSIX_NAME}: ${LIBUCONTEXT_NAME} ${LIBUCONTEXT_POSIX_OBJ}
	$(CC) -fPIC -o ${LIBUCONTEXT_POSIX_NAME} -Wl,-soname,${LIBUCONTEXT_POSIX_SONAME} \
		-shared ${LIBUCONTEXT_POSIX_OBJ} ${LDFLAGS}

${LIBUCONTEXT_POSIX_STATIC_NAME}: ${LIBUCONTEXT_STATIC_NAME} ${LIBUCONTEXT_POSIX_OBJ}
	$(AR) rcs ${LIBUCONTEXT_POSIX_STATIC_NAME} ${LIBUCONTEXT_POSIX_OBJ}

${LIBUCONTEXT_POSIX_SONAME}: ${LIBUCONTEXT_POSIX_NAME}
	ln -sf ${LIBUCONTEXT_POSIX_NAME} ${LIBUCONTEXT_POSIX_SONAME}

${LIBUCONTEXT_STATIC_NAME}: ${LIBUCONTEXT_HEADERS} ${LIBUCONTEXT_OBJ}
	$(AR) rcs ${LIBUCONTEXT_STATIC_NAME} ${LIBUCONTEXT_OBJ}

${LIBUCONTEXT_NAME}: ${LIBUCONTEXT_HEADERS} ${LIBUCONTEXT_OBJ}
	$(CC) -fPIC -o ${LIBUCONTEXT_NAME} -Wl,-soname,${LIBUCONTEXT_SONAME} \
		-shared ${LIBUCONTEXT_OBJ} ${LDFLAGS}

${LIBUCONTEXT_SONAME}: ${LIBUCONTEXT_NAME}
	ln -sf ${LIBUCONTEXT_NAME} ${LIBUCONTEXT_SONAME}

${LIBUCONTEXT_PC}: libucontext.pc.in
	sed -e s:@LIBUCONTEXT_VERSION@:${LIBUCONTEXT_VERSION}:g \
	    -e s:@LIBUCONTEXT_LIBDIR@:${LIBDIR}:g \
	    -e s:@LIBUCONTEXT_INCLUDEDIR@:${INCLUDEDIR}:g $< > $@

MANPAGES_SYMLINKS_3 = \
	libucontext_getcontext.3 \
	libucontext_makecontext.3 \
	libucontext_setcontext.3 \
	libucontext_swapcontext.3
MANPAGES_3 = doc/libucontext.3

MANPAGES = ${MANPAGES_3}

.scd.3:
	${SCDOC} < $< > $@

.SUFFIXES: .scd .3

docs: ${MANPAGES}

.c.o:
	$(CC) -std=gnu99 -D_BSD_SOURCE -fPIC -DPIC ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

.S.o:
	$(CC) -fPIC -DPIC ${CFLAGS} ${CPPFLAGS} -c -o $@ $<

${LIBUCONTEXT_NAME}_clean:
	rm -f ${LIBUCONTEXT_NAME}

${LIBUCONTEXT_SONAME}_clean:
	rm -f ${LIBUCONTEXT_SONAME}

${LIBUCONTEXT_STATIC_NAME}_clean:
	rm -f ${LIBUCONTEXT_STATIC_NAME}

libucontext_obj_clean:
	rm -f ${LIBUCONTEXT_OBJ}

${LIBUCONTEXT_PC}_clean:
	rm -f ${LIBUCONTEXT_PC}

bits_clean:
	rm -f include/libucontext/bits.h

${LIBUCONTEXT_POSIX_NAME}_clean:
	rm -f ${LIBUCONTEXT_POSIX_NAME}

${LIBUCONTEXT_POSIX_STATIC_NAME}_clean:
	rm -f ${LIBUCONTEXT_POSIX_STATIC_NAME}

libucontext_posix_obj_clean:
	rm -f ${LIBUCONTEXT_POSIX_OBJ}

check_clean: check_bare_clean check_posix_clean check_bare_posixabi_clean

check_bare_clean:
	rm -f test_libucontext

check_posix_clean:
	rm -f test_libucontext_posix

check_bare_posixabi_clean:
	rm -f test_libucontext_bare_posixabi

docs_clean:
	rm -f ${MANPAGES}

clean: ${LIBUCONTEXT_NAME}_clean
clean: ${LIBUCONTEXT_SONAME}_clean
clean: ${LIBUCONTEXT_STATIC_NAME}_clean
clean: ${LIBUCONTEXT_PC}_clean
clean: bits_clean
clean: ${LIBUCONTEXT_POSIX_NAME}_clean
clean: ${LIBUCONTEXT_POSIX_STATIC_NAME}_clean
clean: libucontext_posix_obj_clean
clean: libucontext_obj_clean
clean: check_clean
clean: docs_clean

install: all
	install -D -m755 ${LIBUCONTEXT_NAME} ${DESTDIR}${LIBUCONTEXT_PATH}
	install -D -m664 ${LIBUCONTEXT_STATIC_NAME} ${DESTDIR}${LIBUCONTEXT_STATIC_PATH}
	ln -sf ${LIBUCONTEXT_SONAME} ${DESTDIR}${LIBDIR}/${LIBUCONTEXT_NAME}
	for i in ${LIBUCONTEXT_HEADERS}; do \
		destfn=$$(echo $$i | sed s:include/::g); \
		install -D -m644 $$i ${DESTDIR}${INCLUDEDIR}/$$destfn; \
	done
	install -D -m644 ${LIBUCONTEXT_PC} ${DESTDIR}${PKGCONFIGDIR}/${LIBUCONTEXT_PC}
	if [ -n ${LIBUCONTEXT_POSIX_NAME} ]; then \
		install -D -m755 ${LIBUCONTEXT_POSIX_NAME} ${DESTDIR}${LIBUCONTEXT_POSIX_PATH}; \
		install -D -m644 ${LIBUCONTEXT_POSIX_STATIC_NAME} ${DESTDIR}${LIBUCONTEXT_POSIX_STATIC_PATH}; \
	fi

install_docs: docs
	install -D -m644 doc/libucontext.3 ${DESTDIR}/usr/share/man/man3/libucontext.3
	for i in ${MANPAGES_SYMLINKS_3}; do \
		ln -s libucontext.3 ${DESTDIR}/usr/share/man/man3/$$i; \
	done

ifneq (${FREESTANDING},yes)
check: check_libucontext_posix

check_libucontext_posix: test_libucontext_posix ${LIBUCONTEXT_POSIX_SONAME}
	env LD_LIBRARY_PATH=$(shell pwd) ./test_libucontext_posix

test_libucontext_posix: test_libucontext_posix.c ${LIBUCONTEXT_POSIX_NAME}
	$(CC) -std=gnu99 -D_BSD_SOURCE ${CFLAGS} ${CPPFLAGS} $@.c -o $@ -L. -lucontext -lucontext_posix
endif

ifeq ($(EXPORT_UNPREFIXED),yes)
check: check_libucontext_bare_posixabi

check_libucontext_bare_posixabi: test_libucontext_bare_posixabi ${LIBUCONTEXT_SONAME}
	env LD_LIBRARY_PATH=$(shell pwd) ./test_libucontext_bare_posixabi

test_libucontext_bare_posixabi: test_libucontext_posix.c ${LIBUCONTEXT_NAME}
	$(CC) -std=gnu99 -D_BSD_SOURCE ${CFLAGS} ${CPPFLAGS} test_libucontext_posix.c -o $@ -L. -lucontext
endif

check: test_libucontext ${LIBUCONTEXT_SONAME}
	env LD_LIBRARY_PATH=$(shell pwd) ./test_libucontext

test_libucontext: test_libucontext.c ${LIBUCONTEXT_NAME}
	$(CC) -std=gnu99 -D_BSD_SOURCE ${CFLAGS} ${CPPFLAGS} $@.c -o $@ -L. -lucontext

examples: ${LIBUCONTEXT_EXAMPLES}
examples/cooperative_threading: examples/cooperative_threading.c ${LIBUCONTEXT_NAME}
	$(CC) -std=gnu99 -D_BSD_SOURCE ${CFLAGS} ${CPPFLAGS} $@.c -o $@ -L. -lucontext

ifeq ($(FREESTANDING),no)

include/libucontext/bits.h: arch/common/include/libucontext/bits.h
	cp $< $@

else

include/libucontext/bits.h: arch/${ARCH}/include/libucontext/bits.h
	cp $< $@

endif

PACKAGE_NAME = libucontext
PACKAGE_VERSION = ${LIBUCONTEXT_VERSION}
DIST_NAME = ${PACKAGE_NAME}-${PACKAGE_VERSION}
DIST_TARBALL = ${DIST_NAME}.tar.xz

distcheck: check dist
dist: ${DIST_TARBALL}
${DIST_TARBALL}:
	git archive --format=tar --prefix=${DIST_NAME}/ -o ${DIST_NAME}.tar ${DIST_NAME}
	xz ${DIST_NAME}.tar

.PHONY: check dist
