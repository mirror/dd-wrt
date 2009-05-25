# Shoutcast & Icecast - streaming audio
# Pattern quality: good fast
# usually runs on port 80
#
# Pattern contributed by Deepak Seshadri <dseshadri AT broadbandmaritime.com>
# who says "The difference between [Shoutcast and Icecast] is not
# clearly mentioned anywhere. According to this document, my pattern
# would filter JUST shoutcast packets."
#
# Should now match both Shoutcast and Icecast.  Tested with Winamp.
# 
# http://sander.vanzoest.com/talks/2002/audio_and_apache/
# http://forums.radiotoolbox.com/viewtopic.php?t=74
# www.icecast.org

# If you believe this pattern could be improved, please post to
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers


shoutcast
# This pattern was discovered using Ethereal.
icy [1-5][0-9][0-9] [\x09-\x0d -~]*(content-type:audio|icy-)
