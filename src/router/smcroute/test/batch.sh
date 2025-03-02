#!/bin/sh
# Verify batch mode, we just want to see the daemon accepting the
# batched commands.

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic

# IP world ...
ip addr add 10.0.0.1/24  dev a1
ip addr add 20.0.0.1/24  dev a2
ip addr add 2001:1::1/64 dev a1
ip addr add 2001:2::1/64 dev a2
ip -br a

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

print "Joining groups (batch)"
../src/smcroutectl -u "/tmp/$NM/sock" -b <<-EOF
	join a1 10.0.0.11 225.1.1.1
	join a2 225.2.2.2
	join a1 fc00::2 ff04::111
	join a2 ff2e::22
	EOF

output=$(../src/smcroutectl -pu "/tmp/$NM/sock" show groups)
echo "$output"
[ "$(echo "$output" | grep 225.1.1.1 | sed 's/[[:space:]]*//g')" = "(10.0.0.11,225.1.1.1)a1" ] || FAIL "225.1.1.1"
[ "$(echo "$output" | grep 225.2.2.2 | sed 's/[[:space:]]*//g')" = "(*,225.2.2.2)a2" ]         || FAIL "225.2.2.2"
[ "$(echo "$output" | grep ff04::111 | sed 's/[[:space:]]*//g')" = "(fc00::2,ff04::111)a1" ]   || FAIL "ff04::111"
[ "$(echo "$output" | grep ff2e::22  | sed 's/[[:space:]]*//g')" = "(*,ff2e::22)a2" ]          || FAIL "ff2e::22"

OK
