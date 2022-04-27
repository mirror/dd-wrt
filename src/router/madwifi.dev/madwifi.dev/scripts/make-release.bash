#!/bin/bash
#
# Purpose: automate the process of tagging a release and packing a tarball
# for it.

# check if the script has been called directly
if [[ "$(basename $(pwd))" == "scripts" ]]; then
    echo
    echo "ERROR:"
    echo "Call this script via \"make release\" in the top-level directory of your"
    echo "working copy."
    echo
    exit 1
fi

# RELEASE_TMP and RELEASE_STORE are expected to be exported by make (from
# Makefile.inc)
if [[ "$RELEASE_TMP" == "" || "$RELEASE_STORE" == "" ]]; then
    echo
    echo "ERROR:"
    echo "RELEASE_TMP and/or RELEASE_STORE are not set. Check Makefile.inc and try again."
    echo
    exit 1
fi

# caller must have write access to the madwifi.org repository
valid=0
repos=$(svn info | grep "Repository Root" | cut -d" " -f3)
for f in ~/.subversion/auth/svn.simple/*; do
    if [[ "$(grep -c "$repos" $f)" != "0" ]]; then
	valid=1
	break
    fi
done

if [[ "$valid" != "1" ]]; then
    echo
    echo "WARNING:"
    echo "Write access to the repository is needed in order to successfully run this"
    echo "script."

    read -n1 -p "Do you want to continue? [yN] " choice
    if [[ "$choice" != "y" && "$choice" != "Y" ]]; then
	echo
        echo "Aborted."
	exit 1
    fi
fi


if [[ ! -d "$RELEASE_TMP" ]]; then
    echo
    echo "ERROR:"
    echo "RELEASE_TMP seems to be wrong. $RELEASE_TMP: no such directory"
    echo
    exit 1
fi

# check if we're in the top-level directory of the snapshot
if [[ ! -f ./release.h ]]; then
    echo
    echo "ERROR:"
    echo "It seems that the script has not been called with the top-level directory"
    echo "of the working copy as current working directory. This should not happen"
    echo "if you run \"make release\" in the top-level directory of the working"
    echo "copy."
    echo
    exit 1
fi

# this script does not work for tarball snapshots
svn info > /dev/null 2>&1 || {
    echo
    echo "ERROR:"
    echo "It seems this is no Subversion working copy of MadWifi. This script does"
    echo "not work in such cases."
    echo
    exit 1
}

# check if local working copy has uncommitted changes
if [[ ! -z "$(svn status)" ]]; then
    echo
    echo "ERROR:"
    echo "Your working copy has changes which are not yet committed."
    echo "Either commit or revert them before you continue to make a release."
    echo "Aborting for now."
    echo
    exit 1
fi

# make sure that the local working copy is in sync with the repository
repos=$(svn info | grep URL | cut -d" " -f2)
localrev=$(svn info | grep Last\ Changed\ Rev | cut -d" " -f4)
remoterev=$(svn log -r HEAD --quiet $repos | grep '^r[0-9]* ' | cut -d" " -f1 | cut -b2-)

if [[ "$localrev" != "$remoterev" ]]; then
    echo
    echo "ERROR:"
    echo "Your working copy is not in sync with the repository. Please update your"
    echo "working copy, then restart the release process."
    echo
    exit 1
fi




# ask developer about the version of the new release
reproot=$(svn info | grep URL | cut -d" " -f2 | cut -d"/" -f1-3)
latest=$(svn list $reproot/tags | grep -e "^release-" | cut -d"-" -f2 | cut -d"/" -f1 | sort | tail -n 1)

echo
if [[ ! -z "$latest" ]]; then
    major=$(echo $latest | cut -d"." -f1)
    minor=$(echo $latest | cut -d"." -f2)
    point=$(echo $latest | cut -d"." -f3)

    echo "The latest release is: $major.$minor.$point"
else
    latest="0.0.0"
    major="0"; minor="0"; point="0"
    
    echo "No releases yet."
fi

valid=0
while ! ((valid)); do
    echo
    echo "Please choose the release type:"
    echo " 1: major release (new version will be $((major+1)).0.0)"
    echo " 2: minor release (new version will be $major.$((minor+1)).0)"
    echo " 3: point release (new version will be $major.$minor.$((point+1)))"
    echo " 4: other (enter new version manually)"
    echo " 0: abort"
    echo

    read -n1 -p "Your choice: " choice
    case "$choice" in
    1) newmajor=$((major+1)); newminor=0; newpoint=0; valid=1 ;;
    2) newmajor=$major; newminor=$((minor+1)); newpoint=0; valid=1 ;;
    3) newmajor=$major; newminor=$minor; newpoint=$((point+1)); valid=1 ;;
    4)
	echo
	read -p "Enter release number (x.y.z): " newrelease
	if [[ "$(echo $newrelease | grep -c '^[0-9]*\.[0-9]*\.[0-9]*$')" == "1" ]]; then
	    newmajor=$(echo $newrelease | cut -d"." -f1)
	    newminor=$(echo $newrelease | cut -d"." -f2)
	    newpoint=$(echo $newrelease | cut -d"." -f3)
	    
	    if [[ "$(svn list $reproot/tags | grep -c ^release-$newmajor.$newminor.$newpoint)" != "0" ]]; then
		echo "Release $newmajor.$newminor.$newpoint already exists. Try again."
	    else
		valid=1
	    fi
	else
	    echo "Invalid release number."
	fi
	;;
    0)
	echo
	echo "Aborted."
	exit 0
	;;
	
    *)
	echo "Invalid."
	;;
    esac
done

# reassure that everything is correct
oldrelease="$major.$minor.$point"
newrelease="$newmajor.$newminor.$newpoint"

echo
echo "Last release: $oldrelease"
echo "New release : $newrelease"
echo

read -n1 -p "Is this correct? [yN] " choice
if [[ "$choice" != "y" && "$choice" != "Y" ]]; then
    echo
    echo "Aborted."
    exit 0
fi

echo; echo


# now we have every information we need, let's start the actual release process

# temporarily adjust release type in release.h
echo "temporarily adjusting release.h..."

mv release.h release.h.old
sed -e "/svnversion.h/d" \
    -e "/RELEASE_TYPE/ s/\".*\"/\"RELEASE\"/" \
    -e "/RELEASE_VERSION/ s/\".*\"/\"$newrelease\"/" release.h.old > release.h
rm -f release.h.old 


# tagging the release in the repository
echo "tagging release..."

msg="Tagging r$localrev as release $newrelease."
tag="release-$newrelease"
svn copy . $reproot/tags/release-$newrelease -m "$msg"

# revert local changes to release.h ...
echo "revert changes to release.h..."
svn revert release.h

# ... and modify the RELEASE_VERSION for trunk, too. We assume that the next
# release will be a point release.
echo "adjusting release.h in trunk..."
trunkrelease="$newmajor.$newminor.$((newpoint+1))"
mv release.h release.h.old
sed -e "/RELEASE_VERSION/ s/\"[0-9.]*\"/\"$trunkrelease\"/" release.h.old > release.h
rm -f release.h.old

msg="Adjust release version in response to release $newrelease."
svn commit release.h -m "$msg"
svn update -q

# create the tarball packaging directory
tmp=$RELEASE_TMP
store=$RELEASE_STORE

[[ -d $tmp/madwifi-release ]] || {
    echo "creating packaging directory..."
    mkdir $tmp/madwifi-release || exit 1
}

# remove old directories
rm -r $tmp/madwifi-release/* > /dev/null

# create tarball
echo "exporting new release from repository..."
pushd $tmp/madwifi-release
svn export $reproot/tags/release-$newrelease madwifi-$newrelease

echo "packing release tarball (.tar.gz)..."
tar cf - "madwifi-$newrelease" | gzip -f9 > "madwifi-$newrelease.tar.gz"

echo "packing release tarball (.tar.bz2)..."
tar cf - "madwifi-$newrelease" | bzip2 -f > "madwifi-$newrelease.tar.bz2"

echo "cleaning up..."
rm -r $tmp/madwifi-release/madwifi-$newrelease
popd > /dev/null
mv $tmp/madwifi-release/madwifi-$newrelease.tar.* $store

echo "done."
