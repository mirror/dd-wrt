#!/bin/sh

sha="`git log -1 --pretty=%h 2> /dev/null`"
if [ -z "$sha" ]; then
  sha="0000000"
fi
echo -n "-git_$sha"

md5cmd=md5sum
os=$(uname)
if [ "xDarwin" = "x$os" ] ; then
  md5cmd=md5
fi
echo -n "-hash_"
cat `find . -name *.[ch] | grep -v -E '[/\\]?builddata.c$'`| $md5cmd | awk '{ print $1; }'
