#!/bin/sh -e
status=0
# Dump the current ABI
LC_ALL=C.UTF-8 PATH=/snap/bin:$PATH abi-dumper ./build/libyang.so -o ./build/libyang.dump -lver "$(PKG_CONFIG_PATH=./build pkg-config --modversion libyang)" -public-headers ./src -public-headers ./build/src
# Compare the current ABI with previous ABI
abi-compliance-checker -l libyang.so -old ./libyang.dump -new ./build/libyang.dump -s || status=$?
# Generate and dump text output
w3m -dump -O ascii -T text/html "$(find "compat_reports/${SONAME}" -name '*.html')"
# Dump the new libyang ABI dump if it differs
if [ "$status" -ne 0 ]; then
    echo "-- cut here --"
    cat ./build/libyang.dump
    echo "-- cut here --"
fi
exit $status
