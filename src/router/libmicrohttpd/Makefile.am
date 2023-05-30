# This Makefile.am is in the public domain
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = contrib m4 src .

if HAVE_PO
SUBDIRS += po
endif

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
  acinclude.m4 \
  libmicrohttpd.pc.in \
  $(W32COMMON) $(W32VS2013) $(W32VS2015) $(W32VS2017) $(W32VS2019) $(W32VS2022) $(W32VSAV)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmicrohttpd.pc

dist-hook:
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
	  { echo 'The archive file extention must be set by "ARC_EXT".' >&2; false; }
	-rm -f '$(distdir).$(ARC_EXT)'
	tardir=$(distdir) && $(am__tar) | $(ARC_CMD) >$(distdir).$(ARC_EXT)
	$(am__post_remove_distdir)

dist-custm2: distdir
	@test -n "$(ARC_CMD)" || \
	  { echo 'The valid archive command must be defined by "ARC_CMD".' >&2; false; }
	@test -n "$(ARC_EXT)" || \
	  { echo 'The archive file extention must be set by "ARC_EXT".' >&2; false; }
	-rm -f '$(distdir).$(ARC_EXT)'
	tardir=$(distdir) && $(am__tar) >$(distdir).tar && $(ARC_CMD) $(distdir).tar
	rm -f $(distdir).tar
	$(am__post_remove_distdir)

