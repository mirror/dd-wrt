# DHCP - Dynamic Host Configuration Protocol - RFC 1541
# Pattern quality: good veryfast
# Usually runs on ports 67 (server) and 68 (client)
#
# Also matches BOOTP (Bootstrap Protocol (RFC 951)) in the case that 
# the "vendor specific options" are used (these options were made standard
# for DHCP).
#
# This pattern is lightly tested.
# Please post to l7-filter-developers@lists.sf.net as to whether it works 
# for you or not.  If you believe it could be improved please post your 
# suggestions to that list as well. You may subscribe to this list at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

dhcp
^[\x01\x02][\x01- ]\x06.*c\x82sc

# Let's break that down:
#
# (\x01|\x02) is for BOOTREQUEST or BOOTREPLY
# Is there a demand for doing these seperately?  The Packeteer does.
#
# [\x01-\x20] is for any of the hardware address types listed at
# (http://www.iana.org/assignments/arp-parameters) and hopefully faster 
# ethernets too (100, 1000 and 10000mb) as well (do they share the 10mb 
# number?).
#
# \x06 for "hardware address length = 6 bytes".  Does anyone use other lengths
# these days?  If so, this pattern won't match it as it stands.
#
# .* covers the hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr, 
# chaddr, sname and file fields.  While this can't really be "any number
# of characters" long, it doesn't seem worth it to count.
# Can we make this more specific by restricting the number of hops or seconds?
#
# 0x63825363 is the "magic cookie" which begins the DHCP options field.
