#
# Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2006 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/Makefile#4 $
#

nop:

#
# Release-related support.
#
FILES=	COPYRIGHT README \
	ah.h ah_desc.h ah_devid.h ah_soc.h version.h \
	public/[a-z]*.inc public/*.hal.o.uu public/*.opt_ah.h \
	public/wackelf.c \
	${NULL}
#
# Tag the tree and build a release tarball suitable for open
# distribution (e.g. to sourceforge).  The tag is date-based.
#
release:
	DATE=`date +'%Y%m%d'`; TAG="ATH_HAL_$${DATE}"; \
	cvs tag -F $${TAG} ${FILES} && make rerelease TAG=$${TAG}

#
# Rebuild a release tarball using an existing tag.
#
rerelease:
	if [ -z "${TAG}" ]; then \
		echo "You must specify a TAG to do a re-release"; \
		exit 1; \
	fi; \
	expr "${TAG}" : '^ATH_HAL_' || { \
		echo "TAG must be of the form ATH_HAL_YYYYMMDD"; \
		exit 1; \
	}; \
	DATE=`echo "${TAG}" | sed 's/^ATH_HAL_//'`; \
	DIR="ath_hal-$${DATE}"; \
	rm -rf $${DIR}; \
	mkdir $${DIR} && \
	cvs export -d $${DIR} -r ${TAG} hal && \
	tar zcf $${DIR}.tgz --exclude=CVS $${DIR} && \
	rm -rf $${DIR}

#
# Build a release-like tarball suitable for open distribution
# using the current contents of the local directory.  This is
# useful for distributing private changes that are not committed
# to cvs.  Note that this should not be used to construct
# distributions as there is no cvs history or tag to use in
# tracking issues.
#
tarball:
	DATE=`date +'%Y%m%d'`; DIR="ath_hal-$${DATE}"; \
	ln -s . $${DIR}; \
	TARFILES=`for i in ${FILES}; do echo $${DIR}/$$i; done`; \
	tar zcf $${DIR}.tgz --exclude=CVS $${TARFILES}; \
	rm -f $${DIR};

#
# Build a source distribution for hal licensee's.  Be sure to
# first tag the source files using src_tag then follow this with
# src_release to construct the tarball.  Tags and filenames are
# constructed from the contents of version.h (as opposed to the
# current date) so be sure this file is up to date prior to rolling
# a release.  Note that a source release does NOT include the
# pre-built binary hal files; it is assumed the recipient can/will
# do this themselves.
#

# tag the source code according to the current version
src_tag:
	TAG=`awk '/ATH_HAL_VERSION/ \
		{ gsub("\\"","",$$3); \
		  gsub("[-.]", "_", $$3); print "v" $$3; }' version.h`; \
		cvs tag ${TAGOPTS} $${TAG}; \
	cvs tag -d $${TAG} */*.uu */*opt_ah.h
# create a tarball of the source for the current tagged version
src_release:
	TAG=`awk '/ATH_HAL_VERSION/ \
		{ gsub("\\"","",$$3); \
		  gsub("[-.]", "_", $$3); print "v" $$3; }' version.h`; \
	DIR=`awk '/ATH_HAL_VERSION/ \
		{ gsub("\\"","",$$3); print "hal-" $$3; }' version.h`; \
	rm -rf $${DIR}; mkdir $${DIR}; \
	cvs export -d $${DIR} -r $${TAG} hal; \
	tar zcf $${DIR}.tgz --exclude=CVS $${DIR}; \
	rm -rf $${DIR};
