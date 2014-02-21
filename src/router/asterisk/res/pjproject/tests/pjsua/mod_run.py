# $Id: mod_run.py 369517 2012-07-01 17:28:57Z file $
import imp
import sys

from inc_cfg import *

# Read configuration
cfg_file = imp.load_source("cfg_file", ARGS[1])

# Here where it all comes together
test = cfg_file.test_param
