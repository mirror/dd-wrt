# Graybox Tests: RFC 7915

These tests ensure Jool is RFC7915-conformant.

## Index

1. [Requirements Analysis](#part-1-requirements-analysis)
2. [Tests](#part-2-tests)

## Part 1: Requirements Analysis

Glossary:

- "Not a requirement": Statement is irrelevant to the implementation, so it should yield no test.
- "Too trivial": Requirement is so basic it's essentially already addressed in many other tests.
- "Untestable": Requirement cannot be tested by the graybox framework as currently implemented. (Usually silent packet drops (because Graybox needs responses), emerging IPv4 fragment identifiers (because of randomness) and corresponding IPv4 header checksums (because of fragment identifier randomness).)
- "Fragmentation Needed": ICMPv4 3/4
- "Packet too Big": ICMPv6 2/0

### 1.2. Applicability and Limitations

	This document specifies the translation algorithms between IPv4
	packets and IPv6 packets.

Not a requirement.

	As with [RFC6145], the translating function specified in this
	document does not translate any IPv4 options,

Test: Packet with IPv4 options -> Normal IPv6 packet ([aa](#aa))

	and it does not
	translate IPv6 extension headers except the Fragment Header.

Test: Packet with IPv6 extension headers before the fragment header -> Normal IPv4 packet ([ab](#ab))<br />
Test: Packet with IPv6 extension headers after the fragment header -> ICMPv6 error 1/1 ([ac](#ac))<br />
See [Simple Extension Headers](https://github.com/NICMx/Jool/wiki/RFC-7915-Review#simple-extension-headers).

	The issues and algorithms in the translation of datagrams containing
	TCP segments are described in [RFC5382].

Not a requirement.

	Fragmented IPv4 UDP packets that do not contain a UDP checksum (i.e.,
	the UDP checksum field is zero) are not of significant use on the
	Internet, and in general will not be translated by the IP/ICMP
	translator (Section 4.5).

Test: Fragmented UDP zero checksum -> Drop ([ec](#ec))

	However, when the translator is configured
	to forward the packet without a UDP checksum, the fragmented IPv4 UDP
	packets will be translated.

Requirement makes no sense. (How to compute a checksum out of a fragmented UDP packet?)<br />
Alternate interpretation: Translate the packet into a zero-checksum IPv6/UDP packet. (I don't trust this interpretation because these packets are supposed to be illegal.)<br />
Related: [Fragmented UDP Checksum Zero](https://github.com/NICMx/Jool/wiki/RFC-7915-Review#fragmented-udp-checksum-zero)

	Fragmented ICMP/ICMPv6 packets will not be translated by IP/ICMP
	translators.

Untestable.

	The IP/ICMP header translation specified in this document is
	consistent with requirements of multicast IP/ICMP headers.  However,
	IPv4 multicast addresses [RFC5771] cannot be mapped to IPv6 multicast
	addresses [RFC3307] based on the unicast mapping rule [RFC6052].  An
	example of experiments of the multicast address mapping can be found
	in [RFC6219].

Not a requirement.

### 1.3. Stateless vs. Stateful Mode

	An IP/ICMP translator has two possible modes of operation: stateless
	and stateful [RFC6144].  In both cases, we assume that a system (a
	node or an application) that has an IPv4 address but not an IPv6
	address is communicating with a system that has an IPv6 address but
	no IPv4 address, or that the two systems do not have contiguous
	routing connectivity,

Not a requirement.

	or they might have contiguous routing
	connectivity but are interacting via masking addresses (i.e.,
	hairpinning) [RFC4787], and hence are forced to have their
	communications translated.

Test: Hairpinning ([ga](#ga))

	In the stateless mode, an IP/ICMP translator will convert IPv4
	addresses to IPv6 and vice versa solely based on the configuration of
	the stateless IP/ICMP translator and information contained within the
	packet being translated.  For example, for the default behavior
	defined in [RFC6052], a specific IPv6 address range will represent
	IPv4 systems (IPv4-converted addresses), and the IPv6 systems have
	addresses (IPv4-translatable addresses) that can be algorithmically
	mapped to a subset of the service provider's IPv4 addresses.  Other
	stateless translation algorithms are defined in Section 6.  The
	stateless translator does not keep any dynamic session or binding
	state, thus there is no requirement that the packets in a single
	session or flow traverse a single translator.

	In the stateful mode, a specific IPv6 address range (consisting of
	IPv4-converted IPv6 addresses) will typically represent IPv4 systems.
	The IPv6 nodes may use any IPv6 addresses [RFC4291] except in that
	range.  A stateful IP/ICMP translator continuously maintains a
	dynamic translation table containing bindings between the IPv4 and
	IPv6 addresses, and likely also the Layer-4 identifiers, that are
	used in the translated packets.  The exact address translations of
	any given packet thus become dependent on how packets belonging to
	the same session or flow have been translated.  For this reason,
	stateful translation generally requires that all packets belonging to
	a single flow must traverse the same translator.

	In order to be able to successfully translate a packet from IPv4 to
	IPv6 or vice versa, the translator must implement an address mapping
	algorithm.  This document does not specify any such algorithms,
	instead these are referenced from Section 6.

Not a requirement.

(Address translation algorithms are tested elsewhere.)

### 1.4. Path MTU Discovery and Fragmentation

	Due to the different sizes of the IPv4 and IPv6 header, which are 20+
	octets and 40 octets respectively, handling the maximum packet size
	is critical for the operation of the IPv4/IPv6 translator.  There are
	three mechanisms to handle this issue: path MTU discovery (PMTUD),
	fragmentation, and transport-layer negotiation such as the TCP
	Maximum Segment Size (MSS) option [RFC6691].

Not a requirement.

	Note that the
	translator MUST behave as a router, i.e., the translator MUST send a
	Packet Too Big error message or fragment the packet when the packet
	size exceeds the MTU of the next-hop interface.

Test: IPv4 DF set, large packet -> Fragmentation Needed ([ca](#ca))<br />
Test: IPv4 DF unset, large packet -> IPv6 fragments ([cc](#cc))<br />
Test: IPv6, large packet -> Packet too Big ([cg](#cg))

	Don't Fragment, ICMP Packet Too Big, and packet fragmentation are
	discussed in Sections 4 and 5 of this document.  The reassembling of
	fragmented packets in the stateful translator is discussed in
	[RFC6146], since it requires state maintenance in the translator.

Not a requirement.

### 4. Translating from IPv4 to IPv6

	When an IP/ICMP translator receives an IPv4 datagram addressed to a
	destination towards the IPv6 domain, it translates the IPv4 header of
	that packet into an IPv6 header.  The original IPv4 header on the
	packet is removed and replaced by an IPv6 header, and the transport
	checksum is updated as needed, if that transport is supported by the
	translator.  The data portion of the packet is left unchanged.  The
	IP/ICMP translator then forwards the packet based on the IPv6
	destination address.

Too trivial.

	Path MTU discovery is mandatory in IPv6, but it is optional in IPv4.
	IPv6 routers never fragment a packet -- only the sender can do
	fragmentation.

Not a requirement.

	When an IPv4 node performs path MTU discovery (by setting the Don't
	Fragment (DF) bit in the header), path MTU discovery can operate end-
	to-end, i.e., across the translator.  In this case, either IPv4 or
	IPv6 routers (including the translator) might send back ICMP Packet
	Too Big messages to the sender.  When the IPv6 routers send these
	ICMPv6 errors, they will pass through a translator that will
	translate the ICMPv6 error to a form that the IPv4 sender can
	understand.

Test: Packet too Big -> Fragmentation Needed ([da](#da))<br />
Test: DF set, large packet -> Fragmentation Needed ([ca](#ca))

	As a result, an IPv6 Fragment Header is only included if
	the IPv4 packet is already fragmented.

> Note that this is sentence is unexpectedly valuable, because it's the only one in the entire RFC addressing DF-enabled IPv4 fragments.

In short: "The presence of the fragment header depends on actual fragmentation. It does not in any way depend on DF."

This is the same as the "When the IPv4 sender does not set the DF bit (...)" paragraph below. Scroll down a bit.

	However, when the IPv4 sender does not set the DF bit, the translator
	MUST ensure that the packet does not exceed the path MTU on the IPv6
	side.  This is done by fragmenting the IPv4 packet (with Fragment
	Headers) so that it fits in 1280-byte IPv6 packets, since that is the
	minimum IPv6 MTU.  The IPv6 Fragment Header has been shown to cause
	operational difficulties in practice due to limited firewall
	fragmentation support, etc.  In an environment where the network
	owned/operated by the same entity that owns/operates the translator,
	the translator MUST provide a configuration function for the network
	administrator to adjust the threshold of the minimum IPv6 MTU to a
	value that reflects the real value of the minimum IPv6 MTU in the
	network (greater than 1280 bytes).  This will help reduce the chance
	of including the Fragment Header in the packets.

Test: `lowest-ipv6-mtu` ([ck](#ck))

	When the IPv4 sender does not set the DF bit, the translator MUST NOT
	include the Fragment Header for the non-fragmented IPv6 packets.

Related: [DF and the Fragment Header](https://github.com/NICMx/Jool/wiki/RFC-7915-Review#df-and-the-fragment-header)

Test: IPv4 packet, small, DF -> no fragment header ([cb](#cb))<br />
Test: IPv4 packet, small, !DF -> no fragment header ([cd](#cd))<br />
Test: IPv4 packet, big, DF -> Fragmentation Needed ([ca](#ca))<br />
Test: IPv4 packet, big, !DF -> fragments ([cc](#cc))<br />
Test: IPv4 fragment, small, DF -> fragment ([ce](#ce))<br />
Test: IPv4 fragment, small, !DF -> fragment ([ce](#ce))<br />
Test: IPv4 fragment, big, DF -> Fragmentation Needed ([ca](#ca))<br />
Test: IPv4 fragment, big, !DF -> fragments ([cf](#cf))<br />

Incidentally:

Test: IPv6 packet, small -> IPv4 packet ([ch](#ch), [ci](#ci))<br />
Test: IPv6 packet, big -> Packet Too Big ([cg](#cg))<br />
Test: IPv6 fragment, small -> IPv4 fragment ([cj](#cj))<br />
Test: IPv6 fragment, big -> Packet Too Big ([cg](#cg))<br />

	The rules in Section 4.1 ensure that when packets are fragmented,
	either by the sender or by IPv4 routers, the low-order 16 bits of the
	fragment identification are carried end-to-end, ensuring that packets
	are correctly reassembled.

Test: IPv4 fragment -> IPv6 fragment, same identification number ([ce](#ce))

	Other than the special rules for handling fragments and path MTU
	discovery, the actual translation of the packet header consists of a
	simple translation as defined below.

Not a requirement.

	Note that ICMPv4 packets
	require special handling in order to translate the content of ICMPv4
	error messages and also to add the ICMPv6 pseudo-header checksum.

Test: ICMPv4 error containing IPv4 packet -> ICMPv6 error containing IPv6 packet ([ba](#ba))

	The translator SHOULD make sure that the packets belonging to the
	same flow leave the translator in the same order in which they
	arrived.

Not really a requirement, also essentially untestable.

### 4.1. Translating IPv4 Headers into IPv6 Headers

	If the DF flag is not set and the IPv4 packet will result in an IPv6
	packet larger than a user-defined length (hereinafter referred to as
	"lowest-ipv6-mtu", and which defaults to 1280 bytes), the packet
	SHOULD be fragmented so that the resulting IPv6 packet (with Fragment
	Header added to each fragment) will be less than or equal to lowest-
	ipv6-mtu, For example, if the packet is fragmented prior to the
	translation, the IPv4 packets should be fragmented so that their
	length, excluding the IPv4 header, is at most 1232 bytes (1280 minus
	40 for the IPv6 header and 8 for the Fragment Header).  The
	translator MUST provide a configuration function for the network
	administrator to adjust the threshold of the minimum IPv6 MTU to a
	value greater than 1280 bytes if the real value of the minimum IPv6
	MTU in the network is known to the administrator.  The resulting
	fragments are then translated independently using the logic described
	below.

Test: `lowest-ipv6-mtu` ([ck](#ck))

	If the DF bit is set and the MTU of the next-hop interface is less
	than the total length value of the IPv4 packet plus 20, the
	translator MUST send an ICMPv4 "Fragmentation Needed" error message
	to the IPv4 source address.

Test: DF set, large packet -> Fragmentation Needed ([ca](#ca))

	<Bunch of fields>

Most of these are trivial, and I've decided to postpone most of it.

	Note when
	translating ICMPv4 Error Messages into ICMPv6, the "illegal"
	source address will be translated for the purpose of trouble
	shooting.

Test: Allow illegal sources in ICMPv4 errors ([ha](#ha))

### 4.2. Translating ICMPv4 Headers into ICMPv6 Headers

	All ICMPv4 messages that are to be translated require that the ICMPv6
	checksum field be calculated as part of the translation since ICMPv6,
	unlike ICMPv4, has a pseudo-header checksum just like UDP and TCP.

Too trivial.

	In addition, all ICMPv4 packets MUST have the Type translated and,

Postponed.

	for ICMPv4 error messages, the included IP header also MUST be
	translated.

Test: ICMPv4 error containing IPv4 packet -> ICMPv6 error containing IPv6 packet ([ba](#ba))

	The actions needed to translate various ICMPv4 messages are as
	follows:
	<Bunch of types and codes>

Postponed.

	ICMP Error Payload:  If the received ICMPv4 packet contains an
	   ICMPv4 Extension [RFC4884], the translation of the ICMPv4
	   packet will cause the ICMPv6 packet to change length.  When
	   this occurs, the ICMPv6 Extension length attribute MUST be
	   adjusted accordingly (e.g., longer due to the translation
	   from IPv4 to IPv6).  If the ICMPv4 Extension exceeds the
	   maximum size of an ICMPv6 message on the outgoing interface,
	   the ICMPv4 extension SHOULD be simply truncated.  For
	   extensions not defined in [RFC4884], the translator passes
	   the extensions as opaque bit strings, and those containing
	   IPv4 address literals will not have their included addresses
	   translated to IPv6 address literals; this may cause problems
	   with processing of those ICMP extensions.

Test: ICMP extensions ([ia](#ia))

### 4.3. Translating ICMPv4 Error Messages into ICMPv6

	There are some differences between the ICMPv4 and the ICMPv6 error
	message formats as detailed above.  The ICMP error messages
	containing the packet in error MUST be translated just like a normal
	IP packet (except the TTL value of the inner IPv4/IPv6 packet).  If
	the translation of this "packet in error" changes the length of the
	datagram, the Total Length field in the outer IPv6 header MUST be
	updated.

All of these tests are ICMPv4 error containing IPv4 packet -> ICMPv6 error (TTL - 1) containing IPv6 packet (TTL - 0):<br />
Test: ICMPv4 error sized 400 (20 + 8 + 20 + 352) -> ICMPv6 error sized 440 (40 + 8 + 40 + 352) (normal case) ([ba](#ba))<br />
Test: ICMPv4 error sized 1500 (20 + 8 + 20 + 1452) -> ICMPv6 error sized 1280 (40 + 8 + 40 + 1192) (normal case, result truncated due to ICMP fragmentation preventer) ([bb](#bb))<br />
Test: ICMPv4 error sized 1280 (20 + 8 + 20 + 1232) -> ICMPv6 error sized 1280 (40 + 8 + 40 + 1192) (no datagram size change due to ICMP truncation) ([bc](#bc))

	The translation of the inner IP header can be done by invoking the
	function that translated the outer IP headers.  This process MUST
	stop at the first embedded header and drop the packet if it contains
	more embedded headers.

Untestable.

(It's interesting to note that it doesn't say "silently drop," but I know other RFCs prohibit sending ICMP errors caused by other ICMP errors. This paragraph is clearly wrong.)

### 4.4. Generation of ICMPv4 Error Message

	If the IPv4 packet is discarded, then the translator SHOULD be able
	to send back an ICMPv4 error message to the original sender of the
	packet, unless the discarded packet is itself an ICMPv4 error
	message.  The ICMPv4 message, if sent, has a Type of 3 (Destination
	Unreachable) and a Code of 13 (Communication Administratively
	Prohibited), unless otherwise specified in this document or in
	[RFC6146].  The translator SHOULD allow an administrator to configure
	whether the ICMPv4 error messages are sent, rate-limited, or not
	sent.

Off a quick `Ctrl+F`, these are the only unspecified ICMPv4 errors:

	1.  Dropping the packet and generating a system management event that
	specifies at least the IP addresses and port numbers of the
	packet.

Test: UDP checksum zero + `amend-udp-checksum-zero` disabled -> ICMP error ([ea](#ea))<br />

	Fragmented IPv4 UDP packets that do not contain a UDP checksum (i.e.,
	the UDP checksum field is zero) are not of significant use on the
	Internet, and in general will not be translated by the IP/ICMP
	translator (Section 4.5).

	(...)
	
	A stateless translator cannot compute the UDP checksum of
	fragmented packets, so when a stateless translator receives the
	first fragment of a fragmented UDP IPv4 packet and the checksum
	field is zero, the translator SHOULD drop the packet

Test: Fragmented UDP checksum zero -> ICMP error ([ec](#ec))<br />

	The translation of the inner IP header can be done by invoking the
	function that translated the outer IP headers.  This process MUST
	stop at the first embedded header and drop the packet if it contains
	more embedded headers.

No. This drop is obviously meant to be silent.

> TODO I will need to re-read the entire RFC again (and even the code) and search for more. I've already done this several times (including right now), so I'll wait a bit in case I find yet another similar requirement, so I can kill two birds with one stone.

### 4.5. Transport-Layer Header Translation

	If the address translation algorithm is not checksum neutral (see
	Section 4.1 of [RFC6052]), the recalculation and updating of the
	transport-layer headers that contain pseudo-headers need to be
	performed.  Translators MUST do this for TCP and ICMP packets and for
	UDP packets that contain a UDP checksum (i.e., the UDP checksum field
	is not zero).

Too trivial.

	For UDP packets that do not contain a UDP checksum (i.e., the UDP
	checksum field is zero), the translator SHOULD provide a
	configuration function to allow:

	1.  Dropping the packet and generating a system management event that
	    specifies at least the IP addresses and port numbers of the
	    packet.

Test: UDP checksum zero + `amend-udp-checksum-zero` disabled -> ICMP error ([ea](#ea))

	2.  Calculating an IPv6 checksum and forwarding the packet (which has
	    performance implications).

Test: UDP checksum zero + `amend-udp-checksum-zero` enabled -> Translation ([eb](#eb))

	A stateless translator cannot compute the UDP checksum of
	fragmented packets, so when a stateless translator receives the
	first fragment of a fragmented UDP IPv4 packet and the checksum
	field is zero, the translator SHOULD drop the packet and generate
	a system management event that specifies at least the IP
	addresses and port numbers in the packet.

Test: Fragmented UDP checksum zero -> ICMP error ([ec](#ec))

	    For a stateful translator, the handling of fragmented UDP IPv4
	    packets with a zero checksum is discussed in [RFC6146],
	    Section 3.4.

Not a requirement.

	Other transport protocols (e.g., the Datagram Congestion Control
	Protocol (DCCP)) are OPTIONAL to support.  In order to ease debugging
	and troubleshooting, translators MUST forward all transport protocols
	as described in the "Next Header" step of Section 4.1.

Test: IPv4 containing garbage transport -> IPv6 containing garbage transport ([fa](#fa))

### 4.6. Knowing When to Translate

	If the IP/ICMP translator also provides a normal forwarding function,
	and the destination IPv4 address is reachable by a more specific
	route without translation, the translator MUST forward it without
	translating it.  Otherwise, when an IP/ICMP translator receives an
	IPv4 datagram addressed to an IPv4 destination representing a host in
	the IPv6 domain, the packet MUST be translated to IPv6.

Untestable.

### 5. Translating from IPv6 to IPv4

	When an IP/ICMP translator receives an IPv6 datagram addressed to a
	destination towards the IPv4 domain, it translates the IPv6 header of
	the received IPv6 packet into an IPv4 header.  The original IPv6
	header on the packet is removed and replaced by an IPv4 header.
	Since the ICMPv6 [RFC4443], TCP [RFC793], UDP [RFC768], and DCCP
	[RFC4340] headers contain checksums that cover the IP header, if the
	address mapping algorithm is not checksum neutral, the checksum MUST
	be evaluated before translation and the ICMP and transport-layer
	headers MUST be updated.  The data portion of the packet is left
	unchanged.  The IP/ICMP translator then forwards the packet based on
	the IPv4 destination address.

Too trivial.

	There are some differences between IPv6 and IPv4 (in the areas of
	fragmentation and the minimum link MTU) that affect the translation.
	An IPv6 link has to have an MTU of 1280 bytes or greater.  The
	corresponding limit for IPv4 is 68 bytes.  Path MTU discovery across
	a translator relies on ICMP Packet Too Big messages being received
	and processed by IPv6 hosts.

	The difference in the minimum MTUs of IPv4 and IPv6 is accommodated
	as follows:

Not a requirement.

	o  When translating an ICMPv4 "Fragmentation Needed" packet, the
	   indicated MTU in the resulting ICMPv6 "Packet Too Big" will never
	   be set to a value lower than 1280.  This ensures that the IPv6
	   nodes will never have to encounter or handle Path MTU values lower
	   than the minimum IPv6 link MTU of 1280.  See Section 4.2.

Test: Fragmentation Needed < 1280 -> Packet Too Big = 1280 ([db](#db))

	o  When the resulting IPv4 packet is smaller than or equal to 1260
	   bytes, the translator MUST send the packet with a cleared Don't
	   Fragment bit.  Otherwise, the packet MUST be sent with the Don't
	   Fragment bit set.  See Section 5.1.

Test: IPv6 packet length <= 1280 -> DF disabled ([ci](#ci))<br />
Test: IPv4 packet length > 1280 -> DF enabled ([ch](#ch))

	This approach allows Path MTU Discovery to operate end-to-end for
	paths whose MTU are not smaller than the minimum IPv6 MTU of 1280
	(which corresponds to an MTU of 1260 in the IPv4 domain).  On paths
	that have IPv4 links with MTU < 1260, the IPv4 router(s) connected to
	those links will fragment the packets in accordance with Section 2.3
	of [RFC791].

	Other than the special rules for handling fragments and path MTU
	discovery, the actual translation of the packet header consists of a
	simple translation as defined below.

Not a requirement.

	Note that ICMPv6 packets
	require special handling in order to translate the contents of ICMPv6
	error messages and also to remove the ICMPv6 pseudo-header checksum.

Too trivial.

	The translator SHOULD make sure that the packets belonging to the
	same flow leave the translator in the same order in which they
	arrived.

Untestable.

### 5.1. Translating IPv6 Headers into IPv4 Headers

Postponed.

### 5.1.1. IPv6 Fragment Processing

Mostly postponed.

	Identification:  Copied from the low-order 16 bits in the
	   Identification field in the Fragment Header.

	Flags:  The IPv4 More Fragments (MF) flag is copied from the M flag
	   in the IPv6 Fragment Header.  The IPv4 Don't Fragment (DF) flag is
	   cleared (set to zero), allowing this packet to be further
	   fragmented by IPv4 routers.

Test: IPv6 fragment -> IPv4 fragment ("same" identification number, !DF, MF copied) ([cj](#cj))

### 5.2. Translating ICMPv6 Headers into ICMPv4 Headers

	If a non-checksum-neutral translation address is being used, ICMPv6
	messages MUST have their ICMPv4 checksum field be updated as part of
	the translation since ICMPv6 (unlike ICMPv4) includes a pseudo-header
	in the checksum just like UDP and TCP.

	In addition, all ICMP packets MUST have the Type translated and, for
	ICMP error messages, the included IP header MUST also be translated.

Too trivial.

	<Rest>

Postponed.

	ICMP Error Payload:  If the received ICMPv6 packet contains an
	   ICMPv6 Extension [RFC4884], the translation of the ICMPv6
	   packet will cause the ICMPv4 packet to change length.  When
	   this occurs, the ICMPv6 Extension length attribute MUST be
	   adjusted accordingly (e.g., shorter due to the translation from
	   IPv6 to IPv4).  For extensions not defined in [RFC4884], the
	   translator passes the extensions as opaque bit strings and any
	   IPv6 address literals contained therein will not be translated
	   to IPv4 address literals; this may cause problems with
	   processing of those ICMP extensions.

Test: ICMP extensions ([ia](#ia))

### 5.3. Translating ICMPv6 Error Messages into ICMPv4

	There are some differences between the ICMPv4 and the ICMPv6 error
	message formats as detailed above.  The ICMP error messages
	containing the packet in error MUST be translated just like a normal
	IP packet (except that the TTL/Hop Limit value of the inner IPv4/IPv6
	packet are not decremented).  The translation of this "packet in
	error" is likely to change the length of the datagram; thus, the
	Total Length field in the outer IPv4 header MUST be updated.

Relevant: [TODO](7915 review -> pedantic complaints)

All of these tests are ICMPv6 error containing IPv6 packet -> ICMPv4 error (TTL - 1) containing IPv4 packet (TTL - 0):<br />
Test: ICMPv6 error sized 400 (40 + 8 + 40 + 312) -> ICMPv4 error sized 360 (20 + 8 + 20 + 312) (normal case) ([bd](#bd))<br />
Test: ICMPv6 error sized 700 (40 + 8 + 40 + 612) -> ICMPv4 error sized 576 (20 + 8 + 20 + 528) (result truncated due to ICMP fragmentation preventer) ([be](#be))

<!-- Test: 576 (40 + 8 + 40 + 488?) -> 576 (20 + 8 + 20 + 528) -->

	The translation of the inner IP header can be done by invoking the
	function that translated the outer IP headers.  This process MUST
	stop at the first embedded header and drop the packet if it contains
	more embedded headers.

Untestable.

(It's interesting to note that it doesn't say "silently drop," but I know other RFCs prohibit sending ICMP errors caused by other ICMP errors. This paragraph is clearly wrong.)

### 5.4. Generation of ICMPv6 Error Messages

	If the IPv6 packet is discarded, then the translator SHOULD send back
	an ICMPv6 error message to the original sender of the packet, unless
	the discarded packet is itself an ICMPv6 message.

	The ICMPv6 message MUST have Type 1 (Destination Unreachable) and
	Code 1 (Communication with destination administratively prohibited),
	unless otherwise specified in this document or [RFC6146].  The
	translator SHOULD allow an administrator to configure whether the
	ICMPv6 error messages are sent, rate-limited, or not sent.

Off a quick `Ctrl+F`, these are the unspecified ICMPv6 errors I found:

	Total Length:  If the Next Header field of the Fragment Header is an
	extension header (except ESP, but including the Authentication
	Header (AH)), then the packet SHOULD be dropped and logged.

Test: Extension header after Fragment -> ICMP error ([ac](#ac))

	The translation of the inner IP header can be done by invoking the
	function that translated the outer IP headers.  This process MUST
	stop at the first embedded header and drop the packet if it contains
	more embedded headers.

No. This drop is obviously meant to be silent.

### 5.5. Transport-Layer Header Translation

	If the address translation algorithm is not checksum neutral (see
	Section 4.1 of [RFC6052]), the recalculation and updating of the
	transport-layer headers that contain pseudo-headers need to be
	performed.  Translators MUST do this for TCP, UDP, and ICMP.

Too trivial.

	Other transport protocols (e.g., DCCP) are OPTIONAL to support.  In
	order to ease debugging and troubleshooting, translators MUST forward
	all transport protocols as described in the "Protocol" step of
	Section 5.1.

Test: IPv6 containing garbage transport -> IPv4 containing garbage transport ([fa](#fa))

### 5.6. Knowing When to Translate

	If the IP/ICMP translator also provides a normal forwarding function,
	and the destination address is reachable by a more specific route
	without translation, the router MUST forward it without translating
	it.  When an IP/ICMP translator receives an IPv6 datagram addressed
	to an IPv6 address representing a host in the IPv4 domain, the IPv6
	packet MUST be translated to IPv4.

Untestable.

## Part 2: Tests

Glossary:

- "X must not be translated": "The translator must pretend that X does not exist, and continue translation normally."
- "X must be ignored": Same as "X must not be translated."
- "X must be dropped": "X is a packet, and must be eliminated without response nor translation."
- "X must be rejected": "X is a packet, and must be eliminated without translation, but the source must be informed of this event by way of an ICMP error."

All MTUs default to 1500.

### Extra Header Baggage Tests

See [Simple Extension Headers](https://github.com/NICMx/Jool/wiki/RFC-7915-Review#simple-extension-headers).

#### aa

- Requirement: Jool _must not translate_ any IPv4 options found in the original packet.
- Test packets:
	1. IPv4 without IPv4 options.
	2. `test-1` plus IPv4 option set A.
	3. `test-1` plus IPv4 option set B.
- Expected packets:
	1. Simple IPv6 (no extension headers).
- Validation: Each of the test packets must translate into `expected-1`.

Option set A:

- Security: 130 11 0 0 0 0 0 0 0 0 0
- Loose Source and Record Route: 131 11 12 198.51.100.2 198.51.100.1
- Strict Source and Record Route: 137 11 12 198.51.100.2 198.51.100.1
- Stream Identifier: 136 4 1 2
- End Of Options: 0
- No Operation: 1
- No Operation: 1

Option set B:

- Record Route: 7 11 8 198.51.100.2 0 0 0 0
- Internet Timestamp: 68 20 13 1 198.51.100.2 1 0 0 0 0.0.0.0 0 0 0 0
- End Of Options: 0

Notes:

- I needed two option sets because the maximum option length is 40 and I want to try all the 791-defined options.
- Though RFC 791 states that No Operation "may" be used to "align the beginning of a subsequent option on a 32 bit boundary," it never states that options MUST be aligned.

#### ab

- Requirement: Jool _must not translate_ any SEHs found before the Fragment Header.<br />
  Note: If there is no Fragment Header, behavior is undefined. In these cases, Jool treats all SEHs as "before Fragment" SEHs.
- Test packets:
	1. IPv6 packet with no extension headers.
	2. `test-1` plus SEHs.
	3. `test-2` plus a Fragment Header after the SEHs.
- Expected packets:
	1. Unfragmented IPv4 packet.
	2. Fragmented IPv4 packet.
- Validation: `test-1` and `test-2` must yield `expected-1`, `test-3` must yield `expected-2`.

#### ac

- Requirement: Jool must _reject_ packets containing SEHs after the Fragment Header.
- Test packets:
	1. IPv6 packet with Fragment Header, then a Hop-by-Hop extension header.
	2. IPv6 packet with Fragment Header, then a Destination Options extension header.
	3. IPv6 packet with Fragment Header, then a Routing extension header.
	4. IPv6 packet with a Hop-by-Hop extension header, then a Fragment Header, then a Hop-by-Hop extension header.
	5. IPv6 packet with a Destination Options extension header, then a Fragment Header, then a Destination Options extension header.
	6. IPv6 packet with a Routing extension header, then a Fragment Header, then a Routing extension header.
- Expected packets (src: `2001:db8:1c0:2:1::`, dst: `2001:db8:1c0:2:21::`, no hop limit - 1):
	1. ICMPv6 1/1 error containing `test-1`.
	2. ICMPv6 1/1 error containing `test-2`.
	3. ICMPv6 1/1 error containing `test-3`.
	4. ICMPv6 1/1 error containing `test-4`.
	5. ICMPv6 1/1 error containing `test-5`.
	6. ICMPv6 1/1 error containing `test-6`.
- Validation: `test-n` must yield `expected-n`.

Design notes: The RFC states that these packets "SHOULD be dropped and logged," which seemingly implies that the default ICMP error from [section 5.4](https://tools.ietf.org/html/rfc7915#section-5.4) is supposed to kick in.

#### ad

- Requirement: N/A (Intent: Jool must not crash when it receives an extension header for which there's no defined behavior in RFC 7915.)
- Validation: `adt<n>` must yield `ade<n>`.

	packet adt1
		40	IPv6		nextHeader:59

	packet adt2
		40	IPv6
		8	Fragment	nextHeader:59 identification:16909320

	packet ade1
		20	IPv4		!df ttl-- protocol:59 swap

	packet ade2
		20	IPv4		!df ttl-- protocol:59 swap identification:1032

Jool treats No Next Header like an "unknown" transport protocol. With the empty payload, this test is a special case of [fa](#fa). (Also [cj](#cj).)

### ICMP Error Tests

#### ba

- Requirements:
	1. When translating an ICMPv4 error, the internal packet must also be translated.
	2. Outer TTL must be decremented, inner TTL must not.
- Test packets:
	1. ICMPv4 error sized 400 (20 + 8 + 20 + 8 + 344)
- Expected packets:
	1. ICMPv6 error (TTL - 1) sized 440 (40 + 8 + 40 + 8 + 344)
- Validation: `test-1` must yield `expected-1`.

#### bb

- Requirements:
	1. If an ICMPv4 error's internal packet is truncated during translation, the external packet's total length must reflect this.
	2. Outer TTL must be decremented, inner TTL must not.
	3. ICMPv6 errors must length 1280 bytes at most, to maximize delivery probability.
- Test packets:
	1. ICMPv4 error sized 1500
		1. IPv4: 20
		2. ICMPv4: 8
		3. IPv4: 20 (addresses swapped, TTL-1)
		4. UDP: 8
		5. Payload: 1444
- Helper packets:
	1. IPv6/UDP request
		1. IPv6: 40 (TTL-1)
		2. UDP: 8
		3. Payload: 1444
- Expected packets:
	1. ICMPv6 error sized 1280
		1. IPv6: 40 (addresses swapped, TTL-1)
		2. ICMPv6: 8
		3. Payload: `helper-1` (truncated to 1232)
- Validation: `test-1` must yield `expected-2`.

Requirement 3 is sort of [mandated by Linux](https://github.com/NICMx/Jool/blob/0a1b2c8ebdd4012366a14c7ba2c81427d40c3280/src/mod/common/rfc7915/6to4.c#L77), not the RFC.

#### bc

- Requirement: Same as the previous one, except the packet size does not change.
- Test packets:
	1. ICMPv4 error sized 1280
		1. IPv4: 20
		2. ICMPv4: 8
		3. IPv4: 20 (addresses swapped, TTL-1)
		4. UDP: 8
		5. Payload: 1224
- Helper packet:
	1. IPv6/UDP request
		3. IPv6 (TTL-1): 40
		4. UDP: 8
		5. Payload: 1224
- Expected packets:
	1. ICMPv6 error sized 1280
		1. IPv6: 40 (addresses swapped, TTL-1)
		2. ICMPv6: 8
		3. Payload: `helper-1` (truncated to 1232)
- Validation: `test-1` must yield `expected-1`.

This is just an edge case, and it's probably pointless.

#### bd

- Requirements:
	1. When translating an ICMPv6 error, the internal packet must also be translated.
	2. Outer TTL must be decremented, inner TTL must not.
- Test packets:
	1. ICMPv6 error sized 400
		1. IPv6: 40
		2. ICMPv6: 8
		3. IPv6: 40 (addresses swapped, TTL-1)
		4. UDP: 8
		4. Payload: 304
- Expected packets:
	1. ICMPv4 error sized 360
		1. IPv4: 20 (addresses swapped, !DF, TTL-1)
		2. ICMPv4: 8
		3. IPv4: 20 (!DF, TTL-1)
		4. UDP: 8
		5. Payload: 304
- Validation: `test-1` must yield `expected-1`.

#### be

- Requirements:
	1. If the internal packet is truncated, the external packet's total length must reflect this.
	2. Outer TTL must be decremented, inner TTL must not.
	3. ICMPv4 errors must length 576 bytes at most, to maximize delivery probability.
- Test packets:
	1. ICMPv6 error sized 700
		1. IPv6: 40
		2. ICMPv6: 8
		3. IPv6: 40 (addresses swapped, TTL-1)
		4. UDP: 8
		5. Payload: 604
- Helper packets:
	1. IPv4/UDP request
		3. IPv4: 20 (!DF, TTL-1)
		4. UDP: 8
		5. Payload: 604
- Expected packets:
	1. ICMPv4 error sized 576
		1. IPv4: 20 (addresses swapped, !DF, TTL-1)
		2. ICMPv4: 8
		3. Payload: `helper-1` (truncated to 548)
- Validation: `test-1` must yield `expected-1`.

Requirement 3 is sort of [mandated by Linux](https://github.com/NICMx/Jool/blob/0a1b2c8ebdd4012366a14c7ba2c81427d40c3280/src/mod/common/rfc7915/6to4.c#L77), not the RFC.

### Atomic Fragment Deprecation Tests

In this section, `lowest-ipv6-mtu` must always be 1500 unless otherwise specified.

This section contains some seemingly broken fragments (in that their payloads don't length multiples of 8). They are used to test MTU limits. This is fine; Jool doesn't care whether a fragment can be reassembled or not. It's more important to check that Jool doesn't break when it receives bad data anyway.

#### ca

- Requirement: If an IPv4 packet has DF enabled and is too large, Jool must return Fragmentation Needed.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1261
- Test Packets:
	1. IPv4 packet sized 1261
		1. IPv4: 20 (DF)
		2. TCP: 20
		3. Payload: 1221
	2. IPv4 fragment sized 1261
		1. IPv4: 20 (identification:1234, DF, MF)
		2. TCP: 20
		3. Payload: 1221
- Expected packets:
	1. Fragmentation Needed containing `test-1`, size 576
		1. IPv4: 20 (src: 198.51.100.1, dst: 198.51.100.2, !DF)
		2. ICMPv4: 8 (3/4, MTU: 1260)
		3. Payload: `test-1` (truncated to 548)
	2. Fragmentation Needed containing `test-2`, size 576
		1. IPv4: 20 (src: 198.51.100.1, dst: 198.51.100.2, !DF)
		2. ICMPv4: 8 (3/4, MTU: 1260)
		3. Payload: `test-2` (truncated to 548)
- Validation: `test-n` must yield `expected-n`.

!DF on the expected packets is a Linux quirk. Perhaps that field should be ignored instead.

IPv6 counterpart: [cg](#cg)

#### cb

- Requirement: If an IPv4 packet has DF enabled but it's small enough, translation must succeed.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1260
- Test packets:
	1. IPv4 packet sized 1260
		1. IPv4: 20 (DF)
		2. TCP: 20
		3. Payload: 1220
- Expected packets:
	1. IPv6 packet (no fragment header) sized 1280
		1. IPv6: 40 (TTL-1, swap addresses)
		2. TCP: 20
		3. Payload: 1220
- Validation: `test-1` must yield `expected-1`.

(This an [cd](#cd) are essentially the same test. Might want to merge them.)

IPv6 counterparts: [ch](#ch), [ci](#ci)

#### cc

- Requirement: If an IPv4 packet has DF disabled and it's too large, Jool must fragment.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1261
- Test packets:
	1. IPv4/TCP sized 1261
		1. IPv4: 20 (!DF, ID 1234)
		2. TCP: 20
		3. Payload: 1221
- Helper packets:
	1. Full IPv6 packet (needed to compute the TCP checksum)
		1. IPv6: 40 (TTL-1, swap addresses)
		2. TCP: 20
		3. Payload: 1221
- Expected packets:
	1. IPv6 fragment sized 1280
		1. IPv6: 40 (swap addresses, TTL-1)
		2. Fragment: 8 (MF, ID 1234)
		3. TCP: 20 (checksum: same as `helper-1`)
		3. Payload: 1212
	2. IPv6 fragment sized 57
		1. IPv6: 40 (swap addresses, TTL-1)
		2. Fragment: 8 (Nexthdr TCP, Fragment offset 1232, !MF, ID 1234)
		3. Payload: 9 (offset: 188)
- Validation: `test-1` must yield both `expected-1` and `expected-2`.

Similar to [ck](#ck).

#### cd

- Requirement: If an IPv4 packet has DF disabled and it's small enough, translation must succeed.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1260
- Test packets:
	1. IPv4 packet sized 1260
		1. IPv4: 20 (!DF)
		2. TCP: 20
		3. Payload: 1220
- Expected packets:
	1. IPv6 packet (no fragment header) sized 1280.
		1. IPv6: 40 (TTL-1, swap addresses)
		2. TCP: 20
		3. Payload: 1220
- Validation: `test-1` must yield `expected-1`.

(This an [cb](#cb) are essentially the same test. Might want to merge them.)

#### ce

- Requirement: Small IPv4 fragment must yield an IPv6 fragment with the same identification number.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1252
- Test packets:
	1. IPv4/TCP fragment
		1. IPv4: 20 (DF, MF, ID 1234)
		2. TCP: 20
		3. Payload: 1212
	2. IPv4/TCP fragment
		1. IPv4: 20 (!DF, MF, ID 1234)
		2. TCP: 20
		3. Payload: 1212
- Expected packets:
	1. IPv6 fragment sized 1280
		1. IPv6: 40 (TTL-1, swap addresses)
		2. Fragment: 8 (MF, ID 1234)
		3. TCP: 20
		4. Payload: 1212
- Validation: `test-1` must yield `expected-1`, `test-2` must yield `expected-1`.

IPv6 counterpart: [cj](#cj)

#### cf

- Requirement: Large DF-disabled IPv4 fragment must yield multiple IPv6 fragments with the same identification number.
- Environment: IPv6 MTU = 1280, IPv4 MTU >= 1268
- Helper packets:
	1. Full version of test packets (used to retrieve TCP checksum)
		1. IPv4: 20 (ID 4321)
		2. TCP: 20
		3. Payload: 1300
	2. Full version of expected packets (used to retrieve TCP checksum)
		1. IPv6: 40
		2. TCP: 20
		3. Payload: 1300
- Test packets:
	1. IPv4/TCP fragment sized 1268
		1. IPv4: 20 (!DF, MF, ID 4321)
		2. TCP: 20 (copy checksum from `helper-1` (0x8B00))
		3. Payload: 1228
- Expected packets:
	1. IPv6 fragment sized 1280
		1. IPv6: 40 (TTL-1, swap addresses)
		2. Fragment: 8 (MF, ID 4321)
		3. TCP: 20 (copy checksum from `helper-2` (0xE4D6))
		4. Payload: 1212
	2. IPv6 fragment sized 64
		1. IPv6: 40 (TTL-1, swap addresses)
		2. Fragment: 8 (Nexthdr 6, Fragment offset 1232, MF, ID 4321)
		3. Payload: 16 (Offset: 188)
- Validation: `test-1` must yield `expected-1` and `expected-2`.

#### cg

- Requirement: Large IPv6 packet must yield a Packet too Big.
- Environment: IPv6 MTU >= 1421, IPv4 MTU = 1400
- Test packets:
	1. IPv6 packet sized 1421
		1. IPv6: 40
		2. TCP: 20
		3. Payload: 1361
	2. IPv6 fragment sized 1429
		1. IPv6: 40
		2. Frag: 8 (MF, identification 1234)
		2. TCP: 20
		3. Payload: 1361
- Expected packets:
	1. Packet too Big sized 1280
		1. IPv6: 40 (src: `2001:db8:1c0:2:1::`, dst: `2001:db8:1c0:2:21::`)
		2. ICMPv6: 8 (2/0, MTU 1420)
		3. Payload: `test-1` (truncated to 1232)
	2. Packet too Big sized 1280
		1. IPv6: 40 (src: `2001:db8:1c0:2:1::`, dst: `2001:db8:1c0:2:21::`)
		2. ICMPv6: 8 (2/0, MTU 1420)
		3. Payload: `test-2` (truncated to 1232)
- Validation: `test-n` must yield `expected-n`.

IPv4 counterpart: [ca](#ca)

#### ch

- Requirement: Medium-sized IPv6 packet must yield DF enabled.
- Environment: IPv6 MTU >= 1281, IPv4 >= 1261
- Test packets:
	1. IPv6 packet sized 1281
		1. IPv6: 40
		2. TCP: 20
		3. Payload: 1221
- Expected packets:
	1. IPv4 packet sized 1261
		1. IPv4: 20 (DF, TTL-1, swap addresses)
		2. TCP: 20
		3. Payload: 1221
- Validation: `test-1` must yield `expected-1`.

IPv4 counterpart: [cb](#cb)

I chose those MTUs because they are the smallest possible values for a "medium-sized" packet (see [ci](#ci)). Maybe there should be another test using the largest possible values as well.

#### ci

- Requirement: Small IPv6 packet must yield DF disabled.
- Environment: IPv6 MTU >= 1280, IPv4 MTU >= 1260
- Test packets:
	1. IPv6 packet sized 1280
		1. IPv6: 40
		2. TCP: 20
		3. Payload: 1220
- Expected packets:
	1. IPv4 packetd sized 1260
		1. IPv4: 20 (!DF, TTL-1, swap addresses)
		2. TCP: 20
		3. Payload: 1220
- Validation: `test-1` must yield `expected-1`.

IPv4 counterpart: [cb](#cb)

#### cj

- Requirement: Small or medium IPv6 fragment must yield an IPv4 fragment with the "same" identification number, DF disabled and copied MF.
- Environment: IPv6 MTU=1500, IPv4 MTU=1500
- Test packets:
	1. Small IPv6 fragment, MF
		1. IPv6: 40
		2. Frag: 8 (nextHeader:6, fragmentOffset:80, MF, identification:16909320)
		3. Payload: 1239
	2. Small IPv6 fragment, !MF
		1. IPv6: 40
		2. Frag: 8 (nextHeader:6, fragmentOffset:80, !MF, identification:16909320)
		3. Payload: 1239
	3. Medium-sized IPv6 fragment, MF
		1. IPv6: 40
		2. Frag: 8 (nextHeader:6, fragmentOffset:80, MF, identification:16909320)
		3. Payload: 1240
	4. Medium-sized IPv6 fragment, !MF
		1. IPv6: 40
		2. Frag: 8 (nextHeader:6, fragmentOffset:80, !MF, identification:16909320)
		3. Payload: 1240
- Expected packets:
	1. IPv4 fragment sized 1259, MF
		1. IPv4: 20 (identification:1032, !DF, MF, fragmentOffset:80, TTL-1, protocol:6, swap addresses)
		2. Payload: 1239
	2. IPv4 fragment sized 1259, !MF
		1. IPv4: 20 (identification:1032, !DF, !MF, fragmentOffset:80, TTL-1, protocol:6, swap addresses)
		2. Payload: 1239
	3. IPv4 fragment sized 1260, MF
		1. IPv4: 20 (identification:1032, !DF, MF, fragmentOffset:80, TTL-1, protocol:6, swap addresses)
		2. Payload: 1240
	4. IPv4 fragment sized 1260, !MF
		1. IPv4: 20 (identification:1032, !DF, !MF, fragmentOffset:80, TTL-1, protocol:6, swap addresses)
		2. Payload: 1240
- Validation: `test-n` must yield `expected-n`.

IPv4 counterpart: [ce](#ce)

#### ck

- Requirement: IPv6 counterparts of DF-disabled packets must be size-constrained to `lowest-ipv6-mtu`.
- Environment: `lowest-ipv6-mtu`=1280, all interface MTUs=1500
- Test packets: Same as [cc](#cc).
- Expected packets: Same as [cc](#cc).
- Validation: Same as [cc](#cc).

### Forwarding Path MTU Discovery Tests

Non-forwarding PMTUD tests were already included in the previous section.

#### da

- Requirement: Packet too Big should become Fragmentation Needed, with MTU adjusted.
- Environment: IPv6 MTU > 1280, IPv4 MTU > 1260 (Just go back to 1500 defaults at this point)
- Helper packets:
	1. IPv6 request
		1. IPv6: 40 (src: 2001:db8:1c6:3364:2::, dst: 2001:db8:3::60, TTL-1)
		2. TCP: 20
		3. Payload: 1221
	2. IPv4 request
		1. IPv4: 20 (src: 198.51.100.2, dst: 1.0.0.96, TTL-1)
		2. TCP: 20
		3. Payload: 1221
- Test packets:
	1. Packet too Big
		1. IPv6: 40
		2. ICMPv6: 8 (2/0, MTU 1280)
		3. Payload: `helper-1` (truncated to 1232)
- Expected packets:
	1. Fragmentation Needed
		1. IPv4: 20 (TTL-1, !DF, swap addresses)
		2. ICMPv4: 8 (3/4, MTU 1260)
		3. Payload: `helper-2` (truncated to 548)
- Validation: `test-1` must yield `expected-1`.

#### db

- Requirement: Fragmentation Needed should become Packet too Big, with MTU adjusted. IPv6 MTU must not go below 1280.
- Helper packets:
	1. IPv4 request sized sized 1262
		1. IPv4: 20 (src: 192.0.2.33, dst: 10.0.0.96, TTL-1)
		2. UDP: 8
		3. Payload: 1234
	2. IPv6 request
		1. IPv6: 40 (src: 2001:db8:1c0:2:21::, dst: 2001:db8:2::60, TTL-1)
		2. UDP: 8
		3. Payload: 1234
- Test packets:
	1. Fragmentation Needed
		1. IPv4: 20
		2. ICMPv4: 8 (3/4, MTU 1261)
		3. Payload: `helper-1`
	2. Fragmentation Needed
		1. IPv4: 20
		2. ICMPv4: 8 (3/4, MTU 1260)
		3. Payload: `helper-1`
	3. Fragmentation Needed
		1. IPv4: 20
		2. ICMPv4: 8 (3/4, MTU 1259)
		3. Payload: `helper-1`
- Expected packets:
	1. Packet too Big
		1. IPv6: 40 (TTL-1, swap addresses)
		2. ICMPv6: 8 (2/0, MTU 1281)
		3. Payload: `helper-2` (truncated to 1232)
	2. Packet too Big
		1. IPv6: 40 (TTL-1, swap addresses)
		2. ICMPv6: 8 (2/0, MTU 1280)
		3. Payload: `helper-2` (truncated to 1232)
- Validation: `test-1` must yield `expected-1`, `test-2` and `test-3` must yield `expected-2`.

### Zero UDP Checksum Tests

#### ea

- Requirement: If `amend-udp-checksum-zero` is disabled, an unfragmented UDP packet with checksum = 0 must be _rejected_.
- Environment: Disable `amend-udp-checksum-zero` configuration option.
- Test packets:
	1. IPv4/UDP packet
		1. IPv4: 20
		2. UDP: 8 (Checksum 0)
		3. Payload: 4
- Expected packets:
	1. ICMP error
		1. IPv4: 20 (src: 198.51.100.1, dst: 198.51.100.2, !DF)
		2. ICMPv4: 8 (3/13)
		3. Payload: `test-1`
- Validation: `test-1` must yield `expected-1`.

!DF on `expected-1` is a Linux quirk. Perhaps that field should be ignored instead.

#### eb

- Requirement: If `amend-udp-checksum-zero` is enabled, an unfragmented UDP packet with checksum = 0 must be translated, its checksum computed from scratch.
- Validation: `test-1` must yield `expected-1`.
- Environment: Enable `amend-udp-checksum-zero` configuration option.

	packet test-1
		Same as ea.test-1

	packet expected-1
		40	IPv6	swap ttl--
		8	UDP
		4	Payload

#### ec

- Requirement: Fragmented UDP packets with checksum = 0 must be _rejected_.
- Test packets:
	1. IPv4/UDP fragment
		1. IPv4: 20 (MF)
		2. UDP: 8 (checksum 0)
		3. Payload: 4
- Expected packets:
	1. ICMP error
		1. IPv4: 20 (src: 198.51.100.1, dst: 198.51.100.2, !DF)
		2. ICMPv4: 8 (3/13)
		3. Payload: `test-1`
- Validation: `test-1` must yield `expected-1` regardless of `amend-udp-checksum-zero`.

!DF on `expected-1` is a Linux quirk. Perhaps that field should be ignored instead.

Again, I'm assuming the ICMP error from section 4.4 is supposed to kick in.

### Unknown Transport Protocols Tests

#### fa

- Requirement: Unknown transport protocols should be copied as-is.
- Helper packets:
	1. DCCP
		1. DCCP: 12
		2. Payload: 4
	1. Identical to `helper-1`, except for checksum
- Test packets:
	1. IPv4/DCCP packet
		1. IPv4: 20 (protocol 33)
		2. Payload: `helper-1`
	1. IPv6/DCCP packet
		1. IPv6: 40 (nexthdr 33)
		2. Payload: `helper-2`
- Expected packets:
	1. IPv6/DCCP packet
		1. IPv6: 40 (TTL-1, nexthdr 33, addresses swapped)
		2. Payload: `helper-1`
	2. IPv4/DCCP packet
		1. IPv4: 20 (TTL-1, protocol 33, !DF, addresses swapped)
		2. Payload: `helper-2`
- Validation: `test-n` must yield `expected-n`.

### Hairpinning Tests

#### ga

Hairpinning on SIIT is actually RFC 7757's realm. This test was adapted from [7757#appendix-B.1](https://tools.ietf.org/html/rfc7757#appendix-B.1).

- Requirement: Hairpinning blah blah whatever.
- Test packets:
	1. EAM -> pool6
		1. IPv6: 40 (src 2001:db8:3::8, dst 2001:db8:101:0:9::)
		2. UDP: 8
		3. Payload: 4
- Expected packets:
	1. pool6 -> EAM
		1. IPv6: 40 (src 2001:db8:101:0:8::, dst 2001:db8:3::9, TTL-1)
		2. UDP: 8
		3. Payload: 4
- Validation: `test-1` must yield `expected-1`.

### Address Translation Tests

#### ha

- Requirement: Jool should not drop an incoming ICMPv4 error because the source address is invalid.
- Validation: `hat<n>` must yield `hae<n>`.

	packet hat1
		20	IPv4	src:127.0.0.1
		8	ICMPv4
		20	IPv4	swap ttl--
		8	UDP
		4	Payload

	packet hat2
		20	IPv4	src:0.0.0.0
		8	ICMPv4
		20	IPv4	swap ttl--
		8	UDP
		4	Payload

	packet hae1
		40	IPv6	swap src:2001:db8:17f:0:1:: ttl--
		8	ICMPv6
		40	IPv6	ttl--
		8	UDP
		4	Payload

	packet hae2
		40	IPv6	swap src:2001:db8:100:: ttl--
		8	ICMPv6
		40	IPv6	ttl--
		8	UDP
		4	Payload

### ICMP Extension Tests

#### ia

- Requirement: Jool should treat ICMP extensions like opaque strings, but update the ICMP header length.
- Validations: `ia<n>t` must yield `ia<n>e`.

	packet ia1t: Small ICMPv6 error with ICMP extensions
		40	IPv6
		8	ICMPv6	type:1 code:4 length:19
		40	IPv6	ttl-- swap payloadLength:112
		20	TCP
		92	Payload
		20	Payload # ICMP Extension

	packet ia1e: Small ICMPv4 error with ICMP extensions
		20	IPv4	ttl-- !df swap
		8	ICMPv4	type:3 code:3 length:33
		20	IPv4	ttl-- !df totalLength:132
		20	TCP
		92	Payload
		20	Payload # ICMP extension

	packet ia2t: Small ICMPv4 error with ICMP extensions
		20	IPv4
		8	ICMPv4	type:3 code:3 length:33
		20	IPv4	ttl-- swap totalLength:132
		20	TCP
		92	Payload
		20	Payload # ICMP Extension

	packet ia2e: Small ICMPv6 error with ICMP extensions
		40	IPv6	ttl-- swap
		8	ICMPv6	type:1 code:4 length:19
		40	IPv6	ttl-- payloadLength:112
		20	TCP
		92	Payload
		20	Payload # ICMP extension

#### ib

This is not actually required by the RFC. It's just my common sense speaking.

- Requirement: Jool needs to adjust padding and length to make up for the difference between the ICMPv6 and ICMPv4 length units.
- Validation: `ib<n>t` must yield `ib<n>e`.

	packet ib1th
		40	IPv6	ttl-- swap
		20	TCP
		62	Payload

	packet ib1t: ICMPv6 error with ICMP extensions
		40	IPv6
		8	ICMPv6	type:1 code:4 length:16
		122	Payload	file:ib1th
		6	Padding
		20	Payload # ICMP extension

	packet ib1eh
		20	IPv4	!df ttl--
		20	TCP
		62	Payload

	packet ib1e: ICMPv4 error with ICMP extensions
		20	IPv4	!df ttl-- swap
		8	ICMPv4	type:3 code:3 length:32
		102	Payload	file:ib1eh
		26	Padding
		20	Payload # ICMP extension

	packet ib2th
		20	IPv4	ttl-- swap
		20	TCP
		86	Payload

	packet ib2t: ICMPv4 error with ICMP extensions
		20	IPv4
		8	ICMPv4	type:3 code:3 length:32
		126	Payload	file:ib2th
		2	Padding
		20	Payload # ICMP extension

	packet ib2eh
		40	IPv6	ttl--
		20	TCP
		86	Payload

	packet ib2e: ICMPv6 error with ICMP extensions
		40	IPv6	ttl-- swap
		8	ICMPv6	type:1 code:4 length:18
		144	Payload	file:ib2eh
		20	Payload # ICMP extension

	packet ib3th
		20	IPv4	ttl-- swap
		20	TCP
		88	Payload

	packet ib3t: ICMPv4 error without padding, but internal packet length not multiple of 8
		20	IPv4
		8	ICMPv4	type:3 code:3 length:32
		128	Payload	file:ib3th
		20	Payload # ICMP extension

	packet ib3eh
		40	IPv6	ttl--
		20	TCP
		88	Payload

	packet ib3e: ICMPv6 error with padding
		40	IPv6	ttl-- swap
		8	ICMPv6	type:1 code:4 length:18
		144	Payload	file:ib3eh
		20	Payload # ICMP extension

#### ic

This is implicitly required by RFC 4884, not 7915.

Local glossary:

- EP (Essential Part): From an ICMP error, the external layer 3 headers and the first 128 bytes of the internal packet.
- OP (Optional Part): From an ICMP error, the remainder of the _internal packet_ (ie. ICMP extension excluded).
- IE (ICMP extension): That annoying thing defined by RFC 4884.

Requirements:

- If an ICMP error needs to be truncated to a specified length, the bytes need to be removed from the lowest priority component.
- The EP has priority over the IE.
- The IE has priority over the OP.
- The OP length must be maximized, but the OP can ultimately be trimmed to any >= 0 length to make room for higher-priority components.
- The IE cannot be partially trimmed. If it doesn't fit, it must be chopped off clean. (Along with the ICMP header length.)
- Resulting ICMPv4 error should not exceed "official" maximum length (576).
- Resulting ICMPv6 error should not exceed "official" maximum length (1280).
- Whatever happens, the ICMP header length must correctly reflect the inner packet length plus padding.

Tests:

(Inner packets need to be created separately because otherwise the UDP checksum extends to the IE)

##### ic1

OP barely within limits, IE preserved:

	- `helper-1a`:
		1. IPv6: 40 (TTL-1, swap addresses)
		2. UDP: 8
		3. Payload: 480
	- `helper-1b`:
		1. IPv4: 20 (TTL-1, !DF)
		2. UDP: 8
		3. Payload: 480
	- `test-1`: ICMPv6 error, OP almost large, IE small
		1. IPv6: 40
		2. ICMPv6: 8 (length 66)
		3. `helper-1a`
		4. ICMP extension: 40
	- `expected-1`: ICMPv4 error sized 576, OP intact, IE intact
		1. IPv4: 20 (TTL-1, !DF, swap addresses)
		2. ICMPv4: 8 (length 127)
		3. `helper-1b` (truncated: 508)
		4. ICMP extension: 40

##### ic2

OP barely truncated, IE preserved:

	- `helper-2a`:
		1. IPv6: 40 (TTL-1, swap addresses)
		2. UDP: 8
		3. Payload: 488
	- `helper-2b`:
		1. IPv4: 20 (TTL-1, !DF)
		2. UDP: 8
		3. Payload: 488
	- `test-2`: ICMPv6 error, OP large, IE small
		1. IPv6: 40
		2. ICMPv6: 8 (length 67)
		3. `helper-2a`
		4. ICMP extension: 40
	- `expected-2`: ICMPv4 error sized 576, OP truncated, IE intact
		1. IPv4: 20 (TTL-1, !DF, swap addresses)
		2. ICMPv4: 8 (length 127)
		3. `helper-2b` (truncated: 508)
		4. ICMP extension: 40

##### ic3

(This test is somewhat redundant) OP quite truncated, IE preserved:

	- `helper-3a`:
		1. IPv6: 40 (TTL-1, swap addresses)
		2. UDP: 8
		3. Payload: 1144
	- `helper-3b`:
		1. IPv4: 20 (TTL-1, !DF)
		2. UDP: 8
		3. Payload: 1144
	- `test-3`: ICMPv6 error, OP large, IE small
		1. IPv6: 40
		2. ICMPv6: 8 (length 149)
		3. `helper-3a`
		4. ICMP extension: 40
	- `expected-3`: ICMPv4 error sized 576, OP truncated, IE intact
		1. IPv4: 20 (TTL-1, !DF, swap addresses)
		2. ICMPv4: 8 (length 127)
		3. `helper-3b` (truncated: 508)
		4. ICMP extension: 40

##### ic4

OP preserved, IE barely within limits:

	packet ic4ha
		40	IPv6		ttl-- swap
		8	UDP
		80	Payload

	packet ic4hb
		20	IPv4		ttl-- !df
		8	UDP
		80	Payload

	packet ic4t: ICMPv6 error, OP small, IE almost large
		40	IPv6
		8	ICMPv6		length:16
		128	Payload		file:ic4ha
		420	Payload		# ICMP Extension

	packet ic4e: ICMPv4 error sized 576, OP intact, IE intact
		20	IPv4		ttl-- !df swap
		8	ICMPv4		length:32
		108	Payload		file:ic4hb
		20	Padding
		420	Payload		# ICMP Extension

##### ic5

OP preserved, IE too large:

	packet ic5t: ICMPv6 error, OP small, IE large
		40	IPv6
		8	ICMPv6		length:16
		128	Payload		file:ic4ha
		421	Payload		# ICMP Extension

	packet ic5e: ICMPv4 error, OP intact, IE chopped off
		20	IPv4		ttl-- !df swap
		8	ICMPv4
		108	Payload		file:ic4hb

##### ic6

OP and IE too large, so truncate OP and remove IE:

	- `helper-6a`:
		1. IPv6: 40 (TTL-1, swap addresses)
		2. UDP: 8
		3. Payload: 528
	- `helper-6b`:
		1. IPv4: 20 (TTL-1, !DF)
		2. UDP: 8
		3. Payload: 528
	- `test-6`: ICMPv6 error, OP large, IE large
		1. IPv6: 40
		2. ICMPv6: 8 (length 72)
		3. `helper-6a`
		4. ICMP extension: 521
	- `expected-6`: ICMPv4 error, OP trimmed, IE chopped off
		1. IPv4: 20 (TTL-1, !DF, swap addresses)
		2. ICMPv4: 8
		3. `helper-6b` (truncated: 548)

##### ic7

Invalidly small OP, salvaged for issue #396:

	packet ic7ha
		40	IPv6		ttl-- swap
		8	UDP

	packet ic7hb
		20	IPv4		ttl-- !df
		8	UDP

	packet ic7t: ICMPv6 error, OP small, IE whatever
		40	IPv6
		8	ICMPv6		length:6
		48	Payload		file:ic7ha
		40	Payload		# ICMP Extension

	packet ic7e: ICMPv4 error, padded OP, IE intact
		20	IPv4		ttl-- !df swap
		8	ICMPv4		length:32
		28	Payload		file:ic7hb
		100	Padding
		40	Payload		# ICMP Extension

--------------------------------

Variants to consider:

1. Inner packet's l3 length is much larger than what's actually present.
2. Padded packet?

#### id

Same as `ic`, except in the IPv4 -> IPv6 direction.

> Note: At some point during `ic`, "OP" evolved to mean "Internal Packet" instead of "Optional Part." By this point, this is in full effect.

- No OP truncation nor IE removal (Headers + OP + IE <= 1280)
	- [ida](#ida): Small OP and IE (< 1280)
	- [idb](#idb): Medium OP and IE (OP ~ IE, = 1280)
	- [idc](#idc): Large OP, small IE (= 1280)
	- [idd](#idd): Small OP, large IE (= 1280)
- OP truncation, no IE removal:
	- [ide](#ide): OP ~ IE
	- [idf](#idf): Minimal OP truncation (IE small)
	- [idg](#idg): Maximum OP truncation (IE large)
- IE removal, no OP truncation:
	- [idh](#idh): Small OP, large IE
	- [idi](#idi): Large OP, large IE
- Both OP truncation and IE removal:
	- [idj](#idj): Large OP, Large IE
- Invalidly small OP, salvaged for issue #396:
	- [idk](#idk): Invalidly small OP, IE whatever
- Leftovers:
	- [idz](#idz)
	- [idy](#idy)

The leftovers are old tests that don't fit in the redesign, but there aren't any reasons to delete them.

##### ida

- `idat`: OP almost large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 33)
	3. `idah1` (constraint: >= 128)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 104 (constraints: no padding, equals `idah2.Padding`)
	5. ICMP extension: 40
- `idae`: OP intact, IE intact
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 19)
	3. `idah2` (constraint: >= 128)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 104 (constraints: divisible by 8, equals `idah1.Padding`)
	4. ICMP extension: 40

##### idb

- `idbt`: ICMPv4 error, OP almost large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 143)
	3. `idbh1`
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 544
	4. ICMP extension: 640
- `idbe`: OP intact, IE intact (constraint: 1280)
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 74)
	3. `idbh2` (constraint: must not truncate)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 544
	4. ICMP extension: 640

##### idc

- `idct`
	1. IPv4: 20
	2. ICMPv4: 8 (length 255)
	3. `idch1` (constraint: as big as possible)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 992
	4. ICMP extension: 192 (constraint: same as `idce.ICMP extension`)
- `idce` (constraint: 1280)
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 130)
	3. `idch2` (constraint: as big as possible)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 992 (constraint: same as `idch1.Payload`)
	4. ICMP extension: 192 (constraint: as small as possible)

##### idd

- `iddt`
	1. IPv4: 20
	2. ICMPv4: 8 (length 33)
	3. `iddh1`
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 104 (constraint: no padding, same as `iddh2.Payload`.)
	5. ICMP extension: 1080 (constraint: same as `idde.Payload`)
- `idde` (constraint: 1280)
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 19)
	3. `iddh2` (constraint: as small as possible, must not be truncated)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 104 (constraints: divisible by 8, same as `iddh1.Payload`)
	4. ICMP extension: 1080 (constraint: as big as possible)

##### ide

- `idet`
	1. IPv4: 20
	2. ICMPv4: 8 (length 145)
	3. `ideh1`
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 552 (constraint: same as `ideh2.Payload`)
	4. ICMP extension: 640
- `idee` (constraint: 1280)
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 74)
	3. `ideh2` (truncated to 592) (constraint: must truncate)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 552 (constraint: divisible by 8, `ideh2` > 592)
	4. ICMP extension: 640

##### idf

This test is tight because the ICMPv4 header's length range prevents the IPv4 internal packet from exceeding 1020 bytes. This means we can't use extremely small IEs; it depends on the internal packet.

- `idft`: ICMPv4 error, OP large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 255)
	3. `idfh1` (constraints: length <= 1020, must be divisible by 4, must be as big as possible)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 992
	4. ICMP extension: 200 (constraint: must equal `idfe.ICMP extension`)
- `idfe`: ICMPv6 error sized 1280, OP truncated, IE intact
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 129)
	3. `idfh2` (truncated: 1032) (constraint: must truncate)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 992 (constraint: must equal `idfh1.Payload`)
	4. ICMP extension: 200 (constraint: must be as small as possible)

##### idg

- Environment: IPv4 MTU >= 2152 (constrained by `idgt`)
- `idgt`: ICMPv4 error, OP large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 255)
	3. `idgh1` (constraints: length <= 1020, must be divisible by 4, must be as big as possible)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 992
	4. ICMP extension: 1104 (constraint: must equal `idge.ICMP extension`)
- `idge`: ICMPv6 error sized 1280, OP truncated, IE intact
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 16)
	3. `idgh2` (truncated: 128) (constraint: maximize size)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 992 (constraint: must equal `idgh1.Payload`)
	4. ICMP extension: 1104

##### idh

- `idht`
	1. IPv4: 20
	2. ICMPv4: 8 (length 32)
	3. `idhh1` (constraint: 128)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 100
	4. ICMP extension: 1105 (constraint: too large)
- `idhe`
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8
	3. `idhh2`
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 100

##### idi

- Environment: IPv4 MTU >= 2153 (constrained by `idit`)
- `idit`
	1. IPv4: 20
	2. ICMPv4: 8 (length 255)
	3. `idih1` (constraint: <= 1020)
		1. IPv4: 20 (swap, ttl--)
		2. UDP: 8
		3. Payload: 992
	4. ICMP extension: 1105 (constraint: too large)
- `idie`
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8
	3. `idih2` (constraint: as big as possible)
		1. IPv6: 40 (ttl--)
		2. UDP: 8
		3. Payload: 992 (constraint: same as `idit.Payload`)

##### idj

This test cannot be done, because the ICMPv4 length does not have enough capacity to translate into an IPv6 internal packet that truncates in absence of an ICMPv6 Extension.

##### idk

	packet idkha
		20	IPv4		ttl-- swap
		8	UDP

	packet idkhb
		40	IPv6		ttl--
		8	UDP

	packet idkt: ICMPv4 error, OP small, IE whatever
		20	IPv4
		8	ICMPv4		length:7
		28	Payload		file:idkha
		40	Payload		# ICMP Extension

	packet idke: ICMPv6 error, padded OP, IE intact
		40	IPv6		swap ttl--
		8	ICMPv6		length:16
		48	Payload		file:idkhb
		80	Padding
		40	Payload		# ICMP Extension

##### idz

- Description: OP barely truncated, IE preserved:
- Point: Try minimal OP truncation
- `helper-2a`
	1. IPv4: 20 (swap, ttl--)
	2. UDP: 8
	3. Payload: 548
- `helper-2b`
	1. IPv6: 40 (ttl--)
	2. UDP: 8
	3. Payload: 548
- `test-2`: ICMPv4 error, OP large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 144)
	3. `helper-2a`
	4. ICMP extension: 640
- `expected-2`: ICMPv6 error sized 1280, OP truncated, IE intact
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 74)
	3. `helper-2b` (truncated: 592)
	4. ICMP extension: 640

##### idy

- Description: OP quite truncated, IE preserved.
- Point: Try maximum OP truncation
- Environment: IPv4 MTU >= 1708
- `helper-3a`
	1. IPv4: 20 (swap, ttl--)
	2. UDP: 8
	3. Payload: 992
- `helper-3b`
	1. IPv6: 40 (ttl--)
	2. UDP: 8
	3. Payload: 992
- `test-3`: ICMPv4 error, OP large, IE small
	1. IPv4: 20
	2. ICMPv4: 8 (length 255)
	3. `helper-3a`
	6. ICMP extension: 640
- `expected-3`: ICMPv6 error sized 1280, OP truncated, IE intact
	1. IPv6: 40 (swap, ttl--)
	2. ICMPv6: 8 (length 74)
	3. `helper-3b` (truncated: 592)
	4. ICMP extension: 640

### `lowest-ipv6-mtu` tests

Pseudocode:

	If the packet is an ICMP error:
		lowest-ipv6-mtu does nothing
		(but packet is still constrained to 1280 regardless of `lowest-ipv6-mtu`.)
	else if DF is enabled:
		lowest-ipv6-mtu does nothing
	else
		lowest-ipv6-mtu kicks in

Therefore, tests:

1. [ja](#ja): Raise `lowest-ipv6-mtu` to > 1280, send ICMPv4 error, make sure it does not exceed 1280.
2. Set `lowest-ipv6-mtu`=1280,
	1. [jba](#jba): DF=false, make sure large packet becomes fragments.
	2. [jbb](#jbb): DF=true, make sure large packet stays large.
2. Set `lowest-ipv6-mtu`=1402,
	1. [jca](#jca): DF=false, make sure large packet becomes fragments.
	2. [jcb](#jcb): DF=true, make sure large packet stays large.

`jc` tests purposes:

- Try a MTU value other than the extreme minimum.
- Try a MTU value that doesn't yield perfect fragmentation (as Fragment Offset forces l3 payload to length multiples of 8 bytes).

#### ja

	packet helper-jat: helper-jat's natural translation
		20	IPv4		swap ttl--
		8	UDP
		1205	Payload

	packet helper-jae: original packet
		40	IPv6		ttl--
		8	UDP
		1205	Payload

	packet jat: ICMP error to helper-jat, not truncated
		20	IPv4
		8	ICMPv4
		1233	Payload		file:helper-jat

	packet jae: translated jat, inner packet truncated
		40	IPv6		swap ttl--
		8	ICMPv6
		1232	Payload		file:helper-jae

- Environment: `lowest-ipv6-mtu` > 1280
- Validation: `jat` must yield `jae`.

#### jba

	packet jbah: jbae1 + jbae2, to compute UDP checksum
		40	IPv6		swap ttl--
		8	UDP
		1233	Payload

	packet jbat
		20	IPv4		 !df
		8	UDP
		1233	Payload

	packet jbae1: size 1280
		40	IPv6		swap ttl--
		8	Fragment	m:1
		8	UDP		checksum:30945 length:1241
		1224	Payload

	packet jbae2: remainder of jbae1
		40	IPv6		swap ttl--
		8	Fragment	fragmentOffset:154 nextHeader:17
		9	Payload		offset:200

- Environment: `lowest-ipv6-mtu` = 1280
- Validation: `jbat` must yield `jbae1` and `jbae2`.

#### jbb

	packet jbbt
		20	IPv4
		8	UDP
		1233	Payload

	packet jbbe
		40	IPv6		swap ttl--
		8	UDP
		1233	Payload

- Environment: `lowest-ipv6-mtu` = 1280
- Validation: `jbbt` must yield `jbbe`.

#### jca

	packet jcah: jcae1 + jcae2, to compute UDP checksum
		40	IPv6		swap ttl--
		8	UDP
		1355	Payload

	packet jcat
		20	IPv4		 !df
		8	UDP
		1355	Payload

	packet jcae1: size 1400
		40	IPv6		swap ttl--
		8	Fragment	m:1
		8	UDP		checksum:1722 length:1363
		1344	Payload		# Must length multiple of 8

	packet jcae2: remainder of jcae1
		40	IPv6		swap ttl--
		8	Fragment	fragmentOffset:169 nextHeader:17
		11	Payload		offset:64

- Environment: `lowest-ipv6-mtu` = 1402
- Validation: `jcat` must yield `jcae1` and `jcae2`.

#### jcb


	packet jcbt
		20	IPv4
		8	UDP
		1355	Payload

	packet jcbe
		40	IPv6		swap ttl--
		8	UDP
		1355	Payload

- Environment: `lowest-ipv6-mtu` = 1402
- Validation: `jcbt` must yield `jcbe`.
