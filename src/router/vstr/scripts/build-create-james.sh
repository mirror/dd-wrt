#! /bin/sh -e

if [ ! -r VERSION -o ! -r vstr.spec -o ! -r configure ]; then
  if [ -r configure ]; then
    ./scripts/b/DOCS.sh
  else
    echo "No VERSION, vstr.spec or configure file."
    exit 1
  fi
fi

v="`cat VERSION`"
s="`pwd`"
cd ../build/vstr

rm -rf vstr-${v}james
cp -a $s ./vstr-${v}james
cd ./vstr-${v}james
./scripts/clean.sh full

# Perf output...
rm -rf ./examples/perf*

# Examples droppings...
rm -rf ./examples/html
rm -rf ./examples/conf
rm -rf ./examples/tmp
rm -f ./examples/*_cntl

# Random crap...
rm -rf ./tmp
find . \
 \( -name "*.o" -o -name ".*[%~]" -o -name "*[%~]" -o -name "#*#" \) \
 -print0 | xargs --no-run-if-empty -0 rm -f

# Arch stuff...
rm -rf ./{arch}
find . -name .arch-ids -type d -print0 | xargs -0 rm -rf

# Make a tarball
cp $s/vstr.spec .

cd ..

tar -cf vstr-${v}james.tar vstr-${v}james
bzip2 -9f vstr-${v}james.tar

ls -hl `pwd`/vstr-${v}james.tar.bz2

