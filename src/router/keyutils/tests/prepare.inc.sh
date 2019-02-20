# preparation script for running keyring tests

# Find the relative path from pwd to the directory holding this file
includes=${BASH_SOURCE[0]}
includes=${includes%/*}/

# --- need to run in own session keyring
if [ "x`keyctl rdescribe @s | sed 's/.*;//'`" != "xRHTS/keyctl/$$" ]
then
    echo "Running with session keyring RHTS/keyctl/$$"
    exec keyctl session "RHTS/keyctl/$$" bash $0 $@ || exit 8
fi

# Set up for the Red Hat Test System
RUNNING_UNDER_RHTS=0
if [ -x /usr/bin/rhts_environment.sh ]
then
    PACKAGE=$(rpm -q --qf "%{name}" --whatprovides /bin/keyctl)
    . /usr/bin/rhts_environment.sh
    RUNNING_UNDER_RHTS=1
elif [ -z "$OUTPUTFILE" ]
then
    OUTPUTFILE=$PWD/test.out
    echo -n >$OUTPUTFILE
fi

case `lsb_release -i -s` in
    Fedora*)		OSDIST=Fedora;;
    RedHatEnterprise*)	OSDIST=RHEL;;
    *)			OSDIST=Unknown;;
esac

OSRELEASE=`lsb_release -r -s`

KEYUTILSVER=`keyctl --version 2>/dev/null`
if [ -n "$KEYUTILSVER" ]
then
    :
elif [ -x /bin/rpm ]
then
    KEYUTILSVER=`rpm -q keyutils`
else
    echo "Can't determine keyutils version" >&2
    exit 9
fi

echo "keyutils version: $KEYUTILSVER"
KEYUTILSVER=`expr "$KEYUTILSVER" : '.*keyutils-\([0-9.]*\).*'`

. $includes/version.inc.sh

KERNELVER=`uname -r`

#
# Make sure the TEST envvar is set.
#
if [ -z "$TEST" ]
then
    p=`pwd`
    case $p in
	*/keyctl/*)
	    TEST=keyctl/${p#*/keyctl/}
	    ;;
	*/bugzillas/*)
	    TEST=bugzillas/${p#*/bugzillas/}
	    ;;
	*)
	    TEST=unknown
	    ;;
    esac
fi

#
# Work out whether key invalidation is supported by the kernel
#
have_key_invalidate=0
if keyutils_at_or_later_than 1.5.6 && kernel_at_or_later_than 3.5-rc1
then
    have_key_invalidate=1
fi

#
# Work out whether the big_key type is supported by the kernel
#
have_big_key_type=0
if [ $OSDIST = RHEL ] && ! version_less_than $OSRELEASE 7
then
    # big_key is backported to 3.10 for RHEL-7
    have_big_key_type=1
elif kernel_at_or_later_than 3.13-rc1
then
    have_big_key_type=1
fi

#
# Work out whether Diffie-Hellman is supported by the kernel
#
have_dh_compute=0
if keyutils_at_or_later_than 1.5.10 && kernel_at_or_later_than 4.7-rc1
then
    have_dh_compute=1
fi

#
# Work out whether keyring restrictions are supported by the kernel
#
have_restrict_keyring=0
if keyutils_at_or_later_than 1.6 && kernel_at_or_later_than 4.12-rc1
then
    have_restrict_keyring=1
fi

#
# Check if skipping of tests requiring root was requested
#
skip_root_required=0
if [ "$SKIPROOTREQ" = "yes" ]
then
    skip_root_required=1
fi

#
# Check if skipping of tests requiring installation was requested
#
skip_install_required=0
if [ "$SKIPINSTALLREQ" = "yes" ]
then
    skip_install_required=1
fi
