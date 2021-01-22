echo 'main="$1"; shift'
echo exec "$CC $COPTS"  '-DNEED_PRINTF -o "$main" "$main".o ${1+"$@"}'
