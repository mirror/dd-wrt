#!/bin/sh

# Detect JSON support. Note that $NFT may not be the binary from our build
# tree, hence we detect it by running the binary (instead of asking the build
# configuration).
$NFT -j list ruleset
