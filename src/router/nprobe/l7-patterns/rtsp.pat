# RTSP - Real Time Streaming Protocol - http://www.rtsp.org
# Pattern quality: good fast
# usually runs on port 554
#
# To take full advantage of this pattern, please see the RTSP connection 
# tracking patch to the Linux kernel referenced at the above site.
#
# This pattern has been tested and is believed to work well.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers
rtsp
rtsp/1.0 200 ok
