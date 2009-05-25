# RDP - Remote Desktop Protocol (used in Windows Terminal Services)
# Pattern quality: ok fast
#
# This pattern was submitted by Michael Leong.  It has been tested under the 
# following conditions: "WinXP Pro with all the patches, rdesktop server 
# running on port 7000 instead of 3389 --> WinXP Pro Remote Desktop Client."
# Also tested is WinXP to Win 2000 Server.
# 
# Please report how this pattern works for you at 
# l7-filter-developers@lists.sf.net  This list may be subscribed to at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

rdp
rdpdr.*cliprdr.*rdpsnd

# Old pattern, submitted by Daniel Weatherford.
# rdpdr.*cliprdp.*rdpsnd 


