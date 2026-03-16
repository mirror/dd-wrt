dnl generate symbol list to limit library symbol export

MAPFILE=socks.sym

STDDIRS="lib/" #socks shared/dynamic libraries (Rfoo functions)
for symdir in $STDDIRS; do
    cp /dev/null $symdir/$MAPFILE
    cat include/symbols_common.txt >> $symdir/$MAPFILE
done

PLDDIRS="dlib/" #preloading (standard, non-Rfoo functions)
if test x"$sol64" = xt; then
   PLDDIRS="$PLDDIRS dlib64/"
fi
for symdir in $PLDDIRS; do
    cat include/symbols_preload.txt >> $symdir/$MAPFILE

    #platform dependent symbols
    case $host in
	*-*-darwin*)
	    cat include/symbols_darwin.txt >> $symdir/$MAPFILE
	    ;;

	*-*-linux-*)
	    if test x"${stdio_preload}" = xt; then
		cat include/symbols_glibc.txt >> $symdir/$MAPFILE
	    fi
	    ;;

       *-*-solaris*)
	    cat include/symbols_osol.txt >> $symdir/$MAPFILE
	    if test x"${stdio_preload}" = xt; then
		cat include/symbols_osol_stdio.txt >> $symdir/$MAPFILE
	    fi
	    ;;

	*-*-freebsd*)
	    cat include/symbols_freebsd.txt >> $symdir/$MAPFILE
	    ;;
    esac
done

MAPOPT="-export-symbols socks.sym"
AC_SUBST(MAPOPT)
