#!/bin/sh -e

type="$1"
typecaps="$2"
bits="$3"
bytes=$(expr "$bits" / 8)

exec sed -e "s/@type@/$type/g; s/@TYPE@/$typecaps/g; s/@BITS@/$bits/g; s/@BYTES@/$bytes/g;"
