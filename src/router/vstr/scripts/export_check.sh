#! /bin/sh -e


TMP=`mktemp -t vstr-export.XXXXXXXXXX` || exit 1

# FIXME: No ABI information was used ... *sigh*.

objdump -T $@ | egrep "  Base " | egrep "vstr_[a-z]" | \
  perl -nle '/vstr_(.*)$/ && print $1' | sort > $TMP

objdump -T $@ | egrep "  Base " | egrep -v "vstr_[a-z]" | \
  perl -nle '/\s(\S+)$/ && print $1' | sort >> $TMP

diff -u $TMP ../include/vstr.exported_symbols

rm  -f $TMP
