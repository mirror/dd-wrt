+=====================================================================+
|        Link Quality Fish Eye Mechanism   December 3th 2005          |
|								      |	
|	     	 	    olsrd-0.4.10			      |
+=====================================================================+

	Corinna 'Elektra' Aichele   (onelektra at gmx.net)

---------------
I. Introduction
---------------

Link Quality Fish Eye is a new (experimental) algorithm introduced in
olsrd 0.4.10. To increase stability in a mesh, TC messages should be
sent quite frequently. However, the network would then suffer from the
resulting overhead. The idea is to frequently send TC messages to
adjacent nodes, i.e. nodes that are likely to be involved in routing
loops, without flooding the whole mesh with each sent TC message.

OLSR packets carry a Time To Live (TTL) that specifies the maximal
number of hops that the packets is allowed to travel in the mesh. The
Link Quality Fish Eye mechanism generates TC messages not only with
the default TTL of 255, but with different TTLs, namely 1, 2, 3, and
255, restricting the distribution of TC messages to nodes 1, 2, 3, and
255 hops away. A TC message with a TTL of 1 will just travel to all
one-hop neighbours, a message with a TTL of 2 will in addition reach
all two-hop neighbours, etc.

TC messages with small TTLs are sent more frequently than TC messages
with higher TTLs, such that immediate neighbours are more up to date
with respect to our links than the rest of the mesh. We hope that this
reduces the likelihood of routing loops.

--------------
II. How to use
--------------

The Fish Eye algorithm can be enabled in the configuration file
/etc/olsrd/olsrd.conf with the following lines:

	# Fish Eye mechanism for TC messages 0 = off, 1 = on

	LinkQualityFishEye 1

Fish Eye should be used together with a small TcInterval setting as
follows:

	# TC interval in seconds (float)

        TcInterval 0.5

If olsrd is started with debug-level 3 it will print out a message
every time a TC message is issued or dropped.

The following sequence of TTL values is used by olsrd.

        255 3 2 1 2 1 1 3 2 1 2 1 1

Hence, a TC interval of 0.5 seconds leads to the following TC
broadcast scheme.

  * Out of 13 TC messages, all 13 are seen by one-hop neighbours (TTL
    1, 2, 3, or 255), i.e. a one-hop neighbour sees a TC message every
    0.5 seconds.

  * Two-hop neighbours (TTL 2, 3, or 255) see 7 out of 13 TC messages,
    i.e. about one message per 0.9 seconds.

  * Three-hop neighbours (TTL 3 or 255) see 3 out of 13 TC messages,
    i.e. about one message per 2.2 seconds.

  * All other nodes in the mesh (TTL 255) see 1 out of 13 TC messages,
    i.e. one message per 6.5 seconds.

The sequence of TTL values is hardcoded in lq_packet.c and can be
altered easily for further experiments.

The Link Quality Fish Eye algorithm is compatible with earlier
versions of olsrd or nodes that do not have the Fish Eye feature
enabled.

A default configuration file with the Link Quality Fish Eye mechanism
and ETX enabled is located in ./files/olsrd.conf.default.lq-fisheye

---------------
III. Background
---------------

A major problem of a proactive routing algorithm is keeping topology
control information in sync. If topology information is not in sync
routing loops may occur. Usually routing loops happen in a local area
in the mesh - within a few hops.

It may happen that node A assumes that the best way to send packets to
node C is by forwarding them to node B, while node B thinks that the
best route to C is via node A. So A sends the packet to node B, and B
returns it to A - we have a loop. This can of course also happen with
more than two nodes involved in the loop, but it is unlikely that a
routing loop involves more than a few nodes.

Routing information like all data traffic gets lost on radio links
with weak signal-to-noise ratio (SNR) or if collisions occur. Setting
fragmentation and RTS (request-to-send) to conservative values on
wireless interfaces is a must in a mesh to deal with hidden nodes,
interference and collisions. A mesh that utilizes only one channel has
to deal with many collisions between packets and a lot of
self-generated interference. While a radio interface may have a range
of only 300 meters its signals can disturb receivers that are more
than 1000 meters away. The data traffic of a node in the distance is
not readable, but it's signals add to the noise floor in receivers,
reducing signal-to-noise ratio of local links.

When a route is saturated with traffic the transmitters involved
introduce permanent interference into the mesh, causing packet loss on
other links in the neighbourhood. OLSR messages get lost, causing
confusion. While the timeout values of MID messages or HNA messages
can be increased to increase stability without a big tradeoff, TC
messages that are up to date and arrive in time are critical. It is
also critical to have MPR information in sync if the MPR algorithm is
used, but in the author's opinion this optimization doesn't do any
good anyway. The MPR algorithm introduces a new source of failure and
reduces TC message redundancy, so it should be switched off in the
configuration file /etc/olsrd/olsrd.conf with these lines:

        TcRedundancy 2
        MprCoverage  7

olsrd with LQ Extension attempts to know the best routes all over the
whole mesh cloud, but it is likely that it never will be able to
achieve this in a mesh that has more than a handful of nodes. TC
information is likely to be lost on its way through the whole
mesh. And this likelyhood increases with the number of hops.

But this fact doesn't necessarily harm the routing of traffic on a
long multihop path. A node at one end of a mesh cloud may have the
illusion to know the exact and best path along which its packets
travel when communicating with a node that is several hops away. But
this information may be pretty outdated and incomplete.

In fact all that the algorithm has to achieve is a reasonable choice
for the next two or three hops. If the routing path is 8 hops, for
example, nodes that are 5 hops away from the node initiating traffic
and closer to the destination have better and more accurate
information about the best path. They don't know what a node that
initiates traffic thinks about the path that its packets should take,
they have more accurate routing information and will look into their
routing table and make a choice based on their knowledge.

Someone that sends a snail mail parcel from Europe to India doesn't
have to write the name of the Indian postman on the paket that is
supposed to hand it over to the recipient or the brand of bicycle he
is supposed to ride when transporting the parcel. He doesn't have to
decide the path that the postman has to take in the recipient's
village. The postman knows better than the sender.

It should be sufficient if nodes have a vague idea about the topology
of the mesh in the distance and who is out there. If only a few TC
messages out of many TC packets that are broadcast make it over the
whole mesh this should be sufficient.

Have fun!

Elektra
