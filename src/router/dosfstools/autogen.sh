#!/bin/sh

# Install config.rpath which is needed for AM_ICONV macro
for dir in "$GETTEXT_DIR" /usr/share/gettext /usr/local/share/gettext; do
  if test -f "$dir/config.rpath"; then
    test -f config.rpath || echo "autogen.sh: installing './config.rpath'"
    cp -f "$dir/config.rpath" .
    break
  fi
done

aclocal --force
autoconf --force
automake --add-missing --copy --force-missing
