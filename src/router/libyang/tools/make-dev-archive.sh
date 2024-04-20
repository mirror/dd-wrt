#!/bin/bash
# create archive from current source using git

VERSION=$(git describe --tags --always)
# skip "v" from start of version number (if it exists) and replace - with .
VERSION=${VERSION#v}
VERSION=${VERSION//[-]/.}

NAMEVER=libyang-$VERSION
ARCHIVE=$NAMEVER.tar.gz

git archive --format tgz --output $ARCHIVE --prefix $NAMEVER/ HEAD

# apkg expects stdout to list archive files
echo $ARCHIVE
