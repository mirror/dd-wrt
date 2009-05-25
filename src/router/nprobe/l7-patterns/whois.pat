# Whois - query/response system, usually used for domain name info - RFC 3912
# Pattern quality: good fast overmatch
# Usually runs on TCP port 43
# 
# This pattern has been tested and is believed to work well.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

whois
# Matches the query.  Assumes only that it is printable ASCII without wierd
# whitespace.
^[ !-~]+\x0d\x0a$
