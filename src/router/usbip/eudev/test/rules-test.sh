#!/bin/sh
# Call the udev rule syntax checker on all rules that we ship
#
# (C) 2010 Canonical Ltd.
# Author: Martin Pitt <martin.pitt@ubuntu.com>

[ -n "$builddir" ] || builddir=`dirname $0`/..

# skip if we don't have python
type python >/dev/null 2>&1 || {
        echo "$0: No python installed, skipping udev rule syntax check"
        exit 0
}

$builddir/test/rule-syntax-check.py `find $builddir/rules -name '*.rules'`
