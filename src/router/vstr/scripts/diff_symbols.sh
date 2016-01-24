#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./scripts ]; then
       sc=./scripts
elif [ -r ../scripts ]; then
       sc=../scripts
else
 echo "No scripts dir"
 exit 1;
fi

(
 $sc/list_constants_doc.sh;
 $sc/list_constants_src.pl

 $sc/list_functions_doc.sh;
 $sc/list_functions_src.pl ) | \
   sort | uniq -u
