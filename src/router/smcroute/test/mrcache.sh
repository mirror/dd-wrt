#!/bin/sh
# Verifies both IPv4 and IPv6 (S,G) add, including remove route via IPC bites in kernel.
# Twist: uses only one interface, inteded to mimic Debian tests.
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

debug()
{
    echo "/proc/net/ip_mr_cache -------------------------------------------------------DEBUG-"
    cat /proc/net/ip_mr_cache
    echo "ip mroute -------------------------------------------------------------------DEBUG-"
    ip mroute
    echo "smcroutectl -----------------------------------------------------------------DEBUG-"
    ../src/smcroutectl -pd -u "/tmp/$NM/sock"
    echo "-----------------------------------------------------------------------------DEBUG/"
}

check_add()
{
    s=$1
    g=$2
    input=$3
    output=$4

    print "Adding IPC route ($s,$g) inbound $input outbound $output ..."
    ../src/smcroutectl -u "/tmp/$NM/sock" add "$input" "$s" "$g" "$output"
 #   sleep 1

    print "Verifying kernel route ($s,$g) Iif: $input Oif: $output ..."
#    debug
    ip mroute     > "/tmp/$NM/routes"
    ip -6 mroute >> "/tmp/$NM/routes"
    sg=$(awk "/$s,$g/{print \$1}" "/tmp/$NM/routes")
    iif=$(awk "/$s,$g/{print \$3}" "/tmp/$NM/routes")
    oif=$(awk "/$s,$g/{print \$5}" "/tmp/$NM/routes")
    state=$(awk "/$s,$g/{print \$7}" "/tmp/$NM/routes")
    CHECK "$sg" = "($s,$g)"
    CHECK "$iif" = "$input"
    CHECK "$oif" = "$output"
    CHECK "$state" = "resolved"
}

check_del()
{
    s=$1
    g=$2
    input=$3
    output=$4

    print "Removing IPC route ($s,$g) inbound $input outbound $output ..."
    ../src/smcroutectl -u "/tmp/$NM/sock" del "$input" "$s" "$g"
#    sleep 1

    print "Verifying kernel route for ($s,$g) has been removed ..."
    #debug
    ip mroute > "/tmp/$NM/routes2"
    sg=$(awk "/$s,$g/{print \$1}" "/tmp/$NM/routes2")
    CHECK -z "$sg"
}

test_one()
{
    check_add $1 $2 $3 $4
    check_del $1 $2 $3 $4
}

print "Creating world ..."
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add fc01::1/64  dev a1
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# empty
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

test_one 10.0.0.1 224.0.1.20 a1 a1
test_one fc01::1  ff01::114  a1 a1
OK
