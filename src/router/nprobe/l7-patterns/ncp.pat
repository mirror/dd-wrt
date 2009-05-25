# NCP - Novell Core Protocol
# Pattern quality: good veryfast
#
# This pattern has been tested and is believed to work well.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

# ncp request
# dmdt means Request
#  *any length
#
#  *any reply buffer size
# "" means service request
#  | \x17\x17 means create a service connection
#  | uu means destroy service connection

# ncp reply
# tncp means reply
# 33 means service reply

ncp
^(dmdt.*\x01.*(""|\x11\x11|uu)|tncp.*33)
