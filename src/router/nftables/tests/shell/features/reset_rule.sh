#!/bin/bash

# 8daa8fde3fc3 ("netfilter: nf_tables: Introduce NFT_MSG_GETRULE_RESET")
# v6.2-rc1~99^2~210^2~2

unshare -n bash -c "$NFT \"add table t; add chain t c ; add rule t c counter packets 1 bytes 42\"; \
$NFT reset rules chain t c ; \
$NFT reset rules chain t c |grep counter\ packets\ 0\ bytes\ 0"
