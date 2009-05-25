# GnucleusLAN - LAN-only P2P filesharing
# Pattern quality: good fast
#
# This pattern has been tested and is believed to work well.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

gnucleuslan
gnuclear connect/[\x09-\x0d -~]*user-agent: gnucleus [\x09-\x0d -~]*lan:
