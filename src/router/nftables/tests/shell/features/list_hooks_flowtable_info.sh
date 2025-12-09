#!/bin/sh

# check for flowtable info in 'list hooks' output

unshare -n bash -c " \
$NFT \"table inet t { flowtable ft { hook ingress priority 0; devices = { lo }; }; }\"; \
$NFT list hooks netdev device lo | grep -q flowtable\ inet\ t\ ft"
