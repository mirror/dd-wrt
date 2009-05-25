# CVS - Concurrent Versions System
# Pattern quality: good veryfast
#
# Please post to l7-filter-developers@lists.sf.net as to whether this pattern 
# works for you or not.  If you believe it could be improved please post your 
# suggestions to that list as well. You may subscribe to this list at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

cvs

# Matches pserver login.  AUTH is for actually starting the protocol
# VERIFICATION is for authenticating without starting the protocols
# and GSSAPI is for using security services such as kerberos.
# http://www.loria.fr/~molli/cvs/doc/cvsclient_3.html

^BEGIN (AUTH|VERIFICATION|GSSAPI) REQUEST\x0a
