#!/bin/sh
# do the funky auto* stuff

echo "Generating configure script using autoconf, automake and gettext"

gettextize --force --intl --no-changelog
aclocal -I m4
autoconf
autoheader
automake --add-missing --gnu

echo "Now you are ready to run ./configure with the desired options"
