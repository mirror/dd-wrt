#!/bin/sh

cat <<EOF
/* ISC license. */

#ifndef SYSDEPS_H
#define SYSDEPS_H

#undef SKALIBS_TARGET
#define SKALIBS_TARGET "$1"

EOF

while read k v ; do
  k=$(echo "${k%%:}" | tr '[:lower:]' '[:upper:]')
  if test ${k} != ${k##SIGNED} ; then
    echo "#undef SKALIBS_HASUN$k"
    echo "#undef SKALIBS_HAS$k"
    if test $v = yes ; then
      echo "#define SKALIBS_HAS$k"
    else
      echo "#define SKALIBS_HASUN$k"
    fi
  elif test ${k} != ${k##SIZEOF} ; then
    echo "#undef SKALIBS_$k"
    echo "#define SKALIBS_$k $v"
  else
    if test $v = yes ; then
      echo "#undef SKALIBS_HAS$k"
      echo "#define SKALIBS_HAS$k"
    elif test $v = no ; then
      echo "#undef SKALIBS_HAS$k"
    else
      echo "#undef SKALIBS_$k"
      if test $v != none ; then
        echo "#define SKALIBS_$k \"$v\""
      fi
    fi
  fi
  echo
done

echo '#endif'
