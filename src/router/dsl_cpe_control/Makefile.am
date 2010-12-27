## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.9 nostdinc

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@

dsl_cpe_control_docdir = ${prefix}/doc/dsl_cpe_control

dsl_cpe_control_doc_DATA = \
	ChangeLog
   
EXTRA_DIST = $(dsl_cpe_control_doc_DATA) \
	build_vxworks/control_dsl_cpe_api.wpj \
	build_win32/dsl_cpe_control_version.h \
	build_win32/vc7/dsl_cpe_control.sln \
	build_win32/vc7/dsl_cpe_control.vcproj \
	README\
	AUTHORS\
	NEWS\
	TODO

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

lint:
	@target=lint; \
	list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		echo "Making $$target in $$subdir"; \
		if test "$$subdir" = "."; then \
		dot_seen=yes; \
		local_target="$$target-am"; \
		else \
		local_target="$$target"; \
		fi; \
		(cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
		|| eval $$failcom; \
	done;