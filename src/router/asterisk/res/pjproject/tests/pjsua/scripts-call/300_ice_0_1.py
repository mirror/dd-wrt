# $Id: 300_ice_0_1.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# ICE mismatch
test_param = TestParam(
		"Callee=no ICE, caller=use ICE",
		[
			InstanceParam("callee", "--null-audio --max-calls=1"),
			InstanceParam("caller", "--null-audio --use-ice --max-calls=1")
		]
		)
