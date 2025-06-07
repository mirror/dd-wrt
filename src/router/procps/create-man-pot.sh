#!/bin/sh
#
# Run this command in the top level directory to create the translation template:

SRCDIR=$(dirname ${0})
test -z "${SRCDIR}" && SRCDIR=.

if [ -d po-man ] ; then
  echo "po-man: directory exists, will be reused"
  else
    mkdir -p po-man
fi

PO_FILES="po-man/*.po"

po4a-updatepo -f man -m ${SRCDIR}/free.1 \
                     -m ${SRCDIR}/kill.1 \
                     -m ${SRCDIR}/pgrep.1 \
                     -m ${SRCDIR}/pidof.1 \
                     -m ${SRCDIR}/pkill.1 \
                     -m ${SRCDIR}/pmap.1 \
                     -m ${SRCDIR}/pwdx.1 \
                     -m ${SRCDIR}/skill.1 \
                     -m ${SRCDIR}/slabtop.1 \
                     -m ${SRCDIR}/snice.1 \
                     -m ${SRCDIR}/sysctl.8 \
                     -m ${SRCDIR}/sysctl.conf.5 \
                     -m ${SRCDIR}/tload.1 \
                     -m ${SRCDIR}/uptime.1 \
                     -m ${SRCDIR}/vmstat.8 \
                     -m ${SRCDIR}/w.1 \
                     -m ${SRCDIR}/watch.1 \
                     -p po-man/template-man.pot ${PO_FILES}

po4a-updatepo -f man -m ${SRCDIR}/ps/ps.1 \
                     -p po-man/template-man-ps.pot

po4a-updatepo -f man -m ${SRCDIR}/top/top.1 \
                     -p po-man/template-man-top.pot

# Rename the file according to the version of your (pre-release) tarball.
# Send the new file together with a link to the tarball to the TP coordinators:
# <coordinator@translationproject.org>
