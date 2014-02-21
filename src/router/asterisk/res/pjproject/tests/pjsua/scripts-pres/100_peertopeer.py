# $Id: 100_peertopeer.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# Direct peer to peer presence
test_param = TestParam(
		"Direct peer to peer presence",
		[
			InstanceParam("client1", "--null-audio"),
			InstanceParam("client2", "--null-audio")
		]
		)
