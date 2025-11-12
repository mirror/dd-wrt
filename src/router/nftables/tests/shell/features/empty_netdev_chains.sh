#!/bin/bash

# check if netdev chains survive without a single device

unshare -n bash -c "ip link add d0 type dummy; \
	$NFT \"table netdev t { \
		chain c { \
			type filter hook ingress priority 0; devices = { d0 }; \
		}; \
	}\"; \
	ip link del d0; \
	$NFT list chain netdev t c"
