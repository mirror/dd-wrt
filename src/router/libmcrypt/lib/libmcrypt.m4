dnl Autoconf macros for libmcrypt
dnl $id$

# This script detects libmcrypt version and defines
# LIBMCRYPT_CFLAGS, LIBMCRYPT_LIBS
# and LIBMCRYPT24 or LIBMCRYPT22 depending on libmcrypt version
# found.

# Modified for LIBMCRYPT -- nmav
# Configure paths for LIBGCRYPT
# Shamelessly stolen from the one of XDELTA by Owen Taylor
# Werner Koch   99-12-09

dnl AM_PATH_LIBMCRYPT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libmcrypt, and define LIBMCRYPT_CFLAGS and LIBMCRYPT_LIBS
dnl
AC_DEFUN([AM_PATH_LIBMCRYPT],
[dnl
dnl Get the cflags and libraries from the libmcrypt-config script
dnl
AC_ARG_WITH(libmcrypt-prefix,
          [  --with-libmcrypt-prefix=PFX   Prefix where libmcrypt is installed (optional)],
          libmcrypt_config_prefix="$withval", libmcrypt_config_prefix="")

  if test x$libmcrypt_config_prefix != x ; then
     libmcrypt_config_args="$libmcrypt_config_args --prefix=$libmcrypt_config_prefix"
     if test x${LIBMCRYPT_CONFIG+set} != xset ; then
        LIBMCRYPT_CONFIG=$libmcrypt_config_prefix/bin/libmcrypt-config
     fi
  fi

  AC_PATH_PROG(LIBMCRYPT_CONFIG, libmcrypt-config, no)
  min_libmcrypt_version=ifelse([$1], ,2.4.0,$1)
  AC_MSG_CHECKING(for libmcrypt - version >= $min_libmcrypt_version)
  no_libmcrypt=""
  if test "$LIBMCRYPT_CONFIG" = "no" ; then
dnl libmcrypt-config was not found (pre 2.4.11 versions)
dnl Try to detect libmcrypt version
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>

int
main ()
{
#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    return 0;
#else
/* version 2.4 */
    return 1;
#endif /* 19991015 */
}
],  libmcrypt_config_version="2.2.0"
    if test x$libmcrypt_config_prefix != x ; then
	TTLIBS="-L${libmcrypt_config_prefix}/libs"
	TTINCLUDE="-I${libmcrypt_config_prefix}/include"
    fi
    LIBMCRYPT_CFLAGS="${TTINCLUDE}"
    LIBMCRYPT_LIBS="${TTLIBS} -lmcrypt"
    AC_DEFINE(LIBMCRYPT22, 1, [have libmcrypt 2.2])

,   libmcrypt_config_version="2.4.0"
    if test x$libmcrypt_config_prefix != x ; then
	TTLIBS="-L${libmcrypt_config_prefix}/libs"
	TTINCLUDE="-I${libmcrypt_config_prefix}/include"
    fi
    LIBMCRYPT_CFLAGS="${TTINCLUDE}"
    LIBMCRYPT_LIBS="${TTLIBS} -lmcrypt -lltdl ${LIBADD_DL}"
    AC_DEFINE(LIBMCRYPT24, 1, [have libmcrypt 2.4]))
  else
dnl libmcrypt-config was found
    LIBMCRYPT_CFLAGS=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --cflags`
    LIBMCRYPT_LIBS=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --libs`
    libmcrypt_config_version=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --version`
    AC_DEFINE(LIBMCRYPT24, 1, [have libmcrypt 2.4])
  fi

  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS $LIBMCRYPT_CFLAGS"
  LIBS="$LIBS $LIBMCRYPT_LIBS"

dnl
dnl Now check if the installed libmcrypt is sufficiently new. Also sanity
dnl checks the results of libmcrypt-config to some extent
dnl
      rm -f conf.libmcrypttest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>

#define TWO "2.2"

int
main ()
{
#if MCRYPT_API_VERSION <= 20010201

#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    int x = mcrypt_get_key_size(MCRYPT_TWOFISH_128);
    system ("touch conf.libmcrypttest");

    if( strncmp( TWO, "$min_libmcrypt_version", strlen(TWO))) {
      printf("\n*** Requested libmcrypt %s, but LIBMCRYPT (%s)\n",
             "$min_libmcrypt_version", TWO );
      printf("*** was found!\n"); 
      return 1;
    }
    return 0;
#else
/* version 2.4 before 11 */
    MCRYPT td = mcrypt_module_open("twofish", NULL, "cbc", NULL);
    system ("touch conf.libmcrypttest");
    mcrypt_module_close(td);

    return 0;
#endif /* 19991015 */

#else

    system ("touch conf.libmcrypttest");

    if( strcmp( mcrypt_check_version(NULL), "$libmcrypt_config_version" ) )
    {
      printf("\n*** 'libmcrypt-config --version' returned %s, but LIBMCRYPT (%s)\n",
             "$libmcrypt_config_version", mcrypt_check_version(NULL) );
      printf("*** was found! If libmcrypt-config was correct, then it is best\n");
      printf("*** to remove the old version of LIBMCRYPT. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libmcrypt-config was wrong, set the environment variable LIBMCRYPT_CONFIG\n");
      printf("*** to point to the correct copy of libmcrypt-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    }
    else if ( strcmp(mcrypt_check_version(NULL), LIBMCRYPT_VERSION ) )
    {
      printf("\n*** LIBMCRYPT header file (version %s) does not match\n", LIBMCRYPT_VERSION);
      printf("*** library (version %s)\n", mcrypt_check_version(NULL) );
    }
    else
    {
      if ( mcrypt_check_version( "$min_libmcrypt_version" ) )
      {
        return 0;
      }
     else
      {
        printf("no\n*** An old version of LIBMCRYPT (%s) was found.\n",
                mcrypt_check_version(NULL) );
        printf("*** You need a version of LIBMCRYPT newer than %s. The latest version of\n",
               "$min_libmcrypt_version" );
        printf("*** LIBMCRYPT is always available from ftp://mcrypt.hellug.gr/pub/mcrypt.\n");
        printf("*** \n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libmcrypt-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LIBMCRYPT, but you can also set the LIBMCRYPT_CONFIG environment to point to the\n");
        printf("*** correct copy of libmcrypt-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time)\n");
      }
    }
  return 1;

#endif /* 20010201 */

}
],, no_libmcrypt=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"


  if test "x$no_libmcrypt" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     if test -f conf.libmcrypttest ; then
        :
     else
        AC_MSG_RESULT(no)
     fi
     
     if test -f conf.libmcrypttest ; then
        :
     else
          echo "*** Could not run libmcrypt test program, checking why..."
          CFLAGS="$CFLAGS $LIBMCRYPT_CFLAGS"
          LIBS="$LIBS $LIBMCRYPT_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>
],      [ 
#if MCRYPT_API_VERSION <= 20010201

#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    int x = mcrypt_get_key_size(MCRYPT_TWOFISH_128);
    return 0;
#else
/* version 2.4 before 11 */
    MCRYPT td = mcrypt_module_open("twofish", NULL, "cbc", NULL);
    mcrypt_module_close(td);
    return 0;
#endif /* 19991015 */
#else

return !!mcrypt_check_version(NULL); 

#endif /* 20010201 */

],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LIBMCRYPT or finding the wrong"
          echo "*** version of LIBMCRYPT. If it is not finding LIBMCRYPT, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LIBMCRYPT was incorrectly installed"
          echo "*** or that you have moved LIBMCRYPT since it was installed. In the latter case, you"
          echo "*** may want to edit the libmcrypt-config script: $LIBMCRYPT_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
     fi
     
     LIBMCRYPT_CFLAGS=""
     LIBMCRYPT_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  rm -f conf.libmcrypttest
  AC_SUBST(LIBMCRYPT_CFLAGS)
  AC_SUBST(LIBMCRYPT_LIBS)
])

dnl *-*wedit:notab*-*  Please keep this as the last line.
