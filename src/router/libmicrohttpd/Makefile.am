# This Makefile.am is in the public domain
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = contrib m4 src .

if BUILD_DOC
SUBDIRS += doc
endif


W32COMMON = w32/common/gen_dll_res.ps1 w32/common/microhttpd_dll_res_vc.rc.in w32/common/microhttpd_dll_res_vc.rc \
  w32/common/MHD_config.h w32/common/vs_dirs.props \
  w32/common/common-build-settings.props w32/common/libmicrohttpd-build-settings.props \
  w32/common/apps-build-settings.props \
  w32/common/project-configs.props w32/common/project-configs-xp.props \
  w32/common/libmicrohttpd-files.vcxproj w32/common/libmicrohttpd-filters.vcxproj \
  w32/common/hellobrowser-files.vcxproj w32/common/hellobrowser-filters.vcxproj
W32VS2013 = w32/VS2013/libmicrohttpd.vcxproj w32/VS2013/libmicrohttpd.vcxproj.filters \
  w32/VS2013/hellobrowser.vcxproj w32/VS2013/hellobrowser.vcxproj.filters \
  w32/VS2013/simplepost.vcxproj w32/VS2013/largepost.vcxproj \
  w32/VS2013/libmicrohttpd.sln
W32VS2015 = w32/VS2015/libmicrohttpd.vcxproj w32/VS2015/libmicrohttpd.vcxproj.filters \
  w32/VS2015/hellobrowser.vcxproj w32/VS2015/hellobrowser.vcxproj.filters \
  w32/VS2015/simplepost.vcxproj w32/VS2015/largepost.vcxproj \
  w32/VS2015/libmicrohttpd.sln
W32VS2017 = w32/VS2017/libmicrohttpd.vcxproj w32/VS2017/libmicrohttpd.vcxproj.filters \
  w32/VS2017/hellobrowser.vcxproj w32/VS2017/hellobrowser.vcxproj.filters \
  w32/VS2017/simplepost.vcxproj w32/VS2017/largepost.vcxproj \
  w32/VS2017/libmicrohttpd.sln
W32VS2019 = w32/VS2019/libmicrohttpd.vcxproj w32/VS2019/libmicrohttpd.vcxproj.filters \
  w32/VS2019/hellobrowser.vcxproj w32/VS2019/hellobrowser.vcxproj.filters \
  w32/VS2019/simplepost.vcxproj w32/VS2019/largepost.vcxproj \
  w32/VS2019/libmicrohttpd.sln
W32VS2022 = w32/VS2022/libmicrohttpd.vcxproj w32/VS2022/libmicrohttpd.vcxproj.filters \
  w32/VS2022/hellobrowser.vcxproj w32/VS2022/hellobrowser.vcxproj.filters \
  w32/VS2022/simplepost.vcxproj w32/VS2022/largepost.vcxproj \
  w32/VS2022/libmicrohttpd.sln
W32VSAV = w32/VS-Any-Version/libmicrohttpd.vcxproj w32/VS-Any-Version/libmicrohttpd.vcxproj.filters \
  w32/VS-Any-Version/hellobrowser.vcxproj w32/VS-Any-Version/hellobrowser.vcxproj.filters \
  w32/VS-Any-Version/simplepost.vcxproj w32/VS-Any-Version/largepost.vcxproj \
  w32/VS-Any-Version/libmicrohttpd.sln

EXTRA_DIST = \
  libmicrohttpd.pc.in \
  $(W32COMMON) $(W32VS2013) $(W32VS2015) $(W32VS2017) \
  $(W32VS2019) $(W32VS2022) $(W32VSAV)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmicrohttpd.pc

EXTRA_DIST += pre-dist-hook-dummy
MOSTLYCLEANFILES = pre-dist-hook-dummy
DISTCLEANFILES =
MAINTAINERCLEANFILES = m4/c_backported.m4

pre-dist-hook-dummy: pre-dist-hook Makefile
	@echo "dummy" > $@

dist-hook: dist-po
	@chmod u+w '$(distdir)/pre-dist-hook-dummy' && \
	  rm -f '$(distdir)/pre-dist-hook-dummy'
	@rm -f pre-dist-hook-dummy
	@if test -w '$(distdir)/m4/c_backported.m4'; then \
	  echo "Use empty m4/c_backported.m4 for dist target"; \
	  touch -r '$(distdir)/m4/c_backported.m4' '$(distdir)/m4/c_backported.m4-tmst' && \
	    echo 'dnl Not used for distribution' > '$(distdir)/m4/c_backported.m4' && \
	    touch -r '$(distdir)/m4/c_backported.m4-tmst' '$(distdir)/m4/c_backported.m4' && \
	    rm -f '$(distdir)/m4/c_backported.m4-tmst'; \
	else \
	  true; \
	fi

dist-custm: distdir
	@test -n "$(ARC_CMD)" || \
	  { echo 'The valid archive command must be defined by "ARC_CMD".' >&2; false; }
	@test -n "$(ARC_EXT)" || \
	  { echo 'The archive file extension must be set by "ARC_EXT".' >&2; false; }
	-rm -f '$(distdir).$(ARC_EXT)'
	tardir=$(distdir) && $(am__tar) | $(ARC_CMD) >$(distdir).$(ARC_EXT)
	$(am__post_remove_distdir)

dist-custm2: distdir
	@test -n "$(ARC_CMD)" || \
	  { echo 'The valid archive command must be defined by "ARC_CMD".' >&2; false; }
	@test -n "$(ARC_EXT)" || \
	  { echo 'The archive file extension must be set by "ARC_EXT".' >&2; false; }
	-rm -f '$(distdir).$(ARC_EXT)'
	tardir=$(distdir) && $(am__tar) >$(distdir).tar && $(ARC_CMD) $(distdir).tar
	rm -f $(distdir).tar
	$(am__post_remove_distdir)

pre-dist-hook: pre-dist-hook-doc
	@echo "Preparing to make dist"

pre-dist-hook-doc:
	@echo "Preparing to make dist in doc"
	@if test -w '$(top_srcdir)/doc' ; then \
	  $(am__cd) doc && $(MAKE) $(AM_MAKEFLAGS) update-stamp; \
	else \
	  echo "Source tree is read-only, skipping force doc update"; \
	fi;

.PHONY: pre-dist-hook pre-dist-hook-doc

# Works with old automake versions (<1.12.2) as "false"
MHD_V = $(AM_V_P) false

distclean-local: distclean-po

maintainer-clean-local: maintainer-clean-po

srcdir_po = $(top_srcdir)/po

PO_ACLOCAL_M4 = $(srcdir_po)/aclocal.m4

PO_MAIN_FILES = $(srcdir_po)/Makefile.in.in $(srcdir_po)/remove-potcdate.sin \
  $(srcdir_po)/quot.sed $(srcdir_po)/boldquot.sed \
  $(srcdir_po)/en@quot.header $(srcdir_po)/en@boldquot.header \
  $(srcdir_po)/insert-header.sin $(srcdir_po)/Rules-quot

PO_EXTRA_FILES = $(srcdir_po)/Makevars.template

PO_ROOT_FILES = $(srcdir_po)/ABOUT-NLS

PO_M4_FILES = $(srcdir_po)/m4/gettext.m4 $(srcdir_po)/m4/host-cpu-c-abi.m4 \
  $(srcdir_po)/m4/iconv.m4 $(srcdir_po)/m4/intlmacosx.m4 \
  $(srcdir_po)/m4/lib-ld.m4 $(srcdir_po)/m4/lib-link.m4 \
  $(srcdir_po)/m4/lib-prefix.m4 $(srcdir_po)/m4/nls.m4 \
  $(srcdir_po)/m4/po.m4 $(srcdir_po)/m4/progtest.m4

PO_AUX_FILES = $(srcdir_po)/$(MHD_AUX_DIR)/config.rpath

# All autopoint-created files
PO_ALL_FILES = $(PO_MAIN_FILES) $(PO_EXTRA_FILES) $(PO_ROOT_FILES) \
  $(PO_M4_FILES) $(PO_AUX_FILES)

am__po_aclocal_m4_deps = $(PO_M4_FILES) $(srcdir_po)/configure.ac
am__po_configure_deps = $(am__po_aclocal_m4_deps) $(PO_ACLOCAL_M4)

$(PO_ACLOCAL_M4): $(am__po_aclocal_m4_deps)
	@{ $(MHD_V) && echo "Building $@" ; } || true
	@echo "cd $(srcdir_po) && $(ACLOCAL) $(ACLOCAL_AMFLAGS)" && \
	  $(am__cd) '$(srcdir_po)' && $(ACLOCAL) $(ACLOCAL_AMFLAGS)

# Do update po/configure.ac only if template files updated
$(srcdir_po)/configure.ac: $(srcdir_po)/po-configure.ac.in $(top_srcdir)/configure.ac
	@{ $(MHD_V) && echo "Building $@" ; } || true
	@$(MAKE) $(AM_MAKEFLAGS) po/po-configure.ac
	@cp -f po/po-configure.ac '$@'

$(top_srcdir)/po-configure: $(srcdir_po)/configure.ac $(PO_ACLOCAL_M4) $(PO_AUX_FILES) $(srcdir_po)/$(MHD_AUX_DIR)/install-sh
	@{ $(MHD_V) && echo "Building $@" ; } || true
	@echo "cd $(srcdir_po) && $(AUTOCONF)" && \
	  ( $(am__cd) '$(srcdir_po)' && $(AUTOCONF) )
	mv -f '$(srcdir_po)/configure' '$@'
	-chmod a-x '$@'

EXTRA_DIST += $(top_srcdir)/$(MHD_AUX_DIR)/config.rpath $(srcdir_po)/$(MHD_AUX_DIR)/install-sh \
  $(PO_ALL_FILES) \
  $(PO_ACLOCAL_M4) \
  $(srcdir_po)/po-configure.ac.in $(srcdir_po)/configure.ac \
  $(top_srcdir)/po-configure \
  $(srcdir_po)/stamp-m.in

DISTCLEANFILES += config.main.log po-config.log po/stamp-m
MAINTAINERCLEANFILES += $(srcdir_po)/configure.ac

$(srcdir_po)/stamp-m.in:
	@: > '$@'

po-config.status: $(top_srcdir)/po-configure $(top_srcdir)/$(MHD_AUX_DIR)/install-sh $(top_srcdir)/$(MHD_AUX_DIR)/config.rpath
	@if test -f config.log; then \
	  mv -f config.log config.main.log; \
	else \
	  true; \
	fi
	@SHELL@ '$(top_srcdir)/po-configure' $(ac_configure_args) --silent --no-create --no-recursion --disable-option-checking
	@mv -f config.log po-config.log
	@if test -f config.main.log; then \
	  mv -f config.main.log config.log; \
	else \
	  true; \
	fi

po/Makefile: $(srcdir_po)/Makefile.in.in $(srcdir_po)/Makevars po-config.status $(srcdir_po)/POTFILES.in $(srcdir_po)/stamp-m.in
	@: && @SHELL@ ./po-config.status po/stamp-m po/Makefile.in po-directories

dist-po: po/Makefile $(PO_MAIN_FILES)
	@dir1="po"; dir2="$(distdir)/po"; \
	$(am__relativize); \
	rel_distsubdir=$$reldir; \
	echo "cd po && $(MAKE) $(AM_MAKEFLAGS) distdir='$$rel_distsubdir' distdir" && \
	$(am__cd) po && $(MAKE) $(AM_MAKEFLAGS) distdir="$$rel_distsubdir" distdir

$(srcdir_po)/POTFILES.in: $(top_srcdir)/src/microhttpd/Makefile.am
	@echo "cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) update-po-POTFILES.in" && \
	$(am__cd) '$(top_builddir)/src/microhttpd' && $(MAKE) $(AM_MAKEFLAGS) update-po-POTFILES.in

AUTOPOINT = autopoint
AUTOPOINT_FLAGS =

APIM_LOCK_BASE = autopoint-updating-lock
APIM_TMSTMP_BASE = autopoint-timestamp
APIM_TMSTMP_TMP_BASE = $(APIM_TMSTMP_BASE)-tmp
APIM_TRIGGER_BASE = autopoint-trigger
APIM_LOCK = $(srcdir_po)/$(APIM_LOCK_BASE)
APIM_TMSTMP = $(srcdir_po)/$(APIM_TMSTMP_BASE)
APIM_TMSTMP_TMP = $(srcdir_po)/$(APIM_TMSTMP_TMP_BASE)
APIM_TRIGGER = $(srcdir_po)/$(APIM_TRIGGER_BASE)
# Run 'autopoint' even if no file missing
FORCE_AUTOPOINT_CHECK = no

LOCK_TIMEOUT_VALUE = 120

EXTRA_DIST += $(APIM_TMSTMP) $(APIM_TRIGGER)

sleep_with_timeout = \
    sleep 1; \
    sec_waiting=`expr ${sec_waiting} + 1`; \
    if test $${sec_waiting} -gt '$(LOCK_TIMEOUT_VALUE)'; then \
        echo "Waiting timeout" 1>&2; \
        rmdir "$${lock_name}" ; \
        exit 1; \
    fi

wait_for_unlock = \
    test -n "$${lock_name}" || lock_name='$(APIM_LOCK)'; \
    if test -d "$${lock_name}"; then \
        { $(MHD_V) && \
            echo "Autopoint files are being updated in parallel thread, wait"; } || : ; \
        sec_waiting=0; \
        while test -d "$${lock_name}"; do \
            $(sleep_with_timeout) ; \
        done; \
    fi

apim_prepare = \
    $(am__cd) '$(srcdir_po)' || exit 1; \
    lock_name='$(APIM_LOCK_BASE)'; \
    { trap "rm -rf $(APIM_TMSTMP_BASE) $(APIM_TMSTMP_TMP_BASE) $${lock_name} po" HUP ABRT INT TERM 2>/dev/null && \
    trap "rm -rf $(APIM_TMSTMP_BASE) $(APIM_TMSTMP_TMP_BASE) $${lock_name} po" PIPE 2>/dev/null ; } || \
    trap "rm -rf $(APIM_TMSTMP_BASE) $(APIM_TMSTMP_TMP_BASE) $${lock_name} po" 1 2 13 15; \
    lock_held='false'; \
    sec_waiting=0; \
    while : ; do \
        if mkdir "$${lock_name}" 2>/dev/null; then lock_held=':' ; else : ; fi; \
        $${lock_held} && break || : ; \
        test 'xyes' = 'x$(FORCE_AUTOPOINT_CHECK)' || break ; \
        $(sleep_with_timeout) ; \
    done; \
    if $${lock_held} ; then \
        rm -f '$(APIM_TMSTMP_BASE)' '$(APIM_TMSTMP_TMP_BASE)' || exit 1; \
        echo 'timestamp' > '$(APIM_TMSTMP_TMP_BASE)' || exit 1; \
        if test 'xyes' = 'x$(FORCE_AUTOPOINT_CHECK)'; then \
            call_autopoint=':'; check_all='false'; \
        else \
            call_autopoint='false'; check_all=':'; \
        fi; \
        if $(MHD_V) ; then apim_echo='echo' ; else apim_echo=':'; fi ;\
        $$apim_echo "Sync autopoint files"; \
        files_missing='false'; \
        $(MKDIR_P) 'po'; \
        $(MKDIR_P) '$(MHD_AUX_DIR)'; \
        main_f_names=`for f in $(PO_MAIN_FILES) $(PO_EXTRA_FILES); do echo "$$f"; done | $(SED) -e 's|^.*/||'`; \
        root_f_names=`for f in $(PO_ROOT_FILES); do echo "$$f"; done | $(SED) -e 's|^.*/||'`; \
        m4_f_names=`for f in $(PO_M4_FILES); do echo "$$f"; done | $(SED) -e 's|^.*/||'`; \
        aux_f_names=`for f in $(PO_AUX_FILES); do echo "$$f"; done | $(SED) -e 's|^.*/||'`; \
        for f in $${main_f_names}; do \
            if test -f "$$f"; then \
                 cp -f "$$f" "po/$$f" ; \
            else \
                $$apim_echo "Missing $(srcdir_po)/$$f" ; \
                files_missing=':' ; \
            fi; \
        done; \
        if $${check_all} ; then \
            for f in $${root_f_names}; do \
                if test -f "./$$f"; then : ; \
                else \
                    $$apim_echo "Missing $(srcdir_po)/$$f" ; \
                    files_missing=':' ; \
                fi; \
            done; \
            for f in $${m4_f_names}; do \
                if test -f "m4/$$f"; then : ; \
                else \
                    $$apim_echo "Missing $(srcdir_po)/m4/$$f" ; \
                    files_missing=':' ; \
                fi; \
            done; \
            for f in $${aux_f_names}; do \
                if test -f "$(MHD_AUX_DIR)/$$f"; then : ;\
                else \
                    $$apim_echo "Missing $(srcdir_po)/$(MHD_AUX_DIR)/$$f" ; \
                    files_missing=':' ; \
                fi; \
            done; \
        fi; \
        if $${files_missing} ; then \
            call_autopoint=':' ; \
            $$apim_echo "Some files are missing, call autopoint to restore them" ; \
        elif $${call_autopoint}; then $$apim_echo "Check and update autopoint files" ; \
        fi; \
        rc_code=0; \
        if $${call_autopoint} ; then \
            echo '  cd $(srcdir_po) && $(AUTOPOINT) $(AUTOPOINT_FLAGS)'; \
            '$(AUTOPOINT)' $(AUTOPOINT_FLAGS) || rc_code=1; \
        fi; \
        if test "$${rc_code}" = "0"; then \
            $$apim_echo "Re-sync autopoint files back"; \
            for f in $${aux_f_names}; do \
                if test -f "$(MHD_AUX_DIR)/$$f"; then \
                    touch -f "$(MHD_AUX_DIR)/$$f" && \
                    cp -f "$(MHD_AUX_DIR)/$$f" "../$(MHD_AUX_DIR)/$$f"; \
                else \
                    $$apim_echo "Still missing $(srcdir_po)/$(MHD_AUX_DIR)/$$f" 1>&2 ; \
                    rc_code=1; \
                fi; \
            done; \
            for f in $${m4_f_names}; do \
                if test -f "m4/$$f"; then \
                    touch "m4/$$f"; \
                else \
                    $$apim_echo "Still missing $(srcdir_po)/m4/$$f" 1>&2 ; \
                    rc_code=1; \
                fi; \
            done; \
            for f in $${root_f_names}; do \
                if test -f "./$$f"; then \
                    touch "./$$f"; \
                else \
                    $$apim_echo "Still missing $(srcdir_po)/$$f" 1>&2 ; \
                    rc_code=1; \
                fi; \
            done; \
            for f in $${main_f_names}; do \
                if test -f "po/$$f"; then \
                    cp -f "po/$$f" "$$f"; \
                else \
                    $$apim_echo "Still missing $(srcdir_po)/$$f" 1>&2 ; \
                    rc_code=1; \
                fi; \
            done; \
        fi; \
        if test $${rc_code} = 0; then \
            cp -fp '$(APIM_TMSTMP_TMP_BASE)' '$(APIM_TMSTMP_BASE)' ; \
            touch -r '$(APIM_TMSTMP_TMP_BASE)' '$(APIM_TMSTMP_BASE)' ; \
            $$apim_echo "All autopoint files have been synced"; \
            rm -f '$(APIM_TMSTMP_TMP_BASE)'; \
        else \
            rm -f '$(APIM_TMSTMP_BASE)' '$(APIM_TMSTMP_TMP_BASE)'; \
        fi; \
        rm -fr 'po'; \
        rmdir "$${lock_name}"; \
        exit $${rc_code} ; \
    else \
        $(wait_for_unlock) ; \
        test -f '$(APIM_TMSTMP_BASE)' || exit 1; \
    fi; \
    :

mostlyclean-local: mostlyclean-autopoint-update
mostlyclean-autopoint-update:
	-test -d '$(APIM_LOCK)' && rm -rf '$(APIM_LOCK)'
	-test -d '$(srcdir_po)/po' && rm -rf '$(srcdir_po)/po'
	rm -f '$(APIM_TMSTMP_TMP)'

$(APIM_TRIGGER):
	@echo "autopoint files rebuild trigger" > "$@"

$(APIM_TMSTMP): $(srcdir_po)/configure.ac $(APIM_TRIGGER)
	@ $(apim_prepare)

.DELETE_ON_ERROR: $(APIM_TMSTMP)

$(PO_ALL_FILES): $(APIM_TMSTMP) $(srcdir_po)/configure.ac
	@ ( $(apim_prepare) )
	@test -f '$@'

prepare-autopoint: $(srcdir_po)/configure.ac
	@ ( $(apim_prepare) )
	@test -f '$(APIM_TMSTMP)'

check-prepare-autopoint: $(srcdir_po)/$(MHD_AUX_DIR)/install-sh $(APIM_TMSTMP)
	@echo "Check all autopoint files"; \
	    files_missing='false'; \
	    for f in $(PO_ALL_FILES); do \
	        if test -f "$$f"; then : ; \
	        else \
	            echo "Missing $$f" ; \
	            files_missing=':' ; \
	        fi; \
	    done; \
	    if $${files_missing}; then \
	        $(MAKE) $(AM_MAKEFLAGS) prepare-autopoint || exit 1; \
	    else \
	        exit 0; \
	    fi

# This could be used to update autopoint files for git or for 'dist' target
update-autopoint-force:
	$(MAKE) $(AM_MAKEFLAGS) 'FORCE_AUTOPOINT_CHECK=yes' 'AUTOPOINT_FLAGS=$(AUTOPOINT_FLAGS) -f' prepare-autopoint

autopoint-files-all: $(APIM_TRIGGER) $(APIM_TMSTMP) $(PO_MAIN_FILES) $(PO_EXTRA_FILES) $(PO_M4_FILES) $(PO_AUX_FILES) $(srcdir_po)/$(MHD_AUX_DIR)/install-sh

$(top_srcdir)/$(MHD_AUX_DIR)/config.rpath: $(srcdir_po)/$(MHD_AUX_DIR)/config.rpath
	cp -f '$(srcdir_po)/$(MHD_AUX_DIR)/config.rpath' '$@'

$(srcdir_po)/$(MHD_AUX_DIR)/install-sh: $(top_srcdir)/$(MHD_AUX_DIR)/install-sh
	$(MKDIR_P) '$(srcdir_po)/$(MHD_AUX_DIR)'
	cp -f '$(top_srcdir)/$(MHD_AUX_DIR)/install-sh' '$@'

.PHONY: mostlyclean-autopoint-update prepare-autopoint update-autopoint-force check-prepare-autopoint autopoint-files-all

distclean-po:
	@( if test -f po/Makefile; then \
	   $(am__cd) po && $(MAKE) $(AM_MAKEFLAGS) distclean; \
	else \
	  true; \
	fi )
	-rm -f po-config.status po/Makefile po/stamp-m
	-rm -f po/POTFILES po/remove-potcdate.sed po/Makefile.in

maintainer-clean-po:
	@( if test -f po/Makefile; then \
	  $(am__cd) po && $(MAKE) $(AM_MAKEFLAGS) maintainer-clean; \
	else \
	  true; \
	fi )
	-rm -f po/$(PACKAGE_TARNAME).pot
	-rm -rf $(srcdir_po)/autom4te.cache

.PHONY: distclean-po maintainer-clean-po
