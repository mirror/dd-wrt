# Gopher - A precursor to HTTP - RFC 1436
# Pattern quality: good notsofast undermatch
# Usually runs on port 70
#
# This pattern is untested.  If it does not
# work for you, or you believe it could be improved, please post to 
# l7-filter-developers@lists.sf.net .  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

gopher
# This matches the server's response, but naturally only if it is a
# directory listing, not if it is sending a file, because then the data 
# is totally arbitrary.

# Matches one of the file type characters, any characters, a tab, any 
# characters, a tab something that has at least one letter (maybe something 
# else), then a dot and at least two letters for a TLD (see dns.pat), a tab 
# and then a number which could be the start of a port number.
# i.e. "0About internet Gopher\tStuff:About us\trawBits.micro.umn.edu\t70"
#^[1-9,\+TgI].*\x09.*\x09.*[a-z].*\..*[a-z][a-z]\x09[1-9]

# The above is very very VERY slow with our current regexp implementation.
# This one won't bring your machine down:
^[1-9,+tgi][\x09-\x0d -~]*\x09[\x09-\x0d -~]*\x09[a-z0-9.]*\.[a-z][a-z].?.?\x09[1-9]
