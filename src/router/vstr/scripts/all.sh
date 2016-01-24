#! /bin/sh -e

if false; then
 echo "Not reached."
elif [ -f ./configure ]; then
        s=./scripts/
elif [ -f ../configure ]; then
        s=../scripts/
else
  echo "Not in right place, dying."
  exit 1;
fi

$s/clean.sh full
$s/b/def-all.sh
$s/b/DOCS.sh

echo '------------------------------------------------------------------'
$s/tst_coverage_diff.sh
$s/diff_symbols.sh
echo '------------------------------------------------------------------'

$s/rpms-create-james.sh

$s/clean.sh
