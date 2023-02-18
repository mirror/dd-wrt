if PATH_PLUGINS
pkgplugindir=$(PATH_PLUGINS)
else
pkgplugindir=$(libdir)/libnfsidmap
endif

if ENABLE_LDAP
UMICH_LDAP_LIB = umich_ldap.la
else
UMICH_LDAP_LIB = 
endif
if ENABLE_GUMS
GUMS_MAPPING_LIB = gums.la
else
GUMS_MAPPING_LIB =
endif
if ENABLE_LDAP_SASL
KRB5_GSS_LIB=-lgssapi_krb5
endif
lib_LTLIBRARIES = libnfsidmap.la
pkgplugin_LTLIBRARIES = nsswitch.la static.la regex.la $(UMICH_LDAP_LIB) $(GUMS_MAPPING_LIB)

# Library versioning notes from:
#  http://sources.redhat.com/autobook/autobook/autobook_91.html
#
# -version-info <current>:<revision>:<age>
#  <current>	The number of the current interface exported by library.
#  <revision>	The implementation number of the most recent interface
#		exported by the library.  (i.e. revision should be updated
#		with each new release of the library, and reset to zero
#		when <current> is updated.)
#  <age> 	The number of previous additional interfaces supported
#  		by this library.

libnfsidmap_la_SOURCES = libnfsidmap.c nfsidmap_common.c
libnfsidmap_la_LDFLAGS = -version-info 1:0:0
libnfsidmap_la_LIBADD = -ldl ../../support/nfs/libnfsconf.la

nsswitch_la_SOURCES = nss.c nfsidmap_common.c
nsswitch_la_LDFLAGS = -module -avoid-version
nsswitch_la_LIBADD = ../../support/nfs/libnfsconf.la

static_la_SOURCES = static.c
static_la_LDFLAGS = -module -avoid-version
static_la_LIBADD = ../../support/nfs/libnfsconf.la

regex_la_SOURCES = regex.c
regex_la_LDFLAGS = -module -avoid-version
regex_la_LIBADD = ../../support/nfs/libnfsconf.la

umich_ldap_la_SOURCES = umich_ldap.c
umich_ldap_la_LDFLAGS = -module -avoid-version
umich_ldap_la_LIBADD = -lldap $(KRB5_GSS_LIB) ../../support/nfs/libnfsconf.la

gums_la_SOURCES = gums.c
gums_la_LDFLAGS = -module -avoid-version

man3_MANS = nfs4_uid_to_name.3
man5_MANS	= idmapd.conf.5
include_HEADERS = nfsidmap.h nfsidmap_plugin.h

EXTRA_DIST = $(man3_MANS) \
	     $(man5_MANS) \
	     libtest.c \
	     idmapd.conf

#	XXX: also exclude debian/files and debian/files.new ? do a clean??
dist-hook:
	mkdir $(distdir)/debian/
	find $(srcdir)/debian -maxdepth 1 -not -type d |xargs -i cp {} $(distdir)/debian/

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA = libnfsidmap.pc

$(pkgconfig_DATA): $(top_builddir)/config.status
