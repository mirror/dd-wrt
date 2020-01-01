#!/bin/sh

if [[ ${1} = "-a" ]]; then
	aclocal
	libtoolize --automake --copy --force
	automake --add-missing --copy --force
	autoconf
fi


if [[ ${1} = "-r" ]]; then
	rm -f config.cache config.log
fi


CC="${ARCH}-linux-uclibc-gcc" CFLAGS="${2}" \
	glib_cv_prog_cc_ansi_proto=no \
	ac_cv_sizeof_char=1 \
	ac_cv_sizeof_short=2 \
	ac_cv_sizeof_long=4 \
	ac_cv_sizeof_void_p=4 \
	ac_cv_sizeof_int=4 \
	ac_cv_sizeof_long_long=8 \
	glib_cv_has__inline=yes \
	glib_cv_has__inline__=yes \
	glib_cv_hasinline=yes \
	glib_cv_sane_realloc=yes \
	glib_cv_va_copy=no \
	glib_cv___va_copy=yes \
	glib_cv_va_val_copy=yes \
	glib_cv_rtldglobal_broken=no \
	glib_cv_uscore=no \
	ac_cv_func_getpwuid_r=yes \
	glib_cv_func_pthread_mutex_trylock_posix=yes \
	glib_cv_func_pthread_cond_timedwait_posix=yes \
	glib_cv_sizeof_gmutex=24 \
	glib_cv_byte_contents_gmutex="0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" \
	./configure --prefix=${PWD}/../glib-1.2.10-install \
	--cache-file=config.cache --host=mipsel-linux \
	AR_FLAGS="cru ${3}" \
	RANLIB="${ARCH}-linux-ranlib ${3}"

