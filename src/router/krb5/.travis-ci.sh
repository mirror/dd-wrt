cd src
autoreconf
./configure --enable-maintainer-mode --with-ldap
make $MAKEVARS
make check
make distclean
# Check for files unexpectedly not removed by make distclean.
rm -rf autom4te.cache configure include/autoconf.h.in
if [ -n "$(git ls-files -o)" ]; then
  exit 1
fi
