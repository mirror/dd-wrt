#!/bin/sh
md5cmd=md5sum
os=$(uname)
if [ "xDarwin" = "x$os" ] ; then
  md5cmd=md5
fi
cat `find . -name *.[ch]`| $md5cmd
