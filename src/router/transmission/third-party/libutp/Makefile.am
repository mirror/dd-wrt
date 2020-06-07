AM_CPPFLAGS = -fno-exceptions -fno-rtti -ansi -DPOSIX

noinst_LIBRARIES = libutp.a
libutp_a_SOURCES = utp.cpp utp_utils.cpp
libutp_a_DEPENDENCIES = $(builddir)/include/libutp
noinst_HEADERS = StdAfx.h  templates.h  utp_config_example.h  utp.h  utp_config.h utp_utils.h  utypes.h
EXTRA_DIST = CMakeLists.txt LICENSE README.md

$(builddir)/include/libutp:
	$(MKDIR_P) $(builddir)/include
	(cd $(builddir)/include && $(LN_S) $(abs_srcdir) libutp)
