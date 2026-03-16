#miniupnpc
AC_ARG_WITH(upnp,
[  --without-upnp          disable upnp support @<:@default=detect@:>@],
[UPNP=$withval])

no_upnp=t
if test x"${UPNP}" != xno; then
    AC_MSG_CHECKING([for miniupnpc headers])
    AC_TRY_COMPILE([#include <miniupnpc/miniupnpc.h>],
        [struct UPNPDev upnpdev;],
        [AC_MSG_RESULT(yes)
         upnphdr=t],
        [AC_MSG_RESULT(no)
         unset upnphdr])

    if test "x$upnphdr" = "xt"; then
	oLIBS=$LIBS
        AC_SEARCH_LIBS(UPNP_GetValidIGD, miniupnpc, [have_libminiupnp=t])
	UPNPLIB=$LIBS
	LIBS=$oLIBS
    fi
    if test x"${have_libminiupnp}" = xt; then
        AC_MSG_CHECKING([for miniupnpc version >= 2.2.8])
	AC_TRY_COMPILE([
            #include <stdio.h>
            #include <miniupnpc/miniupnpc.h>
            #include <miniupnpc/upnpcommands.h>
            #include <miniupnpc/upnperrors.h>], [

            #ifndef MINIUPNPC_API_VERSION
	    #error "no api version define"
            #else
            # if MINIUPNPC_API_VERSION < 18
            #error "api version too low"
            # endif
            #endif],
         [AC_MSG_RESULT(yes)
          AC_DEFINE(HAVE_LIBMINIUPNP, 1, [UPNP support library])
          AC_DEFINE(HAVE_LIBMINIUPNP228, 1, [UPNP support library 2.2.8])
          unset no_upnp
	  SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$UPNPLIB"
	  DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$UPNPLIB"],
         [AC_MSG_RESULT(no)])

        AC_MSG_CHECKING([for miniupnpc version >= 1.7])
	AC_TRY_COMPILE([
            #include <stdio.h>
            #include <miniupnpc/miniupnpc.h>
            #include <miniupnpc/upnpcommands.h>
            #include <miniupnpc/upnperrors.h>], [

            #ifndef MINIUPNPC_API_VERSION
	    #error "no api version define"
            #else
            # if MINIUPNPC_API_VERSION < 8 || MINIUPNPC_API_VERSION > 17
            #error "api version too low or high"
            # endif
            #endif],
         [AC_MSG_RESULT(yes)
          AC_DEFINE(HAVE_LIBMINIUPNP, 1, [UPNP support library])
          AC_DEFINE(HAVE_LIBMINIUPNP17, 1, [UPNP support library 1.7])
          unset no_upnp
	  SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$UPNPLIB"
	  DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$UPNPLIB"],
         [AC_MSG_RESULT(no)])

        AC_MSG_CHECKING([for miniupnpc version >= 1.4])
	AC_TRY_COMPILE([
            #include <stdio.h>
            #include <miniupnpc/miniupnpc.h>
            #include <miniupnpc/upnpcommands.h>
            #include <miniupnpc/upnperrors.h>], [

            char *str = NULL;
            struct UPNPDev *UPNPDev;
            struct UPNPUrls *UPNPUrls = NULL;
            struct IGDdatas *IGDdatas = NULL;

	    (void)sizeof(IGDdatas->CIF.servicetype);

            (void)UPNP_GetIGDFromUrl(str, UPNPUrls, IGDdatas, str, 0);
            (void)UPNP_GetValidIGD(UPNPDev, UPNPUrls, IGDdatas, str, 0);
            (void)UPNP_GetExternalIPAddress(str, str, str);
            (void)UPNP_AddPortMapping(str, str, str, str, str, str, str, str);
            (void)UPNP_DeletePortMapping(str, str, str, str, str);],
         [AC_MSG_RESULT(yes)
          AC_DEFINE(HAVE_LIBMINIUPNP, 1, [UPNP support library])
          AC_DEFINE(HAVE_LIBMINIUPNP14, 1, [UPNP support library 1.4])
          unset no_upnp
	  SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$UPNPLIB"
	  DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$UPNPLIB"],
         [AC_MSG_RESULT(no)])

	 if test x"${no_upnp}" != x; then
	         AC_MSG_CHECKING([for miniupnpc version 1.3])
		 AC_TRY_COMPILE([
                    #include <stdio.h>
                    #include <miniupnpc/miniupnpc.h>
                    #include <miniupnpc/upnpcommands.h>
                    #include <miniupnpc/upnperrors.h>], [

			 char *str = NULL;
			 struct UPNPDev *UPNPDev;
			 struct UPNPUrls *UPNPUrls = NULL;
			 struct IGDdatas *IGDdatas = NULL;

			 (void)strlen(IGDdatas->servicetype);

			 (void)UPNP_GetIGDFromUrl(str, UPNPUrls, IGDdatas, str, 0);
			 (void)UPNP_GetValidIGD(UPNPDev, UPNPUrls, IGDdatas, str, 0);
			 (void)UPNP_GetExternalIPAddress(str, str, str);
			 (void)UPNP_AddPortMapping(str, str, str, str, str, str, str, str);
			 (void)UPNP_DeletePortMapping(str, str, str, str, str);],
		 [AC_MSG_RESULT(yes)
		  AC_DEFINE(HAVE_LIBMINIUPNP, 1, [UPNP support library])
		  AC_DEFINE(HAVE_LIBMINIUPNP13, 1, [UPNP support library 1.3])
		  unset no_upnp
		  SOCKDDEPS="${SOCKDDEPS}${SOCKDDEPS:+ }$UPNPLIB"
		  DLIBDEPS="${DLIBDEPS}${DLIBDEPS:+ }$UPNPLIB"],
		 [AC_MSG_RESULT(no)])
	 fi
    fi
fi
