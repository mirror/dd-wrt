#! /bin/sh

if test -z "$CPP"; then
  CPP="cc -E"
fi
if test -z "$srcdir"; then
  srcdir=.
fi

rm -f core*

sed < $srcdir/osdef.h.in -n -e '/^extern/s@.*[)* 	][)* 	]*\([^ *]*\) __P.*@/[)*, 	]\1[ 	(]/i\\\
\\/\\[^a-zA-Z_\\]\1 __P\\/d@p' > osdef1.sed
cat << EOF > osdef0.c
#include "config.h"
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>
#ifdef SHADOWPW
#include <shadow.h>
#endif
#ifndef sun
#include <sys/ioctl.h>
#endif
#ifdef linux
#include <string.h>
#include <stdlib.h>
#endif
#include <sys/socket.h>
#ifndef NOSYSLOG
#include <syslog.h>
#endif
#include "os.h"
#if defined(UTMPOK) && defined (GETTTYENT) && !defined(GETUTENT)
#include <ttyent.h>
#endif
#ifdef SVR4
# include <sys/resource.h>
#endif
EOF
cat << EOF > osdef2.sed
1i\\
/*
1i\\
 * This file is automagically created from osdef.sh -- DO NOT EDIT
1i\\
 */
EOF
$CPP -I. -I$srcdir osdef0.c | sed -n -f osdef1.sed >> osdef2.sed
sed -f osdef2.sed < $srcdir/osdef.h.in > osdef.h
rm osdef0.c osdef1.sed osdef2.sed

if test -f core*; then
  file core*
  echo "  Sorry, your sed is broken. Call the system administrator."
  echo "  Meanwhile, you may try to compile screen with an empty osdef.h file."
  echo "  But if your compiler needs to have all functions declared, you should"
  echo "  retry 'make' now and only remove offending lines from osdef.h later."
  exit 1
fi
if eval test "`diff osdef.h $srcdir/osdef.h.in | wc -l`" -eq 4; then
  echo "  Hmm, sed is very pessimistic about your system header files."
  echo "  But it did not dump core -- strange! Let's continue carefully..."
  echo "  If this fails, you may want to remove offending lines from osdef.h"
  echo "  or try with an empty osdef.h file, if your compiler can do without"
  echo "  function declarations."
fi
