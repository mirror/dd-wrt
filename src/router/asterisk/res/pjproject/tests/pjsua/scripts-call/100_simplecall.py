# $Id: 100_simplecall.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# Simple call
test_param = TestParam(
		"Basic call",
		[
			InstanceParam("callee", "--null-audio --max-calls=1"),
			InstanceParam("caller", "--null-audio --max-calls=1")
		]
		)
