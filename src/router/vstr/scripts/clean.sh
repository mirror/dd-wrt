#! /bin/sh

if false; then
 echo "Not reached."
elif [ -f ./configure ]; then
        r=./
elif [ -f ../configure ]; then
        r=../
else
  echo "Not in right place, dying."
  exit 1;
fi

cd $r
make maintainer-clean 2>&1 > /dev/null

rm -rf tst/*/.libs

# Remove arch stuff
rm -rf ,,*

rm -rf autocheck?.log autom4te.cache
rm -f  gmon.out examples/gmon.out

if [ "x$1" != "x" ]; then
  rm -rf dbg opt j cov
fi
