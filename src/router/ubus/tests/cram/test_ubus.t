set environment for convenience:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ alias ubus='valgrind --quiet --leak-check=full ubus'

check usage:

  $ ubus
  Usage: ubus [<options>] <command> [arguments...]
  Options:
   -s <socket>:\t\tSet the unix domain socket to connect to (esc)
   -t <timeout>:\t\tSet the timeout (in seconds) for a command to complete (esc)
   -S:\t\t\tUse simplified output (for scripts) (esc)
   -v:\t\t\tMore verbose output (esc)
   -m <type>:\t\t(for monitor): include a specific message type (esc)
  \t\t\t(can be used more than once) (esc)
   -M <r|t>\t\t(for monitor): only capture received or transmitted traffic (esc)
  
  Commands:
   - list [<path>]\t\t\tList objects (esc)
   - call <path> <method> [<message>]\tCall an object method (esc)
   - subscribe <path> [<path>...]\tSubscribe to object(s) notifications (esc)
   - listen [<path>...]\t\t\tListen for events (esc)
   - send <type> [<message>]\t\tSend an event (esc)
   - wait_for <object> [<object>...]\tWait for multiple objects to appear on ubus (esc)
   - monitor\t\t\t\tMonitor ubus traffic (esc)
  
  [1]

  $ ubus-san
  Usage: ubus-san [<options>] <command> [arguments...]
  Options:
   -s <socket>:\t\tSet the unix domain socket to connect to (esc)
   -t <timeout>:\t\tSet the timeout (in seconds) for a command to complete (esc)
   -S:\t\t\tUse simplified output (for scripts) (esc)
   -v:\t\t\tMore verbose output (esc)
   -m <type>:\t\t(for monitor): include a specific message type (esc)
  \t\t\t(can be used more than once) (esc)
   -M <r|t>\t\t(for monitor): only capture received or transmitted traffic (esc)
  
  Commands:
   - list [<path>]\t\t\tList objects (esc)
   - call <path> <method> [<message>]\tCall an object method (esc)
   - subscribe <path> [<path>...]\tSubscribe to object(s) notifications (esc)
   - listen [<path>...]\t\t\tListen for events (esc)
   - send <type> [<message>]\t\tSend an event (esc)
   - wait_for <object> [<object>...]\tWait for multiple objects to appear on ubus (esc)
   - monitor\t\t\t\tMonitor ubus traffic (esc)
  
  [1]

check monitor command:

  $ ubus monitor
  Failed to connect to ubus
  [255]

  $ ubus-san monitor
  Failed to connect to ubus
  [255]
