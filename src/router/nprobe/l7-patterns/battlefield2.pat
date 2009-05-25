# Battlefield 2 - An EA game.
# Pattern quality: ok notsofast
#
# This pattern is unconfirmed.
# Please post to l7-filter-developers@lists.sf.net as to whether it works
# for you or not.  If you believe it could be improved please post your
# suggestions to that list as well. You may subscribe to this list at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

battlefield2
# gameplay|account-login|server browsing/information
# See http://protocolinfo.org/wiki/Battlefield_2
# Can we put a ^ on the last branch?  If so, nosofast --> veryfast
^(\x11\x20\x01\xa0\x98\x11|\xfe\xfd.?.?.?.?.?.?(\x14\x01\x06|\xff\xff\xff))|[\x01\x5c].?battlefield2
