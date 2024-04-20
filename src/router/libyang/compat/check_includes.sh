#!/bin/sh

RETVAL=0

# params - paths to the source files to search
SRC="$*"

# param FUNC - name of the function in compat to check
check_compat_func () {
	FILES=`grep -rE "([^[:alnum:]]|^)$1\([^\)]+\)" --include=\*.{c,h} $SRC | cut -d: -f1 | uniq`
	for f in $FILES; do
		grep -q "#include \"compat.h\"" $f
		if [ $? -ne 0 ]; then
			echo "Missing #include \"compat.h\" in file $f for function $1()"
			RETVAL=$((RETVAL+1))
		fi
	done
}

check_compat_macro () {
	FILES=`grep -rE "([^[:alnum:]]|^)$1([^[:alnum:]]|$)" --include=\*.{c,h} $SRC | cut -d: -f1 | uniq`
	for f in $FILES; do
		grep -q "#include \"compat.h\"" $f
		if [ $? -ne 0 ]; then
			echo "Missing #include \"compat.h\" in file $f for macro $1"
			RETVAL=$((RETVAL+1))
		fi
	done
}

check_compat_func vdprintf
check_compat_func asprintf
check_compat_func vasprintf
check_compat_func getline
check_compat_func strndup
check_compat_func strnstr
check_compat_func strdupa
check_compat_func strchrnul
check_compat_func get_current_dir_name
check_compat_func pthread_mutex_timedlock
check_compat_func UNUSED
check_compat_macro _PACKED

exit $RETVAL
