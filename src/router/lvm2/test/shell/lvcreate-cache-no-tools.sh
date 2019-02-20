#!/usr/bin/env bash

# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Exercise creation of cache without cache_check


SKIP_WITH_LVMPOLLD=1

. lib/inittest

if test -e LOCAL_CLVMD ; then
# In cluster, the error from activation is logged in clvmd
# so we can only check resulting state of activation
	GREP=echo
else
	GREP=grep
fi

make_fake_() {
	cat <<- EOF >fake-tool.sh
#!/bin/sh
echo "$1"
exit 1
EOF
	chmod +x fake-tool.sh
}

check_change_() {
	lvchange -an $vg |& tee out
	"$GREP" "$1" out

	lvchange -ay $vg |& tee out
	"$GREP" "$1" out
}

# Integrity check fails, but deactivation is OK
check_change_failed_() {
	lvchange -an $vg |& tee out
	"$GREP" "failed" out

	# Activation must fail
	fail lvchange -ay $vg |& tee out
	"$GREP" "failed" out

	cat <<- EOF >fake-tool.sh
#!/bin/sh
exit
EOF
	chmod +x fake-tool.sh
	# Activate without any check
	lvchange -ay $vg
}


aux have_cache 1 3 0 || skip

# FIXME: parallel cache metadata allocator is crashing when used value 8000!
aux prepare_vg 5 80000

aux lvmconf 'global/cache_check_executable = "./fake-tool.sh"'
rm -f fake-tool.sh

# On cache target that supports  V2
if aux have_cache 1 10 0 ; then

lvcreate -aey -l1 -n $lv1 $vg
lvcreate -H -l2 $vg/$lv1

check_change_ "Check is skipped"

# prepare fake version of cache_check tool that reports old version
make_fake_ "0.1.0"
check_change_ "upgrade"

# prepare fake version of cache_check tool that reports garbage
make_fake_ "garbage"
check_change_ "parse"

# prepare fake version of cache_check tool with high version
make_fake_ "99.0.0"
check_change_failed_

lvremove -f $vg

fi

# Enforce older cache target format V1
aux lvmconf 'allocation/cache_metadata_format = 1'

rm -f fake-tool.sh

lvcreate -aey -l1 -n $lv1 $vg
lvcreate -H -l2 $vg/$lv1

check_change_ "Check is skipped"

# prepare fake version of cache_check tool that reports old version
make_fake_ "0.1.0"
check_change_failed_

# prepare fake version of cache_check tool that reports garbage
make_fake_ "garbage"
check_change_failed_

# prepare fake version of cache_check tool with high version
make_fake_ "99.0.0"
check_change_failed_


vgremove -ff $vg
