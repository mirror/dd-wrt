# $Id: 100_simple.py 369517 2012-07-01 17:28:57Z file $
#
# Just about the simple pjsua command line parameter, which should
# never fail in any circumstances
from inc_cfg import *

test_param = TestParam(
		"Basic run", 
		[
			InstanceParam("pjsua", "--null-audio --rtp-port 0")
		]
		)

