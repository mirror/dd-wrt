SUBDIRS = src
sysconf_DATA = nocat.conf
man_MANS = man/splashd.8 man/nocat.conf.5
pkglibexec_SCRIPTS = @libexecscripts@
EXTRA_SCRIPTS = libexec/iptables/access.fw libexec/iptables/clear.fw libexec/iptables/dump.fw libexec/iptables/initialize.fw libexec/iptables/reset.fw libexec/iptables/throttle.fw libexec/loopback/access.fw libexec/loopback/clear.fw libexec/loopback/dump.fw libexec/loopback/initialize.fw 
htdocs_DATA = htdocs/splash.html htdocs/status.html htdocs/test.html
images_DATA = htdocs/images/auth_logo.gif htdocs/images/continue.gif htdocs/images/login.gif htdocs/images/logout.gif htdocs/images/register.gif htdocs/images/reset.gif htdocs/images/skip.gif htdocs/images/update.gif
pgp_DATA = pgp/trustedkeys.gpg
EXTRA_DIST = $(EXTRA_SCRIPTS) $(images_DATA) $(htdocs_DATA) $(pgp_DATA) $(man_MANS) nocat.conf.in rc.nocat
CLEANFILES = $(sysconf_DATA)

nocat.conf: nocat.conf.in
	sed -e s,@htdocsdir\@,$(htdocsdir),g \
	    -e s,@pgpdir\@,$(pgpdir),g \
	    -e s,@pkglibexecdir\@,$(pkglibexecdir),g < $< > $@
