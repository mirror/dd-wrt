# SIP - Internet telephony - RFC 3261
# Pattern quality: ok veryfast
#
# This pattern has been tested with the Ubiquity SIP user agent.
# 
# If this pattern does not work for you, or you believe it could be
# improved, please post to l7-filter-developers@lists.sf.net .  This
# list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers
#
# Thanks to Ankit Desai for this pattern.
#
# This pattern is based on SIP request format as per RFC 3261. I'm not
# sure about the version part. The RFC doesn't say anything about it, so
# I have allowed version ranging from 0.x to 2.x.

#Request-Line  =  Method SP Request-URI SP SIP-Version CRLF
sip
^(invite|register|cancel) sip[\x09-\x0d -~]*sip/[0-2]\.[0-9]
