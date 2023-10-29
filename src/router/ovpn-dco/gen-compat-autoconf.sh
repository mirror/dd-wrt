#! /bin/sh
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020-2023 OpenVPN, Inc.
#
#  Author:	Antonio Quartulli <antonio@openvpn.net>

set -e

TARGET=${1:="compat-autoconf.h"}
TMP="${TARGET}.tmp"

echo > "${TMP}"

gen_config() {
	KEY="${1}"
	VALUE="${2}"

	echo "#undef ${KEY}"
	echo "#undef __enabled_${KEY}"
	echo "#undef __enabled_${KEY}_MODULE"
	case "${VALUE}" in
	y)
		echo "#define ${KEY} 1"
		echo "#define __enabled_${KEY} 1"
		echo "#define __enabled_${KEY}_MODULE 0"
		;;
	m)
		echo "#define ${KEY} 1"
		echo "#define __enabled_${KEY} 0"
		echo "#define __enabled_${KEY}_MODULE 1"
		;;
	n)
		echo "#define __enabled_${KEY} 0"
		echo "#define __enabled_${KEY}_MODULE 0"
		;;
	*)
		echo "#define ${KEY} \"${VALUE}\""
		;;
	esac
}

gen_config 'CONFIG_OVPN_DCO_DEBUG' ${CONFIG_OVPN_DCO_DEBUG:="n"} >> "${TMP}"

# only regenerate compat-autoconf.h when config was changed
diff "${TMP}" "${TARGET}" > /dev/null 2>&1 || cp "${TMP}" "${TARGET}"
