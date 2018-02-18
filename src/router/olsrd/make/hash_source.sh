#!/bin/sh

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

if [ $# -ne 3 ]; then
	echo "ERROR: Supply filename, version and verbosity"
	echo "       Example: $0 builddata.txt pre-0.6.7 1"
	exit 1
fi

buildDataTxt="$1"
version="$2"
verbose="$3"

md5Command="md5sum"
osName="$(uname)"
if [ "x$osName" = "xDarwin" ] ; then
  md5Command="md5"
elif [ "x$osName" = "xOpenBSD" ] ; then
  md5Command="md5"
fi


gitSha="$(git log -1 --pretty=%h 2> /dev/null)"
if [ -z "$gitSha" ]; then
  gitSha="0000000"
fi

gitShaFull="$(git rev-list -1 HEAD 2> /dev/null)"
if [ -z "$gitShaFull" ]; then
  gitShaFull="0000000000000000000000000000000000000000"
fi

gitDescriptor="$(git describe --dirty --always 2> /dev/null)"

sourceHash="$(cat $(find . -name *.[ch] | grep -v -E '[/\\]?builddata.c$') | "$md5Command" | awk '{ print $1; }')"
hostName="$(hostname)"
buildDate="$(date +"%Y-%m-%d %H:%M:%S")"

tmpBuildDataTxt="$(mktemp -t olsrd.hash_source.XXXXXXXXXX)"
cat > "$tmpBuildDataTxt" << EOF
const char olsrd_version[]   = "olsr.org - $version-git_$gitSha-hash_$sourceHash";

const char build_date[]      = "$buildDate";
const char build_host[]      = "$hostName";
const char git_descriptor[]  = "$gitDescriptor";
const char git_sha[]         = "$gitShaFull";
const char release_version[] = "$version";
const char source_hash[]     = "$sourceHash";
EOF


if [ ! -e "$buildDataTxt" ]; then
  echo "[CREATE] $buildDataTxt"
  if [ "$verbose" = "0" ]; then
    cp -p "$tmpBuildDataTxt" "$buildDataTxt"
  else
    cp -p -v "$tmpBuildDataTxt" "$buildDataTxt"
  fi
elif [ -n "$(diff -I "^const char build_date\[\].*\$" "$tmpBuildDataTxt" "$buildDataTxt" | sed 's/"/\\"/g')" ]; then
  echo "[UPDATE] $buildDataTxt"
  if [ "$verbose" = "0" ]; then
    cp -p "$tmpBuildDataTxt" "$buildDataTxt"
  else
    cp -p -v "$tmpBuildDataTxt" "$buildDataTxt"
  fi
fi
rm -f "$tmpBuildDataTxt"

