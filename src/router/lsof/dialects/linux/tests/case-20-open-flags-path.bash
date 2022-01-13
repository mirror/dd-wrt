#!/bin/sh

pat=".*DIR[ \t]\+.*PATH.*[ \t]\+.*/tmp$"
source $3/util-open-flags.bash "$@" "$pat" /tmp path
