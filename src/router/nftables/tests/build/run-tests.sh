#!/bin/bash

cd $(dirname $0)

log_file="$(pwd)/tests.log"
dir=../..
argument=( --without-cli --with-cli=linenoise --with-cli=editline --enable-debug --with-mini-gmp
	   --enable-man-doc --with-xtables --with-json --with-unitdir --with-unitdir=/lib/systemd/system)
ok=0
failed=0

[ -f "$log_file" ] && rm -rf "$log_file"

tmpdir=$(mktemp -d)
if [ ! -w "$tmpdir" ] ; then
        echo "Failed to create tmp file" >&2
        exit 0
fi

git clone "$dir" "$tmpdir" &>>"$log_file"
cd "$tmpdir" || exit

# do not leak data from a calling 'make check' run into the new build otherwise
# this will defeat the test suite invocation prevention for 'make distcheck'
unset MAKEFLAGS

if ! autoreconf -fi &>>"$log_file" ; then
	echo "Something went wrong. Check the log '${log_file}' for details."
	exit 1
fi

if ! ./configure &>>"$log_file" ; then
	echo "Something went wrong. Check the log '${log_file}' for details."
	exit 1
fi

echo  "Testing build with distcheck"
if ! make distcheck &>>"$log_file" ; then
	echo "Something went wrong. Check the log '${log_file}' for details."
	exit 1
fi

echo -en "\033[1A\033[K"
echo "Build works. Now, testing compile options"

for var in "${argument[@]}" ; do
	echo "[EXECUTING] Testing compile option $var"
	./configure "$var" &>>"$log_file"
	make -j 8 &>>"$log_file"
	rt=$?
	echo -en "\033[1A\033[K" # clean the [EXECUTING] foobar line

	if [ $rt -eq 0 ] ; then
		echo "[OK] Compile option $var works."
		((ok++))
	else
		echo "[FAILED] Compile option $var does not work. Check log for details."
		((failed++))
	fi
done

rm -rf "$tmpdir"

echo "results: [OK] $ok [FAILED] $failed [TOTAL] $((ok+failed))"
[ "$failed" -eq 0 ]
