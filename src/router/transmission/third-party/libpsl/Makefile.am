# got some hints from https://gitorious.org/openismus-playground/examplelib/source

SUBDIRS = po include src tools
if ENABLE_GTK_DOC
  SUBDIRS += docs/libpsl
else
if ENABLE_MAN
  SUBDIRS += docs/libpsl
endif
endif
SUBDIRS += fuzz tests msvc

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

# Enable GTK-Doc during make distcheck
#DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-man

## Install the generated pkg-config file (.pc) into the expected location for
## architecture-dependent package configuration information. Occasionally,
## pkg-config files are also used for architecture-independent data packages,
## in which case the correct install location would be $(datadir)/pkgconfig.
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libpsl.pc

EXTRA_DIST = build-aux/config.rpath LICENSE meson.build  meson_options.txt
dist-hook:
	mkdir -p $(distdir)/list/tests
	cp -p $(PSL_FILE) $(distdir)/list
	cp -p $(PSL_TESTFILE) $(distdir)/list/tests

clean-local:
	rm -rf */*.gc?? */*/*.gc?? libpsl.info lcov

LCOV_INFO=libpsl.info
check-coverage: clean
	if test -z "$(XLIB)"; then \
		CFLAGS=$$CFLAGS" --coverage -O0" LDFLAGS=$$LDFLAGS" --coverage" ./configure --disable-runtime --disable-builtin; \
	else \
		CFLAGS=$$CFLAGS" --coverage -O0" LDFLAGS=$$LDFLAGS" --coverage" ./configure --enable-runtime=$(XLIB) --enable-builtin=$(XLIB); \
	fi
	$(MAKE)
	lcov --capture --initial --directory src --output-file $(LCOV_INFO)
	$(MAKE) check
	lcov --capture --directory src --output-file $(LCOV_INFO)
	genhtml --prefix . $(LCOV_INFO) --legend --title "libpsl" --output-directory=lcov
	@echo
	@echo "You can now view the coverage report with 'xdg-open lcov/index.html'"

check-coverage-libidn:
	XLIB=libidn $(MAKE) check-coverage

check-coverage-libidn2:
	XLIB=libidn2 $(MAKE) check-coverage

check-coverage-libicu:
	XLIB=libicu $(MAKE) check-coverage

fuzz-coverage: clean
	$(MAKE) -C src CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	$(MAKE) -C fuzz fuzz-coverage CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
	lcov --capture --initial --directory src --directory fuzz --output-file $(LCOV_INFO)
	lcov --capture --directory src --directory fuzz --output-file $(LCOV_INFO)
#	lcov --remove $(LCOV_INFO) '*/test_linking.c' '*/css_tokenizer.lex' '*/<stdout>' -o $(LCOV_INFO)
	genhtml --prefix . --ignore-errors source $(LCOV_INFO) --legend --title "libpsl-fuzz" --output-directory=lcov
	@echo
	@echo "You can now view the coverage report with 'xdg-open lcov/index.html'"

