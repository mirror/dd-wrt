#!/bin/sh

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
sourceHash="$(cat $(find . -name *.[ch] | grep -v -E '[/\\]?builddata.c$') | "$md5Command" | awk '{ print $1; }')"
hostName="$(hostname)"
buildDate="$(date +"%Y-%m-%d %H:%M:%S")"

tmpBuildDataTxt="$(mktemp -t olsrd.hash_source.XXXXXXXXXX)"
cat > "$tmpBuildDataTxt" << EOF
const char olsrd_version[] = "olsr.org - $version-git_$gitSha-hash_$sourceHash";
const char build_host[]    = "$hostName";
const char build_date[]    = "$buildDate";
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

