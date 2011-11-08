#!/bin/bash
#
# Automate generation a new release
#

. ./VERSION

version=${PKG_MAJOR}.${PKG_MINOR}.${PKG_REVISION}
date=`date +"%-d %B %Y"`

echo "Updating CHANGES"
sed -e "s/${version}.*/${version} (${date})/" doc/CHANGES > doc/CHANGES.tmp && \
	mv doc/CHANGES.tmp doc/CHANGES

echo "Commiting CHANGES update to git"
git commit -s -a -m "${version} release"

echo "Tagging git repository"
git tag -s -a -m "${version} release" v${version}

echo "Done.  Please remember to push out tags using \"git push --tags\""
echo "If you wish to create a source tarball, run \"make dist\""

