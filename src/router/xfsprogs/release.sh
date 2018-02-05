#!/bin/sh
#
# Automate generation of a new release
#
# Need to first update these files:
#
# VERSION (with new version number)
# docs/CHANGES (with changelog and version/date string)
# configure.ac (with new version string)
# debian/changelog (with new release entry, only for release version)

. ./VERSION

version=${PKG_MAJOR}.${PKG_MINOR}.${PKG_REVISION}
date=`date +"%-d %B %Y"`

echo "Cleaning up"
make realclean

echo "Updating CHANGES"
sed -e "s/${version}.*/${version} (${date})/" doc/CHANGES > doc/CHANGES.tmp && \
	mv doc/CHANGES.tmp doc/CHANGES

echo "Commiting CHANGES update to git"
git commit -a -m "${version} release"

echo "Tagging git repository"
git tag -a -m "${version} release" v${version}

echo "Making source tarball"
make dist

#echo "Sign the source tarball"
#gpg --detach-sign xfsprogs-${version}.tar.gz

echo "Done.  Please remember to push out tags using \"git push --tags\""
