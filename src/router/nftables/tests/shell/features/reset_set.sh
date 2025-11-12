#!/bin/bash

# 079cd633219d ("netfilter: nf_tables: Introduce NFT_MSG_GETSETELEM_RESET")
# v6.5-rc1~163^2~9^2~1

unshare -n bash -c "$NFT add table t; \
 $NFT add set t s { type ipv4_addr\; counter\; elements = { 127.0.0.1 counter packets 1 bytes 2 } } ; \
 $NFT reset set t s ; \
 $NFT reset set t s | grep counter\ packets\ 0\ bytes\ 0
"
