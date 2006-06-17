# Skype - UDP voice call (http://skype.com/)
# Pattern quality: ok
#
# By Myles Uyema <mylesuyema AT gmail.com>
#
# The 0d 6b pattern matches SkypeOut calls to real phone numbers
# The 0d 01 pattern matches Skype-to-Skype voice calls
# This has been verified by at least one person other than myself
# although the optimization of the pattern could still be tweaked.
#
# If you believe this pattern could be improved, please post to 
# l7-filter-developers@lists.sf.net .  Subscribe at 
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

skype
^..\x0d\x6b..\x0d\x6b..\x0d\x6b$|..\x0d\x01..\x0d\x01$

