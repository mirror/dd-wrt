# $Id: 350_prack_a.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# TCP call
test_param = TestParam(
		"Callee requires PRACK",
		[
			InstanceParam("callee", "--null-audio --max-calls=1 --use-100rel"),
			InstanceParam("caller", "--null-audio --max-calls=1")
		]
		)
