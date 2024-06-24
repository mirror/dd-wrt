#!/bin/sh
# Run this to generate all the initial makefiles, etc.

DIE=0

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

# On macOS, all commands have been installed with the prefix "g" via brew
# installation.
# See https://formulae.brew.sh/formula/libtool
case "$(uname -sr)" in
  Darwin*)
    export PATH="$(brew --prefix)/opt/libtool/libexec/gnubin:$PATH"
    ;;
  *)
    ;;
esac

(libtoolize --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`libtool' installed to compile JamVM."
  echo "Install the appropriate package for your distribution,"
  echo "or get the source tarball at http://ftp.gnu.org/gnu/libtool"
  DIE=1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed to compile JamVM."
  echo "Install the appropriate package for your distribution,"
  echo "or get the source tarball at http://ftp.gnu.org/gnu/autoconf"
  DIE=1
  NO_AUTOCONF=yes
}

# autoheader is part of autoconf, but check it's present anyway
test -n "$NO_AUTOCONF" || (autoheader --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`autoheader'. This should be part of \'autoconf'."
  echo "Install the appropriate package for your distribution,"
  echo "or get the source tarball at http://ftp.gnu.org/gnu/autoconf"
  DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed to compile JamVM."
  echo "Install the appropriate package for your distribution,"
  echo "or get the source tarball at http://ftp.gnu.org/gnu/automake"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "Install the appropriate package for your distribution,"
  echo "or get the source tarball at http://ftp.gnu.org/gnu/automake"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

echo "Running aclocal $ACLOCAL_FLAGS ..."
aclocal $ACLOCAL_FLAGS || {
  echo
  echo "**Error**: aclocal failed. This may mean that you have not"
  echo "installed all of the packages you need, or you may need to"
  echo "set ACLOCAL_FLAGS to include \"-I \$prefix/share/aclocal\""
  echo "for the prefix where you installed the packages whose"
  echo "macros were not found"
  exit 1
}

echo "Running libtoolize ..."
libtoolize --force || { echo "**Error**: libtoolize failed."; exit 1; }

echo "Running autoheader ..."
autoheader || { echo "**Error**: autoheader failed."; exit 1; }

echo "Running automake --gnu ..."
automake --add-missing --gnu ||
  { echo "**Error**: automake failed."; exit 1; }

echo "Running autoconf ..."
autoconf || { echo "**Error**: autoconf failed."; exit 1; }
