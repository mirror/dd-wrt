#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# Copy firmware files based on WHENCE list
#

verbose=:
prune=no

while test $# -gt 0; do
    case $1 in
        -v | --verbose)
            verbose=echo
            shift
            ;;

        -P | --prune)
            prune=yes
            shift
            ;;

        *)
            if test "x$destdir" != "x"; then
                echo "ERROR: unknown command-line options: $@"
                exit 1
            fi

            destdir="$1"
            shift
            ;;
    esac
done

grep '^File:' WHENCE | sed -e's/^File: *//g' -e's/"//g' | while read f; do
    test -f "$f" || continue
    $verbose "copying file $f"
    mkdir -p $destdir/$(dirname "$f")
    cp -d "$f" $destdir/"$f"
done

grep -E '^Link:' WHENCE | sed -e's/^Link: *//g' -e's/-> //g' | while read f d; do
    if test -L "$f"; then
        test -f "$destdir/$f" && continue
        $verbose "copying link $f"
        mkdir -p $destdir/$(dirname "$f")
        cp -d "$f" $destdir/"$f"

        if test "x$d" != "x"; then
            target=`readlink "$f"`

            if test "x$target" != "x$d"; then
                $verbose "WARNING: inconsistent symlink target: $target != $d"
            else
                if test "x$prune" != "xyes"; then
                    $verbose "WARNING: unneeded symlink detected: $f"
                else
                    $verbose "WARNING: pruning unneeded symlink $f"
                    rm -f "$f"
                fi
            fi
        else
            $verbose "WARNING: missing target for symlink $f"
        fi
    else
        $verbose "creating link $f -> $d"
        mkdir -p $destdir/$(dirname "$f")
        ln -sf "$d" "$destdir/$f"
    fi
done

exit 0

# vim: et sw=4 sts=4 ts=4
