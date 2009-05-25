# IMAP - Internet Message Access Protocol (A common e-mail protocol)
# Pattern quality: good veryfast
# This matches IMAP4 (RFC 3501) and probably IMAP2 (RFC 1176)
#
# This pattern has been tested and is believed to work well.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers
# 
# This matches the IMAP welcome message or a noop command (which for 
# some unknown reason can happen at the start of a connection?)  
imap
^(\* ok|a[0-9]+ noop)

