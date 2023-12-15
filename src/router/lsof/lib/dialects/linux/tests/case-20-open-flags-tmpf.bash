#!/bin/bash
source tests/common.bash

pat=".*REG[ \t]\+.*TMPF.*/tmp/.*$"
source $tcasedir/util-open-flags.bash "$lsof" "$report" "$tcasedir" "$dialect" "$pat" /tmp tmpf rdwr
