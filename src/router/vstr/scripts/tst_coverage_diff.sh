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

( $sc/tst_coverage_list_constants.sh | egrep -v "\(\)$"
  $sc/list_constants_src.pl | egrep -v "\(\)$"
) | sort | uniq -u

( $sc/tst_coverage_list_constants.sh | egrep "\(\)$"
  $sc/list_functions_src.pl | egrep "^VSTR_"
  $sc/list_constants_src.pl | egrep "\(\)$"
) | sort | uniq -u

( $sc/tst_coverage_list_functions.zsh;
  $sc/list_functions_src.pl | egrep "^vstr_";
) | sort | uniq -u

