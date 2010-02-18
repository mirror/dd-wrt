../configure: aclocal.m4 configure.ac iconv.m4
	/bin/rm -rf autom4te.cache
	autoconf && mv ./configure ..
update: config.sub config.guess
config.guess: /usr/share/misc/config.guess
	/bin/cp -f /usr/share/misc/config.guess config.guess
config.sub: /usr/share/misc/config.sub
	/bin/cp -f /usr/share/misc/config.sub config.sub
