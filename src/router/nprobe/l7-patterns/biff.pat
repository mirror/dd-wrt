# Biff - new mail notification 
# Pattern quality: good veryfast undermatch overmatch
# Usually runs on port 512
#
# This pattern is untested.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers
biff
# This is a rare case where we will specify a $ (end of line), since
# this is the entirety of the communication.
# something that looks like a username, an @, a number.
# won't catch usernames that have strange characters in them.
^[a-z][a-z0-9]+@[1-9][0-9]+$
