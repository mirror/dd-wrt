# Finger - User information server - RFC 1288
# Pattern quality: good notsofast undermatch overmatch
# Usually runs on port 79
#
# This pattern is lightly tested.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

# If you really want to use this pattern, I would set it up so that
# it only gets consulted to confirm that traffic on port 79 is actually
# finger.

finger
# The first matches the client request, which should look like a username.
# The second matches the usual UNIX reply (but remember that they are
# allowed to say whatever they want)
^[a-z][a-z0-9\-_]+|login: [\x09-\x0d -~]* name: [\x09-\x0d -~]* Directory: 
