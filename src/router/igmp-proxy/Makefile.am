SUBDIRS = doc src
EXTRA_DIST = autogen.sh GPL.txt Stanford.txt
dist_sysconf_DATA = igmpproxy.conf

ChangeLog: $(srcdir)/.git/HEAD
	git --git-dir=$(srcdir)/.git log --format="%ai  %aN <%aE>%n%n%x09* %B" | sed 's/^\([^2\t]\)/\t  \1/' > $@

AUTHORS: ChangeLog
	( echo "Authors and contributors, in alphabetical order:"; echo; \
	sed -r "s/^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} [+-][0-9]{4}  (.*)/\1/;t;d" ChangeLog $< | LC_ALL=C sort -u ) > $@
