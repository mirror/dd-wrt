#!/bin/sh -e

sysdeps="$1"
bits="$2"
dfmt="$3"
ofmt="$4"
xfmt="$5"
bfmt="$6"

gen_bits() {
  sed -e "s/@BITS@/$1/g; s/@DFMT@/$2/g; s/@OFMT@/$3/g; s/@XFMT@/$4/g; s/@BFMT@/$5/g;" < src/headers/bits-template
}

tools/gen-types-internal.sh "" "" "$bits" < src/headers/bits-header

if test "$bits" = 64 ; then
  cat src/headers/uint64-defs
  if grep -qF 'uint64t: no' "$sysdeps" ; then
    if grep -qF 'sizeofulong: 8' "$sysdeps" ; then
      cat src/headers/uint64-ulong64
    else
      cat src/headers/uint64-noulong64
    fi
    cat src/headers/uint64-macros
  fi
else
  cat src/headers/uint64-include
fi

if grep -qF 'endianness: little' < "$sysdeps" ; then
  endian=l
elif grep -qF 'endianness: big' < "$sysdeps" ; then
  endian=b
else
  echo 'Error ! Unsupported endianness' 1>&2
  ./crash
fi

cat "src/headers/uint${bits}-bswap"
tools/gen-types-internal.sh "" "" "$bits" < src/headers/bits-${endian}endian
gen_bits "$bits" "$dfmt" "$ofmt" "$xfmt" "$bfmt"
exec tools/gen-types-internal.sh "" "" "$bits" < src/headers/bits-footer
