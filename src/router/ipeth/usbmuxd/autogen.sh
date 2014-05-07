libtoolize --force
aclocal -I m4
autoheader
automake -a --add-missing
autoconf
