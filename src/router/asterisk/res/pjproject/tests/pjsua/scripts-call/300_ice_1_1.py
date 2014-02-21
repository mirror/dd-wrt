# $Id: 300_ice_1_1.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# ICE mismatch
test_param = TestParam(
		"Callee=use ICE, caller=use ICE",
		[
			InstanceParam("callee", "--null-audio --use-ice --max-calls=1", enable_buffer=True),
			InstanceParam("caller", "--null-audio --use-ice --max-calls=1", enable_buffer=True)
		]
		)
