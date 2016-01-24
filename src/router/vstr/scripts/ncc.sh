#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./configure ]; then
       src=./src
elif [ -r ../configure ]; then
       src=../src
else
 echo "No configure script"
 exit 1;
fi

cd $src
cd ..
./configure --enable-debug --enable-tst-noattr-visibility --enable-tst-noattr-alias --enable-wrap-memcpy--enable-wrap-memcmp --enable-wrap-memchr --enable-wrap-memrchr --enable-wrap-memset --enable-wrap-memmove
cd $src

for a in *.c; do
  ncc -I../include $a -ncoo;
done
cat *.nccout > Code.map
nccnav

