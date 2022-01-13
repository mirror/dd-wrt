#!/bin/bash

pat=".*REG[ \t]\+.*TMPF.*/tmp/.*$"
source $3/util-open-flags.bash "$@" "$pat" /tmp tmpf rdwr
