#!/bin/sh
# Verify stop-filter to SSM transition by adding *,G/LEN routes
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable
phyint a2 enable

mroute from a1 source 10.0.0.1 group 225.1.2.3 to a2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &

# Start emitters on a1 for 225.1.2.1 .. 225.1.2.10
emitter a1 10
show_mroute

# Sanity check, the route from the conf file should work
collect a2 -c3 'dst 225.1.2.3'
sleep 3
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee "/tmp/$NM/result" | wc -l)
# shellcheck disable=SC2086
[ $lines1 -lt 3 ] && FAIL

# Add (*,G) routes for 225.1.2.1 .. 225.1.2.7, overlapping with previous 225.1.2.3
print "Adding *,225.1.2.3/29 ASM routes ..."
../src/smcroutectl -u "/tmp/$NM/sock" add a1 225.1.2.3/29 a2
show_mroute

# Check for a sample of the added routes
collect a2 -c6 'dst 225.1.2.6 or dst 225.1.2.3'
sleep 3

print "Analyzing ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee    "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.6 | tee -a "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
# shellcheck disable=SC2086 disable=SC2166
[ $lines1 -lt 3 -o $lines2 -lt 3 ] && FAIL

# When removing (*,G) routes, the (S,G) route from the conf file should remain
print "Removing *,225.1.2.3/29 ASM routes ..."
../src/smcroutectl -u "/tmp/$NM/sock" del a1 225.1.2.3/29
show_mroute

# Check for same sample, now we should no longer see 225.1.2.6, only 225.1.2.3
collect a2 -c3 'dst 225.1.2.6 or dst 225.1.2.3'
sleep 3

print "Analyzing ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee    "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.6 | tee -a "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines1 for group 225.1.2.3 from R1, expected > 1"
echo " => $lines2 for group 225.1.2.6 from R2, expected = 0"
# shellcheck disable=SC2086 disable=SC2166
[ $lines1 -gt 1 -a $lines2 -eq 0 ] && OK
FAIL
