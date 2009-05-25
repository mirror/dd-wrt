# Soulseek - P2P filesharing - http://slsknet.org
# Pattern quality: good veryfast
# All my tests show that this pattern is fast, but one user has reported that
# it is slow.  Your milage may vary.

# This has been tested and works for "pierce firewall" commands and file
# transfers.  It does *not* match all the various sorts of chatter that go on, 
# such as searches, pings and whatnot.

# If it does not work for you, or you believe it could be improved,
# please post to l7-filter-developers@lists.sf.net .  This list may be
# subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

soulseek
# (Pierce firewall: in theory the token could be 4 bytes, but the last two 
# seem to always be zero.|download: Peer Init)
^(\x05..?|.\x01.[ -~]+\x01F..?.?.?.?.?.?.?)$
