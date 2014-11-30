#!/bin/bash

if [ ! $# -eq 1 ]; then
  echo "ERROR: specify 1 argument: the mingw gcc"
  exit 1
fi

GCC="$1"
if [ ! -x "$GCC" ]; then
  GCC="$(which "$GCC" | head -1)"
  if [ ! -x "$GCC" ]; then
    echo "ERROR: the mingw gcc ($GCC) is not executable"
    exit 1
  fi
fi

versions=( $("$GCC" -dumpversion | sed -r -e "s/\./ /g") )
while [ ${#versions[*]} -lt 3 ]; do \
  versions[${#versions[*]}]="0"; \
done
if [ ${#versions[*]} -ne 3 ]; then
  echo "WARNING: could not detect the mingw gcc version, setting to 0.0.0"
  versions=( 0 0 0 )
fi

# Ubuntu 13.10 and Debian Wheezy both report mingw 4.6.
# - Debian Wheezy needs the old setup for 64 bits and the new setup for 32 bits.
# - Ubuntu 13.10 needs the new setup for both 32 bits and 64 bits.
# --> We decrement the version to 4.5 for Debian Wheezy 64 bits
if [ "${versions[0]}" == "4" ] && \
   [ "${versions[1]}" == "6" ] && \
   [ "${versions[2]}" == "0" ]; then
  detectDist="$(uname -a | grep -i debian)"
  detectArch="$("$GCC" -dumpmachine | grep -i x86_64)"
  if [ -n "$detectDist" ] && \
     [ -n "$detectArch" ]; then
    versions[1]=5
  fi
fi

version=$(( versions[0]*10000 + versions[1]*100 + versions[2] ))
echo "$version"
