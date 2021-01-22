#!/bin/sh

case "$1" in
osf1-*) 
  # OSF/1 (Tru64) needs /usr/lib/librt.a for mlock()
  echo /usr/lib/librt.a
  ;;
unix_sv*) ;;
irix64-*) ;;
irix-*) ;;
dgux-*) ;;
hp-ux-*) ;;
sco*) ;;
*)
  ;;
esac
