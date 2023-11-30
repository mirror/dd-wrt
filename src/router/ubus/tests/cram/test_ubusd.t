set environment for convenience:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ alias ubusd='valgrind --quiet --leak-check=full ubusd'

check usage:

  $ ubusd -h
  ubusd: invalid option -- 'h'
  Usage: ubusd [<options>]
  Options: 
    -A <path>:\t\tSet the path to ACL files (esc)
    -s <socket>:\t\tSet the unix domain socket to listen on (esc)
  
  [1]

  $ ubusd-san -h
  ubusd-san: invalid option -- 'h'
  Usage: ubusd-san [<options>]
  Options: 
    -A <path>:\t\tSet the path to ACL files (esc)
    -s <socket>:\t\tSet the unix domain socket to listen on (esc)
  
  [1]
