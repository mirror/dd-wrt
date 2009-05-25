# MUTE - P2P filesharing - http://mute-net.sourceforge.net
# Pattern quality: marginal veryfast
#
# This pattern is lightly tested.  I don't know for sure that it will 
# match the actual file transfers.
#
# Please post to l7-filter-developers@lists.sf.net as to whether it works 
# for you or not.  If you believe it could be improved please post your 
# suggestions to that list as well. You may subscribe to this list at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

mute
^(Public|AES)Key: [0-9a-f]*\x0aEnd(Public|AES)Key\x0a$
