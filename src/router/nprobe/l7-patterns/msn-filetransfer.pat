# MSN Filetransfers - Filetransfers as used by MSN Messenger
# Pattern quality: good veryfast
#
# http://www.hypothetic.org/docs/msn/client/file_transfer.php
#
# This pattern has been tested and is believed to work well.  It, does,
# however, require more testing with various versions of the official
# MSN client as well as with clones such as Trillian, Miranda, Gaim,
# etc.  If you are using a MSN clone and this pattern DOES work for you,
# please, also let us know.
#
# Please post to l7-filter-developers@lists.sf.net as to whether it works
# for you or not.  If you believe it could be improved please post your
# suggestions to that list as well. You may subscribe to this list at
# http://lists.sourceforge.net/lists/listinfo/l7-filter-developers

# A MSN filetransfer is a normal MSN connection except that the protocol
# is MSNFTP. Some clients (especially Trillian) send other protocol versions
# besides MSNFTP which should be matched by the [ -~]*.

msn-filetransfer
^ver [ -~]*msnftp\x0d\x0aver msnftp\x0d\x0ausr

