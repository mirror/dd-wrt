#!/bin/bash
source tests/common.bash

pat=".*DIR[ \t]\+.*PATH.*[ \t]\+.*/tmp$"
source $tcasedir/util-open-flags.bash "$lsof" "$report" "$tcasedir" "$dialect" "$pat" /tmp path
