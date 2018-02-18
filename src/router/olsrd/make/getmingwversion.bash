#!/bin/bash

# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#

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
