#!/bin/sh
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# Run this to generate all the initial makefiles for nProbe
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#
# Copyright (C) 2004 Rocco Carbone <rocco@ntop.org>
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#

# Defaults
LIBTOOL=libtool
LIBTOOLIZE=libtoolize


# OSx
(uname -a|grep -v Darwin) < /dev/null > /dev/null 2>&1 ||
{
echo "Adding fix for OSX"
LIBTOOL=glibtool
LIBTOOLIZE=glibtoolize
}



# The name of this program.
progname=`echo "$0" | sed 's%^.*/%%'`

GNU_OR_DIE=1

help="Try \`$progname --help' for more information"


for arg
do
  case "$arg" in
  -h | --help)
    cat <<EOF
Usage: $progname [OPTION]...


This script should help you to configure 'nProbe'

-h, --help            display this message and exit
-v, --version         print version information and exit
-d, --debug           enable verbose shell tracing
-p, --purge           purge all files which are not part of the source package

EOF
    exit 0
    ;;

  -v | --version)
    echo "$progname 0.1.0"
    exit 0
    ;;

  -d | --debug)
    echo "$progname: enabling shell trace mode"
    set -x
    ;;

  esac
done


($LIBTOOL --version) < /dev/null > /dev/null 2>&1 ||
{
  echo
  echo "You must have libtool installed to compile $#NAME#."
  echo "Download the appropriate package for your distribution, or get the"
  echo "source tarball at ftp://ftp.gnu.org/pub/gnu/libtool-1.5.0a.tar.gz"
  GNU_OR_DIE=0
}

(automake --version) < /dev/null > /dev/null 2>&1 ||
{
  echo
  echo "You must have automake installed to compile $#NAME#."
  echo "Download the appropriate package for your distribution, or get the"
  echo "source tarball at ftp://ftp.gnu.org/pub/gnu/automake-1.6.3.tar.gz"
  GNU_OR_DIE=0
}

(autoconf --version) < /dev/null > /dev/null 2>&1 ||
{
  echo
  echo "You must have autoconf installed to compile $progname."
  echo "Download the appropriate package for your distribution, or get the"
  echo "source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf-2.13.tar.gz"
  GNU_OR_DIE=0
}


if test "$GNU_OR_DIE" -eq 0; then
  exit 1
fi


if [ $# != 0 ]
  then
     case $1 in
        -p | --purge)
          shift

          # cleanup from previous run and exit

          echo "cleaning file system from locally generated files..."

          if [ -f Makefile ]; then
            make -k clean
          fi

          rm -rf .deps

          rm -f libtool.m4.in
          rm -f config.guess
          rm -f config.sub
          rm -f install-sh
          rm -f ltconfig
          rm -f ltmain.sh
          rm -f missing
          rm -f mkinstalldirs
          rm -f INSTALL
          rm -f COPYING
          rm -f texinfo.tex

          rm -f acinclude.m4
          rm -f aclocal.m4
          rm -f config.h.in
          rm -f stamp-h.in
          rm -f Makefile.in

          rm -f configure
          rm -f config.h
          rm -f depcomp
          rm -f stamp.h
          rm -f libtool
          rm -f Makefile
          rm -f stamp-h.in
          rm -f stamp-h
          rm -f stamp-h1

          rm -f config.cache
          rm -f config.status
          rm -f config.log

          rm -fr autom4te.cache

          rm -f Makefile
          rm -f Makefile.in
          
          rm -f compile
          
          rm -f plugins/Makefile
          rm -f plugins/Makefile.in

          rm -f *~

          exit 1
        ;;

#
# all the other options are passed to the configure script
#
#        -*)
#          echo "$progname: unrecognized option \`$arg'" 1>&2
#          echo "$help" 1>&2
#          exit 1
#        ;;
#
#        *)
#          echo "$progname: too many arguments" 1>&2
#          echo "$help" 1>&2
#          exit 1
#        ;;
esac
fi

echo
echo "Trying to configure nProbe please wait...."
echo


#
# 0. prepare the package to use libtool
#
$LIBTOOLIZE --copy --force

if [ ! -f libtool.m4.in ]; then
  if [ -f /usr/local/share/aclocal/libtool.m4 ]; then
     cp /usr/local/share/aclocal/libtool.m4 libtool.m4.in
     echo "0. libtool.m4.in ... done"
  else
     if [ -f /usr/share/aclocal/libtool.m4 ]; then
      cp /usr/share/aclocal/libtool.m4 libtool.m4.in
      echo "0. libtool.m4.in ... done"
     fi
  fi
fi


#
# 1. create local definitions for automake
#
cat acinclude.m4.in libtool.m4.in > acinclude.m4
echo "1. acinclude.m4 .... done"


#
# 2. run 'aclocal' to create aclocal.m4 from configure.in (optionally acinclude.m4)
#
aclocal $ACLOCAL_FLAGS
echo "2. aclocal.m4 ...... done"

#
# 3. run 'autoheader' to create config.h.in from configure.in
#
autoheader
echo "3. config.h.in ..... done"

echo "timestamp" > stamp-h.in

#
# 4.
# run 'automake' to create Makefile.in from configure.in and Makefile.am
# (optionally aclocal.m4)
# the generated Makefile.in is compliant to GNU Makefile standard
#
touch NEWS AUTHORS ChangeLog
automake --add-missing --copy
echo "4. Makefile.in ..... done"

#
# 5.
# run 'autoconf' to create configure from configure.in
#
autoconf
echo "5. configure ....... done"
echo ""

#
# 6.
# run './configure' for real fun!
#
if [ -x config.status -a -z "$*" ]; then
  ./config.status --recheck
else
  if test -z "$*"; then
    echo "I am going to run ./configure with no arguments"
    echo "if you wish to pass any to it, please specify them on the $0 command line."
  fi
  ./configure "$@" || exit 1
fi
echo ""

#
# 7. Have fun
#
echo "just type make to compile nProbe"
echo ""

#
# cleanup to handle programs garbage
#
rm -f /tmp/acin* /tmp/acout*
rm -f autoha*
rm -f confdefs.h


# Local Variables: 
# mode:shell-script 
# sh-indentation:2 
# End: 
