#!/bin/sh
#
# T.sh
#
# FIX  Short description.
#

. eval_tools.sh

HEADER a short description of your test

#------------------------------------ -o- 
# Test.
#

# Start the agent if needed (make sure it stop it below)
STARTAGENT

CAPTURE "<executable_with_arguments:_stores_stdout/stderr_for_use_later>"

CHECKEXACT "<string_to_look_for_an_exact_match_of_in_the_CAPTUREd_file_output>"
[ $? -eq 1 ]
FAILED $? "<diagnostic_label>"

#------------------------------------ -o- 
# Cleanup, exit.
#

# Stop the agent if necessary
STOPAGENT

FINISHED
