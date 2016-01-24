#! /bin/sh

if false; then
 echo "Not reached."
# elif [ -f ./configure ]; then
        s=./scripts
        s=./Documentation
	doln=false
elif [ -f ../configure ]; then
        s=../scripts
        d=../Documentation
	doln=true
else
  echo "Not in right place, goto a seperate build directory."
  exit 1;
fi

if [ "x$1" = "x" ]; then
  echo "Not got arg."
  exit 1;
fi

gendesc $d/coverage_descriptions.txt -o descriptions

cd src
lcov --capture --directory . --output-file ../lib-$1.info --test-name lib-$1
cd ..

cd examples
lcov --capture --directory . --output-file ../examples-$1.info --test-name examples-$1
cd ..

mkdir output || true
genhtml lib*.info --output-directory output/lib --title "Vstr coverage" --show-details --description-file descriptions
genhtml examples*.info --output-directory output/examples --title "Vstr EXAMPLES coverage" --show-details --description-file descriptions
genhtml *.info --output-directory output/all --title "Vstr ALL coverage" --show-details --description-file descriptions
echo Point your browser at file:`pwd`/output/lib/index.html
