#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./configure ]; then
       tst=./tst
elif [ -r ../configure ]; then
       tst=../tst
else
 echo "No configure script"
 exit 1;
fi

# Can be used more than once, so need to sort/uniq
find $tst -name \*.c -print0 | \
  xargs -0 perl -ne 'while (/(?:^|\W)(VSTR_[[:alnum:]][_[:alnum:]]+) *(\()?/g)
                     { print $1 . (defined($2) ? "()" : "") . "\n" } ' | \
  egrep -v "^VSTR_AUTOCONF_" | \
  egrep -v "^VSTR_CNTL_(OPT|CONF|BASE)_[GS]ET_$" | \
  egrep -v "^VSTR_FLAG[0-9][0-9]()" | \
  sort | uniq

