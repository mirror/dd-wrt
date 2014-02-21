# $Id: 200_tcp.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# TCP call
test_param = TestParam(
		"TCP transport",
		[
			InstanceParam("callee", "--null-audio --no-udp --max-calls=1", uri_param=";transport=tcp"),
			InstanceParam("caller", "--null-audio --no-udp --max-calls=1")
		]
		)
