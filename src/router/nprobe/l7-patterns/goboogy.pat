# GoBoogy - A Japanese (?) P2P protocol
# pattern quality: marginal notsofast
# 
# This pattern is untested and likely does not work in all cases!  If it
# does not work for you, or you believe it could be improved, please
# post to l7-filter-developers@lists.sf.net .  This list may be
# subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers
#
# By Adam Przybyla, modified by Matthew Strait.  Possibly lifted from 
# Josh Ballard (oofle.com).

goboogy
<peerplat>|^get /getfilebyhash\.cgi\?|^get /queue_register\.cgi\?|^get /getupdowninfo\.cgi\?
