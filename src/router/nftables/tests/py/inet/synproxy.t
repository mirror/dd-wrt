:synproxychain;type filter hook input priority 0

*ip;synproxyip;synproxychain
*ip6;synproxyip6;synproxychain
*inet;synproxyinet;synproxychain

synproxy;ok
synproxy mss 1460 wscale 7;ok
synproxy mss 1460 wscale 5 timestamp sack-perm;ok
synproxy timestamp sack-perm;ok
synproxy timestamp;ok
synproxy sack-perm;ok

