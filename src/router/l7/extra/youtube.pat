# HTTP - Youtube over HyperText Transfer Protocol (RFC 2616)
# Pattern attributes: good fast fast subset
# Protocol groups: streaming_video document_retrieval
# Wiki: none.
# Copyright (C) 2012 by "Porter" on http://linksysinfo.org/index.php?threads/qos-development-thread.31886/#post-204861
#
# Usually runs on port 80
#
#
# This pattern has been roughly tested and seems to work well. It's a Beta!
#
# To get or provide more information about this protocol and/or pattern:
# http://linksysinfo.org/index.php?threads/qos-development-thread.31886/#post-204861
#
# If you use this, you should be aware that:
#
# - It's a beta! Pattern overmatches slightly, meaning it not only matches youtube traffic.
#  The crossdomain-part matches small HTML documents.
# - Benchmarks show that this pattern is fast.
# - Obviously, since this is a subset of HTTP, you need to match it
#  earlier in your iptables rules than HTTP.
 
youtube
GET (\/videoplayback\?|\/crossdomain\.xml)
