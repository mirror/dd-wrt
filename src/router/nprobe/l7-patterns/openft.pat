# OpenFT - P2P filesharing (implemented in giFT library)
# Pattern quality: good fast
#
# Please post to l7-filter-developers@lists.sf.net as to whether it works 
# for you or not.  If you believe it could be improved please post your 
# suggestions to that list as well. You may subscribe to this list at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

# Ben Efros <ben AT xgendev.com> says:
# "This pattern identifies openFT P2P transfers fine.  openFT is part of giFT
# and is a pretty large p2p network. I would describe this pattern as pretty 
# weak, but it works for the giFT-based clients I've used."

openft
x-openftalias: [-)(0-9a-z ~.]
