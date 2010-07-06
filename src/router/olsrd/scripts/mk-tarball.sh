#!/bin/sh
# 
# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2008, Hannes Gredler (hannes@gredler.at)
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
# 
# * Redistributions of source code must retain the above copyright 
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright 
#   notice, this list of conditions and the following disclaimer in 
#   the documentation and/or other materials provided with the 
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its 
#   contributors may be used to endorse or promote products derived 
#   from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
# 
# Visit http://www.olsr.org for more information.
# 
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
# 
#
# mk-tarball.sh 
# Create a release tarball based on the current VERS variable in the Makefile.
#

# first determine the tarball name
NAME=`grep -E "^VERS" ../Makefile | sed 's/^VERS..../olsrd-/;s/ *$//'`
#empty the directory in case it exists already
rm -rf /tmp/$NAME
mkdir /tmp/$NAME
# clean stuff up first
cd ..;make uberclean
# sync the stuff to a working directory
rsync -a . /tmp/$NAME/ --exclude=.project --exclude=.cproject --exclude=.settings --exclude=.hg* --exclude=.git* --exclude=*.rej --exclude=*.orig --delete
cd /tmp/
echo "### creating /tmp/$NAME.tar.gz"
tar -czf /tmp/$NAME.tar.gz $NAME
md5sum /tmp/$NAME.tar.gz
echo "### creating /tmp/$NAME.tar.bz2"
tar -cjf /tmp/$NAME.tar.bz2 $NAME
md5sum /tmp/$NAME.tar.bz2
#clean up
rm -rf /tmp/$NAME
