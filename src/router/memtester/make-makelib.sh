echo 'main="$1"; shift'
echo 'rm -f "$main"'
echo 'ar cr "$main" ${1+"$@"}'

case "$1" in
sunos-5.*) ;;
unix_sv*) ;;
irix64-*) ;;
irix-*) ;;
dgux-*) ;;
hp-ux-*) ;;
sco*) ;;
*)
  echo 'ranlib "$main"'
  ;;
esac
