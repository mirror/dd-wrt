# Telnet - Insecure remote login - RFC 854
# Pattern quality: good veryfast
# Usually runs on port 23
#
# This pattern is lightly tested.  If it does not work for you, or you
# believe it could be improved, please post to
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

telnet
# Matches at least three IAC (Do|Will|Don't|Won't) commands in a row.  
# My telnet client sends 9 when I connect, so this should be fine.
# This pattern could fail on a unchatty connection or it could be 
# matched by something non-telnet spewing a lot of stuff in the fb-ff range.
^\xff[\xfb-\xfe].\xff[\xfb-\xfe].\xff[\xfb-\xfe]
