Introduction
------------
Tagging packets is a way to continue logging packets from a session or host
that generated an event in Snort.  When an event is generated based on a rule
that contains a tag option, information such as the IPs and ports involved, the
type of tagging decision that should be made (by session or host), for how long
to tag packets (the number of packets, seconds and/or bytes), the event id of
the packet that generated the alert (to be included in the logging information
with each tagged packet), etc. are saved into a data structure so that
subsequent packets can be checked against this information and a decision can
be made whether or not to tag/log the packet.  Tagged traffic is logged to
allow analysis of response codes and post-attack traffic.  Tag alerts will be
sent to the same output plugins as the original alert, but it is the
responsibility of the output plugin to properly handle these special alerts.
Currently, the database output plugin does not properly handle tag alerts.

Snort will only check to see whether or not it should tag a packet if that
packet did not generate an event.  An exception to this is if the event was
based on a PASS rule and that rule does not contain a tag option, that packet
will be checked.


Format
------

tag: <type>, <count>, <metric>, [direction]

type
        session - Log packets in the session that set off the rule
        host    - Log packets from the host that caused the tag to activate
                  (uses [direction] modifier)

count - Count is specified as a number of units. Units are specified in the
        <metric> field.

metric
        packets - Tag the host/session for <count> packets
        seconds - Tag the host/session for <count> seconds
        bytes   - Tag the host/session for <count> bytes

direction - only relevant if host type is used.

	src - Tag packets containing the source IP address of the packet that
              generated the initial event.

	dst - Tag packets containing the destination IP address of the packet
              that generated the initial event.

Note that the stream preprocessor is not checked for the existence of a
session.  A session here is based only on socket (IP address:port) pairs, so
that a session could end, but if a new session is started using the same socket
pair, packets will continue to get tagged.

A tag option with the "host" type MUST specify a direction.

Tagged Packet Limit
-------------------
If you have a tag option in a rule that uses a metric other than packets, a
tagged_packet_limit will be used to limit the number of tagged packets
regardless of whether the seconds or bytes count has been reached. The default
tagged packet limit value is 256 and can be modified by using a config option
in your snort.conf file. You can disable this packet limit for a particular
rule by adding a packets metric to your tag option and setting its count to 0
(This can be done on a global scale by setting the tagged_packet_limit option
in snort.conf to 0). Doing this will ensure that packets are tagged for the
full amount of seconds or bytes and will not be cut off by the
tagged_packet_limit. (Note that the tagged_packet_limit was introduced to avoid
DoS situations on high bandwidth sensors for tag rules with a high seconds or
bytes counts.)

Example:

config tagged_packet_limit: 512


To disable the tagged_packet_limit:

config tagged_packet_limit: 0


Examples
--------

tag:host,100,seconds,src
tagged_packet_limit = 256

When an event is triggered on this rule, Snort will tag packets containing an
IP address that matches the source IP address of the packet that caused this
rule to alert for the next 100 seconds or 256 packets, whichever comes first.


tag:host,1000,bytes,100,packets,src
tagged_packet_limit = 256

When an event is triggered on this rule, Snort will tag packets containing an
IP address that matches the source IP address of the packet that caused this
rule to alert for the next 1000 bytes or 100 packets, whichever comes first.

NOTE: The tagged_packet_limit will be ignored whenever the packets metric is
used in the tag option.


Using multiple metrics
----------------------

When the metrics used are bytes and seconds, Snort will not stop tagging
packets until at least each of the counts for both metrics are reached.  Of
course, if you have not disabled the tagged_packet_limit, the packet limit will
take precedence.

The following tag option will tag relevant packets for at least 1000 bytes and
at least 100 seconds or until Snort has tagged 256 packets.

tag:host,1000,bytes,100,seconds,src
tagged_packet_limit = 256

To disable the tagged_packet_limit for this rule, it could be written as

tag:host,1000,bytes,100,seconds,0,packets,src

