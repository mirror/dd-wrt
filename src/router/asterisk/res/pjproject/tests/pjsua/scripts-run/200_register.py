# $Id: 200_register.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# Basic registration
test_param = TestParam(
		"Basic registration",
		[
			InstanceParam(	"client", 
					"--null-audio"+
						" --id=\"<sip:test1@pjsip.org>\""+
						" --registrar=sip:sip.pjsip.org" +
						" --username=test1" +
						" --password=test1" +
						" --realm=*",
					uri="sip:test1@pjsip.org",
					have_reg=True),
		]
		)

