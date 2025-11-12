:output;type filter hook output priority 0

*ip;test-ip4;output
*ip6;test-ip6;output
*inet;test-inet;output
*arp;test-arp;output
*bridge;test-bridge;output

log;ok
log level emerg;ok
log level alert;ok
log level crit;ok
log level err;ok
log level warn;ok;log
log level notice;ok
log level info;ok
log level debug;ok
log level audit;ok

log level emerg group 2;fail
log level alert group 2 prefix "log test2";fail

# log level audit must reject all other parameters
log level audit prefix "foo";fail
log level audit group 42;fail
log level audit snaplen 23;fail
log level audit queue-threshold 1337;fail
log level audit flags all;fail

log prefix aaaaa-aaaaaa group 2 snaplen 33;ok;log prefix "aaaaa-aaaaaa" group 2 snaplen 33
# TODO: Add an exception: 'queue-threshold' attribute needs 'group' attribute
# The correct rule is log group 2 queue-threshold 2
log group 2 queue-threshold 2;ok
log group 2 snaplen 33;ok
log group 2 prefix "nft-test: ";ok;log prefix "nft-test: " group 2

log flags all;ok
log level debug flags ip options flags skuid;ok
log flags tcp sequence,options;ok
log flags ip options flags ether flags skuid flags tcp sequence,options;ok;log flags all
log flags all group 2;fail
