#!/bin/sh

# Test for a autoconf commands.

# Test for a autoconf.

HAVE_AUTOCONF=false

for AUTOCONF in autoconf autoconf259; do
    if ${AUTOCONF} --version > /dev/null 2>&1; then
        AUTOCONF_VERSION=`${AUTOCONF} --version | sed 's/^[^0-9]*\([0-9.][0-9.]*\).*/\1/'`
#        echo ${AUTOCONF_VERSION}
        case ${AUTOCONF_VERSION} in
            2.59* | 2.6[0-9]* )
                HAVE_AUTOCONF=true
                break;
                ;;
        esac
    fi
done

# Test for a autoheader.

HAVE_AUTOHEADER=false

for AUTOHEADER in autoheader autoheader259; do
    if ${AUTOHEADER} --version > /dev/null 2>&1; then
        AUTOHEADER_VERSION=`${AUTOHEADER} --version | sed 's/^[^0-9]*\([0-9.][0-9.]*\).*/\1/'`
#        echo ${AUTOHEADER_VERSION}
        case ${AUTOHEADER_VERSION} in
            2.59* | 2.6[0-9]* )
                HAVE_AUTOHEADER=true
                break;
                ;;
        esac
    fi
done

# Test for a autoreconf.

HAVE_AUTORECONF=false

for AUTORECONF in autoreconf; do
    if ${AUTORECONF} --version > /dev/null 2>&1; then
        AUTORECONF_VERSION=`${AUTORECONF} --version | sed 's/^[^0-9]*\([0-9.][0-9.]*\).*/\1/'`
#        echo ${AUTORECONF_VERSION}
        case ${AUTORECONF_VERSION} in
            2.59* | 2.6[0-9]* )
                HAVE_AUTORECONF=true
                break;
                ;;
        esac
    fi
done

if test ${HAVE_AUTOCONF} = false; then
    echo "No proper autoconf was found."
    echo "You must have autoconf 2.59 or later installed."
    exit 1
fi

if test ${HAVE_AUTOHEADER} = false; then
    echo "No proper autoheader was found."
    echo "You must have autoconf 2.59 or later installed."
    exit 1
fi

if test ${HAVE_AUTORECONF} = false; then
    echo "No proper autoreconf was found."
    echo "You must have autoconf 2.59 or later installed."
    exit 1
fi


# Test for a automake commands.

# Test for a aclocal.

HAVE_ACLOCAL=false

for ACLOCAL in aclocal aclocal-1.10 aclocal-1.9 aclocal19; do
    if ${ACLOCAL} --version > /dev/null 2>&1; then
        ACLOCAL_VERSION=`${ACLOCAL} --version | sed 's/^[^0-9]*\([0-9.][0-9.]*\).*/\1/'`
#        echo ${ACLOCAL_VERSION}
        case ${ACLOCAL_VERSION} in
            1.9* | 1.1[0-9]* )
                HAVE_ACLOCAL=true
                break;
                ;;
        esac
    fi
done

# Test for a automake.

HAVE_AUTOMAKE=false

for AUTOMAKE in automake automake-1.10 automake-1.9 automake19; do
    if ${AUTOMAKE} --version > /dev/null 2>&1; then
        AUTOMAKE_VERSION=`${AUTOMAKE} --version | sed 's/^[^0-9]*\([0-9.][0-9.]*\).*/\1/'`
#        echo ${AUTOMAKE_VERSION}
        case ${AUTOMAKE_VERSION} in
            1.9* | 1.1[0-9]* )
                HAVE_AUTOMAKE=true
                break;
                ;;
        esac
    fi
done

if test ${HAVE_ACLOCAL} = false; then
    echo "No proper aclocal was found."
    echo "You must have automake 1.9 or later installed."
    exit 1
fi

if test ${HAVE_AUTOMAKE} = false; then
    echo "No proper automake was found."
    echo "You must have automake 1.9 or later installed."
    exit 1
fi


# Test for a libtoolize.

HAVE_LIBTOOLIZE=false

for LIBTOOLIZE in libtoolize libtoolize15 glibtoolize; do
    if ${LIBTOOLIZE} --version > /dev/null 2>&1; then
        LIBTOOLIZE_VERSION=`${LIBTOOLIZE} --version | sed 's/^.*[^0-9.]\([0-9]\{1,\}\.[0-9.]\{1,\}\).*/\1/'`
#        echo ${LIBTOOLIZE_VERSION}
        case ${LIBTOOLIZE_VERSION} in
            1.5* | 2.* )
                HAVE_LIBTOOLIZE=true
                break;
                ;;
        esac
    fi
done

if test ${HAVE_LIBTOOLIZE} = false; then
    echo "No proper libtoolize was found."
    echo "You must have libtool 1.5 or later installed."
    exit 1
fi


export ACLOCAL AUTOCONF AUTOHEADER AUTOMAKE LIBTOOLIZE

${AUTORECONF} --force --install --no-recursive
