#!/bin/sh

# only checking for a sample in each range
check_output()
{
    print "Verifying ..."
    if [ -n "$1" ]; then
	ip maddr show dev a2
	if ! ip maddr show dev a2 | grep -q "$1"; then
	    FAIL "Cannot find (* $1)"
	fi
    fi

    if [ -n "$2" ]; then
	cat /proc/net/mcfilter
	if ! grep -q "$2" /proc/net/mcfilter; then
	    FAIL "Cannot find ($2)"
	fi
    fi
}

check_output6()
{
    print "Verifying ..."
    if [ -n "$1" ]; then
	ip -6 maddr show dev a2
	if ! ip -6 maddr show dev a2 | grep -q "$1"; then
	    FAIL "Cannot find (* $1)"
	fi
    fi

    if [ -n "$2" ]; then
	cat /proc/net/mcfilter6
	if ! grep -q "$2" /proc/net/mcfilter6; then
	    FAIL "Cannot find ($2)"
	fi
    fi
}

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

################################################################## STATIC GROUPS
print "Phase 1: Join groups (.conf)"
cat <<EOF > "/tmp/$NM/conf"
# ASM + SSM join/leave multicast groups
phyint a1 enable
phyint a2 enable

mgroup from a1 source 10.0.0.10 group 225.1.2.40/24
mgroup from a2 group 225.3.2.250/24
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

check_output "225.3.2.249" "0xe101022a 0x0a00000a"

################################################################### LEAVE GROUPS
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 10.0.0.10 225.1.2.40/24
../src/smcroutectl -u "/tmp/$NM/sock" leave a2 225.3.2.250/24
cat /proc/net/mcfilter
ip maddr show dev a2

#################################################################### JOIN GROUPS
print "Phase 2: Join groups (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 10.0.0.10 225.1.1.1/30
../src/smcroutectl -u "/tmp/$NM/sock" join a2 225.0.0.1/30
check_output "225.0.0.2" "0xe1010102 0x0a00000a"

################################################################### LEAVE GROUPS
print "Debug 1 ..."
../src/smcroutectl -u "/tmp/$NM/sock" show group
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 10.0.0.10 225.1.1.1/30
../src/smcroutectl -u "/tmp/$NM/sock" leave a2 225.0.0.1/30

################################################################### JOIN SOURCES
print "Debug 2 ..."
../src/smcroutectl -u "/tmp/$NM/sock" show group
print "Phase 3: Join group from multiple sources (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 10.0.0.1/26 225.1.2.3
check_output "" "0xe1010203 0x0a00003e"

################################################################### LEAVE GROUPS
print "Debug 3 ..."
../src/smcroutectl -u "/tmp/$NM/sock" show group
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 10.0.0.1/26 225.1.2.3

######################################################## JOIN SOURCES AND GROUPS
print "Phase 4: Join multiple groups from multiple sources (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 10.0.0.1/30 225.1.2.3/30
check_output "" "0xe1010201 0x0a000003"

############################################################## JOIN IPv6 SOURCES
print "Phase 5: Join IPv6 group from multiple sources (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 2001:1::1/122 ff2e::42
check_output6 "" "ff2e0000000000000000000000000042 2001000100000000000000000000001c"

../src/smcroutectl -u "/tmp/$NM/sock" show group
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 2001:1::1/122 ff2e::42

################################################### JOIN IPv6 SOURCES AND GROUPS
print "Phase 6: Join multiple IPv6 groups from multiple sources (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 2001:1::1/126 ff2e::42/126
check_output6 "" "ff2e0000000000000000000000000041 20010001000000000000000000000002"

../src/smcroutectl -u "/tmp/$NM/sock" show group
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 2001:1::1/122 ff2e::42/126

########################################################################### DONE
OK
