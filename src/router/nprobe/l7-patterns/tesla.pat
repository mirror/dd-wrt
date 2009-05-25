# Tesla Advanced Communication - P2P filesharing (?)
# Pattern quality: marginal notsofast
#
# This pattern is untested!
# Please report on how this pattern works for you at
# l7-filter-developers@lists.sf.net .  If you can improve on this pattern, 
# please also post to that list. You may subscribe at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

# This is lifted from http://oofle.com/filesharing.php?app=tesla
# There is no explaination of what these numbers mean.
# The above page says that the first string is found only in TCP packets
# and the second only in UDP.

tesla
\x03\x9a\x89\x22\x31\x31\x31\.\x30\x30\x20\x42\x65\x74\x61\x20|\xe2\x3c\x69\x1e\x1c\xe9
