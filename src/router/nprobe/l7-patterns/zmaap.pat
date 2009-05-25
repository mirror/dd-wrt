# ZMAAP - Zeroconf Multicast Address Allocation Protocol
# Pattern quality: ok veryfast
#
# http://files.zeroconf.org/draft-ietf-zeroconf-zmaap-02.txt
# (Note that this reference is an Internet-Draft, and therefore must
# be considered a work in progress.)
#
# This pattern is untested!
# Please post to l7-filter-developers@lists.sf.net as to whether it works 
# for you or not.  If you believe it could be improved please post your 
# suggestions to that list as well. You may subscribe to this list at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

zmaap
# - 4 byte magic number.
# - 1 byte version. Allow 1 & 2, even though only version 1 currently exists.
# - 1 byte message type,which is either 0 or 1
# - 1 byte address family.  L7-filter only works in IPv4, so this is 1.
^\x1b\xd7\x3b\x48[\x01\x02]\x01?\x01
