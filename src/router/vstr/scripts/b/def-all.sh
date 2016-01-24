#! /bin/sh -e

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
mkdir dbg opt j cov || true

cd dbg
../scripts/b/def-debug.sh
cd ..

cd opt
../scripts/b/def-opt.sh
cd ..

cd j
../scripts/b/def-tst.sh
cd ..

cd cov
../scripts/coverage.sh
cd ..
