#!/bin/sh
# This script upgrades SQL databases from BDB 5.0 to early 6.1
# to late 6.1 and up by reindexing them.
#

for var in $@
do
    echo Recovering database $var
    db_recover -f -h ${var}-journal
    echo Reindexing database $var
    echo .quit | dbsql -cmd REINDEX  $var
done
