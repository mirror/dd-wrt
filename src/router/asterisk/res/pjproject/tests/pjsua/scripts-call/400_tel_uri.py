# $Id: 400_tel_uri.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# Simple call
test_param = TestParam(
		"tel: URI in From",
		[
			InstanceParam("callee", "--null-audio --max-calls=1 --id tel:+111"),
			InstanceParam("caller", "--null-audio --max-calls=1")
		]
		)
