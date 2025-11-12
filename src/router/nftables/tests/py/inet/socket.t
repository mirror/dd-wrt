:sockchain;type filter hook prerouting priority -150

*ip;sockip4;sockchain
*ip6;sockip6;sockchain
*inet;sockin;sockchain

socket transparent 0;ok
socket transparent 1;ok
socket transparent 2;fail

socket mark 0x00000005;ok

socket wildcard 0;ok
socket wildcard 1;ok
socket wildcard 2;fail
