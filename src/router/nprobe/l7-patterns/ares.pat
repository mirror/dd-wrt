# Ares - P2P filesharing - http://www.aresgalaxy.org
# Pattern quality: good veryfast undermatch

# This pattern catches only client-server connect messages.  This is
# sufficient for blocking, but not for shaping, since it doesn't catch 
# the actual file transfers (?). :-(

# Pattern by Brandon Enright <bmenrigh at the server known as ucsd.edu>

# This pattern has been tested.  If it does not work for you as
# advertised, or you believe it could be improved, please post to
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

ares

# old pattern: ^\x03\x5a.?.?\x05\x8d\x38
# 
# It has been brought to my attention that the second to last byte, \x8d
# may not actually be consistent.  This seems to be random or time
# based(?) further testing shows that the byte can be \x39, \x15, \x27,
# \x81 and many(?) others.  The following pattern should account for that.

^\x03\x5a.?.?\x05.?\x38
