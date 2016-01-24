#! /bin/sh

if false; then
 echo "Not reached."
elif [ -f ./configure ]; then
        c=./configure
elif [ -f ../configure ]; then
        c=../configure
else
  echo "Not in right place, dying."
  exit 1;
fi

export CFLAGS="-g -fprofile-arcs -ftest-coverage -DUSE_SYSCALL_MAIN $CFLAGS -O0"
$c \
  --enable-tst-noinline \
  --enable-tst-nosyscall-asm \
  --enable-tst-noassert-loop \
  --enable-tst-noattr-alias \
  --enable-tst-noopt \
  --enable-wrap-memcpy \
  --enable-wrap-memcmp \
  --enable-wrap-memchr \
  --enable-wrap-memrchr \
  --enable-wrap-memset \
  --enable-wrap-memmove \
  --enable-static \
  --disable-shared \
    $@ && make clean && make check
