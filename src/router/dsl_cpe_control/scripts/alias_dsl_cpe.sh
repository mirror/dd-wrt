#
# alias definitions for the "DSL CPE Control Application"
# to simplify the usage of the pipe interface
#

echo ... run alias defs for dsl_cpe_control application

if [ "$DSL_CPE_DIR" = "" ]; then
   DSL_CPE_DIR="/opt/ifx"
fi

# main alias to send commands
alias cpe="$DSL_CPE_DIR/dsl_cpe_pipe.sh"

# definitions for message-dumps and events
alias cpe_log_dump_cout='tail -f /tmp/pipe/dsl_cpe0_dump &'
alias cpe_log_dump='tail -f /tmp/pipe/dsl_cpe0_dump > dump.txt &'
alias cpe_log_event_cout='tail -f /tmp/pipe/dsl_cpe0_event &'
alias cpe_log_event='tail -f /tmp/pipe/dsl_cpe0_event > event.txt &'
