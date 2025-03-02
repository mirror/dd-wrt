#!/bin/sh
# Verify join/leave multicast groups for both IPv4 and IPv6
# a1 will be used to verify SSM and a2 to verify ASM
#
# Note: this test is really ugly and full of code duplcation

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

mgroup from a1 source fc00::1 group ff04:0:0:0:0:0:0:114
mgroup from a1 source 10.0.0.10 group 225.1.2.3

mgroup from a2 group ff2e::42
mgroup from a2 group 225.3.2.1
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show interfaces
echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
echo "-----------------------------------------------------------------------------------"
grep "0xe1010203 0x0a00000a" /proc/net/mcfilter
config_ssm=$?
grep "ff040000000000000000000000000114 fc000000000000000000000000000001" /proc/net/mcfilter6
config_ssm6=$?
ip maddr show dev a2 | grep 225.3.2.1
config_asm=$?
ip -6 maddr show dev a2 | grep ff2e::42
config_asm6=$?

# shellcheck disable=SC2166
if [ $config_ssm -eq 0 -a $config_ssm6 -eq 0 ]; then
    echo "Config SSM join OK"
    config_ssm=0
else
    echo "Config SSM join FAIL"
    config_ssm=1
fi

# shellcheck disable=SC2166
if [ $config_asm -eq 0 -a $config_asm6 -eq 0 ]; then
    echo "Config ASM join OK"
    config_asm=0
else
    echo "Config ASM join FAIL"
    config_asm=1
fi

# shellcheck disable=SC2166
if [ $config_ssm -eq 0 -a $config_asm -eq 0 ]; then
    echo "Config join OK"
    config=0
else
    echo "Config join FAIL"
    config=1
fi

#################################################################### JOIN GROUPS
print "Phase 2: Join groups (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" join a1 10.0.0.11 225.1.1.1
../src/smcroutectl -u "/tmp/$NM/sock" join a2 225.2.2.2

../src/smcroutectl -u "/tmp/$NM/sock" join a1 fc00::2 ff04::111
../src/smcroutectl -u "/tmp/$NM/sock" join a2 ff2e::22

echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show interfaces
echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
echo "-----------------------------------------------------------------------------------"
grep "0xe1010101 0x0a00000b" /proc/net/mcfilter
dynamic_ssm=$?
grep "ff040000000000000000000000000111 fc000000000000000000000000000002" /proc/net/mcfilter6
dynamic_ssm6=$?
ip maddr show dev a2 | grep 225.2.2.2
dynamic_asm=$?
ip -6 maddr show dev a2 | grep ff2e::22
dynamic_asm6=$?

# shellcheck disable=SC2166
if [ $dynamic_ssm -eq 0 -a $dynamic_ssm6 -eq 0 ]; then
    echo "Dynamic SSM join OK"
    dynamic_ssm=0
else
    echo "Dynamic SSM join FAIL"
    dynamic_ssm=1
fi

# shellcheck disable=SC2166
if [ $dynamic_asm -eq 0 -a $dynamic_asm6 -eq 0 ]; then
    echo "Dynamic ASM join OK"
    dynamic_asm=0
else
    echo "Dynamic ASM join FAIL"
    dynamic_asm=1
fi

# shellcheck disable=SC2166
if [ $dynamic_ssm -eq 0 -a $dynamic_asm -eq 0 ]; then
    echo "Dynamic join OK"
    dynamic=0
else
    echo "Dynamic join FAIL"
    dynamic=1
fi

################################################################### LEAVE GROUPS
print "Phase 3: Leave groups (IPC)"
../src/smcroutectl -u "/tmp/$NM/sock" leave a1 10.0.0.10 225.1.2.3
../src/smcroutectl -u "/tmp/$NM/sock" leave a2 225.3.2.1

../src/smcroutectl -u "/tmp/$NM/sock" leave a1 fc00::1 ff04::114
../src/smcroutectl -u "/tmp/$NM/sock" leave a2 ff2e::42

echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show interfaces
echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
echo "-----------------------------------------------------------------------------------"
grep "0xe1010203 0x0a00000a" /proc/net/mcfilter
leave_ssm=$?
grep "ff040000000000000000000000000114 fc000000000000000000000000000001" /proc/net/mcfilter6
leave_ssm6=$?
ip maddr show dev a2 | grep 225.3.2.1
leave_asm=$?
ip -6 maddr show dev a2 | grep ff2e::42
leave_asm6=$?

# shellcheck disable=SC2166
if [ $leave_ssm -eq 1 -a $leave_ssm6 -eq 1 ]; then
    echo "Dynamic SSM leave OK"
    leave_ssm=0
else
    echo "Dynamic SSM leave FAIL"
    leave_ssm=1
fi

# shellcheck disable=SC2166
if [ $leave_asm -eq 1 -a $leave_asm6 -eq 1 ]; then
    echo "Dynamic ASM leave OK"
    leave_asm=0
else
    echo "Dynamic ASM leave FAIL"
    leave_asm=1
fi

# shellcheck disable=SC2166
if [ $leave_ssm -eq 0 -a $leave_asm -eq 0 ]; then
    echo "Dynamic leave OK"
    leave=0
else
    echo "Dynamic leave FAIL"
    leave=1
fi

########################################################################### DONE
# shellcheck disable=SC2166
[ $config -eq 0 -a $dynamic -eq 0 -a $leave -eq 0 ] && OK
FAIL
