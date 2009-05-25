# X Windows Version 11 - Networked GUI system used in most Unices
# Pattern quality: good veryfast
# specification: http://www.msu.edu/~huntharo/xwin/docs/xwindows/PROTO.pdf
# Usually runs on port 6000 (6001 for the second server on a host, etc)
#
# This pattern has been tested.  If this pattern does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

x11
# 'l' = little-endian.  'B' = big endian
# ".?" is for the unused byte that comes next.  If it's a null, it won't appear.
# \x0b = protocol-major-version 11.
# For some reason, protocol-minor-version is 0, not 6, so can't match it.
# This pattern is too general. 
^[lb].?\x0b
