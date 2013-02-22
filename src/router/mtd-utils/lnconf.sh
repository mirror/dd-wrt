#!/bin/sh
#
# Generic configure replacement.
#
# $Id: lnconf.sh,v 1.1 2004/04/05 21:55:59 igor Exp $ 
#
# Copies all files from the script directory to the current one.
# Intended to replace 'configure' for packages that don't have one, to
# allow building outside of the source tree.
#
# Note: this does not do any fancy things with detecting shells and
# supporting other platforms.  But it should work on Cygwin.

# find out where the script is located
tdir=`echo "$0" | sed 's%[\\/][^\\/][^\\/]*$%%'`
test "x$tdir" = "x$0" && tdir=.

a_srcdir=`cd $tdir; pwd`
a_destdir=`pwd`

# sanity checks:
# are we in the script directory?
test "x$a_srcdir" = "x$a_destdir" && exit 0
# is there any chance that this is the script directory?
test "x`cd "$a_srcdir" && /bin/ls -id`" = "x`/bin/ls -id`" && exit 0

# try to find lndir and use it if it's available
LNDIR="`which lndir 2>/dev/null`"
if [ "x$LNDIR" = "x" ]; then
  lndir() {
    test "x$1" = "x" && return 1
    # be careful of the current directory
    DINODE=`find . -maxdepth 0 -ls | sed 's/ .*$//'` 
    case "`pwd`" in
      "`cd "$1" && pwd`"/*) CUR="-type d -inum $DINODE -prune -o";;
    esac
    # duplicate the directory structure
    (cd "$1" && find . $CUR -type d -mindepth 1 -print) | xargs -tr mkdir -p
    # copy all symbolic links
    (cd "$1" && find . $CUR -type l -mindepth 1 -print) | xargs -ri sh -c "ln -s \"\`readlink "$1/{}"\`\" \"{}\""
    # or simply
    #(cd "$1" && find . $CUR -type l -mindepth 1 -print) | xargs -ri ln -s "$1"/{} {}
    # link all files
    (cd "$1" && find . $CUR -type f -mindepth 1 -print) | xargs -ri ln -s "$1"/{} {}
  }
else
  lndir() {
    "$LNDIR" "$@"
  }
fi

lndir "$tdir"

