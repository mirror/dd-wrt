#!/bin/bash
# create archive from current source using git

VERSION=$(git log --oneline -n1 --grep="^VERSION" | rev | cut -d' ' -f1 | rev)

NAMEVER=libyang-$VERSION
ARCHIVE=$NAMEVER.tar.gz

git archive --format tgz --output $ARCHIVE --prefix $NAMEVER/ HEAD
mkdir -p pkg/archives/dev/
mv $ARCHIVE pkg/archives/dev/

# apkg expects stdout to list archive files
echo pkg/archives/dev/$ARCHIVE
