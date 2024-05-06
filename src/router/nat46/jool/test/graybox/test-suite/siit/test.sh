#!/bin/sh


# Arguments:
# $1: List of the names of the test groups you want to run, separated by any
#     character.
#     Example: "udp64, tcp46, icmpe64"
#     If this argument is unspecified, the script will run all the tests.
#     The current groups are:
#     - udp64: IPv6->IPv4 UDP tests (documented in ../../rfc/pktgen.md)
#     - udp46: IPv4->IPv6 UDP tests (documented in ../../rfc/pktgen.md)
#     - tcp64: IPv6->IPv4 TCP tests (documented in ../../rfc/pktgen.md)
#     - icmpi64: IPv6->IPv4 ICMP ping tests (documented in ../../rfc/pktgen.md)
#     - icmpi46: IPv4->IPv6 ICMP ping tests (documented in ../../rfc/pktgen.md)
#     - icmpe64: IPv6->IPv4 ICMP error tests (documented in ../../rfc/pktgen.md)
#     - icmpe46: IPv4->IPv6 ICMP error tests (documented in ../../rfc/pktgen.md)
#     - manual: random tests (documented in ../../rfc/manual.md)
#     - rfc7915: RFC 7915 compliance tests (documented in ../../rfc/7915.md)
#     (Feel free to add new groups if you want.)


GRAYBOX=`dirname $0`/../../usr/graybox

# When Linux creates an ICMPv4 error on behalf of Jool, it writes 'c0' on the
# outer TOS field for me. This seems to mean "Network Control" messages
# according to DSCP, which is probably fair. Since TOS 0 would also be correct,
# we'll just accept whatever.
TOS=1
# The translated IPv4 identification is always random, so it should be always
# ignored during validation. Unfortunately, of course, the header checksum is
# also affected.
IDENTIFICATION=4,5,10,11
INNER_IDENTIFICATION=32,33,38,39


# IPv6 to IPv4 test boilerplate: pktgen tests.
# $1: Test packet
# $2: Expected packet
# $3: Exceptions (optional)
test64_auto() {
	ip netns exec client4ns $GRAYBOX expect add `dirname $0`/pktgen/receiver/$2.pkt $3
	ip netns exec client6ns $GRAYBOX send `dirname $0`/pktgen/sender/$1.pkt
	sleep 0.1
	ip netns exec client4ns $GRAYBOX expect flush
}

test46_auto() {
	ip netns exec client6ns $GRAYBOX expect add `dirname $0`/pktgen/receiver/$2.pkt $3
	ip netns exec client4ns $GRAYBOX send `dirname $0`/pktgen/sender/$1.pkt
	sleep 0.1
	ip netns exec client6ns $GRAYBOX expect flush
}

test_11() {
	ip netns exec $1 $GRAYBOX expect add `dirname $0`/$3/$5.pkt $6
	ip netns exec $2 $GRAYBOX send `dirname $0`/$3/$4.pkt
	sleep 0.1
	ip netns exec $1 $GRAYBOX expect flush
}

test_12() {
	ip netns exec $1 $GRAYBOX expect add `dirname $0`/$3/$5.pkt $7
	ip netns exec $1 $GRAYBOX expect add `dirname $0`/$3/$6.pkt $7
	ip netns exec $2 $GRAYBOX send `dirname $0`/$3/$4.pkt
	sleep 0.1
	ip netns exec $1 $GRAYBOX expect flush
}

# IPv6 to IPv4 test boilerplate: Sends one packet, expects one.
# $1: Subdirectory
# $2: Test packet
# $3: Expected packet
# $4: Exceptions (optional)
test64_11() {
	test_11 client4ns client6ns $1 $2 $3 $4
}

# IPv4 to IPv6 test boilerplate: Sends one packet, expects one.
# $1: Subdirectory
# $2: Test packet
# $3: Expected packet
# $4: Exceptions (optional)
test46_11() {
	test_11 client6ns client4ns $1 $2 $3 $4
}

test66_11() {
	test_11 client6ns client6ns $1 $2 $3 $4
}

test44_11() {
	test_11 client4ns client4ns $1 $2 $3 $4
}

# IPv6 to IPv4 test boilerplate: Sends one packet, expects two.
# $1: Subdirectory
# $2: Test packet
# $3: Expected packet 1
# $4: Expected packet 2
# $5: Exceptions (optional)
test64_12() {
	test_12 client4ns client6ns $1 $2 $3 $4 $5
}

# IPv6 to IPv4 test boilerplate: Sends one packet, expects two.
# $1: Subdirectory
# $2: Test packet
# $3: Expected packet 1
# $4: Expected packet 2
# $5: Exceptions (optional)
test46_12() {
	test_12 client6ns client4ns $1 $2 $3 $4 $5
}


`dirname $0`/../wait.sh 2001:db8:1c6:3364:2::
if [ $? -ne 0 ]; then
	exit 1
fi

echo "Testing! Please wait..."


# UDP, 6 -> 4
if [ -z "$1" -o "$1" = "udp64" ]; then
	test64_auto 6-udp-csumok-df-nofrag 4-udp-csumok-df-nofrag $IDENTIFICATION
	test64_auto 6-udp-csumok-nodf-nofrag 4-udp-csumok-nodf-nofrag
	test64_auto 6-udp-csumok-nodf-frag0 4-udp-csumok-nodf-frag0
	test64_auto 6-udp-csumok-nodf-frag1 4-udp-csumok-nodf-frag1
	test64_auto 6-udp-csumok-nodf-frag2 4-udp-csumok-nodf-frag2

	test64_auto 6-udp-csumfail-df-nofrag 4-udp-csumfail-df-nofrag $IDENTIFICATION
	test64_auto 6-udp-csumfail-nodf-nofrag 4-udp-csumfail-nodf-nofrag
	test64_auto 6-udp-csumfail-nodf-frag0 4-udp-csumfail-nodf-frag0
	test64_auto 6-udp-csumfail-nodf-frag1 4-udp-csumfail-nodf-frag1
	test64_auto 6-udp-csumfail-nodf-frag2 4-udp-csumfail-nodf-frag2
fi

# UDP, 4 -> 6
if [ -z "$1" -o "$1" = "udp46" ]; then
	test46_auto 4-udp-csumok-df-nofrag 6-udp-csumok-df-nofrag
	test46_auto 4-udp-csumok-nodf-nofrag 6-udp-csumok-nodf-nofrag
	test46_auto 4-udp-csumok-nodf-frag0 6-udp-csumok-nodf-frag0
	test46_auto 4-udp-csumok-nodf-frag1 6-udp-csumok-nodf-frag1
	test46_auto 4-udp-csumok-nodf-frag2 6-udp-csumok-nodf-frag2

	test46_auto 4-udp-csumfail-df-nofrag 6-udp-csumfail-df-nofrag
	test46_auto 4-udp-csumfail-nodf-nofrag 6-udp-csumfail-nodf-nofrag
	test46_auto 4-udp-csumfail-nodf-frag0 6-udp-csumfail-nodf-frag0
	test46_auto 4-udp-csumfail-nodf-frag1 6-udp-csumfail-nodf-frag1
	test46_auto 4-udp-csumfail-nodf-frag2 6-udp-csumfail-nodf-frag2
fi

# TCP, 6 -> 4
if [ -z "$1" -o "$1" = "tcp64" ]; then
	test64_auto 6-tcp-csumok-df-nofrag 4-tcp-csumok-df-nofrag $IDENTIFICATION
	test64_auto 6-tcp-csumok-nodf-nofrag 4-tcp-csumok-nodf-nofrag
	test64_auto 6-tcp-csumok-nodf-frag0 4-tcp-csumok-nodf-frag0
	test64_auto 6-tcp-csumok-nodf-frag1 4-tcp-csumok-nodf-frag1
	test64_auto 6-tcp-csumok-nodf-frag2 4-tcp-csumok-nodf-frag2

	test64_auto 6-tcp-csumfail-df-nofrag 4-tcp-csumfail-df-nofrag $IDENTIFICATION
	test64_auto 6-tcp-csumfail-nodf-nofrag 4-tcp-csumfail-nodf-nofrag
	test64_auto 6-tcp-csumfail-nodf-frag0 4-tcp-csumfail-nodf-frag0
	test64_auto 6-tcp-csumfail-nodf-frag1 4-tcp-csumfail-nodf-frag1
	test64_auto 6-tcp-csumfail-nodf-frag2 4-tcp-csumfail-nodf-frag2
fi

# ICMP info, 6 -> 4
if [ -z "$1" -o "$1" = "icmpi64" ]; then
	test64_auto 6-icmp6info-csumok-df-nofrag 4-icmp4info-csumok-df-nofrag $IDENTIFICATION
	test64_auto 6-icmp6info-csumok-nodf-nofrag 4-icmp4info-csumok-nodf-nofrag

	test64_auto 6-icmp6info-csumfail-df-nofrag 4-icmp4info-csumfail-df-nofrag $IDENTIFICATION
	test64_auto 6-icmp6info-csumfail-nodf-nofrag 4-icmp4info-csumfail-nodf-nofrag
fi

# ICMP info, 4 -> 6
if [ -z "$1" -o "$1" = "icmpi46" ]; then
	test46_auto 4-icmp4info-csumok-df-nofrag 6-icmp6info-csumok-df-nofrag
	test46_auto 4-icmp4info-csumok-nodf-nofrag 6-icmp6info-csumok-nodf-nofrag

	test46_auto 4-icmp4info-csumfail-df-nofrag 6-icmp6info-csumfail-df-nofrag
	test46_auto 4-icmp4info-csumfail-nodf-nofrag 6-icmp6info-csumfail-nodf-nofrag
fi

# ICMP error, 6 -> 4
if [ -z "$1" -o "$1" = "icmpe64" ]; then
	# 22,23 = ICMP csum. Inherits the followind fields' randomness.
	# 32,33 = inner frag id. Same as above.
	# 34 = inner DF. An atomic fragments free Jool has no way to know the DF of the original packet.
	# 38,39 = inner IPv4 csum. Inherits other field's randomness.
	test64_auto 6-icmp6err-csumok-df-nofrag 4-icmp4err-csumok-df-nofrag $IDENTIFICATION,22,23,32,33,34,38,39
	# This one doesn't have ignored bytes because DF and IDs have to be inferred from the fragment headers.
	test64_auto 6-icmp6err-csumok-nodf-nofrag 4-icmp4err-csumok-nodf-nofrag
fi

# ICMP error, 4 -> 6
if [ -z "$1" -o "$1" = "icmpe46" ]; then
	test46_auto 4-icmp4err-csumok-df-nofrag 6-icmp6err-csumok-df-nofrag
	test46_auto 4-icmp4err-csumok-nodf-nofrag 6-icmp6err-csumok-nodf-nofrag
fi

# "Manual" tests
if [ -z "$1" -o "$1" = "misc" ]; then
	test64_11 manual 6791v64t 6791v64e $IDENTIFICATION,$INNER_IDENTIFICATION
	test66_11 manual 6791v66t 6791v66e

	ip netns exec joolns jool_siit global update rfc6791v4-prefix null
	test64_11 manual 6791v64t 6791v64e-empty $IDENTIFICATION,$INNER_IDENTIFICATION
	ip netns exec joolns jool_siit global update rfc6791v4-prefix 203.0.113.8

	ip netns exec joolns jool_siit global update pool6 null
	ip netns exec joolns jool_siit eamt add 198.51.100.0/24 2001:db8:1c6:3364::/72
	ip netns exec joolns jool_siit eamt add 192.0.2.0/24 2001:db8:1c0:2::/72
	test46_11 manual 6791v46t 6791v46e-empty
	ip netns exec joolns jool_siit global update rfc6791v6-prefix 2::2
	test46_11 manual 6791v46t 6791v46e
	ip netns exec joolns jool_siit global update pool6 2001:db8:100::/40
	ip netns exec joolns jool_siit eamt remove 198.51.100.0/24 2001:db8:1c6:3364::/72
	ip netns exec joolns jool_siit eamt remove 192.0.2.0/24 2001:db8:1c0:2::/72
	ip netns exec joolns jool_siit global update rfc6791v6-prefix null
fi

# "RFC 6791" tests
if [ -z "$1" -o "$1" = "rfc7915" ]; then
	# a
	test46_11 7915 aat1 aae1
	test46_11 7915 aat2 aae1
	test46_11 7915 aat3 aae1
	test64_11 7915 abt1 abe1 $IDENTIFICATION
	test64_11 7915 abt2 abe1 $IDENTIFICATION
	test64_11 7915 abt3 abe2
	test66_11 7915 act1 ace1
	test66_11 7915 act2 ace2
	test66_11 7915 act3 ace3
	test66_11 7915 act4 ace4
	test66_11 7915 act5 ace5
	test66_11 7915 act6 ace6
	test64_11 7915 adt1 ade1 $IDENTIFICATION
	test64_11 7915 adt2 ade2

	# b
	test46_11 7915 bat1 bae1
	test46_11 7915 bbt1 bbe1
	test46_11 7915 bct1 bce1
	test64_11 7915 bdt1 bde1 $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 bet1 bee1 $IDENTIFICATION,$INNER_IDENTIFICATION

	# c
	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1500
	ip netns exec joolns ip link set dev to_client_v6 mtu 1280
	test44_11 7915 cat1 cae1 $TOS,$IDENTIFICATION
	test44_11 7915 cat2 cae2 $TOS,$IDENTIFICATION
	test46_11 7915 cbt1 cbe1
	# Implementation quirk: The RFC wants us to copy the IPv4 identification
	# value (16 bits) to the IPv6 identification field (32 bits).
	# However, fragmentation is done by the kernel after the translation,
	# which means Jool does not get to decide the identification value.
	#
	# I think this is fine because identification preservation is only
	# relevant when the packet is already fragmented. As a matter of fact,
	# it's better if the kernel decides the identification because it will
	# generate a 32 bit number, and not be constrained to 16 bits like Jool.
	#
	# Identification preservation for already fragmented packets is tested
	# in cf.
	test46_12 7915 cct1 cce1 cce2 44,45,46,47
	test46_11 7915 cdt1 cde1
	test46_11 7915 cet1 cee1
	test46_11 7915 cet2 cee1
	test46_12 7915 cft1 cfe1 cfe2
	ip netns exec joolns ip link set dev to_client_v6 mtu 1500

	ip netns exec joolns ip link set dev to_client_v4 mtu 1400
	test66_11 7915 cgt1 cge1
	test66_11 7915 cgt2 cge2
	ip netns exec joolns ip link set dev to_client_v4 mtu 1500

	test64_11 7915 cht1 che1 $IDENTIFICATION
	test64_11 7915 cit1 cie1 $IDENTIFICATION
	test64_11 7915 cjt1 cje1
	test64_11 7915 cjt2 cje2
	test64_11 7915 cjt3 cje3
	test64_11 7915 cjt4 cje4

	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1280
	# This one is actually ck.
	test46_12 7915 cct1 cce1 cce2

	# d
	test64_11 7915 dat1 dae1 $IDENTIFICATION,$INNER_IDENTIFICATION
	test46_11 7915 dbt1 dbe1
	test46_11 7915 dbt2 dbe2
	test46_11 7915 dbt3 dbe2

	# e
	test44_11 7915 eat1 eae1 $TOS,$IDENTIFICATION
	ip netns exec joolns jool_siit global update amend-udp-checksum-zero 1
	# Cannot be tested by graybox anymore, because the checksum is now
	# offloaded. The interface refuses to do it because it's virtual.
	#test46_11 7915 eat1 ebe1
	test44_11 7915 ect1 ece1 $TOS,$IDENTIFICATION
	ip netns exec joolns jool_siit global update amend-udp-checksum-zero 0
	test44_11 7915 ect1 ece1 $TOS,$IDENTIFICATION

	# f
	test46_11 7915 fat1 fae1
	test64_11 7915 fbt1 fbe1 $IDENTIFICATION

	# g
	test66_11 7915 gat1 gae1

	# h
	test46_11 7915 hat1 hae1
	test46_11 7915 hat2 hae2

	# i
	test64_11 7915 ia1t ia1e $IDENTIFICATION,$INNER_IDENTIFICATION
	test46_11 7915 ia2t ia2e
	test64_11 7915 ib1t ib1e $IDENTIFICATION,$INNER_IDENTIFICATION
	test46_11 7915 ib2t ib2e
	test46_11 7915 ib3t ib3e
	test64_11 7915 ic1t ic1e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic2t ic2e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic3t ic3e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic4t ic4e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic5t ic5e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic6t ic6e $IDENTIFICATION,$INNER_IDENTIFICATION
	test64_11 7915 ic7t ic7e $IDENTIFICATION,$INNER_IDENTIFICATION
	test46_11 7915 idat idae
	test46_11 7915 idbt idbe
	test46_11 7915 idct idce
	test46_11 7915 iddt idde
	test46_11 7915 idet idee
	test46_11 7915 idft idfe
	test46_11 7915 idht idhe
	test46_11 7915 idkt idke
	test46_11 7915 idzt idze

	ip netns exec client4ns ip link set dev to_jool_v4 mtu 2153
	ip netns exec joolns ip link set dev to_client_v4 mtu 2153
	sleep 0.5
	test46_11 7915 idgt idge
	test46_11 7915 idit idie
	test46_11 7915 idyt idye
	ip netns exec client4ns ip link set dev to_jool_v4 mtu 1500
	ip netns exec joolns ip link set dev to_client_v4 mtu 1500

	# j
	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1500
	test46_11 7915 jat jae
	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1280

	test46_12 7915 jbat jbae1 jbae2
	test46_11 7915 jbbt jbbe
	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1402
	test46_12 7915 jcat jcae1 jcae2
	test46_11 7915 jcbt jcbe
	ip netns exec joolns jool_siit global update lowest-ipv6-mtu 1280
fi

#if [ -z "$1" -o "$1" = "new" ]; then
#fi

$GRAYBOX stats display
result=$?
$GRAYBOX stats flush

exit $result
