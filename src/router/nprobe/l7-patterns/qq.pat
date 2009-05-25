# Tencent QQ Protocol - Chinese instant messenger protocol - http://www.qq.com
# Pattern quality: good veryfast
#
# Over six million people use QQ in China, according to wsgtrsys.
# 
# This pattern has been tested and is believed to work well. If this
# pattern does not work for you, or you believe it could be improved,
# please post to l7-filter-developers@lists.sf.net .  This list may be
# subscribed to at lists.sourceforge.net/lists/listinfo/l7-filter-developers
#
# QQ uses three (two?) methods to connect to server(s?).
# one is udp, and another is tcp
# udp protocol: the first byte is 02 and last byte is 03
# tcp protocol: the second byte is 02 and last byte is 03
# pattern written by www.routerclub.com wsgtrsys

qq
^.?\x02.+\x03$

