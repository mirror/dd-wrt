# $Id: 100_resample_lf_8_44.py 369517 2012-07-01 17:28:57Z file $
#
from inc_cfg import *

# simple test
test_param = TestParam(
		"Resample (large filter) 8 KHZ to 44 KHZ",
		[
			InstanceParam("endpt", "--null-audio --quality 10 --clock-rate 44100 --play-file wavs/input.8.wav --rec-file wavs/tmp.44.wav")
		]
		)
