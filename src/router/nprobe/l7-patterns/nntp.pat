# NNTP - Network News Transfer Protocol - RFCs 977 and 2980
# Pattern quality: good veryfast
# usually runs on port 119

# This pattern is tested and is believed to work well (but could use
# more testing).  If it does not work for you, or you believe it could
# be improved, please post to l7-filter-developers@lists.sf.net .  This
# list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

nntp
# matches authorized login
# OR 
# matches unauthorized login if the server says "news" after 200/201
# (Half of the 2 servers I tested did :-), but they both required authorization
# so it's quite possible that this pattern will miss some nntp traffic.)
^(20[01][\x09-\x0d -~]*AUTHINFO USER|20[01][\x09-\x0d -~]*news)

# same thing, slightly more accurate, but 100+ times slower
#^20[01][\x09-\x0d -~]*\x0d\x0a[\x09-\x0d -~]*AUTHINFO USER|20[01][\x09-\x0d -~]*news
