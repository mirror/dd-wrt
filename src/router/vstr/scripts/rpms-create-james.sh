#! /bin/bash -e

pkg=vstr

if [ ! -r VERSION -o ! -r $pkg.spec -o ! -r configure ]; then
  if [ -r configure ]; then
#    ./scripts/b/DOCS.sh
    ./scripts/b/def-debug.sh
  else
    echo "No VERSION, $pkg.spec or configure file." &>2
    exit 1
  fi
fi

v="`cat VERSION`"
s="`pwd`"
cd ../build/$pkg

rm -rf $pkg-$v
cp -a $s ./$pkg-$v
cd ./$pkg-$v

./scripts/clean.sh full

rm -rf tmp

# Backup files...
find . \
 \( -name "*.o" -o -name ".*[%~]" -o -name "*[%~]" -o -name "#*#" \) \
 -print0 | xargs --no-run-if-empty -0 rm -f

# Arch stuff...
rm -rf ./{arch}
find . -name .arch-ids -type d -print0 | xargs -0 rm -rf

# Create tarballs/RPMS
cp $s/$pkg.spec .

cd ..

chk=1
rel=1
if [ "x$1" = "xnochk" ]; then
echo Not doing checking.
chk=0
shift
else
echo Doing checking.
args="$args  --define \"chk 1\""
fi

if [ "x$1" = "xrel" ]; then
shift
echo Using custom release $1.
rel=$1
shift
else
echo Using normal release of 1.
fi

perl -i -pe "s/^Release: 1/Release: $rel/" $pkg-$v/$pkg.spec

tar -cf   $pkg-$v.tar $pkg-$v
bzip2 -9f $pkg-$v.tar

tar -cf   $pkg-$v.tar $pkg-$v
gzip -9f  $pkg-$v.tar

sudo rpmbuild -ta --define "chk $chk" $pkg-$v.tar.gz

echo "/usr/src/redhat/RPMS/*/$pkg*-$v-$rel*"
echo "/usr/src/redhat/SRPMS/$pkg*-$v-$rel*"

ls -ahslF /usr/src/redhat/RPMS/*/$pkg*-$v-$rel*
ls -ahslF /usr/src/redhat/SRPMS/$pkg*-$v-$rel*

