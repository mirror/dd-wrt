#! /bin/sh -e

tst_klibc=false
tst_quick=false

if [ "x$1" != "x" ]; then
  tst_quick=true
fi


if false; then
 echo "Not reached."
elif [ -f ../configure ]; then
        s=../scripts
else
  echo "Not in right place, goto a seperate build directory."
  exit 1;
fi


# Remove ccache... 
if [ "x$CC" = "x" ]; then
for i in `perl -e 'print join "\n", split ":", $ENV{PATH}'`; do
  if [ "x$CC" = "x" ]; then
    if [ -f $i/gcc ]; then
      if ! readlink $i/gcc | egrep -q ccache; then
        export CC=$i/gcc
      fi
    fi
  fi
done
fi


rm -f *.info

function del()
{
    rm -rf Documentation/ include/ src/ tst/ examples/ \
    Makefile config.log config.status libtool vstr.pc vstr.spec
}

function linkup()
{
  for dir in src examples include; do
  cd $dir

  lndir ../$s/../$dir

## 4.0 does everything different, again.
### Newer GCCs put them in the $srcdir
 ## if [ ! -f ex_httpd-ex_httpd.da -a -f ex_httpd -a ! -f vstr.da ]; then
 ##   for i in .libs/*.da; do
 ##     ln -f $i; rm -f $i
 ##   done
 ## fi

  cd ..
  done
}

function cov()
{
  type=$1; shift
  del
  $s/b/COVERAGE.sh $@
  linkup
  $s/lcov.sh $type
}

cov dbg --enable-debug --enable-examples
cov opt --enable-examples

if $tst_quick; then
 echo Doing quick coverage test...
else
cov noopt --enable-tst-noopt --enable-examples
cov ansi --enable-noposix-host \
         --enable-tst-noattr-visibility \
         --enable-tst-noattr-alias \
         --enable-tst-nosyscall-asm
CFLAGS="-DUSE_RESTRICTED_HEADERS=1" cov small-libc --enable-examples
fi


# This doesn't work, coverage with either klibc or dietlibc are NOGO...
# However you can compile programs linked with either and Vstr ... just not with
# coverage.
if $tst_klibc; then
  export LDFLAGS="-nodefaultlibs /usr/lib/klibc/crt0.o `gcc --print-libgcc` /usr/lib/klibc/libc.a"
  export CFLAGS="-nostdinc -iwithprefix include -I/usr/include/klibc -I/usr/include/klibc/kernel"
  cov klibc --enable-examples
fi

